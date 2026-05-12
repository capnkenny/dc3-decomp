#include "wordwrap.h"

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

bool WordWrap_CanBreakLineAt(wchar_t const * leftWord, wchar_t const * rightWord)
{
    wchar_t wVar1;
    unsigned long long uVar2;
    unsigned long long uVar3;
    int iVar4;
    int iVar5;
    unsigned int uVar7;
    unsigned long long uVar6;
    unsigned int uVar8;
    int iVar9;
    char checkChar;
    wchar_t firstLetter;
  
  uVar8 = g_uOption;
  if (leftWord != rightWord) {
    firstLetter = *leftWord;
      if((firstLetter == L'\t' || firstLetter == L'\r' || firstLetter == L' ' ||
        firstLetter == L'\x3000'))  
    if ((g_uOption & 1) != 0) {
        iVar9 = 0;
        iVar4 = 0x91;
        do {
          uVar7 = iVar4 - iVar9;
          iVar5 = ((int)uVar7 >> 1) + (unsigned int)((int)uVar7 < 0 && (uVar7 & 1) != 0) + iVar9;
          // if (leftWord[1] == *(wchar_t *)(&DAT_82f16ad0 + iVar5 * 4)) {
          //   checkChar = (&DAT_82f16ad2)[iVar5 * 4];
          //   goto LAB_827317a8;
          // }
          // if ((unsigned short)leftWord[1] < (unsigned short)GetWidthW(iVar5 * 4)){//*(wchar_t *)(&DAT_82f16ad0 + iVar5 * 4)) {
          //   iVar4 = iVar5 + -1;
          // }
          // else {
            iVar9 = iVar5 + 1;
          //}
        } while (iVar9 <= iVar4);
      }
      checkChar = '\0';
LAB_827317a8:
      if (checkChar != '\0') {
        return false;
      }
    }
    if (((((int)((int)leftWord - (int)rightWord & 0xfffffffeU) < 3)  ||
         (((((wVar1 = leftWord[-2], wVar1 != L'\t' && (wVar1 != L'\r')) &&
            ((wVar1 != L' ' && (wVar1 != L'\x3000')))) ||
           ((leftWord[-1] != L'\"' || (firstLetter == L'\t')))) || (firstLetter == L'\r')))) ||
        ((firstLetter == L' ' || (firstLetter == L'\x3000')))) &&
       (((uVar3 = (unsigned long long)(unsigned short)leftWord[-1], uVar3 == 9 ||
         (((uVar3 == 0xd || (uVar3 == 0x20)) || (uVar3 == 0x3000 )))) ||
        ((firstLetter != L'\"' ||
         (((wVar1 = leftWord[1], wVar1 != L'\t' && (wVar1 != L'\r')) &&
          ((wVar1 != L' ' && (wVar1 != L'\x3000')))))))))) {
      if (((firstLetter == L'\t') || (firstLetter == L'\r')) ||
         ((firstLetter == L' ' ||
          ((((firstLetter == L'\x3000' ||
             (uVar6 = uVar3, uVar2 = IsEastAsianChar(firstLetter), (uVar2 & 0xff) != 0)) ||
            (uVar2 = IsEastAsianChar((unsigned short)uVar3), (uVar2 & 0xff) != 0)) ||
           ((uVar6 & 0xffffffff) == 0x2d)))))) {
        if ((uVar8 & 1) != 0) {
          iVar9 = 0;
          iVar4 = 0x91;
          do {
            uVar7 = iVar4 - iVar9;
            iVar5 = ((int)uVar7 >> 1) + (unsigned int)((int)uVar7 < 0 && ( uVar7 & 1) != 0) + iVar9;
            // if (firstLetter == *(wchar_t *)(&DAT_82f16ad0 + iVar5 * 4)) {
            //   //checkChar = (&DAT_82f16ad2)[iVar5 * 4];
            //   goto LAB_82731908;
            // }
            // if ((unsigned short)firstLetter < (unsigned short)*(wchar_t *)(&DAT_82f16ad0 + iVar5 * 4)) {
            //   iVar4 = iVar5 + -1;
            // }
            // else {
              iVar9 = iVar5 + 1;
            //}
          } while (iVar9 <= iVar4);
        }
        checkChar = '\0';
LAB_82731908:
        if (checkChar == '\0') {
          if ((uVar8 & 1) != 0) {
            iVar9 = 0;
            iVar4 = 0x91;
            do {
              uVar8 = iVar4 - iVar9;
              iVar5 = ((int)uVar8 >> 1) + (unsigned int)((int)uVar8 < 0 && (uVar8 & 1) != 0) + iVar9;
              // if ((uVar3 & 0xffff) == (unsigned long long)*(unsigned short *)(&DAT_82f16ad0 + iVar5 * 4)) {
              //   checkChar = (&DAT_82f16ad3)[iVar5 * 4];
              //   goto LAB_8273196c;
              // }
              // if ((uVar3 & 0xffff) < (unsigned long long)*(unsigned short *)(&DAT_82f16ad0 + iVar5 * 4)) {
              //   iVar4 = iVar5 + -1;
              // }
              // else {
                iVar9 = iVar5 + 1;
              //}
            } while (iVar9 <= iVar4);
          }
          checkChar = '\0';
LAB_8273196c:
        if (checkChar == '\0') 
        {
          return true;
        }
      }
    }
    return false;
  }
}
