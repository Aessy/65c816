#pragma once

#include <cstdint>
#include <map>
#include <vector>

using Byte = uint8_t;
using Word = uint16_t;
using Address = uint32_t;
using MemoryData = std::map<std::size_t, std::vector<Byte>>;

struct Memory
{
    static constexpr std::size_t ram_size = 512 * 1024;
    std::vector<Byte> ram;
};

inline Address bank(Byte b)
{
    return uint32_t(b) << 16;
}

inline void loadMemory(MemoryData& memory_data, Memory& memory)
{
    for (auto const& [address, data] : memory_data)
    {
        std::copy(data.begin(), data.end(), memory.ram.begin()+address);
    }
}

inline void writeByte(Memory& mem, Address address, Byte value)
{
    // TODO: Add bounds check
    mem.ram[address] = value;
}

inline Byte readByte(Memory const& mem, Address address)
{
    // TODO: Add bounds check
    return mem.ram[address];
}

inline Byte readByte(Memory const& mem, Byte b, Word adr)
{
    return readByte(mem, (Address(b) << 16) | Address(adr));
}

inline Word readWord(Memory const& mem, Address address)
{
    auto const hi = mem.ram[address];
    auto const lo = mem.ram[address+1];

    return Word((hi<<8) | lo);
}

inline Word readWord(Memory const& mem, Byte b, Word addr)
{
    return readWord(mem, bank(b) | addr);
}
