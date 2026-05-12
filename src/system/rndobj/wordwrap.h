#pragma once

#include "xdk/win_types.h"

unsigned int g_uOption = 1;

unsigned int (*GetWidthW)(wchar_t character);
unsigned int (*Reserved)();

void WordWrap_SetOption(unsigned int option);
bool IsEastAsianChar(wchar_t character);
bool WordWrap_CanBreakLineAt(wchar_t const * a1, wchar_t const * a2);
