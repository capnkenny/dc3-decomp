#include "char/CharClipDisplay.h"
#include "math/Geo.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"

float CharClipDisplay::sZoom = 1;
float CharClipDisplay::sEm;
ObjectDir *CharClipDisplay::sDir;

static const float sDisplayFloat = 2;

float CharClipDisplay::GetX(float f1) const {
    float f2;
    if (unk10 > unkc) {
        f2 = unk10 - unkc;
    } else {
        f2 = 1;
    }
    float f3 = unk64 + unk14 + sEm * 3.0f;
    return ((TheRnd.Width() - sEm * 3.0f) - f3) * ((f1 - unkc) / f2) + f3;
}

void CharClipDisplay::GetXY(Vector2 &v, float f1) const { v.Set(GetX(f1), unk18); }

void CharClipDisplay::DrawBeatString(char const *c, float f1, const Hmx::Color &color) {
    Vector2 xy;
    GetXY(xy, f1);
    TheRnd.DrawString(c, Vector2(xy.x - 4, xy.y - 18), color, true);
}

void CharClipDisplay::DrawBeatString(float f1, const Hmx::Color &c) {
    const char *str;
    if (f1 == floorf(f1)) {
        str = MakeString("%d", (int)f1);
    } else {
        str = MakeString("%.1f", f1);
    }
    DrawBeatString(str, f1, c);
}

void CharClipDisplay::DrawCursor() {
    Hmx::Color yellow(1, 1, 0);
    Vector2 v38;
    GetXY(v38, unk1c);
    TheRnd.DrawRect(Hmx::Rect(v38.x, v38.y - 3, 1, 9), yellow, nullptr, nullptr, nullptr);
    const char *str;
    if (unk20 < 1) {
        str = MakeString("%.1f (%.2f)", unk1c, unk20);
    } else {
        str = MakeString("%.1f", unk1c);
    }
    DrawBeatString(str, unk1c, yellow);
}

void CharClipDisplay::DrawBlend(float f1, float f2) {
    Hmx::Rect r(0, unk18 + 1, 0, sDisplayFloat);
    r.x = GetX(f1);
    float sub = GetX(f1 + f2) - r.x;
    Hmx::Color c(0, 0, 1, 0.4f);
    r.w = sub;
    TheRnd.DrawRect(r, c, nullptr, nullptr, nullptr);
    r.y = unk18 - 1;
    r.h = 4;
    r.w = 3;
    r.x = GetX(f2 / 2.0f + f1) - 1;
    TheRnd.DrawRect(r, Hmx::Color(0, 0, 1), nullptr, nullptr, nullptr);
}

void CharClipDisplay::SetStartEnd(float f1, float f2, bool b3) {
    unk4 = f1;
    unk8 = f2;
    unkc = f1;
    unk10 = f2;
    float div = 16.0f / sZoom;
    if (b3) {
        float em3 = sEm * 3.0f;
        float w = TheRnd.Width();
        float fvar1 = unk64 + unk14 + em3;
        unkc = unk1c - ((w / 2.0f - fvar1) * div) / w;
        unk10 = (((w - em3) - fvar1) * div) / TheRnd.Width() + unkc;
    } else if (f2 - f1 > div) {
        float fvar3 = div / 2;
        if (unk1c < fvar3 + f1) {
            unk10 = div + f1;
        } else {
            if (unk1c > f2 - fvar3) {
                unkc = f2 - div;
            } else {
                unkc = unk1c - fvar3;
                unk10 = fvar3 + unk1c;
            }
        }
    } else if (f2 == f1) {
        unkc = f1 - div / 2;
        unk10 = f2 + div / 2;
    }
}

void CharClipDisplay::SetClip(CharClip *clip, bool b) {
    mClip = clip;
    SetText(clip->Name());
    SetStartEnd(clip->StartBeat(), clip->EndBeat(), b);
}

void CharClipDisplay::SetText(const char *text) {
    strcpy(mText, text);
    unk14 = TheRnd.DrawString(text, Vector2(0, 0), Hmx::Color(1.0f, 0.0f, 0.0f), false).x
        + sEm;
}

float CharClipDisplay::LineSpacing() { return sEm * sDisplayFloat; }

void CharClipDisplay::Init(ObjectDir *dir) {
    sDir = dir;
    sEm = TheRnd.DrawString("", Vector2(0, 0), Hmx::Color(1.0f, 0.0f, 0.0f), false).y;
}

Hmx::Object *CharClipDisplay::FindSource(Hmx::Object *obj) {
    for (ObjDirItr<Hmx::Object> it(ObjectDir::Main(), false); it != nullptr; ++it) {
        MsgSinks *sinks = it->Sinks();
        if (sinks && sinks->HasSink(obj)) {
            return it;
        }
    }
    return nullptr;
}
