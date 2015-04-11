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

#include "HookMgr.h"
#include "Offset.h"
#include "Utils.h"
#include "ScriptMgr.h"
#include "CDataStore.h"
#include "Console.h"
#include <cstring>
#include <cstdio>
#include <Windows.h>

// Original functions (WoW.exe)
HookMgr::NetClient_Send2 HookMgr::Send = reinterpret_cast<HookMgr::NetClient_Send2>(Utils::GetBaseAddress() + Offset::NetClient_Send2);
HookMgr::NetClient_ProcessMessage HookMgr::ProcessMessage = reinterpret_cast<HookMgr::NetClient_ProcessMessage>(Utils::GetBaseAddress() + Offset::NetClient_ProcessMessage);

// ASM Cache
HookASM HookMgr::SendASM;
HookASM HookMgr::ProcessMessageASM;

void HookMgr::Initialize()
{
    // NetClient::Send2
    memcpy(SendASM.Original, Send, SendASM.Size);

    uint32_t dst = uint32_t(HookMgr::SendHook) - uint32_t(Send) - (SendASM.Size - 1);

    SendASM.Hook[0] = 0xE9;
    memcpy(&SendASM.Hook[1], &dst, sizeof(dst));

    // NetClient::ProcessMessage
    memcpy(ProcessMessageASM.Original, ProcessMessage, ProcessMessageASM.Size);

    dst = uint32_t(HookMgr::ProcessMessageHook) - uint32_t(ProcessMessage) - (ProcessMessageASM.Size - 1);

    ProcessMessageASM.Hook[0] = 0xE9;
    memcpy(&ProcessMessageASM.Hook[1], &dst, sizeof(dst));
}

void HookMgr::Intercept(HookType type, bool apply)
{
    DWORD oldProtect;

    if (type & HookSend)
    {
        VirtualProtect(reinterpret_cast<void*>(Send), SendASM.Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        {
            memcpy(Send, apply ? SendASM.Hook : SendASM.Original, SendASM.Size);
            FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(Send), SendASM.Size);
        }
        VirtualProtect(reinterpret_cast<void*>(Send), SendASM.Size, oldProtect, nullptr);
    }

    if (type & HookProcessMessage)
    {
        VirtualProtect(reinterpret_cast<void*>(ProcessMessage), ProcessMessageASM.Size, PAGE_EXECUTE_READWRITE, &oldProtect);
        {
            memcpy(ProcessMessage, apply ? ProcessMessageASM.Hook : ProcessMessageASM.Original, ProcessMessageASM.Size);
            FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(ProcessMessage), ProcessMessageASM.Size);
        }
        VirtualProtect(reinterpret_cast<void*>(ProcessMessage), ProcessMessageASM.Size, oldProtect, nullptr);
    }
}

void HookMgr::OnHook(intptr_t netClient, CDataStore* data)
{
    if (!Offset::NetClient)
    {
        Offset::NetClient = netClient;
        Console::SetTextColor(GREEN);
        printf("NetClient: 0x%08X\n", Offset::NetClient);
        Console::SetTextColor(DARKGRAY);
    }

    if (!Offset::CDataStore_VTable)
    {
        Offset::CDataStore_VTable = data->VTable - Utils::GetBaseAddress();
        Console::SetTextColor(GREEN);
        printf("CDataStore_VTable: 0x%08X (0x%08X)\n", Offset::CDataStore_VTable, data->VTable);
        Console::SetTextColor(DARKGRAY);
    }
}

uint32_t __fastcall HookMgr::SendHook(intptr_t netClient, intptr_t dummy, CDataStore* data, intptr_t unk1)
{
    OnHook(netClient, data);

    bool allowPacket = ScriptMgr::OnSend(data);

    if (!allowPacket)
        return false;

    Intercept(HookSend, false);
    uint32_t ret = Send(netClient, data, unk1);
    Intercept(HookSend, true);
    return ret;
}

bool __fastcall HookMgr::ProcessMessageHook(intptr_t netClient, intptr_t dummy, intptr_t unk1, intptr_t unk2, CDataStore* data, intptr_t unk3)
{
    OnHook(netClient, data);

    bool allowPacket = ScriptMgr::OnProcessMessage(data);

    if (!allowPacket)
        return false;

    Intercept(HookProcessMessage, false);
    bool ret = ProcessMessage(netClient, unk1, unk2, data, unk3);
    Intercept(HookProcessMessage, true);
    return ret;
}