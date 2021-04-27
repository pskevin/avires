#include <iostream>
#include <vector>

// #include <unistd.h>

// #include "cycles.h"

using std::cout;
using std::endl;
using std::vector;

int add(int x, int y)
{
    return x+y;
}

// int slept(int us)
// {
//     usleep(us);
//     return 0;
// }

int main(int argc, char **argv)
{
    // vector<uint64_t> cycles_taken; 
    
    // auto cycled = cycles(add, &cycles_taken);
    // auto cycled = cycles(slept, &cycles_taken);

    for(int i=0; i<100; i++)
    {
        add(i, i+1);
        // cycled(100); 
    }

    // cout << "\nChecking cycles added -" << endl;
    // for (int i = 0; i < cycles_taken.size(); i++)
    // {
    //     cout << i << ": " << cycles_taken[i] << endl;
    // }
    
    return 0;
}


// CPU MHz:             1200.532    =   1200532000 cycles/second
// CPU max MHz:         1700.0000   =   1700000000 cycles/second
// CPU min MHz:         1200.0000   =   1200000000 cycles/second