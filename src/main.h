#ifndef MEMSIM_MAIN_H
#define MEMSIM_MAIN_H

#include "pin.H"

#include "./subsystem/include/models/address.h"
#include "./subsystem/include/utils/macros.h"
#include "./subsystem/include/models/event.h"
#include "./subsystem/include/events/execution_time.h"

#include "./subsystem/simulator.h"
#include "./subsystem/tlb/two_level.h"
#include "./subsystem/cache/l1d.h"
#include "./subsystem/memory/fcfs.h"
#include "./subsystem/memory/linux.h"

#include <cstdint>
#include <string>
#include <map>
#include <set>

using std::map;
using std::set;
using std::string;

KNOB<string> KnobOutputFile(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "o",
    "",
    "specify stats output file");

KNOB<int> KnobMemoryController(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "memory-controller",
    "0",
    "specify stats output file");

KNOB<uint64_t> KnobSkipEveryNth(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "skip-every-nth",
    "1",
    "specify stats output file");

KNOB<uint64_t> KnobTakeEveryNth(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "take-every-nth",
    "1",
    "specify stats output file");

KNOB<string> KnobObserveEvents(
    KNOB_MODE_APPEND,
    "pintool",
    "observe-event",
    "",
    "specify stats output file");

KNOB<string> KnobFromSources(
    KNOB_MODE_APPEND,
    "pintool",
    "from-source",
    "",
    "specify stats output file");

KNOB<string> KnobObserveEventFromSource(
    KNOB_MODE_APPEND,
    "pintool",
    "observe-event-from-source",
    "",
    "specify stats output file");


KNOB<string> KnobIgnoreEventFromSource(
    KNOB_MODE_APPEND,
    "pintool",
    "ignore-event-from-source",
    "",
    "specify stats output file");

vector<string> TwoTokens(string str, string delimiter)
{
    vector<string> tokens;

    size_t pos = 0;
    string token;
    while ((pos = str.find(delimiter)) != string::npos)
    {
        token = str.substr(0, pos);
        tokens.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    tokens.push_back(str);

    assert(tokens.size() == 2);
    return tokens;
}

map<string, set<string>> FromEventSourcesMap()
{
    map<string, set<string>> events_sources_map;

    set<string> sources;
    for (uint32_t i = 0; i < KnobFromSources.NumberOfValues(); i++)
    {
        sources.insert(KnobFromSources.Value(i));
    }

    for (uint32_t i = 0; i < KnobObserveEvents.NumberOfValues(); i++)
    {
        events_sources_map[KnobObserveEvents.Value(i)] = sources;
    }


    for (uint32_t i = 0; i < KnobObserveEventFromSource.NumberOfValues(); i++)
    {
        vector<string> event_source = TwoTokens(KnobObserveEventFromSource.Value(i), ",");
        LogMessage("Add %s | %s", event_source[0], event_source[1]);
        events_sources_map[event_source[0]].insert(event_source[1]);
    }

    for (uint32_t i = 0; i < KnobIgnoreEventFromSource.NumberOfValues(); i++)
    {
        vector<string> event_source = TwoTokens(KnobIgnoreEventFromSource.Value(i), ",");
        LogMessage("Remove %s | %s", event_source[0], event_source[1]);
        events_sources_map[event_source[0]].erase(event_source[1]);
    }

    return events_sources_map;
}

Event::ExecutionTime *execution_time;

#endif