/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_select.h"

#include <cstdint>
#include <string>
#include <vector>

#include "base/log/ace_scoring_log.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "bridge/declarative_frontend/jsview/models/select_model_impl.h"
#include "core/components_ng/base/view_abstract_model.h"
#include "core/components_ng/pattern/select/select_model.h"
#include "core/components_ng/pattern/select/select_model_ng.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace {
std::unique_ptr<SelectModel> SelectModel::instance_ = nullptr;
std::mutex SelectModel::mutex_;

SelectModel* SelectModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::SelectModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::SelectModelNG());
            } else {
                instance_.reset(new Framework::SelectModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
void JSSelect::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 0) {
        return;
    }
    if (info[0]->IsArray()) {
        auto paramArray = JSRef<JSArray>::Cast(info[0]);
        size_t size = paramArray->Length();
        std::vector<SelectParam> params(size);
        for (size_t i = 0; i < size; i++) {
            std::string value;
            std::string icon;
            JSRef<JSVal> indexVal = paramArray->GetValueAt(i);
            if (!indexVal->IsObject()) {
                return;
            }
            auto indexObject = JSRef<JSObject>::Cast(indexVal);
            auto selectValue = indexObject->GetProperty("value");
            auto selectIcon = indexObject->GetProperty("icon");
            ParseJsString(selectValue, value);
            ParseJsMedia(selectIcon, icon);
            params[i] = { value, icon };
        }
        SelectModel::GetInstance()->Create(params);
    }
}

void JSSelect::JSBind(BindingTarget globalObj)
{
    JSClass<JSSelect>::Declare("Select");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSSelect>::StaticMethod("create", &JSSelect::Create, opt);

    JSClass<JSSelect>::StaticMethod("selected", &JSSelect::Selected, opt);
    JSClass<JSSelect>::StaticMethod("value", &JSSelect::Value, opt);
    JSClass<JSSelect>::StaticMethod("font", &JSSelect::Font, opt);
    JSClass<JSSelect>::StaticMethod("fontColor", &JSSelect::FontColor, opt);
    JSClass<JSSelect>::StaticMethod("selectedOptionBgColor", &JSSelect::SelectedOptionBgColor, opt);
    JSClass<JSSelect>::StaticMethod("selectedOptionFont", &JSSelect::SelectedOptionFont, opt);
    JSClass<JSSelect>::StaticMethod("selectedOptionFontColor", &JSSelect::SelectedOptionFontColor, opt);
    JSClass<JSSelect>::StaticMethod("optionBgColor", &JSSelect::OptionBgColor, opt);
    JSClass<JSSelect>::StaticMethod("optionFont", &JSSelect::OptionFont, opt);
    JSClass<JSSelect>::StaticMethod("optionFontColor", &JSSelect::OptionFontColor, opt);
    JSClass<JSSelect>::StaticMethod("onSelect", &JSSelect::OnSelected, opt);
    JSClass<JSSelect>::StaticMethod("space", &JSSelect::SetSpace, opt);
    JSClass<JSSelect>::StaticMethod("arrowPosition", &JSSelect::SetArrowPosition, opt);
    JSClass<JSSelect>::StaticMethod("menuAlign", &JSSelect::SetMenuAlign, opt);

    // API7 onSelected deprecated
    JSClass<JSSelect>::StaticMethod("onSelected", &JSSelect::OnSelected, opt);
    JSClass<JSSelect>::StaticMethod("width", &JSSelect::JsWidth);
    JSClass<JSSelect>::StaticMethod("height", &JSSelect::JsHeight);
    JSClass<JSSelect>::StaticMethod("size", &JSSelect::JsSize);
    JSClass<JSSelect>::StaticMethod("padding", &JSSelect::JsPadding);
    JSClass<JSSelect>::StaticMethod("paddingTop", &JSSelect::SetPaddingTop, opt);
    JSClass<JSSelect>::StaticMethod("paddingBottom", &JSSelect::SetPaddingBottom, opt);
    JSClass<JSSelect>::StaticMethod("paddingLeft", &JSSelect::SetPaddingLeft, opt);
    JSClass<JSSelect>::StaticMethod("paddingRight", &JSSelect::SetPaddingRight, opt);
    JSClass<JSSelect>::StaticMethod("optionWidth", &JSSelect::SetOptionWidth, opt);
    JSClass<JSSelect>::StaticMethod("optionHeight", &JSSelect::SetOptionHeight, opt);
    JSClass<JSSelect>::StaticMethod("optionWidthFitTrigger", &JSSelect::SetOptionWidthFitTrigger, opt);

    JSClass<JSSelect>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSSelect>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSSelect>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSSelect>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSSelect>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSSelect>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSSelect>::InheritAndBind<JSViewAbstract>(globalObj);
}

void ParseSelectedObject(const JSCallbackInfo& info, const JSRef<JSVal>& changeEventVal)
{
    CHECK_NULL_VOID(changeEventVal->IsFunction());

    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(changeEventVal));
    auto onSelect = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](int32_t index) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("Select.SelectChangeEvent");
        auto newJSVal = JSRef<JSVal>::Make(ToJSValue(index));
        func->ExecuteJS(1, &newJSVal);
    };
    SelectModel::GetInstance()->SetSelectChangeEvent(onSelect);
}

void JSSelect::Selected(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || info.Length() > 2) {
        return;
    }

    int32_t value = 0;
    if (info.Length() > 0) {
        ParseJsInteger<int32_t>(info[0], value);
    }

    if (value < -1) {
        value = -1;
    }
    if (info.Length() > 1 && info[1]->IsFunction()) {
        ParseSelectedObject(info, info[1]);
    }
    SelectModel::GetInstance()->SetSelected(value);
}

void ParseValueObject(const JSCallbackInfo& info, const JSRef<JSVal>& changeEventVal)
{
    CHECK_NULL_VOID(changeEventVal->IsFunction());

    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(changeEventVal));
    auto onSelect = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](const std::string& value) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("Select.ValueChangeEvent");
        auto newJSVal = JSRef<JSVal>::Make(ToJSValue(value));
        func->ExecuteJS(1, &newJSVal);
    };
    SelectModel::GetInstance()->SetValueChangeEvent(onSelect);
}

void JSSelect::Value(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || info.Length() > 2) {
        return;
    }

    std::string value;
    if (info.Length() > 0) {
        ParseJsString(info[0], value);
    }

    if (info.Length() > 1 && info[1]->IsFunction()) {
        ParseValueObject(info, info[1]);
    }
    SelectModel::GetInstance()->SetValue(value);
}

void JSSelect::Font(const JSCallbackInfo& info)
{
    if (info[0]->IsUndefined() || info[0]->IsNull()) {
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto selectTheme = pipeline->GetTheme<SelectTheme>();
        CHECK_NULL_VOID(selectTheme);
        auto textTheme = pipeline->GetTheme<TextTheme>();
        CHECK_NULL_VOID(textTheme);
        SelectModel::GetInstance()->SetFontSize(selectTheme->GetFontSize());
        SelectModel::GetInstance()->SetFontWeight(FontWeight::MEDIUM);
        SelectModel::GetInstance()->SetFontFamily(textTheme->GetTextStyle().GetFontFamilies());
        SelectModel::GetInstance()->SetItalicFontStyle(textTheme->GetTextStyle().GetFontStyle());
        return;
    }

    if (!info[0]->IsObject()) {
        return;
    }

    auto param = JSRef<JSObject>::Cast(info[0]);
    auto size = param->GetProperty("size");
    if (!size->IsNull()) {
        CalcDimension fontSize;
        if (ParseJsDimensionFp(size, fontSize)) {
            SelectModel::GetInstance()->SetFontSize(fontSize);
        }
    }
    std::string weight;
    auto fontWeight = param->GetProperty("weight");
    if (!fontWeight->IsNull()) {
        if (fontWeight->IsNumber()) {
            weight = std::to_string(fontWeight->ToNumber<int32_t>());
        } else {
            ParseJsString(fontWeight, weight);
        }
        SelectModel::GetInstance()->SetFontWeight(ConvertStrToFontWeight(weight));
    }

    auto family = param->GetProperty("family");
    if (!family->IsNull() && family->IsString()) {
        auto familyVal = family->ToString();
        SelectModel::GetInstance()->SetFontFamily(ConvertStrToFontFamilies(familyVal));
    }

    auto style = param->GetProperty("style");
    if (!style->IsNull() && style->IsNumber()) {
        auto styleVal = static_cast<FontStyle>(style->ToNumber<int32_t>());
        SelectModel::GetInstance()->SetItalicFontStyle(styleVal);
    }
}

void JSSelect::FontColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    Color textColor;
    if (!ParseJsColor(info[0], textColor)) {
        if (info[0]->IsNull() || info[0]->IsUndefined()) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SelectTheme>();
            CHECK_NULL_VOID(theme);
            textColor = theme->GetFontColor();
        } else {
            return;
        }
    }

    SelectModel::GetInstance()->SetFontColor(textColor);
}

void JSSelect::SelectedOptionBgColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    Color bgColor;
    if (!ParseJsColor(info[0], bgColor)) {
        if (info[0]->IsUndefined() || info[0]->IsNull()) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SelectTheme>();
            CHECK_NULL_VOID(theme);
            bgColor = theme->GetSelectedColor();
        } else {
            return;
        }
    }
    SelectModel::GetInstance()->SetSelectedOptionBgColor(bgColor);
}

void JSSelect::SelectedOptionFont(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto selectTheme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(selectTheme);
    if (info[0]->IsUndefined() || info[0]->IsNull()) {
        auto textTheme = pipeline->GetTheme<TextTheme>();
        CHECK_NULL_VOID(textTheme);
        SelectModel::GetInstance()->SetSelectedOptionFontSize(selectTheme->GetFontSize());
        SelectModel::GetInstance()->SetSelectedOptionFontWeight(textTheme->GetTextStyle().GetFontWeight());
        SelectModel::GetInstance()->SetSelectedOptionFontFamily(textTheme->GetTextStyle().GetFontFamilies());
        SelectModel::GetInstance()->SetSelectedOptionItalicFontStyle(textTheme->GetTextStyle().GetFontStyle());
        return;
    }
    if (!info[0]->IsObject()) {
        return;
    }
    auto param = JSRef<JSObject>::Cast(info[0]);
    auto size = param->GetProperty("size");
    if (!size->IsNull()) {
        CalcDimension fontSize;
        if (ParseJsDimensionFp(size, fontSize)) {
            SelectModel::GetInstance()->SetSelectedOptionFontSize(fontSize);
        } else if (size->IsUndefined()) {
            SelectModel::GetInstance()->SetSelectedOptionFontSize(selectTheme->GetFontSize());
        }
    }
    std::string weight;
    auto fontWeight = param->GetProperty("weight");
    if (!fontWeight->IsNull()) {
        if (fontWeight->IsNumber()) {
            weight = std::to_string(fontWeight->ToNumber<int32_t>());
        } else {
            ParseJsString(fontWeight, weight);
        }
        SelectModel::GetInstance()->SetSelectedOptionFontWeight(ConvertStrToFontWeight(weight));
    }
    auto family = param->GetProperty("family");
    if (!family->IsNull() && family->IsString()) {
        auto familyVal = family->ToString();
        SelectModel::GetInstance()->SetSelectedOptionFontFamily(ConvertStrToFontFamilies(familyVal));
    }
    auto style = param->GetProperty("style");
    if (!style->IsNull() && style->IsNumber()) {
        auto styleVal = static_cast<FontStyle>(style->ToNumber<int32_t>());
        SelectModel::GetInstance()->SetSelectedOptionItalicFontStyle(styleVal);
    }
}

void JSSelect::SelectedOptionFontColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    Color textColor;
    if (!ParseJsColor(info[0], textColor)) {
        if (info[0]->IsNull() || info[0]->IsUndefined()) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SelectTheme>();
            CHECK_NULL_VOID(theme);
            textColor = theme->GetSelectedColorText();
        } else {
            return;
        }
    }
    SelectModel::GetInstance()->SetSelectedOptionFontColor(textColor);
}

void JSSelect::OptionBgColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    Color bgColor;
    if (!ParseJsColor(info[0], bgColor)) {
        if (info[0]->IsUndefined() || info[0]->IsNull()) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SelectTheme>();
            CHECK_NULL_VOID(theme);
            bgColor = theme->GetBackgroundColor();
        } else {
            return;
        }
    }

    SelectModel::GetInstance()->SetOptionBgColor(bgColor);
}

void JSSelect::OptionFont(const JSCallbackInfo& info)
{
    if (info[0]->IsUndefined() || info[0]->IsNull()) {
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto selectTheme = pipeline->GetTheme<SelectTheme>();
        CHECK_NULL_VOID(selectTheme);
        auto textTheme = pipeline->GetTheme<TextTheme>();
        CHECK_NULL_VOID(textTheme);
        SelectModel::GetInstance()->SetOptionFontSize(selectTheme->GetMenuFontSize());
        SelectModel::GetInstance()->SetOptionFontWeight(textTheme->GetTextStyle().GetFontWeight());
        SelectModel::GetInstance()->SetOptionFontFamily(textTheme->GetTextStyle().GetFontFamilies());
        SelectModel::GetInstance()->SetOptionItalicFontStyle(textTheme->GetTextStyle().GetFontStyle());
        return;
    }

    if (!info[0]->IsObject()) {
        return;
    }
    auto param = JSRef<JSObject>::Cast(info[0]);

    auto size = param->GetProperty("size");
    if (!size->IsNull()) {
        CalcDimension fontSize;
        if (ParseJsDimensionFp(size, fontSize)) {
            SelectModel::GetInstance()->SetOptionFontSize(fontSize);
        }
        if (size->IsUndefined()) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SelectTheme>();
            CHECK_NULL_VOID(theme);
            SelectModel::GetInstance()->SetOptionFontSize(theme->GetMenuFontSize());
        }
    }
    std::string weight;
    auto fontWeight = param->GetProperty("weight");
    if (!fontWeight->IsNull()) {
        if (fontWeight->IsNumber()) {
            weight = std::to_string(fontWeight->ToNumber<int32_t>());
        } else {
            ParseJsString(fontWeight, weight);
        }
        SelectModel::GetInstance()->SetOptionFontWeight(ConvertStrToFontWeight(weight));
    }

    auto family = param->GetProperty("family");
    if (!family->IsNull() && family->IsString()) {
        auto familyVal = family->ToString();
        SelectModel::GetInstance()->SetOptionFontFamily(ConvertStrToFontFamilies(familyVal));
    }

    auto style = param->GetProperty("style");
    if (!style->IsNull() && style->IsNumber()) {
        auto styleVal = static_cast<FontStyle>(style->ToNumber<int32_t>());
        SelectModel::GetInstance()->SetOptionItalicFontStyle(styleVal);
    }
}

void JSSelect::OptionFontColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    Color textColor;
    if (!ParseJsColor(info[0], textColor)) {
        if (info[0]->IsUndefined() || info[0]->IsNull()) {
            auto pipeline = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            auto theme = pipeline->GetTheme<SelectTheme>();
            CHECK_NULL_VOID(theme);
            textColor = theme->GetMenuFontColor();
        } else {
            return;
        }
    }

    SelectModel::GetInstance()->SetOptionFontColor(textColor);
}

void JSSelect::OnSelected(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onSelect = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](
                        int32_t index, const std::string& value) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("Select.onSelect");
        JSRef<JSVal> params[2];
        params[0] = JSRef<JSVal>::Make(ToJSValue(index));
        params[1] = JSRef<JSVal>::Make(ToJSValue(value));
        func->ExecuteJS(2, params);
    };
    SelectModel::GetInstance()->SetOnSelect(std::move(onSelect));
    info.ReturnSelf();
}

void JSSelect::JsWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    SelectModel::GetInstance()->SetWidth(value);
}

void JSSelect::JsHeight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    SelectModel::GetInstance()->SetHeight(value);
}

bool CheckJSCallbackInfo(
    const std::string& callerName, const JSCallbackInfo& info, std::vector<JSCallbackInfoType>& infoTypes)
{
    if (info.Length() < 1) {
        return false;
    }
    bool typeVerified = false;
    std::string unrecognizedType;
    for (const auto& infoType : infoTypes) {
        switch (infoType) {
            case JSCallbackInfoType::STRING:
                if (info[0]->IsString()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "string|";
                }
                break;
            case JSCallbackInfoType::NUMBER:
                if (info[0]->IsNumber()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "number|";
                }
                break;
            case JSCallbackInfoType::OBJECT:
                if (info[0]->IsObject()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "object|";
                }
                break;
            case JSCallbackInfoType::FUNCTION:
                if (info[0]->IsFunction()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "Function|";
                }
                break;
            default:
                break;
        }
    }
    return typeVerified || infoTypes.size() == 0;
}

void JSSelect::JsSize(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsSize", info, checkList)) {
        return;
    }

    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);

    CalcDimension width;
    if (!ParseJsDimensionVp(sizeObj->GetProperty("width"), width)) {
        return;
    }

    CalcDimension height;
    if (!ParseJsDimensionVp(sizeObj->GetProperty("height"), height)) {
        return;
    }

    SelectModel::GetInstance()->SetSize(width, height);
}

void JSSelect::JsPadding(const JSCallbackInfo& info)
{
    if (!info[0]->IsString() && !info[0]->IsNumber() && !info[0]->IsObject()) {
        return;
    }

    if (info[0]->IsObject()) {
        std::optional<CalcDimension> left;
        std::optional<CalcDimension> right;
        std::optional<CalcDimension> top;
        std::optional<CalcDimension> bottom;
        JSRef<JSObject> paddingObj = JSRef<JSObject>::Cast(info[0]);

        CalcDimension leftDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("left"), leftDimen)) {
            left = leftDimen;
        }
        CalcDimension rightDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("right"), rightDimen)) {
            right = rightDimen;
        }
        CalcDimension topDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("top"), topDimen)) {
            top = topDimen;
        }
        CalcDimension bottomDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("bottom"), bottomDimen)) {
            bottom = bottomDimen;
        }
        if (left.has_value() || right.has_value() || top.has_value() || bottom.has_value()) {
            ViewAbstractModel::GetInstance()->SetPaddings(top, bottom, left, right);
            return;
        }
    }

    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        value.Reset();
    }
    SelectModel::GetInstance()->SetPadding(value);
}

void JSSelect::SetPaddingLeft(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    SelectModel::GetInstance()->SetPaddingLeft(value);
}

void JSSelect::SetPaddingTop(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    SelectModel::GetInstance()->SetPaddingTop(value);
}

void JSSelect::SetPaddingRight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    SelectModel::GetInstance()->SetPaddingRight(value);
}

void JSSelect::SetPaddingBottom(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    SelectModel::GetInstance()->SetPaddingBottom(value);
}

void JSSelect::SetSpace(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    auto selectTheme = GetTheme<SelectTheme>();

    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        value = selectTheme->GetContentSpinnerPadding();
    }
    if (LessNotEqual(value.Value(), 0.0) || value.Unit() == DimensionUnit::PERCENT) {
        value = selectTheme->GetContentSpinnerPadding();
    }

    SelectModel::GetInstance()->SetSpace(value);
}

void JSSelect::SetArrowPosition(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    int32_t direction = 0;
    if (!ParseJsInt32(info[0], direction)) {
        direction = 0;
    }

    if (static_cast<ArrowPosition>(direction) != ArrowPosition::START &&
        static_cast<ArrowPosition>(direction) != ArrowPosition::END) {
        direction = 0;
    }

    SelectModel::GetInstance()->SetArrowPosition(static_cast<ArrowPosition>(direction));
}

void JSSelect::SetMenuAlign(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    MenuAlign menuAlignObj;

    if (!info[0]->IsNumber()) {
        if (!(info[0]->IsUndefined() || info[0]->IsNull())) {
            return;
        }
    } else {
        menuAlignObj.alignType = static_cast<MenuAlignType>(info[0]->ToNumber<int32_t>());
    }

    if (info.Length() > 1) {
        if (info[1]->IsUndefined() || info[1]->IsNull()) {
            SelectModel::GetInstance()->SetMenuAlign(menuAlignObj);
            return;
        }
        if (!info[1]->IsObject()) {
            return;
        }
        auto offsetObj = JSRef<JSObject>::Cast(info[1]);
        CalcDimension dx;
        auto dxValue = offsetObj->GetProperty("dx");
        ParseJsDimensionVp(dxValue, dx);
        CalcDimension dy;
        auto dyValue = offsetObj->GetProperty("dy");
        ParseJsDimensionVp(dyValue, dy);
        menuAlignObj.offset = DimensionOffset(dx, dy);
    }

    SelectModel::GetInstance()->SetMenuAlign(menuAlignObj);
}

void JSSelect::SetOptionWidth(const JSCallbackInfo& info)
{
    CalcDimension value;
    
    if (info[0]->IsString()) {
        std::string modeFlag = info[0]->ToString();
        if (modeFlag.compare("fit_content") == 0) {
            SelectModel::GetInstance()->SetOptionWidthFitTrigger(false);
        } else if (modeFlag.compare("fit_trigger") == 0) {
            SelectModel::GetInstance()->SetOptionWidthFitTrigger(true);
        } else {
            ParseJsDimensionVp(info[0], value);
            SelectModel::GetInstance()->SetOptionWidth(value);
        }
    } else {
        if (!ParseJsDimensionVp(info[0], value)) {
            return;
        }
        SelectModel::GetInstance()->SetOptionWidth(value);
    }
}

void JSSelect::SetOptionHeight(const JSCallbackInfo& info)
{
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    
    SelectModel::GetInstance()->SetOptionHeight(value);
}

void JSSelect::SetOptionWidthFitTrigger(const JSCallbackInfo& info)
{
    bool isFitTrigger = false;
    if (info[0]->IsBoolean()) {
        isFitTrigger = info[0]->ToBoolean();
    }
    
    SelectModel::GetInstance()->SetOptionWidthFitTrigger(isFitTrigger);
}
} // namespace OHOS::Ace::Framework
