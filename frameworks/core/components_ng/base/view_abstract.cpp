/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/view_abstract.h"

#include <cstdint>
#include <optional>
#include <utility>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"
#include "base/subwindow/subwindow.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/bubble/bubble_view.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/option/option_paint_property.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/image/image_source_info.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
namespace {
// common function to bind menu
void BindMenu(const RefPtr<FrameNode> &menuNode, int32_t targetId, const NG::OffsetF &offset)
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    // pass in menuNode to register it in OverlayManager
    overlayManager->ShowMenu(targetId, offset, menuNode);
}

void RegisterMenuCallback(const RefPtr<FrameNode> &menuWrapperNode, const MenuParam &menuParam)
{
    CHECK_NULL_VOID(menuWrapperNode);
    auto pattern = menuWrapperNode->GetPattern<MenuWrapperPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->RegisterMenuAppearCallback(menuParam.onAppear);
    pattern->RegisterMenuDisappearCallback(menuParam.onDisappear);
    pattern->RegisterMenuStateChangeCallback(menuParam.onStateChange);
}
} // namespace

void ViewAbstract::SetWidth(const CalcLength &width)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    // get previously user defined ideal height
    std::optional<CalcLength> height = std::nullopt;
    auto &&layoutConstraint = layoutProperty->GetCalcLayoutConstraint();
    if (layoutConstraint && layoutConstraint->selfIdealSize) {
        height = layoutConstraint->selfIdealSize->Height();
    }
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(width, height));
}

void ViewAbstract::SetHeight(const CalcLength &height)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    // get previously user defined ideal width
    std::optional<CalcLength> width = std::nullopt;
    auto &&layoutConstraint = layoutProperty->GetCalcLayoutConstraint();
    if (layoutConstraint && layoutConstraint->selfIdealSize) {
        width = layoutConstraint->selfIdealSize->Width();
    }
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(width, height));
}

void ViewAbstract::SetClickEffectLevel(const ClickEffectLevel &level, float scaleValue)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ClickEffectInfo clickEffectInfo;
    clickEffectInfo.level = level;
    clickEffectInfo.scaleNumber = scaleValue;
    ACE_UPDATE_RENDER_CONTEXT(ClickEffectLevel, clickEffectInfo);
}

void ViewAbstract::ClearWidthOrHeight(bool isWidth)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->ClearUserDefinedIdealSize(isWidth, !isWidth);
}

void ViewAbstract::SetMinWidth(const CalcLength &width)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateCalcMinSize(CalcSize(width, std::nullopt));
}

void ViewAbstract::SetMinHeight(const CalcLength &height)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateCalcMinSize(CalcSize(std::nullopt, height));
}

void ViewAbstract::ResetMinSize(bool resetWidth)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->ResetCalcMinSize(resetWidth);
}

void ViewAbstract::SetMaxWidth(const CalcLength &width)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateCalcMaxSize(CalcSize(width, std::nullopt));
}

void ViewAbstract::SetMaxHeight(const CalcLength &height)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateCalcMaxSize(CalcSize(std::nullopt, height));
}

void ViewAbstract::ResetMaxSize(bool resetWidth)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->ResetCalcMaxSize(resetWidth);
}

void ViewAbstract::SetAspectRatio(float ratio)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, AspectRatio, ratio);
}

void ViewAbstract::ResetAspectRatio()
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_RESET_LAYOUT_PROPERTY(LayoutProperty, AspectRatio);
}

void ViewAbstract::SetBackgroundAlign(const Alignment &align)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackgroundAlign, align);
}

void ViewAbstract::SetBackgroundColor(const Color &color)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackgroundColor, color);
}

void ViewAbstract::SetBackgroundColor(FrameNode *frameNode, const Color &color)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BackgroundColor, color, frameNode);
}

void ViewAbstract::SetBackgroundImage(const ImageSourceInfo &src)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackgroundImage, src);
}

void ViewAbstract::SetBackgroundImage(FrameNode *frameNode, const ImageSourceInfo &src)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BackgroundImage, src, frameNode);
}

void ViewAbstract::SetBackgroundImageRepeat(const ImageRepeat &imageRepeat)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackgroundImageRepeat, imageRepeat);
}

void ViewAbstract::SetBackgroundImageRepeat(FrameNode *frameNode, const ImageRepeat &imageRepeat)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BackgroundImageRepeat, imageRepeat, frameNode);
}

void ViewAbstract::SetBackgroundImageSize(const BackgroundImageSize &bgImgSize)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackgroundImageSize, bgImgSize);
}

void ViewAbstract::SetBackgroundImageSize(FrameNode *frameNode, const BackgroundImageSize &bgImgSize)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BackgroundImageSize, bgImgSize, frameNode);
}

void ViewAbstract::SetBackgroundImagePosition(const BackgroundImagePosition &bgImgPosition)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackgroundImagePosition, bgImgPosition);
}

void ViewAbstract::SetBackgroundImagePosition(FrameNode *frameNode, const BackgroundImagePosition &bgImgPosition)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BackgroundImagePosition, bgImgPosition, frameNode);
}

void ViewAbstract::SetBackgroundBlurStyle(const BlurStyleOption &bgBlurStyle)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetBackgroundEffect().has_value()) {
            target->UpdateBackgroundEffect(std::nullopt);
        }
        target->UpdateBackBlurStyle(bgBlurStyle);
        if (target->GetBackBlurRadius().has_value()) {
            target->UpdateBackBlurRadius(Dimension());
        }
    }
}

void ViewAbstract::SetBackgroundEffect(const EffectOption &effectOption)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetBackBlurRadius().has_value()) {
            target->UpdateBackBlurRadius(Dimension());
        }
        if (target->GetBackBlurStyle().has_value()) {
            target->UpdateBackBlurStyle(std::nullopt);
        }
        target->UpdateBackgroundEffect(effectOption);
    }
}

void ViewAbstract::SetForegroundBlurStyle(const BlurStyleOption &fgBlurStyle)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        target->UpdateFrontBlurStyle(fgBlurStyle);
        if (target->GetFrontBlurRadius().has_value()) {
            target->UpdateFrontBlurRadius(Dimension());
        }
    }
}

void ViewAbstract::SetSphericalEffect(double radio)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(SphericalEffect, radio);
}

void ViewAbstract::SetPixelStretchEffect(PixStretchEffectOption &option)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(PixelStretchEffect, option);
}

void ViewAbstract::SetLightUpEffect(double radio)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(LightUpEffect, radio);
}

void ViewAbstract::SetLayoutWeight(int32_t value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, LayoutWeight, static_cast<float>(value));
}

void ViewAbstract::SetLayoutDirection(TextDirection value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, LayoutDirection, value);
}

void ViewAbstract::SetAlignRules(const std::map<AlignDirection, AlignRule> &alignRules)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, AlignRules, alignRules);
}

void ViewAbstract::SetAlignSelf(FlexAlign value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, AlignSelf, value);
}

void ViewAbstract::SetFlexShrink(float value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, FlexShrink, value);
}

void ViewAbstract::ResetFlexShrink()
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_RESET_LAYOUT_PROPERTY(LayoutProperty, FlexShrink);
}

void ViewAbstract::SetFlexGrow(float value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, FlexGrow, value);
}

void ViewAbstract::SetFlexBasis(const Dimension &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    if (LessNotEqual(value.Value(), 0.0f)) {
        ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, FlexBasis, Dimension());
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, FlexBasis, value);
}

void ViewAbstract::SetDisplayIndex(int32_t value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, DisplayIndex, value);
}

void ViewAbstract::SetPadding(const CalcLength &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    PaddingProperty padding;
    padding.SetEdges(value);
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, Padding, padding);
}

void ViewAbstract::SetPadding(const PaddingProperty &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, Padding, value);
}

void ViewAbstract::SetMargin(const CalcLength &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    MarginProperty margin;
    margin.SetEdges(value);
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, Margin, margin);
}

void ViewAbstract::SetMargin(const MarginProperty &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, Margin, value);
}

void ViewAbstract::SetBorderRadius(const Dimension &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(value);
    borderRadius.multiValued = false;
    ACE_UPDATE_RENDER_CONTEXT(BorderRadius, borderRadius);
}

void ViewAbstract::SetBorderRadius(const BorderRadiusProperty &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BorderRadius, value);
}

void ViewAbstract::SetBorderColor(const Color &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderColorProperty borderColor;
    borderColor.SetColor(value);
    ACE_UPDATE_RENDER_CONTEXT(BorderColor, borderColor);
}

void ViewAbstract::SetBorderColor(const BorderColorProperty &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BorderColor, value);
}

void ViewAbstract::SetBorderWidth(const Dimension &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderWidthProperty borderWidth;
    if (Negative(value.Value())) {
        borderWidth.SetBorderWidth(Dimension(0));
    } else {
        borderWidth.SetBorderWidth(value);
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, BorderWidth, borderWidth);
    ACE_UPDATE_RENDER_CONTEXT(BorderWidth, borderWidth);
}

void ViewAbstract::SetBorderWidth(const BorderWidthProperty &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, BorderWidth, value);
    ACE_UPDATE_RENDER_CONTEXT(BorderWidth, value);
}

void ViewAbstract::SetBorderStyle(const BorderStyle &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderStyleProperty borderStyle;
    borderStyle.SetBorderStyle(value);
    ACE_UPDATE_RENDER_CONTEXT(BorderStyle, borderStyle);
}

void ViewAbstract::SetBorderStyle(FrameNode *frameNode, const BorderStyle &value)
{
    BorderStyleProperty borderStyle;
    borderStyle.SetBorderStyle(value);
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderStyle, borderStyle, frameNode);
}

void ViewAbstract::SetBorderStyle(const BorderStyleProperty &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BorderStyle, value);
}

void ViewAbstract::SetBorderStyle(FrameNode *frameNode, const BorderStyleProperty &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderStyle, value, frameNode);
}

void ViewAbstract::SetOuterBorderRadius(const Dimension& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(value);
    borderRadius.multiValued = false;
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderRadius, borderRadius);
}

void ViewAbstract::SetOuterBorderRadius(const BorderRadiusProperty& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderRadius, value);
}

void ViewAbstract::SetOuterBorderColor(const Color& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderColorProperty borderColor;
    borderColor.SetColor(value);
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderColor, borderColor);
}

void ViewAbstract::SetOuterBorderColor(const BorderColorProperty& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderColor, value);
}

void ViewAbstract::SetOuterBorderWidth(const Dimension& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderWidthProperty borderWidth;
    if (Negative(value.Value())) {
        borderWidth.SetBorderWidth(Dimension(0));
    } else {
        borderWidth.SetBorderWidth(value);
    }
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderWidth, borderWidth);
}

void ViewAbstract::SetOuterBorderWidth(const BorderWidthProperty& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderWidth, value);
}

void ViewAbstract::SetOuterBorderStyle(const BorderStyleProperty& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderStyle, value);
}

void ViewAbstract::SetOuterBorderStyle(const BorderStyle& value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    BorderStyleProperty borderStyle;
    borderStyle.SetBorderStyle(value);
    ACE_UPDATE_RENDER_CONTEXT(OuterBorderStyle, borderStyle);
}

void ViewAbstract::DisableOnClick()
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->ClearUserOnClick();
}

void ViewAbstract::DisableOnTouch()
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->ClearUserOnTouch();
}

void ViewAbstract::DisableOnKeyEvent()
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->ClearUserOnKey();
}

void ViewAbstract::DisableOnHover()
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeInputEventHub();
    CHECK_NULL_VOID(eventHub);
    eventHub->ClearUserOnHover();
}

void ViewAbstract::DisableOnMouse()
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeInputEventHub();
    CHECK_NULL_VOID(eventHub);
    eventHub->ClearUserOnMouse();
}

void ViewAbstract::DisableOnAppear()
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->ClearUserOnAppear();
}

void ViewAbstract::DisableOnDisAppear()
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->ClearUserOnDisAppear();
}

void ViewAbstract::DisableOnAreaChange()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->ClearUserOnAreaChange();
}

void ViewAbstract::DisableOnFocus()
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->ClearUserOnFocus();
}

void ViewAbstract::DisableOnBlur()
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->ClearUserOnBlur();
}

void ViewAbstract::SetOnClick(GestureEventFunc &&clickEventFunc)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetUserOnClick(std::move(clickEventFunc));
}

void ViewAbstract::SetOnGestureJudgeBegin(GestureJudgeFunc &&gestureJudgeFunc)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetOnGestureJudgeBegin(std::move(gestureJudgeFunc));
}

void ViewAbstract::SetOnTouch(TouchEventFunc &&touchEventFunc)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetTouchEvent(std::move(touchEventFunc));
}

void ViewAbstract::SetOnMouse(OnMouseEventFunc &&onMouseEventFunc)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeInputEventHub();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetMouseEvent(std::move(onMouseEventFunc));
}

void ViewAbstract::SetOnHover(OnHoverFunc &&onHoverEventFunc)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeInputEventHub();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetHoverEvent(std::move(onHoverEventFunc));
}

void ViewAbstract::SetHoverEffect(HoverEffectType hoverEffect)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeInputEventHub();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetHoverEffect(hoverEffect);
}

void ViewAbstract::SetHoverEffectAuto(HoverEffectType hoverEffect)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeInputEventHub();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetHoverEffectAuto(hoverEffect);
}

void ViewAbstract::SetEnabled(bool enabled)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    if (eventHub) {
        eventHub->SetEnabled(enabled);
    }

    // The SetEnabled of focusHub must be after at eventHub
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    if (focusHub) {
        focusHub->SetEnabled(enabled);
    }
}

void ViewAbstract::SetFocusable(bool focusable)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetFocusable(focusable);
}

void ViewAbstract::SetOnFocus(OnFocusFunc &&onFocusCallback)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetOnFocusCallback(std::move(onFocusCallback));
}

void ViewAbstract::SetOnBlur(OnBlurFunc &&onBlurCallback)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetOnBlurCallback(std::move(onBlurCallback));
}

void ViewAbstract::SetOnKeyEvent(OnKeyCallbackFunc &&onKeyCallback)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetOnKeyCallback(std::move(onKeyCallback));
}

void ViewAbstract::SetTabIndex(int32_t index)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetTabIndex(index);
}

void ViewAbstract::SetFocusOnTouch(bool isSet)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetIsFocusOnTouch(isSet);
}

void ViewAbstract::SetDefaultFocus(bool isSet)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetIsDefaultFocus(isSet);
}

void ViewAbstract::SetGroupDefaultFocus(bool isSet)
{
    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetIsDefaultGroupFocus(isSet);
}

void ViewAbstract::SetOnAppear(std::function<void()> &&onAppear)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnAppear(std::move(onAppear));
}

void ViewAbstract::SetOnDisappear(std::function<void()> &&onDisappear)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDisappear(std::move(onDisappear));
}

void ViewAbstract::SetOnAreaChanged(std::function<void(const RectF &oldRect, const OffsetF &oldOrigin,
    const RectF &rect, const OffsetF &origin)> &&onAreaChanged)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->SetOnAreaChangeCallback(std::move(onAreaChanged));
    pipeline->AddOnAreaChangeNode(frameNode->GetId());
}

void ViewAbstract::SetOnVisibleChange(std::function<void(bool, double)> &&onVisibleChange,
    const std::vector<double> &ratioList)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->ClearVisibleAreaUserCallback();

    for (const auto &ratio : ratioList) {
        pipeline->AddVisibleAreaChangeNode(frameNode, ratio, onVisibleChange);
    }
}

void ViewAbstract::SetResponseRegion(const std::vector<DimensionRect> &responseRegion)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->MarkResponseRegion(true);
    gestureHub->SetResponseRegion(responseRegion);
}

void ViewAbstract::SetMouseResponseRegion(const std::vector<DimensionRect> &mouseRegion)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->MarkResponseRegion(true);
    gestureHub->SetMouseResponseRegion(mouseRegion);
}

void ViewAbstract::SetTouchable(bool touchable)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetTouchable(touchable);
}

void ViewAbstract::SetMonopolizeEvents(bool monopolizeEvents)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetMonopolizeEvents(monopolizeEvents);
}

void ViewAbstract::SetHitTestMode(HitTestMode hitTestMode)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetHitTestMode(hitTestMode);
}

void ViewAbstract::AddDragFrameNodeToManager()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);

    dragDropManager->AddDragFrameNode(frameNode->GetId(), AceType::WeakClaim(AceType::RawPtr(frameNode)));
}

void ViewAbstract::SetDraggable(bool draggable)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (draggable) {
        if (!frameNode->IsDraggable()) {
            gestureHub->InitDragDropEvent();
        }
    } else {
        gestureHub->RemoveDragEvent();
    }
    frameNode->SetCustomerDraggable(draggable);
}

void ViewAbstract::SetOnDragStart(
    std::function<DragDropInfo(const RefPtr<OHOS::Ace::DragEvent> &, const std::string &)> &&onDragStart)
{
    auto gestureHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->InitDragDropEvent();

    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDragStart(std::move(onDragStart));
}

void ViewAbstract::SetOnDragEnter(
    std::function<void(const RefPtr<OHOS::Ace::DragEvent> &, const std::string &)> &&onDragEnter)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCustomerOnDragFunc(DragFuncType::DRAG_ENTER, std::move(onDragEnter));

    AddDragFrameNodeToManager();
}

void ViewAbstract::SetOnDragLeave(
    std::function<void(const RefPtr<OHOS::Ace::DragEvent> &, const std::string &)> &&onDragLeave)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCustomerOnDragFunc(DragFuncType::DRAG_LEAVE, std::move(onDragLeave));

    AddDragFrameNodeToManager();
}

void ViewAbstract::SetOnDragMove(
    std::function<void(const RefPtr<OHOS::Ace::DragEvent> &, const std::string &)> &&onDragMove)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCustomerOnDragFunc(DragFuncType::DRAG_MOVE, std::move(onDragMove));

    AddDragFrameNodeToManager();
}

void ViewAbstract::SetOnDrop(std::function<void(const RefPtr<OHOS::Ace::DragEvent> &, const std::string &)> &&onDrop)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCustomerOnDragFunc(DragFuncType::DRAG_DROP, std::move(onDrop));

    AddDragFrameNodeToManager();
}

void ViewAbstract::SetOnDragEnd(std::function<void(const RefPtr<OHOS::Ace::DragEvent> &)> &&onDragEnd)
{
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCustomerOnDragFunc(DragFuncType::DRAG_END, std::move(onDragEnd));

    AddDragFrameNodeToManager();
}

void ViewAbstract::SetAlign(Alignment alignment)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, Alignment, alignment);
}

void ViewAbstract::SetAlign(FrameNode *frameNode, Alignment alignment)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, Alignment, alignment, frameNode);
}

void ViewAbstract::SetVisibility(VisibleType visible)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    if (layoutProperty) {
        layoutProperty->UpdateVisibility(visible, true);
    }

    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    if (focusHub) {
        focusHub->SetShow(visible == VisibleType::VISIBLE);
    }
}

void ViewAbstract::SetGeometryTransition(const std::string &id, bool followWithoutTransition)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    if (layoutProperty) {
        layoutProperty->UpdateGeometryTransition(id, followWithoutTransition);
    }
}

void ViewAbstract::SetGeometryTransition(FrameNode *frameNode, const std::string &id, bool followWithoutTransition)
{
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    if (layoutProperty) {
        layoutProperty->UpdateGeometryTransition(id, followWithoutTransition);
    }
}

void ViewAbstract::SetOpacity(double opacity)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(Opacity, opacity);
}
void ViewAbstract::SetAllowDrop(const std::set<std::string> &allowDrop)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->SetAllowDrop(allowDrop);
}

void ViewAbstract::SetPosition(const OffsetT<Dimension> &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(Position, value);
}

void ViewAbstract::SetOffset(const OffsetT<Dimension> &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(Offset, value);
}

void ViewAbstract::MarkAnchor(const OffsetT<Dimension> &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(Anchor, value);
}

void ViewAbstract::SetZIndex(int32_t value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(ZIndex, value);
}

void ViewAbstract::SetScale(const NG::VectorF &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(TransformScale, value);
}

void ViewAbstract::SetScale(FrameNode *frameNode, const NG::VectorF &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(TransformScale, value, frameNode);
}

void ViewAbstract::SetPivot(const DimensionOffset &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(TransformCenter, value);
}

void ViewAbstract::SetPivot(FrameNode *frameNode, const DimensionOffset &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(TransformCenter, value, frameNode);
}

void ViewAbstract::SetTranslate(const NG::TranslateOptions &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(TransformTranslate, value);
}

void ViewAbstract::SetTranslate(FrameNode *frameNode, const NG::TranslateOptions &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(TransformTranslate, value, frameNode);
}

void ViewAbstract::SetRotate(const NG::Vector5F &value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(TransformRotate, value);
}

void ViewAbstract::SetRotate(FrameNode *frameNode, const NG::Vector5F &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(TransformRotate, value, frameNode);
}

void ViewAbstract::SetTransformMatrix(const Matrix4 &matrix)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(TransformMatrix, matrix);
}

void ViewAbstract::BindPopup(const RefPtr<PopupParam> &param, const RefPtr<FrameNode> &targetNode,
    const RefPtr<UINode> &customNode)
{
    CHECK_NULL_VOID(targetNode);
    auto targetId = targetNode->GetId();
    auto targetTag = targetNode->GetTag();
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    auto popupInfo = overlayManager->GetPopupInfo(targetId);
    auto isShow = param->IsShow();
    auto isUseCustom = param->IsUseCustom();
    auto showInSubWindow = param->IsShowInSubWindow();
    // subwindow model needs to use subContainer to get popupInfo
    if (showInSubWindow) {
        auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(Container::CurrentId());
        if (subwindow) {
            subwindow->GetPopupInfoNG(targetId, popupInfo);
        }
    }

    auto popupId = popupInfo.popupId;
    auto popupNode = popupInfo.popupNode;
    RefPtr<BubblePattern> popupPattern;
    if (popupNode) {
        popupPattern = popupNode->GetPattern<BubblePattern>();
    }

    if (popupInfo.isCurrentOnShow) {
        // Entering / Normal / Exiting
        bool popupShowing = popupPattern ? popupPattern->IsOnShow() : false;
        if (popupShowing == isShow) {
            return;
        }
        if (!popupShowing && isShow) {
            popupInfo.markNeedUpdate = false;
        } else {
            popupInfo.markNeedUpdate = true;
        }
    } else {
        // Invisable
        if (!isShow) {
            return;
        }
        popupInfo.markNeedUpdate = true;
    }

    // Create new popup.
    if (popupInfo.popupId == -1 || !popupNode) {
        if (!isUseCustom) {
            popupNode = BubbleView::CreateBubbleNode(targetTag, targetId, param);
        } else {
            CHECK_NULL_VOID(customNode);
            popupNode = BubbleView::CreateCustomBubbleNode(targetTag, targetId, customNode, param);
        }
        if (popupNode) {
            popupId = popupNode->GetId();
        }
        if (!showInSubWindow) {
            // erase popup when target node destroy
            auto destructor = [id = targetNode->GetId()]() {
                auto pipeline = NG::PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID(pipeline);
                auto overlayManager = pipeline->GetOverlayManager();
                CHECK_NULL_VOID(overlayManager);
                overlayManager->ErasePopup(id);
                SubwindowManager::GetInstance()->HideSubWindowNG();
            };
            targetNode->PushDestroyCallback(destructor);
        } else {
            // erase popup in subwindow when target node destroy
            auto destructor = [id = targetNode->GetId(), containerId = Container::CurrentId()]() {
                auto subwindow = SubwindowManager::GetInstance()->GetSubwindow(containerId);
                CHECK_NULL_VOID(subwindow);
                auto overlayManager = subwindow->GetOverlayManager();
                CHECK_NULL_VOID(overlayManager);
                overlayManager->ErasePopup(id);
                SubwindowManager::GetInstance()->HideSubWindowNG();
            };
            targetNode->PushDestroyCallback(destructor);
        }
    } else {
        // use param to update PopupParm
        if (!isUseCustom) {
            BubbleView::UpdatePopupParam(popupId, param, targetNode);
            popupNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        } else {
            BubbleView::UpdateCustomPopupParam(popupId, param);
            popupNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }
    // update PopupInfo props
    popupInfo.popupId = popupId;
    popupInfo.popupNode = popupNode;
    popupInfo.isBlockEvent = param->IsBlockEvent();
    if (popupNode) {
        popupNode->MarkModifyDone();
        popupPattern = popupNode->GetPattern<BubblePattern>();
    }
    popupInfo.target = AceType::WeakClaim(AceType::RawPtr(targetNode));
    popupInfo.targetSize = SizeF(param->GetTargetSize().Width(), param->GetTargetSize().Height());
    popupInfo.targetOffset = OffsetF(param->GetTargetOffset().GetX(), param->GetTargetOffset().GetY());
    if (showInSubWindow) {
        if (isShow) {
            SubwindowManager::GetInstance()->ShowPopupNG(targetId, popupInfo);
        } else {
            SubwindowManager::GetInstance()->HidePopupNG(targetId);
        }
        return;
    }
    if (!popupInfo.isCurrentOnShow) {
        targetNode->OnAccessibilityEvent(AccessibilityEventType::CHANGE,
            WindowsContentChangeTypes::CONTENT_CHANGE_TYPE_SUBTREE);
    }
    if (isShow) {
        overlayManager->ShowPopup(targetId, popupInfo);
    } else {
        overlayManager->HidePopup(targetId, popupInfo);
    }
}

void ViewAbstract::BindMenuWithItems(std::vector<OptionParam> &&params, const RefPtr<FrameNode> &targetNode,
    const NG::OffsetF &offset, const MenuParam &menuParam)
{
    CHECK_NULL_VOID(targetNode);

    if (params.empty()) {
        return;
    }
    auto menuNode =
        MenuView::Create(std::move(params), targetNode->GetId(), targetNode->GetTag(), MenuType::MENU, menuParam);
    RegisterMenuCallback(menuNode, menuParam);
    BindMenu(menuNode, targetNode->GetId(), offset);
}

void ViewAbstract::BindMenuWithCustomNode(const RefPtr<UINode> &customNode, const RefPtr<FrameNode> &targetNode,
    const NG::OffsetF &offset, const MenuParam &menuParam, const RefPtr<UINode> &previewCustomNode)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto expandDisplay = theme->GetExpandDisplay();
    CHECK_NULL_VOID(customNode);
    CHECK_NULL_VOID(targetNode);
    auto menuNode =
        MenuView::Create(customNode, targetNode->GetId(), targetNode->GetTag(), menuParam, true, previewCustomNode);
    RegisterMenuCallback(menuNode, menuParam);
    if (menuParam.type == MenuType::CONTEXT_MENU) {
        SubwindowManager::GetInstance()->ShowMenuNG(menuNode, targetNode->GetId(), offset, menuParam.isAboveApps);
        return;
    }
    if (menuParam.type == MenuType::MENU && expandDisplay) {
        bool isShown = SubwindowManager::GetInstance()->GetShown();
        if (!isShown) {
            SubwindowManager::GetInstance()->ShowMenuNG(menuNode, targetNode->GetId(), offset, menuParam.isAboveApps);
        } else {
            SubwindowManager::GetInstance()->HideMenuNG(menuNode, targetNode->GetId());
        }
        return;
    }
    BindMenu(menuNode, targetNode->GetId(), offset);
}

void ViewAbstract::ShowMenu(int32_t targetId, const NG::OffsetF &offset, bool isContextMenu)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto expandDisplay = theme->GetExpandDisplay();
    if (isContextMenu || expandDisplay) {
        SubwindowManager::GetInstance()->ShowMenuNG(nullptr, targetId, offset);
        return;
    }
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);

    overlayManager->ShowMenu(targetId, offset, nullptr);
}

void ViewAbstract::SetBackdropBlur(const Dimension &radius, const BlurOption &blurOption)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetBackgroundEffect().has_value()) {
            target->UpdateBackgroundEffect(std::nullopt);
        }
        target->UpdateBackBlur(radius, blurOption);
        if (target->GetBackBlurStyle().has_value()) {
            target->UpdateBackBlurStyle(std::nullopt);
        }
    }
}

void ViewAbstract::SetBackdropBlur(FrameNode *frameNode, const Dimension &radius)
{
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetBackgroundEffect().has_value()) {
            target->UpdateBackgroundEffect(std::nullopt);
        }
        target->UpdateBackBlurRadius(radius);
        if (target->GetBackBlurStyle().has_value()) {
            target->UpdateBackBlurStyle(std::nullopt);
        }
    }
}

void ViewAbstract::SetLinearGradientBlur(NG::LinearGradientBlurPara blurPara)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(LinearGradientBlur, blurPara);
}

void ViewAbstract::SetDynamicLightUp(float rate, float lightUpDegree)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(DynamicLightUpRate, rate);
    ACE_UPDATE_RENDER_CONTEXT(DynamicLightUpDegree, lightUpDegree);
}

void ViewAbstract::SetFrontBlur(const Dimension &radius, const BlurOption &blurOption)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        target->UpdateFrontBlur(radius, blurOption);
        if (target->GetFrontBlurStyle().has_value()) {
            target->UpdateFrontBlurStyle(std::nullopt);
        }
    }
}

void ViewAbstract::SetFrontBlur(FrameNode *frameNode, const Dimension &radius)
{
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        target->UpdateFrontBlurRadius(radius);
        if (target->GetFrontBlurStyle().has_value()) {
            target->UpdateFrontBlurStyle(std::nullopt);
        }
    }
}

void ViewAbstract::SetBackShadow(const Shadow &shadow)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackShadow, shadow);
}

void ViewAbstract::SetBackShadow(FrameNode *frameNode, const Shadow &shadow)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BackShadow, shadow, frameNode);
}

void ViewAbstract::SetBlendMode(BlendMode blendMode)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        LOGD("current state is not processed, return");
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BackBlendMode, blendMode);
}

void ViewAbstract::SetLinearGradient(const NG::Gradient &gradient)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(LinearGradient, gradient);
}

void ViewAbstract::SetSweepGradient(const NG::Gradient &gradient)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(SweepGradient, gradient);
}

void ViewAbstract::SetRadialGradient(const NG::Gradient &gradient)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(RadialGradient, gradient);
}

void ViewAbstract::SetInspectorId(const std::string &inspectorId)
{
    auto uiNode = ViewStackProcessor::GetInstance()->GetMainElementNode();
    if (uiNode) {
        uiNode->UpdateInspectorId(inspectorId);
    }
}

void ViewAbstract::SetRestoreId(int32_t restoreId)
{
    auto uiNode = ViewStackProcessor::GetInstance()->GetMainElementNode();
    if (uiNode) {
        uiNode->SetRestoreId(restoreId);
    }
}

void ViewAbstract::SetDebugLine(const std::string &line)
{
    auto uiNode = ViewStackProcessor::GetInstance()->GetMainElementNode();
    if (uiNode) {
        uiNode->SetDebugLine(line);
    }
}

void ViewAbstract::SetGrid(std::optional<int32_t> span, std::optional<int32_t> offset, GridSizeType type)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    // frame node is mounted to parent when pop from stack later, no grid-container is added here
    layoutProperty->UpdateGridProperty(span, offset, type);
}

void ViewAbstract::Pop()
{
    ViewStackProcessor::GetInstance()->Pop();
}

void ViewAbstract::SetTransition(const TransitionOptions &options)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(Transition, options);
}

void ViewAbstract::SetChainedTransition(const RefPtr<NG::ChainedTransitionEffect> &effect)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(ChainedTransition, effect);
}

void ViewAbstract::SetClipShape(const RefPtr<BasicShape> &basicShape)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetClipEdge().has_value()) {
            target->UpdateClipEdge(false);
        }
        target->UpdateClipShape(basicShape);
    }
}

void ViewAbstract::SetClipShape(FrameNode *frameNode, const RefPtr<BasicShape> &basicShape)
{
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetClipEdge().has_value()) {
            target->UpdateClipEdge(false);
        }
        target->UpdateClipShape(basicShape);
    }
}

void ViewAbstract::SetClipEdge(bool isClip)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetClipShape().has_value()) {
            target->UpdateClipShape(nullptr);
        }
        target->UpdateClipEdge(isClip);
    }
}

void ViewAbstract::SetClipEdge(FrameNode *frameNode, bool isClip)
{
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetClipShape().has_value()) {
            target->UpdateClipShape(nullptr);
        }
        target->UpdateClipEdge(isClip);
    }
}

void ViewAbstract::SetMask(const RefPtr<BasicShape> &basicShape)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->HasProgressMask()) {
            target->UpdateProgressMask(nullptr);
        }
        target->UpdateClipMask(basicShape);
    }
}

void ViewAbstract::SetProgressMask(const RefPtr<ProgressMaskProperty> &progress)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->HasClipMask()) {
            target->UpdateClipMask(nullptr);
        }
        target->UpdateProgressMask(progress);
    }
}

void ViewAbstract::SetBrightness(const Dimension &brightness)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontBrightness, brightness);
}

void ViewAbstract::SetBrightness(FrameNode *frameNode, const Dimension &brightness)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontBrightness, brightness, frameNode);
}

void ViewAbstract::SetGrayScale(const Dimension &grayScale)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontGrayScale, grayScale);
}

void ViewAbstract::SetGrayScale(FrameNode *frameNode, const Dimension &grayScale)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontGrayScale, grayScale, frameNode);
}

void ViewAbstract::SetContrast(const Dimension &contrast)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontContrast, contrast);
}

void ViewAbstract::SetContrast(FrameNode *frameNode, const Dimension &contrast)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontContrast, contrast, frameNode);
}

void ViewAbstract::SetSaturate(const Dimension &saturate)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontSaturate, saturate);
}

void ViewAbstract::SetSaturate(FrameNode *frameNode, const Dimension &saturate)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontSaturate, saturate, frameNode);
}

void ViewAbstract::SetSepia(const Dimension &sepia)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontSepia, sepia);
}

void ViewAbstract::SetSepia(FrameNode *frameNode, const Dimension &sepia)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontSepia, sepia, frameNode);
}

void ViewAbstract::SetInvert(const InvertVariant &invert)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontInvert, invert);
}

void ViewAbstract::SetInvert(FrameNode *frameNode, const InvertVariant &invert)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontInvert, invert, frameNode);
}

void ViewAbstract::SetHueRotate(float hueRotate)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontHueRotate, hueRotate);
}

void ViewAbstract::SetHueRotate(FrameNode *frameNode, float hueRotate)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontHueRotate, hueRotate, frameNode);
}

void ViewAbstract::SetColorBlend(const Color &colorBlend)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(FrontColorBlend, colorBlend);
}

void ViewAbstract::SetColorBlend(FrameNode *frameNode, const Color &colorBlend)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(FrontColorBlend, colorBlend, frameNode);
}

void ViewAbstract::SetBorderImage(const RefPtr<BorderImage> &borderImage)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BorderImage, borderImage);
}

void ViewAbstract::SetBorderImageSource(const std::string &bdImageSrc)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ImageSourceInfo imageSourceInfo(bdImageSrc);
    ACE_UPDATE_RENDER_CONTEXT(BorderImageSource, imageSourceInfo);
}

void ViewAbstract::SetHasBorderImageSlice(bool tag)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(HasBorderImageSlice, tag);
}

void ViewAbstract::SetHasBorderImageWidth(bool tag)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(HasBorderImageWidth, tag);
}

void ViewAbstract::SetHasBorderImageOutset(bool tag)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(HasBorderImageOutset, tag);
}

void ViewAbstract::SetHasBorderImageRepeat(bool tag)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(HasBorderImageRepeat, tag);
}

void ViewAbstract::SetBorderImageGradient(const Gradient &gradient)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(BorderImageGradient, gradient);
}

void ViewAbstract::SetOverlay(const OverlayOptions &overlay)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(OverlayText, overlay);
}

void ViewAbstract::SetMotionPath(const MotionPathOption &motionPath)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(MotionPath, motionPath);
}

void ViewAbstract::SetSharedTransition(const std::string &shareId,
    const std::shared_ptr<SharedTransitionOption> &option)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto target = frameNode->GetRenderContext();
    if (target) {
        target->SetSharedTransitionOptions(option);
        target->SetShareId(shareId);
    }
}

void ViewAbstract::SetUseEffect(bool useEffect)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(UseEffect, useEffect);
}

void ViewAbstract::SetUseShadowBatching(bool useShadowBatching)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(UseShadowBatching, useShadowBatching);
}

void ViewAbstract::SetForegroundColor(const Color &color)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColor, color);
    ACE_RESET_RENDER_CONTEXT(RenderContext, ForegroundColorStrategy);
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColorFlag, true);
}

void ViewAbstract::SetForegroundColorStrategy(const ForegroundColorStrategy &strategy)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColorStrategy, strategy);
    ACE_RESET_RENDER_CONTEXT(RenderContext, ForegroundColor);
    ACE_UPDATE_RENDER_CONTEXT(ForegroundColorFlag, true);
}

void ViewAbstract::SetKeyboardShortcut(const std::string &value, const std::vector<ModifierKey> &keys,
    std::function<void()> &&onKeyboardShortcutAction)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto eventManager = pipeline->GetEventManager();
    CHECK_NULL_VOID(eventManager);
    auto eventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    if (value.empty() || (keys.size() == 0 && value.length() == 1)) {
        eventHub->SetKeyboardShortcut("", 0, nullptr);
        return;
    }
    auto key = eventManager->GetKeyboardShortcutKeys(keys);
    if ((key == 0 && value.length() == 1) || (key == 0 && keys.size() > 0 && value.length() > 1)) {
        return;
    }
    if (eventManager->IsSameKeyboardShortcutNode(value, key)) {
        return;
    }
    eventHub->SetKeyboardShortcut(value, key, std::move(onKeyboardShortcutAction));
    eventManager->AddKeyboardShortcutNode(WeakPtr<NG::FrameNode>(frameNode));
}

void ViewAbstract::CreateAnimatablePropertyFloat(const std::string &propertyName, float value,
    const std::function<void(float)> &onCallbackEvent)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->CreateAnimatablePropertyFloat(propertyName, value, onCallbackEvent);
}

void ViewAbstract::UpdateAnimatablePropertyFloat(const std::string &propertyName, float value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->UpdateAnimatablePropertyFloat(propertyName, value);
}

void ViewAbstract::CreateAnimatableArithmeticProperty(const std::string &propertyName,
    RefPtr<CustomAnimatableArithmetic> &value,
    std::function<void(const RefPtr<CustomAnimatableArithmetic> &)> &onCallbackEvent)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->CreateAnimatableArithmeticProperty(propertyName, value, onCallbackEvent);
}

void ViewAbstract::UpdateAnimatableArithmeticProperty(const std::string &propertyName,
    RefPtr<CustomAnimatableArithmetic> &value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->UpdateAnimatableArithmeticProperty(propertyName, value);
}

void ViewAbstract::SetObscured(const std::vector<ObscuredReasons> &reasons)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(Obscured, reasons);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void ViewAbstract::UpdateSafeAreaExpandOpts(const SafeAreaExpandOpts &opts)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_LAYOUT_PROPERTY(LayoutProperty, SafeAreaExpandOpts, opts);
}

void ViewAbstract::SetRenderGroup(bool isRenderGroup)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(RenderGroup, isRenderGroup);
}

void ViewAbstract::SetRenderFit(RenderFit renderFit)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(RenderFit, renderFit);
}

void ViewAbstract::SetBorderRadius(FrameNode *frameNode, const BorderRadiusProperty &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderRadius, value, frameNode);
}

void ViewAbstract::SetBorderRadius(FrameNode *frameNode, const Dimension &value)
{
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(value);
    borderRadius.multiValued = false;
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderRadius, borderRadius, frameNode);
}

void ViewAbstract::SetBorderWidth(FrameNode *frameNode, const BorderWidthProperty &value)
{
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, BorderWidth, value, frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderWidth, value, frameNode);
}

void ViewAbstract::SetBorderWidth(FrameNode *frameNode, const Dimension &value)
{
    BorderWidthProperty borderWidth;
    if (Negative(value.Value())) {
        borderWidth.SetBorderWidth(Dimension(0));
        LOGW("border width is negative, reset to 0");
    } else {
        borderWidth.SetBorderWidth(value);
    }
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, BorderWidth, borderWidth, frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderWidth, borderWidth, frameNode);
}

void ViewAbstract::SetBorderColor(FrameNode *frameNode, const BorderColorProperty &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderColor, value, frameNode);
}

void ViewAbstract::SetBorderColor(FrameNode *frameNode, const Color &value)
{
    BorderColorProperty borderColor;
    borderColor.SetColor(value);
    ACE_UPDATE_NODE_RENDER_CONTEXT(BorderColor, borderColor, frameNode);
}

void ViewAbstract::SetWidth(FrameNode *frameNode, const CalcLength &width)
{
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    // get previously user defined ideal height
    std::optional<CalcLength> height = std::nullopt;
    auto &&layoutConstraint = layoutProperty->GetCalcLayoutConstraint();
    if (layoutConstraint && layoutConstraint->selfIdealSize) {
        height = layoutConstraint->selfIdealSize->Height();
    }
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(width, height));
}

void ViewAbstract::SetHeight(FrameNode *frameNode, const CalcLength &height)
{
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    std::optional<CalcLength> width = std::nullopt;
    auto &&layoutConstraint = layoutProperty->GetCalcLayoutConstraint();
    if (layoutConstraint && layoutConstraint->selfIdealSize) {
        width = layoutConstraint->selfIdealSize->Width();
    }
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(width, height));
}

void ViewAbstract::ClearWidthOrHeight(FrameNode *frameNode, bool isWidth)
{
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->ClearUserDefinedIdealSize(isWidth, !isWidth);
}

void ViewAbstract::SetPosition(FrameNode *frameNode, const OffsetT<Dimension> &value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(Position, value, frameNode);
}

void ViewAbstract::SetTransformMatrix(FrameNode *frameNode, const Matrix4 &matrix)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(TransformMatrix, matrix, frameNode);
}

void ViewAbstract::SetHitTestMode(FrameNode *frameNode, HitTestMode hitTestMode)
{
    CHECK_NULL_VOID(frameNode);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetHitTestMode(hitTestMode);
}

void ViewAbstract::SetOpacity(FrameNode *frameNode, double opacity)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(Opacity, opacity, frameNode);
}

void ViewAbstract::SetZIndex(FrameNode *frameNode, int32_t value)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(ZIndex, value, frameNode);
}

void ViewAbstract::SetLinearGradient(FrameNode *frameNode, const NG::Gradient &gradient)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(LinearGradient, gradient, frameNode);
}

void ViewAbstract::SetSweepGradient(FrameNode *frameNode, const NG::Gradient &gradient)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(SweepGradient, gradient, frameNode);
}

void ViewAbstract::SetRadialGradient(FrameNode *frameNode, const NG::Gradient &gradient)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(RadialGradient, gradient, frameNode);
}

void ViewAbstract::SetForegroundBlurStyle(FrameNode *frameNode, const BlurStyleOption &fgBlurStyle)
{
    auto target = frameNode->GetRenderContext();
    if (target) {
        target->UpdateFrontBlurStyle(fgBlurStyle);
        if (target->GetFrontBlurRadius().has_value()) {
            target->UpdateFrontBlurRadius(Dimension());
        }
    }
}

void ViewAbstract::SetLinearGradientBlur(FrameNode *frameNode, NG::LinearGradientBlurPara blurPara)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(LinearGradientBlur, blurPara, frameNode);
}

void ViewAbstract::SetBackgroundBlurStyle(FrameNode *frameNode, const BlurStyleOption &bgBlurStyle)
{
    auto target = frameNode->GetRenderContext();
    if (target) {
        if (target->GetBackgroundEffect().has_value()) {
            target->UpdateBackgroundEffect(std::nullopt);
        }
        target->UpdateBackBlurStyle(bgBlurStyle);
        if (target->GetBackBlurRadius().has_value()) {
            target->UpdateBackBlurRadius(Dimension());
        }
    }
}

void ViewAbstract::SetPixelStretchEffect(FrameNode *frameNode, PixStretchEffectOption &option)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(PixelStretchEffect, option, frameNode);
}

void ViewAbstract::SetLightUpEffect(FrameNode *frameNode, double radio)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(LightUpEffect, radio, frameNode);
}

void ViewAbstract::SetSphericalEffect(FrameNode *frameNode, double radio)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(SphericalEffect, radio, frameNode);
}

void ViewAbstract::SetRenderGroup(FrameNode *frameNode, bool isRenderGroup)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(RenderGroup, isRenderGroup, frameNode);
}

void ViewAbstract::SetRenderFit(FrameNode *frameNode, RenderFit renderFit)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(RenderFit, renderFit, frameNode);
}

void ViewAbstract::SetUseEffect(FrameNode *frameNode, bool useEffect)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(UseEffect, useEffect, frameNode);
}

void ViewAbstract::SetForegroundColor(FrameNode *frameNode, const Color &color)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(ForegroundColor, color, frameNode);
    ACE_RESET_NODE_RENDER_CONTEXT(RenderContext, ForegroundColorStrategy, frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(ForegroundColorFlag, true, frameNode);
}

void ViewAbstract::SetForegroundColorStrategy(FrameNode *frameNode, const ForegroundColorStrategy &strategy)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(ForegroundColorStrategy, strategy, frameNode);
    ACE_RESET_NODE_RENDER_CONTEXT(RenderContext, ForegroundColor, frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(ForegroundColorFlag, true, frameNode);
}

void ViewAbstract::SetLightPosition(const CalcDimension &positionX, const CalcDimension &positionY,
    const CalcDimension &positionZ)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(LightPosition, TranslateOptions(positionX, positionY, positionZ));
}

void ViewAbstract::SetLightIntensity(const float value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(LightIntensity, value);
}

void ViewAbstract::SetLightIlluminated(const uint32_t value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(LightIlluminated, value);
}

void ViewAbstract::SetBloom(const float value)
{
    if (!ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess()) {
        return;
    }
    ACE_UPDATE_RENDER_CONTEXT(Bloom, value);
}

void ViewAbstract::SetMotionPath(FrameNode *frameNode, const MotionPathOption &motionPath)
{
    ACE_UPDATE_NODE_RENDER_CONTEXT(MotionPath, motionPath, frameNode);
}

void ViewAbstract::SetFocusOnTouch(FrameNode *frameNode, bool isSet)
{
    CHECK_NULL_VOID(frameNode);
    auto focusHub = frameNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetIsFocusOnTouch(isSet);
}

void ViewAbstract::SetGroupDefaultFocus(FrameNode *frameNode, bool isSet)
{
    CHECK_NULL_VOID(frameNode);
    auto focusHub = frameNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetIsDefaultGroupFocus(isSet);
}

void ViewAbstract::SetFocusable(FrameNode *frameNode, bool focusable)
{
    CHECK_NULL_VOID(frameNode);
    auto focusHub = frameNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetFocusable(focusable);
}

void ViewAbstract::SetTouchable(FrameNode *frameNode, bool touchable)
{
    CHECK_NULL_VOID(frameNode);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetTouchable(touchable);
}

void ViewAbstract::SetDefaultFocus(FrameNode *frameNode, bool isSet)
{
    CHECK_NULL_VOID(frameNode);
    auto focusHub = frameNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetIsDefaultFocus(isSet);
}

void ViewAbstract::SetDisplayIndex(FrameNode* frameNode, int32_t value)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, DisplayIndex, value, frameNode);
}

void ViewAbstract::SetOffset(FrameNode *frameNode, const OffsetT<Dimension> &value)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(Offset, value, frameNode);
}

void ViewAbstract::MarkAnchor(FrameNode *frameNode, const OffsetT<Dimension> &value)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(Anchor, value, frameNode);
}

void ViewAbstract::SetVisibility(FrameNode *frameNode, VisibleType visible)
{
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty();
    if (layoutProperty) {
        layoutProperty->UpdateVisibility(visible, true);
    }

    auto focusHub = ViewStackProcessor::GetInstance()->GetOrCreateMainFrameNodeFocusHub();
    if (focusHub) {
        focusHub->SetShow(visible == VisibleType::VISIBLE);
    }
}

void ViewAbstract::SetPadding(FrameNode *frameNode, const CalcLength &value)
{
    CHECK_NULL_VOID(frameNode);
    PaddingProperty padding;
    padding.SetEdges(value);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, Padding, padding, frameNode);
}

void ViewAbstract::SetPadding(FrameNode *frameNode, const PaddingProperty &value)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, Padding, value, frameNode);
}

void ViewAbstract::SetMargin(FrameNode *frameNode, const CalcLength &value)
{
    CHECK_NULL_VOID(frameNode);
    MarginProperty margin;
    margin.SetEdges(value);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, Margin, margin, frameNode);
}

void ViewAbstract::SetMargin(FrameNode *frameNode, const PaddingProperty &value)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(LayoutProperty, Margin, value, frameNode);
}

void ViewAbstract::SetAllowDrop(FrameNode* frameNode, const std::set<std::string>& allowDrop)
{
    CHECK_NULL_VOID(frameNode);
    frameNode->SetAllowDrop(allowDrop);
}
} // namespace OHOS::Ace::NG
