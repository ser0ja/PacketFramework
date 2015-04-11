/*
 * Copyright (C) 2015 Dehravor <dehravor@gmail.com>
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ByteBuffer.h"
#include <sstream>
#include <cstdio>

#define snprintf _snprintf_s

ByteBufferPositionException::ByteBufferPositionException(bool add, size_t pos, size_t size, size_t valueSize)
{
    std::ostringstream ss;

    ss << "Attempted to " << (add ? "put" : "get") << " value with size: "
       << valueSize << " in ByteBuffer (pos: " << pos << " size: " << size
       << ")";

    message().assign(ss.str());
}

ByteBufferSourceException::ByteBufferSourceException(size_t pos, size_t size, size_t valueSize)
{
    std::ostringstream ss;

    ss << "Attempted to put a "
       << (valueSize > 0 ? "NULL-pointer" : "zero-sized value")
       << " in ByteBuffer (pos: " << pos << " size: " << size << ")";

    message().assign(ss.str());
}

void ByteBuffer::hexlike() const
{
    uint32_t j = 1, k = 1;

    std::ostringstream o;

    char opcodeBuffer[5];
    snprintf(opcodeBuffer, 5, "%04X", GetOpcode());
    o << "OPCODE: 0x" << opcodeBuffer << " SIZE: " << size() << "\n";

    for (uint32_t i = 0; i < size(); ++i)
    {
        char buf[4];
        snprintf(buf, 4, "%02X ", read<uint8_t>(i));
        if ((i == (j * 8)) && ((i != (k * 16))))
        {
            o << "| ";
            ++j;
        }
        else if (i == (k * 16))
        {
            o << "\n";
            ++k;
            ++j;
        }

        o << buf;
    }
    o << " ";
    
    printf("%s\n", o.str().c_str());
}

std::string ByteBuffer::GetOpcodeStr() const
{
    char opcodeBuffer[5];
    snprintf(opcodeBuffer, 5, "%04X", GetOpcode());
    return std::string(opcodeBuffer);
}