#include <iostream>
#include <vector>
#include <map>

#include "Memory.h"
#include "CPU56C816.h"
#include <fmt/format.h>

void printCpu(Cpu65C816 const& cpu)
{
    std::cout << fmt::format("ra: {:x}", cpu.ra.w) << '\n';
    std::cout << fmt::format("rx: {:x}", cpu.rx.w) << '\n';
    std::cout << fmt::format("ry: {:x}", cpu.ry.w) << '\n';
    std::cout << fmt::format("sp: {:x}", cpu.sp.w) << '\n';
    std::cout << fmt::format("rd: {:x}", cpu.rd.w) << '\n';
    std::cout << fmt::format("pc: {:x}", cpu.pc) << '\n';
    std::cout << fmt::format("dbr: {:x}", cpu.dbr) << '\n';
    std::cout << fmt::format("rk: {:x}", cpu.rk) << '\n';

    std::cout << fmt::format("status register: {:x}", cpu.status_register) << '\n';
    std::cout << "\n\n";
}

int main()
{
    std::cout << "Initializing memory\n";
    Memory mem {};
    mem.ram.resize(512*1024);

    MemoryData mem_data = {
        // Address       Data
        {0x000010,       {0x12,0x23,0x45}},
        {0x001100,       { 0xA5, 0x10,       // LDA $10
                           0xEA}}            // EA
    };
    
    std::cout << "Loading program\n";
    loadMemory(mem_data, mem);


    std::cout << "Initializing CPU\n";
    Cpu65C816 cpu{};
    cpu.pc = 0x1100;
    cpu.sp.w = 0xfff0;

    std::cout << "CPU:\n";
    printCpu(cpu);

    std::cout << "Running program\n";
    Byte running = 1;
    while (running == 1)
    {
        running = step(cpu,mem);
    }
    std::cout << "Program finished\n";

    std::cout << "CPU:\n";
    printCpu(cpu);
}
