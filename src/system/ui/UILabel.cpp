#include "ui/UILabel.h"

#include "macros.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "obj/PropSync_p.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Text.h"
#include "rndobj/Trans.h"
#include "ui/UI.h"
#include "ui/UIComponent.h"
#include "ui/UILabelDir.h"
#include "ui/UIListWidget.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Locale.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <cstring>

bool UILabel::sDeferUpdate = false;
UILabel *gMe = nullptr;

float GetTextSizeFromPctHeight(float);
float GetPctHeightFromTextSize(float);

UILabel::UILabel() : unk122(1), unk124(this) {
    unk124.resize(1);
    unk120 = 0;
    unk121 = false;
}

BEGIN_HANDLERS(UILabel)
    HANDLE(set_token_fmt, OnSetTokenFmt)
    HANDLE(set_prelocalized_string, OnSetPrelocalizedString)
    HANDLE(set_int, OnSetInt)
    HANDLE_ACTION(set_float, SetFloat(_msg->Str(2), _msg->Float(3)))
    HANDLE(set_time_hms, OnSetTimeHMS)
    HANDLE_ACTION(
        center_with_label,
        CenterWithLabel(_msg->Obj<UILabel>(2), _msg->Int(3), _msg->Float(4))
    )
    HANDLE_EXPR(get_font_mats, UILabelDir::GetMatVariations(LStyle(_msg->Int(2)).unk14))
    HANDLE(set_height_from_text, OnSetHeightFromText)
    HANDLE_EXPR(draw_rect_width, unkbc)
    HANDLE_ACTION(reload_string, (SetTextToken(mTextToken), unk122 = true))
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(UILabel::LabelStyle)
    int idx = (&o - &gMe->LStyle(0));
    SYNC_PROP_MODIFY(font_resource, o.unk14, gMe->RefreshFontMat(idx))
    SYNC_PROP(color_override, o.mColorOverride)
    SYNC_PROP_SET(
        font_mat_variation, gMe->GetFontMat(idx), gMe->SetFontMat(_val.Str(), idx);
        if (!UILabel::sDeferUpdate) { gMe->LabelUpdate(false); }
    )
    RndText::Style &textStyle = gMe->Style(idx);
    SYNC_PROP_SET(
        text_size,
        GetPctHeightFromTextSize(textStyle.mSize),
        textStyle.mSize = GetTextSizeFromPctHeight(_val.Float());
        if (!UILabel::sDeferUpdate) { gMe->LabelUpdate(false); }
    )
    SYNC_PROP_SET(font_alpha, textStyle.GetAlpha(), textStyle.SetAlpha(_val.Float()))
    SYNC_PROP_MODIFY(
        italics, textStyle.mItalics, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
    SYNC_PROP_MODIFY(
        kerning, textStyle.mKerning, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
    SYNC_PROP_MODIFY(
        z_offset, textStyle.mZOffset, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
    SYNC_PROP_MODIFY(
        blacklight, textStyle.mBlacklight, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
END_CUSTOM_PROPSYNC

bool PropSync(
    ObjVector<UILabel::LabelStyle> &v, DataNode &val, DataArray *prop, int i, PropOp op
) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize, 0x4A9);
        val = (int)v.size();
        return true;
    } else {
        int idx = prop->Int(i++);
        MILO_ASSERT(v.size() == gMe->Styles().size(), 0x4B0);
        auto labelIt = v.begin() + idx;
        auto stylesIt = gMe->Styles().begin() + idx;
        if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
            return PropSync(*labelIt, val, prop, i, op);
        } else if (op == kPropRemove) {
            if (v.size() > 1) {
                v.erase(labelIt);
                gMe->Styles().erase(stylesIt);
            }
            return true;
        } else if (op == kPropInsert) {
            UILabel::LabelStyle labelStyle(v.Owner());
            if (PropSync(labelStyle, val, prop, i, op)) {
                if (v.size() < 8) {
                    v.insert(labelIt, labelStyle);
                    RndText::Style textStyle = gMe->Styles().Owner();
                    gMe->Styles().insert(stylesIt, textStyle);
                }
                return true;
            }
        }
        return false;
    }
}

BEGIN_PROPSYNCS(UILabel)
    SYNC_PROP_SET(text_token, TextToken(), SetTextToken(_val.ForceSym()))
    SYNC_PROP_SET(icon, &unk120, SetIcon(_val.Str(0)[0]))
    SYNC_PROP_SET(edit_text, unk118.c_str(), SetEditText(_val.Str()))
    SYNC_PROP_MODIFY(width, mWidth, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(height, mHeight, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(circle, mCircle, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(alignment, (int &)mAlign, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(fit_type, (int &)mFitType, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(caps_mode, (int &)mCapsMode, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(markup, mMarkup, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(scroll_delay, mScrollDelay, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(scroll_rate, mScrollRate, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(scroll_pause, mScrollPause, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(leading, mLeading, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(indentation, mIndentation, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(basic_markup, mBasicMarkup, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_SET(
        fixed_length, mFixedLength, SetFixedLength(_val.Int());
        if (!sDeferUpdate) LabelUpdate(false)
    )
    SYNC_PROP(draw_width, unkbc)
    gMe = this;
    SYNC_PROP(styles, unk124)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

// BEGIN_SAVES(UILabel)
// END_SAVES

BEGIN_COPYS(UILabel)
    COPY_SUPERCLASS(UIComponent)
    COPY_SUPERCLASS(RndText)
    CREATE_COPY(UILabel)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTextToken)
        COPY_MEMBER(unk118)
        // looks like an strcpy here
    END_COPYING_MEMBERS
    if (sDeferUpdate == false) {
        LabelUpdate(false);
    }
END_COPYS

void UILabel::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

// void UILabel::PreLoad(BinStream &) {}

// void UILabel::PostLoad(BinStream &bs) {}

Symbol UILabel::TextToken() { return mTextToken; }

// void UILabel::Poll() {}

// void UILabel::Highlight() {}

// void UILabel::DrawShowing() {}

void UILabel::SetTextToken(Symbol s) {
    mTextToken = s;

    SetTokenFmtImp(mTextToken, 0, 0, 0, true);
}

void UILabel::SetInt(int i, bool b) {
    if (b) {
        SetDisplayText(LocalizeSeparatedInt(i, TheLocale), true);
    } else
        SetDisplayText(MakeString("%d", i), true);
}

void UILabel::SetFloat(const char *cc, float f) {
    SetDisplayText(LocalizeFloat(cc, f), true);
}

void UILabel::SetDateTime(DateTime const &dt, Symbol s) {
    String str(Localize(s, false, TheLocale));
    dt.Format(str);
    SetDisplayText(str.c_str(), true);
}

// void UILabel::SetIcon(char c) {
//     unk120 = c;
//     if (c == '\0' && TheLoadMgr.EditMode() != 0) {
//         SetEditText(unk118.c_str());
//         return;
//     }
// }

// void UILabel::SetTokenFmt(const DataArray *) {}

// RndText::Style &UILabel::Style(int) { return Style(0); }

// void UILabel::SetPrelocalizedString(String &s) {}

// void UILabel::SetSubtitle(const DataArray *) {}

// void UILabel::SetTimeHMS(int, bool) {}

// bool UILabel::CheckValid(bool) { return false; }

// void UILabel::SetEditText(const char *c) {}

char const *UILabel::GetDefaultText() const {
    if (unk120 != 0) {
        return &unk120;
    }

    if (TheLoadMgr.EditMode() && !unk118.empty())
        return unk118.c_str();
    else
        return Localize(mTextToken, nullptr, TheLocale);
}

// void UILabel::CenterWithLabel(UILabel *, bool, float) {}

// UILabel::LabelStyle &UILabel::LStyle(int) { return new LabelStyle(0); }

// void UILabel::OldResourcePreload(BinStream &) {}

void UILabel::SetDisplayText(const char *cc, bool b) {
    if (b)
        mTextToken = gNullStr;
    RndText::SetText(cc);
    if (strchr(cc, 60)) {
        mMarkup = true;
    }
    if (!sDeferUpdate)
        LabelUpdate(false);
}

void UILabel::Init() {
    REGISTER_OBJ_FACTORY(UILabel);
    UILabelDir::Init();
}

void UILabel::SetTokenFmtImp(
    Symbol s, const DataArray *da1, const DataArray *da2, int i, bool b
) {}

// DataNode UILabel::OnSetPrelocalizedString(DataArray const *da) { return NULL_OBJ; }

// DataNode UILabel::OnSetTokenFmt(DataArray const *da) { return NULL_OBJ; }

// DataNode UILabel::OnSetInt(DataArray const *da) { return DataNode(1); }

// DataNode UILabel::OnSetTimeHMS(DataArray const *) { return NULL_OBJ; }

// bool UILabel::AllowEditText() const { return false; }

// void UILabel::LabelUpdate(bool b) { unk122 = false; }

// DataNode UILabel::OnSetHeightFromText(DataArray *) { return NULL_OBJ; }

// void UILabel::SetFontMat(char const *c, int i) {
//     RndMat *rndmat = nullptr;
//     auto labelStyle = LStyle(i);
// }

// char const *UILabel::GetFontMat(int) { return 0; }

void UILabel::RefreshFontMat(int i) {
    auto mat = GetFontMat(i);
    SetFontMat(mat, i);
    if (sDeferUpdate == false) {
        LabelUpdate(false);
    }
}
