#include "char/CharHair.h"
#include "char/CharCollide.h"
#include "char/Character.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Trig.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"
#include "world/Dir.h"

CharHair *gHair;
CharHair::Strand *gStrand;

#pragma region CharHair::Point

CharHair::Point::Point(Hmx::Object *owner)
    : bone(owner), collides(owner), length(0), radius(0), outerRadius(0), sideLength(-1) {
    pos.Zero();
    force.Zero();
    lastFriction.Zero();
    lastZ.Zero();
    unk78.Zero();
}

CharHair::Point::Point(const CharHair::Point &pt) : bone(pt.bone), collides(pt.collides) {
    pos = pt.pos;
    bone = pt.bone;
    length = pt.length;
    collides = pt.collides;
    radius = pt.radius;
    outerRadius = pt.outerRadius;
    force = pt.force;
    lastFriction = pt.lastFriction;
    lastZ = pt.lastZ;
    sideLength = pt.sideLength;
    unk78 = pt.unk78;
}

BEGIN_CUSTOM_PROPSYNC(CharHair::Point)
    SYNC_PROP(bone, o.bone)
    SYNC_PROP(length, o.length)
    SYNC_PROP(collides, o.collides)
    SYNC_PROP(radius, o.radius)
    SYNC_PROP(outer_radius, o.outerRadius)
    SYNC_PROP(side_length, o.sideLength)
END_CUSTOM_PROPSYNC

void operator<<(BinStream &bs, const CharHair::Point &p) {
    bs << p.pos;
    bs << p.bone;
    bs << p.length;
    bs << p.radius;
    bs << p.outerRadius;
    bs << p.sideLength;
    bs << p.unk78;
}

void operator>>(BinStreamRev &d, CharHair::Point &pt) {
    d >> pt.pos;
    d >> pt.bone;
    d >> pt.length;
    if (d.rev < 3) {
        int x;
        char buf[0x100];
        d >> x;
        d.stream.ReadString(buf, 0xff);
    } else if (d.rev == 3) {
        int x;
        d >> x;
    }
    d >> pt.radius;
    if (d.rev > 1) {
        d >> pt.outerRadius;
    } else {
        pt.outerRadius = 0;
    }
    if (d.rev < 9 && d.rev > 5) {
        float x;
        d >> x;
        pt.radius += x;
        pt.outerRadius += x;
    }
    char buf[0x100];
    if (d.rev == 6) {
        d.stream.ReadString(buf, 0xff);
    }
    if (d.rev < 8) {
        pt.sideLength = -1;
        if (d.rev > 5) {
            int x;
            d >> x >> x;
        }
    } else {
        bool b = false;
        if (d.rev < 9) {
            d >> b;
        }
        d >> pt.sideLength;
        if (d.rev < 9 && !b) {
            pt.sideLength = -1;
        }
    }
    if (d.rev > 9) {
        d >> pt.unk78;
    }
    pt.collides.clear();
    pt.force.Zero();
    pt.lastFriction.Zero();
    pt.lastZ.Zero();
}

#pragma endregion CharHair::Point
#pragma region CharHair::Strand

BEGIN_CUSTOM_PROPSYNC(CharHair::Strand)
    gStrand = &o;
    SYNC_PROP_SET(root, o.mRoot.Ptr(), o.SetRoot(_val.Obj<RndTransformable>()))
    SYNC_PROP_SET(angle, o.mAngle, o.SetAngle(_val.Float()))
    SYNC_PROP(points, o.mPoints)
    SYNC_PROP(hookup_flags, o.mHookupFlags)
    SYNC_PROP(show_spheres, o.mShowSpheres)
    SYNC_PROP(show_collide, o.mShowCollide)
    SYNC_PROP(show_pose, o.mShowPose)
END_CUSTOM_PROPSYNC

void CharHair::Strand::Save(BinStream &bs) const {
    bs << mRoot;
    bs << mAngle;
    bs << mPoints;
    bs << mBaseMat;
    bs << mRootMat;
    bs << mHookupFlags;
}

BinStream &operator<<(BinStream &bs, const CharHair::Strand &s) {
    s.Save(bs);
    return bs;
}

void CharHair::Strand::Load(BinStreamRev &d) {
    d >> mRoot;
    d >> mAngle;
    d >> mPoints;
    d >> mBaseMat;
    d >> mRootMat;
    if (d.rev > 2) {
        d >> mHookupFlags;
    } else {
        mHookupFlags = 0;
    }
}

BinStreamRev &operator>>(BinStreamRev &d, CharHair::Strand &s) {
    s.Load(d);
    return d;
}

void CharHair::Strand::SetAngle(float ang) {
    mAngle = ang;
    Hmx::Matrix3 mtx;
    MakeRotMatrixX(ang * DEG2RAD, mtx);
    Multiply(mtx, mBaseMat, mRootMat);
}

void CharHair::Strand::SetRoot(RndTransformable *trans) {
    mRoot = trans;
    if (!mRoot)
        mPoints.resize(0);
    else {
        float len = mPoints.size() != 0 ? mPoints.back().length : 0;
        mBaseMat = mRoot->LocalXfm().m;
        SetAngle(mAngle);

        int depth = 0;
        for (RndTransformable *it = mRoot;; it = it->Children().front()) {
            depth++;
            if (it->Children().empty())
                break;
        }

        mPoints.resize(depth);
        depth = 0;
        for (RndTransformable *it = mRoot;; it = it->Children().front(), depth++) {
            mPoints[depth].bone = it;
            if (it->Children().empty())
                break;
        }

        Point *pt = nullptr;
        for (int i = 1; i < mPoints.size(); i++) {
            pt = &mPoints[i - 1];
            RndTransformable *bone = mPoints[i].bone;
            pt->length = bone->LocalXfm().v.y;
            pt->pos = bone->WorldXfm().v;
        }
        Point *backpt = &mPoints.back();
        backpt->length = len ? len : (pt ? pt->length : gUnitsPerMeter * 0.127f);
        ScaleAdd(
            backpt->bone->WorldXfm().v,
            backpt->bone->WorldXfm().m.y,
            backpt->length,
            backpt->pos
        );
    }
}

#pragma endregion CharHair::Strand
#pragma region CharHair

CharHair::CharHair()
    : mStiffness(0.04), mTorsion(0.1), mInertia(0.7), mGravity(1), mWeight(0.5),
      mFriction(0.3), mWind(1), mFlat(1), mMinSlack(0), mMaxSlack(0), mStrands(this),
      mReset(1), mSimulate(1), mUsePostProc(1), mWindObj(this), mCollides(this),
      mManagedHookup(0) {}

CharHair::~CharHair() {}

BEGIN_HANDLERS(CharHair)
    HANDLE_ACTION(reset, mReset = _msg->Int(2))
    HANDLE_ACTION(hookup, Hookup())
    HANDLE_ACTION(set_cloth, SetCloth(_msg->Int(2)))
    HANDLE_ACTION(freeze_pose, FreezePose())
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharHair)
    gHair = this;
    SYNC_PROP(stiffness, mStiffness)
    SYNC_PROP(torsion, mTorsion)
    SYNC_PROP(inertia, mInertia)
    SYNC_PROP(gravity, mGravity)
    SYNC_PROP(weight, mWeight)
    SYNC_PROP(friction, mFriction)
    SYNC_PROP(wind_obj, mWindObj)
    SYNC_PROP(wind, mWind)
    SYNC_PROP(flat, mFlat)
    SYNC_PROP(strands, mStrands)
    SYNC_PROP(simulate, mSimulate)
    SYNC_PROP(min_slack, mMinSlack)
    SYNC_PROP(max_slack, mMaxSlack)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharHair)
    SAVE_REVS(0xD, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mStiffness;
    bs << mTorsion;
    bs << mInertia;
    bs << mGravity;
    bs << mWeight;
    bs << mFriction;
    bs << mMinSlack;
    bs << mMaxSlack;
    bs << mStrands;
    bs << mSimulate;
    bs << mWindObj;
    bs << mWind;
    bs << mFlat;
END_SAVES

BEGIN_COPYS(CharHair)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharHair)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mStiffness)
        COPY_MEMBER(mInertia)
        COPY_MEMBER(mGravity)
        COPY_MEMBER(mWeight)
        COPY_MEMBER(mFriction)
        COPY_MEMBER(mTorsion)
        COPY_MEMBER(mStrands)
        COPY_MEMBER(mSimulate)
        COPY_MEMBER(mMinSlack)
        COPY_MEMBER(mMaxSlack)
        COPY_MEMBER(mWindObj)
        COPY_MEMBER(mWind)
        COPY_MEMBER(mFlat)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(13, 0)

BEGIN_LOADS(CharHair)
    LOAD_REVS(bs);
    ASSERT_REVS(13, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mStiffness >> mTorsion >> mInertia >> mGravity >> mWeight >> mFriction;
    if (d.rev < 8) {
        mMinSlack = 0.0f;
        mMaxSlack = 0.0f;
    } else {
        d >> mMinSlack >> mMaxSlack;
    }
    d >> mStrands;
    d >> mSimulate;
    if (d.rev > 10) {
        d >> mWindObj;
    }
    if (d.rev > 11) {
        d >> mWind;
    }
    if (d.rev > 12) {
        d >> mFlat;
    }
END_LOADS

void CharHair::SetName(const char *name, ObjectDir *dir) {
    Hmx::Object::SetName(name, dir);
    mUsePostProc = dynamic_cast<Character *>(dir) || dynamic_cast<WorldDir *>(dir);
}

void CharHair::Poll() {
    Character *cur = Character::Current();
    if (cur) {
        if (cur->Synced()) {
            Hookup();
        }
        if (cur->Teleported()) {
            mReset = 1;
        }
        if (cur->LODCheck()) {
            DoReset(0);
            return;
        }
    }
    if (mReset > 0) {
        DoReset(mReset);
    }
    if (TheTaskMgr.DeltaSeconds() != 0) {
        SimulateLoops(1, GetFPS());
    } else {
        SimulateZeroTime();
    }
}

void CharHair::Enter() {
    mReset = 1;
    RndPollable::Enter();
    Hookup();
}

void CharHair::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    for (int i = 0; i < mStrands.size(); i++) {
        changedBy.push_back(mStrands[i].Root());
        change.push_back(mStrands[i].Root());
    }
}

void CharHair::SetCloth(bool b) {
    for (int i = 0; i < mStrands.size(); i++) {
        Strand &strand = mStrands[i];
        Strand &modidx = mStrands[Mod(i + 1, mStrands.size())];
        for (int j = 0; j < strand.NumPoints(); j++) {
            Point &point = strand.PointAt(j);
            point.sideLength = b && j < modidx.NumPoints()
                ? Distance(point.pos, modidx.PointAt(j).pos)
                : -1.0f;
        }
    }
}

void CharHair::Hookup() {
    if (!mManagedHookup) {
        ObjPtrList<CharCollide> list(this);
        for (ObjDirItr<CharCollide> it(Dir(), true); it != nullptr; ++it) {
            list.push_back(it);
        }
        list.sort(SortCollides());
        Hookup(list);
    }
}

void CharHair::FreezePose() {
    bool oldSim = mSimulate;
    Hookup();
    SimulateLoops(200, 60);
    mSimulate = oldSim;
    FreezePoseRaw();
}

#pragma endregion CharHair
