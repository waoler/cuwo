void save_chunk_now();

#include "main.h"
#include "gensrc/stubs.h"
#include "gensrc/out.cpp"
#include "imports.h"
#include <iostream>
#include <fstream>
#include <string>
#include <boost/lexical_cast.hpp>

// inline includes
#include "cpu.cpp"
#include "memory.cpp"

CPU cpu;
Memory mem;

// VERY specific address - this is the start of the heap, where the game
// manager has been allocated
#define MANAGER_ADDRESS STACK_END

template <class T>
inline std::string to_str(const T & in)
{
    return boost::lexical_cast<std::string>(in);
}

template <class T>
inline int to_int(const T & in)
{
    return boost::lexical_cast<int>(in);
}

void save_chunk(uint32_t addr, const char * file)
{
    std::cout << "Save chunk: " << addr << std::endl;
    std::ofstream fp(file, std::ios::out | std::ios::binary);
    uint32_t entry = mem.read_dword(addr + 168);
    for (int i = 0; i < 256*256; i++) {
        uint32_t vtable = mem.read_dword(entry);
        float something = to_ss(mem.read_dword(entry+4));
        float something2 = to_ss(mem.read_dword(entry+8));
        float something3 = to_ss(mem.read_dword(entry+12));
        uint32_t something4 = mem.read_dword(entry+16);
        uint32_t something5 = mem.read_dword(entry+20);
        uint32_t chunk_data = mem.read_dword(entry+24);
        uint32_t data_size = mem.read_dword(entry+28); // * 4, size
        char * out_data = new char[data_size*4];
        mem.read(chunk_data, out_data, data_size*4);

        // write it out
        fp.write((char*)&something, 4);
        fp.write((char*)&something2, 4);
        fp.write((char*)&something3, 4);
        fp.write((char*)&something4, 4);
        fp.write((char*)&something5, 4);
        fp.write((char*)&data_size, 4);
        fp.write(out_data, data_size*4);
        delete[] out_data;
        entry += 32;
    }
    fp.close();
}

#define GEN_X_BASE (0x8020)
#define GEN_Y_BASE (0x8020)
#define GEN_SQ 11

inline void generate_chunk(uint32_t x, uint32_t y, const char * file)
{
    cpu.reset_stack();
    cpu.push_dword(x);
    cpu.push_dword(y);
    get_self() = MANAGER_ADDRESS;
    add_ret();
    generator_func();

    // save_chunk_now();
    // return 0;
    // address 405E30 is
    // void * __thiscall get_sector_chunk_data(__int64 chunk_pos)
    cpu.push_dword(x);
    cpu.push_dword(y);
    get_self() = MANAGER_ADDRESS;
    add_ret();
    sub_405E30();
    save_chunk(cpu.reg[EAX], file);
}

static uint32_t global_seed = 626466;

uint32_t get_seed()
{
    return global_seed;
}

int main(int argc, const char ** argv)
{
    if (argc != 5) {
        std::cout << "Usage: gen.exe seed x y file" << std::endl;
        return 0;
    }

    global_seed = (uint32_t)to_int(argv[1]);
    int x = to_int(argv[2]);
    int y = to_int(argv[3]);
    const char * file = argv[4];

    init_function_map();
    init_emu();
    init_static();
    entry_point();
    std::cout << "Generator setup!" << std::endl;

    generate_chunk(x, y, file);
    return 0;
}