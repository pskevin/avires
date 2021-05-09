#ifndef MEMSIM_CONTROLLERS_MEMORY_H
#define MEMSIM_CONTROLLERS_MEMORY_H

#include "../models/address.h"
#include "../models/page.h"
#include "../models/page_table_entry.h"

#include <cstdint>

namespace Memory
{
    enum class Type
    {
        Fast = 0,
        Slow,
        Count
    };

    enum class Simulate
    {
        Read,
        Write,
        Move,
        Count
    };

    static map<Simulate, map<Type, uint64_t>> SimulatedTimes()
    {
        map<Simulate, map<Type, uint64_t>> time_to;

        map<Type, uint64_t> read, write, move;

        read[Type::Fast] = 81;
        write[Type::Fast] = 86;
        move[Type::Fast] = 1000;

        read[Type::Slow] = 169;
        write[Type::Slow] = 90;
        move[Type::Slow] = 2000;

        time_to[Simulate::Read] = read;
        time_to[Simulate::Write] = write;
        time_to[Simulate::Move] = move;

        return time_to;
    }

    static map<Type, uint64_t> SimulatedSizes()
    {
        map<Type, uint64_t> size;

        size[Type::Fast] = MB(128);
        size[Type::Slow] = GB(100);

        return size;
    }

    static map<Simulate, map<Type, uint64_t>> TimeTo = SimulatedTimes();
    static map<Type, uint64_t> Size = SimulatedSizes();

    // Fake offset for slowmem in physical memory
    const static uint64_t SlowBit = ((uint64_t)1 << 63);
    const static uint64_t SlowMask = (((uint64_t)1 << 63) - 1);

    class Controller
    {
    public:
        virtual void PageFault(Address *addr, bool readonly);
        virtual PageTableEntry *GetCR3();

    private:
        virtual PageTableEntry *AllocatePageTables(Address *addr, Page::Type pt);
        virtual uint64_t GetPhysicalPage(Address *addr, PageTableEntry *pte);
    };
}
#endif