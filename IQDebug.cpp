#include "IQDebug.h"

#define BUFFER_SIZE     (2048)

static FILE *debugFile = NULL;
static bool verbose = false;
static bool detailed = false;
static char *buffer;

void IQDebugInit(const char *filename, bool verbose, bool detailed)
{
    buffer = (char*)malloc(BUFFER_SIZE);
    if (buffer)
    {
        debugFile = _fsopen(filename, "wt", _SH_DENYWR);
        ::verbose = verbose;
        ::detailed = detailed;
    }
}

static void IQDebugGeneric(const char *function, size_t line, const char *format, va_list va)
{
    int offset = 0;

    offset = sprintf_s(buffer, BUFFER_SIZE - offset, "[%30s:%6zu] ", function, line);
    offset += vsprintf_s(&buffer[offset], BUFFER_SIZE - offset, format, va);
    offset += sprintf_s(&buffer[offset], BUFFER_SIZE - offset, "\n");

    if (debugFile)
    {
        fwrite(buffer, offset, 1, debugFile);
        fflush(debugFile);
    }
}

void IQDebugVerbose(const char *function, size_t line, const char *format, ...)
{
    if (verbose)
    {
        va_list va;
        va_start(va, format);
        IQDebugGeneric(function, line, format, va);
        va_end(va);
    }
}

void IQDebugDetailed(const char *function, size_t line, const char *format, ...)
{
    if (detailed)
    {
        va_list va;
        va_start(va, format);
        IQDebugGeneric(function, line, format, va);
        va_end(va);
    }
}