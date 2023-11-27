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

#include "core/components_ng/event/focus_hub.h"

#include <cinttypes>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/log/dump_log.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components/theme/app_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/ace_event_handler.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

RefPtr<FrameNode> FocusHub::GetFrameNode() const
{
    auto eventHub = eventHub_.Upgrade();
    return eventHub ? eventHub->GetFrameNode() : nullptr;
}

RefPtr<GeometryNode> FocusHub::GetGeometryNode() const
{
    auto frameNode = GetFrameNode();
    return frameNode ? frameNode->GetGeometryNode() : nullptr;
}

std::optional<std::string> FocusHub::GetInspectorKey() const
{
    auto frameNode = GetFrameNode();
    CHECK_NULL_RETURN(frameNode, std::nullopt);
    return frameNode->GetInspectorId();
}

RefPtr<FocusHub> FocusHub::GetParentFocusHub() const
{
    auto frameNode = GetFrameNode();
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto parentNode = frameNode->GetFocusParent();
    return parentNode ? parentNode->GetFocusHub() : nullptr;
}

std::string FocusHub::GetFrameName() const
{
    auto frameNode = GetFrameNode();
    return frameNode ? frameNode->GetTag() : "NULL";
}

int32_t FocusHub::GetFrameId() const
{
    auto frameNode = GetFrameNode();
    return frameNode ? frameNode->GetId() : -1;
}

std::list<RefPtr<FocusHub>>::iterator FocusHub::FlushChildrenFocusHub(std::list<RefPtr<FocusHub>>& focusNodes)
{
    focusNodes.clear();
    std::list<RefPtr<FrameNode>> childrenNode;
    auto frameNode = GetFrameNode();
    if (frameNode) {
        frameNode->GetFocusChildren(childrenNode);
    }
    for (const auto& child : childrenNode) {
        if (child->GetFocusHub()) {
            focusNodes.emplace_back(child->GetFocusHub());
        }
    }
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    if (!lastFocusNode) {
        return focusNodes.end();
    }
    return std::find(focusNodes.begin(), focusNodes.end(), lastFocusNode);
}

bool FocusHub::HandleKeyEvent(const KeyEvent& keyEvent)
{
    if (!IsCurrentFocus()) {
        return false;
    }
    return OnKeyEvent(keyEvent);
}

void FocusHub::DumpFocusTree(int32_t depth)
{
    if (focusType_ == FocusType::NODE) {
        DumpFocusNodeTree(depth);
    } else if (focusType_ == FocusType::SCOPE) {
        DumpFocusScopeTree(depth);
    }
}

void FocusHub::DumpFocusNodeTree(int32_t depth)
{
    if (DumpLog::GetInstance().GetDumpFile()) {
        std::string information = GetFrameName();
        if (IsCurrentFocus()) {
            information += "(Node*)";
        } else {
            information += "(Node)";
        }
        information += (" id:" + std::to_string(GetFrameId()));
        if (!IsFocusable()) {
            information = "(-)" + information;
            information += (" Enabled:" + std::to_string(IsEnabled()) + " Show:" + std::to_string(IsShow()) +
                            " Focusable:" + std::to_string(focusable_) +
                            " ParentFocusable:" + std::to_string(parentFocusable_));
        }
        DumpLog::GetInstance().Print(depth, information, 0);
    }
}

void FocusHub::DumpFocusScopeTree(int32_t depth)
{
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    if (DumpLog::GetInstance().GetDumpFile()) {
        std::string information = GetFrameName();
        if (IsCurrentFocus()) {
            information += "(Scope*)";
        } else {
            information += "(Scope)";
        }
        information += (" id:" + std::to_string(GetFrameId()));
        if (!IsFocusable()) {
            information = "(-)" + information;
            if (!IsFocusableNode()) {
                information += (" Enabled:" + std::to_string(IsEnabled()) + " Show:" + std::to_string(IsShow()) +
                                " Focusable:" + std::to_string(focusable_) +
                                " ParentFocusable:" + std::to_string(parentFocusable_));
            }
        }
        DumpLog::GetInstance().Print(depth, information, static_cast<int32_t>(focusNodes.size()));
    }

    for (const auto& item : focusNodes) {
        item->DumpFocusTree(depth + 1);
    }
}

bool FocusHub::RequestFocusImmediately(bool isJudgeRootTree)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    if (context && context->GetIsFocusingByTab()) {
        if (!IsFocusableByTab()) {
            return false;
        }
    }

    if (IsCurrentFocus()) {
        return true;
    }

    if (!IsFocusableWholePath()) {
        return false;
    }

    if (isJudgeRootTree && !IsOnRootTree()) {
        return false;
    }

    currentFocus_ = true;
    UpdateAccessibilityFocusInfo();

    if (onPreFocusCallback_) {
        onPreFocusCallback_();
    }

    auto parent = GetParentFocusHub();
    if (parent) {
        parent->SwitchFocus(AceType::Claim(this));
    }

    HandleFocus();
    auto mainView = GetCurrentMainView();
    if (mainView && parent) {
        auto mainViewRootScope = mainView->GetMainViewRootScope();
        if (mainViewRootScope && parent == mainViewRootScope) {
            mainView->SetIsViewRootScopeFocused(mainViewRootScope, false);
        }
    }
    return true;
}

void FocusHub::UpdateAccessibilityFocusInfo()
{
    // Need update
}

RefPtr<FocusHub> FocusHub::GetChildMainView()
{
    std::list<RefPtr<FocusHub>> children;
    FlushChildrenFocusHub(children);
    RefPtr<FocusHub> curFocusMainView = nullptr;
    RefPtr<FocusHub> focusableMainView = nullptr;
    for (const auto& child : children) {
        if (!child) {
            continue;
        }
        auto frameName = child->GetFrameName();
        if (frameName == V2::PAGE_ETS_TAG || frameName == V2::DIALOG_ETS_TAG || frameName == V2::MODAL_PAGE_TAG ||
            frameName == V2::MENU_ETS_TAG || frameName == V2::SHEET_PAGE_TAG || frameName == V2::POPUP_ETS_TAG) {
            if (!curFocusMainView && child->IsCurrentFocus()) {
                curFocusMainView = child;
            }
            if (!focusableMainView && child->IsFocusableNode()) {
                focusableMainView = child;
            }
        }
    }
    if (curFocusMainView) {
        return curFocusMainView;
    }
    if (focusableMainView) {
        return focusableMainView;
    }
    for (const auto& child : children) {
        if (!child) {
            continue;
        }
        auto result = child->GetChildMainView();
        if (result) {
            return result;
        }
    }
    return nullptr;
}

RefPtr<FocusHub> FocusHub::GetCurrentMainView()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto rootNode = pipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, nullptr);
    auto rootFocusHub = rootNode->GetFocusHub();
    CHECK_NULL_RETURN(rootFocusHub, nullptr);
    return rootFocusHub->GetChildMainView();
}

RefPtr<FocusHub> FocusHub::GetMainViewRootScope()
{
    auto frameName = GetFrameName();
    int32_t rootScopeDeepth = 0;
    if (frameName == V2::MENU_ETS_TAG) {
        rootScopeDeepth = DEEPTH_OF_MENU;
    } else if (frameName == V2::DIALOG_ETS_TAG) {
        rootScopeDeepth = DEEPTH_OF_DIALOG;
    } else if (frameName == V2::POPUP_ETS_TAG) {
        rootScopeDeepth = DEEPTH_OF_POPUP;
    } else {
        rootScopeDeepth = DEEPTH_OF_PAGE;
    }
    RefPtr<FocusHub> rootScope = AceType::Claim(this);
    for (int32_t i = 0; i < rootScopeDeepth; ++i) {
        CHECK_NULL_RETURN(rootScope, nullptr);
        rootScope = rootScope->GetChildren().front();
    }
    CHECK_NULL_RETURN(rootScope, nullptr);
    if (rootScope->GetFocusType() != FocusType::SCOPE) {
        return rootScope->GetParentFocusHub();
    }
    return rootScope;
}

void FocusHub::LostFocusToViewRoot()
{
    auto mainView = GetCurrentMainView();
    CHECK_NULL_VOID(mainView);
    auto mainViewRootScope = mainView->GetMainViewRootScope();
    CHECK_NULL_VOID(mainViewRootScope);
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Lost focus to view root: %{public}s/%{public}d",
        mainViewRootScope->GetFrameName().c_str(), mainViewRootScope->GetFrameId());
    if (!mainViewRootScope->IsCurrentFocus()) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "View root: %{public}s/%{public}d is not on focusing.",
            mainViewRootScope->GetFrameName().c_str(), mainViewRootScope->GetFrameId());
        return;
    }
    mainView->SetIsViewRootScopeFocused(mainViewRootScope, true);
    auto focusedChild = mainViewRootScope->lastWeakFocusNode_.Upgrade();
    CHECK_NULL_VOID(focusedChild);
    focusedChild->LostFocus();
}

bool FocusHub::HandleFocusOnMainView()
{
    auto viewRootScope = GetMainViewRootScope();
    CHECK_NULL_RETURN(viewRootScope, false);
    if (!viewRootScope->IsCurrentFocus()) {
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "Current view root: %{public}s/%{public}d is not on focusing. Cannot handle focus.",
            viewRootScope->GetFrameName().c_str(), viewRootScope->GetFrameId());
        return false;
    }
    if (viewRootScope->GetFocusDependence() != FocusDependence::SELF) {
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "Current view root: %{public}s/%{public}d is not focus depend self. Do not need handle focus.",
            viewRootScope->GetFrameName().c_str(), viewRootScope->GetFrameId());
        return false;
    }

    TabIndexNodeList tabIndexNodes;
    tabIndexNodes.clear();
    CollectTabIndexNodes(tabIndexNodes);
    if (tabIndexNodes.empty()) {
        // No tabIndex node in current main view. Extend focus from viewRootScope to children.
        SetIsDefaultHasFocused(true);
        SetIsViewRootScopeFocused(viewRootScope, false);
        viewRootScope->OnFocusScope(true);
        return true;
    }

    // First tabIndex node need get focus.
    tabIndexNodes.sort([](std::pair<int32_t, WeakPtr<FocusHub>>& a, std::pair<int32_t, WeakPtr<FocusHub>>& b) {
        return a.first < b.first;
    });
    return GoToFocusByTabNodeIdx(tabIndexNodes, 0);
}

void FocusHub::LostFocus(BlurReason reason)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Node %{public}s/%{public}d lost focus. Lost reason: %{public}d.",
        GetFrameName().c_str(), GetFrameId(), reason);
    if (IsCurrentFocus()) {
        blurReason_ = reason;
        currentFocus_ = false;
        UpdateAccessibilityFocusInfo();
        OnBlur();
    }
}

void FocusHub::LostSelfFocus()
{
    if (IsCurrentFocus()) {
        SetFocusable(false);
        SetFocusable(true);
    }
}

void FocusHub::RemoveSelf(BlurReason reason)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Node %{public}s/%{public}d remove self.", GetFrameName().c_str(), GetFrameId());
    auto parent = GetParentFocusHub();
    if (parent) {
        parent->RemoveChild(AceType::Claim(this), reason);
    } else {
        LostFocus(reason);
    }
}

void FocusHub::RemoveChild(const RefPtr<FocusHub>& focusNode, BlurReason reason)
{
    // Not belong to this focus scope.
    if (!focusNode || focusNode->GetParentFocusHub() != this) {
        return;
    }

    std::list<RefPtr<FocusHub>> focusNodes;
    auto itLastFocusNode = FlushChildrenFocusHub(focusNodes);

    if (focusNode->IsCurrentFocus()) {
        // Try to goto next focus, otherwise goto previous focus.
        if (!GoToNextFocusLinear(FocusStep::TAB) && !GoToNextFocusLinear(FocusStep::SHIFT_TAB)) {
            lastWeakFocusNode_ = nullptr;
            auto mainView = GetCurrentMainView();
            auto mainViewRootScope = mainView ? mainView->GetMainViewRootScope() : nullptr;
            if (mainViewRootScope && mainViewRootScope == AceType::Claim(this)) {
                mainView->SetIsViewRootScopeFocused(mainViewRootScope, true);
            } else {
                RemoveSelf(reason);
            }
        }
        focusNode->LostFocus(reason);
    } else {
        if (itLastFocusNode != focusNodes.end() && (*itLastFocusNode) == focusNode) {
            lastWeakFocusNode_ = nullptr;
        }
    }

    auto it = std::find(focusNodes.begin(), focusNodes.end(), focusNode);
    if (it == focusNodes.end()) {
        return;
    }
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    if (lastFocusNode == focusNode) {
        lastWeakFocusNode_ = nullptr;
    }
}

// Need update RebuildChild function

bool FocusHub::IsFocusable()
{
    if (focusType_ == FocusType::NODE) {
        return IsFocusableNode();
    }
    if (focusType_ == FocusType::SCOPE) {
        return IsFocusableScope();
    }
    return false;
}

bool FocusHub::IsFocusableScope()
{
    if (!IsFocusableNode()) {
        return false;
    }
    if (focusDepend_ == FocusDependence::SELF || focusDepend_ == FocusDependence::AUTO) {
        return true;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    return std::any_of(focusNodes.begin(), focusNodes.end(),
        [](const RefPtr<FocusHub>& focusNode) { return focusNode->IsFocusable(); });
}

bool FocusHub::IsFocusableNode()
{
    return IsEnabled() && IsShow() && focusable_ && parentFocusable_;
}

void FocusHub::SetFocusable(bool focusable)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Set node: %{public}s/%{public}d focusable from %{public}d to %{public}d",
        GetFrameName().c_str(), GetFrameId(), focusable_, focusable);
    if (focusable_ == focusable) {
        return;
    }
    focusable_ = focusable;
    if (!focusable) {
        RemoveSelf(BlurReason::FOCUS_SWITCH);
    }
    RefreshParentFocusable(IsFocusableNode());
}

bool FocusHub::IsEnabled() const
{
    auto eventHub = eventHub_.Upgrade();
    return eventHub ? eventHub->IsEnabled() : true;
}

void FocusHub::SetEnabled(bool enabled)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Set node: %{public}s/%{public}d enabled to %{public}d", GetFrameName().c_str(),
        GetFrameId(), enabled);
    if (!enabled) {
        RemoveSelf(BlurReason::FOCUS_SWITCH);
    }
    RefreshParentFocusable(IsFocusableNode());
}

void FocusHub::SetEnabledNode(bool enabled)
{
    if (!enabled) {
        RefreshFocus();
    }
}

void FocusHub::SetEnabledScope(bool enabled)
{
    SetEnabledNode(enabled);
    RefreshParentFocusable(IsFocusableNode());
}

bool FocusHub::IsShow() const
{
    auto frameNode = GetFrameNode();
    CHECK_NULL_RETURN(frameNode, true);
    bool curIsVisible = frameNode->IsVisible();
    auto parent = frameNode->GetParent();
    while (parent) {
        auto parentFrame = AceType::DynamicCast<FrameNode>(parent);
        if (parentFrame && !parentFrame->IsVisible()) {
            curIsVisible = false;
            break;
        }
        parent = parent->GetParent();
    }
    return curIsVisible;
}

void FocusHub::SetShow(bool show)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Set node: %{public}s/%{public}d show to %{public}d", GetFrameName().c_str(),
        GetFrameId(), show);
    if (!show) {
        RemoveSelf(BlurReason::FOCUS_SWITCH);
    }
    RefreshParentFocusable(IsFocusableNode());
}

void FocusHub::SetShowNode(bool show)
{
    if (!show) {
        RefreshFocus();
    }
}

void FocusHub::SetShowScope(bool show)
{
    SetShowNode(show);
}

bool FocusHub::IsCurrentFocusWholePath()
{
    if (!currentFocus_) {
        return false;
    }
    if (focusType_ == FocusType::NODE) {
        return true;
    }
    if (focusType_ == FocusType::SCOPE) {
        if (focusDepend_ == FocusDependence::SELF || focusDepend_ == FocusDependence::AUTO) {
            return true;
        }
        std::list<RefPtr<FocusHub>> focusNodes;
        auto itLastFocusNode = FlushChildrenFocusHub(focusNodes);
        if (itLastFocusNode == focusNodes.end() || !(*itLastFocusNode)) {
            return false;
        }
        return (*itLastFocusNode)->IsCurrentFocusWholePath();
    }
    return false;
}

void FocusHub::SetIsFocusOnTouch(bool isFocusOnTouch)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Set node: %{public}s/%{public}d focusOnTouch to %{public}d", GetFrameName().c_str(),
        GetFrameId(), isFocusOnTouch);
    if (!focusCallbackEvents_) {
        focusCallbackEvents_ = MakeRefPtr<FocusCallbackEvents>();
    }
    if (focusCallbackEvents_->IsFocusOnTouch().has_value() &&
        focusCallbackEvents_->IsFocusOnTouch().value() == isFocusOnTouch) {
        return;
    }
    focusCallbackEvents_->SetIsFocusOnTouch(isFocusOnTouch);

    auto frameNode = GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto gesture = frameNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);

    if (!isFocusOnTouch && !focusOnTouchListener_) {
        return;
    }
    if (!isFocusOnTouch && focusOnTouchListener_) {
        gesture->RemoveTouchEvent(focusOnTouchListener_);
        return;
    }
    if (!focusOnTouchListener_) {
        auto touchCallback = [weak = WeakClaim(this)](const TouchEventInfo& info) {
            auto focusHub = weak.Upgrade();
            if (focusHub && info.GetTouches().front().GetTouchType() == TouchType::UP) {
                focusHub->RequestFocusImmediately();
            }
        };
        focusOnTouchListener_ = MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
    }
    gesture->AddTouchEvent(focusOnTouchListener_);
}

void FocusHub::SetIsDefaultFocus(bool isDefaultFocus)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Set node: %{public}s/%{public}d defaultFocus to %{public}d", GetFrameName().c_str(),
        GetFrameId(), isDefaultFocus);
    if (!focusCallbackEvents_) {
        focusCallbackEvents_ = MakeRefPtr<FocusCallbackEvents>();
    }
    focusCallbackEvents_->SetIsDefaultFocus(isDefaultFocus);
}
void FocusHub::SetIsDefaultGroupFocus(bool isDefaultGroupFocus)
{
    if (!focusCallbackEvents_) {
        focusCallbackEvents_ = MakeRefPtr<FocusCallbackEvents>();
    }
    focusCallbackEvents_->SetIsDefaultGroupFocus(isDefaultGroupFocus);
}

void FocusHub::RefreshFocus()
{
    if (!IsCurrentFocus()) {
        return;
    }

    // lost current focus and request another focus
    auto parent = GetParentFocusHub();

    // current node is root node
    if (!parent) {
        LostFocus();
        return;
    }
    while (!parent->IsFocusable()) {
        // parent node is root node
        if (!parent->GetParentFocusHub()) {
            parent->LostFocus();
            return;
        }
        parent = parent->GetParentFocusHub();
    }
    parent->LostFocus();
    parent->RequestFocusImmediately();
}

bool FocusHub::OnKeyEvent(const KeyEvent& keyEvent)
{
    if (focusType_ == FocusType::SCOPE) {
        return OnKeyEventScope(keyEvent);
    }
    if (focusType_ == FocusType::NODE) {
        return OnKeyEventNode(keyEvent);
    }
    TAG_LOGW(AceLogTag::ACE_FOCUS, "Current node focus type: %{public}d is invalid.", focusType_);
    return false;
}

bool FocusHub::OnKeyEventNode(const KeyEvent& keyEvent)
{
    ACE_DCHECK(IsCurrentFocus());

    auto retInternal = false;
    auto pipeline = PipelineContext::GetCurrentContext();
    bool isBypassInner = keyEvent.IsKey({ KeyCode::KEY_TAB }) && pipeline && pipeline->IsTabJustTriggerOnKeyEvent();
    if (!isBypassInner && !onKeyEventsInternal_.empty()) {
        retInternal = ProcessOnKeyEventInternal(keyEvent);
    }
    TAG_LOGD(AceLogTag::ACE_FOCUS,
        "OnKeyEventInteral: Node %{public}s/%{public}d consume KeyEvent(code:%{public}d, action:%{public}d) return: "
        "%{public}d",
        GetFrameName().c_str(), GetFrameId(), keyEvent.code, keyEvent.action, retInternal);

    auto info = KeyEventInfo(keyEvent);
    if (pipeline &&
        (pipeline->IsKeyInPressed(KeyCode::KEY_META_LEFT) || pipeline->IsKeyInPressed(KeyCode::KEY_META_RIGHT))) {
        info.SetMetaKey(1);
    }
    auto retCallback = false;
    auto onKeyEventCallback = GetOnKeyCallback();
    if (onKeyEventCallback) {
        onKeyEventCallback(info);
        retCallback = info.IsStopPropagation();
    }
    TAG_LOGD(AceLogTag::ACE_FOCUS,
        "OnKeyEventUser: Node %{public}s/%{public}d consume KeyEvent(code:%{public}d, action:%{public}d) return: "
        "%{public}d",
        GetFrameName().c_str(), GetFrameId(), keyEvent.code, keyEvent.action, retCallback);

    if (!retInternal && !retCallback && keyEvent.action == KeyAction::DOWN) {
        auto ret = false;
        switch (keyEvent.code) {
            case KeyCode::KEY_SPACE:
            case KeyCode::KEY_ENTER:
            case KeyCode::KEY_NUMPAD_ENTER:
                ret = OnClick(keyEvent);
                break;
            default:;
        }
        TAG_LOGD(AceLogTag::ACE_FOCUS,
            "OnClick: Node %{public}s/%{public}d consume KeyEvent(code:%{public}d, action:%{public}d) return: "
            "%{public}d",
            GetFrameName().c_str(), GetFrameId(), keyEvent.code, keyEvent.action, ret);
        return ret;
    }
    return retInternal || retCallback;
}

bool FocusHub::OnKeyEventScope(const KeyEvent& keyEvent)
{
    ACE_DCHECK(IsCurrentFocus());
    std::list<RefPtr<FocusHub>> focusNodes;
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    if (lastFocusNode && lastFocusNode->HandleKeyEvent(keyEvent)) {
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "OnKeyEvent: Node %{public}s/%{public}d will not handle KeyEvent(code:%{public}d, action:%{public}d). "
            "Because its child %{public}s/%{public}d already has consumed this event.",
            GetFrameName().c_str(), GetFrameId(), keyEvent.code, keyEvent.action,
            lastFocusNode->GetFrameName().c_str(), lastFocusNode->GetFrameId());
        return true;
    }

    if (OnKeyEventNode(keyEvent)) {
        return true;
    }

    if (keyEvent.action != KeyAction::DOWN) {
        return false;
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    if (!pipeline->GetIsFocusActive()) {
        return false;
    }
    if (keyEvent.IsKey({ KeyCode::KEY_TAB }) && pipeline->IsTabJustTriggerOnKeyEvent()) {
        return false;
    }

    ScrollToLastFocusIndex();
    if (!CalculatePosition()) {
        return false;
    }

    switch (keyEvent.code) {
        case KeyCode::TV_CONTROL_UP:
            return RequestNextFocus(FocusStep::UP, GetRect());
        case KeyCode::TV_CONTROL_DOWN:
            return RequestNextFocus(FocusStep::DOWN, GetRect());
        case KeyCode::TV_CONTROL_LEFT:
            return RequestNextFocus(FocusStep::LEFT, GetRect());
        case KeyCode::TV_CONTROL_RIGHT:
            return RequestNextFocus(FocusStep::RIGHT, GetRect());
        case KeyCode::KEY_TAB: {
            auto context = NG::PipelineContext::GetCurrentContext();
            bool ret = false;
            if (keyEvent.pressedCodes.size() == 1) {
                context->SetIsFocusingByTab(true);
                ret = RequestNextFocus(FocusStep::TAB, GetRect());
                auto focusParent = GetParentFocusHub();
                if (!focusParent || !focusParent->IsCurrentFocus()) {
                    ret = FocusToHeadOrTailChild(true);
                }
                context->SetIsFocusingByTab(false);
            } else if (keyEvent.IsShiftWith(KeyCode::KEY_TAB)) {
                context->SetIsFocusingByTab(true);
                ret = RequestNextFocus(FocusStep::SHIFT_TAB, GetRect());
                auto focusParent = GetParentFocusHub();
                if (!focusParent || !focusParent->IsCurrentFocus()) {
                    ret = FocusToHeadOrTailChild(false);
                }
                context->SetIsFocusingByTab(false);
            }
            return ret;
        }
        case KeyCode::KEY_MOVE_HOME:
            return RequestNextFocus(FocusStep::LEFT_END, GetRect()) || RequestNextFocus(FocusStep::UP_END, GetRect());
        case KeyCode::KEY_MOVE_END:
            return RequestNextFocus(FocusStep::RIGHT_END, GetRect()) ||
                   RequestNextFocus(FocusStep::DOWN_END, GetRect());
        default:
            return false;
    }
}

void FocusHub::RequestFocus() const
{
    if (IsCurrentFocus()) {
        return;
    }
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->AddDirtyFocus(GetFrameNode());
}

void FocusHub::RequestFocusWithDefaultFocusFirstly()
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus with default focus on node: %{public}s/%{public}d.",
        GetFrameName().c_str(), GetFrameId());
    RefPtr<FocusHub> viewScope;
    if (GetFrameName() == V2::MENU_WRAPPER_ETS_TAG) {
        viewScope = GetChildren().front();
    } else {
        viewScope = Claim(this);
    }
    if (viewScope && viewScope->GetIsViewRootScopeFocused()) {
        auto viewRootScope = viewScope->GetMainViewRootScope();
        if (viewRootScope) {
            viewRootScope->SetFocusDependence(FocusDependence::SELF);
        }
    }
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->AddDirtyDefaultFocus(viewScope ? viewScope->GetFrameNode() : GetFrameNode());
}

bool FocusHub::RequestNextFocus(FocusStep moveStep, const RectF& rect)
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Request next focus on node: %{public}s/%{public}d by step: %{public}d.",
        GetFrameName().c_str(), GetFrameId(), moveStep);
    SetScopeFocusAlgorithm();
    if (!focusAlgorithm_.getNextFocusNode) {
        if (focusAlgorithm_.scopeType == ScopeType::PROJECT_AREA) {
            auto lastFocusNode = lastWeakFocusNode_.Upgrade();
            CHECK_NULL_RETURN(lastFocusNode, false);
            RefPtr<FocusHub> nextFocusHub = nullptr;
            if (IsFocusStepTab(moveStep)) {
                nextFocusHub = lastFocusNode->GetNearestNodeByProjectArea(
                    GetChildren(), moveStep == FocusStep::TAB ? FocusStep::RIGHT : FocusStep::LEFT);
            }
            if (!nextFocusHub) {
                nextFocusHub = lastFocusNode->GetNearestNodeByProjectArea(GetChildren(), moveStep);
            }
            if (!nextFocusHub || nextFocusHub == lastFocusNode) {
                TAG_LOGI(
                    AceLogTag::ACE_FOCUS, "Request next focus failed becase cannot find next node by project area.");
                return false;
            }
            auto ret = TryRequestFocus(nextFocusHub, rect, moveStep);
            TAG_LOGI(AceLogTag::ACE_FOCUS,
                "Request next focus by project area. Next focus node is %{public}s/%{public}d. Return %{public}d",
                nextFocusHub->GetFrameName().c_str(), nextFocusHub->GetFrameId(), ret);
            return ret;
        }
        if (!IsFocusStepTab(moveStep) && focusAlgorithm_.isVertical != IsFocusStepVertical(moveStep)) {
            TAG_LOGI(AceLogTag::ACE_FOCUS,
                "Request next focus failed because direction of node(%{pubic}d) is different with step(%{public}d).",
                focusAlgorithm_.isVertical, moveStep);
            return false;
        }
        auto ret = GoToNextFocusLinear(moveStep, rect);
        TAG_LOGI(AceLogTag::ACE_FOCUS, "Request next focus by default linear algorithm. Return %{public}d.", ret);
        return ret;
    }
    if (!lastWeakFocusNode_.Upgrade()) {
        return false;
    }
    WeakPtr<FocusHub> nextFocusHubWeak;
    focusAlgorithm_.getNextFocusNode(moveStep, lastWeakFocusNode_, nextFocusHubWeak);
    auto nextFocusHub = nextFocusHubWeak.Upgrade();
    if (!nextFocusHub || nextFocusHub == lastWeakFocusNode_.Upgrade()) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "Request next focus failed by custom focus algorithm.");
        return false;
    }
    auto ret = TryRequestFocus(nextFocusHub, rect, moveStep);
    TAG_LOGI(AceLogTag::ACE_FOCUS,
        "Request next focus by custom focus algorithm. Next focus node is %{public}s/%{public}d. Return %{public}d",
        nextFocusHub->GetFrameName().c_str(), nextFocusHub->GetFrameId(), ret);
    return ret;
}

bool FocusHub::FocusToHeadOrTailChild(bool isHead)
{
    if (!IsFocusableWholePath()) {
        return false;
    }
    if (focusType_ != FocusType::SCOPE || (focusType_ == FocusType::SCOPE && focusDepend_ == FocusDependence::SELF)) {
        return RequestFocusImmediately();
    }

    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    bool canChildBeFocused = false;
    if (isHead) {
        canChildBeFocused = std::any_of(focusNodes.begin(), focusNodes.end(),
            [](const RefPtr<FocusHub>& node) { return node->FocusToHeadOrTailChild(true); });
    } else {
        canChildBeFocused = std::any_of(focusNodes.rbegin(), focusNodes.rend(),
            [](const RefPtr<FocusHub>& node) { return node->FocusToHeadOrTailChild(false); });
    }

    if (focusDepend_ == FocusDependence::CHILD) {
        return canChildBeFocused;
    }
    if (focusDepend_ == FocusDependence::AUTO) {
        if (!canChildBeFocused) {
            return RequestFocusImmediately();
        }
        return canChildBeFocused;
    }
    return false;
}

void FocusHub::RefreshParentFocusable(bool focusable)
{
    if (focusType_ != FocusType::SCOPE) {
        return;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    for (auto& item : focusNodes) {
        if (focusable != item->IsParentFocusable()) {
            item->SetParentFocusable(focusable);
            item->RefreshParentFocusable(item->IsFocusableNode());
        }
    }
}

bool FocusHub::OnClick(const KeyEvent& event)
{
    auto onClickCallback = GetOnClickCallback();
    if (onClickCallback) {
        auto info = GestureEvent();
        info.SetTimeStamp(event.timeStamp);
        auto geometryNode = GetGeometryNode();
        CHECK_NULL_RETURN(geometryNode, false);
        auto rect = geometryNode->GetFrameRect();
        info.SetGlobalLocation(Offset((rect.Left() + rect.Right()) / 2, (rect.Top() + rect.Bottom()) / 2));
        info.SetLocalLocation(Offset((rect.Right() - rect.Left()) / 2, (rect.Bottom() - rect.Top()) / 2));
        info.SetSourceDevice(event.sourceType);
        info.SetDeviceId(event.deviceId);
        TAG_LOGD(AceLogTag::ACE_FOCUS, "Do click callback on %{public}s/%{public}d with key event",
            GetFrameName().c_str(), GetFrameId());
        onClickCallback(info);
        return true;
    }
    return false;
}

void FocusHub::SwitchFocus(const RefPtr<FocusHub>& focusNode)
{
    if (focusType_ != FocusType::SCOPE) {
        TAG_LOGW(AceLogTag::ACE_FOCUS, "SwitchFocus: parent focus node is not a scope!");
        return;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);

    auto focusNodeNeedBlur = lastWeakFocusNode_.Upgrade();
    lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(focusNode));

    TAG_LOGD(AceLogTag::ACE_FOCUS, "Switch focus from %{public}s/%{public}d to %{public}s/%{public}d",
        focusNodeNeedBlur ? focusNodeNeedBlur->GetFrameName().c_str() : "NULL",
        focusNodeNeedBlur ? focusNodeNeedBlur->GetFrameId() : -1, focusNode->GetFrameName().c_str(),
        focusNode->GetFrameId());
    if (IsCurrentFocus()) {
        if (focusNodeNeedBlur && focusNodeNeedBlur != focusNode) {
            focusNodeNeedBlur->LostFocus();
        }
    } else {
        RequestFocusImmediately();
    }
}

bool FocusHub::GoToNextFocusLinear(FocusStep step, const RectF& rect)
{
    if (step == FocusStep::NONE) {
        return false;
    }
    bool reverse = !IsFocusStepForward(step);
    if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
        reverse = !reverse;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    auto itNewFocusNode = FlushChildrenFocusHub(focusNodes);
    if (focusNodes.empty()) {
        return false;
    }
    if (itNewFocusNode == focusNodes.end()) {
        itNewFocusNode = focusNodes.begin();
    }
    if (reverse) {
        if (itNewFocusNode == focusNodes.begin()) {
            itNewFocusNode = focusNodes.end();
            return false;
        }
        --itNewFocusNode;

        while (itNewFocusNode != focusNodes.begin()) {
            if (TryRequestFocus(*itNewFocusNode, rect, step)) {
                return true;
            }
            --itNewFocusNode;
        }
        if (itNewFocusNode == focusNodes.begin()) {
            if (TryRequestFocus(*itNewFocusNode, rect, step)) {
                return true;
            }
        }
    } else {
        if (itNewFocusNode != focusNodes.end()) {
            ++itNewFocusNode;
        }
        while (itNewFocusNode != focusNodes.end()) {
            if (TryRequestFocus(*itNewFocusNode, rect, step)) {
                return true;
            }
            ++itNewFocusNode;
        }
    }

    return false;
}

bool FocusHub::TryRequestFocus(const RefPtr<FocusHub>& focusNode, const RectF& rect, FocusStep step)
{
    if (IsFocusStepTab(step) && focusNode->AcceptFocusOfSpecifyChild(step)) {
        return focusNode->RequestFocusImmediately();
    }
    if (rect.IsValid()) {
        RectF childRect;
        if (!CalculateRect(focusNode, childRect) ||
            !focusNode->AcceptFocusByRectOfLastFocus(rect - childRect.GetOffset())) {
            return false;
        }
    }
    return focusNode->RequestFocusImmediately();
}

bool FocusHub::CalculatePosition()
{
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    CHECK_NULL_RETURN(lastFocusNode, false);

    RectF childRect;
    if (!CalculateRect(lastFocusNode, childRect)) {
        return false;
    }

    if (lastFocusNode->IsChild()) {
        auto lastFocusGeometryNode = lastFocusNode->GetGeometryNode();
        CHECK_NULL_RETURN(lastFocusGeometryNode, false);
        RectF rect(childRect.GetOffset(), lastFocusGeometryNode->GetFrameSize());
        lastFocusNode->SetRect(rect);
        SetRect(rect);
    } else {
        SetRect(lastFocusNode->GetRect() + childRect.GetOffset());
    }

    return true;
}

void FocusHub::SetScopeFocusAlgorithm()
{
    auto frame = GetFrameNode();
    CHECK_NULL_VOID(frame);
    auto pattern = frame->GetPattern();
    CHECK_NULL_VOID(pattern);
    focusAlgorithm_ = pattern->GetScopeFocusAlgorithm();
}

void FocusHub::SetLastFocusNodeIndex(const RefPtr<FocusHub>& focusNode)
{
    auto frame = GetFrameNode();
    CHECK_NULL_VOID(frame);
    auto pattern = frame->GetPattern();
    CHECK_NULL_VOID(pattern);
    lastFocusNodeIndex_ = pattern->GetFocusNodeIndex(focusNode);
}

void FocusHub::ScrollToLastFocusIndex() const
{
    if (lastFocusNodeIndex_ == -1) {
        return;
    }
    auto frame = GetFrameNode();
    CHECK_NULL_VOID(frame);
    auto pattern = frame->GetPattern();
    CHECK_NULL_VOID(pattern);
    pattern->ScrollToFocusNodeIndex(lastFocusNodeIndex_);
}

void FocusHub::OnFocus()
{
    if (focusType_ == FocusType::NODE) {
        OnFocusNode();
    } else if (focusType_ == FocusType::SCOPE) {
        OnFocusScope();
    }
}

void FocusHub::OnBlur()
{
    if (focusType_ == FocusType::NODE) {
        OnBlurNode();
    } else if (focusType_ == FocusType::SCOPE) {
        OnBlurScope();
    }
}

void FocusHub::OnFocusNode()
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Node(%{public}s/%{public}d) on focus", GetFrameName().c_str(), GetFrameId());
    if (onFocusInternal_) {
        onFocusInternal_();
    }
    auto onFocusCallback = GetOnFocusCallback();
    if (onFocusCallback) {
        onFocusCallback();
    }
    auto parentFocusHub = GetParentFocusHub();
    if (parentFocusHub) {
        parentFocusHub->SetLastFocusNodeIndex(AceType::Claim(this));
    }
    HandleParentScroll(); // If current focus node has a scroll parent. Handle the scroll event.
    auto pipeline = PipelineContext::GetCurrentContext();
    auto rootNode = pipeline ? pipeline->GetRootElement() : nullptr;
    auto rootFocusHub = rootNode ? rootNode->GetFocusHub() : nullptr;
    if (rootFocusHub && pipeline->GetIsFocusActive()) {
        rootFocusHub->ClearAllFocusState();
        rootFocusHub->PaintAllFocusState();
    }
    auto frameNode = GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    frameNode->OnAccessibilityEvent(AccessibilityEventType::FOCUS);
    CHECK_NULL_VOID(pipeline);
    if (frameNode->GetFocusType() == FocusType::NODE) {
        pipeline->SetFocusNode(frameNode);
    }
}

void FocusHub::OnBlurNode()
{
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Node(%{public}s/%{public}d) on blur by %{public}d", GetFrameName().c_str(),
        GetFrameId(), blurReason_);
    if (onBlurInternal_) {
        onBlurInternal_();
    }
    if (onBlurReasonInternal_) {
        onBlurReasonInternal_(blurReason_);
    }
    auto onBlurCallback = GetOnBlurCallback();
    if (onBlurCallback) {
        onBlurCallback();
    }
    if (blurReason_ != BlurReason::FRAME_DESTROY) {
        ClearFocusState();
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    auto rootNode = pipeline ? pipeline->GetRootElement() : nullptr;
    auto rootFocusHub = rootNode ? rootNode->GetFocusHub() : nullptr;
    if (rootFocusHub && pipeline->GetIsFocusActive()) {
        rootFocusHub->ClearAllFocusState();
        rootFocusHub->PaintAllFocusState();
    }
    auto frameNode = GetFrameNode();
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(pipeline);
    if (frameNode->GetFocusType() == FocusType::NODE && frameNode == pipeline->GetFocusNode()) {
        pipeline->SetFocusNode(nullptr);
    }
}

void FocusHub::CheckFocusStateStyle(bool onFocus)
{
    auto eventHub = eventHub_.Upgrade();
    CHECK_NULL_VOID(eventHub);
    if (onFocus) {
        eventHub->UpdateCurrentUIState(UI_STATE_FOCUSED);
    } else {
        eventHub->ResetCurrentUIState(UI_STATE_FOCUSED);
    }
}

bool FocusHub::HasFocusStateStyle()
{
    auto eventHub = eventHub_.Upgrade();
    CHECK_NULL_RETURN(eventHub, false);
    return eventHub->HasStateStyle(UI_STATE_FOCUSED);
}

void FocusHub::OnFocusScope(bool currentHasFocused)
{
    if (focusDepend_ == FocusDependence::SELF) {
        lastWeakFocusNode_ = nullptr;
        OnFocusNode();
        return;
    }

    std::list<RefPtr<FocusHub>> focusNodes;
    auto itLastFocusNode = FlushChildrenFocusHub(focusNodes);
    bool isAnyChildFocusable = focusNodes.empty()
                                   ? false
                                   : std::any_of(focusNodes.begin(), focusNodes.end(),
                                         [](const RefPtr<FocusHub>& focusNode) { return focusNode->IsFocusable(); });

    if (focusDepend_ == FocusDependence::AUTO && !isAnyChildFocusable) {
        lastWeakFocusNode_ = nullptr;
        OnFocusNode();
        return;
    }

    if ((focusDepend_ == FocusDependence::AUTO || focusDepend_ == FocusDependence::CHILD) && isAnyChildFocusable) {
        auto itFocusNode = itLastFocusNode;
        do {
            if (itLastFocusNode == focusNodes.end()) {
                itLastFocusNode = focusNodes.begin();
                lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*itLastFocusNode));
                if (itLastFocusNode == itFocusNode) {
                    break;
                }
            }
            lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*itLastFocusNode));
            if ((*itLastFocusNode)->RequestFocusImmediately()) {
                if (!currentHasFocused) {
                    OnFocusNode();
                }
                return;
            }
        } while ((++itLastFocusNode) != itFocusNode);

        // Not found any focusable node, clear focus.
        itLastFocusNode = focusNodes.end();
        lastWeakFocusNode_ = nullptr;
    }
}

void FocusHub::OnBlurScope()
{
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    OnBlurNode();
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    if (lastFocusNode) {
        lastFocusNode->LostFocus(blurReason_);
    }
}

bool FocusHub::PaintFocusState(bool isNeedStateStyles)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto frameNode = GetFrameNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    if (!context->GetIsFocusActive() || !IsNeedPaintFocusState()) {
        return false;
    }

    bool stateStylesResult = false;
    if (isNeedStateStyles) {
        // do focus state style.
        CheckFocusStateStyle(true);
        stateStylesResult = true;
    }

    if (focusStyleType_ == FocusStyleType::NONE) {
        return stateStylesResult;
    }

    if (focusStyleType_ == FocusStyleType::CUSTOM_REGION) {
        CHECK_NULL_RETURN(getInnerFocusRectFunc_, false);
        RoundRect focusRectInner;
        focusRectInner.SetRect({ -1, -1, -1, -1 });
        getInnerFocusRectFunc_(focusRectInner);
        if (!focusRectInner.GetRect().IsValid()) {
            return false;
        }
        return PaintInnerFocusState(focusRectInner);
    }

    auto appTheme = context->GetTheme<AppTheme>();
    CHECK_NULL_RETURN(appTheme, false);
    Color paintColor;
    if (HasPaintColor()) {
        paintColor = GetPaintColor();
    } else {
        paintColor = appTheme->GetFocusColor();
    }
    Dimension paintWidth;
    if (HasPaintWidth()) {
        paintWidth = GetPaintWidth();
    } else {
        paintWidth = appTheme->GetFocusWidthVp();
    }

    if (focusStyleType_ == FocusStyleType::CUSTOM_BORDER) {
        if (!HasPaintRect()) {
            return false;
        }
        renderContext->PaintFocusState(GetPaintRect(), paintColor, paintWidth);
        return true;
    }

    Dimension focusPaddingVp = Dimension(0.0, DimensionUnit::VP);
    if (HasFocusPadding()) {
        focusPaddingVp = GetFocusPadding();
    } else {
        if (focusStyleType_ == FocusStyleType::INNER_BORDER) {
            focusPaddingVp = -appTheme->GetFocusWidthVp();
        } else if (focusStyleType_ == FocusStyleType::OUTER_BORDER) {
            focusPaddingVp = appTheme->GetFocusOutPaddingVp();
        }
    }
    if (HasPaintRect()) {
        renderContext->PaintFocusState(GetPaintRect(), focusPaddingVp, paintColor, paintWidth);
    } else {
        renderContext->PaintFocusState(focusPaddingVp, paintColor, paintWidth);
    }
    return true;
}

bool FocusHub::PaintAllFocusState()
{
    if (PaintFocusState()) {
        return true;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    if (lastFocusNode && lastFocusNode->IsCurrentFocus() && lastFocusNode->IsFocusableNode()) {
        return lastFocusNode->PaintAllFocusState();
    }
    if (onPaintFocusStateCallback_) {
        return onPaintFocusStateCallback_();
    }
    return false;
}

bool FocusHub::PaintInnerFocusState(const RoundRect& paintRect)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto frameNode = GetFrameNode();
    CHECK_NULL_RETURN(frameNode, false);
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    if (!context->GetIsFocusActive() || !IsNeedPaintFocusState()) {
        return false;
    }
    auto appTheme = context->GetTheme<AppTheme>();
    CHECK_NULL_RETURN(appTheme, false);
    Color paintColor;
    if (HasPaintColor()) {
        paintColor = GetPaintColor();
    } else {
        paintColor = appTheme->GetFocusColor();
    }
    Dimension paintWidth;
    if (HasPaintWidth()) {
        paintWidth = GetPaintWidth();
    } else {
        paintWidth = appTheme->GetFocusWidthVp();
    }
    renderContext->ClearFocusState();
    renderContext->PaintFocusState(paintRect, paintColor, paintWidth);
    return true;
}

void FocusHub::ClearFocusState(bool isNeedStateStyles)
{
    if (isNeedStateStyles) {
        // check focus state style.
        CheckFocusStateStyle(false);
    }
    if (onClearFocusStateCallback_) {
        onClearFocusStateCallback_();
    }
    if (focusStyleType_ != FocusStyleType::NONE) {
        auto frameNode = GetFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto renderContext = frameNode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->ClearFocusState();
    }
}

void FocusHub::ClearAllFocusState()
{
    ClearFocusState();
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    if (lastFocusNode) {
        lastFocusNode->ClearAllFocusState();
    }
}

bool FocusHub::IsNeedPaintFocusState()
{
    if (currentFocus_ && IsFocusableNode() &&
        (focusDepend_ == FocusDependence::SELF || focusType_ == FocusType::NODE)) {
        return focusStyleType_ != FocusStyleType::NONE || HasFocusStateStyle();
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    auto lastFocusNode = GetLastWeakFocusNode().Upgrade();
    while (lastFocusNode) {
        if (!lastFocusNode->IsCurrentFocus() || !lastFocusNode->IsFocusableNode()) {
            break;
        }
        if (lastFocusNode->GetFocusStyleType() != FocusStyleType::NONE || lastFocusNode->HasFocusStateStyle()) {
            return false;
        }
        focusNodes.clear();
        lastFocusNode->FlushChildrenFocusHub(focusNodes);
        lastFocusNode = lastFocusNode->GetLastWeakFocusNode().Upgrade();
    }
    return focusStyleType_ != FocusStyleType::NONE || HasFocusStateStyle();
}

bool FocusHub::AcceptFocusOfSpecifyChild(FocusStep step)
{
    if (focusType_ == FocusType::NODE) {
        return IsFocusable();
    }
    if (focusType_ != FocusType::SCOPE || !IsFocusableScope()) {
        return false;
    }
    if (focusDepend_ == FocusDependence::SELF) {
        return true;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    bool canChildBeFocused = false;
    if (!focusNodes.empty()) {
        if (step == FocusStep::TAB) {
            auto iterNewFocusNode = focusNodes.begin();
            while (iterNewFocusNode != focusNodes.end()) {
                if (*iterNewFocusNode && (*iterNewFocusNode)->AcceptFocusOfSpecifyChild(step)) {
                    lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*iterNewFocusNode));
                    canChildBeFocused = true;
                    break;
                }
                ++iterNewFocusNode;
            }
        } else if (step == FocusStep::SHIFT_TAB) {
            auto iterNewFocusNode = focusNodes.rbegin();
            while (iterNewFocusNode != focusNodes.rend()) {
                if (*iterNewFocusNode && (*iterNewFocusNode)->AcceptFocusOfSpecifyChild(step)) {
                    lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*iterNewFocusNode));
                    canChildBeFocused = true;
                    break;
                }
                ++iterNewFocusNode;
            }
        }
    }
    if (focusDepend_ == FocusDependence::CHILD) {
        return canChildBeFocused;
    }
    return focusDepend_ == FocusDependence::AUTO;
}

bool FocusHub::AcceptFocusOfLastFocus()
{
    if (focusType_ == FocusType::SCOPE) {
        auto lastFocusNode = lastWeakFocusNode_.Upgrade();
        return lastFocusNode ? lastFocusNode->AcceptFocusOfLastFocus() : false;
    }
    if (focusType_ == FocusType::NODE) {
        return IsFocusable();
    }
    return false;
}

bool FocusHub::AcceptFocusByRectOfLastFocus(const RectF& rect)
{
    if (focusType_ == FocusType::NODE) {
        return AcceptFocusByRectOfLastFocusNode(rect);
    }
    if (focusType_ == FocusType::SCOPE) {
        return AcceptFocusByRectOfLastFocusFlex(rect);
    }
    return false;
}

bool FocusHub::AcceptFocusByRectOfLastFocusNode(const RectF& rect)
{
    return IsFocusable();
}

bool FocusHub::AcceptFocusByRectOfLastFocusScope(const RectF& rect)
{
    std::list<RefPtr<FocusHub>> focusNodes;
    auto itLastFocusNode = FlushChildrenFocusHub(focusNodes);
    if (focusNodes.empty()) {
        return false;
    }
    auto itFocusNode = itLastFocusNode;
    do {
        if (itLastFocusNode == focusNodes.end()) {
            itLastFocusNode = focusNodes.begin();
            lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*itLastFocusNode));
            if (itLastFocusNode == itFocusNode) {
                break;
            }
        }
        RectF childRect;
        if (!CalculateRect(*itLastFocusNode, childRect)) {
            continue;
        }

        if ((*itLastFocusNode)->AcceptFocusByRectOfLastFocus(rect - childRect.GetOffset())) {
            lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*itLastFocusNode));
            return true;
        }
    } while ((++itLastFocusNode) != itFocusNode);
    if (itLastFocusNode == focusNodes.end()) {
        lastWeakFocusNode_ = nullptr;
    } else {
        lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*itLastFocusNode));
    }

    return false;
}

bool FocusHub::AcceptFocusByRectOfLastFocusFlex(const RectF& rect)
{
    if (!rect.IsValid()) {
        return false;
    }

    if (focusType_ != FocusType::SCOPE || !IsFocusableScope()) {
        return false;
    }
    if (focusDepend_ == FocusDependence::SELF) {
        return true;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    bool canChildBeFocused = false;
    OffsetF offset;
    auto itNewFocusNode = focusNodes.end();
    double minVal = std::numeric_limits<double>::max();
    for (auto it = focusNodes.begin(); it != focusNodes.end(); ++it) {
        if (!(*it)->IsFocusable()) {
            continue;
        }

        RectF childRect;
        if (!CalculateRect(*it, childRect)) {
            continue;
        }

        if (!childRect.IsValid() || NearZero(childRect.Width()) || NearZero(childRect.Height())) {
            continue;
        }

        OffsetF vec = childRect.Center() - rect.Center();
        double val = (vec.GetX() * vec.GetX()) + (vec.GetY() * vec.GetY());
        if (minVal > val) {
            minVal = val;
            itNewFocusNode = it;
            offset = childRect.GetOffset();
        }
    }

    if (itNewFocusNode != focusNodes.end() && (*itNewFocusNode)->AcceptFocusByRectOfLastFocus(rect - offset)) {
        lastWeakFocusNode_ = AceType::WeakClaim(AceType::RawPtr(*itNewFocusNode));
        canChildBeFocused = true;
    }
    if (focusDepend_ == FocusDependence::CHILD) {
        return canChildBeFocused;
    }
    return focusDepend_ == FocusDependence::AUTO;
}

bool FocusHub::CalculateRect(const RefPtr<FocusHub>& childNode, RectF& rect) const
{
    auto childGeometryNode = childNode->GetGeometryNode();
    CHECK_NULL_RETURN(childGeometryNode, false);
    rect = childGeometryNode->GetFrameRect();
    return true;
}

bool FocusHub::IsFocusableByTab()
{
    if (focusType_ == FocusType::NODE) {
        return IsFocusableNodeByTab();
    }
    if (focusType_ == FocusType::SCOPE) {
        return IsFocusableScopeByTab();
    }
    return false;
}

bool FocusHub::IsFocusableNodeByTab()
{
    auto parent = GetParentFocusHub();
    CHECK_NULL_RETURN(parent, GetTabIndex() == 0);
    return (GetTabIndex() == 0) && (parent->GetTabIndex() == 0);
}

bool FocusHub::IsFocusableScopeByTab()
{
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    if (!IsFocusableNodeByTab()) {
        return false;
    }
    if (focusNodes.empty()) {
        return true;
    }
    return std::any_of(focusNodes.begin(), focusNodes.end(),
        [](const RefPtr<FocusHub>& focusNode) { return focusNode->IsFocusableByTab(); });
}

bool FocusHub::IsFocusableWholePath()
{
    auto parent = GetParentFocusHub();
    while (parent) {
        if (!parent->IsFocusableNode()) {
            return false;
        }
        parent = parent->GetParentFocusHub();
    }
    return IsFocusable();
}

bool FocusHub::IsOnRootTree() const
{
    auto parent = GetParentFocusHub();
    while (parent) {
        auto parentName = parent->GetFrameName();
        if (parentName == V2::ROOT_ETS_TAG) {
            return true;
        }
        parent = parent->GetParentFocusHub();
    }
    return false;
}

void FocusHub::CollectTabIndexNodes(TabIndexNodeList& tabIndexNodes)
{
    if (GetTabIndex() > 0 && IsFocusable()) {
        tabIndexNodes.emplace_back(GetTabIndex(), WeakClaim(this));
    }
    if (GetFocusType() == FocusType::SCOPE) {
        std::list<RefPtr<FocusHub>> focusNodes;
        FlushChildrenFocusHub(focusNodes);
        for (auto& child : focusNodes) {
            child->CollectTabIndexNodes(tabIndexNodes);
        }
    }
}

bool FocusHub::GoToFocusByTabNodeIdx(TabIndexNodeList& tabIndexNodes, int32_t tabNodeIdx)
{
    auto iter = tabIndexNodes.begin();
    std::advance(iter, tabNodeIdx);
    if (iter == tabIndexNodes.end()) {
        return false;
    }
    auto nodeNeedToFocus = (*iter).second.Upgrade();
    if (!nodeNeedToFocus) {
        TAG_LOGW(AceLogTag::ACE_FOCUS, "Tab index node is null");
        return false;
    }
    auto nodeIdNeedToFocus = nodeNeedToFocus->GetFrameId();
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Move focus to tab index node(%{public}d: %{public}s/%{public}d)", tabNodeIdx,
        nodeNeedToFocus->GetFrameName().c_str(), nodeNeedToFocus->GetFrameId());
    if (nodeNeedToFocus->GetFocusType() == FocusType::SCOPE && !nodeNeedToFocus->IsDefaultGroupHasFocused()) {
        auto defaultFocusNode = nodeNeedToFocus->GetChildFocusNodeByType(FocusNodeType::GROUP_DEFAULT);
        if (defaultFocusNode) {
            if (!defaultFocusNode->IsFocusableWholePath()) {
                TAG_LOGI(AceLogTag::ACE_FOCUS, "node(%{public}d) is not focusable", tabNodeIdx);
                return false;
            }
            nodeNeedToFocus->SetIsDefaultGroupHasFocused(true);
            if (defaultFocusNode->RequestFocusImmediately()) {
                lastTabIndexNodeId_ = nodeIdNeedToFocus;
                return true;
            }
            return false;
        }
    }
    if (!nodeNeedToFocus->IsFocusableWholePath()) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "node(%{public}d) is not focusable", tabNodeIdx);
        return false;
    }
    if (nodeNeedToFocus->RequestFocusImmediately()) {
        lastTabIndexNodeId_ = nodeIdNeedToFocus;
        return true;
    }
    return false;
}

RefPtr<FocusHub> FocusHub::GetChildFocusNodeByType(FocusNodeType nodeType)
{
    if (nodeType == FocusNodeType::DEFAULT && IsDefaultFocus() && IsFocusable()) {
        return AceType::Claim(this);
    }
    if (nodeType == FocusNodeType::GROUP_DEFAULT && IsDefaultGroupFocus() && IsFocusable()) {
        return AceType::Claim(this);
    }
    if (focusType_ != FocusType::SCOPE) {
        return nullptr;
    }
    std::list<RefPtr<FocusHub>> focusNodes;
    FlushChildrenFocusHub(focusNodes);
    for (const auto& child : focusNodes) {
        auto findNode = child->GetChildFocusNodeByType(nodeType);
        if (findNode) {
            return findNode;
        }
    }
    return nullptr;
}

RefPtr<FocusHub> FocusHub::GetChildFocusNodeById(const std::string& id)
{
    if (id.empty()) {
        return nullptr;
    }
    if (GetInspectorKey().has_value() && GetInspectorKey().value() == id) {
        return AceType::Claim(this);
    }
    if (focusType_ == FocusType::SCOPE) {
        std::list<RefPtr<FocusHub>> focusNodes;
        FlushChildrenFocusHub(focusNodes);
        for (const auto& child : focusNodes) {
            auto findNode = child->GetChildFocusNodeById(id);
            if (findNode) {
                return findNode;
            }
        }
    }
    return nullptr;
}

void FocusHub::HandleParentScroll() const
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (!context->GetIsFocusActive() || (focusType_ != FocusType::NODE && !isFocusUnit_)) {
        return;
    }
    auto parent = GetParentFocusHub();
    RefPtr<FrameNode> parentFrame;
    RefPtr<Pattern> parentPattern;
    while (parent) {
        if (parent->isFocusUnit_) {
            return;
        }
        parentFrame = parent->GetFrameNode();
        if (!parentFrame) {
            parent = parent->GetParentFocusHub();
            continue;
        }
        parentPattern = parentFrame->GetPattern();
        if (parentPattern && parentPattern->ScrollToNode(GetFrameNode())) {
            return;
        }
        parent = parent->GetParentFocusHub();
    }
}

bool FocusHub::RequestFocusImmediatelyById(const std::string& id)
{
    auto focusNode = GetChildFocusNodeById(id);
    if (!focusNode) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus id: %{public}s can not found.", id.c_str());
        return false;
    }
    auto result = true;
    if (!focusNode->IsFocusable()) {
        result = false;
    }
    TAG_LOGI(AceLogTag::ACE_FOCUS, "Request focus immediately by id: %{public}s. The node is %{public}s/%{public}d.",
        id.c_str(), focusNode->GetFrameName().c_str(), focusNode->GetFrameId());
    focusNode->RequestFocus();
    return result;
}

int32_t FocusHub::GetFocusingTabNodeIdx(TabIndexNodeList& tabIndexNodes) const
{
    if (lastTabIndexNodeId_ == DEFAULT_TAB_FOCUSED_INDEX) {
        return DEFAULT_TAB_FOCUSED_INDEX;
    }
    int32_t i = 0;
    for (auto& wpNode : tabIndexNodes) {
        auto node = wpNode.second.Upgrade();
        if (node && node->IsCurrentFocus() && node->GetFrameId() == lastTabIndexNodeId_) {
            return i;
        }
        ++i;
    }
    return DEFAULT_TAB_FOCUSED_INDEX;
}

bool FocusHub::HandleFocusByTabIndex(const KeyEvent& event)
{
    if (event.code != KeyCode::KEY_TAB || event.action != KeyAction::DOWN) {
        return false;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline && pipeline->IsTabJustTriggerOnKeyEvent()) {
        return false;
    }
    TabIndexNodeList tabIndexNodes;
    tabIndexNodes.clear();
    CollectTabIndexNodes(tabIndexNodes);
    if (tabIndexNodes.empty()) {
        return false;
    }
    tabIndexNodes.sort([](std::pair<int32_t, WeakPtr<FocusHub>>& a, std::pair<int32_t, WeakPtr<FocusHub>>& b) {
        return a.first < b.first;
    });
    int32_t curTabFocusIndex = GetFocusingTabNodeIdx(tabIndexNodes);
    if ((curTabFocusIndex < 0 || curTabFocusIndex >= static_cast<int32_t>(tabIndexNodes.size())) &&
        curTabFocusIndex != DEFAULT_TAB_FOCUSED_INDEX) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "Current focused tabIndex: %{public}d is not valid. Use default focus system.",
            curTabFocusIndex);
        return false;
    }
    if (curTabFocusIndex == DEFAULT_TAB_FOCUSED_INDEX) {
        curTabFocusIndex = 0;
    } else {
        if (event.IsShiftWith(KeyCode::KEY_TAB)) {
            --curTabFocusIndex;
        } else {
            ++curTabFocusIndex;
        }
        if (curTabFocusIndex < 0 || curTabFocusIndex >= static_cast<int32_t>(tabIndexNodes.size())) {
            curTabFocusIndex = (curTabFocusIndex + static_cast<int32_t>(tabIndexNodes.size())) %
                               static_cast<int32_t>(tabIndexNodes.size());
        }
    }
    return GoToFocusByTabNodeIdx(tabIndexNodes, curTabFocusIndex);
}

double FocusHub::GetProjectAreaOnRect(const RectF& rect, const RectF& projectRect, FocusStep step)
{
    float areaWidth = 0.0;
    float areaHeight = 0.0;
    switch (step) {
        case FocusStep::UP:
            if (rect.Top() < projectRect.Bottom() && rect.Right() > projectRect.Left() &&
                rect.Left() < projectRect.Right()) {
                areaWidth = std::min(rect.Right(), projectRect.Right()) - std::max(rect.Left(), projectRect.Left());
                areaHeight = std::min(rect.Bottom(), projectRect.Bottom()) - rect.Top();
            }
            break;
        case FocusStep::DOWN:
            if (rect.Bottom() > projectRect.Top() && rect.Right() > projectRect.Left() &&
                rect.Left() < projectRect.Right()) {
                areaWidth = std::min(rect.Right(), projectRect.Right()) - std::max(rect.Left(), projectRect.Left());
                areaHeight = rect.Bottom() - std::max(rect.Top(), projectRect.Top());
            }
            break;
        case FocusStep::LEFT:
            if (rect.Left() < projectRect.Right() && rect.Bottom() > projectRect.Top() &&
                rect.Top() < projectRect.Bottom()) {
                areaWidth = std::min(rect.Right(), projectRect.Right()) - rect.Left();
                areaHeight = std::min(rect.Bottom(), projectRect.Bottom()) - std::max(rect.Top(), projectRect.Top());
            }
            break;
        case FocusStep::RIGHT:
            if (rect.Right() > projectRect.Left() && rect.Bottom() > projectRect.Top() &&
                rect.Top() < projectRect.Bottom()) {
                areaWidth = rect.Right() - std::max(rect.Left(), projectRect.Left());
                areaHeight = std::min(rect.Bottom(), projectRect.Bottom()) - std::max(rect.Top(), projectRect.Top());
            }
            break;
        default:
            break;
    }
    return areaWidth * areaHeight;
}

RefPtr<FocusHub> FocusHub::GetNearestNodeByProjectArea(const std::list<RefPtr<FocusHub>>& allNodes, FocusStep step)
{
    CHECK_NULL_RETURN(!allNodes.empty(), nullptr);
    auto curFrameNode = GetFrameNode();
    CHECK_NULL_RETURN(curFrameNode, nullptr);
    auto curFrameOffset = curFrameNode->GetOffsetRelativeToWindow();
    auto curGeometryNode = curFrameNode->GetGeometryNode();
    CHECK_NULL_RETURN(curGeometryNode, nullptr);
    RectF curFrameRect = RectF(curFrameOffset, curGeometryNode->GetFrameRect().GetSize());
    curFrameRect.SetOffset(curFrameOffset);
    TAG_LOGD(AceLogTag::ACE_FOCUS,
        "Current focus node is %{public}s/%{public}d. Rect is {%{public}f,%{public}f,%{public}f,%{public}f}.",
        GetFrameName().c_str(), GetFrameId(), curFrameRect.Left(), curFrameRect.Top(), curFrameRect.Right(),
        curFrameRect.Bottom());
    bool isTabStep = IsFocusStepTab(step);
    double resDistance = !isTabStep ? std::numeric_limits<double>::max() : 0.0f;
    RefPtr<FocusHub> nextNode;
    for (const auto& node : allNodes) {
        if (!node || AceType::RawPtr(node) == this) {
            continue;
        }
        auto frameNode = node->GetFrameNode();
        if (!frameNode) {
            continue;
        }
        auto frameOffset = frameNode->GetOffsetRelativeToWindow();
        auto geometryNode = frameNode->GetGeometryNode();
        if (!geometryNode) {
            continue;
        }
        RectF frameRect = RectF(frameOffset, geometryNode->GetFrameRect().GetSize());
        auto realStep = step;
        if (step == FocusStep::TAB) {
            frameRect -= OffsetF(0, curFrameRect.Height());
            realStep = FocusStep::LEFT;
        } else if (step == FocusStep::SHIFT_TAB) {
            frameRect += OffsetF(0, curFrameRect.Height());
            realStep = FocusStep::RIGHT;
        }
        auto projectArea = GetProjectAreaOnRect(frameRect, curFrameRect, realStep);
        if (Positive(projectArea)) {
            OffsetF vec = frameRect.Center() - curFrameRect.Center();
            double val = (vec.GetX() * vec.GetX()) + (vec.GetY() * vec.GetY());
            if ((step == FocusStep::TAB && Positive(vec.GetX())) ||
                (step == FocusStep::SHIFT_TAB && Negative(vec.GetX()))) {
                val *= -1.0;
            }
            if ((!isTabStep && val < resDistance) || (isTabStep && val > resDistance)) {
                resDistance = val;
                nextNode = node;
            }
        }
    }
    TAG_LOGD(AceLogTag::ACE_FOCUS, "Next focus node is %{public}s/%{public}d. Min distance is %{public}f.",
        nextNode ? nextNode->GetFrameName().c_str() : "NULL", nextNode ? nextNode->GetFrameId() : -1, minDistance);
    return nextNode;
}

} // namespace OHOS::Ace::NG
