#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "macros.h"

typedef struct {
    bool const namePresent;
    bool const argumentPresent;
} CommandParseResult;

Command const *parseCommand(Command const commands[], size_t commandCount, char const *commandName, size_t maxLength);
void splitCommandNameAndArgument(char *input, char const **commandName, long long **argument);
size_t getMaxNameLength(Command const commands[], size_t commandCount);
unsigned digitCount(long long n);
char *allocateString(size_t length);

Command const *inputCommand(Command const commands[], size_t commandCount, long long *argument)
{
    size_t const maxNameLength = getMaxNameLength(commands, commandCount);
    size_t const inputLength = maxNameLength + digitCount(LLONG_MAX);

    char *const input = allocateString(inputLength);

    while (true)
    {
        printf("\n> ");

        if (gets_s(input, inputLength) == NULL)
        {
            printf("Invalid input: get_s failed");
        }

        char *commandName = NULL;

        Command const *command = NULL;

        long long arg;
        long long *parsedArgument = &arg;

        splitCommandNameAndArgument(input, &commandName, &parsedArgument);

        if (commandName == NULL)
        {
            continue;
        }
        else if ((command = parseCommand(commands, commandCount, commandName, maxNameLength)) == NULL)
        {
            printf("Unknown command");
        }
        else if (command->hasArgument && parsedArgument == NULL)
        {
            printf("Argument missing");
        }
        else if (!command->hasArgument && parsedArgument != NULL)
        {
            printf("No argument was expected");
        }
        else
        {
            if (command->hasArgument)
            {
                *argument = *parsedArgument;
            }

            customFree(input);
            return command;
        }
    }
}

void showCommandMenu(Command const commands[], size_t commandCount)
{
    foreach(Command const, command, commands, commandCount)
    {
        printf("%*s %s\n", -(int)getMaxNameLength(commands, commandCount), command->name, command->description);
    }
}

Command const *parseCommand(Command const commands[], size_t commandCount, char const *commandName, size_t maxLength)
{
    foreach(Command const, command, commands, commandCount)
    {
        if (streqn(command->name, commandName, maxLength))
        {
            return command;
        }
    }
    return NULL;
}

void splitCommandNameAndArgument(char *input, char const **commandName, long long **argument)
{
    // If available, set value, otherwise return NULL.

    char *context = NULL;
    char const *const separators = " \f\n\r\t\v";

    // Get first token (command name)
    char *token = strtok_s(input, separators, &context);
    if (token == NULL) // input is empty
    {
        *commandName = NULL;
        *argument = NULL;
        return;
    }

    // Store command name
    *commandName = token;

    // Get next token (integer)
    token = strtok_s(NULL, separators, &context);
    if (token == NULL)
    {
        // argument is optional
        *argument = NULL;
        return;
    }

    // Convert argument
    char *endptr;
    errno = 0;
    unsigned long long arg = strtoull(token, &endptr, 0);
    if (errno == ERANGE) // entered number out of range
    {
        printf("Argument must be in [0 ; %lld]", LLONG_MAX);
        *argument = NULL;
    }
    else if (*endptr != '\0') // conversion failed
    {
        printf("Argument is not a number");
        *argument = NULL;
    }
    else
    {
        // Store argument
        **argument = arg;
    }
}

size_t getMaxNameLength(Command const commands[], size_t commandCount)
{
    size_t maxNameLength = 0;

    foreach(Command const, command, commands, commandCount)
    {
        size_t nameLength = strlen(command->name);
        if (nameLength > maxNameLength)
        {
            maxNameLength = nameLength;
        }
    }

    return maxNameLength;
}

char *allocateString(size_t length)
{
    size_t size = length * sizeof(char) + 1;
    void *const ptr = customAlloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "Allocating %zu bytes failed.", size);
        abort();
    }
    return ptr;
}

unsigned digitCount(long long n)
{
    return n == 0 ? 1 : (unsigned)log10((double)llabs(n)) + 1;
}
