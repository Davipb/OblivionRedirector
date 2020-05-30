#pragma once
#include <wchar.h>
#include <stdint.h>

void OR_StartLogging();
void OR_StopLogging();
void OR_Log(uint8_t level, const wchar_t* message, ...);

enum
{
	OR_LOG_LEVEL_TRACE = 0,
	OR_LOG_LEVEL_DEBUG,
	OR_LOG_LEVEL_INFO,
	OR_LOG_LEVEL_WARN,
	OR_LOG_LEVEL_ERROR,
	OR_LOG_LEVEL_OFF
};

// Only enable tracing for the debug version
// This allows us to trace every redirect call when debugging, then removes the overhead of
// a function call on every file operation for the final release
#ifdef DEBUG
#define OR_TRACE(message, ...) OR_Log(OR_LOG_LEVEL_TRACE, L##message, __VA_ARGS__)
#else
#define OR_TRACE(message, ...) ((void)0)
#endif

#define OR_DEBUG(message, ...) OR_Log(OR_LOG_LEVEL_DEBUG, L##message, __VA_ARGS__)
#define OR_INFO(message, ...) OR_Log(OR_LOG_LEVEL_INFO, L##message, __VA_ARGS__)
#define OR_WARN(message, ...) OR_Log(OR_LOG_LEVEL_WARN, L##message, __VA_ARGS__)
#define OR_ERROR(message, ...) OR_Log(OR_LOG_LEVEL_ERROR, L##message, __VA_ARGS__)
