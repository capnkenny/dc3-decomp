#pragma once
#include "os/Debug.h"
#include "utl/MemMgr.h"

class Rand {
public:
    Rand(int);
    void Seed(int);

    int Int() {
        unsigned int ret = mRandTable[mRandIndex1] ^ mRandTable[mRandIndex2];
        mRandTable[mRandIndex1] = ret;
        mRandIndex1++;
        mRandIndex2++;
        if (0xF9 <= mRandIndex1) {
            mRandIndex1 = 0;
        }
        if (0xF9 <= mRandIndex2) {
            mRandIndex2 = 0;
        }
        return ret;
    }

    int Int(int low, int high) {
        MILO_ASSERT(high > low, 0x3A);
        return (Int() % (high - low)) + low;
    }

    int FastInt(int low, int high) {
        MILO_ASSERT(high > low, 0x33);
        return ((Int() & 0xFFFF) * (high - low) >> 16) + low;
    }

    float Float();
    float Float(float, float);
    float Gaussian();

    static Rand sRand;

    MEM_OVERLOAD(Rand, 0x16)

private:
    unsigned int mRandIndex1;
    unsigned int mRandIndex2;
    unsigned int mRandTable[256];
    float mSpareGaussianValue;
    bool mSpareGaussianAvailable;
};

void SeedRand(int);
int RandomInt();
int RandomInt(int, int);
float RandomFloat();
float RandomFloat(float, float);
