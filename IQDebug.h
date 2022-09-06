#ifndef _IQ_DEBUG_H
#define _IQ_DEBUG_H

#include "Platform.h"

void IQDebugInit(const char *filename, bool verbose, bool detailed);
void IQDebugVerbose(const char *function, size_t line, const char *format, ...);
void IQDebugDetailed(const char *function, size_t line, const char *format, ...);

#define LOGV(format, ...) IQDebugVerbose(__FUNCTION__, __LINE__, format, __VA_ARGS__)
#define LOGD(format, ...) IQDebugDetailed(__FUNCTION__, __LINE__, format, __VA_ARGS__)

#endif /* _IQ_DEBUG_H */