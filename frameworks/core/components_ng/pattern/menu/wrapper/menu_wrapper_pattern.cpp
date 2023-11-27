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

#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"

#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/event/click_event.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void MenuWrapperPattern::HideMenu(const RefPtr<FrameNode>& menu)
{
    if (GetHost()->GetTag() == V2::SELECT_OVERLAY_ETS_TAG) {
        return;
    }
    isHided_ = true;

    auto menuPattern = menu->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->HideMenu();
    CallMenuStateChangeCallback("false");
}

void MenuWrapperPattern::OnAttachToFrameNode()
{
    RegisterOnTouch();
}

void MenuWrapperPattern::OnModifyDone()
{
    Pattern::OnModifyDone();

    isHided_ = false;
}

// close subMenu when mouse move outside
void MenuWrapperPattern::HandleMouseEvent(const MouseInfo& info, RefPtr<MenuItemPattern>& menuItemPattern)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto subMenu = host->GetChildren().back();
    if (host->GetChildren().size() <= 1) {
        return;
    }
    auto subMenuNode = DynamicCast<FrameNode>(subMenu);
    CHECK_NULL_VOID(subMenuNode);
    auto subMenuPattern = subMenuNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(subMenuPattern);
    auto currentHoverMenuItem = subMenuPattern->GetParentMenuItem();
    CHECK_NULL_VOID(currentHoverMenuItem);

    auto menuItemNode = menuItemPattern->GetHost();
    CHECK_NULL_VOID(menuItemNode);
    if (currentHoverMenuItem->GetId() != menuItemNode->GetId()) {
        return;
    }
    const auto& mousePosition = info.GetGlobalLocation();
    if (!menuItemPattern->IsInHoverRegions(mousePosition.GetX(), mousePosition.GetY()) &&
        menuItemPattern->IsSubMenuShowed()) {
        LOGI("MenuWrapperPattern Hide SubMenu");
        HideSubMenu();
        menuItemPattern->SetIsSubMenuShowed(false);
        menuItemPattern->ClearHoverRegions();
        menuItemPattern->ResetWrapperMouseEvent();
    }
}

void MenuWrapperPattern::HideMenu()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto menuNode = DynamicCast<FrameNode>(host->GetChildAtIndex(0));
    CHECK_NULL_VOID(menuNode);
    HideMenu(menuNode);
}

void MenuWrapperPattern::HideSubMenu()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (host->GetChildren().size() <= 1) {
        // sub menu not show
        return;
    }
    auto subMenu = host->GetChildren().back();
    auto iter = host->GetChildren().begin();
    int32_t focusNodeId = 2;
    std::advance(iter, host->GetChildren().size() - focusNodeId);
    auto focusMenu = *iter;
    if (focusMenu) {
        auto menuHub = DynamicCast<FrameNode>(focusMenu);
        CHECK_NULL_VOID(menuHub);
        auto focusHub = menuHub->GetFocusHub();
        CHECK_NULL_VOID(focusHub);
        focusHub->SetParentFocusable(true);
        focusHub->RequestFocusImmediately();
    }
    host->RemoveChild(subMenu);
    auto menuPattern = DynamicCast<FrameNode>(subMenu)->GetPattern<MenuPattern>();
    if (menuPattern) {
        menuPattern->RemoveParentHoverStyle();
        auto frameNode = FrameNode::GetFrameNode(menuPattern->GetTargetTag(), menuPattern->GetTargetId());
        CHECK_NULL_VOID(frameNode);
        auto menuItem = frameNode->GetPattern<MenuItemPattern>();
        if (menuItem) {
            menuItem->SetIsSubMenuShowed(false);
        }
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
}

void MenuWrapperPattern::RegisterOnTouch()
{
    // if already initialized touch event
    CHECK_NULL_VOID(!onTouch_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    // hide menu when touched outside the menu region
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnTouchEvent(info);
    };
    onTouch_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gesture->AddTouchEvent(onTouch_);
}

void MenuWrapperPattern::OnTouchEvent(const TouchEventInfo& info)
{
    CHECK_NULL_VOID(!info.GetTouches().empty());
    auto touch = info.GetTouches().front();
    // filter out other touch types
    if (touch.GetTouchType() != TouchType::DOWN) {
        return;
    }
    if (IsHided()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto position = OffsetF(
        static_cast<float>(touch.GetGlobalLocation().GetX()), static_cast<float>(touch.GetGlobalLocation().GetY()));
    position -= host->GetPaintRectOffset();
    auto children = host->GetChildren();
    for (auto child = children.rbegin(); child != children.rend(); ++child) {
        // get menu frame node (child of menu wrapper)
        auto menuNode = DynamicCast<FrameNode>(*child);
        CHECK_NULL_VOID(menuNode);
        // get menuNode's touch region
        auto menuZone = menuNode->GetGeometryNode()->GetFrameRect();
        if (menuZone.IsInRegion(PointF(position.GetX(), position.GetY()))) {
            return;
        }
        // if DOWN-touched outside the menu region, then hide menu
        auto menuPattern = menuNode->GetPattern<MenuPattern>();
        CHECK_NULL_VOID(menuPattern);
        if (menuPattern->IsSubMenu() || menuPattern->IsSelectOverlaySubMenu()) {
            HideSubMenu();
        } else {
            HideMenu(menuNode);
        }
    }
}

void MenuWrapperPattern::CheckAndShowAnimation()
{
    if (isFirstShow_) {
        // only start animation when menu wrapper mount on.
        StartShowAnimation();
        isFirstShow_ = false;
    }
}

bool MenuWrapperPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, false);
    auto expandDisplay = theme->GetExpandDisplay();
    if ((IsContextMenu() && !IsHided()) || (expandDisplay && !IsHided())) {
        SetHotAreas(dirty);
    }
    CheckAndShowAnimation();
    return false;
}

void MenuWrapperPattern::SetHotAreas(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto expandDisplay = theme->GetExpandDisplay();
    if ((layoutWrapper->GetAllChildrenWithBuild().empty() || !IsContextMenu()) && !expandDisplay) {
        return;
    }
    auto layoutProps = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProps);
    float safeAreaInsetsLeft = 0.0f;
    float safeAreaInsetsTop = 0.0f;
    auto&& safeAreaInsets = layoutProps->GetSafeAreaInsets();
    if (safeAreaInsets) {
        safeAreaInsetsLeft = static_cast<float>(safeAreaInsets->left_.end);
        safeAreaInsetsTop = static_cast<float>(safeAreaInsets->top_.end);
    }
    std::vector<Rect> rects;
    for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
        auto frameRect = child->GetGeometryNode()->GetFrameRect();
        // rect is relative to window
        auto rect = Rect(frameRect.GetX() + safeAreaInsetsLeft, frameRect.GetY() + safeAreaInsetsTop, frameRect.Width(),
            frameRect.Height());

        rects.emplace_back(rect);
    }
    SubwindowManager::GetInstance()->SetHotAreas(rects, GetHost()->GetId(), GetContainerId());
}

void MenuWrapperPattern::StartShowAnimation()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetRenderContext();
    CHECK_NULL_VOID(context);
    if (GetPreviewMode() == MenuPreviewMode::NONE) {
        context->UpdateOffset(GetAnimationOffset());
        context->UpdateOpacity(0.0);
    }

    AnimationUtils::Animate(
        animationOption_,
        [context, weak = WeakClaim(this)]() {
            if (context) {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                if (pattern->GetPreviewMode() == MenuPreviewMode::NONE) {
                    context->UpdateOffset(OffsetT<Dimension>());
                    context->UpdateOpacity(1.0);
                }
            }
        },
        animationOption_.GetOnFinishEvent());
}

void MenuWrapperPattern::SetAniamtinOption(const AnimationOption& animationOption)
{
    animationOption_.SetCurve(animationOption.GetCurve());
    animationOption_.SetDuration(animationOption.GetDuration());
    animationOption_.SetFillMode(animationOption.GetFillMode());
    animationOption_.SetOnFinishEvent(animationOption.GetOnFinishEvent());
}

OffsetT<Dimension> MenuWrapperPattern::GetAnimationOffset()
{
    OffsetT<Dimension> offset;

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, offset);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_RETURN(theme, offset);
    auto animationOffset = theme->GetMenuAnimationOffset();

    switch (menuPlacement_) {
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            offset.SetX(animationOffset);
            break;
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            offset.SetX(-animationOffset);
            break;
        case Placement::TOP:
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            offset.SetY(animationOffset);
            break;
        default:
            offset.SetY(-animationOffset);
            break;
    }
    return offset;
}
} // namespace OHOS::Ace::NG
