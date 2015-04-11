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
#include <lua.hpp>
#include "Luna.h"

class MemoryEditor
{
    public:
        MemoryEditor(lua_State* L);

        int GetBaseAddress(lua_State* L);

        int ReadInt8(lua_State* L);
        int ReadInt16(lua_State* L);
        int ReadInt32(lua_State* L);
        int ReadInt64(lua_State* L);
        int ReadFloat(lua_State* L);
        int ReadDouble(lua_State* L);
        int ReadString(lua_State* L);

        int WriteInt8(lua_State* L);
        int WriteInt16(lua_State* L);
        int WriteInt32(lua_State* L);
        int WriteInt64(lua_State* L);
        int WriteFloat(lua_State* L);
        int WriteDouble(lua_State* L);
        int WriteString(lua_State* L);

        // LUA
        static const char className[];
        static const Luna<MemoryEditor>::PropertyType properties[];
        static const Luna<MemoryEditor>::FunctionType methods[];
};