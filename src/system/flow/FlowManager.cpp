#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "os/Timer.h"
#include "rndobj/Overlay.h"
#include "utl/MakeString.h"

FlowManager *TheFlowMgr;

FlowManager::FlowManager() : unk2c(0), unk2d(0), mPollables(this) {
    mFlowQueue.clear();
    unk94 = 0;
    unk7c = 0;
    unk80 = 0;
    unk18c = 0;
    unk190 = 0;
    for (int i = 0; i < 60; i++) {
        unk98[i] = 0;
    }
    mFlowOverlay = RndOverlay::Find("flow", false);
    mFlowPeakOverlay = RndOverlay::Find("flow_peak", false);
    mFlowTaskOverlay = RndOverlay::Find("flow_task", false);
    mFlowEventOverlay = RndOverlay::Find("flow_event", false);
}

FlowManager::~FlowManager() {}

void FlowManager::AddPollable(FlowNode *n) { mPollables.push_back(n); }
void FlowManager::RemovePollable(FlowNode *n) { mPollables.remove(n); }

void FlowManager::QueueCommand(FlowNode *n, FlowNode::QueueState q) {
    if (unk2d && q != FlowNode::kQueueOne) {
        n->Execute(q);
    } else
        mFlowQueue[n] = q;
}

void FlowManager::CancelCommand(FlowNode *n) { mFlowQueue[n] = FlowNode::kImmediate; }

void FlowManager::AddEventTime(Symbol s, float f1) {
    float fsub = f1 - unk190;
    if (unk64.find(s) != unk64.end()) {
        DataNode &n = unk64[s];
        float f7 = n.Array()->Float(0);
        int i5 = n.Array()->Int(1);
        float f8 = n.Array()->Float(2) + unk190;
        i5++;
        n.Array()->Node(0) = f7 + fsub;
        n.Array()->Node(1) = i5;
        n.Array()->Node(2) = f8;
    } else {
        DataArrayPtr ptr(fsub, 1, unk190);
        unk64[s] = ptr;
    }
    unk190 = 0;
}

void FlowManager::Poll() {
    float f17 = 0;
    unk18c = 0;
    Timer timer;
    timer.Reset();
    timer.Start();
    unk2d = true;
    FOREACH (it, mFlowQueue) {
        if (it->second != FlowNode::kImmediate) {
            it->first->Execute(it->second);
        }
    }
    mFlowQueue.clear();
    ObjPtrVec<FlowNode> polls(mPollables);
    FOREACH (it, polls) {
        (*it)->Execute(FlowNode::kWhenAble);
    }
    unk2d = false;
    timer.Stop();
    unk2c = false;
    float timerMs = timer.Ms();
    Symbol s114 = 0;
    Symbol s11c = 0;
    float f18 = -1;
    float f19 = -1;
    if (unk64.size() != 0) {
        if (mFlowEventOverlay->Showing()) {
            *mFlowEventOverlay << "\n\n\n\n\n\n\n\n\n\n";
        }
        FOREACH (it, unk64) {
            Symbol key = it->first;
            DataNode value = it->second;
            float f14 = value.Array()->Float(0);
            float f20 = value.Array()->Float(2);
            f17 += f14;
            if (f14 >= f18) {
                f18 = f14;
                s114 = key;
            }
            if (f20 >= f19) {
                f19 = f20;
                s11c = key;
            }
            if (mFlowEventOverlay->Showing()) {
                float f134 = value.Array()->Float(2);
                float f138 = value.Array()->Float(0);
                int f13c = value.Array()->Int(1);
                *mFlowEventOverlay << MakeString(
                    "%s    count: %i   time: %.3f ms   task: %.3f ms\n",
                    key.Str(),
                    f13c,
                    f138,
                    f134
                );
            }
        }
        if (mFlowOverlay->Showing()) {
            MILO_LOG(
                "Worst:   FlowTime: %s  %.3f    TaskTime: %s  %.3f\n",
                s114.Str(),
                f18,
                s11c.Str(),
                f19
            );
        }
    }
    f17 = (timerMs + f17) + unk7c;
    if (mFlowOverlay->Showing()) {
        MILO_LOG(
            "Events: %.3f ms  %i Commands in %.3f ms  Release: %.3f ms  Tasks: %.3f ms\n",
            f17,
            (int)mFlowQueue.size()
        );
    }
    unk98[unk94] = f17;
    unk94++;
    if (f17 > unk80) {
        unk80 = f17;
        DataArrayPtr ptr(s114, f18, s11c, f19, f17);
        unk194 = ptr;
    }
    if (unk94 >= 60) {
        unk188 = 0;
        for (int i = 0; i < 60; i++) {
            unk188 += unk98[i];
        }
        unk80 = 0;
        unk94 = 0;
        if (unk194.Type() == kDataArray) {
            DataArray *a = unk194.Array();
            if (a->Float(1) > 0 && mFlowPeakOverlay->Showing()) {
                *mFlowPeakOverlay << MakeString("%s %.3f ms\n", a->Sym(0), a->Float(1));
            }
            if (a->Float(3) > 0 && mFlowTaskOverlay->Showing()) {
                *mFlowTaskOverlay << MakeString("%s %.3f ms\n", a->Sym(2), a->Float(3));
            }
        }
    }
    if (mFlowOverlay->Showing()) {
        *mFlowOverlay << MakeString(
            "Average: %.3f ms   Peak: %.3f ms    Frame: %.3f ms\n", unk188, 0, f17
        );
    }
    Timer *t = AutoTimer::GetTimer("flow");
    if (t) {
        t->SetLastMs(f17);
    }
    unk64.clear();
    unk7c = 0;
    unk18c = 0;
}
