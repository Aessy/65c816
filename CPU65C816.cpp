#include "CPU56C816.h"
#include "Memory.h"

#include <functional>
#include <string>

struct InstructionInfo
{
    Address effective_address;
    Word address; // Address within a bank (24 bit)
};

enum class AddressingMode
{
    Absolute,
    AbsoluteX,
    AbsoluteY,
    AbsoluteIndirect,
    AbsoluteIndirectX,
    AbsoluteIndirectY,
    Accumulator,
    DirectPage,
    DirectPageX,
    DirectPageY,
    DirectPageIndirectX,
    Immediate
};

struct Instruction
{
    std::string name;
    AddressingMode mode;
    Byte size;
    Byte cycles;
    std::function<void(Cpu65C816&, Memory&, InstructionInfo const& info)> func;
};

void pushByte(Cpu65C816& cpu, Memory& mem, Byte value)
{
    writeByte(mem, cpu.sp.w, value);

    if (cpu.emulation_mode_flag)
    {
        --cpu.sp.b;
    }
    else
    {
        --cpu.sp.w;
    }
}

Byte pullByte(Cpu65C816& cpu, Memory& mem)
{
    if (cpu.emulation_mode_flag)
    {
        ++cpu.sp.b;
    }
    else
    {
        ++cpu.sp.w;
    }

    return readByte(mem, cpu.sp.w);
}

void pushWord(Cpu65C816& cpu, Memory& mem, Word value)
{
    Byte hi = (value >> 8);
    Byte lo = (value & 0xff);

    pushByte(cpu, mem, hi);
    pushByte(cpu, mem, lo);
}

Word pullWord(Cpu65C816& cpu, Memory& mem)
{
    Word lo = pullByte(cpu, mem);
    Word hi = pullByte(cpu, mem);

    return hi << 8 | lo;
}

// 5.2
InstructionInfo absolute(Cpu65C816 const& cpu, Memory const& mem)
{
    Word b = readWord(mem, cpu.rk, cpu.pc+1);
    Address ea = bank(cpu.dbr) | b;

    return
    {
        ea,
        {}
    };
}

// 5.3
InstructionInfo absoluteX(Cpu65C816 const& cpu, Memory const& mem)
{
    Word b = readWord(mem, cpu.rk, cpu.pc+1);
    Address ea = (cpu.status_register & Cpu65C816::StatusFlag::X) ? (bank(cpu.dbr) | b) + cpu.rx.b
                                                                  : (bank(cpu.dbr) | b) + cpu.rx.w;

    return
    {
        ea,
        {}
    };
}

// 5.3
InstructionInfo absoluteY(Cpu65C816 const& cpu, Memory const& mem)
{
    Word b = readWord(mem, cpu.rk, cpu.pc+1);
    Address ea = (cpu.status_register & Cpu65C816::StatusFlag::X) ? (bank(cpu.dbr) | b) + cpu.ry.b
                                                                  : (bank(cpu.dbr) | b) + cpu.ry.w;

    return
    {
        ea,
        {}
    };
}

// 5.4
InstructionInfo absoluteIndirect(Cpu65C816 const& cpu, Memory const& mem)
{
    Word b = readWord(mem, cpu.rk, cpu.pc+1);

    return
    {
        b,
        {}
    };
}


// 5.5: Absolute X indirect
InstructionInfo absoluteIndirectX(Cpu65C816 const& cpu, Memory const& mem)
{
    Word b = readWord(mem, cpu.rk, cpu.pc+1);

    Word a = (cpu.status_register & Cpu65C816::StatusFlag::X) ? (b | cpu.rx.b)
                                                              : (b | cpu.rx.w);

    Word addr = readWord(mem, cpu.rk, a);
    Address ea = bank(cpu.rk) | a;

    return
    {
        ea,
        addr
    };
}

// 5.6 Accumulator
InstructionInfo accumulator(Cpu65C816 const& cpu, Memory const& mem)
{
    return
    {
        0,
        0
    };
}

// 5.7
InstructionInfo directPage(Cpu65C816 const& cpu, Memory const& mem)
{
    Byte b = readByte(mem, cpu.rk, cpu.pc+1);
    Word addr = cpu.rd.w + b;

    return
    {
        addr,
        addr
    };
}

// 5.8 X
InstructionInfo directPageX(Cpu65C816 const& cpu, Memory const& mem)
{
    Byte b = readByte(mem, cpu.rk, cpu.pc+1);
    Word addr = cpu.rd.w + b + (cpu.status_register & Cpu65C816::StatusFlag::X) ? cpu.rx.b
                                                                                : cpu.rx.w;

    return
    {
        addr,
        addr
    };
}

// 5.8 Y
InstructionInfo directPageY(Cpu65C816 const& cpu, Memory const& mem)
{
    Byte b = readByte(mem, cpu.rk, cpu.pc+1);
    Word addr = cpu.rd.w + b + (cpu.status_register & Cpu65C816::StatusFlag::X) ? cpu.ry.b
                                                                                : cpu.ry.w;

    return
    {
        addr,
        addr
    };
}

InstructionInfo immediateAdressing(Cpu65C816 const& cpu, Memory const& mem)
{
    Word i = cpu.pc + Word(1);
    return
    {
          i
        , i
    };
}


// Direct page indexed indirect. 5.11
InstructionInfo directPageIndirectX(Cpu65C816 const& cpu, Memory const& mem)
{

    Byte byte = readByte(mem, cpu.rk, cpu.pc+1);
    auto addr = readWord(mem, 0x00, Word(byte) + cpu.rx.w + cpu.rd.w);

    return
    {
        addr,
        addr
    };
}

std::map<AddressingMode, std::function<InstructionInfo(Cpu65C816 const&, Memory const&)>> addressing
{
    { AddressingMode::Immediate, &immediateAdressing}
   ,{ AddressingMode::Absolute,  &absolute}
   ,{ AddressingMode::AbsoluteX,  &absoluteX}
   ,{ AddressingMode::AbsoluteY,  &absoluteY}
   ,{ AddressingMode::AbsoluteIndirect,  &absoluteIndirect}
   ,{ AddressingMode::AbsoluteIndirectX,  &absoluteIndirectX}
   ,{ AddressingMode::Accumulator,  &accumulator}
   ,{ AddressingMode::DirectPage,  &directPage}
   ,{ AddressingMode::DirectPageX,  &directPageX}
   ,{ AddressingMode::DirectPageY,  &directPageY}
   ,{ AddressingMode::DirectPageIndirectX,  &directPageIndirectX}
};

void op_rep(Cpu65C816& cpu, Memory& mem, InstructionInfo const& info)
{
    auto const clear_flags = ~readByte(mem, info.effective_address);
    cpu.status_register = cpu.status_register & clear_flags;
}

void op_sep(Cpu65C816& cpu, Memory& mem, InstructionInfo const& info)
{
    auto const set_flags = readByte(mem, info.effective_address);
    cpu.status_register = (cpu.status_register | set_flags);
}

void op_lda(Cpu65C816& cpu, Memory& mem, InstructionInfo const& info)
{
    if (cpu.status_register & Cpu65C816::StatusFlag::M)
    {
        cpu.ra.b = readByte(mem, info.effective_address);
    }
    else
    {
        cpu.ra.w = readWord(mem, info.effective_address);
    }
}

void pushByte(Byte b, Cpu65C816& cpu, Memory& mem)
{
    mem.ram[cpu.sp.w] = b;
    --cpu.sp.w;
}

void pushWord(Word w, Cpu65C816& cpu, Memory& mem)
{
    pushByte(w & 0xff, cpu, mem);
    pushByte((w >> 8), cpu, mem);
}

void op_pha(Cpu65C816& cpu, Memory& mem, InstructionInfo const& info)
{
    pushWord(cpu.ra.w, cpu, mem);
}

std::map<Byte, Instruction> instruction_table
{
    { 0xC2, { "REP", AddressingMode::Immediate, 2, 3,           &op_rep}}
   ,{ 0xE2, { "SET", AddressingMode::Immediate, 2, 3,           &op_sep}}
   ,{ 0xA1, { "LDA", AddressingMode::DirectPageIndirectX, 2, 7, &op_lda}}
   ,{ 0xA5, { "LDA", AddressingMode::DirectPage, 2, 4,          &op_lda}}
};

Byte step(Cpu65C816& cpu, Memory& mem)
{
    // 1. Load current op code
    Byte const op_code = readByte(mem, cpu.rk, cpu.pc);

    if (op_code == 0xEA)
    {
        return 0;
    }

    // 2. Find instruction.
    auto const instruction_it = instruction_table.find(op_code);

    if (instruction_it == instruction_table.end())
    {
        return 2;
    }

    Instruction const& instruction = instruction_it->second;

    // 3. Perform memory addressing if any
    auto const addressing_it = addressing.find(instruction.mode);
    if (addressing_it == addressing.end())
    {
        return 2;
    }

    // 4. Perform op-code
    InstructionInfo const instruction_info = addressing_it->second(cpu, mem);
    instruction.func(cpu,mem,instruction_info);

    cpu.pc += instruction.size;

    return 1;
}


