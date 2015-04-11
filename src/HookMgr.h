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
#include <cstdint>

enum HookType
{
    HookSend            = 0x01,
    HookProcessMessage  = 0x02,
    HookBoth            = HookSend | HookProcessMessage
};

struct HookASM
{
    const static uint32_t Size = 0x06;
    uint8_t Original[Size];
    uint8_t Hook[Size];
};

struct CDataStore;

class HookMgr
{
    public:
        static void Initialize();
        static void Intercept(HookType type, bool apply);
        static void OnHook(intptr_t netClient, CDataStore* data);

        typedef uint32_t(__thiscall *NetClient_Send2)(intptr_t netClient, CDataStore* data, intptr_t unk1);
        typedef bool(__thiscall *NetClient_ProcessMessage)(intptr_t netClient, intptr_t unk1, intptr_t unk2, CDataStore* data, intptr_t unk3);

    // Original functions (WoW.exe)
    public:
        static NetClient_Send2 Send;
        static NetClient_ProcessMessage ProcessMessage;

    // Our intercepted functions (PacketFramework.dll)
    public:
        static uint32_t __fastcall SendHook(intptr_t netClient, intptr_t dummy, CDataStore* data, intptr_t unk1);
        static bool __fastcall ProcessMessageHook(intptr_t netClient, intptr_t dummy, intptr_t unk1, intptr_t unk2, CDataStore* data, intptr_t unk3);

    // ASM Cache
    public:
        static HookASM SendASM;
        static HookASM ProcessMessageASM;
};