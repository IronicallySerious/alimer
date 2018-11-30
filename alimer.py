#!/usr/bin/env python
#-*- coding: ascii -*-

# Alimer
# Copyright (c) 2018 Amer Koleci and contributors.
# Licensed under the MIT License.

import argparse, shutil, multiprocessing, os, platform, subprocess, sys
VERSION = '0.9.0'
enableLogVerbose = False
WIN_SDK_VERSION = "10.0.17763.0"
emscriptenToolchain = "CMake/Toolchains/Emscripten.cmake"
emscriptenInstallPrefix = "Alimer-SDK-Web"

def logError(message):
	print("%s\n" % message)
	sys.stdout.flush()
	if 0 == sys.platform.find("win"):
		pauseCmd = "pause"
	else:
		pauseCmd = "read"
	subprocess.call(pauseCmd, shell = True)
	sys.exit(1)

def logInfo(message):
	print(message)
	sys.stdout.flush()

def logWarning(message):
    """print a warning message"""
    print("[WARN] %s" % message)
    sys.stdout.flush()

def logVerbose(message):
    """print a warning message"""
    if enableLogVerbose:
        print(message)
        sys.stdout.flush()

def FindProgramFilesFolder():
	env = os.environ
	if "64bit" == platform.architecture()[0]:
		if "ProgramFiles(x86)" in env:
			programFilesFolder = env["ProgramFiles(x86)"]
		else:
			programFilesFolder = "C:\Program Files (x86)"
	else:
		if "ProgramFiles" in env:
			programFilesFolder = env["ProgramFiles"]
		else:
			programFilesFolder = "C:\Program Files"
	return programFilesFolder

def FindVS2017Folder(programFilesFolder):
	tryVswhereLocation = programFilesFolder + "\\Microsoft Visual Studio\\Installer\\vswhere.exe"
	if os.path.exists(tryVswhereLocation):
		vsLocation = subprocess.check_output([tryVswhereLocation,
			"-latest",
			"-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
			"-property", "installationPath",
			"-version", "[15.0,16.0)",
			"-prerelease"]).decode().split("\r\n")[0]
		tryFolder = vsLocation + "\\VC\\Auxiliary\\Build\\"
		tryVcvarsall = "VCVARSALL.BAT"
		if os.path.exists(tryFolder + tryVcvarsall):
			return tryFolder
	else:
		names = ("Preview", "2017")
		skus = ("Community", "Professional", "Enterprise")
		for name in names:
			for sku in skus:
				tryFolder = programFilesFolder + "\\Microsoft Visual Studio\\%s\\%s\\VC\\Auxiliary\\Build\\" % (name, sku)
				tryVcvarsall = "VCVARSALL.BAT"
				if os.path.exists(tryFolder + tryVcvarsall):
					return tryFolder
	return ""

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
			retCode = subprocess.call(batchFileName, shell = True)
		else:
			subprocess.call("chmod 777 " + batchFileName, shell = True)
			retCode = subprocess.call("./" + batchFileName, shell = True)
		os.remove(batchFileName)
		return retCode

if __name__ == "__main__":
        originalDir = os.path.abspath(os.curdir)

        # Parse argument first.
        parser = argparse.ArgumentParser()
        parser.add_argument('-v', '--verbose', help="Log verbose", action='store_true')
        parser.add_argument('-a', '--action', help="Action to execute", choices=['build', 'generate', 'clean'])
        parser.add_argument('-p', '--platform', help="Build platform", choices=['desktop', 'android', 'ios', 'web', 'all'])
        parser.add_argument('--arch', help="Build architecture", choices=['x64', 'x86', 'ARM'])
        parser.add_argument('-c', '--config', help="Build config", choices=['Debug', 'Dev', 'Release'])
        parser.parse_args()
        args = parser.parse_args()

        # Ensure build folder exists
        if not os.path.exists("build"):
		    os.mkdir("build")

        hostPlatform = sys.platform
        if 0 == hostPlatform.find("win"):
		        hostPlatform = "win"
        elif 0 == hostPlatform.find("linux"):
		        hostPlatform = "linux"
        elif 0 == hostPlatform.find("darwin"):
		        hostPlatform = "darwin"

        if (args.verbose):
            enableLogVerbose = True

        if args.action is not None:
            action = args.action
        else:
            action = 'build'

        if args.platform is not None:
            _platform = args.platform
        else:
            action = 'desktop'

        _platform = args.platform
        if _platform == "desktop":
            if hostPlatform == "win":
                buildSys = "vs2017"
            else:
                buildSys = "ninja"
        elif _platform == "android":
            buildSys = "ninja"
        elif _platform == "web":
            buildSys = "ninja"
        
        logVerbose('Default build set: {}'.format(buildSys))

        if args.arch is not None:
		    arch = args.arch
        else:
            if hostPlatform == "win":
                arch = "x64"
                logVerbose('Default architecture set: {}'.format(arch))
            
        if args.config is not None:
		    configuration = args.config[3]
        else:
		    configuration = "Release"
            # logVerbose('Default configuration set: {}'.format(configuration))

        multiConfig = (buildSys.find("vs") == 0)

        if _platform == "web":
            buildDir = "build/web-%s" % (buildSys)
        else:
            buildDir = "build/%s-%s" % (buildSys, arch)

        if not multiConfig:
                buildDir += "-%s" % configuration;
        if not os.path.exists(buildDir):
		        os.mkdir(buildDir)
        os.chdir(buildDir)
        buildDir = os.path.abspath(os.curdir)

        logVerbose('Executing {}'.format(action))
        if action == "build":
            parallel = multiprocessing.cpu_count()

            logInfo('Building {}-{}-{} parallel {}'.format(buildSys, arch, configuration, parallel))
            batCmd = BatchCommand(hostPlatform)

            if (buildSys != "ninja"):
                if hostPlatform == "win":
                    vs2017Folder = FindVS2017Folder(FindProgramFilesFolder())
                    if "x64" == arch:
                        vcOption = "amd64"
                    elif "x86" == arch:
                        vcOption = "x86"
                    else:
                        logError("Unsupported architecture.\n")
                    batCmd.AddCommand("@call \"%sVCVARSALL.BAT\" %s" % (vs2017Folder, vcOption))
                    batCmd.AddCommand("@cd /d \"%s\"" % buildDir)

            if (buildSys == "ninja"):
                if _platform == "web":
                    batCmd.AddCommand("cmake -G \"Ninja\" -DCMAKE_TOOLCHAIN_FILE=%s -DCMAKE_BUILD_TYPE=\"%s\" -DCMAKE_INSTALL_PREFIX=\"%s\" ../../" % (emscriptenToolchain, configuration, emscriptenInstallPrefix))
                else:
                    if hostPlatform == "win":
                        batCmd.AddCommand("set CC=cl.exe")
                        batCmd.AddCommand("set CXX=cl.exe")
                    batCmd.AddCommand("cmake -G Ninja -DCMAKE_BUILD_TYPE=\"%s\" -DSC_ARCH_NAME=\"%s\" ../../" % (configuration, arch))
                batCmd.AddCommand("ninja -j%d" % parallel)
            else:
                batCmd.AddCommand("cmake -G \"Visual Studio 15 2017\" -T host=x64 -DCMAKE_SYSTEM_VERSION=\"%s\" -A %s ../../" % (WIN_SDK_VERSION, arch))
                batCmd.AddCommand("MSBuild ALL_BUILD.vcxproj /nologo /m:%d /v:m /p:Configuration=%s,Platform=%s" % (parallel, configuration, arch))
            if batCmd.Execute() != 0:
                logError("Build failed.")
        elif action == 'generate':
            logInfo('Generate {}-{}-{}'.format(buildSys, arch, configuration))

            batCmd = BatchCommand(hostPlatform)

            if _platform == "web":
                batCmd.AddCommand("cmake -G \"Ninja\" -DCMAKE_TOOLCHAIN_FILE=%s -DCMAKE_BUILD_TYPE=\"%s\" -DCMAKE_INSTALL_PREFIX=\"%s\" ../../" % (emscriptenToolchain, configuration, emscriptenInstallPrefix))
            elif (buildSys == "ninja"):
                batCmd.AddCommand("cmake -G Ninja -DCMAKE_BUILD_TYPE=\"%s\" ../../" % (configuration))
            else:
                batCmd.AddCommand("cmake -G \"Visual Studio 15 2017\" -T host=x64 -DCMAKE_SYSTEM_VERSION=\"%s\" -A %s ../../" % (WIN_SDK_VERSION, arch))

            if batCmd.Execute() != 0:
                logError("Build failed.")

        elif action == 'clean':
            logInfo('Clean {}-{}-{}'.format(buildSys, arch, configuration))
            os.chdir(originalDir)
            shutil.rmtree(buildDir)
            logInfo("  deleted '{}'".format(buildDir))

        os.chdir(originalDir)
