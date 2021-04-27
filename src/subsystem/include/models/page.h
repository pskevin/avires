#ifndef MEMSIM_MODELS_PAGE_H
#define MEMSIM_MODELS_PAGE_H

#include <cstdint>
#include <cassert>
#include <map>

#include "../utils/macros.h"

#include "address.h"
#include "page_table_entry.h"


using std::map;

class Page
{
public:

    enum class Type
    {
        Giga = 0,
        Huge,
        Base,
        Count
    };

    static map<Type, uint64_t> Size;
    static map<Type, uint64_t> OffsetMask;
    static map<Type, uint64_t> FrameNumberMask;

    Address* addr;
    PageTableEntry* pte;

    Page(){}

    static uint64_t FrameNumber (uint64_t addr, Type type)
    {
        return addr & FrameNumberMask[type];
    }

    static Type TypeAt(int i)
    {
        return Type(i);
    }

private:
    
    static map<Type, uint64_t> PopulateSize()
    {
        map<Type, uint64_t> m;
        m[Type::Giga] = GB(1);
        m[Type::Huge] = MB(2);
        m[Type::Base] = KB(4);
        return m;
    }

    static map<Type, uint64_t> PopulateOffsetMask()
    {
        map<Type, uint64_t> m;
        m[Type::Giga] = GB(1)-1;
        m[Type::Huge] = MB(2)-1;
        m[Type::Base] = KB(4)-1;
        return m;
    }


    static map<Type, uint64_t> PopulateFrameNumberMask()
    {
        map<Type, uint64_t> m;
        m[Type::Giga] = (GB(1)-1)^UINT64_MAX;
        m[Type::Huge] = (MB(2)-1)^UINT64_MAX;
        m[Type::Base] = (KB(4)-1)^UINT64_MAX;
        return m;
    }
};







// // How to iterate over an enum
// Page::Type type;
// for(int index = 0; type != Page::Type::Count; type = Page::TypeAt(index))
// {
//     index++;
// }


#endif