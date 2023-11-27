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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_INDICATOR_SWIPER_INDICATOR_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_INDICATOR_SWIPER_INDICATOR_PATTERN_H

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/swiper_indicator/dot_indicator/dot_indicator_layout_algorithm.h"
#include "core/components_ng/pattern/swiper_indicator/dot_indicator/dot_indicator_paint_method.h"
#include "core/components_ng/pattern/swiper_indicator/digit_indicator/digit_indicator_layout_algorithm.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_layout_property.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_utils.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
namespace OHOS::Ace::NG {
class SwiperIndicatorPattern : public Pattern {
    DECLARE_ACE_TYPE(SwiperIndicatorPattern, Pattern);
public:
    SwiperIndicatorPattern() = default;
    ~SwiperIndicatorPattern() override = default;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<SwiperIndicatorLayoutProperty>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        if (SwiperIndicatorUtils::GetSwiperIndicatorType() == SwiperIndicatorType::DOT) {
            return MakeRefPtr<DotIndicatorPaintProperty>();
        } else {
            return MakeRefPtr<PaintProperty>();
        }
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto swiperNode = GetSwiperNode();
        CHECK_NULL_RETURN(swiperNode, nullptr);
        auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
        CHECK_NULL_RETURN(swiperPattern, nullptr);
        if (swiperPattern->GetIndicatorType() == SwiperIndicatorType::DOT) {
            auto indicatorLayoutAlgorithm = MakeRefPtr<DotIndicatorLayoutAlgorithm>();
            indicatorLayoutAlgorithm->SetIsHoverOrPress(isHover_ || isPressed_);
            indicatorLayoutAlgorithm->SetHoverPoint(hoverPoint_);
            return indicatorLayoutAlgorithm;
        } else {
            auto indicatorLayoutAlgorithm = MakeRefPtr<DigitIndicatorLayoutAlgorithm>();
            indicatorLayoutAlgorithm->SetIsHoverOrPress(isHover_ || isPressed_);
            indicatorLayoutAlgorithm->SetHoverPoint(hoverPoint_);
            return indicatorLayoutAlgorithm;
        }
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        auto swiperNode = GetSwiperNode();
        CHECK_NULL_RETURN(swiperNode, nullptr);
        auto swiperPattern = swiperNode->GetPattern<SwiperPattern>();
        CHECK_NULL_RETURN(swiperPattern, nullptr);
        if (swiperPattern->GetIndicatorType() == SwiperIndicatorType::DOT) {
            if (!dotIndicatorModifier_) {
                dotIndicatorModifier_ = AceType::MakeRefPtr<DotIndicatorModifier>();
            }
            auto swiperLayoutProperty = swiperPattern->GetLayoutProperty<SwiperLayoutProperty>();
            CHECK_NULL_RETURN(swiperLayoutProperty, nullptr);
            dotIndicatorModifier_->SetAnimationDuration(swiperPattern->GetDuration());
            auto paintMethod = MakeRefPtr<DotIndicatorPaintMethod>(dotIndicatorModifier_);
            paintMethod->SetAxis(swiperPattern->GetDirection());
            paintMethod->SetCurrentIndex(swiperPattern->GetLoopIndex(swiperPattern->GetCurrentFirstIndex()));
            paintMethod->SetItemCount(swiperPattern->TotalCount());
            paintMethod->SetDisplayCount(swiperLayoutProperty->GetDisplayCount().value_or(1));
            paintMethod->SetTurnPageRate(swiperPattern->GetTurnPageRate());
            paintMethod->SetIsLoop(swiperPattern->IsLoop());
            paintMethod->SetIsHover(isHover_);
            paintMethod->SetIsPressed(isPressed_);
            paintMethod->SetHoverPoint(hoverPoint_);
            if (mouseClickIndex_) {
                mouseClickIndex_ = swiperPattern->GetLoopIndex(mouseClickIndex_.value());
            }
            paintMethod->SetMouseClickIndex(mouseClickIndex_);
            paintMethod->SetIsTouchBottom(touchBottomType_);
            paintMethod->SetTouchBottomRate(swiperPattern->GetTouchBottomRate());
            mouseClickIndex_ = std::nullopt;

            auto geometryNode = swiperNode->GetGeometryNode();
            CHECK_NULL_RETURN(geometryNode, nullptr);
            auto host = GetHost();
            CHECK_NULL_RETURN(host, nullptr);
            auto indicatorGeometryNode = host->GetGeometryNode();
            CHECK_NULL_RETURN(indicatorGeometryNode, nullptr);
            auto boundsValue =
                (geometryNode->GetFrameSize().Width() - indicatorGeometryNode->GetFrameSize().Width()) * 0.5f;
            auto boundsRectOriginX = -boundsValue;
            auto boundsRectOriginY = 0.0f;
            auto boundsRectWidth = geometryNode->GetFrameSize().Width();
            auto boundsRectHeight = indicatorGeometryNode->GetFrameSize().Height();
            if (swiperPattern->GetDirection() == Axis::VERTICAL) {
                boundsValue =
                    (geometryNode->GetFrameSize().Height() - indicatorGeometryNode->GetFrameSize().Height()) * 0.5f;
                boundsRectOriginX = 0.0f;
                boundsRectOriginY = -boundsValue;
                boundsRectWidth = indicatorGeometryNode->GetFrameSize().Width();
                boundsRectHeight = geometryNode->GetFrameSize().Height();
            }
            RectF boundsRect(boundsRectOriginX, boundsRectOriginY, boundsRectWidth, boundsRectHeight);
            dotIndicatorModifier_->SetBoundsRect(boundsRect);

            return paintMethod;
        } else {
            return nullptr;
        }
    }

    RefPtr<FrameNode> GetSwiperNode() const
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto swiperNode = host->GetParent();
        CHECK_NULL_RETURN(swiperNode, nullptr);
        return DynamicCast<FrameNode>(swiperNode);
    }

    FocusPattern GetFocusPattern() const override
    {
        auto pipelineContext = PipelineBase::GetCurrentContext();
        CHECK_NULL_RETURN(pipelineContext, FocusPattern());
        auto swiperTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
        CHECK_NULL_RETURN(swiperTheme, FocusPattern());
        FocusPaintParam paintParam;
        paintParam.SetPaintWidth(swiperTheme->GetFocusedBorderWidth());
        paintParam.SetPaintColor(swiperTheme->GetFocusedColor());
        return { FocusType::NODE, true, FocusStyleType::INNER_BORDER, paintParam };
    }

    void DumpAdvanceInfo() override;

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void InitClickEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleClick(const GestureEvent& info);
    void HandleMouseClick(const GestureEvent& info);
    void HandleTouchClick(const GestureEvent& info);
    void InitHoverMouseEvent();
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleMouseEvent(const MouseInfo& info);
    void HandleHoverEvent(bool isHover);
    void HoverInAnimation(const Color& hoverColor);
    void HoverOutAnimation(const Color& normalColor);
    void HandleTouchEvent(const TouchEventInfo& info);
    void HandleTouchDown();
    void HandleTouchUp();
    void HandleDragStart(const GestureEvent& info);
    void HandleDragEnd(double dragVelocity);
    void GetMouseClickIndex();
    void UpdateTextContent(const RefPtr<SwiperIndicatorLayoutProperty>& layoutProperty,
        const RefPtr<FrameNode>& firstTextNode, const RefPtr<FrameNode>& lastTextNode);
    void UpdateTextContentSub(
        const RefPtr<SwiperIndicatorLayoutProperty>& layoutProperty,
        const RefPtr<FrameNode>& firstTextNode, const RefPtr<FrameNode>& lastTextNode);
    bool CheckIsTouchBottom(const GestureEvent& info);
    void InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleLongPress(GestureEvent& info);
    void HandleLongDragUpdate(const TouchLocationInfo& info);
    bool CheckIsTouchBottom(const TouchLocationInfo& info);
    float HandleTouchClickMargin();
    RefPtr<ClickEvent> clickEvent_;
    RefPtr<InputEvent> hoverEvent_;
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<LongPressEvent> longPressEvent_;
    bool isHover_ = false;
    bool isPressed_ = false;
    PointF hoverPoint_;
    PointF dragStartPoint_;
    TouchBottomType touchBottomType_ = TouchBottomType::NONE;
    bool isClicked_ = false;
    bool isRepeatClicked_ = false;

    std::optional<int32_t> mouseClickIndex_ = std::nullopt;
    RefPtr<DotIndicatorModifier> dotIndicatorModifier_;
    SwiperIndicatorType swiperIndicatorType_ = SwiperIndicatorType::DOT;
    ACE_DISALLOW_COPY_AND_MOVE(SwiperIndicatorPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_INDICATOR_SWIPER_INDICATOR_PATTERN_H
