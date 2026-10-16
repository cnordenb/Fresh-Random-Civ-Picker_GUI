#pragma once

// Assertions in the form of P10 rule 5 (CLAUDE2.md section 3): a side-effect-free Boolean test of something that
// should never happen. A failed assertion is reported (file, line and expression go to the debugger output) and
// the macro evaluates to false, so the caller takes its recovery action:
//     if (!c_assert(index < MAX_CIVS)) return false;
// It never aborts and it stays in Release builds, unlike assert() from <cassert>.

bool ReportAssertionFailure(const char* file, int line, const char* expression);

#define c_assert(expression) ((expression) ? true : ReportAssertionFailure(__FILE__, __LINE__, #expression))
