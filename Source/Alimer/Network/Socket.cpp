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

#include "../Network/Socket.h"
#include "../Core/Log.h"
#include "../Debug/DebugNew.h"

#ifdef _WIN32
#   ifndef NOMINMAX
#       define NOMINMAX 1
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN 1
#   endif
#   include <WinSock2.h>
#   include <WS2tcpip.h>
#   define INVALID_SOCKET_HANDLE INVALID_SOCKET
#else
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <errno.h>
#   include <poll.h>
#   include <netdb.h>
#   include <unistd.h>
#   define INVALID_SOCKET_HANDLE -1
#endif

namespace Alimer
{
#if defined(_WIN32)
    struct SocketInitializer
    {
        SocketInitializer()
        {
            /* Setup WSA */
            WORD sockVersion = MAKEWORD(2, 2);
            WSADATA wsaData;
            int error = WSAStartup(sockVersion, &wsaData);
            if (error != 0)
            {
                ALIMER_LOGERROR("Win32 - Failed to start WinSock failed");
            }

            if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
            {
                ALIMER_LOGERROR("Win32 - Invalid WinSock version");
            }
        }

        ~SocketInitializer()
        {
            WSACleanup();
        }
    };

    SocketInitializer s_win32GlobaSocketInitializer;
#endif

    Socket::Socket()
        : _handle(INVALID_SOCKET_HANDLE)
    {

    }

    Socket::~Socket()
    {
        Close();
    }

    void Socket::Close()
    {
        if (_handle != INVALID_SOCKET_HANDLE)
        {
#if defined(_WIN32)
            closesocket(_handle);
#else
            ::close(_handle);
#endif
            _handle = INVALID_SOCKET_HANDLE;
        }
    }
}
