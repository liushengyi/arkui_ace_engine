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

#include "core/components_ng/pattern/navigation/title_bar_pattern.h"

#include "core/animation/spring_curve.h"
#include "core/common/container.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_node.h"
#include "core/components_ng/pattern/navigation/title_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

namespace {
constexpr int32_t DEFAULT_ANIMATION_DURATION = 200;
constexpr int32_t TITLE_RATIO = 2;

void MountBackButton(const RefPtr<TitleBarNode>& hostNode)
{
    auto titleBarLayoutProperty = hostNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto navigatorNode = AceType::DynamicCast<FrameNode>(hostNode->GetBackButton());
    CHECK_NULL_VOID(navigatorNode);
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) == TitleBarParentType::NAVBAR) {
        auto buttonNode = navigatorNode->GetChildren().front();
        CHECK_NULL_VOID(buttonNode);
        auto backButtonImageNode = AceType::DynamicCast<FrameNode>(buttonNode->GetChildren().front());
        CHECK_NULL_VOID(backButtonImageNode);
        auto backButtonImageLayoutProperty = backButtonImageNode->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(backButtonImageLayoutProperty);
        if (titleBarLayoutProperty->HasNoPixMap() && titleBarLayoutProperty->HasImageSource()) {
            backButtonImageLayoutProperty->UpdateImageSourceInfo(titleBarLayoutProperty->GetImageSourceValue());
        }
        auto navBarNode = AceType::DynamicCast<FrameNode>(hostNode->GetParent());
        CHECK_NULL_VOID(navBarNode);
        auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
        CHECK_NULL_VOID(navBarLayoutProperty);
        auto hideBackButton = navBarLayoutProperty->GetHideBackButtonValue(false);
        auto backButtonLayoutProperty = AceType::DynamicCast<FrameNode>(buttonNode)->GetLayoutProperty();
        CHECK_NULL_VOID(backButtonLayoutProperty);
        backButtonLayoutProperty->UpdateVisibility(hideBackButton ? VisibleType::GONE : VisibleType::VISIBLE);
        backButtonImageNode->MarkModifyDone();
        return;
    }
    if (!titleBarLayoutProperty->HasNoPixMap()) {
        navigatorNode->MarkModifyDone();
        return;
    }
    RefPtr<ImageLayoutProperty> backButtonImageLayoutProperty;
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        backButtonImageLayoutProperty = navigatorNode->GetLayoutProperty<ImageLayoutProperty>();
    } else {
        auto buttonNode = navigatorNode->GetChildren().front();
        CHECK_NULL_VOID(buttonNode);
        auto backButtonImageNode = AceType::DynamicCast<FrameNode>(buttonNode->GetChildren().front());
        CHECK_NULL_VOID(backButtonImageNode);
        backButtonImageLayoutProperty = backButtonImageNode->GetLayoutProperty<ImageLayoutProperty>();
    }
    CHECK_NULL_VOID(backButtonImageLayoutProperty);
    if (titleBarLayoutProperty->HasImageSource()) {
        backButtonImageLayoutProperty->UpdateImageSourceInfo(titleBarLayoutProperty->GetImageSourceValue());
        navigatorNode->MarkModifyDone();
        return;
    }

    if (titleBarLayoutProperty->HasPixelMap()) {
        // TODO: use pixelMap
        navigatorNode->MarkModifyDone();
        return;
    }
}

void MountSubTitle(const RefPtr<TitleBarNode>& hostNode)
{
    CHECK_NULL_VOID(hostNode);
    auto titleBarLayoutProperty = hostNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto subtitleNode = AceType::DynamicCast<FrameNode>(hostNode->GetSubtitle());
    CHECK_NULL_VOID(subtitleNode);
    auto titleLayoutProperty = subtitleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(titleLayoutProperty);

    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI) {
        auto theme = NavigationGetTheme();
        CHECK_NULL_VOID(theme);
        titleLayoutProperty->UpdateAdaptMinFontSize(MIN_ADAPT_SUBTITLE_FONT_SIZE);
        titleLayoutProperty->UpdateAdaptMaxFontSize(theme->GetSubTitleFontSize());
        titleLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
    }

    subtitleNode->MarkModifyDone();
}

} // namespace

void TitleBarPattern::MountTitle(const RefPtr<TitleBarNode>& hostNode)
{
    auto titleBarLayoutProperty = hostNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto titleNode = AceType::DynamicCast<FrameNode>(hostNode->GetTitle());
    CHECK_NULL_VOID(titleNode);
    auto navBarNode = AceType::DynamicCast<NavBarNode>(hostNode->GetParent());
    CHECK_NULL_VOID(navBarNode);
    // if title node is custom node markModifyDone and return
    if (navBarNode->GetPrevTitleIsCustomValue(false)) {
        titleNode->MarkModifyDone();
        return;
    }

    auto titleLayoutProperty = titleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(titleLayoutProperty);

    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    auto currentFontSize = titleLayoutProperty->GetFontSizeValue(Dimension(0));
    auto currentMaxLine = titleLayoutProperty->GetMaxLinesValue(0);
    auto titleMode = titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE);
    if (titleMode == NavigationTitleMode::MINI) {
        if (titleBarLayoutProperty->HasHideBackButton() && titleBarLayoutProperty->GetHideBackButtonValue()) {
            titleLayoutProperty->UpdateFontSize(theme->GetTitleFontSize());
            titleLayoutProperty->UpdateAdaptMaxFontSize(theme->GetTitleFontSize());
        } else {
            titleLayoutProperty->UpdateFontSize(theme->GetTitleFontSizeMin());
            titleLayoutProperty->UpdateAdaptMaxFontSize(theme->GetTitleFontSizeMin());
        }
        UpdateSubTitleOpacity(1.0);
    } else if (titleMode == NavigationTitleMode::FULL) {
        titleLayoutProperty->UpdateFontSize(theme->GetTitleFontSizeBig());
        titleLayoutProperty->UpdateAdaptMaxFontSize(theme->GetTitleFontSizeBig());
        UpdateSubTitleOpacity(1.0);
    } else {
        if (fontSize_.has_value()) {
            titleLayoutProperty->UpdateFontSize(Dimension(fontSize_.value(), DimensionUnit::PX));
            titleLayoutProperty->UpdateAdaptMaxFontSize(Dimension(fontSize_.value(), DimensionUnit::PX));
        } else {
            titleLayoutProperty->UpdateFontSize(theme->GetTitleFontSizeBig());
            titleLayoutProperty->UpdateAdaptMaxFontSize(theme->GetTitleFontSizeBig());
        }
        if (opacity_.has_value()) {
            UpdateSubTitleOpacity(opacity_.value());
        } else {
            UpdateSubTitleOpacity(1.0);
        }
    }
    
    titleLayoutProperty->UpdateAdaptMinFontSize(MIN_ADAPT_TITLE_FONT_SIZE);
    titleLayoutProperty->UpdateHeightAdaptivePolicy(TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST);
    auto maxLines = hostNode->GetSubtitle() ? 1 : TITLEBAR_MAX_LINES;
    titleLayoutProperty->UpdateMaxLines(maxLines);
    if (currentFontSize != titleLayoutProperty->GetFontSizeValue(Dimension(0)) ||
        currentMaxLine != titleLayoutProperty->GetMaxLinesValue(0)) {
        titleNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
    }
    titleNode->MarkModifyDone();
}

void TitleBarPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto hostNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    MountBackButton(hostNode);
    MountTitle(hostNode);
    MountSubTitle(hostNode);
}

void TitleBarPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!panEvent_);

    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        TAG_LOGD(AceLogTag::ACE_NAVIGATION, "Pan event start");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (info.GetInputEventType() == InputEventType::AXIS) {
            return;
        }
        pattern->HandleDragStart(info);
    };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (info.GetInputEventType() == InputEventType::AXIS) {
            return;
        }
        pattern->HandleDragUpdate(info);
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        TAG_LOGD(AceLogTag::ACE_NAVIGATION, "Pan event end mainVelocity: %{public}lf", info.GetMainVelocity());
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (info.GetInputEventType() == InputEventType::AXIS) {
            return;
        }
        pattern->HandleDragEnd(info.GetMainVelocity());
    };

    auto actionCancelTask = [weak = WeakClaim(this)]() {
        TAG_LOGD(AceLogTag::ACE_NAVIGATION, "Pan event cancel");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragEnd(0.0);
    };

    if (panEvent_) {
        gestureHub->RemovePanEvent(panEvent_);
    }

    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    PanDirection panDirection = { .type = PanDirection::VERTICAL };
    gestureHub->AddPanEvent(panEvent_, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void TitleBarPattern::HandleDragStart(const GestureEvent& info)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
    defaultTitleBarHeight_ = titleBarNode->GetGeometryNode()->GetFrameSize().Height();
    SetMaxTitleBarHeight();
    SetTempTitleBarHeight(static_cast<float>(info.GetOffsetY()));
    minTitleOffsetY_ = (static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()) - minTitleHeight_) / 2;
    maxTitleOffsetY_ = initialTitleOffsetY_;
    moveRatio_ = (maxTitleOffsetY_ - minTitleOffsetY_) /
                 (maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
    titleMoveDistance_ = (tempTitleBarHeight_ - defaultTitleBarHeight_) * moveRatio_;
    defaultTitleOffsetY_ = GetTitleOffsetY();
    SetTempTitleOffsetY();
    defaultSubtitleOffsetY_ = GetSubTitleOffsetY();
    SetTempSubTitleOffsetY();
    titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);

    // title font size
    SetDefaultTitleFontSize();
    auto titleBarHeightDiff = maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    auto titleFontSizeDiff = MAX_TITLE_FONT_SIZE - MIN_TITLE_FONT_SIZE;
    fontSizeRatio_ = titleFontSizeDiff.Value() / titleBarHeightDiff;
    auto tempFontSize = static_cast<float>(
        (tempTitleBarHeight_ - defaultTitleBarHeight_) * fontSizeRatio_ + defaultTitleFontSize_.Value());
    UpdateTitleFontSize(Dimension(tempFontSize, DimensionUnit::VP));

    // subTitle Opacity
    SetDefaultSubtitleOpacity();
    opacityRatio_ = 1.0f / titleBarHeightDiff;
    auto tempOpacity =
        static_cast<float>((tempTitleBarHeight_ - defaultTitleBarHeight_) * opacityRatio_ + defaultSubtitleOpacity_);
    UpdateSubTitleOpacity(tempOpacity);
}

void TitleBarPattern::HandleDragUpdate(const GestureEvent& info)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
    SetTempTitleBarHeight(static_cast<float>(info.GetOffsetY()));
    titleMoveDistance_ = (tempTitleBarHeight_ - defaultTitleBarHeight_) * moveRatio_;
    SetTempTitleOffsetY();
    SetTempSubTitleOffsetY();
    titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);

    // title font size
    auto tempFontSize = static_cast<float>(
        (tempTitleBarHeight_ - defaultTitleBarHeight_) * fontSizeRatio_ + defaultTitleFontSize_.Value());
    UpdateTitleFontSize(Dimension(tempFontSize, DimensionUnit::VP));

    // subTitle Opacity
    auto tempOpacity =
        static_cast<float>((tempTitleBarHeight_ - defaultTitleBarHeight_) * opacityRatio_ + defaultSubtitleOpacity_);
    UpdateSubTitleOpacity(tempOpacity);
}

void TitleBarPattern::HandleDragEnd(double dragVelocity)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
}

void TitleBarPattern::ProcessTitleDragStart(float offset)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
    if (springController_ && !springController_->IsStopped()) {
        // clear stop listener before stop
        springController_->ClearStopListeners();
        springController_->Stop();
    }
    if (animator_ && !animator_->IsStopped()) {
        animator_->Stop();
    }

    defaultTitleBarHeight_ = titleBarNode->GetGeometryNode()->GetFrameSize().Height();
    SetMaxTitleBarHeight();
    SetTempTitleBarHeight(offset);
    minTitleOffsetY_ = (static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()) - minTitleHeight_) / 2.0f;
    maxTitleOffsetY_ = initialTitleOffsetY_;
    moveRatio_ = (maxTitleOffsetY_ - minTitleOffsetY_) /
                 (maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
    titleMoveDistance_ = (tempTitleBarHeight_ - defaultTitleBarHeight_) * moveRatio_;
    defaultTitleOffsetY_ = GetTitleOffsetY();
    SetTempTitleOffsetY();
    defaultSubtitleOffsetY_ = GetSubTitleOffsetY();
    SetTempSubTitleOffsetY();
    titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);

    // title font size
    SetDefaultTitleFontSize();
    auto mappedOffset = GetMappedOffset(offset);
    auto tempFontSize = GetFontSize(mappedOffset);
    UpdateTitleFontSize(Dimension(tempFontSize, DimensionUnit::PX));

    // subTitle Opacity
    SetDefaultSubtitleOpacity();
    auto tempOpacity = GetSubtitleOpacity();
    UpdateSubTitleOpacity(tempOpacity);

    dragScrolling_ = true;
}

void TitleBarPattern::ProcessTitleDragUpdate(float offset, float dragOffsetY)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
    SetTitleStyleByOffset(offset);
    if (CanOverDrag_) {
        overDragOffset_ = dragOffsetY + defaultTitleBarHeight_ - maxTitleBarHeight_;
    } else {
        overDragOffset_ = 0.0f;
    }
    if (Positive(overDragOffset_)) {
        UpdateScaleByDragOverDragOffset(overDragOffset_);
    } else {
        overDragOffset_ = 0.0f;
    }
}
void TitleBarPattern::SetTitleStyleByOffset(float offset)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
    SetTempTitleBarHeight(offset);
    titleMoveDistance_ = (tempTitleBarHeight_ - defaultTitleBarHeight_) * moveRatio_;
    SetTempTitleOffsetY();
    SetTempSubTitleOffsetY();
    titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);

    // title font size
    auto mappedOffset = GetMappedOffset(offset);
    fontSize_ = GetFontSize(mappedOffset);
    UpdateTitleFontSize(Dimension(fontSize_.value(), DimensionUnit::PX));

    // subTitle Opacity
    opacity_ = GetSubtitleOpacity();
    UpdateSubTitleOpacity(opacity_.value());
}

void TitleBarPattern::ProcessTitleDragEnd()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }

    dragScrolling_ = false;

    if (Positive(overDragOffset_)) {
        SpringAnimation(overDragOffset_, 0);
        enableAssociatedScroll_ = false;
    }
    if (CanOverDrag_ || isTitleScaleChange_) {
        auto titleMiddleValue =
            (static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()) + maxTitleBarHeight_) / TITLE_RATIO;
        if (LessNotEqual(tempTitleBarHeight_, titleMiddleValue) || NearEqual(tempTitleBarHeight_, titleMiddleValue)) {
            AnimateTo(static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()) - defaultTitleBarHeight_);
            enableAssociatedScroll_ = false;
            return;
        } else if (GreatNotEqual(tempTitleBarHeight_, titleMiddleValue)) {
            AnimateTo(maxTitleBarHeight_ - defaultTitleBarHeight_);
            enableAssociatedScroll_ = false;
            return;
        }
    }
}

float TitleBarPattern::GetSubtitleOpacity()
{
    auto titleBarHeightDiff = maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    opacityRatio_ = 1.0f / titleBarHeightDiff;
    auto tempOpacity = static_cast<float>(
        (tempTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx())) * opacityRatio_ + 0.0f);
    return tempOpacity;
}

float TitleBarPattern::GetFontSize(float offset)
{
    auto titleBarHeight = defaultTitleBarHeight_ + offset;
    auto titleFontSizeDiff = MAX_TITLE_FONT_SIZE - MIN_TITLE_FONT_SIZE;
    auto titleBarHeightDiff = maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    fontSizeRatio_ = titleFontSizeDiff.ConvertToPx() / titleBarHeightDiff;
    auto tempFontSize = static_cast<float>(
        (titleBarHeight - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx())) * fontSizeRatio_ +
        MIN_TITLE_FONT_SIZE.ConvertToPx());
    if (GreatNotEqual(tempFontSize, MAX_TITLE_FONT_SIZE.ConvertToPx())) {
        tempFontSize = MAX_TITLE_FONT_SIZE.ConvertToPx();
    }
    if (LessNotEqual(tempFontSize, MIN_TITLE_FONT_SIZE.ConvertToPx())) {
        tempFontSize = MIN_TITLE_FONT_SIZE.ConvertToPx();
    }
    return tempFontSize;
}

float TitleBarPattern::GetMappedOffset(float offset)
{
    auto titleOffset = offset + defaultTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    auto heightDiff = maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    float moveRatio = Curves::SHARP->MoveInternal(std::clamp(titleOffset / heightDiff, 0.0f, 1.0f));
    auto mappedTitleOffset = moveRatio * heightDiff;
    auto mappedOffset =
        mappedTitleOffset - defaultTitleBarHeight_ + static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    return mappedOffset;
}

void TitleBarPattern::SpringAnimation(float startPos, float endPos)
{
    float mass = 1.0f;        // The move animation spring curve mass is 1.0f
    float stiffness = 228.0f; // The move animation spring curve stiffness is 228.0f
    float damping = 30.0f;    // The move animation spring curve damping is 30.0f
    const RefPtr<SpringProperty> DEFAULT_OVER_SPRING_PROPERTY =
        AceType::MakeRefPtr<SpringProperty>(mass, stiffness, damping);
    if (!springMotion_) {
        springMotion_ = AceType::MakeRefPtr<SpringMotion>(overDragOffset_, 0, 0, DEFAULT_OVER_SPRING_PROPERTY);
    } else {
        springMotion_->Reset(overDragOffset_, 0, 0, DEFAULT_OVER_SPRING_PROPERTY);
        springMotion_->ClearListeners();
    }
    springMotion_->AddListener([weak = AceType::WeakClaim(this)](float value) {
        auto titlebar = weak.Upgrade();
        CHECK_NULL_VOID(titlebar);
        titlebar->SetOverDragOffset(value);
        titlebar->UpdateScaleByDragOverDragOffset(value);
        auto host = titlebar->GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    });

    if (!springController_) {
        springController_ = AceType::MakeRefPtr<Animator>(PipelineBase::GetCurrentContext());
    }
    springController_->ClearStopListeners();
    springController_->PlayMotion(springMotion_);
    springController_->AddStopListener([weak = AceType::WeakClaim(this)]() {
        auto titlebar = weak.Upgrade();
        CHECK_NULL_VOID(titlebar);
        titlebar->ClearDragState();
        auto host = titlebar->GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    });
}

void TitleBarPattern::ClearDragState()
{
    overDragOffset_ = 0.0f;
}

void TitleBarPattern::UpdateScaleByDragOverDragOffset(float overDragOffset)
{
    if (Negative(overDragOffset)) {
        return;
    }
    auto host = GetHost();
    auto navBarNode = AceType::DynamicCast<NavBarNode>(host->GetParent());
    CHECK_NULL_VOID(navBarNode);
    if (navBarNode->GetPrevTitleIsCustomValue(true)) {
        return;
    }
    auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
    CHECK_NULL_VOID(navBarLayoutProperty);
    if (navBarLayoutProperty->GetHideTitleBar().value_or(false)) {
        return;
    }

    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleNode = titleBarNode->GetTitle();
    CHECK_NULL_VOID(titleNode);
    auto title = AceType::DynamicCast<FrameNode>(titleNode);
    TransformScale(overDragOffset, title);
    auto subtitleNode = titleBarNode->GetSubtitle();
    if (subtitleNode) {
        auto subtitle = AceType::DynamicCast<FrameNode>(subtitleNode);
        TransformScale(overDragOffset, subtitle);
    }

    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TitleBarPattern::TransformScale(float overDragOffset, const RefPtr<FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto renderCtx = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderCtx);
    auto offset = std::clamp(overDragOffset, 0.0f, static_cast<float>(MAX_OVER_DRAG_OFFSET.ConvertToPx()));
    auto scaleRatio = offset / static_cast<float>(MAX_OVER_DRAG_OFFSET.ConvertToPx());
    VectorF scaleValue = VectorF(scaleRatio * 0.1f + 1.0f, scaleRatio * 0.1f + 1.0f);
    renderCtx->UpdateTransformScale(scaleValue);
}

void TitleBarPattern::AnimateTo(float offset)
{
    if (!animator_) {
        animator_ = CREATE_ANIMATOR(PipelineBase::GetCurrentContext());
    }
    auto animation = AceType::MakeRefPtr<CurveAnimation<float>>(GetCurrentOffset(), offset, Curves::FAST_OUT_SLOW_IN);
    animation->AddListener([weakScroll = AceType::WeakClaim(this)](float value) {
        auto titlebar = weakScroll.Upgrade();
        CHECK_NULL_VOID(titlebar);
        titlebar->SetTitleStyleByOffset(value);
        auto host = titlebar->GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        auto pipeline = PipelineContext::GetCurrentContext();
        if (pipeline) {
            pipeline->FlushUITasks();
        }
    });
    animator_->ClearInterpolators();
    animator_->AddInterpolator(animation);
    animator_->SetDuration(DEFAULT_ANIMATION_DURATION);
    animator_->Play();
}

void TitleBarPattern::SetMaxTitleBarHeight()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    if (titleBarNode->GetSubtitle()) {
        maxTitleBarHeight_ = static_cast<float>(FULL_DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    } else {
        maxTitleBarHeight_ = static_cast<float>(FULL_SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    }
}

void TitleBarPattern::SetTempTitleBarHeight(float offsetY)
{
    tempTitleBarHeight_ = defaultTitleBarHeight_ + offsetY;
    if (tempTitleBarHeight_ < static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx())) {
        tempTitleBarHeight_ = static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    }
    if (tempTitleBarHeight_ > maxTitleBarHeight_) {
        tempTitleBarHeight_ = maxTitleBarHeight_;
    }
}

void TitleBarPattern::SetTempTitleOffsetY()
{
    tempTitleOffsetY_ = defaultTitleOffsetY_ + titleMoveDistance_;
    if (tempTitleOffsetY_ < minTitleOffsetY_) {
        tempTitleOffsetY_ = minTitleOffsetY_;
    }
    if (tempTitleOffsetY_ > maxTitleOffsetY_) {
        tempTitleOffsetY_ = maxTitleOffsetY_;
    }
}

void TitleBarPattern::SetTempSubTitleOffsetY()
{
    tempSubTitleOffsetY_ = tempTitleOffsetY_ + GetTitleHeight();
    if (tempTitleOffsetY_ < minTitleOffsetY_) {
        tempSubTitleOffsetY_ = minTitleOffsetY_;
    }
    if (tempTitleOffsetY_ > maxTitleOffsetY_) {
        tempSubTitleOffsetY_ = maxTitleOffsetY_;
    }
}

void TitleBarPattern::SetDefaultTitleFontSize()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    CHECK_NULL_VOID(titleBarNode->GetTitle());
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_VOID(titleNode);
    auto textLayoutProperty = titleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    if (defaultTitleBarHeight_ == maxTitleBarHeight_) {
        defaultTitleFontSize_ = textLayoutProperty->GetFontSizeValue(MAX_TITLE_FONT_SIZE);
    } else {
        defaultTitleFontSize_ = textLayoutProperty->GetFontSizeValue(MIN_TITLE_FONT_SIZE);
    }
}

void TitleBarPattern::SetDefaultSubtitleOpacity()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    CHECK_NULL_VOID(titleBarNode->GetSubtitle());
    auto subtitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_VOID(subtitleNode);
    auto context = subtitleNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    if (defaultTitleBarHeight_ == maxTitleBarHeight_) {
        defaultSubtitleOpacity_ = context->GetOpacityValue(1.0f);
    } else {
        defaultSubtitleOpacity_ = context->GetOpacityValue(0.0f);
    }
}

float TitleBarPattern::GetTitleHeight()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_RETURN(titleNode, 0.0f);
    auto geometryNode = titleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetFrameSize().Height();
}

float TitleBarPattern::GetTitleOffsetY()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_RETURN(titleNode, 0.0f);
    auto geometryNode = titleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetMarginFrameOffset().GetY();
}

float TitleBarPattern::GetSubTitleHeight()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto subTitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_RETURN(subTitleNode, 0.0f);
    auto geometryNode = subTitleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetFrameSize().Height();
}

float TitleBarPattern::GetSubTitleOffsetY()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto subTitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_RETURN(subTitleNode, 0.0f);
    auto geometryNode = subTitleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetMarginFrameOffset().GetY();
}

void TitleBarPattern::UpdateTitleFontSize(const Dimension& tempTitleFontSize)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_VOID(titleNode);
    auto textLayoutProperty = titleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    textLayoutProperty->UpdateFontSize(tempTitleFontSize);
    textLayoutProperty->UpdateAdaptMaxFontSize(tempTitleFontSize);
    titleNode->MarkModifyDone();
}

void TitleBarPattern::UpdateSubTitleOpacity(const double& value)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto subTitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_VOID(subTitleNode);
    auto context = subTitleNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateOpacity(value);
}

bool TitleBarPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto titleBarLayoutAlgorithm = DynamicCast<TitleBarLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(titleBarLayoutAlgorithm, false);
    UpdateTitleModeChange();

    initialTitleOffsetY_ = titleBarLayoutAlgorithm->GetInitialTitleOffsetY();
    isInitialTitle_ = titleBarLayoutAlgorithm->IsInitialTitle();
    initialSubtitleOffsetY_ = titleBarLayoutAlgorithm->GetInitialSubtitleOffsetY();
    isInitialSubtitle_ = titleBarLayoutAlgorithm->IsInitialSubtitle();
    minTitleHeight_ = titleBarLayoutAlgorithm->GetMinTitleHeight();
    return true;
}

void TitleBarPattern::UpdateTitleModeChange()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto geometryNode = titleBarNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);

    auto titleBarHeight = geometryNode->GetFrameSize().Height();
    if ((titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::FREE) &&
        !NearZero(maxTitleBarHeight_)) {
        if (titleBarHeight >= maxTitleBarHeight_) {
            titleMode_ = NavigationTitleMode::FULL;
        } else if (NearEqual(titleBarHeight, static_cast<float>(TITLEBAR_HEIGHT_MINI.ConvertToPx()))) {
            titleMode_ = NavigationTitleMode::MINI;
        }
    }
}

void TitleBarPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

void TitleBarPattern::ResetAssociatedScroll()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    defaultTitleBarHeight_ = titleBarNode->GetGeometryNode()->GetFrameSize().Height();
    associatedScrollOffset_ = 0.0f;
    associatedScrollOverSize_ = false;
    defaultTitleOffsetY_ = GetTitleOffsetY();
    associatedScrollOffsetMax_ = 0.0f;
    enableAssociatedScroll_ =
        (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::FREE) &&
        !IsTitleFullStatus();
    SetMaxTitleBarHeight();
}

bool TitleBarPattern::UpdateAssociatedScrollOffset(float offset)
{
    if (!enableAssociatedScroll_) {
        return true;
    }

    associatedScrollOffset_ += offset;
    if (Negative(associatedScrollOffset_)) {
        associatedScrollOffset_ = 0.0f;
    }

    if (GreatNotEqual(associatedScrollOffset_, associatedScrollOffsetMax_)) {
        associatedScrollOffsetMax_ = associatedScrollOffset_;
    }

    if (dragScrolling_) {
        return true;
    }

    auto titleMiddleValue =
        (static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()) + maxTitleBarHeight_) / TITLE_RATIO;
    auto tempTitleBarHeight = defaultTitleBarHeight_ + associatedScrollOffset_;

    if (Positive(offset) && GreatNotEqual(tempTitleBarHeight, maxTitleBarHeight_)) {
        associatedScrollOverSize_ = true;
        CanOverDrag_ = false;
        isOverDrag_ = true;
    }

    float titleBarOffset = associatedScrollOffset_;
    if (Negative(offset)) {
        if (associatedScrollOverSize_) {
            SpringAnimation(associatedScrollOffsetMax_ + defaultTitleBarHeight_ - maxTitleBarHeight_, 0);
            enableAssociatedScroll_ = false;
            return true;
        } else {
            if (GreatNotEqual(associatedScrollOffsetMax_ + defaultTitleBarHeight_, titleMiddleValue)) {
                SetTitleStyleByOffset(maxTitleBarHeight_ - defaultTitleBarHeight_);
                    enableAssociatedScroll_ = false;
            } else {
                AnimateTo(static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()) - defaultTitleBarHeight_);
                enableAssociatedScroll_ = false;
            }
            return true;
        }
    }

    return ProcessTitleAssociatedUpdate(titleBarOffset);
}

bool TitleBarPattern::ProcessTitleAssociatedUpdate(float offset)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, true);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_RETURN(titleBarLayoutProperty, true);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return true;
    }
    SetTitleStyleByOffset(offset);
    if (isOverDrag_) {
        overDragOffset_ = offset + defaultTitleBarHeight_ - maxTitleBarHeight_;
    } else {
        overDragOffset_ = 0.0f;
    }
    if (Positive(overDragOffset_)) {
        UpdateScaleByDragOverDragOffset(overDragOffset_);
        return true;
    } else {
        overDragOffset_ = 0.0f;
    }
    return true;
}
} // namespace OHOS::Ace::NG
