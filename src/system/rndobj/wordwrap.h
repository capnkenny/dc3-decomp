#pragma once
#include "types.h"

typedef struct {
    /** The unicode character to associate with start/end rules. */
    wchar_t key;
    /** Indicates if the character cannot start a line. */
    u8 noStart;
    /** Indicates if the character cannot end a line. */
    u8 noEnd;
} KinsokuEntry;

void WordWrap_SetOption(uint option);

/** Checks to see if the provided character is considered an East Asian / CJK unicode
    character. */
bool IsEastAsianChar(wchar_t character);

/** Determines if a line break is allowed between two characters. Follows kinsoku line
    breaking rules for CJK unicode text. */
bool WordWrap_CanBreakLineAt(wchar_t const *beforeBreak, wchar_t const *afterBreak);
