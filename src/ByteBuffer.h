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

#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

#include <cstdint>
#include <cassert>
#include <exception>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <time.h>
#include <cmath>
#include <type_traits>

// Root of ByteBuffer exception hierarchy
class ByteBufferException : public std::exception
{
public:
    ~ByteBufferException() throw() { }

    char const* what() const throw() override { return msg_.c_str(); }

protected:
    std::string & message() throw() { return msg_; }

private:
    std::string msg_;
};

class ByteBufferPositionException : public ByteBufferException
{
public:
    ByteBufferPositionException(bool add, size_t pos, size_t size, size_t valueSize);

    ~ByteBufferPositionException() throw() { }
};

class ByteBufferSourceException : public ByteBufferException
{
public:
    ByteBufferSourceException(size_t pos, size_t size, size_t valueSize);

    ~ByteBufferSourceException() throw() { }
};

class ByteBuffer
{
    public:
        static size_t const DEFAULT_SIZE = 0x1000;
        static uint8_t const InitialBitPos = 8;

        // constructor
        ByteBuffer() : _rpos(0), _wpos(0), _bitpos(InitialBitPos), _curbitval(0)
        {
            _storage.reserve(DEFAULT_SIZE);
        }

        ByteBuffer(uint32_t opcode) : _rpos(0), _wpos(0), _bitpos(InitialBitPos), _curbitval(0)
        {
            _storage.reserve(DEFAULT_SIZE);
            _storage.resize(4);
            SetOpcode(opcode);
        }

        std::vector<uint8_t>&& Move()
        {
            _rpos = 0;
            _wpos = 0;
            _bitpos = InitialBitPos;
            _curbitval = 0;
            return std::move(_storage);
        }

        ByteBuffer& operator=(ByteBuffer const& right)
        {
            if (this != &right)
            {
                _rpos = right._rpos;
                _wpos = right._wpos;
                _bitpos = right._bitpos;
                _curbitval = right._curbitval;
                _storage = right._storage;
            }

            return *this;
        }

        ByteBuffer& operator=(ByteBuffer&& right)
        {
            if (this != &right)
            {
                _rpos = right._rpos;
                _wpos = right._wpos;
                _bitpos = right._bitpos;
                _curbitval = right._curbitval;
                _storage = right.Move();
            }

            return *this;
        }

        virtual ~ByteBuffer() { }

        void clear()
        {
            _rpos = 0;
            _wpos = 0;
            _bitpos = InitialBitPos;
            _curbitval = 0;
            _storage.clear();
        }

        template <typename T> void append(T value)
        {
            static_assert(std::is_fundamental<T>::value, "append(compound)");
            append((uint8_t *)&value, sizeof(value));
        }

        void FlushBits()
        {
            if (_bitpos == 8)
                return;

            _bitpos = 8;

            append((uint8_t *)&_curbitval, sizeof(uint8_t));
            _curbitval = 0;
        }

        void ResetBitPos()
        {
            if (_bitpos > 7)
                return;

            _bitpos = 8;
            _curbitval = 0;
        }

        bool WriteBit(uint32_t bit)
        {
            --_bitpos;
            if (bit)
                _curbitval |= (1 << (_bitpos));

            if (_bitpos == 0)
            {
                _bitpos = 8;
                append((uint8_t *)&_curbitval, sizeof(_curbitval));
                _curbitval = 0;
            }

            return (bit != 0);
        }

        bool ReadBit()
        {
            ++_bitpos;
            if (_bitpos > 7)
            {
                _curbitval = read<uint8_t>();
                _bitpos = 0;
            }

            return ((_curbitval >> (7-_bitpos)) & 1) != 0;
        }

        template <typename T> void WriteBits(T value, size_t bits)
        {
            for (int32_t i = bits-1; i >= 0; --i)
                WriteBit((value >> i) & 1);
        }

        uint32_t ReadBits(size_t bits)
        {
            uint32_t value = 0;
            for (int32_t i = bits-1; i >= 0; --i)
                if (ReadBit())
                    value |= (1 << (i));

            return value;
        }

        // Reads a byte (if needed) in-place
        void ReadByteSeq(uint8_t& b)
        {
            if (b != 0)
                b ^= read<uint8_t>();
        }

        void WriteByteSeq(uint8_t b)
        {
            if (b != 0)
                append<uint8_t>(b ^ 1);
        }

        template <typename T> void put(size_t pos, T value)
        {
            static_assert(std::is_fundamental<T>::value, "append(compound)");
            put(pos, (uint8_t *)&value, sizeof(value));
        }

        /**
          * @name   PutBits
          * @brief  Places specified amount of bits of value at specified position in packet.
          *         To ensure all bits are correctly written, only call this method after
          *         bit flush has been performed

          * @param  pos Position to place the value at, in bits. The entire value must fit in the packet
          *             It is advised to obtain the position using bitwpos() function.

          * @param  value Data to write.
          * @param  bitCount Number of bits to store the value on.
        */
        template <typename T> void PutBits(size_t pos, T value, uint32_t bitCount)
        {
            if (!bitCount)
                throw ByteBufferSourceException((pos + bitCount) / 8, size(), 0);

            if (pos + bitCount > size() * 8)
                throw ByteBufferPositionException(false, (pos + bitCount) / 8, size(), (bitCount - 1) / 8 + 1);

            for (uint32_t i = 0; i < bitCount; ++i)
            {
                size_t wp = (pos + i) / 8;
                size_t bit = (pos + i) % 8;
                if ((value >> (bitCount - i - 1)) & 1)
                    _storage[wp] |= 1 << (7 - bit);
                else
                    _storage[wp] &= ~(1 << (7 - bit));
            }
        }

        ByteBuffer &operator<<(uint8_t value)
        {
            append<uint8_t>(value);
            return *this;
        }

        ByteBuffer &operator<<(uint16_t value)
        {
            append<uint16_t>(value);
            return *this;
        }

        ByteBuffer &operator<<(uint32_t value)
        {
            append<uint32_t>(value);
            return *this;
        }

        ByteBuffer &operator<<(uint64_t value)
        {
            append<uint64_t>(value);
            return *this;
        }

        // signed as in 2e complement
        ByteBuffer &operator<<(int8_t value)
        {
            append<int8_t>(value);
            return *this;
        }

        ByteBuffer &operator<<(int16_t value)
        {
            append<int16_t>(value);
            return *this;
        }

        ByteBuffer &operator<<(int32_t value)
        {
            append<int32_t>(value);
            return *this;
        }

        ByteBuffer &operator<<(int64_t value)
        {
            append<int64_t>(value);
            return *this;
        }

        // floating points
        ByteBuffer &operator<<(float value)
        {
            append<float>(value);
            return *this;
        }

        ByteBuffer &operator<<(double value)
        {
            append<double>(value);
            return *this;
        }

        ByteBuffer &operator<<(const std::string &value)
        {
            if (size_t len = value.length())
                append((uint8_t const*)value.c_str(), len);
            append<uint8_t>(0);
            return *this;
        }

        ByteBuffer &operator<<(const char *str)
        {
            if (size_t len = (str ? strlen(str) : 0))
                append((uint8_t const*)str, len);
            append<uint8_t>(0);
            return *this;
        }

        ByteBuffer &operator>>(bool &value)
        {
            value = read<char>() > 0 ? true : false;
            return *this;
        }

        ByteBuffer &operator>>(uint8_t &value)
        {
            value = read<uint8_t>();
            return *this;
        }

        ByteBuffer &operator>>(uint16_t &value)
        {
            value = read<uint16_t>();
            return *this;
        }

        ByteBuffer &operator>>(uint32_t &value)
        {
            value = read<uint32_t>();
            return *this;
        }

        ByteBuffer &operator>>(uint64_t &value)
        {
            value = read<uint64_t>();
            return *this;
        }

        //signed as in 2e complement
        ByteBuffer &operator>>(int8_t &value)
        {
            value = read<int8_t>();
            return *this;
        }

        ByteBuffer &operator>>(int16_t &value)
        {
            value = read<int16_t>();
            return *this;
        }

        ByteBuffer &operator>>(int32_t &value)
        {
            value = read<int32_t>();
            return *this;
        }

        ByteBuffer &operator>>(int64_t &value)
        {
            value = read<int64_t>();
            return *this;
        }

        ByteBuffer &operator>>(float &value)
        {
            value = read<float>();
            if (!std::isfinite(value))
                throw ByteBufferException();
            return *this;
        }

        ByteBuffer &operator>>(double &value)
        {
            value = read<double>();
            if (!std::isfinite(value))
                throw ByteBufferException();
            return *this;
        }

        ByteBuffer &operator>>(std::string& value)
        {
            value.clear();
            while (rpos() < size())                         // prevent crash at wrong string format in packet
            {
                char c = read<char>();
                if (c == 0)
                    break;
                value += c;
            }
            return *this;
        }

        uint8_t& operator[](size_t const pos)
        {
            if (pos >= size())
                throw ByteBufferPositionException(false, pos, 1, size());
            return _storage[pos];
        }

        uint8_t const& operator[](size_t const pos) const
        {
            if (pos >= size())
                throw ByteBufferPositionException(false, pos, 1, size());
            return _storage[pos];
        }

        size_t rpos() const { return _rpos; }

        size_t rpos(size_t rpos_)
        {
            _rpos = rpos_;
            return _rpos;
        }

        void rfinish()
        {
            _rpos = wpos();
        }

        size_t wpos() const { return _wpos; }

        size_t wpos(size_t wpos_)
        {
            _wpos = wpos_;
            return _wpos;
        }

        /// Returns position of last written bit
        size_t bitwpos() const { return _wpos * 8 + 8 - _bitpos; }

        size_t bitwpos(size_t newPos)
        {
            _wpos = newPos / 8;
            _bitpos = 8 - (newPos % 8);
            return _wpos * 8 + 8 - _bitpos;
        }

        template<typename T>
        void read_skip() { read_skip(sizeof(T)); }

        void read_skip(size_t skip)
        {
            if (_rpos + skip > size())
                throw ByteBufferPositionException(false, _rpos, skip, size());

            ResetBitPos();
            _rpos += skip;
        }

        template <typename T> T read()
        {
            ResetBitPos();
            T r = read<T>(_rpos);
            _rpos += sizeof(T);
            return r;
        }

        template <typename T> T read(size_t pos) const
        {
            if (pos + sizeof(T) > size())
                throw ByteBufferPositionException(false, pos, sizeof(T), size());
            T val = *((T const*)&_storage[pos]);
            return val;
        }

        void read(uint8_t *dest, size_t len)
        {
            if (_rpos + len > size())
               throw ByteBufferPositionException(false, _rpos, len, size());

            ResetBitPos();
            std::memcpy(dest, &_storage[_rpos], len);
            _rpos += len;
        }

        void ReadPackedUInt64(uint64_t& guid)
        {
            guid = 0;
            ReadPackedUInt64(read<uint8_t>(), guid);
        }

        void ReadPackedUInt64(uint8_t mask, uint64_t& value)
        {
            value = 0;

            for (uint32_t i = 0; i < 8; ++i)
                if (mask & (uint8_t(1) << i))
                    value |= (uint64_t(read<uint8_t>()) << (i * 8));
        }

        std::string ReadString(uint32_t length)
        {
            if (_rpos + length > size())
                throw ByteBufferPositionException(false, _rpos, length, size());

            if (!length)
                return std::string();

            ResetBitPos();
            std::string str((char const*)&_storage[_rpos], length);
            _rpos += length;
            return str;
        }

        //! Method for writing strings that have their length sent separately in packet
        //! without null-terminating the string
        void WriteString(std::string const& str)
        {
            if (size_t len = str.length())
                append(str.c_str(), len);
        }

        void WriteString(char const* str, size_t len)
        {
            if (len)
                append(str, len);
        }

        uint32_t ReadPackedTime()
        {
            uint32_t packedDate = read<uint32_t>();
            tm lt = tm();

            lt.tm_min = packedDate & 0x3F;
            lt.tm_hour = (packedDate >> 6) & 0x1F;
            //lt.tm_wday = (packedDate >> 11) & 7;
            lt.tm_mday = ((packedDate >> 14) & 0x3F) + 1;
            lt.tm_mon = (packedDate >> 20) & 0xF;
            lt.tm_year = ((packedDate >> 24) & 0x1F) + 100;

            return uint32_t(mktime(&lt));
        }

        ByteBuffer& ReadPackedTime(uint32_t& time)
        {
            time = ReadPackedTime();
            return *this;
        }

        uint8_t* contents()
        {
            if (_storage.empty())
                throw ByteBufferException();
            return _storage.data();
        }

        uint8_t const* contents() const
        {
            if (_storage.empty())
                throw ByteBufferException();
            return _storage.data();
        }

        size_t size() const { return _storage.size(); }
        bool empty() const { return _storage.empty(); }

        void resize(size_t newsize)
        {
            _storage.resize(newsize, 0);
            _rpos = 0;
            _wpos = size();
        }

        void reserve(size_t ressize)
        {
            if (ressize > size())
                _storage.reserve(ressize);
        }

        void append(const char *src, size_t cnt)
        {
            return append((const uint8_t *)src, cnt);
        }

        template<class T> void append(const T *src, size_t cnt)
        {
            return append((const uint8_t *)src, cnt * sizeof(T));
        }

        void append(const uint8_t *src, size_t cnt)
        {
            if (!cnt)
                throw ByteBufferSourceException(_wpos, size(), cnt);

            if (!src)
                throw ByteBufferSourceException(_wpos, size(), cnt);

            assert(size() < 10000000);

            FlushBits();

            if (_storage.size() < _wpos + cnt)
                _storage.resize(_wpos + cnt);
            std::memcpy(&_storage[_wpos], src, cnt);
            _wpos += cnt;
        }

        void append(const ByteBuffer& buffer)
        {
            if (buffer.wpos())
                append(buffer.contents(), buffer.wpos());
        }

        // can be used in SMSG_MONSTER_MOVE opcode
        void appendPackXYZ(float x, float y, float z)
        {
            uint32_t packed = 0;
            packed |= ((int)(x / 0.25f) & 0x7FF);
            packed |= ((int)(y / 0.25f) & 0x7FF) << 11;
            packed |= ((int)(z / 0.25f) & 0x3FF) << 22;
            *this << packed;
        }

        void AppendPackedUInt64(uint64_t guid)
        {
            uint8_t mask = 0;
            size_t pos = wpos();
            *this << uint8_t(mask);

            uint8_t packed[8];
            if (size_t packedSize = PackUInt64(guid, &mask, packed))
                append(packed, packedSize);

            put<uint8_t>(pos, mask);
        }

        size_t PackUInt64(uint64_t value, uint8_t* mask, uint8_t* result) const
        {
            size_t resultSize = 0;
            *mask = 0;
            memset(result, 0, 8);

            for (uint8_t i = 0; value != 0; ++i)
            {
                if (value & 0xFF)
                {
                    *mask |= uint8_t(1 << i);
                    result[resultSize++] = uint8_t(value & 0xFF);
                }

                value >>= 8;
            }

            return resultSize;
        }

        void AppendPackedTime(time_t time)
        {
            tm lt;
            localtime_s(&lt, &time);
            append<uint32_t>((lt.tm_year - 100) << 24 | lt.tm_mon  << 20 | (lt.tm_mday - 1) << 14 | lt.tm_wday << 11 | lt.tm_hour << 6 | lt.tm_min);
        }

        void put(size_t pos, const uint8_t *src, size_t cnt)
        {
            if (pos + cnt > size())
                throw ByteBufferPositionException(true, pos, cnt, size());

            if (!src)
                throw ByteBufferSourceException(_wpos, size(), cnt);

            std::memcpy(&_storage[pos], src, cnt);
        }

        void hexlike() const;

        uint32_t GetOpcode() const
        {
            return read<uint32_t>(0);
        }

        std::string GetOpcodeStr() const;

        void SetOpcode(uint32_t opcode)
        {
            put<uint32_t>(0, opcode);
        }
    protected:
        size_t _rpos, _wpos, _bitpos;
        uint8_t _curbitval;
        std::vector<uint8_t> _storage;
};

template <typename T>
inline ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
    b << (uint32_t)v.size();
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
    uint32_t vsize;
    b >> vsize;
    v.clear();
    while (vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename T>
inline ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
    b << (uint32_t)v.size();
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
    uint32_t vsize;
    b >> vsize;
    v.clear();
    while (vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename K, typename V>
inline ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
{
    b << (uint32_t)m.size();
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V>
inline ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
    uint32_t msize;
    b >> msize;
    m.clear();
    while (msize--)
    {
        K k;
        V v;
        b >> k >> v;
        m.insert(make_pair(k, v));
    }
    return b;
}

/// @todo Make a ByteBuffer.cpp and move all this inlining to it.
template<> inline std::string ByteBuffer::read<std::string>()
{
    std::string tmp;
    *this >> tmp;
    return tmp;
}

template<>
inline void ByteBuffer::read_skip<char*>()
{
    std::string temp;
    *this >> temp;
}

template<>
inline void ByteBuffer::read_skip<char const*>()
{
    read_skip<char*>();
}

template<>
inline void ByteBuffer::read_skip<std::string>()
{
    read_skip<char*>();
}

#endif