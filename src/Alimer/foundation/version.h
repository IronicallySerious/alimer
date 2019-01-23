//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#pragma once

#define ALIMER_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))
#define ALIMER_VERSION_GET_MAJOR(version) ((unsigned)(version) >> 22)
#define ALIMER_VERSION_GET_MINOR(version) (((unsigned)(version) >> 12) & 0x3ff)
#define ALIMER_VERSION_GET_PATCH(version) ((unsigned)(version) & 0xfff)

#define ALIMER_VERSION_MAJOR   0
#define ALIMER_VERSION_MINOR   1
#define ALIMER_VERSION_PATCH   0
#define ALIMER_VERSION         ALIMER_MAKE_VERSION(0, 1, 0)
#define ALIMER_VERSION_STR     "0.1.0"
#define ALIMER_GIT_BRANCH "master"
#define ALIMER_GIT_COMMIT_HASH "73bde4c"
