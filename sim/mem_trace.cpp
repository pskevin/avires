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

#include "mmgr_simple.h"
#include "mmgr_linux.h"

// std::unordered_map<uint64_t, int> hashmap;

using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;

KNOB<string> KnobOutputPrefix(KNOB_MODE_WRITEONCE, "pintool",
    "o", "experiments/sim_", "specify stats output directory");
KNOB<uint32_t> KnobMemoryManager(KNOB_MODE_WRITEONCE, "pintool",
   "mm","0", "Defines which memory manager to use. 0=simple, 1=linux");

MemorySimulator* sim;

volatile uint64_t timestep = 0;

// Print a memory read record
VOID RecordMemRead(uint64_t addr, uint32_t size)
{   
    sim->memaccess(addr, TYPE_READ, size, timestep);
    timestep++;
}

// Print a memory write record
VOID RecordMemWrite(uint64_t addr, uint32_t size)
{
    sim->memaccess(addr, TYPE_WRITE, size, timestep);
    timestep++;
}

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
                IARG_END);
        }
    }
}

VOID PrepareForFini(VOID *v) {
    if (KnobMemoryManager == 1) {
        ((LinuxMemoryManager*) sim->GetMemoryManager())->shutdown();
    }
}


VOID Fini(INT32 code, VOID *v)
{
    std::cout << "Total runtime: " << (double)sim->runtime / 1e9 << std::endl;
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
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage(); 

    MemoryManager* mgr;

    switch (KnobMemoryManager)
    {
        case 0:
            std::cout << "Running with Simple Memory Manager" << std::endl;
            mgr = new SimpleMemoryManager();
            break;
        case 1:
            std::cout << "Running with Linux Memory Manager" << std::endl;
            mgr = new LinuxMemoryManager();
            break;

    default:
        return 1;
        break;
    }

    // printf("INITIALIZING CACHE\n");
    L1DataCache* l1d = new L1DataCache();
    // printf("INITIALIZING TLB\n");
    FourLevelTLB* tlb = new FourLevelTLB();
    // printf("INITIALIZING MEMSIM\n");

    sim = new MemorySimulator(mgr, tlb, l1d);
    
    mgr->init(sim);

    PIN_AddPrepareForFiniFunction(PrepareForFini, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);
    
    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
