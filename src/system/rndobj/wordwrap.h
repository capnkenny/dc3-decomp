#pragma once

typedef struct {
    /** "The unicode character to associate with start/end rules." */
    wchar_t key;
    /** "Indicates if the character cannot start a line." */
    unsigned char noStart;
    /** "Indicates if the character cannot end a line."" */
    unsigned char noEnd;
} KinsokuEntry;

extern KinsokuEntry kinsokuChars[0x91];
/** "Bitmask flag for controlling what affects line breaks.
    Bit 0 - enables/disables checking against the `kinsokuChars` table.
    Bit 2 - excludes Hangul from inclusion in `IsEastAsianChar`.
    By default, value is '1' - table checks are enabled, and Hangul 
    characters are included." */
extern unsigned int g_uOption;

extern unsigned int (*GetWidthW)(wchar_t character);
extern unsigned int (*Reserved)();
extern unsigned int lbl_830E1CD4;
extern unsigned int lbl_830E1CD8;
extern unsigned int lbl_830E1CDC;

void WordWrap_SetOption(unsigned int option);

/** "Checks to see if the provided character is considered an East Asian / CJK unicode
    character." */
bool IsEastAsianChar(wchar_t character);

/** "Determines if a line break is allowed between two characters. Follows kinsoku line
    breaking rules for CJK unicode text." */
bool WordWrap_CanBreakLineAt(wchar_t const *beforeBreak, wchar_t const *afterBreak);
