#include "macros.h"

#include <cstdarg>
#include <iostream>

// #include <string>
// using std::string;

using std::cerr;
using std::endl;


void LogMetadata(int line, const char *fileName, const char *functionName)
{
    char buffer[256] = { 0 };
    sprintf(buffer, "%s@%s[%d] ", functionName, fileName, line);
    cerr << buffer;
}

void MemsimLog(int line, const char *fileName, const char *functionName, const char *formattedMessage, ...)
{
    LogMetadata(line, fileName, functionName);
    
    va_list args;
    char buffer[256] = {0};
    va_start(args, formattedMessage);
    vsprintf(buffer, formattedMessage, args);
    va_end(args);
    cerr << buffer << endl;
}


void MemsimLog(int line, const char *fileName, const char *functionName)
{
    LogMetadata(line, fileName, functionName);
    cerr << endl;
}
