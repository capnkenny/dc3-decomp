#pragma once
#include "os/Debug.h"
#include "utl/MemMgr.h"

class Rand {
public:
    Rand(int);
    void Seed(int);
    int Int();
    int Int(int, int);

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
