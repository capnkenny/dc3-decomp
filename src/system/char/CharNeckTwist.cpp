#include "char/CharNeckTwist.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"

CharNeckTwist::CharNeckTwist() : mTwist(this), mHead(this) {}

BEGIN_HANDLERS(CharNeckTwist)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharNeckTwist)
    SYNC_PROP(head, mHead)
    SYNC_PROP(twist, mTwist)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharNeckTwist)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mHead;
    bs << mTwist;
END_SAVES

BEGIN_COPYS(CharNeckTwist)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharNeckTwist)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mHead)
        COPY_MEMBER(mTwist)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(CharNeckTwist)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mHead;
    d >> mTwist;
END_LOADS

void CharNeckTwist::Poll() {
    if (mHead && mTwist) {
        RndTransformable *twistParent = mTwist->TransParent();
        if ((int)twistParent) {
            Transform localHead(mHead->LocalXfm());
            RndTransformable *transItr = mHead->TransParent();
            if (transItr) {
                for (; transItr && transItr != twistParent;
                     transItr = transItr->TransParent()) {
                    Multiply(localHead, transItr->LocalXfm(), localHead);
                }
                if (transItr) {
                    Hmx::Quat rot;
                    MakeRotQuatUnitX(localHead.m.x, rot);
                    Vector3 v90;
                    Multiply(localHead.m.y, rot, v90);
                    float ang = LimitAng(std::atan2(v90.z, v90.y)) / 2.0f;
                    if (!IsNaN(ang)) {
                        MakeRotMatrixX(ang, mTwist->DirtyLocalXfm().m);
                    }
                }
            }
        }
    }
}

void CharNeckTwist::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mHead);
    change.push_back(mTwist);
}
