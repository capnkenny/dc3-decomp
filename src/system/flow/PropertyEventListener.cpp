#include "flow/PropertyEventListener.h"
#include "flow/FlowNode.h"
#include "flow/DrivenPropertyEntry.h"
#include "obj/Object.h"

PropertyEventListener::PropertyEventListener(Hmx::Object *owner)
    : mAutoPropEntries(owner), mEventsRegistered(0) {}

void PropertyEventListener::RegisterEvents(FlowNode *node) {
    static Symbol reactivate("reactivate");
    if (!mEventsRegistered) {
        if (!mAutoPropEntries.empty()) {
            FOREACH (it, mAutoPropEntries) {
                if (it->unk4) {
                    it->unk4->AddPropertySink(node, it->unk0, reactivate);
                }
            }
        }
    }
    mEventsRegistered = true;
}

void PropertyEventListener::UnregisterEvents(FlowNode *node) {
    if (mEventsRegistered) {
        if (!mAutoPropEntries.empty()) {
            FOREACH (it, mAutoPropEntries) {
                if (it->unk4) {
                    it->unk4->RemovePropertySink(node, it->unk0);
                }
            }
        }
    }
    mEventsRegistered = false;
}

void PropertyEventListener::GenerateAutoNames(FlowNode *node, bool clear) {
    if (clear) {
        mAutoPropEntries.clear();
    }
    FOREACH (it, node->DrivenPropEntries()) {
        FOREACH (op, it->MathOps()) {
            AutoPropEntry entry(node);
            mAutoPropEntries.push_back(entry);
        }
    }
}
