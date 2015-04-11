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

#include "MemoryEditor.h"
#include "Utils.h"
#include <cstdint>

MemoryEditor::MemoryEditor(lua_State* L)
{

}

int MemoryEditor::GetBaseAddress(lua_State* L)
{
    lua_pushnumber(L, Utils::GetBaseAddress());
    return 1;
}

int MemoryEditor::ReadInt8(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint8_t value = *(uint8_t*)(offset);
    lua_pushnumber(L, value);
    return 1;
}

int MemoryEditor::ReadInt16(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint16_t value = *(uint16_t*)(offset);
    lua_pushnumber(L, value);
    return 1;
}

int MemoryEditor::ReadInt32(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint32_t value = *(uint32_t*)(offset);
    lua_pushnumber(L, value);
    return 1;
}

int MemoryEditor::ReadInt64(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint64_t value = *(uint64_t*)(offset);
    lua_pushnumber(L, static_cast<lua_Number>(value));
    return 1;
}

int MemoryEditor::ReadFloat(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    float value = *(float*)(offset);
    lua_pushnumber(L, static_cast<lua_Number>(value));
    return 1;
}

int MemoryEditor::ReadDouble(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    double value = *(double*)(offset);
    lua_pushnumber(L, value);
    return 1;
}

int MemoryEditor::ReadString(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    const char* value = (const char*)(offset);
    lua_pushstring(L, value);
    return 1;
}

int MemoryEditor::WriteInt8(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint8_t value = static_cast<uint8_t>(luaL_checknumber(L, 2));
    *(uint8_t*)(offset) = value;
    return 0;
}

int MemoryEditor::WriteInt16(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint16_t value = static_cast<uint16_t>(luaL_checknumber(L, 2));
    *(uint16_t*)(offset) = value;
    return 0;
}

int MemoryEditor::WriteInt32(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checknumber(L, 2));
    *(uint32_t*)(offset) = value;
    return 0;
}

int MemoryEditor::WriteInt64(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    uint64_t value = static_cast<uint64_t>(luaL_checknumber(L, 2));
    *(uint64_t*)(offset) = value;
    return 0;
}

int MemoryEditor::WriteFloat(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    float value = static_cast<float>(luaL_checknumber(L, 2));
    *(float*)(offset) = value;
    return 0;
}

int MemoryEditor::WriteDouble(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    double value = luaL_checknumber(L, 2);
    *(double*)(offset) = value;
    return 0;
}

int MemoryEditor::WriteString(lua_State* L)
{
    intptr_t offset = static_cast<intptr_t>(luaL_checknumber(L, 1));
    const char* value = luaL_checkstring(L, 2);
    memcpy(reinterpret_cast<void*>(offset), value, strlen(value) + 1);
    return 0;
}

#define method(class, name) {#name, &class::name}

const char MemoryEditor::className[] = "MemoryEditor";

const Luna<MemoryEditor>::PropertyType MemoryEditor::properties[] = {
    { 0, 0 }
};

const Luna<MemoryEditor>::FunctionType MemoryEditor::methods[] = {
    method(MemoryEditor, GetBaseAddress),

    method(MemoryEditor, ReadInt8),
    method(MemoryEditor, ReadInt16),
    method(MemoryEditor, ReadInt32),
    method(MemoryEditor, ReadInt64),
    method(MemoryEditor, ReadFloat),
    method(MemoryEditor, ReadDouble),
    method(MemoryEditor, ReadString),

    method(MemoryEditor, WriteInt8),
    method(MemoryEditor, WriteInt16),
    method(MemoryEditor, WriteInt32),
    method(MemoryEditor, WriteInt64),
    method(MemoryEditor, WriteFloat),
    method(MemoryEditor, WriteDouble),
    method(MemoryEditor, WriteString),
    { 0, 0 }
};