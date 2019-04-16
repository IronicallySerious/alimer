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


#include "alimer_config.h"
#include "core.h"
#include "lua_script.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
typedef struct alimer_emscripten_context {
    lua_State* L;
    lua_State* T;
    int argc;
    char** argv;
} alimer_emscripten_context;

static void emscripten_loop(void* arg) {
    alimer_emscripten_context* context = (alimer_emscripten_context*)arg;
    lua_State* T = context->T;

    luax_geterror(T);
    luax_clearerror(T);
    if (lua_resume(T, 1) != LUA_YIELD) {
        bool restart = lua_type(T, -1) == LUA_TSTRING && !strcmp(lua_tostring(T, -1), "restart");
        int status = lua_tonumber(T, -1);

        lua_close(context->L);
        emscripten_cancel_main_loop();

        if (restart) {
            main(context->argc, context->argv);
        }
        else {
            alimer_platform_shutdown();
            exit(status);
        }
    }
}
#endif /* defined(__EMSCRIPTEN__) */

lua_State* alimer_init(lua_State* L, int argc, char** argv);
void alimer_destroy(void* arg);
int main(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc > 1 && (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v"))) {
        alimer_log_info("Alimer %d.%d.%d (%s)\n", ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH, ALIMER_VERSION_ALIAS);
        exit(0);
    }

    int status;
    bool restart;

    do {
        lua_State* L = luaL_newstate();
        alimer_lua_setmainthread(L);
        luaL_openlibs(L);

        lua_State* T = alimer_init(L, argc, argv);
        if (!T) {
            return 1;
        }

        // alimer_set_error_callback((alimer_error_handler)luax_vthrow, T);

#if defined(__EMSCRIPTEN__)
        alimer_emscripten_context context = { L, T, argc, argv };
        emscripten_set_main_loop_arg(emscripten_loop, (void*)&context, 0, 1);
        return 0;
#else
        while (lua_resume(T, 0) == LUA_YIELD) {
            alimer_sleep_seconds(0.001);
        }

        restart = lua_type(T, -1) == LUA_TSTRING && !strcmp(lua_tostring(T, -1), "restart");
        status = lua_tonumber(T, -1);
        lua_close(L);
#endif
    } while (restart);

    alimer_platform_shutdown();

    return status;
}

lua_State* alimer_init(lua_State* L, int argc, char** argv) {
    //alimer_assert(alimer_platform_init(), "Failed to initialize platform");
    alimer_platform_init();

    lua_State* T = lua_newthread(L);
    lua_pushvalue(L, -2);
    lua_xmove(L, T, 1);
    return T;
}
