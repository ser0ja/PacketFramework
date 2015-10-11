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
#include <mutex>
#include "CDataStore.h"

class Hook
{
    public:
        virtual void Initialize() = 0;
        virtual void Intercept(bool apply) = 0;
        virtual void Lock() = 0;
        virtual void Unlock() = 0;

    protected:
        static const uint32_t ASMSize = 0x06;
};

class SendHook : public Hook
{
    public:
        static void Initialize();
        static void Intercept(bool apply);
        static void Lock();
        static void Unlock();

    private:
        typedef int32_t(__thiscall *NetClient_Send2)(intptr_t netClient, CDataStore* packet, intptr_t unk1);

        static uint8_t OriginalASM[ASMSize];
        static uint8_t HookASM[ASMSize];
        static std::recursive_mutex HookMutex;

    public:
        static NetClient_Send2 OriginalFunction;
        static int32_t __fastcall HookFunction(intptr_t netClient, intptr_t unk0, CDataStore* packet, intptr_t unk1);
};

class ProcessMessageHook : public Hook
{
    public:
        static void Initialize();
        static void Intercept(bool apply);
        static void Lock();
        static void Unlock();

    private:
        typedef int32_t(__thiscall *NetClient_ProcessMessage)(intptr_t netClient, intptr_t unk1, intptr_t unk2, CDataStore* packet, intptr_t unk3);

        static uint8_t OriginalASM[ASMSize];
        static uint8_t HookASM[ASMSize];
        static std::recursive_mutex HookMutex;

    public:
        static NetClient_ProcessMessage OriginalFunction;
        static int32_t __fastcall HookFunction(intptr_t netClient, intptr_t unk0, intptr_t unk1, intptr_t unk2, CDataStore* packet, intptr_t unk3);
};