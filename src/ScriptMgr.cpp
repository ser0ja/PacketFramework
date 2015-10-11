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

#include "ScriptMgr.h"
#include "CDataStore.h"
#include "LuaPacket.h"
#include "Console.h"
#include "Opcodes.h"
#include "MemoryEditor.h"

bool ScriptMgr::Load(const std::string& scriptName)
{
    // Check if script is already loaded
    if (IsLoaded(scriptName))
        Unload(scriptName);

    // Initialize LUA script
    lua_State* state = luaL_newstate();
    luaL_openlibs(state);

    // Register bridged classes
    Luna<LuaPacket>::Register(state);
    Luna<MemoryEditor>::Register(state);

    // Push opcode enum
    AddOpcodes(state);

    // Include '<scriptName>.lua'
    if (luaL_dofile(state, std::string("Scripts/" + scriptName + ".lua").c_str()))
    {
        Console::SetTextColor(RED);
        printf("LUA error: %s\n", lua_tostring(state, -1));
        Console::SetTextColor(DARKGRAY);
        lua_close(state);
        return false;
    }

    // LUA function callback
    if (lua_getglobal(state, "OnLoad"))
    {
        // Call the function
        try
        {
            if (lua_pcall(state, 0, 0, 0))
            {
                std::string error = lua_tostring(state, -1);
                Console::SetTextColor(RED);
                printf("LUA error: %s\n", error.c_str());
                Console::SetTextColor(DARKGRAY);
            }
        }
        catch (const ByteBufferException& e)
        {
            Console::SetTextColor(RED);
            printf("ByteBufferException: %s\n", e.what());
            Console::SetTextColor(DARKGRAY);
        }
    }
    else
    {
        lua_pop(state, 1);
    }

    Scripts[scriptName] = state;
    Console::SetTextColor(YELLOW);
    printf("Script '%s' loaded!\n", scriptName.c_str());
    Console::SetTextColor(DARKGRAY);
    return true;
}

bool ScriptMgr::Unload(const std::string& scriptName)
{
    auto itr = Scripts.find(scriptName);

    if (itr == Scripts.end())
    {
        Console::SetTextColor(RED);
        printf("Script '%s' is not loaded!\n", scriptName.c_str());
        Console::SetTextColor(DARKGRAY);
        return false;
    }

    // LUA function callback
    if (lua_getglobal(itr->second, "OnUnload"))
    {
        // Call the function
        try
        {
            if (lua_pcall(itr->second, 0, 0, 0))
            {
                std::string error = lua_tostring(itr->second, -1);
                Console::SetTextColor(RED);
                printf("LUA error: %s\n", error.c_str());
                Console::SetTextColor(DARKGRAY);
            }
        }
        catch (const ByteBufferException& e)
        {
            Console::SetTextColor(RED);
            printf("ByteBufferException: %s\n", e.what());
            Console::SetTextColor(DARKGRAY);
        }
    }
    else
    {
        lua_pop(itr->second, 1);
    }

    lua_close(itr->second);
    Scripts.erase(itr);
    Console::SetTextColor(YELLOW);
    printf("Script '%s' unloaded!\n", scriptName.c_str());
    Console::SetTextColor(DARKGRAY);
    return true;
}

bool ScriptMgr::UnloadAll()
{
    if (Scripts.empty())
    {
        Console::SetTextColor(RED);
        printf("There are no loaded scripts!\n");
        Console::SetTextColor(DARKGRAY);
    }

    for (auto script : Scripts)
        lua_close(script.second);

    Scripts.clear();
    Console::SetTextColor(YELLOW);
    printf("All scripts are unloaded!\n");
    Console::SetTextColor(DARKGRAY);
    return true;
}

bool ScriptMgr::IsLoaded(const std::string& scriptName)
{
    auto itr = Scripts.find(scriptName);
    return itr != Scripts.end();
}

bool ScriptMgr::OnSend(CDataStore* packet)
{
    bool allowPacket = true;

    for (auto const& script : Scripts)
    {
        // LUA function callback
        if (!lua_getglobal(script.second, "OnSend"))
        {
            lua_pop(script.second, 1);
            continue;
        }

        // Push packet
        LuaPacket* luaPacket = new LuaPacket(packet);
        Luna<LuaPacket>::push(script.second, luaPacket);

        // Call the function
        try
        {
            if (lua_pcall(script.second, 1, 1, 0))
            {
                std::string error = lua_tostring(script.second, -1);
                Console::SetTextColor(RED);
                printf("LUA error: %s\n", error.c_str());
                Console::SetTextColor(DARKGRAY);
            }
        }
        catch (const ByteBufferException& e)
        {
            Console::SetTextColor(RED);
            printf("ByteBufferException: %s\n", e.what());
            Console::SetTextColor(DARKGRAY);
        }

        // Check whether the packet should be handled
        if (!lua_toboolean(script.second, -1))
            allowPacket = false;

        lua_pop(script.second, 1);

        // Apply modifications if necessary
        if (luaPacket->IsChanged())
        {
            if (luaPacket->size() > packet->Length)
            {
                Console::SetTextColor(RED);
                printf("Error: The size of the modified packet must be less than or equal to the original packet's size!\n");
                Console::SetTextColor(DARKGRAY);
                continue;
            }

            memcpy(packet->Buffer, luaPacket->contents(), luaPacket->size());
            packet->Length = luaPacket->size();
        }
    }

    return allowPacket;
}

bool ScriptMgr::OnProcessMessage(CDataStore* packet)
{
    bool allowPacket = true;

    for (auto const& script : Scripts)
    {
        // LUA function callback
        if (!lua_getglobal(script.second, "OnProcessMessage"))
        {
            lua_pop(script.second, 1);
            continue;
        }

        // Push packet
        LuaPacket* luaPacket = new LuaPacket(packet);
        Luna<LuaPacket>::push(script.second, luaPacket);

        // Call the function
        try
        {
            if (lua_pcall(script.second, 1, 1, 0))
            {
                std::string error = lua_tostring(script.second, -1);
                Console::SetTextColor(RED);
                printf("LUA error: %s\n", error.c_str());
                Console::SetTextColor(DARKGRAY);
            }
        }
        catch (const ByteBufferException& e)
        {
            Console::SetTextColor(RED);
            printf("ByteBufferException: %s\n", e.what());
            Console::SetTextColor(DARKGRAY);
        }

        // Check whether the packet should be handled
        if (!lua_toboolean(script.second, -1))
            allowPacket = false;

        lua_pop(script.second, 1);

        // Apply modifications if necessary
        if (luaPacket->IsChanged())
        {
            if (luaPacket->size() > packet->Length)
            {
                Console::SetTextColor(RED);
                printf("Error: The size of the modified packet must be less than or equal to the original packet's size!\n");
                Console::SetTextColor(DARKGRAY);
                continue;
            }

            memcpy(packet->Buffer, luaPacket->contents(), luaPacket->size());
            packet->Length = luaPacket->size();
        }
    }

    return allowPacket;
}

void ScriptMgr::AddOpcodes(lua_State* L)
{
    for (const auto& opcodeEntry : Opcodes)
    {
        lua_pushnumber(L, opcodeEntry.second);
        lua_setglobal(L, opcodeEntry.first.c_str());
    }
};

std::map<std::string, lua_State*> ScriptMgr::Scripts;