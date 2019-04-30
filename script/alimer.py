#!/usr/bin/env python
# -*- coding: ascii -*-

# Alimer
# Copyright (c) 2018 Amer Koleci and contributors.
# Licensed under the MIT License.

import argparse
import shutil
import multiprocessing
import os
import platform
import subprocess
import sys
VERSION = '0.9.0'
enableLogVerbose = False
WIN_SDK_VERSION = "10.0.17763.0"


def logError(message):
    print("[ERROR] %s" % message)
    sys.stdout.flush()
    if 0 == sys.platform.find("win"):
        pauseCmd = "pause"
    else:
        pauseCmd = "read"
    subprocess.call(pauseCmd, shell=True)
    sys.exit(1)


def logInfo(message):
    print("[INFO] %s" % message)
    sys.stdout.flush()


def logWarning(message):
    print("[WARN] %s" % message)
    sys.stdout.flush()


def logVerbose(message):
    if enableLogVerbose:
        logInfo(message)


def FindProgramFilesFolder():
    env = os.environ
    if "64bit" == platform.architecture()[0]:
        if "ProgramFiles(x86)" in env:
            programFilesFolder = env["ProgramFiles(x86)"]
        else:
            programFilesFolder = r"C:\Program Files (x86)"
    else:
        if "ProgramFiles" in env:
            programFilesFolder = env["ProgramFiles"]
        else:
            programFilesFolder = r"C:\Program Files"
    return programFilesFolder


def FindVS2017OrUpFolder(programFilesFolder, vsVersion, vsName):
    tryVswhereLocation = programFilesFolder + \
        "\\Microsoft Visual Studio\\Installer\\vswhere.exe"
    if os.path.exists(tryVswhereLocation):
        vsLocation = subprocess.check_output([tryVswhereLocation,
                                              "-latest",
                                              "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                                              "-property", "installationPath",
                                              "-version", "[%d.0,%d.0)" % (
                                                  vsVersion, vsVersion + 1),
                                              "-prerelease"]).decode().split("\r\n")[0]
        tryFolder = vsLocation + "\\VC\\Auxiliary\\Build\\"
        tryVcvarsall = "VCVARSALL.BAT"
        if os.path.exists(tryFolder + tryVcvarsall):
            return tryFolder
    else:
        names = ("Preview", vsName)
        skus = ("Community", "Professional", "Enterprise")
        for name in names:
            for sku in skus:
                tryFolder = programFilesFolder + \
                    "\\Microsoft Visual Studio\\%s\\%s\\VC\\Auxiliary\\Build\\" % (
                        name, sku)
                tryVcvarsall = "VCVARSALL.BAT"
                if os.path.exists(tryFolder + tryVcvarsall):
                    return tryFolder
    logError("Could NOT find VS%s.\n" % vsName)
    return ""


def FindVS2019Folder(programFilesFolder):
    return FindVS2017OrUpFolder(programFilesFolder, 16, "2019")


def FindVS2017Folder(programFilesFolder):
    return FindVS2017OrUpFolder(programFilesFolder, 15, "2017")


class BatchCommand:
    def __init__(self, hostPlatform):
        self.commands = []
        self.hostPlatform = hostPlatform

    def AddCommand(self, cmd):
        self.commands += [cmd]

    def Execute(self):
        batchFileName = "scBuild."
        if "win" == self.hostPlatform:
            batchFileName += "bat"
        else:
            batchFileName += "sh"
        batchFile = open(batchFileName, "w")
        batchFile.writelines([cmd_line + "\n" for cmd_line in self.commands])
        batchFile.close()
        if "win" == self.hostPlatform:
            retCode = subprocess.call(batchFileName, shell=True)
        else:
            subprocess.call("chmod 777 " + batchFileName, shell=True)
            retCode = subprocess.call("./" + batchFileName, shell=True)
        os.remove(batchFileName)
        return retCode


if __name__ == "__main__":
    # Parse argument first.
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose',
                        help="Log verbose", action='store_true')
    parser.add_argument('-i', '--input', help="Input source directory")
    parser.add_argument('-a', '--action', help="Action to execute",
                        choices=['build', 'generate', 'clean'])
    parser.add_argument('-p', '--platform', help="Build platform",
                        choices=['desktop', 'android', 'ios', 'web', 'uwp', 'all'])
    parser.add_argument('--buildSystem', help="Build system",
                        choices=['vs2019', 'vs2017', 'ninja'])
    parser.add_argument('-c', '--compiler', help="Build compiler",
                        choices=['vc140', 'vc141', 'gcc7', 'gcc8', 'gcc9', 'clang6', 'clang7', 'clang9'])
    parser.add_argument(
        '--architecture', help="Build architecture", choices=['x64', 'x86', 'ARM'])
    parser.add_argument('--config', help="Build config",
                        choices=['Debug', 'Dev', 'Release'])
    parser.parse_args()
    args = parser.parse_args()

    # Enable log verbosity first.
    if (args.verbose):
        enableLogVerbose = True

    # Setup parameters
    originalDir = os.path.abspath(os.curdir)
    if args.input is None:
        sourceDir = os.path.abspath(os.path.join(os.curdir, os.pardir))
    else:
        sourceDir = args.input
    buildDir = os.path.join(sourceDir, "build")

    hostPlatform = sys.platform
    if 0 == hostPlatform.find("win"):
        hostPlatform = "win"
    elif 0 == hostPlatform.find("linux"):
        hostPlatform = "linux"
    elif 0 == hostPlatform.find("darwin"):
        hostPlatform = "osx"

    # Ensure build folder exists
    if not os.path.exists(buildDir):
        os.mkdir(buildDir)

    if args.action is not None:
        action = args.action
    else:
        action = "build"

    if args.platform is None:
        _platform = "desktop"
    else:
        _platform = args.platform

    if args.buildSystem is None:
        if _platform == "desktop":
            if hostPlatform == "win":
                buildSystem = "vs2019"
            else:
                buildSystem = "ninja"
        elif _platform == "uwp":
            buildSystem = "vs2019"
        elif _platform == "android" or _platform == "web":
            buildSystem = "ninja"
    else:
        buildSystem = args.buildSystem

    if args.compiler is None:
        if buildSystem == "vs2019":
            compiler = "vc142"
        elif buildSystem == "vs2017":
            compiler = "vc141"
        elif buildSystem == "vs2015":
            compiler = "vc140"
        else:
            compiler = "gcc"
    else:
        compiler = args.compiler

    if args.architecture is None:
        if hostPlatform == "win":
            architecture = "x64"
    else:
        architecture = args.architecture

    if args.config is None:
        configuration = "Release"
    else:
        configuration = args.config

    # Log some usefull info
    logVerbose('Host: {}'.format(hostPlatform))
    logVerbose('Platform: {}'.format(_platform))
    if _platform == "desktop":
        logVerbose('Compiler: {}'.format(compiler))
    logVerbose('Action: {}'.format(action))

    multiConfig = (buildSystem.find("vs") == 0)
    if _platform == "desktop":
        buildFolderName = "%s-%s-%s-%s" % (buildSystem,
                                           hostPlatform, compiler, architecture)
    else:
        buildFolderName = "%s-%s" % (buildSystem, _platform)
    if not multiConfig:
        buildFolderName += "-%s" % configuration

    buildDir = os.path.join(buildDir, buildFolderName)
    if not os.path.exists(buildDir):
        os.mkdir(buildDir)

    os.chdir(buildDir)
    buildDir = os.path.abspath(os.curdir)

    logVerbose('Executing {}'.format(action))

    if action == "build" or action == "generate":
        if _platform == "desktop":
            logInfo('Generating build files: {}-{}-{}-{}'.format(buildSystem,
                                                                 hostPlatform, compiler, architecture))
        else:
            logInfo('Generating build files: {}-{}-{}'.format(buildSystem,
                                                              _platform, configuration))

        parallel = multiprocessing.cpu_count()
        batCmd = BatchCommand(hostPlatform)

        if (buildSystem == "vs2019") or (buildSystem == "vs2017") and hostPlatform == "win":
            programFilesFolder = FindProgramFilesFolder()
            if (buildSystem == "vs2019") or ((buildSystem == "ninja") and (compiler == "vc142")):
                vsFolder = FindVS2019Folder(programFilesFolder)
            elif (buildSystem == "vs2017") or ((buildSystem == "ninja") and (compiler == "vc141")):
                vsFolder = FindVS2017Folder(programFilesFolder)
            if architecture == "x64":
                vcOption = "amd64"
            elif architecture == "x86":
                vcOption = "x86"
            else:
                logError("Unsupported architecture.")
            vcToolset = ""
            if (buildSystem == "vs2019") and (compiler == "vc141"):
                vcOption += " -vcvars_ver=14.1"
                vcToolset = "v141,"
            elif ((buildSystem == "vs2019") or (buildSystem == "vs2017")) and (compiler == "vc140"):
                vcOption += " -vcvars_ver=14.0"
                vcToolset = "v140,"

            batCmd.AddCommand("@call \"%sVCVARSALL.BAT\" %s" %
                              (vsFolder, vcOption))
            batCmd.AddCommand("@cd /d \"%s\"" % buildDir)

        if (buildSystem == "ninja"):
            if _platform == "desktop" and hostPlatform == "win":
                batCmd.AddCommand("set CC=cl.exe")
                batCmd.AddCommand("set CXX=cl.exe")

            if _platform == "desktop":
                batCmd.AddCommand(
                    "cmake -G Ninja -DCMAKE_BUILD_TYPE=\"%s\" ../../" % (configuration))
            elif _platform == "web":
                emscriptenSDKDir = os.environ.get('EMSCRIPTEN')
                emscriptenToolchain = os.path.join(
                    emscriptenSDKDir, "cmake/Modules/Platform/Emscripten.cmake")
                emscriptenInstallPrefix = "alimer-sdk-Web"
                batCmd.AddCommand("cmake -G \"Ninja\" -DCMAKE_TOOLCHAIN_FILE=\"%s\" -DCMAKE_BUILD_TYPE=\"%s\" -DCMAKE_INSTALL_PREFIX=\"%s\" ../../" %
                                  (emscriptenToolchain, configuration, emscriptenInstallPrefix))

                # Generate files to run servers
                if not os.path.exists(os.path.join(buildDir, "bin")):
                    os.mkdir(os.path.join(buildDir, "bin"))

                _batchFileExt = "sh"
                if "win" == hostPlatform:
                    _batchFileExt = "bat"

                # Python 2
                serverFile = open(os.path.join(
                    buildDir, "bin", "python_server." + _batchFileExt), "w")
                serverFile.writelines("python -m SimpleHTTPServer")
                serverFile.close()

                # Python 3
                serverFile = open(os.path.join(
                    buildDir, "bin", "python_server3." + _batchFileExt), "w")
                serverFile.writelines("python -m http.server")
                serverFile.close()
            elif _platform == "android":
                androidNDKDir = os.environ.get('ANDROID_NDK')
                androidInstallPrefix = "alimer-sdk-Android"
                batCmd.AddCommand("cmake -G \"Ninja\" -DANDROID_NDK=\"%s\" -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=21 -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang -DCMAKE_ANDROID_STL_TYPE=c++_static -DCMAKE_BUILD_TYPE=\"%s\" -DCMAKE_INSTALL_PREFIX=\"%s\" ../../" % (androidNDKDir, configuration, androidInstallPrefix))

            if action == "build":
                batCmd.AddCommand("ninja -j%d" % parallel)
        else:
            if buildSystem == "vs2019":
                generator = "Visual Studio 16"
            elif buildSystem == "vs2017":
                generator = "Visual Studio 15"

            if _platform == "uwp":
                batCmd.AddCommand("cmake -G \"%s\" -T %shost=x64 -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 -DCMAKE_SYSTEM_VERSION=\"%s\" -A %s ../../" %
                                  (generator, vcToolset, WIN_SDK_VERSION, architecture))
            else:
                batCmd.AddCommand("cmake -G \"%s\" -T %shost=x64 -DCMAKE_SYSTEM_VERSION=\"%s\" -A %s ../../" %
                                  (generator, vcToolset, WIN_SDK_VERSION, architecture))

            if action == "build":
                batCmd.AddCommand("MSBuild ALL_BUILD.vcxproj /nologo /m:%d /v:m /p:Configuration=%s,Platform=%s" %
                                  (parallel, configuration, architecture))

        if batCmd.Execute() != 0:
            logError("Batch command execute failed.")

    # Restore original directory
    os.chdir(originalDir)
    if action == "clean":
        if _platform == "desktop":
            logInfo('Clean: {}-{}-{}-{}'.format(buildSystem,
                                                hostPlatform, compiler, architecture))
        else:
            logInfo('Clean: {}-{}-{}'.format(buildSystem,
                                             _platform, configuration))
        shutil.rmtree(buildDir)
        logInfo("  deleted '{}'".format(buildDir))
