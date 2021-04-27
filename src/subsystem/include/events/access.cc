#include "./access.h"
#include "../utils/macros.h"

#include <string>

using std::string;

namespace Event
{
    string Access::ID = "access";

    void Access::Observe(string source, Attempt obs)
    {
        if (!dummy_)
        {
            if (obs == Access::Hit)
            {
                Type::Observe(source, 1);
            }
            else
            {
                Type::Observe(source, 0);
            }
        }
    }
}