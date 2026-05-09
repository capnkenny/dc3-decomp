#include "char/CharTransDraw.h"
#include "char/Character.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "utl/Std.h"

CharTransDraw::CharTransDraw() : mChars(this), mForceDraw(false) {}

CharTransDraw::~CharTransDraw() { SetDrawModes(Character::kCharDrawAll); }

void CharTransDraw::SetDrawModes(Character::DrawMode mode) {
    FOREACH (it, mChars) {
        (*it)->SetDrawMode(mode);
    }
}

BEGIN_PROPSYNCS(CharTransDraw)
    SYNC_PROP(chars, mChars)
    SYNC_PROP(force_draw, mForceDraw)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharTransDraw)
    SAVE_REVS(2, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mChars;
    bs << mForceDraw;
END_SAVES

BEGIN_COPYS(CharTransDraw)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(CharTransDraw)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mChars)
        COPY_MEMBER(mForceDraw)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 1)

BEGIN_LOADS(CharTransDraw)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 1)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mChars;
    if (d.altRev > 0) {
        d >> mForceDraw;
    }
    SetDrawModes(Character::kCharDrawOpaque);
END_LOADS

void CharTransDraw::DrawShowing() {
    FOREACH (it, mChars) {
        Character *cur = *it;
        if (cur->Showing()) {
            cur->SetDrawMode(Character::kCharDrawTranslucent);
            cur->Draw();
            cur->SetDrawMode(Character::kCharDrawOpaque);
        } else if (mForceDraw) {
            cur->SetDrawMode(Character::kCharDrawTranslucent);
            cur->SetShowing(true);
            cur->Draw();
            cur->SetShowing(false);
            cur->SetDrawMode(Character::kCharDrawOpaque);
        }
    }
}

BEGIN_HANDLERS(CharTransDraw)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
