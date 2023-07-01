#ifndef MACROS_H_INCLUDED
#define MACROS_H_INCLUDED

#include <string.h>

/// <summary>Determines the length of a static array.</summary>
#define ARRAYLENGTH(array) (sizeof(array) / sizeof((array)[0]))

/// <summary>Determines the length of a string literal.</summary>
#define STRLEN(s) (sizeof(s) / sizeof(char) - 1)

/// <summary>Foreach loop construct.</summary>
#define foreach(type, varName, array, count) for ( \
    type *varName = array;                         \
    varName != array + count;                      \
    ++varName)

/// <summary>Checks if i is a valid index of an array of length length.</summary>
#define IN_ARRAY_BOUNDS(i, length) (0 <= (i) && (i) < (length))

/// <summary>Determines whether 2 strings are equal using strcmp.</summary>
#define streq(str1, str2) (strcmp((str1), (str2)) == 0)

/// <summary>Determines whether 2 strings are equal using strncmp.</summary>
#define streqn(str1, str2, n) (strncmp((str1), (str2), (n)) == 0)

#endif // MACROS_H_INCLUDED