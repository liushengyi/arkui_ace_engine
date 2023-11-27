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

#include "core/components_ng/event/drag_event.h"

#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/interaction/interaction_data.h"
#include "core/common/interaction/interaction_interface.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/gesture_info.h"
#include "core/components_ng/gestures/recognizers/long_press_recognizer.h"
#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/gestures/recognizers/sequenced_recognizer.h"
#include "core/pipeline_ng/pipeline_context.h"

#ifdef ENABLE_DRAG_FRAMEWORK
#include "base/msdp/device_status/interfaces/innerkits/interaction/include/interaction_manager.h"
#include "base/subwindow/subwindow_manager.h"
#include "core/animation/animation_pub.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text_drag/text_drag_base.h"
#include "core/components_ng/pattern/text_drag/text_drag_pattern.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"
#endif // ENABLE_DRAG_FRAMEWORK

#ifdef WEB_SUPPORTED
#include "core/components_ng/pattern/web/web_pattern.h"
#endif // WEB_SUPPORTED

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t PAN_FINGER = 1;
constexpr double PAN_DISTANCE = 5.0;
constexpr int32_t LONG_PRESS_DURATION = 500;
constexpr int32_t PREVIEW_LONG_PRESS_RECONGNIZER = 800;
#ifdef ENABLE_DRAG_FRAMEWORK
constexpr Dimension FILTER_VALUE(0.0f);
constexpr float PIXELMAP_DRAG_SCALE_MULTIPLE = 1.05f;
constexpr int32_t PIXELMAP_ANIMATION_TIME = 800;
constexpr float SCALE_NUMBER = 0.95f;
constexpr int32_t FILTER_TIMES = 250;
constexpr float PIXELMAP_ANIMATION_SCALE = 1.1f;
constexpr int32_t PIXELMAP_ANIMATION_DURATION = 300;
constexpr float SPRING_RESPONSE = 0.416f;
constexpr float SPRING_DAMPING_FRACTION = 0.73f;
constexpr Dimension PIXELMAP_BORDER_RADIUS = 16.0_vp;
#endif // ENABLE_DRAG_FRAMEWORK
} // namespace

DragEventActuator::DragEventActuator(
    const WeakPtr<GestureEventHub>& gestureEventHub, PanDirection direction, int32_t fingers, float distance)
    : gestureEventHub_(gestureEventHub), direction_(direction), fingers_(fingers), distance_(distance)
{
    if (fingers_ < PAN_FINGER) {
        fingers_ = PAN_FINGER;
    }

    if (LessOrEqual(distance_, PAN_DISTANCE)) {
        distance_ = PAN_DISTANCE;
    }

    panRecognizer_ = MakeRefPtr<PanRecognizer>(fingers_, direction_, distance_);
    panRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    longPressRecognizer_ = AceType::MakeRefPtr<LongPressRecognizer>(LONG_PRESS_DURATION, fingers_, false, false);
    longPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    previewLongPressRecognizer_ =
        AceType::MakeRefPtr<LongPressRecognizer>(PREVIEW_LONG_PRESS_RECONGNIZER, fingers_, false, false);
    previewLongPressRecognizer_->SetGestureInfo(MakeRefPtr<GestureInfo>(GestureTypeName::DRAG, true));
    isNotInPreviewState_ = false;
}

void DragEventActuator::StartDragTaskForWeb(const GestureEvent& info)
{
    auto gestureInfo = const_cast<GestureEvent&>(info);
    if (actionStart_) {
        actionStart_(gestureInfo);
    }
}

void DragEventActuator::StartLongPressActionForWeb()
{
    if (!isReceivedLongPress_) {
        LOGW("not received long press action, don't start long press action for web");
        return;
    }
    if (longPressUpdate_) {
        longPressUpdate_(longPressInfo_);
    }
    isReceivedLongPress_ = false;
}

void DragEventActuator::CancelDragForWeb()
{
    if (actionCancel_) {
        actionCancel_();
    }
}

void DragEventActuator::OnCollectTouchTarget(const OffsetF& coordinateOffset, const TouchRestrict& touchRestrict,
    const GetEventTargetImpl& getEventTargetImpl, TouchTestResult& result)
{
    CHECK_NULL_VOID(userCallback_);
    isDragUserReject_ = false;
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
#ifdef ENABLE_DRAG_FRAMEWORK
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    if (dragDropManager->IsDragging() ||
        dragDropManager->IsMsdpDragging() ||
        (!frameNode->IsDraggable() && frameNode->IsCustomerSet())) {
        LOGD("not handle because of dragging now.");
        return;
    }
#endif
    auto actionStart = [weak = WeakClaim(this), this](GestureEvent& info) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("DragEvent panRecognizer onActionStart.");
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto dragDropManager = pipeline->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->ResetDragging(DragDropMgrState::ABOUT_TO_PREVIEW);
#ifdef ENABLE_DRAG_FRAMEWORK
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto renderContext = frameNode->GetRenderContext();
        if (info.GetSourceDevice() != SourceType::MOUSE) {
            if (gestureHub->GetTextDraggable()) {
                if (gestureHub->GetIsTextDraggable()) {
                    SetTextPixelMap(gestureHub);
                }
            } else if (!isNotInPreviewState_) {
                if (gestureHub->GetTextDraggable()) {
                    HideTextAnimation(true, info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
                } else {
                    HideEventColumn();
                    HidePixelMap(true, info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
                    HideFilter();
                    SubwindowManager::GetInstance()->HideMenuNG(false, true);
                }
            }
        }

        if (info.GetSourceDevice() == SourceType::MOUSE) {
            auto pattern = frameNode->GetPattern<TextBase>();
            if (gestureHub->GetTextDraggable() && pattern) {
                if (!pattern->IsSelected() || pattern->GetMouseStatus() == MouseStatus::MOVE) {
                    dragDropManager->ResetDragging();
                    gestureHub->SetIsTextDraggable(false);
                    return;
                }
                if (pattern->BetweenSelectedPosition(info.GetGlobalLocation())) {
                    gestureHub->SetIsTextDraggable(true);
                    if (textDragCallback_) {
                        textDragCallback_(info.GetGlobalLocation());
                    }
                }
            }
        }

#endif // ENABLE_DRAG_FRAMEWORK
       // Trigger drag start event set by user.
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionStart = actuator->userCallback_->GetActionStartEventFunc();
        if (userActionStart) {
            userActionStart(info);
        }
        // Trigger custom drag start event
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionStart = actuator->customCallback_->GetActionStartEventFunc();
        if (customActionStart) {
            customActionStart(info);
        }
    };
    actionStart_ = actionStart;
    panRecognizer_->SetOnActionStart(actionStart);

    auto actionUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("DragEvent panRecognizer onActionUpdate.");
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionUpdate = actuator->userCallback_->GetActionUpdateEventFunc();
        if (userActionUpdate) {
            userActionUpdate(info);
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionUpdate = actuator->customCallback_->GetActionUpdateEventFunc();
        if (customActionUpdate) {
            customActionUpdate(info);
        }
    };
    panRecognizer_->SetOnActionUpdate(actionUpdate);

    auto actionEnd = [weak = WeakClaim(this)](GestureEvent& info) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("DragEvent panRecognizer onActionEnd.");
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionEnd = actuator->userCallback_->GetActionEndEventFunc();
        if (userActionEnd) {
            userActionEnd(info);
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionEnd = actuator->customCallback_->GetActionEndEventFunc();
        if (customActionEnd) {
            customActionEnd(info);
        }
        actuator->SetIsNotInPreviewState(false);
    };
    panRecognizer_->SetOnActionEnd(actionEnd);
#ifdef ENABLE_DRAG_FRAMEWORK
    auto actionCancel = [weak = WeakClaim(this), this]() {
#else
    auto actionCancel = [weak = WeakClaim(this)]() {
#endif // ENABLE_DRAG_FRAMEWORK
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("DragEvent panRecognizer onActionCancel.");
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
#ifdef ENABLE_DRAG_FRAMEWORK
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (!GetIsBindOverlayValue(actuator)) {
            if (gestureHub->GetTextDraggable()) {
                if (gestureHub->GetIsTextDraggable()) {
                    HideTextAnimation();
                }
            } else {
                auto frameNode = gestureHub->GetFrameNode();
                CHECK_NULL_VOID(frameNode);
                auto renderContext = frameNode->GetRenderContext();
                BorderRadiusProperty borderRadius;
                if (renderContext->GetBorderRadius().has_value()) {
                    borderRadius.UpdateWithCheck(renderContext->GetBorderRadius().value());
                }
                borderRadius.multiValued = false;
                AnimationOption option;
                option.SetDuration(PIXELMAP_ANIMATION_DURATION);
                option.SetCurve(Curves::FRICTION);
                AnimationUtils::Animate(
                    option,
                    [renderContext_ = renderContext, borderRadius_ = borderRadius]() {
                        renderContext_->UpdateBorderRadius(borderRadius_);
                    },
                    option.GetOnFinishEvent());
                HideEventColumn();
                HidePixelMap();
                HideFilter();
            }
        } else {
            if (actuator->panRecognizer_->getDeviceType() == SourceType::MOUSE) {
                if (!gestureHub->GetTextDraggable()) {
                    HideEventColumn();
                    HidePixelMap();
                    HideFilter();
                }
            }
        }
        actuator->SetIsNotInPreviewState(false);
#endif // ENABLE_DRAG_FRAMEWORK
        CHECK_NULL_VOID(actuator->userCallback_);
        auto userActionCancel = actuator->userCallback_->GetActionCancelEventFunc();
        if (userActionCancel) {
            userActionCancel();
        }
        CHECK_NULL_VOID(actuator->customCallback_);
        auto customActionCancel = actuator->customCallback_->GetActionCancelEventFunc();
        if (customActionCancel) {
            customActionCancel();
        }
    };
    panRecognizer_->SetIsForDrag(true);
    panRecognizer_->SetMouseDistance(DRAG_PAN_DISTANCE_MOUSE.ConvertToPx());
    actionCancel_ = actionCancel;
    panRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    panRecognizer_->SetOnActionCancel(actionCancel);
#ifdef ENABLE_DRAG_FRAMEWORK
    if (touchRestrict.sourceType == SourceType::MOUSE) {
        std::vector<RefPtr<NGGestureRecognizer>> recognizers { panRecognizer_ };
        SequencedRecognizer_ = AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
        SequencedRecognizer_->RemainChildOnResetStatus();
        SequencedRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
        SequencedRecognizer_->SetGetEventTargetImpl(getEventTargetImpl);
        result.emplace_back(SequencedRecognizer_);
        return;
    }
    auto longPressUpdateValue = [weak = WeakClaim(this)](GestureEvent& info) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("DragEvent longPressRecognizer onActionUpdate.");
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        actuator->SetIsNotInPreviewState(true);
    };
    longPressRecognizer_->SetOnActionUpdate(longPressUpdateValue);
    auto longPressUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("DragEvent previewLongPressRecognizer onActionUpdate.");
        }
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto dragDropManager = pipeline->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        if (dragDropManager->IsAboutToPreview() || dragDropManager->IsDragging()) {
            return;
        }
        auto actuator = weak.Upgrade();
        CHECK_NULL_VOID(actuator);
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        if (gestureHub->GetTextDraggable()) {
            actuator->SetIsNotInPreviewState(false);
            if (gestureHub->GetIsTextDraggable()) {
                actuator->SetTextAnimation(gestureHub, info.GetGlobalLocation());
            }
            return;
        }

        bool isAllowedDrag = actuator->IsAllowedDrag();
        if (!isAllowedDrag) {
            actuator->longPressInfo_ = info;
            actuator->isReceivedLongPress_ = true;
            return;
        }

        actuator->SetFilter(actuator);
        auto manager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(manager);
        actuator->SetIsNotInPreviewState(false);
        actuator->SetPixelMap(actuator);
        auto motion = AceType::MakeRefPtr<ResponsiveSpringMotion>(SPRING_RESPONSE, SPRING_DAMPING_FRACTION, 0);
        auto column = manager->GetPixelMapNode();
        CHECK_NULL_VOID(column);

        auto imageNode = AceType::DynamicCast<FrameNode>(column->GetFirstChild());
        CHECK_NULL_VOID(imageNode);
        auto imageContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageContext);
        if (gestureHub->GetPreviewMode() == MenuPreviewMode::NONE) {
            AnimationOption option;
            option.SetDuration(PIXELMAP_ANIMATION_TIME);
            option.SetCurve(motion);
            AnimationUtils::Animate(
                option,
                [imageContext]() {
                    imageContext->UpdateTransformScale({ PIXELMAP_DRAG_SCALE_MULTIPLE, PIXELMAP_DRAG_SCALE_MULTIPLE });
                },
                option.GetOnFinishEvent());
        } else {
            imageContext->UpdateOpacity(0.0);
        }
        actuator->SetEventColumn(actuator);
    };
    longPressUpdate_ = longPressUpdate;
    previewLongPressRecognizer_->SetOnAction(longPressUpdate);
#endif // ENABLE_DRAG_FRAMEWORK
    previewLongPressRecognizer_->SetGestureHub(gestureEventHub_);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    bool isAllowedDrag = gestureHub->IsAllowedDrag(eventHub);
    if (!longPressRecognizer_->HasThumbnailCallback() && isAllowedDrag) {
        auto callback = [weakPtr = gestureEventHub_](Offset point) {
            auto gestureHub = weakPtr.Upgrade();
            CHECK_NULL_VOID(gestureHub);
            auto frameNode = gestureHub->GetFrameNode();
            CHECK_NULL_VOID(frameNode);
            auto context = frameNode->GetRenderContext();
            CHECK_NULL_VOID(context);
            auto pixelMap = context->GetThumbnailPixelMap();
            gestureHub->SetPixelMap(pixelMap);
        };
        longPressRecognizer_->SetThumbnailCallback(std::move(callback));
    }
    std::vector<RefPtr<NGGestureRecognizer>> recognizers { longPressRecognizer_, panRecognizer_ };
    SequencedRecognizer_ = AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
    SequencedRecognizer_->RemainChildOnResetStatus();
    previewLongPressRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    longPressRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    SequencedRecognizer_->SetCoordinateOffset(Offset(coordinateOffset.GetX(), coordinateOffset.GetY()));
    SequencedRecognizer_->SetGetEventTargetImpl(getEventTargetImpl);
    result.emplace_back(SequencedRecognizer_);
    result.emplace_back(previewLongPressRecognizer_);
}

#ifdef ENABLE_DRAG_FRAMEWORK
void DragEventActuator::SetFilter(const RefPtr<DragEventActuator>& actuator)
{
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent start setFilter.");
    }
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto parent = frameNode->GetParent();
    CHECK_NULL_VOID(parent);
    while (parent && parent->GetDepth() != 1) {
        parent = parent->GetParent();
    }
    if (!parent) {
        if (SystemProperties::GetDebugEnabled()) {
            TAG_LOGW(AceLogTag::ACE_DRAG, "DragFrameNode is %{public}s, depth %{public}d, can not find filter root",
                frameNode->GetTag().c_str(), frameNode->GetDepth());
        }
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (!manager->GetHasFilter() && !manager->GetIsOnAnimation()) {
        if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
#ifdef WEB_SUPPORTED
            auto webPattern = frameNode->GetPattern<WebPattern>();
            CHECK_NULL_VOID(webPattern);
            bool isWebmageDrag = webPattern->IsImageDrag();
            CHECK_NULL_VOID(isWebmageDrag && SystemProperties::GetDeviceType() == DeviceType::PHONE);
#endif
        } else {
            bool isBindOverlayValue = frameNode->GetLayoutProperty()->GetIsBindOverlayValue(false);
            CHECK_NULL_VOID(isBindOverlayValue && SystemProperties::GetDeviceType() == DeviceType::PHONE);
        }
        // insert columnNode to rootNode
        auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        columnNode->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
        // set filter
        LOGI("User Device use default Filter");
        auto container = Container::Current();
        if (container && container->IsScenceBoardWindow()) {
            auto windowScene = manager->FindWindowScene(frameNode);
            manager->MountFilterToWindowScene(columnNode, windowScene);
        } else {
            columnNode->MountToParent(parent);
            columnNode->OnMountToParentDone();
            manager->SetHasFilter(true);
            manager->SetFilterColumnNode(columnNode);
            parent->MarkDirtyNode(NG::PROPERTY_UPDATE_BY_CHILD_REQUEST);
        }
        AnimationOption option;
        BlurStyleOption styleOption;
        styleOption.blurStyle = static_cast<BlurStyle>(BlurStyle::BACKGROUND_THIN);
        styleOption.colorMode = static_cast<ThemeColorMode>(static_cast<int32_t>(ThemeColorMode::SYSTEM));
        option.SetDuration(FILTER_TIMES);
        option.SetCurve(Curves::SHARP);
        columnNode->GetRenderContext()->UpdateBackBlurRadius(FILTER_VALUE);
        AnimationUtils::Animate(
            option, [columnNode, styleOption]() { columnNode->GetRenderContext()->UpdateBackBlurStyle(styleOption); },
            option.GetOnFinishEvent());
    }
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent set filter success.");
    }
}

OffsetF DragEventActuator::GetFloatImageOffset(const RefPtr<FrameNode>& frameNode)
{
    auto offsetToWindow = frameNode->GetPaintRectOffset();
    auto offsetX = offsetToWindow.GetX();
    auto offsetY = offsetToWindow.GetY();
#ifdef WEB_SUPPORTED
    if (frameNode->GetTag() == V2::WEB_ETS_TAG) {
        auto webPattern = frameNode->GetPattern<WebPattern>();
        if (webPattern) {
            offsetX += webPattern->GetDragOffset().GetX();
            offsetY += webPattern->GetDragOffset().GetY();
        }
    }
#endif
    return OffsetF(offsetX, offsetY);
}

void DragEventActuator::SetPixelMap(const RefPtr<DragEventActuator>& actuator)
{
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent start set pixelMap");
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (manager->GetHasPixelMap()) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGW("DragDropManager don't have pixelMap, set pixelMap fail.");
        }
        return;
    }
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    RefPtr<PixelMap> pixelMap = gestureHub->GetPixelMap();
    CHECK_NULL_VOID(pixelMap);
    auto width = pixelMap->GetWidth();
    auto height = pixelMap->GetHeight();
    auto offsetX = GetFloatImageOffset(frameNode).GetX();
    auto offsetY = GetFloatImageOffset(frameNode).GetY();
    // Check web tag.
    if (pipelineContext->HasFloatTitle()) {
        offsetX -= static_cast<float>((CONTAINER_BORDER_WIDTH + CONTENT_PADDING).ConvertToPx());
        offsetY -= static_cast<float>((CONTAINER_TITLE_HEIGHT + CONTAINER_BORDER_WIDTH).ConvertToPx());
    }
    // create imageNode
    auto imageNode = FrameNode::GetOrCreateFrameNode(V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ImagePattern>(); });
    auto renderProps = imageNode->GetPaintProperty<ImageRenderProperty>();
    renderProps->UpdateImageInterpolation(ImageInterpolation::HIGH);
    auto props = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    props->UpdateAutoResize(false);
    props->UpdateImageSourceInfo(ImageSourceInfo(pixelMap));
    auto targetSize = CalcSize(NG::CalcLength(width), NG::CalcLength(height));
    props->UpdateUserDefinedIdealSize(targetSize);
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    imageContext->UpdatePosition(OffsetT<Dimension>(Dimension(offsetX), Dimension(offsetY)));
    ClickEffectInfo clickEffectInfo;
    clickEffectInfo.level = ClickEffectLevel::LIGHT;
    clickEffectInfo.scaleNumber = SCALE_NUMBER;
    imageContext->UpdateClickEffectLevel(clickEffectInfo);
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(imageNode);
    auto hub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(hub);
    hub->SetPixelMap(gestureHub->GetPixelMap());
    // mount to rootNode
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountPixelMapToWindowScene(columnNode, windowScene);
    } else {
        manager->MountPixelMapToRootNode(columnNode);
    }
    imageNode->MarkModifyDone();
    ShowPixelMapAnimation(imageNode);
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent set pixelMap success.");
    }
}

void DragEventActuator::SetEventColumn(const RefPtr<DragEventActuator>& actuator)
{
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent start set eventColumn.");
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    if (manager->GetHasEvent()) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGW("DragDropManager don't have event, set event column fail.");
        }
        return;
    }
    auto rootNode = pipelineContext->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto geometryNode = rootNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto width = geometryNode->GetFrameSize().Width();
    auto height = geometryNode->GetFrameSize().Height();
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto props = columnNode->GetLayoutProperty<LinearLayoutProperty>();
    auto targetSize = CalcSize(NG::CalcLength(width), NG::CalcLength(height));
    props->UpdateUserDefinedIdealSize(targetSize);
    BindClickEvent(columnNode);
    columnNode->MarkModifyDone();
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        auto gestureHub = actuator->gestureEventHub_.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        auto frameNode = gestureHub->GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto windowScene = manager->FindWindowScene(frameNode);
        manager->MountEventToWindowScene(columnNode, windowScene);
    } else {
        manager->MountEventToRootNode(columnNode);
    }
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent set eventColumn success.");
    }
}

void DragEventActuator::HideFilter()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemoveFilterAnimation();
}

void DragEventActuator::HidePixelMap(bool startDrag, double x, double y)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemovePixelMapAnimation(startDrag, x, y);
}

void DragEventActuator::HideEventColumn()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->RemoveEventColumn();
}

void DragEventActuator::BindClickEvent(const RefPtr<FrameNode>& columnNode)
{
    auto callback = [this, weak = WeakClaim(this)](GestureEvent& /* info */) {
        HideEventColumn();
        HidePixelMap();
        HideFilter();
    };
    auto columnGestureHub = columnNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(columnGestureHub);
    auto clickListener = MakeRefPtr<ClickEvent>(std::move(callback));
    columnGestureHub->AddClickEvent(clickListener);
}

void DragEventActuator::ShowPixelMapAnimation(const RefPtr<FrameNode>& imageNode)
{
    auto imageContext = imageNode->GetRenderContext();
    CHECK_NULL_VOID(imageContext);
    // pixel map animation
    AnimationOption option;
    option.SetDuration(PIXELMAP_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    imageContext->UpdateTransformScale({ 1, 1 });
    auto shadow = imageContext->GetBackShadow();
    if (!shadow.has_value()) {
        shadow = Shadow::CreateShadow(ShadowStyle::None);
    }
    imageContext->UpdateBackShadow(shadow.value());

    AnimationUtils::Animate(
        option,
        [imageContext, shadow]() mutable {
            auto color = shadow->GetColor();
            auto newColor = Color::FromARGB(100, color.GetRed(), color.GetGreen(), color.GetBlue());
            shadow->SetColor(newColor);
            imageContext->UpdateBackShadow(shadow.value());
            imageContext->UpdateTransformScale({ PIXELMAP_ANIMATION_SCALE, PIXELMAP_ANIMATION_SCALE });
            BorderRadiusProperty borderRadius;
            borderRadius.SetRadius(PIXELMAP_BORDER_RADIUS);
            imageContext->UpdateBorderRadius(borderRadius);
        },
        option.GetOnFinishEvent());
}

void DragEventActuator::SetThumbnailCallback(std::function<void(Offset)>&& callback)
{
    textDragCallback_ = callback;
    longPressRecognizer_->SetThumbnailCallback(std::move(callback));
}

void DragEventActuator::SetTextPixelMap(const RefPtr<GestureEventHub>& gestureHub)
{
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    CHECK_NULL_VOID(pattern);

    auto dragNode = pattern->MoveDragNode();
    pattern->CloseSelectOverlay();
    CHECK_NULL_VOID(dragNode);
    auto pixelMap = dragNode->GetRenderContext()->GetThumbnailPixelMap();
    if (pixelMap) {
        gestureHub->SetPixelMap(pixelMap);
    } else {
        gestureHub->SetPixelMap(nullptr);
    }
}

void DragEventActuator::SetTextAnimation(const RefPtr<GestureEventHub>& gestureHub, const Offset& globalLocation)
{
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent start set textAnimation.");
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto manager = pipelineContext->GetOverlayManager();
    CHECK_NULL_VOID(manager);
    manager->SetHasFilter(false);
    CHECK_NULL_VOID(gestureHub);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    auto textBase = frameNode->GetPattern<TextBase>();
    CHECK_NULL_VOID(pattern);
    CHECK_NULL_VOID(textBase);
    if (!textBase->BetweenSelectedPosition(globalLocation)) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGW("Position is between selected position, stop set text animation.");
        }
        return;
    }
    pattern->CloseHandleAndSelect();
    auto dragNode = pattern->MoveDragNode();
    CHECK_NULL_VOID(dragNode);
    // create columnNode
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    columnNode->AddChild(dragNode);
    // mount to rootNode
    manager->MountPixelMapToRootNode(columnNode);
    auto modifier = dragNode->GetPattern<TextDragPattern>()->GetOverlayModifier();
    modifier->StartAnimate();
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent set text animation success.");
    }
}

void DragEventActuator::HideTextAnimation(bool startDrag, double globalX, double globalY)
{
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent start hide text animation.");
    }
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_VOID(gestureHub);
    bool isAllowedDrag = IsAllowedDrag();
    if (!gestureHub->GetTextDraggable() || !isAllowedDrag) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGW("Text is not draggable, stop set hide text animation.");
        }
        return;
    }
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextDragBase>();
    CHECK_NULL_VOID(pattern);
    auto removeColumnNode = [id = Container::CurrentId(), startDrag, weakPattern = WeakPtr<TextDragBase>(pattern),
                                weakEvent = gestureEventHub_] {
        ContainerScope scope(id);
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto manager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(manager);
        manager->RemovePixelMap();
        if (!startDrag) {
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->CreateHandles();
        }
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("In removeColumnNode callback, set DragWindowVisible true.");
        }
        InteractionInterface::GetInstance()->SetDragWindowVisible(true);
        auto gestureHub = weakEvent.Upgrade();
        CHECK_NULL_VOID(gestureHub);
        gestureHub->SetPixelMap(nullptr);
    };
    AnimationOption option;
    option.SetDuration(PIXELMAP_ANIMATION_DURATION);
    option.SetCurve(Curves::SHARP);
    option.SetOnFinishEvent(removeColumnNode);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetOverlayManager();
    auto dragNode = manager->GetPixelMapNode();
    CHECK_NULL_VOID(dragNode);
    auto dragFrame = dragNode->GetGeometryNode()->GetFrameRect();
    auto frameWidth = dragFrame.Width();
    auto frameHeight = dragFrame.Height();
    auto pixelMap = gestureHub->GetPixelMap();
    float scale = 1.0f;
    if (pixelMap) {
        scale = gestureHub->GetPixelMapScale(pixelMap->GetHeight(), pixelMap->GetWidth());
    }
    auto context = dragNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateTransformScale(VectorF(1.0f, 1.0f));
    AnimationUtils::Animate(
        option,
        [context, startDrag, globalX, globalY, frameWidth, frameHeight, scale]() {
            if (startDrag) {
                context->UpdatePosition(OffsetT<Dimension>(Dimension(globalX + frameWidth * PIXELMAP_WIDTH_RATE),
                    Dimension(globalY + frameHeight * PIXELMAP_HEIGHT_RATE)));
                context->UpdateTransformScale(VectorF(scale, scale));
                context->OnModifyDone();
            }
        },
        option.GetOnFinishEvent());
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("DragEvent set hide text animation success.");
    }
}
bool DragEventActuator::GetIsBindOverlayValue(const RefPtr<DragEventActuator>& actuator)
{
    auto gestureHub = actuator->gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, true);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, true);
    bool isBindOverlayValue = frameNode->GetLayoutProperty()->GetIsBindOverlayValue(false);
    return isBindOverlayValue;
}

bool DragEventActuator::IsAllowedDrag()
{
    auto gestureHub = gestureEventHub_.Upgrade();
    CHECK_NULL_RETURN(gestureHub, false);
    auto frameNode = gestureHub->GetFrameNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto eventHub = frameNode->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    bool isAllowedDrag = gestureHub->IsAllowedDrag(eventHub);
    return isAllowedDrag;
}
#endif // ENABLE_DRAG_FRAMEWORK
} // namespace OHOS::Ace::NG
