#pragma once

#include "Memory.h"

struct Cpu65C816
{
    union Reg
    {
        Byte b;
        Word w;
    };

    // 16-bit registers;
    Reg ra{};
    Reg rx{};
    Reg ry{};
    Reg sp{};

    // Direct page register (D = direct, DL = direct low, DH = direct high)
    Reg rd{};

    // Program counter;
    Word pc{};

    // Databank (DBR) and Program bank (RK)
    Byte dbr, rk{};

    enum StatusFlag : uint8_t 
    {
         N = 1 << 0
       , V = 1 << 1
       , M = 1 << 2
       , X = 1 << 3
       , D = 1 << 4
       , I = 1 << 5
       , Z = 1 << 6
       , C = 1 << 7
    };

    // Status flags: N,V,m,x,D,I,Z,C
    Byte status_register{};

    Byte emulation_mode_flag{};
    Byte break_flag{};
};

Byte step(Cpu65C816& cpu, Memory& mem);
