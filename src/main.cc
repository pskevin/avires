// This is the main pin tool which runs a given application instrumented with Memsim
#include "main.h"

#include "./subsystem/include/utils/tsc.h"

#include <iostream>

using std::cout;
using std::endl;

MemorySimulator *memsim;

uint64_t accesses_observed = 0;
uint64_t accesses_attempted = 0;

uint64_t start = 0, end = 0;

// Interpose Memory Reads
VOID ObserveReadAccess(uint64_t vaddr, uint32_t size)
{
    accesses_attempted++;
    if ((KnobSkipEveryNth > 1) && accesses_attempted % KnobSkipEveryNth == 0)
        return;
    if ((KnobTakeEveryNth > 1) && accesses_attempted % KnobTakeEveryNth != 0)
        return;

    accesses_observed++;
    memsim->Access(memsim->NewAddress(vaddr, READ_MEM), size);
}

// Interpose Memory Writes
VOID ObserveWriteAccess(uint64_t vaddr, uint32_t size)
{
    accesses_attempted++;
    if ((KnobSkipEveryNth > 1) && accesses_attempted % KnobSkipEveryNth == 0)
        return;
    if ((KnobTakeEveryNth > 1) && accesses_attempted % KnobTakeEveryNth != 0)
        return;

    accesses_observed++;
    memsim->Access(memsim->NewAddress(vaddr, WRITE_MEM), size);
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
            const uint64_t size = INS_MemoryOperandSize(ins, memOp);
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)ObserveReadAccess,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, size,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            const uint64_t size = INS_MemoryOperandSize(ins, memOp);

            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)ObserveWriteAccess,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, size,
                IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID *v)
{
    end = read_tsc() - start;
    cout << "End-to-End exection time - " << end << endl;
    if (!KnobOutputFile.Value().empty())
    {
        cout << "Writing observed values." << endl;
        start = read_tsc();
        LogMessage("Total Accesses Observed %lld", accesses_observed);
        LogMessage("Total Accesses Attempted %lld", accesses_attempted);
        LogMessage("Fraction of accesses skipped %f", 1 - (accesses_observed * 1.0 / accesses_attempted));
        Event::GetPool()->WriteAll(KnobOutputFile.Value());
        end = read_tsc() - start;
        cout << "Time taken to write " << end << endl;
    };
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
INT32 Usage()
{
    cout << "This tool represents a memoroy simulator.\n"
            "\n";

    cout << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv))
        return Usage();

    start = read_tsc();

    Event::ConfigurePool(FromEventSourcesMap());
    LogPoint();
    TLB::TwoLevel *tlb = new TLB::TwoLevel();
    LogPoint();
    Cache::L1D *cache = new Cache::L1D();
    LogPoint();
    Memory::FirstComeFirstServe *mem = new Memory::FirstComeFirstServe(Page::Type::Base);
    LogPoint();
    memsim = new MemorySimulator(mem, tlb, cache);
    LogPoint();

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
