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
#include "ByteBuffer.h"
#include <lua.hpp>
#include "Luna.h"

struct CDataStore;

class Packet : public ByteBuffer
{
    public:
        // Constructs a packet from a CDataStore object
        Packet(CDataStore* data);
        // Constructs a packet from LUA
        Packet(lua_State* L);

        // Helper
        bool IsChanged();

        // Write
        int WriteInt8(lua_State* L);
        int WriteInt16(lua_State* L);
        int WriteInt32(lua_State* L);
        int WriteInt64(lua_State* L);
        int WriteInt128(lua_State* L);
        int WriteBool(lua_State* L);
        int WriteFloat(lua_State* L);
        int WriteDouble(lua_State* L);
        int WriteString(lua_State* L);
        int WriteCString(lua_State* L);
        int WriteBit(lua_State* L);
        int WriteBits(lua_State* L);
        int WriteByteSeq(lua_State* L);
        int WriteMyGUID(lua_State* L);
        int WriteTargetGUID(lua_State* L);
        int WriteMouseOverGUID(lua_State* L);

        // Read
        int ReadInt8(lua_State* L);
        int ReadInt16(lua_State* L);
        int ReadInt32(lua_State* L);
        int ReadInt64(lua_State* L);
        int ReadInt128(lua_State* L);
        int ReadBool(lua_State* L);
        int ReadFloat(lua_State* L);
        int ReadDouble(lua_State* L);
        int ReadString(lua_State* L);
        int ReadCString(lua_State* L);
        int ReadBit(lua_State* L);
        int ReadBits(lua_State* L);
        int ReadByteSeq(lua_State* L);

        // Bitwise
        int FlushBits(lua_State* L);
        int ResetBitPosition(lua_State* L);

        // Misc
        int Print(lua_State* L);
        int Dump(lua_State* L);
        int GetSize(lua_State* L);
        int GetOpcode(lua_State* L);
        int GetOpcodeStr(lua_State* L);
        int SetOpcode(lua_State* L);
        int SetWritePosition(lua_State* L);
        int GetWritePosition(lua_State* L);
        int SetReadPosition(lua_State* L);
        int GetReadPosition(lua_State* L);
        int SyncWritePosition(lua_State* L);
        int SyncReadPosition(lua_State* L);
        int Truncate(lua_State* L);

        // Network
        int Process(lua_State* L);
        int Send(lua_State* L);

        // LUA
        static const char className[];
        static const Luna<Packet>::PropertyType properties[];
        static const Luna<Packet>::FunctionType methods[];

    private:
        bool changed_;
};