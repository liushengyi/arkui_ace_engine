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

#include "core/components_ng/pattern/menu/sub_menu_layout_algorithm.h"

#include "base/geometry/ng/offset_t.h"
#include "core/common/ace_engine.h"
#include "core/common/container_scope.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"
namespace OHOS::Ace::NG {

void SubMenuLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto menuNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(menuNode);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    auto parentMenuItem = menuPattern->GetParentMenuItem();
    CHECK_NULL_VOID(parentMenuItem);

    InitHierarchicalParameters();
    auto pipelineContext = PipelineContext::GetMainPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto windowGlobalRect = pipelineContext->GetDisplayWindowRectInfo();
    float windowsOffsetY = static_cast<float>(windowGlobalRect.GetOffset().GetY());
    if (hierarchicalParameters_ || !Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        windowsOffsetY = 0.0f;
    }
    InitializePadding(layoutWrapper);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        ModifySubMenuWrapper(layoutWrapper);
    }
    auto menuItemSize = parentMenuItem->GetGeometryNode()->GetFrameSize();
    position_ = GetSubMenuPosition(parentMenuItem);
    float x = HorizontalLayoutSubMenu(size, position_.GetX(), menuItemSize);
    x = std::clamp(x, paddingStart_, wrapperSize_.Width() - menuItemSize.Width() - paddingEnd_);
    float y = 0.0f;
    if (hierarchicalParameters_ || !Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
        y = VerticalLayoutSubMenu(size, position_.GetY(), menuItemSize);
    } else {
        y = VerticalLayoutSubMenuHalfScreen(size, position_.GetY(), menuItemSize);
    }
    y = std::clamp(
        y, windowsOffsetY + paddingTop_, windowsOffsetY + wrapperSize_.Height() - size.Height() - paddingBottom_);
    const auto& geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    geometryNode->SetMarginFrameOffset(NG::OffsetF(x, y));

    if (parentMenuItem) {
        auto parentPattern = parentMenuItem->GetPattern<MenuItemPattern>();
        CHECK_NULL_VOID(parentPattern);
        auto topLeftPoint = OffsetF(x, y);
        auto bottomRightPoint = OffsetF(x + size.Width(), y + size.Height());

        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto windowManager = pipelineContext->GetWindowManager();
        auto isContainerModal = pipelineContext->GetWindowModal() == WindowModal::CONTAINER_MODAL && windowManager &&
                                windowManager->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING;
        OffsetF wrapperOffset;
        if (isContainerModal) {
            auto newOffsetX = static_cast<float>(CONTAINER_BORDER_WIDTH.ConvertToPx()) +
                              static_cast<float>(CONTENT_PADDING.ConvertToPx());
            auto newOffsetY = static_cast<float>(CONTAINER_TITLE_HEIGHT.ConvertToPx()) +
                              static_cast<float>(CONTAINER_BORDER_WIDTH.ConvertToPx());
            wrapperOffset = OffsetF(newOffsetX, newOffsetY);
        }
        parentPattern->AddHoverRegions(topLeftPoint + wrapperOffset, bottomRightPoint + wrapperOffset);
    }

    auto child = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(child);
    child->Layout();
}

OffsetF SubMenuLayoutAlgorithm::GetSubMenuPosition(const RefPtr<FrameNode>& parentMenuItem)
{
    auto parentFrameSize = parentMenuItem->GetGeometryNode()->GetMarginFrameSize();
    auto position = parentMenuItem->GetPaintRectOffset() + OffsetF(parentFrameSize.Width(), 0.0);

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, OffsetF());
    auto windowManager = pipelineContext->GetWindowManager();
    CHECK_NULL_RETURN(windowManager, OffsetF());
    auto isContainerModal = pipelineContext->GetWindowModal() == WindowModal::CONTAINER_MODAL && windowManager &&
                            windowManager->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING;
    if (isContainerModal) {
        auto newOffsetX = static_cast<float>(CONTAINER_BORDER_WIDTH.ConvertToPx()) +
                              static_cast<float>(CONTENT_PADDING.ConvertToPx());
        auto newOffsetY = static_cast<float>(CONTAINER_TITLE_HEIGHT.ConvertToPx()) +
                            static_cast<float>(CONTAINER_BORDER_WIDTH.ConvertToPx());
        position -= OffsetF(newOffsetX, newOffsetY);
    }

    auto parentMenu = AceType::DynamicCast<FrameNode>(parentMenuItem->GetParent());
    CHECK_NULL_RETURN(parentMenu, position);
    auto scroll = AceType::DynamicCast<FrameNode>(parentMenu->GetParent());
    CHECK_NULL_RETURN(scroll, position);
    while (scroll->GetTag() != V2::SCROLL_ETS_TAG) {
        scroll = AceType::DynamicCast<FrameNode>(scroll->GetParent());
    }
    CHECK_NULL_RETURN(scroll, position);
    auto scrollGeometryNode = scroll->GetGeometryNode();
    CHECK_NULL_RETURN(scrollGeometryNode, position);
    auto scrollTop = scroll->GetPaintRectOffset().GetY();
    auto scrollHeight = scrollGeometryNode->GetFrameSize().Height();
    auto bottomOffset = scrollTop + scrollHeight;
    if (parentMenuItem->GetPaintRectOffset().GetY() > bottomOffset) {
        return scroll->GetPaintRectOffset() + OffsetF(parentFrameSize.Width(), 0.0);
    } else {
        return position;
    }
}

float SubMenuLayoutAlgorithm::VerticalLayoutSubMenuHalfScreen(
    const SizeF& size, float position, const SizeF& menuItemSize)
{
    auto pipelineContext = PipelineContext::GetMainPipelineContext();
    CHECK_NULL_RETURN(pipelineContext, 0.0f);
    auto safeAreaManager = pipelineContext->GetSafeAreaManager();
    CHECK_NULL_RETURN(safeAreaManager, 0.0f);
    auto windowGlobalRect = pipelineContext->GetDisplayWindowRectInfo();
    float windowsOffsetY = static_cast<float>(windowGlobalRect.GetOffset().GetY());
    float wrapperHeight = wrapperSize_.Height();

    float bottomSpace = wrapperSize_.Height() - (position_.GetY() - windowsOffsetY) - margin_ * 2.0f;
    // line up top of subMenu with top of the menuItem
    if (bottomSpace >= size.Height()) {
        return windowsOffsetY + position;
    }
    // line up bottom of menu with bottom of the screen
    if (size.Height() < wrapperHeight) {
        return windowsOffsetY + wrapperHeight - size.Height();
    }
    // can't fit in screen, line up with top of the screen
    return windowsOffsetY;
}

// return submenu vertical offset
float SubMenuLayoutAlgorithm::VerticalLayoutSubMenu(const SizeF& size, float position, const SizeF& menuItemSize)
{
    float wrapperHeight = wrapperSize_.Height();
    float bottomSpace = wrapperHeight - position;
    // line up top of subMenu with top of the menuItem
    if (bottomSpace >= size.Height()) {
        return position;
    }
    // line up bottom of menu with bottom of the screen
    if (size.Height() < wrapperHeight) {
        return wrapperHeight - size.Height();
    }
    // can't fit in screen, line up with top of the screen
    return 0.0f;
}

// returns submenu horizontal offset
float SubMenuLayoutAlgorithm::HorizontalLayoutSubMenu(const SizeF& size, float position, const SizeF& menuItemSize)
{
    float wrapperWidth = wrapperSize_.Width();
    float rightSpace = wrapperWidth - position;
    float leftSpace = position - menuItemSize.Width();
    // can fit subMenu on the right side of menuItem
    if (rightSpace >= size.Width()) {
        return position;
    }
    // fit subMenu on the left side of menuItem
    if (leftSpace >= size.Width()) {
        return position - size.Width() - menuItemSize.Width();
    }
    // line up right side of menu with right boundary of the screen
    if (size.Width() < wrapperWidth) {
        return wrapperWidth - size.Width() - paddingEnd_;
    }
    // can't fit in screen, line up with left side of the screen
    return 0.0f;
}

void SubMenuLayoutAlgorithm::ModifySubMenuWrapper(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto props = AceType::DynamicCast<MenuLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(props);
    auto pipelineContext = PipelineContext::GetMainPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto safeAreaManager = pipelineContext->GetSafeAreaManager();
    CHECK_NULL_VOID(safeAreaManager);
    auto theme = pipelineContext->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto expandDisplay = theme->GetExpandDisplay();
    auto windowGlobalRect = pipelineContext->GetDisplayWindowRectInfo();
    auto bottom = safeAreaManager->GetSystemSafeArea().bottom_.Length();
    auto menuNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(menuNode);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    bool isMainContextMenu = menuPattern->GetMainMenuPattern()->IsContextMenu();
    if (isMainContextMenu && !expandDisplay) {
        wrapperSize_ = SizeF(windowGlobalRect.Width(), windowGlobalRect.Height() - bottom);
    } else {
        wrapperSize_ = SizeF(wrapperSize_.Width(), wrapperSize_.Height());
    }
}

void SubMenuLayoutAlgorithm::InitializePadding(LayoutWrapper* layoutWrapper)
{
    auto menuPattern = layoutWrapper->GetHostNode()->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    auto pipeline = PipelineContext::GetMainPipelineContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    margin_ = static_cast<float>(theme->GetOutPadding().ConvertToPx());
    if (!hierarchicalParameters_) {
        paddingStart_ = static_cast<float>(theme->GetMaxPaddingStart().ConvertToPx());
        paddingEnd_ = static_cast<float>(theme->GetMaxPaddingEnd().ConvertToPx());
        paddingTop_ = static_cast<float>(theme->GetDefaultPaddingTop().ConvertToPx());
        paddingBottom_ = static_cast<float>(theme->GetDefaultPaddingBottomFixed().ConvertToPx());
    } else {
        paddingStart_ = margin_;
        paddingEnd_ = margin_;
        paddingTop_ = margin_;
        paddingBottom_ = margin_;
    }
}

} // namespace OHOS::Ace::NG
