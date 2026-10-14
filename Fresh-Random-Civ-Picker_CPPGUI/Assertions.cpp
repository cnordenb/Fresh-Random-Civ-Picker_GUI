#include "Assertions.h"

#include <cstdio>

#include "framework.h"

bool ReportAssertionFailure(const char* file, int line, const char* expression)
{
    char message[512] = "";
    const char* shown_file = file != nullptr ? file : "?";
    const char* shown_expression = expression != nullptr ? expression : "?";
    // A message cut short by the buffer is still worth reporting, so the count written isn't needed.
    (void)sprintf_s(message, "Assertion failed: %s (%s, line %d)\n", shown_expression, shown_file, line);
    OutputDebugStringA(message);
    return false;
}
