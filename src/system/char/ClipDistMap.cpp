#include "char/ClipDistMap.h"
#include "char/CharClip.h"
#include "math/Utl.h"
#include "rndobj/Rnd.h"
#include "utl/Std.h"
#include <cmath>

struct DistMapNodeSort {
    bool operator()(const ClipDistMap::Node &n1, const ClipDistMap::Node &n2) const {
        return n1.a < n2.a;
    }
};

void FindWeights(
    std::vector<RndTransformable *> &transes,
    std::vector<float> &floats,
    const DataArray *arr
) {
    floats.resize(transes.size());
    float f1 = 0;
    for (int i = 0; i < transes.size(); i++) {
        float len = Length(transes[i]->LocalXfm().v);
        if (arr) {
            float f84 = 1;
            arr->FindData(transes[i]->Name(), f84, false);
            len *= f84;
        }
        floats[i] = len;
        f1 += floats[i];
    }
    for (int i = 0; i < floats.size(); i++) {
        floats[i] *= floats.size() / f1;
    }
}

#pragma region DistEntry

DistEntry &DistEntry::operator=(const DistEntry &right) {
    beat = right.beat;
    bones = right.bones;
    for (int i = 0; i < 4; i++) {
        facing[i] = right.facing[i];
    }
    return *this;
}

DistEntry::DistEntry(const DistEntry &entry) : beat(entry.beat), bones(entry.bones) {
    memcpy(facing, entry.facing, sizeof(entry.facing));
}

#pragma endregion
#pragma region Array2d

void ClipDistMap::Array2d::Resize(int w, int h) {
    delete mData;
    mWidth = w;
    mHeight = h;
    mData = new float[w * h];
}

#pragma endregion

ClipDistMap::ClipDistMap(
    CharClip *clip1, CharClip *clip2, float f1, float f2, int i, const DataArray *a
)
    : mClipA(clip1), mClipB(clip2), mWeightData(a), mSamplesPerBeat(8),
      mLastMinErr(kHugeFloat), mBeatAlign(f1), mBeatAlignOffset(0), mBlendWidth(f2),
      mNumSamples(i), mDists(CalcWidth(), CalcHeight()) {
    mBeatAlignPeriod = mBeatAlign * (float)mSamplesPerBeat + 0.5;
    if (mBeatAlignPeriod != 0) {
        int tmp = (-mAStart - -mBStart) * (float)mSamplesPerBeat;
        mBeatAlignOffset = Offset(tmp);
    }
}

bool ClipDistMap::BeatAligned(int i1, int i2) {
    return Offset(i1 - i2) == mBeatAlignOffset;
}

bool ClipDistMap::FindBestNode(float f1, float f2, float f3, ClipDistMap::Node &node) {
    int temp1;
    int temp2;
    if (f2 < f3) {
        temp1 = (f3 - mAStart) * mSamplesPerBeat;
        temp2 = (f2 - mAStart) * mSamplesPerBeat;
        temp2 = 0xffffffff - (temp2 >> 0x1f) & temp2;
        int max = Max(temp1, mDists.Width());
        // if (temp1 <= mDists.Width()) {
        //     mDists.Width() = temp1;
        // }
        for (int i = 0; i < max; i++) {
        }
    }
    return false;
}

void ClipDistMap::FindNodes(float f1, float f2, float f3) {
    mNodes.clear();
    mLastMinErr = f1;

    float f4 = f2 * 0.45f;
    if (f2 == 0.0f) {
        f4 = kHugeFloat;
        f3 = f4;
    } else if (f3 == 0.0f) {
        f3 = f4;
    }

    FindBestNodeRecurse(f1, f4, (f4 * 2.0f - f2), mAStart, mAEnd);

    std::sort(mNodes.begin(), mNodes.end(), DistMapNodeSort());

    if (!mNodes.empty() && f3 > 0.0f) {
        float lastNodeDist = mAEnd - mNodes.back().a;
        if (lastNodeDist > f3) {
            ClipDistMap::Node node;
            if (FindBestNode(f1, mAEnd - f3, mAEnd, node)) {
                mNodes.push_back(node);
                std::sort(mNodes.begin(), mNodes.end(), DistMapNodeSort());
            }
        }
    }

    int limit = mNodes.size() - 1;
    if (limit > 1) {
        for (int i = 1; i < limit;) {
            float dist = mNodes[i + 1].a - mNodes[i].a;
            if (dist < f2) {
                mNodes.erase(mNodes.begin() + (i + 1));
                i--;
            }
            i++;
            limit = mNodes.size() - 1;
        }
    }
}

int ClipDistMap::CalcWidth() {
    float clipAStartBeat = mClipA->StartBeat();
    float div = 1.0f / (float)mSamplesPerBeat;
    mAStart = clipAStartBeat - Mod(clipAStartBeat, div);
    if (mAStart < mClipA->StartBeat()) {
        mAStart += div;
    }
    float clipAEndBeat = mClipA->EndBeat();
    mAEnd = clipAEndBeat - Mod(clipAEndBeat, div);
    if (mAEnd + div <= mClipA->EndBeat()) {
        mAEnd += div;
    }
    int i1 = Max(0, (int)floorf((mAEnd - mAStart) * (float)(mSamplesPerBeat) + 0.5f)) + 1;
    mAEnd = (float)(i1 - 1) / (float)mSamplesPerBeat + mAStart;
    return i1;
}

int ClipDistMap::CalcHeight() {
    float clipBStartBeat = mClipB->StartBeat();
    float div = 1.0f / (float)mSamplesPerBeat;
    mBStart = clipBStartBeat - Mod(clipBStartBeat, div);
    if (mBStart < mClipB->StartBeat()) {
        mBStart += div;
    }
    float clipBEndBeat = mClipB->EndBeat();
    float bEnd = clipBEndBeat - Mod(clipBEndBeat, div);
    if (bEnd + div <= mClipB->EndBeat()) {
        bEnd += div;
    }
    return Max(0, (int)floorf((bEnd - mBStart) * (float)(mSamplesPerBeat) + 0.5f)) + 1;
}

void ClipDistMap::SetNodes(ClipDistMap::Node *node1, ClipDistMap::Node *node2) {
    mClipA->GetTransitions().RemoveClip(mClipB);
    for (int i = 0; i < mNodes.size(); i++) {
        if (node1 && MinEq(node1->err, mNodes[i].err)) {
            *node1 = mNodes[i];
        }
        if (node2 && MaxEq(node2->err, mNodes[i].err)) {
            *node2 = mNodes[i];
        }
        CharGraphNode graphNode;
        graphNode.curBeat = mNodes[i].a;
        graphNode.nextBeat = mNodes[i].b;
        mClipA->GetTransitions().AddNode(mClipB, graphNode);
    }
}

void ClipDistMap::DrawDot(float x, float y, float f3, float f4, Hmx::Color const &color) {
    Hmx::Rect rect;
    rect.w = 2.0;
    rect.h = 2.0;
    float scale = (float)mSamplesPerBeat;
    rect.x = (f3 - mAStart) * scale * 2.0f + (x - 1.0f);
    rect.y = ((f4 - mBStart) * scale - (float)(mDists.Height() - 1)) * 2.0f + y + 1.0f;
    TheRnd.DrawRect(rect, color, nullptr, nullptr, nullptr);
}
