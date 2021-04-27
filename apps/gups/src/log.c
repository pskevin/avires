#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


void LogMetadata(int line, const char *fileName)
{
    fprintf(stderr, "%s[%d] ", fileName, line);
}

void LogVariadic(int line, const char *fileName, const char *formattedMessage,...)
{
    LogMetadata(line, fileName);
    
    va_list args;
    va_start(args, formattedMessage);
    vfprintf(stderr, formattedMessage, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}


void Log(int line, const char *fileName)
{
    LogMetadata(line, fileName);
    fprintf(stderr, "\n");
    fflush(stderr);
}
