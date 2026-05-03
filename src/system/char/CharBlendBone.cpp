#include "char/CharBlendBone.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"

#pragma region CharBlendBone

CharBlendBone::CharBlendBone()
    : mTargets(this), mSrc1(this), mSrc2(this), mTransX(false), mTransY(false),
      mTransZ(false), mRotation(false), mSetLocal(false) {}

CharBlendBone::ConstraintSystem::ConstraintSystem(Hmx::Object *o)
    : mTarget(o), mWeight(0.5f) {}

BEGIN_HANDLERS(CharBlendBone)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharBlendBone::ConstraintSystem)
    SYNC_PROP(target, o.mTarget)
    SYNC_PROP(weight, o.mWeight)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharBlendBone)
    SYNC_PROP(target, mTargets)
    SYNC_PROP(src_one, mSrc1)
    SYNC_PROP(src_two, mSrc2)
    SYNC_PROP(trans_x, mTransX)
    SYNC_PROP(trans_y, mTransY)
    SYNC_PROP(trans_z, mTransZ)
    SYNC_PROP(rotation, mRotation)
    SYNC_PROP(set_local, mSetLocal)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const CharBlendBone::ConstraintSystem &cs) {
    bs << cs.mTarget;
    bs << cs.mWeight;
    return bs;
}

BEGIN_SAVES(CharBlendBone)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mTargets;
    bs << mSrc1;
    bs << mSrc2;
    bs << mTransX;
    bs << mTransY;
    bs << mTransZ;
    bs << mRotation;
    bs << mSetLocal;
END_SAVES

BEGIN_COPYS(CharBlendBone)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharBlendBone)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTargets)
        COPY_MEMBER(mSrc1)
        COPY_MEMBER(mSrc2)
        COPY_MEMBER(mTransX)
        COPY_MEMBER(mTransY)
        COPY_MEMBER(mTransZ)
        COPY_MEMBER(mRotation)
        COPY_MEMBER(mSetLocal)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(4, 0)

BinStream &operator>>(BinStream &bs, CharBlendBone::ConstraintSystem &cs) {
    bs >> cs.mTarget;
    bs >> cs.mWeight;
    return bs;
}

BEGIN_LOADS(CharBlendBone)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    MILO_ASSERT(d.rev > 2, 0x66);
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mTargets;
    d >> mSrc1;
    d >> mSrc2;
    d >> mTransX;
    d >> mTransY;
    d >> mTransZ;
    d >> mRotation;
    if (d.rev > 3) {
        d >> mSetLocal;
    }
END_LOADS

void CharBlendBone::Poll() {
    FOREACH (it, mTargets) {
        RndTransformable *target = it->mTarget;
        if (target && mSrc1 && mSrc2) {
            const Transform &src1Xfm = mSrc1->WorldXfm();
            const Transform &src2Xfm = mSrc2->WorldXfm();
            Transform targetXfm = target->WorldXfm();
            if (mTransX || mTransY || mTransZ) {
                if (mTransX) {
                    Interp(src1Xfm.v.x, src2Xfm.v.x, it->mWeight, targetXfm.v.x);
                }
                if (mTransY) {
                    Interp(src1Xfm.v.y, src2Xfm.v.y, it->mWeight, targetXfm.v.y);
                }
                if (mTransZ) {
                    Interp(src1Xfm.v.z, src2Xfm.v.z, it->mWeight, targetXfm.v.z);
                }
            }
            if (mRotation) {
                Interp(src1Xfm.m, src2Xfm.m, it->mWeight, targetXfm.m);
            }
            if (mSetLocal) {
                if (target->TransParent()) {
                    Transform inv;
                    Invert(target->TransParent()->WorldXfm(), inv);
                    Multiply(targetXfm, inv, target->DirtyLocalXfm());
                } else {
                    target->SetLocalXfm(targetXfm);
                }
            } else {
                target->SetWorldXfm(targetXfm);
            }
        }
    }
}

void CharBlendBone::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mSrc1);
    changedBy.push_back(mSrc2);
    FOREACH (it, mTargets) {
        change.push_back(it->mTarget);
    }
}

#pragma endregion CharBlendBone
