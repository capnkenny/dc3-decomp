#include "char/CharIKFoot.h"
#include "CharIKHand.h"
#include "char/Character.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Trans.h"

static const float sFSMFloat = 0.5f;

CharIKFoot::CharIKFoot() : unkb0(this), unkc4(0), mData(this), mDataIndex(0) {
    unkb0 = Hmx::Object::New<RndTransformable>();
    unkb0->DirtyLocalXfm().Reset();
}

CharIKFoot::~CharIKFoot() { delete unkb0; }

BEGIN_HANDLERS(CharIKFoot)
    HANDLE_SUPERCLASS(CharIKHand)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKFoot)
    SYNC_PROP(data, mData)
    SYNC_PROP(data_index, mDataIndex)
    SYNC_SUPERCLASS(CharIKHand)
END_PROPSYNCS

BEGIN_SAVES(CharIKFoot)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(CharIKHand)
    bs << mData;
    bs << mDataIndex;
END_SAVES

BEGIN_COPYS(CharIKFoot)
    COPY_SUPERCLASS(CharIKHand)
    CREATE_COPY(CharIKFoot)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mData)
        COPY_MEMBER(mDataIndex)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(6, 0)

BEGIN_LOADS(CharIKFoot)
    LOAD_REVS(bs)
    ASSERT_REVS(6, 0)
    LOAD_SUPERCLASS(CharIKHand)
    if (d.rev < 6) {
        Symbol s;
        d >> s;
    }
    if (d.rev < 5) {
        int i;
        if (d.rev > 1)
            d >> i;
        if (d.rev > 2)
            d >> i;
        if (d.rev > 3)
            d >> i;
    } else {
        d >> mData;
        d >> mDataIndex;
    }
END_LOADS

void CharIKFoot::Enter() {
    unkc4 = 0;
    unkf0 = 0.0f;
}

void CharIKFoot::PollDeps(std::list<Hmx::Object *> &l1, std::list<Hmx::Object *> &l2) {
    CharIKHand::PollDeps(l1, l2);
}

void CharIKFoot::Poll() {
    if (mFinger && mHand && mData) {
        mTargets.clear();
        mTargets.push_back(IKTarget(unkb0, 0));
        DoFSM(Character::Current(), unkb0->DirtyLocalXfm());
        CharIKHand::Poll();
        mTargets.clear();
    }
}

void CharIKFoot::DoFSM(Character *c, Transform &tf) {
    unkf4 = mFinger->WorldXfm();
    if (c && c->Teleported()) {
        unkc4 = 0;
    }
    float secs = TheTaskMgr.DeltaSeconds();
    if (secs < 0) {
        secs = 0;
    }
    tf.m = mFinger->WorldXfm().m;
    tf.v.z = mFinger->WorldXfm().v.z;
    unke0.z = tf.v.z;
    bool b7 = false;
    float vecIdx = mData->LocalXfm().v[mDataIndex];
    if (vecIdx >= 0.98f) {
        b7 = true;
    } else if (vecIdx > 0 && tf.v.z < (unkc4 == 1 ? 0.6f : sFSMFloat)) {
        b7 = true;
    }
    if (unkc4 == 0) {
        const Transform &fingerXfm = mFinger->WorldXfm();
        tf.v.x = fingerXfm.v.x;
        tf.v.y = fingerXfm.v.y;
        if (b7) {
            unke0 = tf.v;
            unkc4 = 1;
        }
    }
    if (unkc4 == 1) {
        if (!b7) {
            unkc4 = 2;
            unkf0 = Distance(mFinger->WorldXfm().v, tf.v);
        } else {
            Vector3 v3c;
            Subtract(mFinger->WorldXfm().v, unke0, v3c);
            float len = Length(v3c);
            if (len > 0.125f) {
                // Scale(v3c, 0.125f / len, v3c);
                v3c *= 0.125f / len;
            }
            Add(unke0, v3c, tf.v);
            return;
        }
    }
    if (unkc4 == 2) {
        Vector3 v48;
        Subtract(mFinger->WorldXfm().v, tf.v, v48);
        float len = Length(v48);
        unkf0 = Min(-(secs * 25.0f - unkf0), len);
        if (unkf0 <= 0.0f)
            unkc4 = 0;
        else
            v48 *= (len - unkf0) / len;
        tf.v += v48;
        if (b7) {
            unke0 = tf.v;
            unkc4 = 1;
        }
    }
}
