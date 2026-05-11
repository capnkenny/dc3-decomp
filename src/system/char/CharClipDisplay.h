#pragma once
#include "char/CharClip.h"
#include "math/Color.h"
#include "obj/Dir.h"
#include "obj/Object.h"

struct CharClipDisplay {
public:
    CharClipDisplay()
        : mClip(0), unk4(0), unk8(0), unkc(0), unk10(0), unk14(0), unk18(0), unk1c(0),
          unk20(0), unk64(0) {}

    void SetText(char const *);
    float GetX(float) const;
    void GetXY(Vector2 &, float) const;
    void SetStartEnd(float, float, bool);
    void DrawBlend(float, float);
    void DrawBeatString(const char *, float, const Hmx::Color &);
    void DrawCursor();
    void SetClip(CharClip *, bool);
    void DrawBeatString(float, const Hmx::Color &);
    void DrawTrack();

    /** "Zoom value for the highlight display" */
    static float sZoom;
    static float GetSEm() { return sEm; }
    static float LineSpacing();
    static void Init(ObjectDir *);
    static Hmx::Object *FindSource(Hmx::Object *);

    CharClip *mClip; // 0x0
    float unk4;
    float unk8;
    float unkc;
    float unk10;
    float unk14;
    float unk18;
    float unk1c;
    float unk20;
    char mText[64]; // 0x24
    float unk64;

protected:
    static float sEm;
    static ObjectDir *sDir;
};
