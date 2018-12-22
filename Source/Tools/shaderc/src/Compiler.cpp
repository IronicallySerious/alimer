//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Compiler.h"

#include <dxc/Support/Global.h>
#include <dxc/Support/WinAdapter.h>
#include <dxc/Support/WinIncludes.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <fstream>
#include <dxc/dxcapi.h>

#ifdef _WIN32
#   include <specstrings.h>
#else
#   include <clocale>
#   define _Use_decl_annotations_
#endif

#ifndef _WIN32
// MultiByteToWideChar which is a Windows-specific method.
// This is a very simplistic implementation for non-Windows platforms. This
// implementation completely ignores CodePage and dwFlags.
int MultiByteToWideChar(uint32_t CodePage, uint32_t /*dwFlags*/,
    const char *lpMultiByteStr, int cbMultiByte,
    wchar_t *lpWideCharStr, int cchWideChar) {

    if (cbMultiByte == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    // if cbMultiByte is -1, it indicates that lpMultiByteStr is null-terminated
    // and the entire string should be processed.
    if (cbMultiByte == -1) {
        for (cbMultiByte = 0; lpMultiByteStr[cbMultiByte] != '\0'; ++cbMultiByte)
            ;
        // Add 1 for the null-terminating character.
        ++cbMultiByte;
    }
    // If zero is given as the destination size, this function should
    // return the required size (including the null-terminating character).
    // This is the behavior of mbstowcs when the target is null.
    if (cchWideChar == 0) {
        lpWideCharStr = nullptr;
    }
    else if (cchWideChar < cbMultiByte) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }

    size_t rv;
    const char *locale = CPToLocale(CodePage);
    locale = setlocale(LC_ALL, locale);
    if (lpMultiByteStr[cbMultiByte] != '\0') {
        char *srcStr = (char *)malloc((cbMultiByte + 1) * sizeof(char));
        strncpy(srcStr, lpMultiByteStr, cbMultiByte);
        srcStr[cbMultiByte] = '\0';
        rv = mbstowcs(lpWideCharStr, srcStr, cchWideChar);
        free(srcStr);
    }
    else {
        rv = mbstowcs(lpWideCharStr, lpMultiByteStr, cchWideChar);
    }
    setlocale(LC_ALL, locale);
    if (rv == (size_t)cbMultiByte) return rv;
    return rv + 1; // mbstowcs excludes the terminating character
}
#endif

#if defined(__GNUC__)

#if defined(__i386__) || defined(__x86_64__)
#   define ALIMER_BREAKPOINT() __asm__ __volatile__("int $3\n\t")
#else
#   define ALIMER_BREAKPOINT() ((void)0)
#endif

#   define ALIMER_UNREACHABLE() __builtin_unreachable()

#elif defined(_MSC_VER)

#define ALIMER_BREAKPOINT() __debugbreak()
#define ALIMER_UNREACHABLE() __assume(false)

#else

#define ALIMER_BREAKPOINT() ((void)0)
#define ALIMER_UNREACHABLE()((void)0)

#endif

namespace
{
    bool dllDetaching = false;

    _Use_decl_annotations_ bool UTF8ToUTF16String(const char *pUTF8, size_t cbUTF8, std::wstring *pUTF16)
    {
        DXASSERT_NOMSG(pUTF16 != nullptr);

        // Handle zero-length as a special case; it's a special value to indicate
        // errors in MultiByteToWideChar.
        if (cbUTF8 == 0) {
            pUTF16->resize(0);
            return true;
        }

        int cUTF16 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pUTF8,
            static_cast<int>(cbUTF8), nullptr, 0);
        if (cUTF16 == 0)
            return false;

        pUTF16->resize(cUTF16);

        cUTF16 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pUTF8, static_cast<int>(cbUTF8),
            &(*pUTF16)[0], static_cast<int>(pUTF16->size()));
        DXASSERT(cUTF16 > 0, "otherwise contents changed");
        DXASSERT((*pUTF16)[pUTF16->size()] == L'\0',
            "otherwise wstring didn't null-terminate after resize() call");
        return true;
    }

    _Use_decl_annotations_ bool UTF8ToUTF16String(const char *pUTF8, std::wstring *pUTF16)
    {
        size_t cbUTF8 = (pUTF8 == nullptr) ? 0 : strlen(pUTF8);
        return UTF8ToUTF16String(pUTF8, cbUTF8, pUTF16);
    }

    _Success_(return != false)
        bool UTF16ToEncodedString(_In_z_ const wchar_t* text, DWORD cp, DWORD flags, _Inout_ std::string* pValue, _Out_opt_ bool* lossy) {
        BOOL usedDefaultChar;
        LPBOOL pUsedDefaultChar = (lossy == nullptr) ? nullptr : &usedDefaultChar;
        size_t cUTF16 = wcslen(text);
        if (lossy != nullptr) *lossy = false;

        // Handle zero-length as a special case; it's a special value to indicate errors in WideCharToMultiByte.
        if (cUTF16 == 0) {
            pValue->resize(0);
            DXASSERT(lossy == nullptr || *lossy == false, "otherwise earlier initialization in this function was updated");
            return true;
        }

        int cbUTF8 = ::WideCharToMultiByte(cp, flags, text, 
            static_cast<int>(cUTF16), nullptr, 0, nullptr, pUsedDefaultChar);
        if (cbUTF8 == 0)
            return false;

        pValue->resize(cbUTF8);

        cbUTF8 = ::WideCharToMultiByte(cp, flags, text, static_cast<int>(cUTF16), 
            &(*pValue)[0], static_cast<int>(pValue->size()), nullptr, pUsedDefaultChar);
        DXASSERT(cbUTF8 > 0, "otherwise contents have changed");
        DXASSERT((*pValue)[pValue->size()] == '\0', "otherwise string didn't null-terminate after resize() call");

        if (lossy != nullptr) *lossy = usedDefaultChar;
        return true;
    }

    _Use_decl_annotations_ bool UTF16ToUTF8String(const wchar_t *pUTF16, std::string *pUTF8)
    {
        DXASSERT_NOMSG(pUTF16 != nullptr);
        DXASSERT_NOMSG(pUTF8 != nullptr);
        return UTF16ToEncodedString(pUTF16, CP_UTF8, 0, pUTF8, nullptr);
    }

    class Dxcompiler
    {
    public:
        ~Dxcompiler()
        {
            this->Destroy();
        }

        static Dxcompiler& Instance()
        {
            static Dxcompiler instance;
            return instance;
        }

        IDxcLibrary* Library() const
        {
            return m_library;
        }

        IDxcCompiler* Compiler() const
        {
            return m_compiler;
        }

        void Destroy()
        {
            if (m_dxcompilerDll)
            {
                m_compiler = nullptr;
                m_library = nullptr;

                m_createInstanceFunc = nullptr;

#ifdef _WIN32
                ::FreeLibrary(m_dxcompilerDll);
#else
                ::dlclose(m_dxcompilerDll);
#endif

                m_dxcompilerDll = nullptr;
            }
        }

        void Terminate()
        {
            if (m_dxcompilerDll)
            {
                m_compiler.Detach();
                m_library.Detach();

                m_createInstanceFunc = nullptr;

                m_dxcompilerDll = nullptr;
            }
        }

    private:
        Dxcompiler()
        {
            if (dllDetaching)
            {
                return;
            }

#ifdef _WIN32
            const char* dllName = "dxcompiler.dll";
#elif __APPLE__
            const char* dllName = "libdxcompiler.dylib";
#else
            const char* dllName = "libdxcompiler.so";
#endif
            const char* functionName = "DxcCreateInstance";

#ifdef _WIN32
            m_dxcompilerDll = ::LoadLibraryA(dllName);
#else
            m_dxcompilerDll = ::dlopen(dllName, RTLD_LAZY);
#endif

            if (m_dxcompilerDll != nullptr)
            {
#ifdef _WIN32
                m_createInstanceFunc = (DxcCreateInstanceProc)::GetProcAddress(m_dxcompilerDll, functionName);
#else
                m_createInstanceFunc = (DxcCreateInstanceProc)::dlsym(m_dxcompilerDll, functionName);
#endif

                if (m_createInstanceFunc != nullptr)
                {
                    IFT(m_createInstanceFunc(CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<void**>(&m_library)));
                    IFT(m_createInstanceFunc(CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<void**>(&m_compiler)));
                }
                else
                {
                    this->Destroy();

                    throw std::runtime_error(std::string("COULDN'T get ") + functionName + " from dxcompiler.");
                }
            }
            else
            {
                throw std::runtime_error("COULDN'T load dxcompiler.");
            }
        }

    private:
        HMODULE m_dxcompilerDll = nullptr;
        DxcCreateInstanceProc m_createInstanceFunc = nullptr;

        CComPtr<IDxcLibrary> m_library;
        CComPtr<IDxcCompiler> m_compiler;
    };

    class ScIncludeHandler : public IDxcIncludeHandler
    {
    public:
        explicit ScIncludeHandler(std::function<std::string(const std::string& includeName)> loadCallback)
            : m_loadCallback(std::move(loadCallback))
        {
        }

        HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR fileName, IDxcBlob** includeSource) override
        {
            if ((fileName[0] == L'.') && (fileName[1] == L'/'))
            {
                fileName += 2;
            }

            std::string utf8FileName;
            if (!UTF16ToUTF8String(fileName, &utf8FileName))
            {
                return E_FAIL;
            }

            const std::string source = m_loadCallback(utf8FileName);
            if (source.empty())
            {
                return E_FAIL;
            }

            *includeSource = nullptr;
            return Dxcompiler::Instance().Library()->CreateBlobWithEncodingOnHeapCopy(
                source.data(), static_cast<UINT32>(source.size()), CP_UTF8, reinterpret_cast<IDxcBlobEncoding**>(includeSource));
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            ++m_ref;
            return m_ref;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            --m_ref;
            ULONG result = m_ref;
            if (result == 0)
            {
                delete this;
            }
            return result;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
        {
            if (IsEqualIID(iid, __uuidof(IDxcIncludeHandler)))
            {
                *object = dynamic_cast<IDxcIncludeHandler*>(this);
                this->AddRef();
                return S_OK;
            }
            else if (IsEqualIID(iid, __uuidof(IUnknown)))
            {
                *object = dynamic_cast<IUnknown*>(this);
                this->AddRef();
                return S_OK;
            }
            else
            {
                return E_NOINTERFACE;
            }
        }

    private:
        std::function<std::string(const std::string& includeName)> m_loadCallback;

        std::atomic<ULONG> m_ref = 0;
    };

    std::string DefaultLoadCallback(const std::string& includeName)
    {
        std::vector<char> ret;
        std::ifstream includeFile(includeName, std::ios_base::in);
        includeFile.seekg(0, std::ios::end);
        ret.resize(includeFile.tellg());
        includeFile.seekg(0, std::ios::beg);
        includeFile.read(ret.data(), ret.size());
        while (!ret.empty() && (ret.back() == '\0'))
        {
            ret.pop_back();
        }
        return std::string(ret.data(), ret.size());
    }
}

namespace ShaderCompiler
{
    Compiler::CompileResult CompileToBinary(const Compiler::Options& options)
    {
        assert((options.targetLanguage == ShadingLanguage::DXIL)
            || (options.targetLanguage == ShadingLanguage::HLSL)
            || (options.targetLanguage == ShadingLanguage::SPIRV)
        );

        const bool isDXIL = (options.targetLanguage == ShadingLanguage::DXIL)
            || (options.targetLanguage == ShadingLanguage::SPIRV);

        if (isDXIL)
        {
            std::vector<DxcDefine> dxcDefines;
            std::vector<std::wstring> dxcDefineStrings;
            for (const auto& define : options.defines)
            {
                std::wstring nameUtf16Str;
                UTF8ToUTF16String(define.name.c_str(), &nameUtf16Str);
                dxcDefineStrings.emplace_back(std::move(nameUtf16Str));
                const wchar_t* nameUtf16 = dxcDefineStrings.back().c_str();

                const wchar_t* valueUtf16;
                if (define.value.empty())
                {
                    std::wstring valueUtf16Str;
                    UTF8ToUTF16String(define.value.c_str(), &valueUtf16Str);
                    dxcDefineStrings.emplace_back(std::move(valueUtf16Str));
                    valueUtf16 = dxcDefineStrings.back().c_str();
                }
                else
                {
                    valueUtf16 = nullptr;
                }

                dxcDefines.push_back({ nameUtf16, valueUtf16 });
            }

            CComPtr<IDxcBlobEncoding> sourceBlob;
            IFT(Dxcompiler::Instance().Library()->CreateBlobWithEncodingOnHeapCopy(
                options.source.data(), static_cast<UINT32>(options.source.size()), CP_UTF8, &sourceBlob));
            IFTARG(sourceBlob->GetBufferSize() >= 4);

            for (auto& shader : options.shaders)
            {
                std::wstring shaderProfile;
                switch (shader.stage)
                {
                case ShaderStage::Vertex:
                    shaderProfile = L"vs";
                    break;

                case ShaderStage::Pixel:
                    shaderProfile = L"ps";
                    break;

                case ShaderStage::Geometry:
                    shaderProfile = L"gs";
                    break;

                case ShaderStage::Hull:
                    shaderProfile = L"hs";
                    break;

                case ShaderStage::Domain:
                    shaderProfile = L"ds";
                    break;

                case ShaderStage::Compute:
                    shaderProfile = L"cs";
                    break;

                default:
                    ALIMER_UNREACHABLE();
                }
                shaderProfile += L"_6_0";

                std::wstring entryPointUtf16;
                UTF8ToUTF16String(shader.entry.c_str(), &entryPointUtf16);

                // clang-format off
                std::vector<std::wstring> dxcArgStrings =
                {
                    L"-T", shaderProfile,
                    L"-E", entryPointUtf16,
                };
                // clang-format on

                switch (options.targetLanguage)
                {
                case ShadingLanguage::DXIL:
                    break;

                case ShadingLanguage::SPIRV:
                case ShadingLanguage::HLSL:
                case ShadingLanguage::GLSL:
                case ShadingLanguage::ESSL:
                case ShadingLanguage::MSL:
                    dxcArgStrings.push_back(L"-spirv");
                    break;

                default:
                    ALIMER_UNREACHABLE();
                    break;
                }

                std::wstring shaderNameUtf16;
                UTF8ToUTF16String(options.fileName.c_str(), &shaderNameUtf16);

                std::vector<const wchar_t*> dxcArgs;
                dxcArgs.reserve(dxcArgStrings.size());
                for (const auto& arg : dxcArgStrings)
                {
                    dxcArgs.push_back(arg.c_str());
                }

                CComPtr<IDxcIncludeHandler> includeHandler = new ScIncludeHandler(std::move(options.loadIncludeCallback));
                CComPtr<IDxcOperationResult> compileResult;
                IFT(Dxcompiler::Instance().Compiler()->Compile(
                    sourceBlob, shaderNameUtf16.c_str(), entryPointUtf16.c_str(), shaderProfile.c_str(),
                    dxcArgs.data(), static_cast<UINT32>(dxcArgs.size()), dxcDefines.data(),
                    static_cast<UINT32>(dxcDefines.size()), includeHandler, &compileResult));

                HRESULT status;
                IFT(compileResult->GetStatus(&status));

                Compiler::CompileResult result;
                result.isText = false;

                CComPtr<IDxcBlobEncoding> errors;
                IFT(compileResult->GetErrorBuffer(&errors));
                if (errors != nullptr)
                {
                    result.errorWarningMsg.assign(reinterpret_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize());
                    errors = nullptr;
                }

                result.hasError = true;
                if (SUCCEEDED(status))
                {
                    CComPtr<IDxcBlob> program;
                    IFT(compileResult->GetResult(&program));
                    compileResult = nullptr;
                    if (program != nullptr)
                    {
                        const uint8_t* programPtr = reinterpret_cast<const uint8_t*>(program->GetBufferPointer());
                        result.bytecode.assign(programPtr, programPtr + program->GetBufferSize());

                        result.hasError = false;
                    }
                }
            }
        }
        else
        {
        }

        return {};
    }

    Compiler::CompileResult Compiler::Compile(Compiler::Options options)
    {
        if (!options.loadIncludeCallback)
        {
            options.loadIncludeCallback = DefaultLoadCallback;
        }

        const auto binaryLanguage = options.targetLanguage == ShadingLanguage::DXIL ? ShadingLanguage::DXIL : ShadingLanguage::SPIRV;
        Compiler::CompileResult result = CompileToBinary(options);

        if (!result.hasError && (options.targetLanguage != binaryLanguage))
        {
            //ret = ConvertBinary(ret, source, target);
        }

        return result;

#if TODO
        // Set message options.
        int cOptions = EOptionSpv | EOptionVulkanRules | EOptionLinkProgram;


        std::string def("#extension GL_GOOGLE_include_directive : require\n");

        std::vector<std::string> processes;
        for (auto define : options.defines)
        {
            def += "#define " + define.first;
            if (!define.second.empty()) {
                def += std::string(" ") + define.second;
            }
            else
            {
                def += std::string(" 1");
            }
            def += std::string("\n");

            char process[256];
            snprintf(process, sizeof(process), "D%s", define.first.c_str());
            processes.push_back(process);
        }

        auto firstExtStart = options.inputFile.find_last_of(".");
        bool hasFirstExt = firstExtStart != string::npos;
        auto secondExtStart = hasFirstExt ? options.inputFile.find_last_of(".", firstExtStart - 1) : string::npos;
        bool hasSecondExt = secondExtStart != string::npos;
        string firstExt = options.inputFile.substr(firstExtStart + 1);
        bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");

        glslang::EShClient client = glslang::EShClientVulkan;
        glslang::EShSource source = glslang::EShSourceGlsl;
        if (usesUnifiedExt && firstExt == "hlsl")
        {
            source = glslang::EShSourceHlsl;
            cOptions |= EOptionReadHlsl;
        }

        string stageName;
        if (hasFirstExt && !usesUnifiedExt)
        {
            stageName = firstExt;
        }
        else if (usesUnifiedExt && hasSecondExt)
        {
            stageName = options.inputFile.substr(secondExtStart + 1, firstExtStart - secondExtStart - 1);
        }

        std::ifstream stream(options.inputFile);
        std::string shaderSource((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        CompilerShaderStage stage = CompilerShaderStage::Count;
        if (stageName == "vert")
            stage = CompilerShaderStage::Vertex;
        else if (stageName == "tesc")
            stage = CompilerShaderStage::TessControl;
        else if (stageName == "tese")
            stage = CompilerShaderStage::TessEvaluation;
        else if (stageName == "geom")
            stage = CompilerShaderStage::Geometry;
        else if (stageName == "frag")
            stage = CompilerShaderStage::Fragment;
        else if (stageName == "comp")
            stage = CompilerShaderStage::Compute;

        EShMessages messages = EShMsgDefault;
        SetMessageOptions(cOptions, messages);
        int defaultVersion = 100;

        EShLanguage language = MapShaderStage(stage);
        glslang::TShader shader(language);
        const char* shaderStrings = shaderSource.c_str();
        const int shaderLengths = static_cast<int>(shaderSource.length());
        const char* stringNames = options.inputFile.c_str();
        shader.setStringsWithLengthsAndNames(&shaderStrings, &shaderLengths, &stringNames, 1);
        shader.setInvertY(options.invertY ? true : false);
        shader.setEnvInput(source, language, client, defaultVersion);
        shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
        //shader.setEntryPoint(options.entryPoint.c_str());
        //shader.setEnvTargetHlslFunctionality1();
        shader.setShiftSamplerBinding(0);
        shader.setShiftTextureBinding(0);
        shader.setShiftImageBinding(0);
        shader.setShiftUboBinding(0);
        shader.setShiftSsboBinding(0);
        shader.setFlattenUniformArrays(false);
        shader.setNoStorageFormat(false);
        shader.setPreamble(def.c_str());
        shader.addProcesses(processes);

        auto resourceLimits = glslang::DefaultTBuiltInResource;

        if (!shader.parse(&glslang::DefaultTBuiltInResource, defaultVersion, false, messages))
        {
            _errorMessage = shader.getInfoLog();

            return false;
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages))
        {
            _errorMessage = program.getInfoLog();
            return false;
        }

        // Map IO for SPIRV generation.
        if (!program.mapIO())
        {
            _errorMessage = program.getInfoLog();
            return false;
        }

        if (program.getIntermediate(language))
        {
            glslang::SpvOptions spvOptions;
            //spvOptions.generateDebugInfo = false;
            //spvOptions.disableOptimizer = true;
            //spvOptions.optimizeSize = false;
            spvOptions.validate = true;

            spv::SpvBuildLogger logger;
            std::vector<uint32_t> spirv;
            glslang::GlslangToSpv(
                *program.getIntermediate(language),
                spirv,
                &logger,
                &spvOptions);

            /* Write output spv first. */
            CompilerShaderFormat format = options.format;
            if (format == CompilerShaderFormat::DEFAULT)
            {
                auto fileExt = GetExtension(options.outputFile);
                if (fileExt == "h")
                {
                    format = CompilerShaderFormat::C_HEADER;
                }
                else
                {
                    format = CompilerShaderFormat::BLOB;
                }
            }

            switch (format)
            {
            case CompilerShaderFormat::DEFAULT:
            case CompilerShaderFormat::BLOB:
                WriteSpirv(options.outputFile, spirv);
                break;
            case Alimer::CompilerShaderFormat::C_HEADER:
            {
                std::vector<uint8_t> bytecode;
                bytecode.resize(spirv.size() * sizeof(uint32_t));
                memcpy(bytecode.data(), spirv.data(), bytecode.size());
                string fileName = RemoveExtension(GetFileName(options.inputFile));
                fileName += "_" + GetStageName(language);
                WriteCHeader(options.outputFile, fileName, bytecode);
            }
            break;
            default:
                break;
            }
        }

        return true;
#endif // TODO

    }
}

#ifdef _WIN32
#include <windows.h>
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    UNREFERENCED_PARAMETER(instance);

    BOOL result = TRUE;
    if (reason == DLL_PROCESS_DETACH)
    {
        dllDetaching = true;

        if (reserved == 0)
        {
            // FreeLibrary has been called or the DLL load failed
            Dxcompiler::Instance().Destroy();
        }
        else
        {
            // Process termination. We should not call FreeLibrary()
            Dxcompiler::Instance().Terminate();
        }
    }

    return result;
}
#endif
