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

#include "Packet.h"
#include "Utils.h"
#include "Offset.h"
#include "HookMgr.h"
#include "CDataStore.h"
#include "ScriptMgr.h"
#include "Opcodes.h"
#include <sstream>
#include <memory>
#include <iomanip>

Packet::Packet(CDataStore* data) : changed_(false)
{
    append(data->Buffer, data->Length);
    rpos(sizeof(uint32_t));
    wpos(sizeof(uint32_t));
}

Packet::Packet(lua_State* L) : ByteBuffer(0), changed_(false)
{
    uint32_t opcode = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer::SetOpcode(opcode);
    rpos(sizeof(uint32_t));
    wpos(sizeof(uint32_t));
}

bool Packet::IsChanged()
{
    return changed_;
}

int Packet::WriteInt8(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint8_t(value);
    changed_ = true;
    return 0;
}

int Packet::WriteInt16(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint16_t(value);
    changed_ = true;
    return 0;
}

int Packet::WriteInt32(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint32_t(value);
    changed_ = true;
    return 0;
}

int Packet::WriteInt64(lua_State* L)
{
    lua_Number value = luaL_checknumber(L, 1);
    *this << uint64_t(value);
    changed_ = true;
    return 0;
}

int Packet::WriteInt128(lua_State* L)
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

int Packet::WriteBool(lua_State* L)
{
    bool value = (luaL_checknumber(L, 1) != 0);
    *this << bool(value);
    changed_ = true;
    return 0;
}

int Packet::WriteFloat(lua_State* L)
{
    double value = luaL_checknumber(L, 1);
    *this << float(value);
    changed_ = true;
    return 0;
}

int Packet::WriteDouble(lua_State* L)
{
    double value = luaL_checknumber(L, 1);
    *this << double(value);
    changed_ = true;
    return 0;
}

int Packet::WriteString(lua_State* L)
{
    std::string value = luaL_checkstring(L, 1);
    ByteBuffer::WriteString(value);
    changed_ = true;
    return 0;
}

int Packet::WriteCString(lua_State* L)
{
    std::string value = luaL_checkstring(L, 1);
    *this << std::string(value);
    changed_ = true;
    return 0;
}

int Packet::WriteBit(lua_State* L)
{
    uint32_t bit = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer::WriteBit(bit);
    changed_ = true;
    return 0;
}

int Packet::WriteBits(lua_State* L)
{
    uint64_t value = static_cast<uint64_t>(luaL_checknumber(L, 1));
    uint32_t bits = static_cast<uint32_t>(luaL_checknumber(L, 2));
    ByteBuffer::WriteBits(value, bits);
    changed_ = true;
    return 0;
}

int Packet::WriteByteSeq(lua_State* L)
{
    uint8_t b = static_cast<uint8_t>(luaL_checknumber(L, 1));
    ByteBuffer::WriteByteSeq(b);
    changed_ = true;
    return 0;
}

int Packet::WriteMyGUID(lua_State* L)
{
    if (!Offset::LocalPlayer || !Offset::LocalPlayerGUID)
        return 0;

    intptr_t localPlayer = *(intptr_t*)(Utils::GetBaseAddress() + Offset::LocalPlayer);
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

int Packet::WriteTargetGUID(lua_State* L)
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

int Packet::WriteMouseOverGUID(lua_State* L)
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

int Packet::ReadInt8(lua_State* L)
{
    uint8_t value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int Packet::ReadInt16(lua_State* L)
{
    uint16_t value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int Packet::ReadInt32(lua_State* L)
{
    uint32_t value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int Packet::ReadInt64(lua_State* L)
{
    uint64_t value;
    *this >> value;

    lua_pushnumber(L, static_cast<lua_Number>(value));
    return 1;
}

int Packet::ReadInt128(lua_State* L)
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

int Packet::ReadBool(lua_State* L)
{
    bool value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int Packet::ReadFloat(lua_State* L)
{
    float value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int Packet::ReadDouble(lua_State* L)
{
    double value;
    *this >> value;

    lua_pushnumber(L, value);
    return 1;
}

int Packet::ReadString(lua_State* L)
{
    uint32_t length = static_cast<uint32_t>(luaL_checknumber(L, 1));
    std::string value = ByteBuffer::ReadString(length);
    lua_pushstring(L, value.c_str());
    return 1;
}

int Packet::ReadCString(lua_State* L)
{
    std::string value;
    *this >> value;

    lua_pushstring(L, value.c_str());
    return 1;
}

int Packet::ReadBit(lua_State* L)
{
    bool value = ByteBuffer::ReadBit();
    lua_pushnumber(L, value);
    return 1;
}

int Packet::ReadBits(lua_State* L)
{
    uint32_t size = static_cast<uint32_t>(luaL_checknumber(L, 1));
    uint32_t bits = ByteBuffer::ReadBits(size);
    lua_pushnumber(L, bits);
    return 1;
}

int Packet::ReadByteSeq(lua_State* L)
{
    uint8_t b;
    ByteBuffer::ReadByteSeq(b);
    lua_pushnumber(L, b);
    return 1;
}

int Packet::FlushBits(lua_State* L)
{
    ByteBuffer::FlushBits();
    changed_ = true;
    return 0;
}

int Packet::ResetBitPosition(lua_State* L)
{
    ByteBuffer::ResetBitPos();
    return 0;
}

int Packet::Print(lua_State* L)
{
    hexlike();
    return 0;
}

int Packet::Dump(lua_State* L)
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

int Packet::GetSize(lua_State* L)
{
    lua_pushnumber(L, size());
    return 1;
}

int Packet::GetOpcode(lua_State* L)
{
    uint32_t opcode = ByteBuffer::GetOpcode();
    lua_pushnumber(L, opcode);
    return 1;
}

int Packet::GetOpcodeStr(lua_State* L)
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

int Packet::SetOpcode(lua_State* L)
{
    uint32_t opcode = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer::SetOpcode(opcode);
    changed_ = true;
    return 0;
}

int Packet::SetWritePosition(lua_State* L)
{
    uint32_t position = static_cast<uint32_t>(luaL_checknumber(L, 1));
    wpos(position);
    return 0;
}

int Packet::GetWritePosition(lua_State* L)
{
    lua_pushnumber(L, wpos());
    return 1;
}

int Packet::SetReadPosition(lua_State* L)
{
    uint32_t position = static_cast<uint32_t>(luaL_checknumber(L, 1));
    rpos(position);
    return 0;
}

int Packet::GetReadPosition(lua_State* L)
{
    lua_pushnumber(L, rpos());
    return 1;
}

int Packet::SyncWritePosition(lua_State* L)
{
    wpos(rpos());
    return 0;
}

int Packet::SyncReadPosition(lua_State* L)
{
    rpos(wpos());
    return 0;
}

int Packet::Truncate(lua_State* L)
{
    uint32_t position = static_cast<uint32_t>(luaL_checknumber(L, 1));
    ByteBuffer tmp;
    tmp.append(contents(), position);
    clear();
    append(tmp.contents(), tmp.size());
    changed_ = true;
    return 0;
}

int Packet::Process(lua_State* L)
{
    // Process packet
    std::shared_ptr<CDataStore> data(new CDataStore(size()));
    memcpy(data->Buffer, contents(), size());

    HookMgr::Intercept(HookProcessMessage, false);
    HookMgr::ProcessMessage(Offset::NetClient, 0, 0, data.get(), 0);
    HookMgr::Intercept(HookProcessMessage, true);
    return 0;
}

int Packet::Send(lua_State* L)
{
    // Send the packet
    std::shared_ptr<CDataStore> data(new CDataStore(size()));
    memcpy(data->Buffer, contents(), size());

    HookMgr::Intercept(HookSend, false);
    HookMgr::Send(Offset::NetClient, data.get(), 0);
    HookMgr::Intercept(HookSend, true);
    return 0;
}

#define method(class, name) {#name, &class::name}

const char Packet::className[] = "Packet";

const Luna<Packet>::PropertyType Packet::properties[] = {
    { 0, 0 }
};

const Luna<Packet>::FunctionType Packet::methods[] = {
    method(Packet, WriteInt8),
    method(Packet, WriteInt16),
    method(Packet, WriteInt32),
    method(Packet, WriteInt64),
    method(Packet, WriteInt128),
    method(Packet, WriteBool),
    method(Packet, WriteFloat),
    method(Packet, WriteDouble),
    method(Packet, WriteString),
    method(Packet, WriteCString),
    method(Packet, WriteBit),
    method(Packet, WriteBits),
    method(Packet, WriteByteSeq),
    method(Packet, WriteMyGUID),
    method(Packet, WriteTargetGUID),
    method(Packet, WriteMouseOverGUID),

    method(Packet, ReadInt8),
    method(Packet, ReadInt16),
    method(Packet, ReadInt32),
    method(Packet, ReadInt64),
    method(Packet, ReadInt128),
    method(Packet, ReadBool),
    method(Packet, ReadFloat),
    method(Packet, ReadDouble),
    method(Packet, ReadString),
    method(Packet, ReadCString),
    method(Packet, ReadBit),
    method(Packet, ReadBits),
    method(Packet, ReadByteSeq),

    method(Packet, FlushBits),
    method(Packet, ResetBitPosition),

    method(Packet, Print),
    method(Packet, Dump),
    method(Packet, GetSize),
    method(Packet, GetOpcode),
    method(Packet, GetOpcodeStr),
    method(Packet, SetOpcode),
    method(Packet, SetWritePosition),
    method(Packet, GetWritePosition),
    method(Packet, SetReadPosition),
    method(Packet, GetReadPosition),
    method(Packet, SyncWritePosition),
    method(Packet, SyncReadPosition),
    method(Packet, Truncate),

    method(Packet, Process),
    method(Packet, Send),
    { 0, 0 }
};