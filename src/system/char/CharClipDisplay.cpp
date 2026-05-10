#include "char/CharClipDisplay.h"
#include "char/CharBones.h"
#include "char/CharIKFoot.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Utl.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"

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

void CharClipDisplay::DrawTrack() {
    Hmx::Color white(1, 1, 1);
    Hmx::Color green(0, 1, 0);
    Hmx::Color black(0, 0, 0);
    Hmx::Color red(1, 0, 0);
    float unk18Proxy = unk18;
    float f19 = -(sEm / 2 - unk18Proxy);
    float start = Max(unkc, unk4);
    float end = Min(unk10, unk8);
    Hmx::Rect firstRect;
    firstRect.x = GetX(start);
    firstRect.y = unk18;
    firstRect.w = GetX(end) - firstRect.x;
    firstRect.h = 3;
    TheRnd.DrawRect(firstRect, white, nullptr, nullptr, nullptr);
    float f18 = ceilf(start / 1.0f) * 1.0f;
    float f17 = floorf(end / 1.0f) * 1.0f;
    float f11 = f18;
    if (f18 + 1 != f18) {
        for (; f18 <= f17; f18 += 1) {
            Hmx::Rect r;
            r.y = unk18Proxy - 3;
            r.h = 9;
            r.x = GetX(f18);
            r.w = 1;
            TheRnd.DrawRect(r, green, nullptr, nullptr, nullptr);
        }
    }
    if (mClip) {
        bool b1 = true;
        auto &beatEvents = mClip->BeatEvents();
        for (int i = 0; i < beatEvents.size(); i++) {
            auto &cur = beatEvents[i];
            float xBeat = GetX(cur.beat);
            float emdiv = sEm / 2;
            TheRnd.DrawRect(
                Hmx::Rect(xBeat, unk18Proxy - emdiv, 1, 0.2f),
                Hmx::Color(0.2f, 0.2f, 1),
                nullptr,
                nullptr,
                nullptr
            );
            if (b1
                && (cur.beat > unk1c
                    || (i == 0 && unk1c > mClip->BeatEvents().back().beat))) {
                b1 = false;
                TheRnd.DrawString(
                    cur.event.Str(),
                    Vector2(xBeat, unk18Proxy - (10.0f + emdiv)),
                    Hmx::Color(0.2f, 0.2f, 1),
                    true
                );
            }
        }
        CharIKFoot *leftIk = sDir->Find<CharIKFoot>("left.ikfoot", false);
        CharIKFoot *rightIk = sDir->Find<CharIKFoot>("right.ikfoot", false);
        if (!leftIk && !rightIk) {
            float fx;
            Hmx::Rect r(0, unk18Proxy + 1, 1, 1);
            int startSample = mClip->BeatToSample(start, &fx);
            int endSample = mClip->BeatToSample(end, &fx);
            for (; startSample <= endSample; startSample++) {
                r.x = GetX(mClip->SampleToBeat(startSample));
                TheRnd.DrawRect(r, black, nullptr, nullptr, nullptr);
            }
        } else {
            MILO_ASSERT(!rightIk || !leftIk || (rightIk->GetData() == leftIk->GetData()), 0xD1);
            RndTransformable *data = leftIk ? leftIk->GetData() : rightIk->GetData();
            if (data) {
                Symbol channelName =
                    CharBones::ChannelName(data->Name(), CharBones::TYPE_POS);
                void *channel = mClip->GetChannel(channelName);
                float fdata[4];
                mClip->EvaluateChannel(fdata, channel, unk1c);
                Hmx::Color yellow(1, 1, 0);
                float x1c = GetX(unk1c);
                if (leftIk) {
                    const char *dataStr =
                        MakeString("L: %.1f", fdata[leftIk->DataIndex()]);
                    TheRnd.DrawString(
                        dataStr, Vector2(x1c - 90, unk18 + 10), yellow, true
                    );
                }
                if (rightIk) {
                    const char *dataStr =
                        MakeString("R: %.1f", fdata[rightIk->DataIndex()]);
                    TheRnd.DrawString(
                        dataStr, Vector2(x1c - 40, unk18 + 10), yellow, true
                    );
                }
            }
        }
        DrawBeatString(f11, green);
        DrawBeatString(end, green);
        TheRnd.DrawString(
            MakeString("%.1f", unk4),
            Vector2(-(sEm * 2 - (sEm * 3 + unk64 + unk14)), f19),
            white,
            true
        );
        TheRnd.DrawString(
            MakeString("%.1f", unk8),
            Vector2(-(sEm * 3 - (float)TheRnd.Width()), f19),
            white,
            true
        );
    }
    TheRnd.DrawString(mText, Vector2(sEm + unk64, f19), Hmx::Color(1, 1, 1), true);
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
