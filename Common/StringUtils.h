#pragma once
#include <wchar.h>
#include <stdbool.h>

// Gets a locale that can be used to perform invariant string operations.
_locale_t OR_GetInvariantLocale(void);

// Transforms a UTF16 string to a UTF8 string.
// The new string is allocated dynamically must be freed
char* OR_Utf16ToUtf8(const wchar_t* utf16);

// Transforms a UTF16 string to a Windows Codepage string.
// The new string is allocated dynamically must be freed
char* OR_Utf16ToCodepage(const wchar_t* utf16);

// Gets the directory path of a file.
// The returned string is allocated dynamically and must be freed.
wchar_t* OR_GetDirectory(wchar_t* path);

// Gets the file name from a wide file path.
// The returned string points to the same buffer as `path`, and doesn't need to be freed.
const wchar_t* OR_GetFileNameW(const wchar_t* path);

// Gets the file name from a narrow file path.
// The returned string points to the same buffer as `path`, and doesn't need to be freed.
const char* OR_GetFileNameA(const char* path);

// Transforms a wide string to all-upercase, using an invariant locale.
// The returned string is allocated dynamically and must be freed
const wchar_t* OR_ToUpperW(const wchar_t* input);

// Transforms a narrow string to all-upercase, using an invariant locale.
// The returned string is allocated dynamically and must be freed
const char* OR_ToUpperA(const char* input);

// Checks if two wide strings are equal, ignoring case and using an invariant locale.
bool OR_AreCaseInsensitiveEqualW(const wchar_t* first, const wchar_t* second);

// Checks if two narrow strings are equal, ignoring case and using an invariant locale.
bool OR_AreCaseInsensitiveEqualA(const char* first, const char* second);

// Concatenates multiple strings. All arguments must be of type const wchar_t*.
// The returned string is allocated dynamically and must be freed.
wchar_t* OR_Concat(size_t count, ...);

// Checks if a wide string ends with another wide string.
// full: The full string
// component: The string that should be at the end of the full string
bool OR_EndsWithW(const wchar_t* full, const wchar_t* component);

// Checks if a narrow string ends with another narrow string.
// full: The full string
// component: The string that should be at the end of the full string
bool OR_EndsWithA(const char* full, const char* component);