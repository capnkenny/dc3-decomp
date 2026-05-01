#pragma once
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "flow/PropertyEventListener.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "utl/PoolAlloc.h"

class PropertyTask : public Task {
public:
    PropertyTask(
        Hmx::Object *,
        DataNode &,
        DataNode &,
        TaskUnits,
        float,
        EaseType,
        float,
        bool,
        Hmx::Object *
    );
    virtual ~PropertyTask();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(PropertyTask)
    virtual void Poll(float);

    POOL_OVERLOAD(PropertyTask, 0x17)

protected:
    void SetProperty(DataNode &);

    ObjOwnerPtr<Hmx::Object> unk2c; // 0x2c
    DataNode unk40; // 0x40
    DataNode unk48; // 0x48
    DataNode unk50; // 0x50
    float unk58;
    float unk5c;
    bool unk60;
    ObjPtr<Hmx::Object> unk64; // 0x64
    DataType unk78; // 0x78
    EaseFunc *mEaseFunc; // 0x7c
};

class FlowSetProperty : public FlowNode, public PropertyEventListener {
public:
    virtual ~FlowSetProperty();
    virtual bool Replace(ObjRef *from, Hmx::Object *to);
    OBJ_CLASSNAME(FlowSetProperty)
    OBJ_SET_TYPE(FlowSetProperty)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);

    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState);
    virtual bool IsRunning(void);
    virtual void MiloPreRun();
    virtual void MoveIntoDir(ObjectDir *, ObjectDir *);
    virtual void UpdateIntensity(void);

    void ReActivate(void);

    OBJ_MEM_OVERLOAD(0x20)
    NEW_OBJ(FlowSetProperty)

protected:
    FlowSetProperty();

    u32 unk_0x74; // might be fake.
    FlowPtr<Hmx::Object> mTarget; // 0x78
    DataNode mPropPath; // 0x98
    DataNodeObjTrack mValue; // 0xA0
    bool mPersistent; // 0xBC
    RndAnimatable::Rate mRate; // 0xC0
    f32 mBlendTime; // 0xC4
    f32 mChangePerUnit; // 0xC8
    ObjOwnerPtr<Task> unk_0xCC;
    EaseType mEase; // 0xE0
    f32 mEasePower; // 0xE4
    bool unk_0xE8;
    StopMode mStopMode; // 0xEC

    void OnTargetChanged(void);
    void OnAnimEvent(Symbol);
    bool IsBlendable(void);
};
