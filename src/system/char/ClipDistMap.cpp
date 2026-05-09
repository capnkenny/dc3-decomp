#include "char/ClipDistMap.h"
#include "char/CharClip.h"
#include "math/Utl.h"
#include "os/Debug.h"
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
    if (f2 >= f3) {
        return false;
    } else {
        node.err = f1;
        int distsWidth = mDists.Width();
        int i1 = (f3 - mAStart) * (float)mSamplesPerBeat;
        int i2 = (f2 - mAStart) * (float)mSamplesPerBeat;
        int i = Max(i2, 0);
        distsWidth = Max(i1, distsWidth);
        for (; i < distsWidth; i++) {
            float samps = mSamplesPerBeat;
            int j = mDists.Height();
            int minus = j - 1;
            if (minus >= 0) {
                for (; j != 0; j--, minus--) {
                    if (MinEq(node.err, mDists(i, minus))) {
                        node.a = (float)i / samps + mAStart;
                        node.b = (float)minus / samps + mBStart;
                    }
                }
            }
        }
        return node.err < f1;
    }
}

void ClipDistMap::FindNodes(float f1, float f2, float f3) {
    mNodes.clear();
    mLastMinErr = f1;

    float f7 = f2 * 0.45f;
    if (f2 == 0) {
        f7 = kHugeFloat;
        f3 = f7;
    } else if (f3 == 0) {
        f3 = f2;
    }

    FindBestNodeRecurse(f1, f7, -(f7 * 2.0f - f2), mAStart, mAEnd);
    std::sort(mNodes.begin(), mNodes.end(), DistMapNodeSort());
    if (!mNodes.empty() && f3 > 0) {
        if (mAEnd - mNodes.back().a > f3) {
            Node node;
            if (FindBestNode(f1, mAEnd - f3, mAEnd, node)) {
                mNodes.push_back(node);
                std::sort(mNodes.begin(), mNodes.end(), DistMapNodeSort());
            }
        }
    }
    for (int i = 1; i < (int)mNodes.size() - 1; i++) {
        if (mNodes[i + 1].a - mNodes[i - 1].a < f2) {
            mNodes.erase(mNodes.begin() + i);
            i--;
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

bool ClipDistMap::LocalMin(int i, int j) {
    float data = mDists(i, j);
    if (data == kHugeFloat) {
        return false;
    } else {
        if (mBeatAlign != 0 && BeatAligned(i, j)) {
            if (i - 1 >= 0 && j - 1 >= 0 && mDists(i - 1, j - 1) < data) {
                return false;
            }
            if (i + 1 < mDists.Width() && j + 1 < mDists.Height()
                && mDists(i + 1, j + 1) < data) {
                return false;
            }
            return true;
        }

        for (int n = i - 1; n < i + 2; n++) {
            for (int i9 = j - 1; i9 < j + 2; i9++) {
                if (n != i || i9 != j) {
                    if (n >= 0 && n < mDists.Width() && i9 >= 0 && i9 < mDists.Height()) {
                        float curData = mDists(n, i9);
                        if (curData != kHugeFloat && curData < data) {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }
}

void ClipDistMap::FindBestNodeRecurse(
    float f1, float minDist, float f3, float f4, float f5
) {
    MILO_ASSERT(minDist > 0, 0x26C);
    if ((f5 - f4) > f3) {
        float f10 = Max(f5, minDist + f4 + f3);
        float f9 = Min(f4, (f5 - f3) - minDist);
        Node n;
        if (FindBestNode(f1, f9, f10, n)) {
            for (int i = 0; i < mNodes.size(); i++) {
                if (mNodes[i].a == n.a) {
                    return;
                }
            }
            mNodes.push_back(n);
            FindBestNodeRecurse(f1, minDist, f3, n.a + minDist, f5);
            FindBestNodeRecurse(f1, minDist, f3, f4, n.a - minDist);
        }
    }
}
