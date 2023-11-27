/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_textpicker_modifier.h"

#include "core/components/common/properties/text_style.h"
#include "core/pipeline/base/element_register.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/picker/picker_type_define.h"
#include "bridge/common/utils/utils.h"

namespace OHOS::Ace::NG {
constexpr uint32_t DEFAULT_BG_COLOR = 0xFF007DFF;
constexpr int SIZE_OF_THREE = 3;
constexpr int POS_0 = 0;
constexpr int POS_1 = 1;
constexpr int POS_2 = 2;

void SetTextpickerBackgroundColor(NodeHandle node, uint32_t color)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetBackgroundColor(frameNode, Color(color));
}

void ResetTextpickerBackgroundColor(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetBackgroundColor(frameNode, Color(DEFAULT_BG_COLOR));
}

void SetTextpickerCanLoop(NodeHandle node, bool canLoop)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetCanLoop(frameNode, canLoop);
}

void ResetTextpickerCanLoop(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetCanLoop(frameNode, false);
}

void SetTextpickerSelected(NodeHandle node, uint32_t selectedValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetSelected(frameNode, selectedValue);
}

void ResetTextpickerSelected(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetSelected(frameNode, 0);
}

void SetTextpickerSelectedIndex(NodeHandle node, uint32_t* values, int size)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);

    if (TextPickerModel::GetInstance()->IsSingle()) {
        SetSelectedIndexSingle(frameNode, values, size);
    } else {
        SetSelectedIndexMulti(frameNode, values, size);
    }
}

void SetSelectedIndexSingle(FrameNode* frameNode, uint32_t* selectedValues, const int32_t size)
{
    std::vector<NG::RangeContent> rangeResult;
    TextPickerModel::GetInstance()->GetSingleRange(rangeResult);
    if (size > 0) {
        if (selectedValues[0] >= rangeResult.size()) {
            TextPickerModelNG::SetSelected(frameNode, 0);
        } else {
            TextPickerModelNG::SetSelected(frameNode, selectedValues[0]);
        }
    }
}

void SetSelectedIndexMultiInternal(FrameNode* frameNode,
    uint32_t count, std::vector<NG::TextCascadePickerOptions>& options, std::vector<uint32_t>& selectedValues)
{
    if (!TextPickerModel::GetInstance()->IsCascade()) {
        SetSelectedInternal(count, options, selectedValues);
    } else {
        TextPickerModelNG::SetHasSelectAttr(frameNode, true);
        ProcessCascadeSelected(options, 0, selectedValues);
        uint32_t maxCount = TextPickerModel::GetInstance()->GetMaxCount();
        if (selectedValues.size() < maxCount) {
            auto differ = maxCount - selectedValues.size();
            for (uint32_t i = 0; i < differ; i++) {
                selectedValues.emplace_back(0);
            }
        }
    }
}

void SetSelectedInternal(
    uint32_t count, std::vector<NG::TextCascadePickerOptions>& options, std::vector<uint32_t>& selectedValues)
{
    for (uint32_t i = 0; i < count; i++) {
        if (i > selectedValues.size() - 1) {
            selectedValues.emplace_back(0);
        } else {
            if (selectedValues[i] >= options[i].rangeResult.size()) {
                selectedValues[i] = 0;
            }
        }
    }
}

void SetSelectedIndexMulti(FrameNode* frameNode, uint32_t* inputs, const int32_t size)
{
    std::vector<NG::TextCascadePickerOptions> options;
    TextPickerModel::GetInstance()->GetMultiOptions(options);

    auto count =
        TextPickerModel::GetInstance()->IsCascade() ? TextPickerModel::GetInstance()->GetMaxCount() : options.size();
    std::vector<uint32_t> selectedValues;
    selectedValues.assign(inputs, inputs + size);
    if (size > 0) {
        SetSelectedIndexMultiInternal(frameNode, count, options, selectedValues);
    } else {
        TextPickerModelNG::SetSelecteds(frameNode, selectedValues);
        TextPickerModelNG::SetHasSelectAttr(frameNode, false);
        return;
    }
    TextPickerModelNG::SetSelecteds(frameNode, selectedValues);
}

void ProcessCascadeSelected(
    const std::vector<NG::TextCascadePickerOptions>& options, uint32_t index, std::vector<uint32_t>& selectedValues)
{
    std::vector<std::string> rangeResultValue;
    for (size_t i = 0; i < options.size(); i++) {
        rangeResultValue.emplace_back(options[i].rangeResult[0]);
    }

    if (index > selectedValues.size() - 1) {
        selectedValues.emplace_back(0);
    }
    if (selectedValues[index] >= rangeResultValue.size()) {
        selectedValues[index] = 0;
    }
    if (selectedValues[index] <= options.size() - 1 && options[selectedValues[index]].children.size() > 0) {
        ProcessCascadeSelected(options[selectedValues[index]].children, index + 1, selectedValues);
    }
}

void ResetTextpickerSelectedIndex(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetSelected(frameNode, 0);
}

void SetTextpickerTextStyle(NodeHandle node, uint32_t color, const char* fontInfo, int32_t styleVal)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::PickerTextStyle pickerTextStyle = GetPickerTextStyle(color, fontInfo, styleVal);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();
    TextPickerModelNG::SetNormalTextStyle(frameNode, pickerTheme, pickerTextStyle);
}
void ResetTextpickerTextStyle(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::PickerTextStyle pickerTextStyle;
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();
    TextPickerModelNG::SetNormalTextStyle(frameNode, pickerTheme, pickerTextStyle);
}

void SetTextpickerSelectedTextStyle(NodeHandle node, uint32_t color, const char* fontInfo, int32_t styleVal)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::PickerTextStyle pickerTextStyle = GetPickerTextStyle(color, fontInfo, styleVal);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();
    TextPickerModelNG::SetSelectedTextStyle(frameNode, pickerTheme, pickerTextStyle);
}

void ResetTextpickerSelectedTextStyle(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::PickerTextStyle pickerTextStyle;
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();
    TextPickerModelNG::SetSelectedTextStyle(frameNode, pickerTheme, pickerTextStyle);
}

void SetTextpickerDisappearTextStyle(NodeHandle node, uint32_t color, const char* fontInfo, int32_t styleVal)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::PickerTextStyle pickerTextStyle = GetPickerTextStyle(color, fontInfo, styleVal);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();
    TextPickerModelNG::SetDisappearTextStyle(frameNode, pickerTheme, pickerTextStyle);
}

void ResetTextpickerDisappearTextStyle(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::PickerTextStyle pickerTextStyle;
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto pickerTheme = themeManager->GetTheme<PickerTheme>();

    TextPickerModelNG::SetDisappearTextStyle(frameNode, pickerTheme, pickerTextStyle);
}

void SetTextpickerDefaultPickerItemHeight(NodeHandle node, float dVal, int dUnit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextPickerModelNG::SetDefaultPickerItemHeight(frameNode, Dimension(dVal, static_cast<DimensionUnit>(dUnit)));
}

void ResetTextpickerDefaultPickerItemHeight(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    float dVal = -1.0;
    DimensionUnit dUnit = DimensionUnit::FP;
    TextPickerModelNG::SetDefaultPickerItemHeight(frameNode, Dimension(dVal, dUnit));
}

NG::PickerTextStyle GetPickerTextStyle(uint32_t color, const char* fontInfo, int32_t styleVal)
{
    NG::PickerTextStyle textStyle;
    textStyle.textColor = Color(color);

    std::vector<std::string> res;
    std::string fontValues = std::string(fontInfo);
    StringUtils::StringSplitter(fontValues, ':', res);

    if (res.size() != SIZE_OF_THREE) {
        return textStyle;
    }

    if (res[POS_0] != "-1") {
        textStyle.fontSize = StringUtils::StringToCalcDimension(res[POS_0], false);
    } else {
        textStyle.fontSize = Dimension(-1);
    }

    if (res[POS_1] != "-1") {
        textStyle.fontWeight = StringUtils::StringToFontWeight(res[POS_1], FontWeight::NORMAL);
    }

    if (res[POS_2] != "-1") {
        textStyle.fontFamily = Framework::ConvertStrToFontFamilies(res[POS_2]);
    }
    textStyle.fontStyle = static_cast<Ace::FontStyle>(styleVal);
    return textStyle;
}
ArkUITextpickerModifierAPI GetTextpickerModifier()
{
    static const ArkUITextpickerModifierAPI modifier = {
        SetTextpickerBackgroundColor, SetTextpickerCanLoop, SetTextpickerSelected, SetTextpickerSelectedIndex,
        SetTextpickerTextStyle, SetTextpickerSelectedTextStyle, SetTextpickerDisappearTextStyle,
        SetTextpickerDefaultPickerItemHeight, ResetTextpickerCanLoop, ResetTextpickerSelected,
        ResetTextpickerTextStyle, ResetTextpickerSelectedTextStyle, ResetTextpickerDisappearTextStyle,
        ResetTextpickerDefaultPickerItemHeight, ResetTextpickerBackgroundColor };

    return modifier;
}
}