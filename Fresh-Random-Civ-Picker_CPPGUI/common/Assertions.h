#pragma once

bool ReportAssertionFailure(const char* file, int line, const char* expression);

#define c_assert(expression) ((expression) ? true : ReportAssertionFailure(__FILE__, __LINE__, #expression))
