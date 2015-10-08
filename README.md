PacketFramework
=============

PacketFramework is a packet editing framework for World of Warcraft. You may create, send, modify, intercept, block, ... packets with your LUA scripts. The only limit is your imagination.
Here is a [screenshot](http://i.imgur.com/OOiM5Jp.png).

## Build & Usage

This project is a DLL, which you'll need to inject into your running WoW.exe process. Only x86 (32-bit) version of the game is supported at the moment.

The framework tries to locate your scripts under the 'World of Warcraft/Scripts' folder.

## Documentation

Currently there are 4 callbacks that you can override in your scripts.

+ OnLoad
  + Called when the script is loaded
+ OnUnload
  + Called when the script is unloaded
+ OnSend(packet)
  + Called when your client sends a packet
  + Return 'true' means send the packet, 'false' means discard the packet
+ OnProcessMessage(packet)
  + Called when your client receives a packet
  + Return 'true' means process the packet, 'false' means discard the packet

The two main classes usable in scripts are Packet and MemoryEditor.

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
  + Works only if both Offset::LocalPlayer and Offset::LocalPlayerGUID are provided
+ WriteTargetGUID()
  + Works only if Offset::CurrentTargetGUID is provided
+ WriteMouseOverGUID()
  + Works only if Offset::MouseOverGUID is provided
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
+ ReadBits()
+ ReadByteSeq()
+ FlushBits()
+ ResetBitPosition()
+ Print()
  + Prints the raw hex bytes of the packet
+ Dump()
  + It's a shortcut for fast packet reading. You can find an example below.
+ GetSize()
+ GetOpcode()
  + Returns an integer (like 0x1234)
+ GetOpcodeStr()
  + Returns a string (like "CMSG_PING")
+ SetOpcode(opcode)
+ SetWritePosition(wpos)
+ GetWritePosition()
+ SetReadPosition(rpos)
+ GetReadPosition()
+ SyncWritePosition()
  + Sets the write position to the current read position
+ SyncReadPosition()
  + Sets the read position to the current write position
+ Truncate()
  + Deletes everything after position
+ Process()
  + Process the packet with the game client instantly
+ Send()
  + Send the packet to the server instantly
  
Members of **MemoryEditor** (In case you want to modify something in memory)
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

Create a file (ArbitraryName.lua) per example in your 'World of Warcraft/Scripts' directory and type 'load ArbitraryName' in the console after the framework has been injected.

+ Anti-AFK (The OnSend function returns false thus preventing the packet to be sent to the server)
```
function OnSend(packet)
    if (packet.GetOpcode() == CMSG_CHAT_MESSAGE_AFK) then
        return false
    end

    return true
end
```

+ Change MOTD (Creates a new MOTD packet and makes the client process it then discards the original one)
```
function OnProcessMessage(packet)
    if packet.GetOpcode() == SMSG_MOTD then
        local customMOTD = Packet(SMSG_MOTD)
        customMOTD.WriteBits(1, 4)
        customMOTD.FlushBits()
        customMOTD.WriteBits(32, 7)
        customMOTD.FlushBits()
        customMOTD.WriteString("PacketFramework has been loaded!")
        customMOTD.Process()
        return false
    end

    return true
end
```

+ Dump packet contents (just a shortcut)
```
function OnSend(packet)
    if packet.GetOpcode() == SMSG_TIME_SYNC_REQUEST then
        serial, latency = packet.Dump("int32 int32")
        print(serial, latency)
    end
    return true
end
```

+ Memory read (out of date, offsets are for older version)
```
function PrintClientPosition()
    editor = MemoryEditor()
    localPlayer = editor.ReadInt32(editor.GetBaseAddress() + 0x00DBCA04)
    print(editor.ReadFloat(localPlayer + 0xA90), editor.ReadFloat(localPlayer + 0xA94), editor.ReadFloat(localPlayer + 0xA98))
end
```

+ A very simple packet logger
```
function OnSend(packet)
    print("SEND", packet.GetOpcodeStr(), "SIZE", packet.GetSize())
    return true
end

function OnProcessMessage(packet)
    print("RECV", packet.GetOpcode())
    return true
end
```