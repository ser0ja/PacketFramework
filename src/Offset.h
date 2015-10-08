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

/*
    [Network offsets]
    When a new patch arrives you MUST update the following offsets:
        - NetClient_Send2
        - NetClient_ProcessMessage

    [Helper offsets]
    The following offsets are NOT REQUIRED at all, the program is still fully operational without them:
        - CurrentTargetGUID (Used by LUA helper function Packet::WriteTargetGUID)
        - LocalPlayer (Used by LUA helper function Packet::WriteMyGUID)
        - LocalPlayerGUID (Used by LUA helper function Packet::WriteMyGUID)

    [Dynamic offsets]
    The following offsets are located AUTOMATICALLY by the program, so you don't need to modify them:
        - NetClient
        - CDataStore_VTable
*/

struct Offset
{
    // Network offsets
    static const intptr_t NetClient_Send2           = 0x002948A3;   // 6.2.2 20490
    static const intptr_t NetClient_ProcessMessage  = 0x00292CE5;   // 6.2.2 20490

    // Helper offsets
    static const intptr_t CurrentTargetGUID         = 0x00EAEE50;   // 6.2.2 20490
    static const intptr_t MouseOverGUID             = 0x00EAEE20;   // 6.2.2 20490
    static const intptr_t LocalPlayer               = 0x00E37930;   // 6.2.2 20490
    static const intptr_t LocalPlayerGUID           = 0x28;         // 6.2.2 20490

    // Dynamic offsets
    static intptr_t NetClient;
    static intptr_t CDataStore_VTable;
};
