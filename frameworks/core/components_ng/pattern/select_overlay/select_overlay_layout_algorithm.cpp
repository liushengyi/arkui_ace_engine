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

#include "core/components_ng/pattern/select_overlay/select_overlay_layout_algorithm.h"

#include <cmath>

#include <optional>

#include "base/geometry/ng/offset_t.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension MORE_MENU_INTERVAL = 8.0_vp;
}
void SelectOverlayLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto menu = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(menu);
    if (!CheckInShowArea(*info_) ||
        (!info_->firstHandle.isShow && !info_->secondHandle.isShow && !info_->isSelectRegionVisible)) {
        menu->SetActive(false);
        return;
    } else {
        menu->SetActive(true);
    }

    // custom menu need to layout all menu and submenu
    if (info_->menuInfo.menuBuilder) {
        for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
            child->Layout();
        }
        return;
    }

    auto menuOffset = ComputeSelectMenuPosition(layoutWrapper);
    menu->GetGeometryNode()->SetMarginFrameOffset(menuOffset);
    menu->Layout();
    auto button = layoutWrapper->GetOrCreateChildByIndex(1);
    CHECK_NULL_VOID(button);
    auto menuNode = menu->GetHostNode();
    CHECK_NULL_VOID(menuNode);
    auto menuContext = menuNode->GetRenderContext();
    CHECK_NULL_VOID(menuContext);
    auto offset = OffsetF();
    if (menuContext->GetOffset()) {
        offset =
            OffsetF(menuContext->GetOffset()->GetX().ConvertToPx(), menuContext->GetOffset()->GetY().ConvertToPx());
    }
    if (!info_->menuInfo.menuIsShow || info_->menuInfo.menuDisable) {
        hasExtensionMenu_ = false;
        return;
    }
    hasExtensionMenu_ = true;
    button->GetGeometryNode()->SetMarginFrameOffset(menuOffset);
    button->Layout();
    auto extensionMenuOffset = ComputeExtensionMenuPosition(layoutWrapper, offset);

    auto extensionMenu = layoutWrapper->GetOrCreateChildByIndex(2);
    CHECK_NULL_VOID(extensionMenu);
    extensionMenu->GetGeometryNode()->SetMarginFrameOffset(extensionMenuOffset);
    extensionMenu->Layout();
}

bool SelectOverlayLayoutAlgorithm::CheckInShowArea(const SelectOverlayInfo& info)
{
    if (info.useFullScreen) {
        return true;
    }
    if (info.isSingleHandle) {
        return info.firstHandle.paintRect.IsWrappedBy(info.showArea);
    }
    return info.firstHandle.paintRect.IsWrappedBy(info.showArea) &&
           info.secondHandle.paintRect.IsWrappedBy(info.showArea);
}

OffsetF SelectOverlayLayoutAlgorithm::ComputeSelectMenuPosition(LayoutWrapper* layoutWrapper)
{
    auto menuItem = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_RETURN(menuItem, OffsetF());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, OffsetF());
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(theme, OffsetF());
    OffsetF menuPosition;
    bool isExtension = false;

    // Calculate the spacing with text and handle, menu is fixed up the handle and text.
    double menuSpacingBetweenText = theme->GetMenuSpacingWithText().ConvertToPx();
    double menuSpacingBetweenHandle = theme->GetHandleDiameter().ConvertToPx();

    auto width = menuItem->GetGeometryNode()->GetMarginFrameSize().Width();
    auto height = menuItem->GetGeometryNode()->GetMarginFrameSize().Height();

    // When the extended menu is displayed, the default menu becomes circular, but the position of the circle is aligned
    // with the end of the original menu.
    if (GreatNotEqual(width, height)) {
        menuWidth_ = width;
        menuHeight_ = height;
    } else {
        isExtension = true;
    }
    auto menuWidth = menuWidth_.value_or(width);
    auto menuHeight = menuHeight_.value_or(height);

    // paint rect is in global position, need to convert to local position
    auto offset = layoutWrapper->GetGeometryNode()->GetFrameOffset();
    const auto firstHandleRect = info_->firstHandle.paintRect - offset;
    const auto secondHandleRect = info_->secondHandle.paintRect - offset;

    auto singleHandle = firstHandleRect;
    if (!info_->firstHandle.isShow) {
        singleHandle = secondHandleRect;
    }
    if (IsTextAreaSelectAll()) {
        singleHandle = RectF(info_->menuInfo.menuOffset.value().GetX(), info_->menuInfo.menuOffset.value().GetY(),
            singleHandle.Width(), singleHandle.Height());
    }

    if (info_->isSingleHandle) {
        auto menuSpacing = static_cast<float>(menuSpacingBetweenText);
        menuPosition = OffsetF((singleHandle.Left() + singleHandle.Right() - menuWidth) / 2.0f,
            static_cast<float>(singleHandle.Top() - menuSpacing - menuHeight));
    } else {
        auto menuSpacing = static_cast<float>(menuSpacingBetweenText + menuSpacingBetweenHandle);
        menuPosition = OffsetF((firstHandleRect.Left() + secondHandleRect.Left() - menuWidth) / 2.0f,
            static_cast<float>(firstHandleRect.Top() - menuSpacing - menuHeight));

        if (!info_->firstHandle.isShow && info_->secondHandle.isShow && !info_->handleReverse) {
            menuPosition.SetY(secondHandleRect.Bottom() + menuSpacing);
        }
        if (info_->firstHandle.isShow && !info_->secondHandle.isShow && info_->handleReverse) {
            menuPosition.SetY(firstHandleRect.Bottom() + menuSpacing);
        }
        if (info_->firstHandle.isShow && info_->secondHandle.isShow &&
            !NearEqual(firstHandleRect.Top(), secondHandleRect.Top())) {
            auto top = std::min(firstHandleRect.Top(), secondHandleRect.Top());
            menuPosition.SetY(static_cast<float>(top - menuSpacing - menuHeight));
        }
    }

    auto overlayWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width();
    RectF viewPort = layoutWrapper->GetGeometryNode()->GetFrameRect() - offset;
    auto frameNode = info_->callerFrameNode.Upgrade();
    if (frameNode) {
        auto viewPortOption = frameNode->GetViewPort();
        if (viewPortOption.has_value()) {
            viewPort = viewPortOption.value();
        }
    }
    LOGD("select_overlay viewPort Rect: %{public}s", viewPort.ToString().c_str());

    // Adjust position of overlay.
    if (LessOrEqual(menuPosition.GetX(), viewPort.GetX())) {
        menuPosition.SetX(theme->GetDefaultMenuPositionX());
    } else if (GreatOrEqual(menuPosition.GetX() + menuWidth, viewPort.GetX() + viewPort.Width())) {
        menuPosition.SetX(overlayWidth - menuWidth - theme->GetDefaultMenuPositionX());
    }
    if (LessNotEqual(menuPosition.GetY(), menuHeight)) {
        if (IsTextAreaSelectAll()) {
            menuPosition.SetY(singleHandle.Top());
        } else {
            menuPosition.SetY(
                static_cast<float>(singleHandle.Bottom() + menuSpacingBetweenText + menuSpacingBetweenHandle));
        }
    }
    if (LessNotEqual(menuPosition.GetY(), viewPort.GetY() - menuSpacingBetweenText - menuHeight) ||
        LessNotEqual(menuPosition.GetY(), menuSpacingBetweenText)) {
        auto menuOffsetY = viewPort.GetY() - menuSpacingBetweenText - menuHeight;
        if (menuOffsetY > menuSpacingBetweenText) {
            menuPosition.SetY(menuOffsetY);
        } else {
            menuPosition.SetY(menuSpacingBetweenText);
        }
    } else if (GreatOrEqual(menuPosition.GetY(), viewPort.GetY() + viewPort.Height() + menuSpacingBetweenText)) {
        menuPosition.SetY(viewPort.GetY() + viewPort.Height() + menuSpacingBetweenText);
    }

    auto safeAreaManager = pipeline->GetSafeAreaManager();
    if (safeAreaManager) {
        // ignore status bar
        auto top = safeAreaManager->GetSystemSafeArea().top_.Length();
        if (menuPosition.GetY() < top) {
            menuPosition.SetY(top);
        }
    }
    if (info_->firstHandle.isShow && info_->secondHandle.isShow &&
        !NearEqual(firstHandleRect.Top(), secondHandleRect.Top())) {
        auto menuRect = RectF(menuPosition, SizeF(menuWidth, menuHeight));
        auto downHandleRect =
            LessNotEqual(firstHandleRect.Top(), secondHandleRect.Top()) ? secondHandleRect : firstHandleRect;
        auto circleDiameter = menuSpacingBetweenHandle;
        auto circleOffset =
            OffsetF(downHandleRect.GetX() - (circleDiameter - downHandleRect.Width()) / 2.0f, downHandleRect.Bottom());
        auto downHandleCircleRect = RectF(circleOffset, SizeF(circleDiameter, circleDiameter));
        if (menuRect.IsIntersectWith(downHandleRect) || menuRect.IsInnerIntersectWith(downHandleCircleRect)) {
            auto menuSpacing = static_cast<float>(menuSpacingBetweenText + circleDiameter);
            menuPosition.SetY(downHandleRect.Bottom() + menuSpacing);
        }
    }
    LOGD("select_overlay menuPosition: %{public}s", menuPosition.ToString().c_str());
    defaultMenuEndOffset_ = menuPosition + OffsetF(menuWidth, 0.0f);
    if (isExtension) {
        return defaultMenuEndOffset_ - OffsetF(width, 0);
    }
    return menuPosition;
}

OffsetF SelectOverlayLayoutAlgorithm::ComputeExtensionMenuPosition(LayoutWrapper* layoutWrapper, const OffsetF& offset)
{
    auto extensionItem = layoutWrapper->GetOrCreateChildByIndex(2);
    CHECK_NULL_RETURN(extensionItem, OffsetF());
    auto extensionLayoutConstraint = extensionItem->GetLayoutProperty()->GetLayoutConstraint();
    auto extensionLayoutConstraintMaxSize = extensionLayoutConstraint->maxSize;
    auto extensionWidth = extensionItem->GetGeometryNode()->GetMarginFrameSize().Width();
    auto extensionHeight = extensionItem->GetGeometryNode()->GetMarginFrameSize().Height();
    auto menuItem = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_RETURN(menuItem, OffsetF());
    auto menuHeight = menuItem->GetGeometryNode()->GetMarginFrameSize().Height();
    auto extensionOffset =
        defaultMenuEndOffset_ - OffsetF(extensionWidth, -menuHeight - MORE_MENU_INTERVAL.ConvertToPx());
    if (extensionOffset.GetY() + extensionHeight > extensionLayoutConstraintMaxSize.Height()) {
        extensionOffset =
            defaultMenuEndOffset_ - OffsetF(extensionWidth, extensionHeight + MORE_MENU_INTERVAL.ConvertToPx());
    }
    return extensionOffset;
}

bool SelectOverlayLayoutAlgorithm::IsTextAreaSelectAll()
{
    return info_->menuInfo.menuOffset.has_value() && (!info_->firstHandle.isShow || !info_->secondHandle.isShow);
}

} // namespace OHOS::Ace::NG