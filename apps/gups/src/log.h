#ifndef LOG_H
#define LOG_H

#if (LOGGING == 1)

void LogVariadic(int line, const char *fileName, const char *formattedMessage,...);
void Log(int line, const char *fileName);

#define LogMessage(msg, ...) LogVariadic(__LINE__, __FILE__, msg, ##__VA_ARGS__);

#define LogPoint() Log(__LINE__, __FILE__);

#else

#define LogMessage(msg, ...) \
    while(0) {}

#define LogPoint() \
    while(0) {}
#endif

#endif