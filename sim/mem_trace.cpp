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
#include "l1_data_cache.h"


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

KNOB<string> KnobOutputPrefix(KNOB_MODE_WRITEONCE,    "pintool",
    "o", "experiments/sim_", "specify stats output directory");
KNOB<uint64_t> KnobThresholdHit(KNOB_MODE_WRITEONCE , "pintool",
   "rh", "0", "only report memops with hit count above threshold");
KNOB<uint64_t> KnobThresholdMiss(KNOB_MODE_WRITEONCE, "pintool",
   "rm","0", "only report memops with miss count above threshold");
KNOB<uint64_t> KnobThresholdPagefault(KNOB_MODE_WRITEONCE, "pintool",
   "rf","0", "only report memops with pagefault count above threshold");
// TODO more knobs

MemorySimulator* sim;

// Print a memory read record
VOID RecordMemRead(uint64_t addr, uint32_t size, ADDRINT insaddr)
{   
    sim->memaccess(addr, TYPE_READ, size, insaddr);
}

// Print a memory write record
VOID RecordMemWrite(uint64_t addr, uint32_t size, ADDRINT insaddr)
{
    sim->memaccess(addr, TYPE_WRITE, size, insaddr);
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
            //  const ADDRINT iaddr = INS_Address(ins);
            // const uint64_t instId = profile.Map(iaddr);

            const uint64_t size = INS_MemoryReadSize(ins);
            // const BOOL single = (size <= 4);
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                // IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, size,
                IARG_ADDRINT, INS_Address(ins),
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            const uint64_t size = INS_MemoryWriteSize(ins);

            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                // IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, size,
                IARG_ADDRINT, INS_Address(ins),
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
    sim->PrintAggregateProfiles();
    sim->WriteStatsFiles(KnobOutputPrefix);

}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
INT32 Usage()
{
    cerr <<
        "This tool represents a memoroy simulator.\n"
        "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl; 
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

    // printf("INITIALIZING CACHE\n");
    L1DataCache* l1d = new L1DataCache();
    // printf("INITIALIZING TLB\n");
    FourLevelTLB* tlb = new FourLevelTLB();
    // printf("INITIALIZING MEMSIM\n");

    COUNTER_HIT_MISS hm_threshold;

    hm_threshold[COUNTER_HIT] = KnobThresholdHit.Value();
    hm_threshold[COUNTER_MISS] = KnobThresholdMiss.Value();

    COUNTER_PAGEFAULTS pf_threshold;

    pf_threshold[COUNTER_PAGEFAULT] = KnobThresholdHit.Value();

    sim = new MemorySimulator(mgr, tlb, l1d, hm_threshold, pf_threshold);
    
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
