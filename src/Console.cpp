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

#include "Console.h"
#include "ScriptMgr.h"
#include "Utils.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

void Console::Initialize()
{
    // Create console window
    FreeConsole();
    AllocConsole();

    // Redirect streams to the newly created console
    FILE* console = nullptr;
    freopen_s(&console, "CONIN$", "r", stdin);
    freopen_s(&console, "CONOUT$", "w", stdout);
    freopen_s(&console, "CONOUT$", "w", stderr);
}

void Console::HandleCommands()
{
    while (true)
    {
        std::string line;
        std::getline(std::cin, line);

        std::vector<std::string> args = Utils::Split(line, ' ');

        if (args.empty())
        {
            SetTextColor(RED);
            printf("No such command.\n");
            SetTextColor(DARKGRAY);
            continue;
        }

        if (args[0] == "load")
        {
            if (args.size() < 2)
                continue;

            ScriptMgr::Load(args[1]);
        }
        else if (args[0] == "unload")
        {
            if (args.size() < 2)
                continue;

            ScriptMgr::Unload(args[1]);
        }
        else if (args[0] == "unloadall")
        {
            ScriptMgr::UnloadAll();
        }
        else
        {
            SetTextColor(RED);
            printf("No such command.\n");
            SetTextColor(DARKGRAY);
        }

        SetTextColor(DARKGRAY);
        printf(">");
    }
}

BOOL Console::SetTextColor(ConsoleColor color)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    return SetConsoleTextAttribute(console, color);
}