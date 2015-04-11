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

#pragma once
#include <Windows.h>
#include <Wincon.h>

enum ConsoleColor
{
    BLACK           = 0,
    DARKBLUE        = FOREGROUND_BLUE,
    DARKGREEN       = FOREGROUND_GREEN,
    DARKCYAN        = FOREGROUND_GREEN | FOREGROUND_BLUE,
    DARKRED         = FOREGROUND_RED,
    DARKMAGENTA     = FOREGROUND_RED | FOREGROUND_BLUE,
    DARKYELLOW      = FOREGROUND_RED | FOREGROUND_GREEN,
    DARKGRAY        = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    GRAY            = FOREGROUND_INTENSITY,
    BLUE            = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    GREEN           = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    CYAN            = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    RED             = FOREGROUND_INTENSITY | FOREGROUND_RED,
    MAGENTA         = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    YELLOW          = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    WHITE           = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
};

class Console
{
    public:
        static void Initialize();
        static void HandleCommands();

        static BOOL SetTextColor(ConsoleColor color);
};