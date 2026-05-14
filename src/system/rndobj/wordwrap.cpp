#include "wordwrap.h"

unsigned int (*GetWidthW)(wchar_t character);
unsigned int (*Reserved)();
unsigned int lbl_830E1CD4;
unsigned int lbl_830E1CD8;
unsigned int lbl_830E1CDC;

unsigned int g_uOption = 1;
char s_wordwraplib_ident[] = "%%%WORDWRAPLIB:091027:%%%";

KinsokuEntry kinsokuChars[0x91] = {
    { 0x0021, 1, 0 },
    { 0x0024, 0, 1 },
    { 0x0025, 1, 0 },
    { 0x0027, 1, 1 },
    { 0x0028, 0, 1 },
    { 0x0029, 1, 0 },
    { 0x002C, 1, 0 },
    { 0x002E, 1, 0 },
    { 0x002F, 1, 1 },
    { 0x003A, 1, 0 },
    { 0x003B, 1, 0 },
    { 0x003F, 1, 0 },
    { 0x005B, 0, 1 },
    { 0x005C, 0, 1 },
    { 0x005D, 1, 0 },
    { 0x007B, 0, 1 },
    { 0x007D, 1, 0 },
    { 0x00A2, 1, 0 },
    { 0x00A3, 0, 1 },
    { 0x00A5, 0, 1 },
    { 0x00A7, 0, 1 },
    { 0x00A8, 1, 0 },
    { 0x00A9, 1, 0 },
    { 0x00AE, 1, 0 },
    { 0x00B0, 1, 0 },
    { 0x00B7, 1, 1 },
    { 0x02C7, 1, 0 },
    { 0x02C9, 1, 0 },
    { 0x2013, 1, 0 },
    { 0x2014, 1, 0 },
    { 0x2015, 1, 0 },
    { 0x2016, 1, 0 },
    { 0x2018, 0, 1 },
    { 0x2019, 1, 0 },
    { 0x201C, 0, 1 },
    { 0x201D, 1, 0 },
    { 0x2022, 1, 0 },
    { 0x2025, 1, 0 },
    { 0x2026, 1, 0 },
    { 0x2027, 1, 0 },
    { 0x2032, 1, 0 },
    { 0x2033, 1, 0 },
    { 0x2035, 0, 1 },
    { 0x2103, 1, 0 },
    { 0x2122, 1, 0 },
    { 0x2236, 1, 0 },
    { 0x2574, 1, 0 },
    { 0x266F, 0, 1 },
    { 0x3001, 1, 0 },
    { 0x3002, 1, 0 },
    { 0x3003, 1, 0 },
    { 0x3005, 1, 0 },
    { 0x3008, 0, 1 },
    { 0x3009, 1, 0 },
    { 0x300A, 0, 1 },
    { 0x300B, 1, 0 },
    { 0x300C, 0, 1 },
    { 0x300D, 1, 0 },
    { 0x300E, 0, 1 },
    { 0x300F, 1, 0 },
    { 0x3010, 0, 1 },
    { 0x3011, 1, 0 },
    { 0x3012, 0, 1 },
    { 0x3014, 0, 1 },
    { 0x3015, 1, 0 },
    { 0x3016, 0, 1 },
    { 0x3017, 1, 0 },
    { 0x301D, 0, 1 },
    { 0x301E, 1, 0 },
    { 0x301F, 1, 0 },
    { 0x3041, 1, 0 },
    { 0x3043, 1, 0 },
    { 0x3045, 1, 0 },
    { 0x3047, 1, 0 },
    { 0x3049, 1, 0 },
    { 0x3063, 1, 0 },
    { 0x3083, 1, 0 },
    { 0x3085, 1, 0 },
    { 0x3087, 1, 0 },
    { 0x308E, 1, 0 },
    { 0x3099, 1, 0 },
    { 0x309A, 1, 0 },
    { 0x309B, 1, 0 },
    { 0x309C, 1, 0 },
    { 0x309D, 1, 0 },
    { 0x309E, 1, 0 },
    { 0x30A1, 1, 0 },
    { 0x30A3, 1, 0 },
    { 0x30A5, 1, 0 },
    { 0x30A7, 1, 0 },
    { 0x30A9, 1, 0 },
    { 0x30C3, 1, 0 },
    { 0x30E3, 1, 0 },
    { 0x30E5, 1, 0 },
    { 0x30E7, 1, 0 },
    { 0x30EE, 1, 0 },
    { 0x30F5, 1, 0 },
    { 0x30F6, 1, 0 },
    { 0x30FB, 1, 0 },
    { 0x30FC, 1, 0 },
    { 0x30FD, 1, 0 },
    { 0x30FE, 1, 0 },
    { 0xFE30, 1, 0 },
    { 0xFE50, 1, 0 },
    { 0xFE51, 1, 0 },
    { 0xFE52, 1, 0 },
    { 0xFE54, 1, 0 },
    { 0xFE55, 1, 0 },
    { 0xFE56, 1, 0 },
    { 0xFE57, 1, 0 },
    { 0xFE59, 0, 1 },
    { 0xFE5A, 1, 0 },
    { 0xFE5B, 0, 1 },
    { 0xFE5C, 1, 0 },
    { 0xFE5D, 0, 1 },
    { 0xFE5E, 1, 0 },
    { 0xFF01, 1, 0 },
    { 0xFF02, 1, 0 },
    { 0xFF04, 0, 1 },
    { 0xFF05, 1, 0 },
    { 0xFF07, 1, 0 },
    { 0xFF08, 0, 1 },
    { 0xFF09, 1, 0 },
    { 0xFF0C, 1, 0 },
    { 0xFF0E, 1, 0 },
    { 0xFF1A, 1, 0 },
    { 0xFF1B, 1, 0 },
    { 0xFF1F, 1, 0 },
    { 0xFF20, 0, 1 },
    { 0xFF3B, 0, 1 },
    { 0xFF3D, 1, 0 },
    { 0xFF40, 1, 0 },
    { 0xFF5B, 0, 1 },
    { 0xFF5C, 1, 0 },
    { 0xFF5D, 1, 0 },
    { 0xFF5E, 1, 0 },
    { 0xFF61, 1, 0 },
    { 0xFF64, 1, 0 },
    { 0xFF70, 1, 0 },
    { 0xFF9E, 1, 0 },
    { 0xFF9F, 1, 0 },
    { 0xFFE0, 1, 1 },
    { 0xFFE1, 0, 1 },
    { 0xFFE5, 0, 1 },
    { 0xFFE6, 0, 1 },
};

void WordWrap_SetOption(unsigned int option)
{
    g_uOption = option;
}

bool IsEastAsianChar(wchar_t character)
{
    if((g_uOption & 4) != 0 && 
        (character >= 0x1100u && character <= 0x11FFu 
            || character >= 0x3130u && character <= 0x318Fu 
            || character >= 0xAC00u && character <= 0xD7A3u))
    {
        return false;
    }

    return character >= 0x1100u && character <= 0x11FFu
      || character >= 0x3000u && character <= 0xD7AFu
      || character >= 0xF900u && character <= 0xFAFFu
      || character >= 0xFF00u && character <= 0xFFDCu;
}

bool WordWrap_CanBreakLineAt(wchar_t const * beforeBreak, wchar_t const * afterBreak)
{
    wchar_t wc;
    unsigned int eastAsian;
    unsigned int prevChar;
    unsigned int hi;
    int mid;
    unsigned int prevCharSave;
    unsigned int tmpOptions;
    unsigned int lo;
    unsigned char checkChar;
    wchar_t currentChar;
  
  tmpOptions = g_uOption;
  if (beforeBreak == afterBreak) 
  {
    return false;
  }

  currentChar = *beforeBreak;
  if((currentChar == L'\t' || currentChar == L'\r' || currentChar == L' ' || currentChar == L'\x3000')) {
    //binary search #1
    if ((tmpOptions & 1) != 0) {
      lo = 0;
      hi = 0x91;
      do {
        int diff = hi - lo; 
        mid = (diff >> 1) + lo;
        if (beforeBreak[1] == kinsokuChars[mid].key) {
          checkChar = kinsokuChars[mid].noStart;
          goto IsCheckCharEmpty_1;
        }
        if ((unsigned short)beforeBreak[1] < (unsigned short)kinsokuChars[mid].key) {
          hi = mid - 1;
        }
        else {
          lo = mid + 1;
        }
      } while (lo <= hi);
    }
    checkChar = '\0';
IsCheckCharEmpty_1:
    if (checkChar != '\0') {
      return false;
    }
  }
  // Horrible to read but needs to happen...checks that a proper break is formed by the current and previous characters.
  if ((((((unsigned int)((char*)beforeBreak - (char*)afterBreak) & ~1u) <= 2u)  ||
        (((((wc = beforeBreak[-2], wc != L'\t' && (wc != L'\r')) &&
          ((wc != L' ' && (wc != L'\x3000')))) ||
          ((beforeBreak[-1] != L'\"' || (currentChar == L'\t')))) || (currentChar == L'\r')))) ||
      ((currentChar == L' ' || (currentChar == L'\x3000')))) &&
      (((prevChar = (unsigned short)beforeBreak[-1], prevChar == 9 ||
        (((prevChar == 0xd || (prevChar == 0x20)) || (prevChar == 0x3000 )))) ||
      ((currentChar != L'\"' ||
        (((wc = beforeBreak[1], wc != L'\t' && (wc != L'\r')) &&
        ((wc != L' ' && (wc != L'\x3000')))))))))) {
    if (((currentChar == L'\t') || (currentChar == L'\r')) ||
        ((currentChar == L' ' ||
        ((((currentChar == L'\x3000' ||
            (prevCharSave = prevChar, eastAsian = IsEastAsianChar(currentChar), (eastAsian & 0xff) != 0)) ||
          (eastAsian = IsEastAsianChar((unsigned short)prevChar), (eastAsian & 0xff) != 0)) ||
          (prevCharSave == 0x2d)))))) {
      // binary search #2
      if ((tmpOptions & 1) != 0) {
        lo = 0;
        hi = 0x91;
        do {
          int diff = hi - lo;
          mid = (diff >> 1) + lo;
          if (currentChar == kinsokuChars[mid].key) {
            checkChar = kinsokuChars[mid].noStart;
            goto EndRuleCheck;
          }
          if ((unsigned short)currentChar < (unsigned short)kinsokuChars[mid].key) {
            hi = mid - 1;
          }
          else {
            lo = mid + 1;
          }
        } while (lo <= hi);
      }
      checkChar = '\0';
EndRuleCheck:
      if (checkChar == '\0') {
        // binary search #3
        if ((tmpOptions & 1) != 0) {
          lo = 0;
          hi = 0x91;
          do {
            int diff = hi - lo;
            mid = (diff >> 1) + lo;
            if (prevChar == (unsigned short)kinsokuChars[mid].key) {
              checkChar = kinsokuChars[mid].noEnd;
              goto IsCheckCharEmpty_2;
            }
            if (prevChar < (unsigned short)kinsokuChars[mid].key) {
              hi = mid - 1;
            }
            else {
              lo = mid + 1;
            }
          } while (lo <= hi);
        }
        checkChar = '\0';
IsCheckCharEmpty_2:
      if (checkChar == '\0') 
      {
        return true;
      }
    }
  }
  return false;
}
  
}
