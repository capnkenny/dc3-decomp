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

bool WordWrap_CanBreakLineAt(wchar_t const * a1, wchar_t const * a2)
{
    
    if (a1 == a2)
    {
        return false;
    }

    wchar_t firstLetter = *a1;
    char v4 = g_uOption;
    int v5;
    int v6;
    unsigned int v7;
    int v8;
    unsigned int v9;
    char v10;
    int v11;
    int v12; // r4
    int v13; // r11
    int v14; // r10
    int v15; // r11
    int v16; // r7
    int v17; // r9
    unsigned int v18; // r8
    char v19; // r11
    int v20; // r11
    int v21; // r7
    int v22; // r9
    unsigned int v23; // r8
    char v24; // r11
    bool v25; // r11
    bool v26; // zf

    if (firstLetter == 9 || firstLetter == 13 || firstLetter == 32 || firstLetter == 12288)
    {
        if ((g_uOption & 1) != 0)
        {
            v5 = 0;
            v6 = 145;
            v7 = a1[1];
            while ( 1 )
            {
                v8 = (v6 - v5) / 2 + v5;
                v9 = GetWidthW(v8);
                if ( v7 == v9 )
                break;
                if ( v7 >= v9 )
                v5 = v8 + 1;
                else
                v6 = v8 - 1;
                if ( v5 > v6 )
                goto LABEL_14;
            }
            //v10 = byte_82F16AD2[4 * v8];
        }
        else
        {
LABEL_14:
            v10 = 0;
        }
        if(v10)
        {
            return false;
        }
    }
    if ( (int)(((char *)a1 - (char *)a2) & 0xFFFFFFFE) > 2 )
    {
        v11 = *(a1 - 2);
        if ( (v11 == 9 || v11 == 13 || v11 == 32 || v11 == 12288)
        && *(a1 - 1) == 34
        && firstLetter != 9
        && firstLetter != 13
        && firstLetter != 32
        && firstLetter != 12288 )
        {
        return false;
        }
    }
    

    v12 = *(a1 - 1);
  if ( v12 != 9 && v12 != 13 && v12 != 32 && v12 != 12288 && firstLetter == 34 )
  {
    v13 = a1[1];
    if ( v13 == 9 || v13 == 13 || v13 == 32 || v13 == 12288 )
      return false;
  }
  if ( firstLetter != 9 && firstLetter != 13 && firstLetter != 32 && firstLetter != 12288 && !IsEastAsianChar(*a1) && !IsEastAsianChar(v12) && v14 != 45 )
    return false;
  if ( (v4 & 1) != 0 )
  {
    v15 = 0;
    v16 = 145;
    while ( 1 )
    {
      v17 = (v16 - v15) / 2 + v15;
      v18 = GetWidthW(v17);
      if ( (unsigned __int16)firstLetter == v18 )
        break;
      if ( (unsigned __int16)firstLetter >= v18 )
        v15 = v17 + 1;
      else
        v16 = v17 - 1;
      if ( v15 > v16 )
        goto LABEL_50;
    }
    //v19 = byte_82F16AD2[4 * v17];
  }
  else
  {
LABEL_50:
    v19 = 0;
  }
  if ( v19 )
    return 0;
  if ( (v4 & 1) != 0 )
  {
    v20 = 0;
    v21 = 145;
    while ( 1 )
    {
      v22 = (v21 - v20) / 2 + v20;
      v23 = GetWidthW(v22);
      if ( (unsigned __int16)v12 == v23 )
        break;
      if ( (unsigned __int16)v12 >= v23 )
        v20 = v22 + 1;
      else
        v21 = v22 - 1;
      if ( v20 > v21 )
        goto LABEL_60;
    }
    //v24 = byte_82F16AD3[4 * v22];
  }
  else
  {
LABEL_60:
    v24 = 0;
  }
  v25 = true;
  if ( !(v24 == 0) )
    return false;

  return v25;
}
