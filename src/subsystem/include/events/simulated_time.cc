#include "./simulated_time.h"
#include "../utils/macros.h"

#include <string>

using std::string;

namespace Event
{
    string SimulatedTime::ID = "simulated_time";

    void SimulatedTime::Observe(string source, uint64_t time)
    {
        if (!dummy_)
        {
            Type::Observe(source, time);
        }
    }

}