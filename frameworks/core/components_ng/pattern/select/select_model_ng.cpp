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

#include "core/components_ng/pattern/select/select_model_ng.h"

#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/select/select_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
void SetSelectDefaultSize(const RefPtr<FrameNode>& select)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);

    auto layoutProperty = select->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateCalcMinSize(CalcSize(CalcLength(theme->GetSelectMinWidth()), std::nullopt));
}

static constexpr Dimension SELECT_MARGIN_VP = 4.0_vp;
static constexpr Dimension SELECT_MIN_SPACE = 8.0_vp;
static constexpr Dimension SELECT_HORIZONTAL_GAP = 24.0_vp;
} // namespace

void SelectModelNG::Create(const std::vector<SelectParam>& params)
{
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = (stack == nullptr ? 0 : stack->ClaimNodeId());
    ACE_SCOPED_TRACE("Create[%s][self:%d]", V2::SELECT_ETS_TAG, nodeId);
    auto select = FrameNode::GetOrCreateFrameNode(
        V2::SELECT_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<SelectPattern>(); });
    ViewStackProcessor::GetInstance()->Push(select);

    SetSelectDefaultSize(select);
    auto pattern = select->GetPattern<SelectPattern>();
    
    CHECK_NULL_VOID(pattern);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pattern->SetSelectDefaultTheme();
    
    NG::PaddingProperty paddings;
    paddings.top = std::nullopt;
    paddings.bottom = std::nullopt;
    paddings.left = NG::CalcLength(SELECT_MARGIN_VP);
    paddings.right = NG::CalcLength(SELECT_MARGIN_VP);
    ViewAbstract::SetPadding(paddings);
    
    pattern->BuildChild();
    // create menu node
    if (!pattern->GetMenuNode()) {
        auto menuWrapper = MenuView::Create(params, nodeId, V2::SELECT_ETS_TAG);
        pattern->SetMenuNode(menuWrapper);
        pattern->InitSelected();
    } else {
        auto menuNode = pattern->GetMenuNode();
        CHECK_NULL_VOID(menuNode);
        auto menuPattern = menuNode->GetPattern<MenuPattern>();
        CHECK_NULL_VOID(menuPattern);
        menuPattern->UpdateSelectParam(params);
    }
    // store option pointers in select
    auto menuContainer = pattern->GetMenuNode();
    CHECK_NULL_VOID(menuContainer);
    pattern->ClearOptions();
    auto menuPattern = menuContainer->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    auto options = menuPattern->GetOptions();
    for (auto&& option : options) {
        pattern->AddOptionNode(option);
    }

    // delete menu when select node destroy
    auto destructor = [id = select->GetId()]() {
        auto pipeline = NG::PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto overlayManager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        overlayManager->DeleteMenu(id);
    };
    select->PushDestroyCallback(destructor);
}

void SelectModelNG::SetSelected(int32_t idx)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelected(idx);
}

void SelectModelNG::SetValue(const std::string& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetValue(value);
}

void SelectModelNG::SetFontSize(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontSize(value);
}

void SelectModelNG::SetFontWeight(const FontWeight& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontWeight(value);
}

void SelectModelNG::SetFontFamily(const std::vector<std::string>& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontFamily(value);
}

void SelectModelNG::SetItalicFontStyle(const Ace::FontStyle& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetItalicFontStyle(value);
}

void SelectModelNG::SetFontColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontColor(color);
}

void SelectModelNG::SetSelectedOptionBgColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionBgColor(color);
}

void SelectModelNG::SetSelectedOptionFontSize(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontSize(value);
}

void SelectModelNG::SetSelectedOptionFontWeight(const FontWeight& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontWeight(value);
}

void SelectModelNG::SetSelectedOptionFontFamily(const std::vector<std::string>& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontFamily(value);
}

void SelectModelNG::SetSelectedOptionItalicFontStyle(const Ace::FontStyle& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionItalicFontStyle(value);
}

void SelectModelNG::SetSelectedOptionFontColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontColor(color);
}

void SelectModelNG::SetOptionBgColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionBgColor(color);
}

void SelectModelNG::SetOptionFontSize(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontSize(value);
}

void SelectModelNG::SetOptionFontWeight(const FontWeight& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontWeight(value);
}

void SelectModelNG::SetOptionFontFamily(const std::vector<std::string>& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontFamily(value);
}

void SelectModelNG::SetOptionItalicFontStyle(const Ace::FontStyle& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionItalicFontStyle(value);
}

void SelectModelNG::SetOptionFontColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontColor(color);
}

void SelectModelNG::SetOnSelect(NG::SelectEvent&& onSelect)
{
    auto hub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<SelectEventHub>();
    CHECK_NULL_VOID(hub);
    hub->SetSelectEvent(std::move(onSelect));
}

void SelectModelNG::SetWidth(Dimension& value)
{
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    
    double minWidth = 2 * pattern->GetFontSize().ConvertToVp() + SELECT_HORIZONTAL_GAP.ConvertToVp() +
        theme->GetSpinnerWidth().ConvertToVp() + SELECT_MIN_SPACE.ConvertToVp();
    if (value.ConvertToVp() < minWidth) {
        pattern->SetSpace(SELECT_MIN_SPACE);
    } else {
        ViewAbstract::SetWidth(NG::CalcLength(value));
    }
}

void SelectModelNG::SetHeight(Dimension& value)
{
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    ViewAbstract::SetHeight(NG::CalcLength(value));
}

void SelectModelNG::SetSize(Dimension& width, Dimension& height)
{
    if (LessNotEqual(width.Value(), 0.0)) {
        width.SetValue(0.0);
    }

    if (LessNotEqual(height.Value(), 0.0)) {
        height.SetValue(0.0);
    }
    ViewAbstract::SetWidth(NG::CalcLength(width));
    ViewAbstract::SetHeight(NG::CalcLength(height));
}

void SelectModelNG::SetPaddings(
    const std::optional<CalcDimension>& top, const std::optional<CalcDimension>& bottom,
    const std::optional<CalcDimension>& left, const std::optional<CalcDimension>& right)
{
    NG::PaddingProperty paddings;
    if (top.has_value()) {
        if (top.value().Unit() == DimensionUnit::CALC) {
            paddings.top =
                NG::CalcLength(top.value().IsNonNegative() ? top.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.top = NG::CalcLength(top.value().IsNonNegative() ? top.value() : CalcDimension());
        }
    }
    if (bottom.has_value()) {
        if (bottom.value().Unit() == DimensionUnit::CALC) {
            paddings.bottom = NG::CalcLength(
                bottom.value().IsNonNegative() ? bottom.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.bottom = NG::CalcLength(bottom.value().IsNonNegative() ? bottom.value() : CalcDimension());
        }
    }
    if (left.has_value()) {
        if (left.value().Unit() == DimensionUnit::CALC) {
            paddings.left = NG::CalcLength(
                left.value().IsNonNegative() ? left.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.left = NG::CalcLength(left.value().IsNonNegative() ? left.value() : CalcDimension());
        }
    }
    if (right.has_value()) {
        if (right.value().Unit() == DimensionUnit::CALC) {
            paddings.right = NG::CalcLength(
                right.value().IsNonNegative() ? right.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.right = NG::CalcLength(right.value().IsNonNegative() ? right.value() : CalcDimension());
        }
    }
    ViewAbstract::SetPadding(paddings);
}

void SelectModelNG::SetPadding(const CalcDimension& value)
{
    if (value.Unit() == DimensionUnit::CALC) {
        // padding must great or equal zero.
        ViewAbstract::SetPadding(
            NG::CalcLength(value.IsNonNegative() ? value.CalcValue() : CalcDimension().CalcValue()));
    } else {
        // padding must great or equal zero.
        ViewAbstract::SetPadding(NG::CalcLength(value.IsNonNegative() ? value : CalcDimension()));
    }
}

void SelectModelNG::SetPaddingLeft(const CalcDimension& leftValue)
{
    NG::PaddingProperty paddings;
    paddings.top = std::nullopt;
    paddings.bottom = std::nullopt;

    if (!NearEqual(leftValue.Value(), 0.0)) {
        if (leftValue.Unit() == DimensionUnit::CALC) {
            paddings.left = NG::CalcLength(
                leftValue.IsNonNegative() ? leftValue.CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.left = NG::CalcLength(leftValue.IsNonNegative() ? leftValue : CalcDimension());
        }
    }
    paddings.right = std::nullopt;
    ViewAbstract::SetPadding(paddings);
}

void SelectModelNG::SetPaddingTop(const CalcDimension& topValue)
{
    NG::PaddingProperty paddings;
    if (!NearEqual(topValue.Value(), 0.0)) {
        if (topValue.Unit() == DimensionUnit::CALC) {
            paddings.top = NG::CalcLength(
                topValue.IsNonNegative() ? topValue.CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.top = NG::CalcLength(topValue.IsNonNegative() ? topValue : CalcDimension());
        }
    }
    paddings.bottom = std::nullopt;
    paddings.left = std::nullopt;
    paddings.right = std::nullopt;
    ViewAbstract::SetPadding(paddings);
}

void SelectModelNG::SetPaddingRight(const CalcDimension& rightValue)
{
    NG::PaddingProperty paddings;
    paddings.top = std::nullopt;
    paddings.bottom = std::nullopt;
    paddings.left = std::nullopt;
    if (!NearEqual(rightValue.Value(), 0.0)) {
        if (rightValue.Unit() == DimensionUnit::CALC) {
            paddings.right = NG::CalcLength(
                rightValue.IsNonNegative() ? rightValue.CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.right = NG::CalcLength(rightValue.IsNonNegative() ? rightValue : CalcDimension());
        }
    }
    ViewAbstract::SetPadding(paddings);
}

void SelectModelNG::SetPaddingBottom(const CalcDimension& buttomValue)
{
    NG::PaddingProperty paddings;
    paddings.top = std::nullopt;
    if (!NearEqual(buttomValue.Value(), 0.0)) {
        if (buttomValue.Unit() == DimensionUnit::CALC) {
            paddings.bottom = NG::CalcLength(
                buttomValue.IsNonNegative() ? buttomValue.CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.bottom = NG::CalcLength(
                buttomValue.IsNonNegative() ? buttomValue : CalcDimension());
        }
    }
    paddings.left = std::nullopt;
    paddings.right = std::nullopt;
    ViewAbstract::SetPadding(paddings);
}

void SelectModelNG::SetSpace(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSpace(value);
}

void SelectModelNG::SetArrowPosition(const ArrowPosition value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetArrowPosition(value);
}

void SelectModelNG::SetMenuAlign(const MenuAlign& menuAlign)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetMenuAlign(menuAlign);
}

void SelectModelNG::SetSelectChangeEvent(NG::SelectChangeEvent&& selectChangeEvent)
{
    auto hub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<SelectEventHub>();
    CHECK_NULL_VOID(hub);
    hub->SetSelectChangeEvent(std::move(selectChangeEvent));
}

void SelectModelNG::SetValueChangeEvent(NG::ValueChangeEvent&& valueChangeEvent)
{
    auto hub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<SelectEventHub>();
    CHECK_NULL_VOID(hub);
    hub->SetValueChangeEvent(std::move(valueChangeEvent));
}

void SelectModelNG::SetOptionWidth(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionWidth(value);
}

void SelectModelNG::SetOptionHeight(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionHeight(value);
}

void SelectModelNG::SetOptionWidthFitTrigger(bool isFitTrigger)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionWidthFitTrigger(isFitTrigger);
}

void SelectModelNG::SetArrowPosition(FrameNode* frameNode, const ArrowPosition value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetArrowPosition(value);
}

void SelectModelNG::SetSpace(FrameNode* frameNode, const Dimension& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSpace(value);
}
void SelectModelNG::SetMenuAlign(FrameNode* frameNode, const MenuAlign& menuAlign)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetMenuAlign(menuAlign);
}
void SelectModelNG::SetValue(FrameNode* frameNode, const std::string& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetValue(value);
}
void SelectModelNG::SetSelected(FrameNode* frameNode, int32_t idx)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSelected(idx);
}
void SelectModelNG::SetFontSize(FrameNode* frameNode, const Dimension& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetFontSize(value);
}
void SelectModelNG::SetFontWeight(FrameNode* frameNode, const FontWeight& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetFontWeight(value);
}
void SelectModelNG::SetFontFamily(FrameNode* frameNode, const std::vector<std::string>& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetFontFamily(value);
}
void SelectModelNG::SetItalicFontStyle(FrameNode* frameNode, const Ace::FontStyle& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetItalicFontStyle(value);
}
void SelectModelNG::SetFontColor(FrameNode* frameNode, const Color& color)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetFontColor(color);
}
void SelectModelNG::SetSelectedOptionBgColor(FrameNode* frameNode, const Color& color)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionBgColor(color);
}
void SelectModelNG::SetOptionFontSize(FrameNode* frameNode, const Dimension& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontSize(value);
}
void SelectModelNG::SetOptionFontWeight(FrameNode* frameNode, const FontWeight& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontWeight(value);
}
void SelectModelNG::SetOptionFontFamily(FrameNode* frameNode, const std::vector<std::string>& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontFamily(value);
}
void SelectModelNG::SetOptionItalicFontStyle(FrameNode* frameNode, const Ace::FontStyle& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionItalicFontStyle(value);
}
void SelectModelNG::SetOptionBgColor(FrameNode* frameNode, const Color& color)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionBgColor(color);
}
void SelectModelNG::SetSelectedOptionFontColor(FrameNode* frameNode, const Color& color)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontColor(color);
}
void SelectModelNG::SetSelectedOptionFontSize(FrameNode* frameNode, const Dimension& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontSize(value);
}
void SelectModelNG::SetSelectedOptionFontWeight(FrameNode* frameNode, const FontWeight& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontWeight(value);
}
void SelectModelNG::SetSelectedOptionFontFamily(FrameNode* frameNode, const std::vector<std::string>& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontFamily(value);
}
void SelectModelNG::SetSelectedOptionItalicFontStyle(FrameNode* frameNode, const Ace::FontStyle& value)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionItalicFontStyle(value);
}
void SelectModelNG::SetOptionFontColor(FrameNode* frameNode, const Color& color)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = AceType::DynamicCast<SelectPattern>(frameNode->GetPattern());
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontColor(color);
}
} // namespace OHOS::Ace::NG
