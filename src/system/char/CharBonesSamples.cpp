#include "char/CharBonesSamples.h"
#include "CharClip.h"
#include "char/CharBones.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/ChunkStream.h"
#include "utl/MemMgr.h"

CharBonesSamples::CharBonesSamples()
    : mNumSamples(0), mPreviewSample(0), mRawData(nullptr) {}

CharBonesSamples::~CharBonesSamples() { MemFree(mRawData); }

void CharBonesSamples::SetPreview(int i) {
    mPreviewSample = Clamp(0, mNumSamples - 1, i);
    mStart = &mRawData[mTotalSize * mPreviewSample];
}

BEGIN_PROPSYNCS(CharBonesSamples)
    SYNC_PROP(num_samples, mNumSamples)
    SYNC_PROP_SET(preview_sample, mPreviewSample, SetPreview(_val.Int()))
    SYNC_PROP(frames, mFrames)
    SYNC_PROP_SET(compression, (int &)mCompression, ) {
        gPropBones = this;
        static Symbol bones("bones");
        if (sym == bones)
            return PropSync(mBones, _val, _prop, _i + 1, _op);
    }
END_PROPSYNCS

void CharBonesSamples::Save(BinStream &bs) {
    SAVE_REVS(0x10, 0);
    bs << mBones;
    for (int i = 0; i < NUM_TYPES; i++) {
        bs << mCounts[i];
    }
    bs << mCompression;
    bs << mNumSamples;
    bs << mFrames;
    bool check =
        (bs.Cached()
         && (bs.GetPlatform() == kPlatformPS3 || bs.GetPlatform() == kPlatformXBox));
    int delta = 0;
    if (check) {
        delta = (mOffsets[6] - mOffsets[0]);
        delta = (delta + 0xFU & 0xFFFFFFF0) - delta;
        MILO_ASSERT(delta >= 0 && delta < 16, 0x24C);
    }
    for (int i = 0; i < mNumSamples; i++) {
        mStart = mRawData + mTotalSize * i;
        if (mCompression >= kCompressVects) {
            ShortVector3 *vecEnd = (ShortVector3 *)QuatOffset();
            for (ShortVector3 *it = (ShortVector3 *)VecOffset(); it < vecEnd; ++it) {
                bs << it->x << it->y << it->z;
            }
        } else {
            Vector3 *vecEnd = (Vector3 *)QuatOffset();
            for (Vector3 *it = (Vector3 *)VecOffset(); it < vecEnd; ++it) {
                bs << *it;
                if (check) {
                    bs << 0.0f;
                }
            }
        }
        if (mCompression >= kCompressQuats) {
            ByteQuat *quatEnd = (ByteQuat *)RotOffset();
            for (ByteQuat *it = (ByteQuat *)QuatOffset(); it < quatEnd; ++it) {
                bs << it->x << it->y << it->z << it->w;
            }
        } else if (mCompression != kCompressNone) {
            ShortQuat *quatEnd = (ShortQuat *)RotOffset();
            for (ShortQuat *it = (ShortQuat *)QuatOffset(); it < quatEnd; ++it) {
                bs << it->x << it->y << it->z << it->w;
            }
        } else {
            Hmx::Quat *quatEnd = (Hmx::Quat *)RotOffset();
            for (Hmx::Quat *it = QuatOffset(); it < quatEnd; ++it) {
                bs << *it;
            }
        }
        if (mCompression != kCompressNone) {
            for (short *it = (short *)RotOffset(); it < (short *)EndOffset(); ++it) {
                bs << *it;
            }
        } else {
            for (float *it = RotOffset(); it < (float *)EndOffset(); ++it) {
                bs << *it;
            }
        }
        if (check) {
            char zero[16];
            memset(zero, 0, 16);
            bs.Write(zero, delta);
        }
        if (bs.GetPlatform() == 5 && (i & 0x7F) == 0x7F) {
            MarkChunk(bs);
        }
    }
}

INIT_REVS(0x10, 0)

BEGIN_LOADS(CharBonesSamples)
    LOAD_REVS(bs)
    if (d.rev > 0x10) {
        MILO_FAIL(
            "%s can\'t load new %s version %d > %d", "", "CharBonesSample", d.rev, gRev
        );
    }
    if (d.altRev > 0) {
        MILO_FAIL(
            "%s can\'t load new %s alt version %d > %d",
            "",
            "CharBonesSample",
            d.altRev,
            gAltRev
        );
    }
    MILO_ASSERT(d.rev > 12, 0x29d);
    LoadHeader(d);
    LoadData(d);
END_LOADS

void CharBonesSamples::LoadHeader(BinStreamRev &d) {
    MemFree(mRawData);
    int numBones;
    d >> numBones;
    mBones.resize(numBones);
    if (d.rev > 0xA) {
        for (int i = 0; i < numBones; i++) {
            d >> mBones[i];
        }
    } else {
        for (int i = 0; i < numBones; i++) {
            d >> mBones[i].name;
        }
    }

    if (d.rev > 9) {
        ReadCounts(d.stream, d.rev > 0xF ? 7 : 10);
        d >> (int &)mCompression >> mNumSamples;
    } else {
        if (d.rev > 5) {
            int count;
            if (d.rev > 7) {
                count = 9;
            } else {
                count = d.rev > 6 ? 6 : 10;
            }
            for (int i = 0; i < count; i++) {
                int sp14;
                d >> sp14;
            }
            d >> (int &)mCompression >> mNumSamples;
        } else {
            d >> mNumSamples;
            if (d.rev > 3) {
                d >> (int &)mCompression;
            }
        }
        for (int i = 0; i < NUM_TYPES; i++) {
            mCounts[i] = 0;
        }
        for (int i = 0; i < mBones.size(); i++) {
            mCounts[CharBones::TypeOf(mBones[i].name) + 1]++;
        }
        for (int i = 1; i < NUM_TYPES; i++) {
            mCounts[i] += mCounts[i - 1];
        }
    }

    if (d.rev > 0xB) {
        d >> mFrames;
    } else {
        mFrames.clear();
    }
    RecomputeSizes();
    mRawData = (char *)MemAlloc(AllocateSize(), __FILE__, 0x301, "CharBonesSamples");
}

void CharBonesSamples::LoadData(BinStreamRev &d) {
    if (d.rev == 0xE) {
        bool b;
        d >> b;
    }
    bool cached = d.stream.Cached();
    if (cached && d.rev > 0xE) {
        mStart = mRawData;
        ReadChunks(d.stream, mStart, AllocateSize(), mTotalSize << 7);
    } else {
        for (int i = 0; i < mNumSamples; i++) {
            mStart = mRawData + mTotalSize * Min(i, mNumSamples - 1);
            if (cached) {
                d.stream.Read(mStart, mOffsets[TYPE_END] - mOffsets[TYPE_POS]);
            } else {
                if (mCompression >= kCompressVects) {
                    ShortVector3 *vecEnd = (ShortVector3 *)QuatOffset();
                    for (ShortVector3 *it = (ShortVector3 *)VecOffset(); it < vecEnd;
                         ++it) {
                        d >> it->x >> it->y >> it->z;
                    }
                } else {
                    Vector3 *vecEnd = (Vector3 *)QuatOffset();
                    for (Vector3 *it = (Vector3 *)VecOffset(); it < vecEnd; ++it) {
                        d >> *it;
                    }
                }
                if (mCompression >= kCompressQuats) {
                    ByteQuat *quatEnd = (ByteQuat *)RotOffset();
                    for (ByteQuat *it = (ByteQuat *)QuatOffset(); it < quatEnd; ++it) {
                        d >> it->x >> it->y >> it->z >> it->w;
                    }
                } else if (mCompression != kCompressNone) {
                    ShortQuat *quatEnd = (ShortQuat *)RotOffset();
                    for (ShortQuat *it = (ShortQuat *)QuatOffset(); it < quatEnd; ++it) {
                        d >> it->x >> it->y >> it->z >> it->w;
                    }
                } else {
                    Hmx::Quat *quatEnd = (Hmx::Quat *)RotOffset();
                    for (Hmx::Quat *it = QuatOffset(); it < quatEnd; ++it) {
                        d >> *it;
                    }
                }
                if (mCompression != kCompressNone) {
                    short *rotEnd = (short *)EndOffset();
                    for (short *it = (short *)RotOffset(); it < rotEnd; ++it) {
                        d >> *it;
                    }
                } else {
                    float *rotEnd = (float *)EndOffset();
                    for (float *it = RotOffset(); it < rotEnd; ++it) {
                        d >> *it;
                    }
                }
            }
            if ((i & 0x7F) == 0x7F) {
                while (d.stream.Eof() == TempEof) {
                    Timer::Sleep(0);
                }
            }
        }
    }
}

int CharBonesSamples::AllocateSize() { return mTotalSize * mNumSamples; }

void CharBonesSamples::RotateBy(CharBones &bones, int i) {
    mStart = &mRawData[mTotalSize * i];
    CharBones::RotateBy(bones);
}

void CharBonesSamples::RotateTo(CharBones &bones, float f1, int i, float f2) {
    mStart = &mRawData[mTotalSize * i];
    CharBones::RotateTo(bones, (1.0f - f2) * f1);
    if (f2 > 0.0f) {
        mStart = &mRawData[mTotalSize * (i + 1)];
        CharBones::RotateTo(bones, f2 * f1);
    }
}

void CharBonesSamples::ScaleAddSample(CharBones &bones, float f1, int i, float f2) {
    mStart = &mRawData[mTotalSize * i];
    CharBones::ScaleAdd(bones, (1.0f - f2) * f1);
    if (f2 > 0.0f) {
        mStart = &mRawData[mTotalSize * (i + 1)];
        CharBones::ScaleAdd(bones, f2 * f1);
    }
}

void CharBonesSamples::ReadCounts(BinStream &bs, int i2) {
    int i = 0;
    int numTypesToRead = Min(7, i2);
    for (; i < numTypesToRead; i++) {
        bs >> mCounts[i];
    }
    for (int numTypesRead = i; numTypesRead < i2; numTypesRead++) {
        int tmp;
        bs >> tmp;
        MILO_ASSERT((tmp - mCounts[NUM_TYPES - 1]) == 0, 0x2af);
    }
    for (; i < NUM_TYPES; i++) {
        mCounts[i] = 0;
    }
}

void CharBonesSamples::Set(
    const std::vector<CharBones::Bone> &bones, int i, CharBones::CompressionType ty
) {
    ClearBones();
    SetCompression(ty);
    mNumSamples = i;
    AddBones(bones);
    MemFree(mRawData);
    mRawData = (char *)MemAlloc(
        AllocateSize(), "CharBonesSamples.cpp", 0x2d, "CharBonesSamples", 0
    );
    mFrames.clear();
}

void CharBonesSamples::Clone(const CharBonesSamples &samp) {
    Set(samp.mBones, samp.mNumSamples, samp.mCompression);
    memcpy(mRawData, samp.mRawData, AllocateSize());
    mFrames = samp.mFrames;
}

void CharBonesSamples::Print() {
    MILO_LOG(
        "samples: %d size: %d address: %x compression %d\n",
        mNumSamples,
        AllocateSize(),
        (int)mRawData,
        GetCompression()
    );
    if (mNumSamples == 0) {
        TheDebug << "Bones:\n";
        for (int i = 0; i < mBones.size(); i++) {
            TheDebug << "   " << mBones[i].name << "\n";
        }
    }
    for (int i = 0; i < mNumSamples; i++) {
        TheDebug << i << ")\n";
        mStart = mRawData + mTotalSize * i;
        CharBones::Print();
    }
}

void CharBonesSamples::Relativize(CharClip *clip) {
    if (mBones.empty())
        return;

    for (int sample = mNumSamples - 1; sample >= 0; sample--) {
        mStart = mRawData + sample * mTotalSize;
        float startBeat = clip->StartBeat();
        int boneIdx = 0;
        if (mCompression < kCompressVects) {
            // ShortVector3 *pos =
        }
    }
}

int CharBonesSamples::FracToSample(float *frac) const {
    if (mNumSamples < 2) {
        *frac = 0.0f;
        return 0;
    } else {
        float old = *frac;
        ClampEq(*frac, 0.0f, 1.0f);
        int total = Max((int)mFrames.size(), mNumSamples);
        float scaledPos = *frac * (total - 1);
        *frac = scaledPos;
        int sampleIdx = scaledPos;
        if (sampleIdx >= total - 1) {
            *frac = 0.0f;
            return mNumSamples - 1;
        } else {
            *frac = scaledPos - sampleIdx;
            if (mFrames.size() != 0) {
                float interp = Interp(mFrames[sampleIdx], mFrames[sampleIdx + 1], *frac);
                sampleIdx = interp;
                *frac = interp - (float)sampleIdx;
            }
            if (sampleIdx < 0 || sampleIdx >= mNumSamples) {
                MILO_NOTIFY_ONCE(
                    "FracToSample: sample is %d, clip only has %d samples, frac was %g, is %g",
                    sampleIdx,
                    mNumSamples,
                    old,
                    *frac
                );
                sampleIdx = 0;
            }
            if (*frac < 0.0f || *frac >= 1.0f) {
                MILO_NOTIFY_ONCE("FracToSample: frac is %g, outside of 0 and 1", *frac);
                *frac = 0.0f;
            }
            return sampleIdx;
        }
    }
}
