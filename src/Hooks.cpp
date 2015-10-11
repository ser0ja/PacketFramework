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

#include "Hooks.h"
#include "Offset.h"
#include "Utils.h"
#include "ScriptMgr.h"
#include "Console.h"
#include <Windows.h>

SendHook::NetClient_Send2 SendHook::OriginalFunction = reinterpret_cast<SendHook::NetClient_Send2>(Utils::GetBaseAddress() + Offset::NetClient_Send2);
uint8_t SendHook::OriginalASM[ASMSize];
uint8_t SendHook::HookASM[ASMSize];
std::recursive_mutex SendHook::HookMutex;

void SendHook::Initialize()
{
    memcpy(OriginalASM, OriginalFunction, ASMSize);

    uint32_t dst = uint32_t(HookFunction) - uint32_t(OriginalFunction) - (ASMSize - 1);
    HookASM[0] = 0xE9;  // JMP
    memcpy(&HookASM[1], &dst, sizeof(uint32_t));

    Lock();
    Intercept(true);
    Unlock();
}

void SendHook::Intercept(bool apply)
{
    DWORD oldProtect;

    VirtualProtect(reinterpret_cast<void*>(OriginalFunction), ASMSize, PAGE_EXECUTE_READWRITE, &oldProtect);
    {
        memcpy(OriginalFunction, apply ? HookASM : OriginalASM, ASMSize);
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(OriginalFunction), ASMSize);
    }
    VirtualProtect(reinterpret_cast<void*>(OriginalFunction), ASMSize, oldProtect, nullptr);
}

void SendHook::Lock()
{
    HookMutex.lock();
}

void SendHook::Unlock()
{
    HookMutex.unlock();
}

int32_t __fastcall SendHook::HookFunction(intptr_t netClient, intptr_t unk0, CDataStore* packet, intptr_t unk1)
{
    if (!Offset::NetClient)
        Offset::NetClient = netClient;

    if (!Offset::CDataStore_VTable)
        Offset::CDataStore_VTable = packet->VTable - Utils::GetBaseAddress();

    bool allowPacket = ScriptMgr::OnSend(packet);

    if (!allowPacket)
        return 0;

    Lock();
    Intercept(false);
    int32_t ret = OriginalFunction(netClient, packet, unk1);
    Intercept(true);
    Unlock();

    return ret;
}

ProcessMessageHook::NetClient_ProcessMessage ProcessMessageHook::OriginalFunction = reinterpret_cast<ProcessMessageHook::NetClient_ProcessMessage>(Utils::GetBaseAddress() + Offset::NetClient_ProcessMessage);
uint8_t ProcessMessageHook::OriginalASM[ASMSize];
uint8_t ProcessMessageHook::HookASM[ASMSize];
std::recursive_mutex ProcessMessageHook::HookMutex;

void ProcessMessageHook::Initialize()
{
    memcpy(OriginalASM, OriginalFunction, ASMSize);

    uint32_t dst = uint32_t(HookFunction) - uint32_t(OriginalFunction) - (ASMSize - 1);
    HookASM[0] = 0xE9;  // JMP
    memcpy(&HookASM[1], &dst, sizeof(uint32_t));

    Lock();
    Intercept(true);
    Unlock();
}

void ProcessMessageHook::Intercept(bool apply)
{
    DWORD oldProtect;

    VirtualProtect(reinterpret_cast<void*>(OriginalFunction), ASMSize, PAGE_EXECUTE_READWRITE, &oldProtect);
    {
        memcpy(OriginalFunction, apply ? HookASM : OriginalASM, ASMSize);
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(OriginalFunction), ASMSize);
    }
    VirtualProtect(reinterpret_cast<void*>(OriginalFunction), ASMSize, oldProtect, nullptr);
}

void ProcessMessageHook::Lock()
{
    HookMutex.lock();
}

void ProcessMessageHook::Unlock()
{
    HookMutex.unlock();
}

int32_t __fastcall ProcessMessageHook::HookFunction(intptr_t netClient, intptr_t unk0, intptr_t unk1, intptr_t unk2, CDataStore* packet, intptr_t unk3)
{
    if (!Offset::NetClient)
        Offset::NetClient = netClient;

    if (!Offset::CDataStore_VTable)
        Offset::CDataStore_VTable = packet->VTable - Utils::GetBaseAddress();

    bool allowPacket = ScriptMgr::OnProcessMessage(packet);

    if (!allowPacket)
        return 0;

    Lock();
    Intercept(false);
    int32_t ret = OriginalFunction(netClient, unk1, unk2, packet, unk3);
    Intercept(true);
    Unlock();

    return ret;
}