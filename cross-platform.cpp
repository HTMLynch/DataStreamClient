//
// cross-platform.cpp - Platform specific functions for low latency client
//
//
// Written by:  Michael J. Lynch  (mlynch@hi-techiques.com)
//
// Copyright (c) 2023 by Hi-Techniques Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
#include    "ll-client.h"

#if defined(__linux__)
//
// Function used to monitor for keypresses and call a callback function when
// a key is pressed
//
void keyPressMonitor(uint32_t uTimeoutMS, std::function<void(char)> cb)
{
    struct termios  oldSettings, newSettings;

    ::tcgetattr(::fileno(stdin), &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~(ICANON | ECHO);
    ::tcsetattr(::fileno(stdin), TCSANOW, &newSettings);

    std::thread t([uTimeoutMS, cb, oldSettings](void) {
        while(true) {
            fd_set          set;
            struct timeval  tv;

            tv.tv_sec = 0;
            tv.tv_usec = uTimeoutMS * 1000;

            FD_ZERO(&set);
            FD_SET(::fileno(stdin), &set);

            int r = ::select(fileno(stdin)+1, &set, nullptr, nullptr, &tv);
            if(r > 0) {
                char    c;
                ::read(fileno(stdin), &c, 1);
                if(c == 'q') break;
                else cb(c);

            } else if(r < 0) {
                break;

            } else {
                cb(0);
            }
        }

        ::tcsetattr(::fileno(stdin), TCSANOW, &oldSettings);
    });

    t.join();
}

#else
#include    "Windows.h"

//
// Function used to show the last windows error in human readable format
//
static void ShowError(std::string sFuncName)
{
    DWORD   errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        std::cerr << "NO ERROR" << std::endl;
        return;
    }

    LPSTR messageBuffer = nullptr;

    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorMessageID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

    std::string message(messageBuffer, size);

    std::cerr << sFuncName << " error: " << message << std::endl;

    ::LocalFree(messageBuffer);
}

//
// Function used to monitor for keypresses and call a callback function when
// a key is pressed
//
void keyPressMonitor(uint32_t uTimeoutMS, std::function<void(char)> cb)
{
    DWORD           fdwMode;
    HANDLE          hStdin;
    DWORD           fdwSavedMode;

    if((hStdin = ::GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
        ShowError("GetStdHandle(In)");
        cb('q');
        return;
    }

    if(!::GetConsoleMode(hStdin, &fdwSavedMode)) {
        ShowError("GetConsoleMode(In)");
        cb('q');
        return;
    }

    fdwMode = ENABLE_WINDOW_INPUT;

    if(!::SetConsoleMode(hStdin, fdwMode)) {
        ShowError("SetConsoleMode(In)");
        cb('q');
        return;
    }

    std::thread t([uTimeoutMS, cb, hStdin, fdwSavedMode](void) {
        DWORD           cNumRead;
        INPUT_RECORD    irInBuf[128];
        bool            bExit = false;

        while(!bExit) {
            if(!::ReadConsoleInput(hStdin, irInBuf,
                sizeof(irInBuf)/sizeof(irInBuf[0]), &cNumRead)) {
                ShowError("ReadConsoleInput");
                cb('q');
                bExit = true;
            }

            for(DWORD i = 0; i < cNumRead; i++) {
                if(irInBuf[i].EventType == KEY_EVENT) {
                    KEY_EVENT_RECORD    ker = irInBuf[i].Event.KeyEvent;
                    if(ker.bKeyDown) {
                        cb(ker.uChar.AsciiChar);
                        if(ker.uChar.AsciiChar == 'q') {
                            bExit = true;
                            break;
                        }
                    }
                }
            }
        }
    });

    ::SetConsoleMode(hStdin, fdwSavedMode);

    t.join();
}
#endif
