/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_LAYOUT_PROPERTY_H

#include "core/common/ime/text_input_type.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/property/property.h"
#include "core/image/image_source_info.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT TextFieldLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(TextFieldLayoutProperty, LayoutProperty);

public:
    TextFieldLayoutProperty() = default;

    ~TextFieldLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<TextFieldLayoutProperty>();
        Clone(value);
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetFontStyle();
        ResetTextLineStyle();
        ResetValue();
        ResetPlaceholderFontStyle();
        ResetPlaceholderTextLineStyle();
        ResetPlaceholder();
        ResetTextInputType();
        ResetInputFilter();
        ResetShowPasswordIcon();
        ResetCopyOptions();
        ResetLastValue();
        ResetNeedFireOnChange();
        ResetWidthAuto();
        ResetErrorText();
        ResetShowErrorText();
        ResetShowCounter();
        ResetShowUnderline();
        ResetShowPasswordSourceInfo();
        ResetHidePasswordSourceInfo();
        ResetTextAlignChanged();
        ResetDisplayMode();
        ResetMaxViewLines();
        ResetSelectionMenuHidden();
        ResetPasswordRules();
        ResetEnableAutoFill();
        ResetCleanNodeStyle();
        ResetIconSize();
        ResetIconSrc();
        ResetIconColor();
        ResetSelectAllValue();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        LayoutProperty::ToJsonValue(json);
        json->Put("showPasswordIcon", propShowPasswordIcon_.value_or(true));
        json->Put("errorText", propErrorText_.value_or("").c_str());
        json->Put("showErrorText", propShowErrorText_.value_or(false));
        json->Put("showCounter", propShowCounter_.value_or(false));
        json->Put("showUnderline", propShowUnderline_.value_or(false));
        auto jsonCancelButton = JsonUtil::Create(true);
        jsonCancelButton->Put("style", static_cast<int32_t>(propCleanNodeStyle_.value_or(CleanNodeStyle::INPUT)));
        auto jsonIconOptions = JsonUtil::Create(true);
        jsonIconOptions->Put("size", propIconSize_.value_or(Dimension()).ToString().c_str());
        jsonIconOptions->Put("src", propIconSrc_.value_or("").c_str());
        jsonIconOptions->Put("color", propIconColor_.value_or(Color()).ColorToString().c_str());
        jsonCancelButton->Put("icon", jsonIconOptions->ToString().c_str());
        json->Put("cancelButton", jsonCancelButton->ToString().c_str());
        json->Put("selectAll", propSelectAllValue_.value_or(false));
    }

    ACE_DEFINE_PROPERTY_GROUP(FontStyle, FontStyle);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(FontStyle, FontSize, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(FontStyle, TextColor, Color, PROPERTY_UPDATE_MEASURE_SELF);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(FontStyle, ItalicFontStyle, Ace::FontStyle, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(FontStyle, FontWeight, FontWeight, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(FontStyle, FontFamily, std::vector<std::string>, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_GROUP(TextLineStyle, TextLineStyle);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(TextLineStyle, TextAlign, TextAlign, PROPERTY_UPDATE_MEASURE_SELF);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(TextLineStyle, MaxLength, uint32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(TextLineStyle, MaxLines, uint32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Value, std::string, PROPERTY_UPDATE_NORMAL);

    ACE_DEFINE_PROPERTY_GROUP(PlaceholderFontStyle, FontStyle);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderFontStyle, FontSize, PlaceholderFontSize, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderFontStyle, TextColor, PlaceholderTextColor, Color, PROPERTY_UPDATE_MEASURE_SELF);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderFontStyle, ItalicFontStyle, PlaceholderItalicFontStyle, Ace::FontStyle, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderFontStyle, FontWeight, PlaceholderFontWeight, FontWeight, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderFontStyle, FontFamily, PlaceholderFontFamily, std::vector<std::string>, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_GROUP(PlaceholderTextLineStyle, TextLineStyle);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderTextLineStyle, LineHeight, PlaceholderLineHeight, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderTextLineStyle, TextAlign, PlaceholderTextAlign, TextAlign, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderTextLineStyle, MaxLength, PlaceholderMaxLength, uint32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        PlaceholderTextLineStyle, MaxLines, PlaceholderMaxLines, uint32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Placeholder, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ErrorText, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ShowErrorText, bool, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(TextInputType, TextInputType, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(InputFilter, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ShowPasswordIcon, bool, PROPERTY_UPDATE_MEASURE_SELF);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ShowCounter, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ShowUnderline, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(DisplayMode, DisplayMode, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(MaxViewLines, uint32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IsEnabled, bool, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(WidthAuto, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(TypeChanged, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(TextAlignChanged, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(CopyOptions, CopyOptions, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(PreferredNewLineHeightNeedToUpdate, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(PreferredTextLineHeightNeedToUpdate, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(PreferredPlaceholderLineHeightNeedToUpdate, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(LastValue, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(NeedFireOnChange, bool, PROPERTY_UPDATE_NORMAL);

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ShowPasswordSourceInfo, ImageSourceInfo, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(HidePasswordSourceInfo, ImageSourceInfo, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SelectionMenuHidden, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(PasswordRules, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(EnableAutoFill, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SelectAllValue, bool, PROPERTY_UPDATE_NORMAL);

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(CleanNodeStyle, CleanNodeStyle, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IconSize, CalcDimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IconSrc, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IconColor, Color, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SetCounter, int32_t, PROPERTY_UPDATE_MEASURE);

protected:
    void Clone(RefPtr<LayoutProperty> property) const override
    {
        auto value = DynamicCast<TextFieldLayoutProperty>(property);
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        value->propFontStyle_ = CloneFontStyle();
        value->propTextLineStyle_ = CloneTextLineStyle();
        value->propValue_ = CloneValue();
        value->propPlaceholderFontStyle_ = ClonePlaceholderFontStyle();
        value->propPlaceholderTextLineStyle_ = ClonePlaceholderTextLineStyle();
        value->propPlaceholder_ = ClonePlaceholder();
        value->propTextInputType_ = CloneTextInputType();
        value->propInputFilter_ = CloneInputFilter();
        value->propShowPasswordIcon_ = CloneShowPasswordIcon();
        value->propCopyOptions_ = CloneCopyOptions();
        value->propLastValue_ = CloneLastValue();
        value->propNeedFireOnChange_ = CloneNeedFireOnChange();
        value->propWidthAuto_ = CloneWidthAuto();
        value->propErrorText_ = CloneErrorText();
        value->propShowErrorText_ = CloneShowErrorText();
        value->propShowCounter_ = CloneShowCounter();
        value->propShowUnderline_ = CloneShowUnderline();
        value->propShowPasswordSourceInfo_ = CloneShowPasswordSourceInfo();
        value->propHidePasswordSourceInfo_ = CloneHidePasswordSourceInfo();
        value->propTextAlignChanged_ = CloneTextAlignChanged();
        value->propDisplayMode_ = CloneDisplayMode();
        value->propMaxViewLines_ = CloneMaxViewLines();
        value->propIsEnabled_ = CloneIsEnabled();
        value->propSelectionMenuHidden_ = CloneSelectionMenuHidden();
        value->propPasswordRules_ = ClonePasswordRules();
        value->propEnableAutoFill_ = CloneEnableAutoFill();
        value->propCleanNodeStyle_ = CloneCleanNodeStyle();
        value->propIconSize_ = CloneIconSize();
        value->propIconColor_ = CloneIconColor();
        value->propSelectAllValue_ = CloneSelectAllValue();
        value->propSetCounter_ = CloneSetCounter();
    }

    ACE_DISALLOW_COPY_AND_MOVE(TextFieldLayoutProperty);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_LAYOUT_PROPERTY_H
