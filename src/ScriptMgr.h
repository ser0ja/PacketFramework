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
#include <lua.hpp>
#include <map>
#include <string>

struct CDataStore;

class ScriptMgr
{
    public:
        static bool Load(const std::string& scriptName);
        static bool Unload(const std::string& scriptName);
        static bool UnloadAll();
        static bool IsLoaded(const std::string& scriptName);

        static bool OnSend(CDataStore* packet);
        static bool OnProcessMessage(CDataStore* packet);

        static void AddOpcodes(lua_State* L);

    private:
        static std::map<std::string, lua_State*> Scripts;
};