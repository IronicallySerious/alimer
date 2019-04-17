//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../Base/String.h"
#include "../Base/Vector.h"
#include "../Base/WString.h"
#include "Arguments.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{
    static Vector<String> arguments;

    const Vector<String>& ParseArguments(const String& cmdLine, bool skipFirstArgument)
    {
        arguments.Clear();

        size_t cmdStart = 0, cmdEnd = 0;
        bool inCmd = false;
        bool inQuote = false;

        for (size_t i = 0; i < cmdLine.Length(); ++i)
        {
            if (cmdLine[i] == '\"')
                inQuote = !inQuote;
            if (cmdLine[i] == ' ' && !inQuote)
            {
                if (inCmd)
                {
                    inCmd = false;
                    cmdEnd = i;
                    // Do not store the first argument (executable name)
                    if (!skipFirstArgument)
                        arguments.Push(cmdLine.Substring(cmdStart, cmdEnd - cmdStart));
                    skipFirstArgument = false;
                }
            }
            else
            {
                if (!inCmd)
                {
                    inCmd = true;
                    cmdStart = i;
                }
            }
        }
        if (inCmd)
        {
            cmdEnd = cmdLine.Length();
            if (!skipFirstArgument)
                arguments.Push(cmdLine.Substring(cmdStart, cmdEnd - cmdStart));
        }

        // Strip double quotes from the arguments
        for (size_t i = 0; i < arguments.Size(); ++i)
            arguments[i].Replace("\"", "");

        return arguments;
    }

    const Vector<String>& ParseArguments(const char* cmdLine)
    {
        return ParseArguments(String(cmdLine));
    }

    const Vector<String>& ParseArguments(const WString& cmdLine)
    {
        return ParseArguments(String(cmdLine));
    }

    const Vector<String>& ParseArguments(const wchar_t* cmdLine)
    {
        return ParseArguments(String(cmdLine));
    }

    const Vector<String>& ParseArguments(int argc, char** argv)
    {
        String cmdLine;

        for (int i = 0; i < argc; ++i)
            cmdLine.AppendWithFormat("\"%s\" ", (const char*)argv[i]);

        return ParseArguments(cmdLine);
    }

    const Vector<String>& Arguments()
    {
        return arguments;
    }
}
