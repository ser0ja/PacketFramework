PacketFramework
=============

PacketFramework is a packet editor framework for retail World of Warcraft using LUA scripts. You can forge new packets, modify existing ones and discard undesired packets at your will.

**WARNING**: Please respect Blizzard's [EULA](http://us.blizzard.com/en-us/company/legal/eula.html) while using the tool. This project is meant for educational purposes only and you are highly discouraged to exploit any aspects of the game.

## How to build

1. Download the source code from the repository
2. Run CMake to create your Visual Studio solution files
3. Compile the DLL with Visual Studio

## How to use

The resulting binary is a DLL file, which you need to inject into your running 'Wow.exe' process. **NOTE:** Only x86 (32-bit) version of the game is supported at the moment. Please ensure that you start the game with the appropriate executable. World of Warcraft Launcher uses 'Wow-64.exe' which is *NOT* supported yet.

The framework tries to locate your scripts under the 'C:/Path/To/Your/World of Warcraft/Scripts' folder. All of your script files must have '.lua' extension, and the name of the file is equivalent to the script's name.

## Console Commands
+ load ScriptName
  + Executes 'C:/Path/To/Your/World of Warcraft/Scripts/ScriptName.lua' and calls its OnLoad callback
+ unload ScriptName
  + Unloads the given script and calls its OnUnload callback
+ unloadall
  + Unloads all loaded scripts

**NOTE:** Closing the console window terminates your World of Warcraft process. If you want to close a script, please use the *unload* command.

## Callbacks

There are 4 callbacks which you can override in your scripts.

+ OnLoad()
  + Called (once) when the script is loaded
  + This method can be considered as a constructor
  + No return value
+ OnUnload()
  + Called (once) when the script is unloaded
  + This method can be considered as a destructor
  + No return value
+ OnSend(packet)
  + Called BEFORE your game client sends a packet (CMSG)
  + Return value *true*: Send the packet to the server
  + Return value *false*: Discard the packet
+ OnProcessMessage(packet)
  + Called BEFORE your game client processes an incoming packet (SMSG)
  + Return value *true*: Process the packet
  + Return value *false*: Discard the packet
 
## Documentation

The two main classes in your scripts are **Packet** and **MemoryEditor**. The Packet class is built on top of TrinityCore's ByteBuffer, so the syntax should be straightforward if you are a developer. The MemoryEditor class helps you to access the memory of the game if you need specific values (e.g. walk speed of the player).

Members of **Packet**
+ WriteInt8(value)
+ WriteInt16(value)
+ WriteInt32(value)
+ WriteInt64(value)
+ WriteInt128(value)
+ WriteBool(value)
+ WriteFloat(value)
+ WriteDouble(value)
+ WriteString(value)
+ WriteCString(value)
+ WriteBit(value)
+ WriteBits(value)
+ WriteByteSeq(value)
+ WriteMyGUID()
+ WriteTargetGUID()
+ WriteMouseOverGUID()
+ ReadInt8()
+ ReadInt16()
+ ReadInt32()
+ ReadInt64()
+ ReadInt128()
+ ReadBool()
+ ReadFloat()
+ ReadDouble()
+ ReadString()
+ ReadCString()
+ ReadBit()
+ ReadBits(size)
+ ReadByteSeq()
+ FlushBits()
+ ResetBitPosition()
+ Print()
  + Prints the content of the packet in hexadecimal form
+ Dump()
  + It's a shortcut for fast packet reading - you'll find an example below.
+ GetSize()
+ GetOpcode()
  + Returns the opcode as an integer (0x1234)
+ GetOpcodeStr()
  + Returns the opcode as a string ("CMSG_PING")
+ SetOpcode(opcode)
+ SetWritePosition(wpos)
+ GetWritePosition()
+ SetReadPosition(rpos)
+ GetReadPosition()
+ SyncWritePosition()
  + Sets the write position to the current read position
+ SyncReadPosition()
  + Sets the read position to the current write position
+ Truncate(position)
  + Deletes everything after the given position
+ Process()
  + Process a packet instantly (SMSG), useful if you create custom packets
+ Send()
  + Send a packet (CMSG) to the server, useful if you create custom packets

Members of **MemoryEditor**
+ GetBaseAddress()
+ ReadInt8(offset)
+ ReadInt16(offset)
+ ReadInt32(offset)
+ ReadInt64(offset)
+ ReadFloat(offset)
+ ReadDouble(offset)
+ ReadString(offset)
+ WriteInt8(offset, value)
+ WriteInt16(offset, value)
+ WriteInt32(offset, value)
+ WriteInt64(offset, value)
+ WriteFloat(offset, value)
+ WriteDouble(offset, value)
+ WriteString(offset, value)

## Examples

Create a file (Example.lua) in your Scripts directory, copy one of the examples below then type 'load Example' in the console after the framework has been injected.

+ Discard a packet
``` lua
function OnSend(packet)
    -- Do not send this specific packet to the server
    if (packet.GetOpcode() == CMSG_CHAT_MESSAGE_AFK) then
        return false
    end

    -- But send everything else
    return true
end
```

+ Create a custom packet
``` lua
function OnProcessMessage(packet)
    -- When message of the day is received
    if (packet.GetOpcode() == SMSG_MOTD) then
        -- Create a custom packet containing our own message
        customMOTD = Packet(SMSG_MOTD)
        customMOTD.WriteBits(1, 4)
        customMOTD.FlushBits()
        customMOTD.WriteBits(32, 7)
        customMOTD.FlushBits()
        customMOTD.WriteString("PacketFramework has been loaded!")
        -- Process our packet
        customMOTD.Process()
        -- Discard the original one
        return false
    end

    return true
end
```

+ Memory read (*offsets are outdated*)
``` lua
function GetPlayerPosition()
    editor = MemoryEditor()
    player = editor.ReadInt32(editor.GetBaseAddress() + 0x00DBCA04)
    x = editor.ReadFloat(player + 0xA90)
    y = editor.ReadFloat(player + 0xA94)
    z = editor.ReadFloat(player + 0xA98)
    print(x, y, z)
end
```

+ A simple packet logger
``` lua
function OnSend(packet)
    print("SEND", packet.GetOpcodeStr(), "SIZE", packet.GetSize())
    return true
end

function OnProcessMessage(packet)
    print("RECV", packet.GetOpcodeStr(), "SIZE", packet.GetSize())
    return true
end
```

+ Read content of packet
``` lua
function OnSend(packet)
    if (packet.GetOpcode() == CMSG_RANDOM_ROLL) then
        min = packet.ReadInt32()
        max = packet.ReadInt32()
        print("Minimum: ", min, "Maximum: ", max)
    end
    
    return true
end
```

+ Modify content of packet
``` lua
function OnSend(packet)
    if (packet.GetOpcode() == CMSG_RANDOM_ROLL) then
        packet.WriteInt32(100)
        packet.WriteInt32(100)
        min = packet.ReadInt32()
        max = packet.ReadInt32()
        print("Minimum: ", min, "Maximum: ", max)
    end
    
    return true
end
```

+ Send custom packet
``` lua
function OnSend(packet)
    -- Send a roll packet on jump
    if (packet.GetOpcode() == CMSG_MOVE_JUMP) then
        customRoll = Packet(CMSG_RANDOM_ROLL)
        customRoll.WriteInt32(1)
        customRoll.WriteInt32(100)
        customRoll.WriteInt8(1)
        customRoll.Send()
    end
    
    return true
end
```

+ Print messages ingame (*packet structure is for 6.2.2*)
``` lua
function PrintIngame(message) 
    packet = Packet(SMSG_CHAT)
    packet.WriteInt8(0)
    packet.WriteInt8(0)
    packet.WriteMyGUID()
    packet.WriteInt128("00000000000000000000000000000000")
    packet.WriteInt128("00000000000000000000000000000000")
    packet.WriteInt128("00000000000000000000000000000000")
    packet.WriteInt32(0)
    packet.WriteInt32(0)
    packet.WriteInt128("00000000000000000000000000000000")
    packet.WriteInt32(0)
    packet.WriteFloat(0)
    packet.WriteBits(0, 11)
    packet.WriteBits(0, 11)
    packet.WriteBits(0, 5)
    packet.WriteBits(0, 7)
    packet.WriteBits(string.len(message), 12)
    packet.WriteBits(0, 11)
    packet.WriteBit(0)
    packet.WriteBit(0)
    packet.FlushBits()
    packet.WriteString(message)
    packet.Process()
end

function OnSend(packet)
    PrintIngame("Sent packet: " .. packet.GetOpcodeStr())
    return true
end
```