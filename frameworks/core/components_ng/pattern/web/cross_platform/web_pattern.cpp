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

#include "core/components_ng/pattern/web/cross_platform/web_pattern.h"

#include <securec.h>

#include "base/geometry/ng/offset_t.h"
#include "base/mousestyle/mouse_style.h"
#include "base/utils/date_util.h"
#include "base/utils/linear_map.h"
#include "base/utils/utils.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/picker/picker_data.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components/web/web_property.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/event/key_event.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/utils/system_properties.h"

#ifdef ENABLE_DRAG_FRAMEWORK
#include "base/geometry/rect.h"
#include "base/msdp/device_status/interfaces/innerkits/interaction/include/interaction_manager.h"
#include "core/common/ace_engine_ext.h"
#include "core/common/udmf/udmf_client.h"
#include "core/common/udmf/unified_data.h"
#endif

#include "core/components_ng/pattern/web/cross_platform/web_delegate_cross.h"

namespace OHOS::Ace::NG {
#ifdef ENABLE_DRAG_FRAMEWORK
using namespace Msdp::DeviceStatus;
#endif // ENABLE_DRAG_FRAMEWORK

constexpr int32_t SINGLE_CLICK_NUM = 1;
constexpr int32_t DOUBLE_CLICK_NUM = 2;
constexpr double DEFAULT_DBCLICK_INTERVAL = 0.5;
constexpr double DEFAULT_DBCLICK_OFFSET = 2.0;
constexpr double DEFAULT_AXIS_RATIO = -0.06;
constexpr uint32_t DEFAULT_WEB_DRAW_HEIGHT = 4000;
const std::string PATTERN_TYPE_WEB = "WEBPATTERN";
constexpr int32_t SURFACE_QUEUE_SIZE = 8;
// web feature params
const std::string VISIBLE_ACTIVE_ENABLE = "persist.web.visible_active_enable";
const std::string MEMORY_LEVEL_ENABEL = "persist.web.memory_level_enable";
const std::vector<int32_t> DEFAULT_HEIGHT_GEAR {7998, 7999, 8001, 8002, 8003};
const std::vector<int32_t> DEFAULT_ORIGN_GEAR {0, 2000, 4000, 6000, 8000};

WebPattern::WebPattern() = default;

WebPattern::WebPattern(std::string webSrc, const RefPtr<WebController>& webController, WebType type)
    : webSrc_(std::move(webSrc)), webController_(webController), type_(type)
{}

WebPattern::WebPattern(std::string webSrc, const SetWebIdCallback& setWebIdCallback, WebType type)
    : webSrc_(std::move(webSrc)), setWebIdCallback_(setWebIdCallback), type_(type)
{}

WebPattern::~WebPattern()
{
    TAG_LOGD(AceLogTag::ACE_WEB, "Web Pattern destory");
    if (isActive_) {
        OnInActive();
    }
}

void WebPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
    host->GetRenderContext()->UpdateBackgroundColor(Color::WHITE);
    host->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddNodesToNotifyMemoryLevel(host->GetId());
    auto OnAreaChangedCallBack = [weak = WeakClaim(this)](float x, float y, float w, float h) mutable {
        auto webPattern = weak.Upgrade();
        CHECK_NULL_VOID(webPattern);
        auto offset = Offset(webPattern->GetCoordinatePoint()->GetX(), webPattern->GetCoordinatePoint()->GetY());
        auto host = webPattern->GetHost();
        CHECK_NULL_VOID(host);
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->SetContentRectToFrame(
            RectF(offset.GetX(), offset.GetY(), webPattern->drawSize_.Width(), webPattern->drawSize_.Height()));
    };
    host->GetRenderContext()->SetSurfaceChangedCallBack(OnAreaChangedCallBack);
}

void WebPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = false;
    delegate_->OnBlur();

    auto id = frameNode->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveWindowStateChangedCallback(id);
    pipeline->RemoveWindowSizeChangeCallback(id);
    pipeline->RemoveNodesToNotifyMemoryLevel(id);
}

void WebPattern::InitEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);

    InitTouchEvent(gestureHub);
#ifdef ENABLE_DRAG_FRAMEWORK
    InitDragEvent(gestureHub);
#endif
    InitPanEvent(gestureHub);

    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    InitMouseEvent(inputHub);
    InitHoverEvent(inputHub);

    auto focusHub = eventHub->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitFocusEvent(focusHub);

    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto langTask = [weak = AceType::WeakClaim(this)]() {
        auto WebPattern = weak.Upgrade();
        CHECK_NULL_VOID(WebPattern);
        WebPattern->UpdateLocale();
    };
    context->SetConfigChangedCallback(std::move(langTask));
}

void WebPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    if (panEvent_) {
        return;
    }
    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& event) { return; };
    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& event) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragMove(event);
    };
    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) { return; };
    auto actionCancelTask = [weak = WeakClaim(this)]() { return; };
    PanDirection panDirection;
    panDirection.type = PanDirection::VERTICAL;
    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->AddPanEvent(panEvent_, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
#endif
}

#ifdef ENABLE_DRAG_FRAMEWORK
void WebPattern::HandleDragMove(const GestureEvent& event)
{
    if (event.GetInputEventType() == InputEventType::AXIS) {
        CHECK_NULL_VOID(delegate_);
        auto localLocation = event.GetLocalLocation();
        delegate_->HandleAxisEvent(localLocation.GetX(), localLocation.GetY(),
            event.GetDelta().GetX() * DEFAULT_AXIS_RATIO, event.GetDelta().GetY() * DEFAULT_AXIS_RATIO);
    }
}
#endif

void WebPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (touchEvent_) {
        return;
    }

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (info.GetChangedTouches().empty()) {
            return;
        }

        // only handle touch event
        if (info.GetSourceDevice() != SourceType::TOUCH) {
            return;
        }

        pattern->isMouseEvent_ = false;
        const auto& changedPoint = info.GetChangedTouches().front();
        if (changedPoint.GetTouchType() == TouchType::DOWN) {
            pattern->HandleTouchDown(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::MOVE) {
            pattern->HandleTouchMove(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::UP) {
            pattern->HandleTouchUp(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::CANCEL) {
            pattern->HandleTouchCancel(info);
            return;
        }
    };
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void WebPattern::InitMouseEvent(const RefPtr<InputEventHub>& inputHub)
{
    if (mouseEvent_) {
        return;
    }

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleMouseEvent(info);
    };

    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void WebPattern::InitHoverEvent(const RefPtr<InputEventHub>& inputHub)
{
    if (hoverEvent_) {
        return;
    }

    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        MouseInfo info;
        info.SetAction(isHover ? MouseAction::HOVER : MouseAction::HOVER_EXIT);
        pattern->WebOnMouseEvent(info);
    };

    hoverEvent_ = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent_);
}

void WebPattern::HandleMouseEvent(MouseInfo& info)
{
    isMouseEvent_ = true;
    WebOnMouseEvent(info);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto mouseEventCallback = eventHub->GetOnMouseEvent();
    CHECK_NULL_VOID(mouseEventCallback);
    mouseEventCallback(info);
}

void WebPattern::WebOnMouseEvent(const MouseInfo& info)
{}

#ifdef ENABLE_DRAG_FRAMEWORK
void WebPattern::ResetDragAction()
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    frameNode->SetDraggable(false);
    auto eventHub = frameNode->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->ResetDragActionForWeb();

    if (!isDragging_) {
        return;
    }

    isDragging_ = false;
    // cancel drag action to avoid web kernel can't process other input event
    CHECK_NULL_VOID(delegate_);
    delegate_->HandleDragEvent(0, 0, DragAction::DRAG_CANCEL);
    gestureHub->CancelDragForWeb();
}

Offset WebPattern::GetDragOffset() const
{
    Offset webDragOffset;
    int x = 0;
    int y = 0;
    if (delegate_ && delegate_->dragData_) {
        delegate_->dragData_->GetDragStartPosition(x, y);
    }

    webDragOffset.SetX(x);
    webDragOffset.SetY(y);

    TAG_LOGD(AceLogTag::ACE_WEB, "web drag start position = (%{public}d, %{public}d)", x, y);
    return webDragOffset;
}
#endif

bool WebPattern::HandleDoubleClickEvent(const MouseInfo& info)
{
    if (info.GetButton() != MouseButton::LEFT_BUTTON || info.GetAction() != MouseAction::PRESS) {
        return false;
    }
    auto localLocation = info.GetLocalLocation();
    MouseClickInfo clickInfo;
    clickInfo.x = localLocation.GetX();
    clickInfo.y = localLocation.GetY();
    clickInfo.start = info.GetTimeStamp();
    if (doubleClickQueue_.empty()) {
        doubleClickQueue_.push(clickInfo);
        return false;
    }
    std::chrono::duration<float> timeout_ = clickInfo.start - doubleClickQueue_.back().start;
    double offsetX = clickInfo.x - doubleClickQueue_.back().x;
    double offsetY = clickInfo.y - doubleClickQueue_.back().y;
    double offset = sqrt(offsetX * offsetX + offsetY * offsetY);
    if (timeout_.count() < DEFAULT_DBCLICK_INTERVAL && offset < DEFAULT_DBCLICK_OFFSET) {
        SendDoubleClickEvent(clickInfo);
        std::queue<MouseClickInfo> empty;
        swap(empty, doubleClickQueue_);
        return true;
    }
    if (doubleClickQueue_.size() == 1) {
        doubleClickQueue_.push(clickInfo);
        return false;
    }
    doubleClickQueue_.pop();
    doubleClickQueue_.push(clickInfo);
    return false;
}

void WebPattern::SendDoubleClickEvent(const MouseClickInfo& info)
{
    CHECK_NULL_VOID(delegate_);
    delegate_->OnMouseEvent(info.x, info.y, MouseButton::LEFT_BUTTON, MouseAction::PRESS, DOUBLE_CLICK_NUM);
}

#ifdef ENABLE_DRAG_FRAMEWORK
bool WebPattern::GenerateDragDropInfo(NG::DragDropInfo& dragDropInfo)
{
    if (delegate_) {
        dragDropInfo.pixelMap = delegate_->GetDragPixelMap();
    }

    if (dragDropInfo.pixelMap) {
        TAG_LOGI(AceLogTag::ACE_WEB, "get w3c drag info success");
        isW3cDragEvent_ = true;
        return true;
    }

    return false;
}

NG::DragDropInfo WebPattern::HandleOnDragStart(const RefPtr<OHOS::Ace::DragEvent>& info)
{
    isDragging_ = true;
    NG::DragDropInfo dragDropInfo;
    if (GenerateDragDropInfo(dragDropInfo)) {
        auto frameNode = GetHost();
        CHECK_NULL_RETURN(frameNode, dragDropInfo);
        // get drag pixel map successfully, disable next drag util received web kernel drag callback
        frameNode->SetDraggable(false);

        // save dragData to udmf by using setData
        RefPtr<UnifiedData> aceUnifiedData = UdmfClient::GetInstance()->CreateUnifiedData();
        std::string plainContent = delegate_->dragData_->GetFragmentText();
        std::string htmlContent = delegate_->dragData_->GetFragmentHtml();
        std::string linkUrl = delegate_->dragData_->GetLinkURL();
        std::string linkTitle = delegate_->dragData_->GetLinkTitle();
        // plain text
        if (!plainContent.empty()) {
            UdmfClient::GetInstance()->AddPlainTextRecord(aceUnifiedData, plainContent);
        }
        // html
        if (!htmlContent.empty()) {
            UdmfClient::GetInstance()->AddHtmlRecord(aceUnifiedData, htmlContent, "");
            TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub HandleOnDragStart, plainContent size:%{public}d,"
                "htmlContent size:%{public}d", (int)plainContent.size(), (int)htmlContent.size());
        }
        // hyperlink
        if (!linkUrl.empty()) {
            UdmfClient::GetInstance()->AddLinkRecord(aceUnifiedData, linkUrl, linkTitle);
            TAG_LOGD(AceLogTag::ACE_WEB,
                "DragDrop event WebEventHub HandleOnDragStart, linkUrl size:%{public}d", (int)linkUrl.size());
        }
        info->SetData(aceUnifiedData);
        HandleOnDragEnter(info);
        return dragDropInfo;
    }

    return dragDropInfo;
}

void WebPattern::HandleOnDropMove(const RefPtr<OHOS::Ace::DragEvent>& info)
{
    if (!isDragging_) {
        return;
    }

    if (!isW3cDragEvent_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->OnContextMenuHide("");
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto viewScale = pipelineContext->GetViewScale();
    int32_t globalX = static_cast<int32_t>(info->GetX()) * viewScale;
    int32_t globalY = static_cast<int32_t>(info->GetY()) * viewScale;
    auto offset = GetCoordinatePoint();
    globalX = static_cast<int32_t>(globalX - offset.value_or(OffsetF()).GetX());
    globalY = static_cast<int32_t>(globalY - offset.value_or(OffsetF()).GetY());
    delegate_->HandleDragEvent(globalX, globalY, DragAction::DRAG_OVER);
}

void WebPattern::InitCommonDragDropEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);

    isDisableDrag_ = false;
    // disable drag
    frameNode->SetDraggable(false);
    // init common drag drop event
    gestureHub->InitDragDropEvent();
    InitWebEventHubDragDropStart(eventHub);
    InitWebEventHubDragDropEnd(eventHub);
}

void WebPattern::InitWebEventHubDragDropStart(const RefPtr<WebEventHub>& eventHub)
{
    auto onDragStartId = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& info,
                             const std::string& extraParams) -> NG::DragDropInfo {
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub onDragStartId, x:%{public}d, y:%{public}d",
            (int)info->GetX(), (int)info->GetY());
        auto pattern = weak.Upgrade();
        return pattern->HandleOnDragStart(info);
    };

    auto onDragEnterId = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& info,
                             const std::string& extraParams) {
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub onDragEnterId, x:%{public}d, y:%{public}d",
            (int)info->GetX(), (int)info->GetY());

        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->isW3cDragEvent_ = true;
        pattern->isDragging_ = true;
        return pattern->HandleOnDragEnter(info);
    };

    auto onDragMoveId = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& info,
                             const std::string& extraParams) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->isDragging_) {
            return;
        }
        pattern->OnQuickMenuDismissed();

        // update drag status
        info->SetResult(pattern->GetDragAcceptableStatus());

        pattern->HandleOnDropMove(info);
    };
    // set custom OnDragStart function
    eventHub->SetOnDragStart(std::move(onDragStartId));
    eventHub->SetOnDragEnter(std::move(onDragEnterId));
    eventHub->SetOnDragMove(std::move(onDragMoveId));
}

void WebPattern::InitWebEventHubDragDropEnd(const RefPtr<WebEventHub>& eventHub)
{
    auto onDragDropId = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& info,
                             const std::string& extraParams) {
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub onDragDropId, x:%{public}d, y:%{public}d",
            (int)info->GetX(), (int)info->GetY());
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->isDragging_) {
            TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub onDragDropId, isDragging_ false return");
            return;
        }
        pattern->HandleOnDragDrop(info);
    };

    auto onDragLeaveId = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& info,
                             const std::string& extraParams) {
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub onDragLeaveId, x:%{public}d, y:%{public}d",
            (int)info->GetX(), (int)info->GetY());
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleOnDragLeave(info->GetX(), info->GetY());
    };

    auto onDragEndId = [weak = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& info) {
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub onDragEndId, x:%{public}d, y:%{public}d",
            (int)info->GetX(), (int)info->GetY());
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragEnd(info->GetX(), info->GetY());
    };
    // set custom OnDragStart function
    eventHub->SetOnDragEnd(std::move(onDragEndId));
    eventHub->SetOnDragLeave(std::move(onDragLeaveId));
    eventHub->SetOnDrop(std::move(onDragDropId));
}

bool WebPattern::IsImageDrag()
{
    if (delegate_) {
        return delegate_->IsImageDrag();
    }
    return false;
}

DragRet WebPattern::GetDragAcceptableStatus()
{
    OHOS::NWeb::NWebDragData::DragOperation status = delegate_->GetDragAcceptableStatus();
    if (status == OHOS::NWeb::NWebDragData::DragOperation::DRAG_OPERATION_MOVE ||
        status == OHOS::NWeb::NWebDragData::DragOperation::DRAG_OPERATION_COPY ||
        status == OHOS::NWeb::NWebDragData::DragOperation::DRAG_OPERATION_LINK) {
        return DragRet::ENABLE_DROP;
    }
    return DragRet::DISABLE_DROP;
}

bool WebPattern::NotifyStartDragTask()
{
    if (isDisableDrag_) {
        TAG_LOGD(AceLogTag::ACE_WEB, "user has set drag callback, don't process web kernel drag event");
        return false;
    }

    isDragging_ = true;
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, false);
    auto eventHub = frameNode->GetEventHub<WebEventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gestureHub, false);
    // received web kernel drag callback, enable drag
    frameNode->SetDraggable(true);
    gestureHub->SetPixelMap(delegate_->GetDragPixelMap());
    if (!isMouseEvent_) {
        // mouse drag does not need long press action
        gestureHub->StartLongPressActionForWeb();
    }
    gestureHub->StartDragTaskForWeb();
    return true;
}

void WebPattern::InitDragEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (dragEvent_) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto userOnDragStartFunc = eventHub->GetOnDragStart();
    if (userOnDragStartFunc) {
        isDisableDrag_ = true;
        return;
    }

    InitCommonDragDropEvent(gestureHub);

    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        int32_t x = info.GetGlobalPoint().GetX();
        int32_t y = info.GetGlobalPoint().GetY();
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event gestureHub actionStartTask x:%{public}d, y:%{public}d", x, y);
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragStart(x, y);
    };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        return;
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        int32_t x = info.GetGlobalPoint().GetX();
        int32_t y = info.GetGlobalPoint().GetY();
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event gestureHub actionEndTask x:%{public}d, y:%{public}d", x, y);
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragEnd(x, y);
    };

    auto actionCancelTask = [weak = WeakClaim(this)]() {
        TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event gestureHub actionCancelTask ");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragCancel();
    };

    float distance = DEFAULT_PAN_DISTANCE;
    auto context = host->GetContext();
    if (context) {
        distance = static_cast<float>(context->NormalizeToPx(Dimension(DEFAULT_PAN_DISTANCE, DimensionUnit::VP)));
    }

    dragEvent_ = MakeRefPtr<DragEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->SetCustomDragEvent(dragEvent_, { PanDirection::ALL }, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void WebPattern::HandleDragStart(int32_t x, int32_t y)
{
    TAG_LOGD(AceLogTag::ACE_WEB,
        "DragDrop event gestureHub actionStart, isW3cDragEvent_:%{public}d", (int)isW3cDragEvent_);
}

void WebPattern::HandleOnDragEnter(const RefPtr<OHOS::Ace::DragEvent>& info)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "DragDrop event WebEventHub HandleOnDragEnter");
    if (!delegate_) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    int32_t globalX = static_cast<int32_t>(info->GetX());
    int32_t globalY = static_cast<int32_t>(info->GetY());
    auto viewScale = pipelineContext->GetViewScale();
    auto offset = GetCoordinatePoint();
    int32_t localX = static_cast<int32_t>(globalX - offset.value_or(OffsetF()).GetX()) * viewScale;
    int32_t localY = static_cast<int32_t>(globalY - offset.value_or(OffsetF()).GetY()) * viewScale;

    // fake drag data when enter
    delegate_->GetOrCreateDragData();
    // use summary to set fake data
    ClearDragData();
    delegate_->HandleDragEvent(localX, localY, DragAction::DRAG_ENTER);
}

void WebPattern::HandleOnDragDrop(const RefPtr<OHOS::Ace::DragEvent>& info)
{
    isDragging_ = false;
    isW3cDragEvent_ = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto viewScale = pipelineContext->GetViewScale();
    auto offset = GetCoordinatePoint();
    int32_t localX = static_cast<int32_t>(info->GetX() - offset.value_or(OffsetF()).GetX()) * viewScale;
    int32_t localY = static_cast<int32_t>(info->GetY() - offset.value_or(OffsetF()).GetY()) * viewScale;

    RefPtr<UnifiedData> aceData = info->GetData();
    // get data from ace(from udmf), and send it to chromium
    if (aceData && aceData->GetSize() >= 1) {
        TAG_LOGD(AceLogTag::ACE_WEB,
            "DragDrop event WebEventHub onDragDropId, size:%{public}d", (int)aceData->GetSize());
        // plain text
        std::string plain = UdmfClient::GetInstance()->GetSinglePlainTextRecord(aceData);
        if (!plain.empty()) {
            delegate_->dragData_->SetFragmentText(plain);
            TAG_LOGD(AceLogTag::ACE_WEB,
                "DragDrop event WebEventHub onDragDropId, plain size:%{public}d", (int)plain.size());
        }
        // html
        std::string htmlContent;
        std::string plainContent;
        UdmfClient::GetInstance()->GetHtmlRecord(aceData, htmlContent, plainContent);
        if (!htmlContent.empty()) {
            delegate_->dragData_->SetFragmentHtml(htmlContent);
            TAG_LOGD(AceLogTag::ACE_WEB,
                "DragDrop event WebEventHub onDragDropId, htmlContent size:%{public}d", (int)htmlContent.size());
        }
        // hyperlink
        std::string linkUrl;
        std::string linkTitle;
        UdmfClient::GetInstance()->GetLinkRecord(aceData, linkUrl, linkTitle);
        if (!linkUrl.empty()) {
            delegate_->dragData_->SetLinkURL(linkUrl);
            delegate_->dragData_->SetLinkTitle(linkTitle);
            TAG_LOGD(AceLogTag::ACE_WEB,
                "DragDrop event WebEventHub onDragDropId, linkUrl size:%{public}d", (int)linkUrl.size());
        }
    } else {
        TAG_LOGE(AceLogTag::ACE_WEB, "DragDrop event WebEventHub onDragDropId get data failed");
    }

    delegate_->HandleDragEvent(localX, localY, DragAction::DRAG_DROP);
}

void WebPattern::HandleOnDragLeave(int32_t x, int32_t y)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "Web DragDrop event WebEventHub HandleOnDragLeave");
    CHECK_NULL_VOID(delegate_);
    isDragging_ = false;
    isW3cDragEvent_ = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto viewScale = pipelineContext->GetViewScale();
    auto offset = GetCoordinatePoint();
    int32_t localX = static_cast<int32_t>(x - offset.value_or(OffsetF()).GetX());
    int32_t localY = static_cast<int32_t>(y - offset.value_or(OffsetF()).GetY());
    delegate_->HandleDragEvent(localX * viewScale, localY * viewScale, DragAction::DRAG_LEAVE);
}

void WebPattern::HandleDragEnd(int32_t x, int32_t y)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "Web DragDrop event gestureHub END");
    CHECK_NULL_VOID(delegate_);

    isDragging_ = false;
    isW3cDragEvent_ = false;
    ClearDragData();
    delegate_->HandleDragEvent(0, 0, DragAction::DRAG_CANCEL);
}

void WebPattern::HandleDragCancel()
{
    TAG_LOGD(AceLogTag::ACE_WEB, "Web DragDrop event gestureHub web drag cancel");
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    // disable drag
    frameNode->SetDraggable(false);
    CHECK_NULL_VOID(delegate_);
    isDragging_ = false;
    isW3cDragEvent_ = false;
    ClearDragData();
    delegate_->HandleDragEvent(0, 0, DragAction::DRAG_CANCEL);
}

void WebPattern::ClearDragData()
{
    std::string plain = "";
    std::string htmlContent = "";
    std::string linkUrl = "";
    std::string linkTitle = "";
    if (delegate_->dragData_) {
        delegate_->dragData_->SetFragmentText(plain);
        delegate_->dragData_->SetFragmentHtml(htmlContent);
        delegate_->dragData_->SetLinkURL(linkUrl);
        delegate_->dragData_->SetLinkTitle(linkTitle);
    }
}
#endif

void WebPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    auto focusTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleFocusEvent();
    };
    focusHub->SetOnFocusInternal(focusTask);

    auto blurTask = [weak = WeakClaim(this)](const BlurReason& blurReason) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent(blurReason);
    };
    focusHub->SetOnBlurReasonInternal(blurTask);

    auto keyTask = [weak = WeakClaim(this)](const KeyEvent& keyEvent) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->HandleKeyEvent(keyEvent);
    };
    focusHub->SetOnKeyEventInternal(keyTask);
}

void WebPattern::HandleFocusEvent()
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = true;
    if (needOnFocus_) {
        delegate_->OnFocus();
    } else {
        needOnFocus_ = true;
    }
}

void WebPattern::HandleBlurEvent(const BlurReason& blurReason)
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = false;
    if (!selectPopupMenuShowing_) {
        delegate_->OnBlur();
    }
}

bool WebPattern::HandleKeyEvent(const KeyEvent& keyEvent)
{
    bool ret = false;

    auto host = GetHost();
    CHECK_NULL_RETURN(host, ret);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_RETURN(eventHub, ret);

    KeyEventInfo info(keyEvent);
    auto keyEventCallback = eventHub->GetOnKeyEvent();
    if (keyEventCallback) {
        keyEventCallback(info);
    }

    auto preKeyEventCallback = eventHub->GetOnPreKeyEvent();
    if (preKeyEventCallback) {
        ret = preKeyEventCallback(info);
        if (ret) {
            return ret;
        }
    }

    ret = WebOnKeyEvent(keyEvent);
    return ret;
}

bool WebPattern::WebOnKeyEvent(const KeyEvent& keyEvent)
{
    CHECK_NULL_RETURN(delegate_, false);
    return delegate_->OnKeyEvent(static_cast<int32_t>(keyEvent.code), static_cast<int32_t>(keyEvent.action));
}

void WebPattern::WebRequestFocus()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto focusHub = eventHub->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);

    focusHub->RequestFocusImmediately();
}

void WebPattern::UpdateContentOffset(const RefPtr<LayoutWrapper>& dirty)
{
    CHECK_NULL_VOID(dirty);
    auto geometryNode = dirty->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto paddingOffset = geometryNode->GetPaddingOffset();
    auto webContentSize = geometryNode->GetContentSize();
    renderContext->SetBounds(
        paddingOffset.GetX(), paddingOffset.GetY(), webContentSize.Width(), webContentSize.Height());
}

bool WebPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (!config.contentSizeChange || isInWindowDrag_) {
        return false;
    }
    CHECK_NULL_RETURN(delegate_, false);
    CHECK_NULL_RETURN(dirty, false);
    auto geometryNode = dirty->GetGeometryNode();
    auto drawSize = Size(geometryNode->GetContentSize().Width(), geometryNode->GetContentSize().Height());
    if (drawSize.IsInfinite() || drawSize.IsEmpty()) {
        TAG_LOGD(AceLogTag::ACE_WEB, "resize invalid %{public}f, %{public}f", drawSize.Width(), drawSize.Height());
        return false;
    }

    drawSize_ = drawSize;
    drawSizeCache_ = drawSize_;
    auto offset = Offset(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    delegate_->SetBoundsOrResize(drawSize_, offset);
    return false;
}

void WebPattern::OnAreaChangedInner()
{
    auto offset = OffsetF(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    if (webOffset_ == offset) {
        return;
    }
    webOffset_ = offset;
    if (isInWindowDrag_)
        return;
    auto resizeOffset = Offset(offset.GetX(), offset.GetY());
    delegate_->SetBoundsOrResize(drawSize_, resizeOffset);
}

void WebPattern::OnWebSrcUpdate()
{
    if (delegate_ && isUrlLoaded_) {
        delegate_->LoadUrl();
    }
}

void WebPattern::OnWebDataUpdate()
{
    if (delegate_ && isUrlLoaded_) {
        delegate_->LoadDataWithRichText();
    }
}

void WebPattern::OnJsEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateJavaScriptEnabled(value);
    }
}

void WebPattern::OnMediaPlayGestureAccessUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateMediaPlayGestureAccess(value);
    }
}

void WebPattern::OnFileAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateAllowFileAccess(value);
    }
}

void WebPattern::OnOnLineImageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateBlockNetworkImage(!value);
    }
}

void WebPattern::OnDomStorageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateDomStorageEnabled(value);
    }
}

void WebPattern::OnImageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateLoadsImagesAutomatically(value);
    }
}

void WebPattern::OnMixedModeUpdate(MixedModeContent value)
{
    if (delegate_) {
        delegate_->UpdateMixedContentMode(value);
    }
}

void WebPattern::OnZoomAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateSupportZoom(value);
    }
}

void WebPattern::OnGeolocationAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateGeolocationEnabled(value);
    }
}

void WebPattern::OnUserAgentUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateUserAgent(value);
    }
}

void WebPattern::OnCacheModeUpdate(WebCacheMode value)
{
    if (delegate_) {
        delegate_->UpdateCacheMode(value);
    }
}

void WebPattern::OnDarkModeUpdate(WebDarkMode mode)
{
    if (delegate_) {
        delegate_->UpdateDarkMode(mode);
    }
}

void WebPattern::OnForceDarkAccessUpdate(bool access)
{
    if (delegate_) {
        delegate_->UpdateForceDarkAccess(access);
    }
}

void WebPattern::OnAudioResumeIntervalUpdate(int32_t resumeInterval)
{
    if (delegate_) {
        delegate_->UpdateAudioResumeInterval(resumeInterval);
    }
}

void WebPattern::OnAudioExclusiveUpdate(bool audioExclusive)
{
    if (delegate_) {
        delegate_->UpdateAudioExclusive(audioExclusive);
    }
}

void WebPattern::OnOverviewModeAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateOverviewModeEnabled(value);
    }
}

void WebPattern::OnFileFromUrlAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateFileFromUrlEnabled(value);
    }
}

void WebPattern::OnDatabaseAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateDatabaseEnabled(value);
    }
}

void WebPattern::OnTextZoomRatioUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateTextZoomRatio(value);
    }
}

void WebPattern::OnWebDebuggingAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateWebDebuggingAccess(value);
    }
}

void WebPattern::OnPinchSmoothModeEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdatePinchSmoothModeEnabled(value);
    }
}

void WebPattern::OnBackgroundColorUpdate(int32_t value)
{
    UpdateBackgroundColorRightNow(value);
    if (delegate_) {
        delegate_->UpdateBackgroundColor(value);
    }
}

void WebPattern::OnInitialScaleUpdate(float value)
{
    if (delegate_) {
        delegate_->UpdateInitialScale(value);
    }
}

void WebPattern::OnMultiWindowAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateMultiWindowAccess(value);
    }
}

void WebPattern::OnAllowWindowOpenMethodUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateAllowWindowOpenMethod(value);
    }
}

void WebPattern::OnWebCursiveFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebCursiveFont(value);
    }
}

void WebPattern::OnWebFantasyFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebFantasyFont(value);
    }
}

void WebPattern::OnWebFixedFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebFixedFont(value);
    }
}

void WebPattern::OnWebSansSerifFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebSansSerifFont(value);
    }
}

void WebPattern::OnWebSerifFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebSerifFont(value);
    }
}

void WebPattern::OnWebStandardFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebStandardFont(value);
    }
}

void WebPattern::OnDefaultFixedFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateDefaultFixedFontSize(value);
    }
}

void WebPattern::OnDefaultFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateDefaultFontSize(value);
    }
}

void WebPattern::OnMinFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateMinFontSize(value);
    }
}

void WebPattern::OnMinLogicalFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateMinLogicalFontSize(value);
    }
}

void WebPattern::OnBlockNetworkUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateBlockNetwork(value);
    }
}

void WebPattern::OnHorizontalScrollBarAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateHorizontalScrollBarAccess(value);
    }
}

void WebPattern::OnVerticalScrollBarAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateVerticalScrollBarAccess(value);
    }
}

void WebPattern::OnScrollBarColorUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateScrollBarColor(value);
    }
}

void WebPattern::RegistVirtualKeyBoardListener()
{
    if (!needUpdateWeb_) {
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->SetVirtualKeyBoardCallback(
        [weak = AceType::WeakClaim(this)](int32_t width, int32_t height, double keyboard) {
            auto webPattern = weak.Upgrade();
            CHECK_NULL_RETURN(webPattern, false);
            return webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
        });
    needUpdateWeb_ = false;
}

void WebPattern::InitEnhanceSurfaceFlag()
{
    if (SystemProperties::GetExtSurfaceEnabled()) {
        isEnhanceSurface_ = true;
    } else {
        isEnhanceSurface_ = false;
    }
}

void WebPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    // called in each update function.
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);

#if !defined(IOS_PLATFORM) && !defined(ANDROID_PLATFORM)
    RegistVirtualKeyBoardListener();
#endif
    if (!delegate_) {
        // first create case,
#if defined(IOS_PLATFORM) || defined(ANDROID_PLATFORM)
        WeakPtr<PipelineContext> context = WeakPtr<PipelineContext>(PipelineContext::GetCurrentContext());
        delegate_ = AceType::MakeRefPtr<WebDelegateCross>(PipelineContext::GetCurrentContext(), nullptr, "web");
        delegate_->SetNGWebPattern(Claim(this));
        delegate_->CreatePlatformResource(Size(0, 0), Offset(0, 0), context);
        if (setWebIdCallback_) {
            setWebIdCallback_(delegate_->GetWebId());
        }
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        if (setHapPathCallback_) {
            setHapPathCallback_(container->GetHapPath());
        }
        if (onControllerAttachedCallback_) {
            onControllerAttachedCallback_();
        }

#else
        delegate_ = AceType::MakeRefPtr<WebDelegate>(PipelineContext::GetCurrentContext(), nullptr, "");
        CHECK_NULL_VOID(delegate_);
        observer_ = AceType::MakeRefPtr<WebDelegateObserver>(delegate_, PipelineContext::GetCurrentContext());
        CHECK_NULL_VOID(observer_);
        delegate_->SetObserver(observer_);
        delegate_->SetWebType(type_);
        InitEnhanceSurfaceFlag();
        delegate_->SetNGWebPattern(Claim(this));
        delegate_->SetEnhanceSurfaceFlag(isEnhanceSurface_);
        delegate_->SetPopup(isPopup_);
        delegate_->SetParentNWebId(parentNWebId_);
        delegate_->SetBackgroundColor(GetBackgroundColorValue(
            static_cast<int32_t>(renderContext->GetBackgroundColor().value_or(Color::WHITE).GetValue())));
        if (isEnhanceSurface_) {
            auto drawSize = Size(1, 1);
            delegate_->SetDrawSize(drawSize);
            delegate_->InitOHOSWeb(PipelineContext::GetCurrentContext());
        } else {
            auto drawSize = Size(1, 1);
            delegate_->SetDrawSize(drawSize);
            renderSurface_->SetRenderContext(host->GetRenderContext());
            if (type_ == WebType::TEXTURE) {
                renderSurface_->SetIsTexture(true);
                renderSurface_->SetPatternType(PATTERN_TYPE_WEB);
                renderSurface_->SetSurfaceQueueSize(SURFACE_QUEUE_SIZE);
            } else {
                renderSurface_->SetIsTexture(false);
            }
            renderSurface_->InitSurface();
            renderSurface_->UpdateXComponentConfig();
            delegate_->InitOHOSWeb(PipelineContext::GetCurrentContext(), renderSurface_);
        }
#endif
        delegate_->UpdateBackgroundColor(GetBackgroundColorValue(
            static_cast<int32_t>(renderContext->GetBackgroundColor().value_or(Color::WHITE).GetValue())));
        delegate_->UpdateJavaScriptEnabled(GetJsEnabledValue(true));
        delegate_->UpdateBlockNetworkImage(!GetOnLineImageAccessEnabledValue(true));
        delegate_->UpdateAllowFileAccess(GetFileAccessEnabledValue(true));
        delegate_->UpdateLoadsImagesAutomatically(GetImageAccessEnabledValue(true));
        delegate_->UpdateMixedContentMode(GetMixedModeValue(MixedModeContent::MIXED_CONTENT_NEVER_ALLOW));
        delegate_->UpdateSupportZoom(GetZoomAccessEnabledValue(true));
        delegate_->UpdateDomStorageEnabled(GetDomStorageAccessEnabledValue(false));
        delegate_->UpdateGeolocationEnabled(GetGeolocationAccessEnabledValue(true));
        delegate_->UpdateCacheMode(GetCacheModeValue(WebCacheMode::DEFAULT));
        delegate_->UpdateDarkMode(GetDarkModeValue(WebDarkMode::Off));
        delegate_->UpdateForceDarkAccess(GetForceDarkAccessValue(false));
        delegate_->UpdateAudioResumeInterval(GetAudioResumeIntervalValue(-1));
        delegate_->UpdateAudioExclusive(GetAudioExclusiveValue(true));
        delegate_->UpdateOverviewModeEnabled(GetOverviewModeAccessEnabledValue(true));
        delegate_->UpdateFileFromUrlEnabled(GetFileFromUrlAccessEnabledValue(false));
        delegate_->UpdateDatabaseEnabled(GetDatabaseAccessEnabledValue(false));
        delegate_->UpdateTextZoomRatio(GetTextZoomRatioValue(DEFAULT_TEXT_ZOOM_RATIO));
        delegate_->UpdateWebDebuggingAccess(GetWebDebuggingAccessEnabledValue(false));
        delegate_->UpdateMediaPlayGestureAccess(GetMediaPlayGestureAccessValue(true));
        delegate_->UpdatePinchSmoothModeEnabled(GetPinchSmoothModeEnabledValue(false));
        delegate_->UpdateMultiWindowAccess(GetMultiWindowAccessEnabledValue(false));
        delegate_->UpdateWebCursiveFont(GetWebCursiveFontValue(DEFAULT_CURSIVE_FONT_FAMILY));
        delegate_->UpdateWebFantasyFont(GetWebFantasyFontValue(DEFAULT_FANTASY_FONT_FAMILY));
        delegate_->UpdateWebFixedFont(GetWebFixedFontValue(DEFAULT_FIXED_fONT_FAMILY));
        delegate_->UpdateWebSansSerifFont(GetWebSansSerifFontValue(DEFAULT_SANS_SERIF_FONT_FAMILY));
        delegate_->UpdateWebSerifFont(GetWebSerifFontValue(DEFAULT_SERIF_FONT_FAMILY));
        delegate_->UpdateWebStandardFont(GetWebStandardFontValue(DEFAULT_STANDARD_FONT_FAMILY));
        delegate_->UpdateDefaultFixedFontSize(GetDefaultFixedFontSizeValue(DEFAULT_FIXED_FONT_SIZE));
        delegate_->UpdateDefaultFontSize(GetDefaultFontSizeValue(DEFAULT_FONT_SIZE));
        delegate_->UpdateMinFontSize(GetMinFontSizeValue(DEFAULT_MINIMUM_FONT_SIZE));
        delegate_->UpdateMinLogicalFontSize(GetMinLogicalFontSizeValue(DEFAULT_MINIMUM_LOGICAL_FONT_SIZE));
        delegate_->UpdateHorizontalScrollBarAccess(GetHorizontalScrollBarAccessEnabledValue(true));
        delegate_->UpdateVerticalScrollBarAccess(GetVerticalScrollBarAccessEnabledValue(true));
        delegate_->UpdateScrollBarColor(GetScrollBarColorValue(DEFAULT_SCROLLBAR_COLOR));
        if (GetBlockNetwork()) {
            delegate_->UpdateBlockNetwork(GetBlockNetwork().value());
        }
        if (GetUserAgent()) {
            delegate_->UpdateUserAgent(GetUserAgent().value());
        }
        if (GetInitialScale()) {
            delegate_->UpdateInitialScale(GetInitialScale().value());
        }
    }

    // Initialize events such as keyboard, focus, etc.
    InitEvent();

    // Initialize scrollupdate listener
    if (type_ == WebType::TEXTURE) {
        auto task = [this]() {
            InitScrollUpdateListener();
        };
        PostTaskToUI(std::move(task));
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddOnAreaChangeNode(host->GetId());
}

bool WebPattern::ProcessVirtualKeyBoard(int32_t width, int32_t height, double keyboard)
{
    TAG_LOGD(AceLogTag::ACE_WEB,
        "Web ProcessVirtualKeyBoard width=%{public}d height=%{public}d keyboard=%{public}f", width, height, keyboard);
    CHECK_NULL_RETURN(delegate_, false);
    if (!isFocus_) {
        if (isVirtualKeyBoardShow_ == VkState::VK_SHOW) {
            drawSize_.SetSize(drawSizeCache_);
            UpdateWebLayoutSize(width, height);
            isVirtualKeyBoardShow_ = VkState::VK_HIDE;
        }
        return false;
    }
    if (NearZero(keyboard)) {
        drawSize_.SetSize(drawSizeCache_);
        UpdateWebLayoutSize(width, height);
        isVirtualKeyBoardShow_ = VkState::VK_HIDE;
    } else if (isVirtualKeyBoardShow_ != VkState::VK_SHOW) {
        drawSizeCache_.SetSize(drawSize_);
        if (drawSize_.Height() <= (height - keyboard - GetCoordinatePoint()->GetY())) {
            isVirtualKeyBoardShow_ = VkState::VK_SHOW;
            return true;
        }
        if (height - GetCoordinatePoint()->GetY() < keyboard) {
            return true;
        }
        drawSize_.SetHeight(height - keyboard - GetCoordinatePoint()->GetY());
        UpdateWebLayoutSize(width, height);
        isVirtualKeyBoardShow_ = VkState::VK_SHOW;
    }
    return true;
}

void WebPattern::UpdateWebLayoutSize(int32_t width, int32_t height)
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto rect = frameNode->GetGeometryNode()->GetFrameRect();
    auto offset = Offset(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    delegate_->SetBoundsOrResize(drawSize_, offset);
    rect.SetSize(SizeF(drawSize_.Width(), drawSize_.Height()));
    frameNode->GetRenderContext()->SyncGeometryProperties(rect);
    frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->SetRootRect(width, height, 0);
}

void WebPattern::HandleTouchDown(const TouchEventInfo& info, bool fromOverlay)
{
    CHECK_NULL_VOID(delegate_);
    Offset touchOffset = Offset(0, 0);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        TAG_LOGD(AceLogTag::ACE_WEB, "Parse touch event down failed, This event will not be handled.");
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchDown(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
    }
}

void WebPattern::HandleTouchUp(const TouchEventInfo& info, bool fromOverlay)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    ResetDragAction();
#endif
    CHECK_NULL_VOID(delegate_);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        TAG_LOGD(AceLogTag::ACE_WEB, "Parse touch event up failed, This event will not be handled.");
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchUp(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
    }
    if (!touchInfos.empty()) {
        WebRequestFocus();
    }
}

void WebPattern::HandleTouchMove(const TouchEventInfo& info, bool fromOverlay)
{
#ifdef ENABLE_DRAG_FRAMEWORK
    if (isDragging_) {
        return;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(manager);
    if (manager->IsDragged()) {
        return;
    }
#endif
    CHECK_NULL_VOID(delegate_);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        TAG_LOGD(AceLogTag::ACE_WEB, "Parse touch event move failed, This event will not be handled.");
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchMove(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
    }
}

void WebPattern::HandleTouchCancel(const TouchEventInfo& info)
{
    CHECK_NULL_VOID(delegate_);
    delegate_->HandleTouchCancel();
}

bool WebPattern::ParseTouchInfo(const TouchEventInfo& info, std::list<TouchInfo>& touchInfos)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto viewScale = context->GetViewScale();
    if (info.GetChangedTouches().empty()) {
        return false;
    }
    for (const auto& point : info.GetChangedTouches()) {
        TouchInfo touchInfo;
        touchInfo.id = point.GetFingerId();
        const Offset& location = point.GetLocalLocation();
        touchInfo.x = static_cast<float>(location.GetX() * viewScale);
        touchInfo.y = static_cast<float>(location.GetY() * viewScale);
        touchInfos.emplace_back(touchInfo);
    }
    return true;
}

void WebPattern::RequestFullScreen()
{
    isFullScreen_ = true;
}

void WebPattern::ExitFullScreen()
{
    isFullScreen_ = false;
}

std::optional<OffsetF> WebPattern::GetCoordinatePoint()
{
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, std::nullopt);
    return frameNode->GetTransformRelativeOffset();
}

void WebPattern::UpdateLocale()
{
    CHECK_NULL_VOID(delegate_);
    delegate_->UpdateLocale();
}

void WebPattern::OnWindowShow()
{
    if (isWindowShow_ || !isVisible_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->ShowWebView();
    isWindowShow_ = true;
}

void WebPattern::OnWindowHide()
{
    if (!isWindowShow_ || !isVisible_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->HideWebView();
    needOnFocus_ = false;
    isWindowShow_ = false;
}

void WebPattern::OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) {}

void WebPattern::OnCompleteSwapWithNewSize()
{
    if (!isInWindowDrag_ || !isWaiting_)
        return;

    ACE_SCOPED_TRACE("WebPattern::OnCompleteSwapWithNewSize");
    isWaiting_ = false;
}

void WebPattern::OnResizeNotWork()
{
    if (!isInWindowDrag_ || !isWaiting_)
        return;

    ACE_SCOPED_TRACE("WebPattern::OnResizeNotWork");
    isWaiting_ = false;
}

bool WebPattern::OnBackPressed() const
{
    if (!isFullScreen_) {
        return false;
    }

    CHECK_NULL_RETURN(fullScreenExitHandler_, false);
    auto webFullScreenExitHandler = fullScreenExitHandler_->GetHandler();
    CHECK_NULL_RETURN(webFullScreenExitHandler, false);
    webFullScreenExitHandler->ExitFullScreen();
    return true;
}

void WebPattern::SetFullScreenExitHandler(const std::shared_ptr<FullScreenEnterEvent>& fullScreenExitHandler)
{
    fullScreenExitHandler_ = fullScreenExitHandler;
}

void WebPattern::OnInActive()
{
    if (!isActive_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->OnInactive();
    isActive_ = false;
}

void WebPattern::OnActive()
{
    if (isActive_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->OnActive();
    isActive_ = true;
}

void WebPattern::OnVisibleChange(bool isVisible)
{
    if (isVisible_ == isVisible) {
        return;
    }

    isVisible_ = isVisible;
    if (!isVisible_) {
        if (isVisibleActiveEnable_) {
            OnInActive();
        }
    } else {
        if (isVisibleActiveEnable_) {
            OnActive();
        }
    }
}

void WebPattern::UpdateBackgroundColorRightNow(int32_t color)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateBackgroundColor(Color(static_cast<uint32_t>(color)));
}

void WebPattern::OnRootLayerChanged(int width, int height)
{
     // cross platform is not support now;
}

void WebPattern::SetNestedScroll(const NestedScrollOptions& nestedOpt)
{
    // cross platform is not support now;
}

void WebPattern::JavaScriptOnDocumentStart(const ScriptItems& scriptItems)
{
    // cross platform is not support now;
}

void WebPattern::UpdateJavaScriptOnDocumentStart()
{
    // cross platform is not support now;
}

void WebPattern::OnOverScrollModeUpdate(int mode)
{
   // cross platform is not support now;
}


RefPtr<ScrollPattern> WebPattern::SearchParent()
{
    // cross platform is not support now;
    return nullptr;
}

void WebPattern::InitScrollUpdateListener()
{
    // cross platform is not support now;
}

void WebPattern::UpdateScrollOffset(SizeF frameSize)
{
   // cross platform is not support now;
}

void WebPattern::CalculateHorizontalDrawRect(SizeF frameSize)
{
   // cross platform is not support now;
}

void WebPattern::CalculateVerticalDrawRect(SizeF frameSize)
{
    // cross platform is not support now;
}

void WebPattern::PostTaskToUI(const std::function<void()>&& task) const
{
    // cross platform is not support now;
}

void WebPattern::SetDrawRect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    // cross platform is not support now;
}

RefPtr<NodePaintMethod> WebPattern::CreateNodePaintMethod()
{
    // cross platform is not support now;
    return MakeRefPtr<WebPaintMethod>(RenderSurface::Create());
}
} // namespace OHOS::Ace::NG