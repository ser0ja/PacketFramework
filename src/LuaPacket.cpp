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

#include "LuaPacket.h"
#include "Utils.h"
#include "Offset.h"
#include "Hooks.h"
#include "CDataStore.h"
#include "ScriptMgr.h"
#include "Opcodes.h"
#include <sstream>
#include <memory>
#include <iomanip>

LuaPacket::LuaPacket(CDataStore* data) : changed_(false)
{
    append(data->Buffer, data->Length);
    rpos(sizeof(uint32_t));
    wpos(sizeof(uint32_t));
}

LuaPacket::LuaPacket(lua_State* L) : ByteBuffer(0), changed_(false)
{
    uint32_t opcode = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer::SetOpcode(opcode);
    rpos(sizeof(uint32_t));
    wpos(sizeof(uint32_t));
}

bool LuaPacket::IsChanged()
{
    return changed_;
}

int LuaPacket::WriteInt8(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint8_t(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteInt16(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint16_t(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteInt32(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint32_t(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteInt64(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint64_t(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteInt128(lua_State* L)
{
    std::string hexStr = luaL_checkstring(L, 1);

    if (hexStr.length() != 32)
        return luaL_argerror(L, 1, "The Int128 hex-string should be exactly 32 characters long");

    uint64_t high = std::stoull(hexStr.substr(0, 16), 0, 16);
    uint64_t low = std::stoull(hexStr.substr(16, 16), 0, 16);

    uint8_t lowMask = 0;
    uint8_t packedLow[8];
    size_t packedLowSize = PackUInt64(low, &lowMask, packedLow);

    uint8_t highMask = 0;
    uint8_t packedHigh[8];
    size_t packedHighSize = PackUInt64(high, &highMask, packedHigh);

    *this << uint8_t(lowMask);
    *this << uint8_t(highMask);

    if (packedLowSize)
        append(packedLow, packedLowSize);

    if (packedHighSize)
        append(packedHigh, packedHighSize);

    changed_ = true;
    return 0;
}

int LuaPacket::WriteBool(lua_State* L)
{
    bool value = (luaL_checknumber(L, 1) != 0);
    *this << bool(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteFloat(lua_State* L)
{
    double value = luaL_checknumber(L, 1);
    *this << float(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteDouble(lua_State* L)
{
    double value = luaL_checknumber(L, 1);
    *this << double(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteString(lua_State* L)
{
    std::string value = luaL_checkstring(L, 1);
    ByteBuffer::WriteString(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteCString(lua_State* L)
{
    std::string value = luaL_checkstring(L, 1);
    *this << std::string(value);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteBit(lua_State* L)
{
    uint32_t bit = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer::WriteBit(bit);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteBits(lua_State* L)
{
    uint64_t value = static_cast<uint64_t>(luaL_checknumber(L, 1));
    uint32_t bits = static_cast<uint32_t>(luaL_checknumber(L, 2));
    ByteBuffer::WriteBits(value, bits);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteByteSeq(lua_State* L)
{
    uint8_t b = static_cast<uint8_t>(luaL_checknumber(L, 1));
    ByteBuffer::WriteByteSeq(b);
    changed_ = true;
    return 0;
}

int LuaPacket::WriteMyGUID(lua_State* L)
{
    if (!Offset::LocalPlayer || !Offset::LocalPlayerGUID)
        return 0;

    intptr_t localPlayer = *(intptr_t*)(Utils::GetBaseAddress() + Offset::LocalPlayer);

    if (!localPlayer)
        return 0;

    uint64_t low = *(uint64_t*)(localPlayer + Offset::LocalPlayerGUID);
    uint64_t high = *(uint64_t*)(localPlayer + Offset::LocalPlayerGUID + sizeof(uint64_t));

    uint8_t lowMask = 0;
    uint8_t packedLow[8];
    size_t packedLowSize = PackUInt64(low, &lowMask, packedLow);

    uint8_t highMask = 0;
    uint8_t packedHigh[8];
    size_t packedHighSize = PackUInt64(high, &highMask, packedHigh);

    *this << uint8_t(lowMask);
    *this << uint8_t(highMask);

    if (packedLowSize)
        append(packedLow, packedLowSize);

    if (packedHighSize)
        append(packedHigh, packedHighSize);

    changed_ = true;
    return 0;
}

int LuaPacket::WriteTargetGUID(lua_State* L)
{
    if (!Offset::CurrentTargetGUID)
        return 0;

    uint64_t low = *(uint64_t*)(Utils::GetBaseAddress() + Offset::CurrentTargetGUID);
    uint64_t high = *(uint64_t*)(Utils::GetBaseAddress() + Offset::CurrentTargetGUID + sizeof(uint64_t));

    uint8_t lowMask = 0;
    uint8_t packedLow[8];
    size_t packedLowSize = PackUInt64(low, &lowMask, packedLow);

    uint8_t highMask = 0;
    uint8_t packedHigh[8];
    size_t packedHighSize = PackUInt64(high, &highMask, packedHigh);

    *this << uint8_t(lowMask);
    *this << uint8_t(highMask);

    if (packedLowSize)
        append(packedLow, packedLowSize);

    if (packedHighSize)
        append(packedHigh, packedHighSize);

    changed_ = true;
    return 0;
}

int LuaPacket::WriteMouseOverGUID(lua_State* L)
{
    if (!Offset::MouseOverGUID)
        return 0;

    uint64_t low = *(uint64_t*)(Utils::GetBaseAddress() + Offset::MouseOverGUID);
    uint64_t high = *(uint64_t*)(Utils::GetBaseAddress() + Offset::MouseOverGUID + sizeof(uint64_t));

    uint8_t lowMask = 0;
    uint8_t packedLow[8];
    size_t packedLowSize = PackUInt64(low, &lowMask, packedLow);

    uint8_t highMask = 0;
    uint8_t packedHigh[8];
    size_t packedHighSize = PackUInt64(high, &highMask, packedHigh);

    *this << uint8_t(lowMask);
    *this << uint8_t(highMask);

    if (packedLowSize)
        append(packedLow, packedLowSize);

    if (packedHighSize)
        append(packedHigh, packedHighSize);

    changed_ = true;
    return 0;
}

int LuaPacket::ReadInt8(lua_State* L)
{
    uint8_t value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int LuaPacket::ReadInt16(lua_State* L)
{
    uint16_t value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int LuaPacket::ReadInt32(lua_State* L)
{
    uint32_t value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int LuaPacket::ReadInt64(lua_State* L)
{
    uint64_t value;
    *this >> value;

    lua_pushnumber(L, static_cast<lua_Number>(value));
    return 1;
}

int LuaPacket::ReadInt128(lua_State* L)
{
    uint8_t lowMask, highMask;
    *this >> lowMask >> highMask;

    uint64_t high = 0;
    uint64_t low = 0;

    ReadPackedUInt64(lowMask, low);
    ReadPackedUInt64(highMask, high);

    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << high;
    ss << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << low;

    lua_pushstring(L, ss.str().c_str());
    return 1;
}

int LuaPacket::ReadBool(lua_State* L)
{
    bool value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int LuaPacket::ReadFloat(lua_State* L)
{
    float value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int LuaPacket::ReadDouble(lua_State* L)
{
    double value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int LuaPacket::ReadString(lua_State* L)
{
    uint32_t length = static_cast<uint32_t>(luaL_checknumber(L, 1));
    std::string value = ByteBuffer::ReadString(length);
    lua_pushstring(L, value.c_str());
    return 1;
}

int LuaPacket::ReadCString(lua_State* L)
{
    std::string value;
    *this >> value;

    lua_pushstring(L, value.c_str());
    return 1;
}

int LuaPacket::ReadBit(lua_State* L)
{
    bool value = ByteBuffer::ReadBit();
    lua_pushnumber(L, value);
    return 1;
}

int LuaPacket::ReadBits(lua_State* L)
{
    uint32_t size = static_cast<uint32_t>(luaL_checknumber(L, 1));
    uint32_t bits = ByteBuffer::ReadBits(size);
    lua_pushnumber(L, bits);
    return 1;
}

int LuaPacket::ReadByteSeq(lua_State* L)
{
    uint8_t b;
    ByteBuffer::ReadByteSeq(b);
    lua_pushnumber(L, b);
    return 1;
}

int LuaPacket::FlushBits(lua_State* L)
{
    ByteBuffer::FlushBits();
    changed_ = true;
    return 0;
}

int LuaPacket::ResetBitPosition(lua_State* L)
{
    ByteBuffer::ResetBitPos();
    return 0;
}

int LuaPacket::Print(lua_State* L)
{
    hexlike();
    return 0;
}

int LuaPacket::Dump(lua_State* L)
{
    // Save old rpos and seek to the beginning (without the opcode)
    size_t oldrpos = rpos();
    rpos(sizeof(uint32_t));

    uint32_t pushed = 0;

    std::string value = lua_tostring(L, 1);
    std::vector<std::string> parts = Utils::Split(value, ' ');

    if (parts.size() < 1)
        return luaL_argerror(L, 1, "Invalid dump format");

    for (uint32_t i = 0; i < parts.size(); i++)
    {
        if (parts[i] == "int8")
        {
            ReadInt8(L);
            pushed++;
        }
        else if (parts[i] == "int16")
        {
            ReadInt16(L);
            pushed++;
        }
        else if (parts[i] == "int32")
        {
            ReadInt32(L);
            pushed++;
        }
        else if (parts[i] == "int64")
        {
            ReadInt64(L);
            pushed++;
        }
        else if (parts[i] == "int128")
        {
            ReadInt128(L);
            pushed++;
        }
        else if (parts[i] == "float")
        {
            ReadFloat(L);
            pushed++;
        }
        else if (parts[i] == "double")
        {
            ReadDouble(L);
            pushed++;
        }
        else if (parts[i] == "string")
        {
            ReadString(L);
            pushed++;
        }
        else if (parts[i] == "bit")
        {
            ReadBit(L);
            pushed++;
        }
    }

    // Restore rpos
    rpos(oldrpos);
    return pushed;
}

int LuaPacket::GetSize(lua_State* L)
{
    lua_pushnumber(L, size());
    return 1;
}

int LuaPacket::GetOpcode(lua_State* L)
{
    uint32_t opcode = ByteBuffer::GetOpcode();
    lua_pushnumber(L, opcode);
    return 1;
}

int LuaPacket::GetOpcodeStr(lua_State* L)
{
    uint32_t opcode = ByteBuffer::GetOpcode();
    
    for (auto const& opcodeEntry : Opcodes)
    {
        if (opcodeEntry.second == opcode)
        {
            lua_pushstring(L, opcodeEntry.first.c_str());
            return 1;
        }
    }

    lua_pushstring(L, ByteBuffer::GetOpcodeStr().c_str());
    return 1;
}

int LuaPacket::SetOpcode(lua_State* L)
{
    uint32_t opcode = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer::SetOpcode(opcode);
    changed_ = true;
    return 0;
}

int LuaPacket::SetWritePosition(lua_State* L)
{
    uint32_t position = static_cast<uint32_t>(luaL_checknumber(L, 1));
    wpos(position);
    return 0;
}

int LuaPacket::GetWritePosition(lua_State* L)
{
    lua_pushnumber(L, wpos());
    return 1;
}

int LuaPacket::SetReadPosition(lua_State* L)
{
    uint32_t position = static_cast<uint32_t>(luaL_checknumber(L, 1));
    rpos(position);
    return 0;
}

int LuaPacket::GetReadPosition(lua_State* L)
{
    lua_pushnumber(L, rpos());
    return 1;
}

int LuaPacket::SyncWritePosition(lua_State* L)
{
    wpos(rpos());
    return 0;
}

int LuaPacket::SyncReadPosition(lua_State* L)
{
    rpos(wpos());
    return 0;
}

int LuaPacket::Truncate(lua_State* L)
{
    uint32_t position = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer tmp;
    tmp.append(contents(), position);
    clear();
    append(tmp.contents(), tmp.size());
    changed_ = true;
    return 0;
}

int LuaPacket::Process(lua_State* L)
{
    std::shared_ptr<CDataStore> data(new CDataStore(size()));
    memcpy(data->Buffer, contents(), size());

    ProcessMessageHook::Lock();
    ProcessMessageHook::Intercept(false);
    ProcessMessageHook::OriginalFunction(Offset::NetClient, 0, 0, data.get(), 0);
    ProcessMessageHook::Intercept(true);
    ProcessMessageHook::Unlock();
    return 0;
}

int LuaPacket::Send(lua_State* L)
{
    std::shared_ptr<CDataStore> data(new CDataStore(size()));
    memcpy(data->Buffer, contents(), size());

    SendHook::Lock();
    SendHook::Intercept(false);
    SendHook::OriginalFunction(Offset::NetClient, data.get(), 0);
    SendHook::Intercept(true);
    SendHook::Unlock();
    return 0;
}

#define method(class, name) {#name, &class::name}

const char LuaPacket::className[] = "Packet";

const Luna<LuaPacket>::PropertyType LuaPacket::properties[] = {
    { 0, 0 }
};

const Luna<LuaPacket>::FunctionType LuaPacket::methods[] = {
    method(LuaPacket, WriteInt8),
    method(LuaPacket, WriteInt16),
    method(LuaPacket, WriteInt32),
    method(LuaPacket, WriteInt64),
    method(LuaPacket, WriteInt128),
    method(LuaPacket, WriteBool),
    method(LuaPacket, WriteFloat),
    method(LuaPacket, WriteDouble),
    method(LuaPacket, WriteString),
    method(LuaPacket, WriteCString),
    method(LuaPacket, WriteBit),
    method(LuaPacket, WriteBits),
    method(LuaPacket, WriteByteSeq),
    method(LuaPacket, WriteMyGUID),
    method(LuaPacket, WriteTargetGUID),
    method(LuaPacket, WriteMouseOverGUID),

    method(LuaPacket, ReadInt8),
    method(LuaPacket, ReadInt16),
    method(LuaPacket, ReadInt32),
    method(LuaPacket, ReadInt64),
    method(LuaPacket, ReadInt128),
    method(LuaPacket, ReadBool),
    method(LuaPacket, ReadFloat),
    method(LuaPacket, ReadDouble),
    method(LuaPacket, ReadString),
    method(LuaPacket, ReadCString),
    method(LuaPacket, ReadBit),
    method(LuaPacket, ReadBits),
    method(LuaPacket, ReadByteSeq),

    method(LuaPacket, FlushBits),
    method(LuaPacket, ResetBitPosition),

    method(LuaPacket, Print),
    method(LuaPacket, Dump),
    method(LuaPacket, GetSize),
    method(LuaPacket, GetOpcode),
    method(LuaPacket, GetOpcodeStr),
    method(LuaPacket, SetOpcode),
    method(LuaPacket, SetWritePosition),
    method(LuaPacket, GetWritePosition),
    method(LuaPacket, SetReadPosition),
    method(LuaPacket, GetReadPosition),
    method(LuaPacket, SyncWritePosition),
    method(LuaPacket, SyncReadPosition),
    method(LuaPacket, Truncate),

    method(LuaPacket, Process),
    method(LuaPacket, Send),
    { 0, 0 }
};