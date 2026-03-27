#pragma once
#include "xdk/win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HJSONREADER__ {
    int idk;
} HJSONREADER;

typedef struct HJSONWRITER__ {
    int idk;
} HJSONWRITER;

#ifdef __cplusplus
}
#endif

// C++ symbols
HRESULT XJSONCloseReader(HJSONREADER *reader);
HRESULT XJSONCloseWriter(HJSONWRITER *writer);
