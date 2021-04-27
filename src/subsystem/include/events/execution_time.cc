#include "./execution_time.h"
#include  "../utils/macros.h"

#include "../utils/tsc.h"

#include <string>

using std::string;

namespace Event
{
    string ExecutionTime::ID = "execution_time";

    void ExecutionTime::ObserveStart(string source)
    {
        if(!this->dummy_)
        {
            start=read_tsc();
        }
    }

    void ExecutionTime::ObserveEnd(string source)
    {
        if(!this->dummy_)
        {
            end=read_tsc()-start;
            Type::Observe(source, end);
        }
    }

}