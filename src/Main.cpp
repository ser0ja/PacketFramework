/*
 * Copyright (C) 2015 Dehravor <dehravor@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Utils.h"
#include "Console.h"
#include "Hooks.h"
#include "Offset.h"
#include "Version.h"
#include <Windows.h>
#include <cstdio>

int main()
{
    Console::Initialize();

    Console::SetTextColor(CYAN);
    printf("PacketFramework for %s (%d)\n", Version.c_str(), Build);
    printf("Source code is available at https://github.com/Dehravor/PacketFramework\n");
    Console::SetTextColor(GREEN);
    printf("[Commands]\n");
    printf(" - load scriptname\n");
    printf(" - unload scriptname\n");
    printf(" - unloadall\n");
    Console::SetTextColor(DARKGRAY);
    printf(">");

    SendHook::Initialize();
    ProcessMessageHook::Initialize();

    Console::HandleCommands();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&main, NULL, 0, NULL);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return true;
}

