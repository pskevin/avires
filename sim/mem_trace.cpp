/*
 * Copyright 2002-2020 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

#include <iostream>

#include "pin.H"
// #include <cassert>
#include <unordered_map>

#include "memsim_new.h"
#include "four_level_tlb.h"


#define MEMORY_MANAGER 0

#if MEMORY_MANAGER == 0
    #include "mmgr_simple.h"
#elif MEMORY_MANAGER == 1
    #include "mmgr_linux.h"
#endif

// std::unordered_map<uint64_t, int> hashmap;

using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;

MemorySimulator* sim;

// Print a memory read record
VOID RecordMemRead(VOID * ip, uint64_t addr)
{
    sim->memaccess(addr, TYPE_READ);
    // hashmap[addr]++;
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, uint64_t addr)
{
    sim->memaccess(addr, TYPE_WRITE);
    // hashmap[addr]++;
}

// std::string ToString(uint64_t val)
// {
//     std::stringstream stream;
//     stream << val;
//     return stream.str();
// }

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
    }
}
VOID Fini(INT32 code, VOID *v)
{
    // fprintf(0, "#eof\n");
    // string output;
    // std::tr1::unordered_map<uint64_t, int>::iterator kv;
    // for ( kv = hashmap.begin(); kv != hashmap.end(); kv++ )
    // {
    //     output.append(ToString(kv->first) + ":" + ToString(kv->second) + "\n");
    // }
    // FILE* file = fopen("addr.hist", "w");
    // fprintf(file, "%s", output.c_str());
    // fclose(file);
    // std::cout << "\n\nhashmap.size() is " << hashmap.size() << std::endl;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{  
    #if MEMORY_MANAGER == 0
        SimpleMemoryManager* mgr = new SimpleMemoryManager();
    #elif MEMORY_MANAGER == 1
        LinuxMemoryManager* mgr = new LinuxMemoryManager();
    #else
        exit(1);
    #endif
    FourLevelTLB* tlb = new FourLevelTLB();
    sim = new MemorySimulator(mgr, tlb);

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    
    mgr->init(sim);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
