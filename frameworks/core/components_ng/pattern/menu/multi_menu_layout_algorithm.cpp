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

#include "core/components_ng/pattern/menu/multi_menu_layout_algorithm.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {
void MultiMenuLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto layoutConstraint = layoutProperty->GetLayoutConstraint();
    CHECK_NULL_VOID(layoutConstraint);
    auto childConstraint = layoutProperty->CreateChildConstraint();
    childConstraint.maxSize.SetWidth(layoutConstraint->maxSize.Width());
    // constraint max size minus padding
    const auto& padding = layoutProperty->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, childConstraint.maxSize);
    if (layoutConstraint->selfIdealSize.Width().has_value()) {
        // when Menu is set self ideal width, make children node adaptively fill up.
        auto idealWidth =
            std::max(layoutConstraint->minSize.Width(),
                std::min(layoutConstraint->maxSize.Width(), layoutConstraint->selfIdealSize.Width().value())) -
            padding.Width();
        childConstraint.selfIdealSize.SetWidth(idealWidth);
    } else {
        // constraint min width base on grid column
        auto columnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::MENU);
        CHECK_NULL_VOID(columnInfo);
        CHECK_NULL_VOID(columnInfo->GetParent());
        columnInfo->GetParent()->BuildColumnWidth();
        auto minWidth = static_cast<float>(columnInfo->GetWidth()) - padding.Width();
        childConstraint.minSize.SetWidth(minWidth);
    }
    // Calculate max width of menu items
    UpdateConstraintBaseOnMenuItems(layoutWrapper, childConstraint);

    float contentHeight = 0.0f;
    float contentWidth = childConstraint.selfIdealSize.Width().value();
    for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
        child->Measure(ResetLayoutConstraintMinWidth(child, childConstraint));
        auto childSize = child->GetGeometryNode()->GetMarginFrameSize();
        contentHeight += childSize.Height();
    }
    layoutWrapper->GetGeometryNode()->SetContentSize(SizeF(contentWidth, contentHeight));
    BoxLayoutAlgorithm::PerformMeasureSelf(layoutWrapper);

    if (layoutConstraint->selfIdealSize.Width().has_value()) {
        auto idealWidth = std::max(layoutConstraint->minSize.Width(),
            std::min(layoutConstraint->maxSize.Width(), layoutConstraint->selfIdealSize.Width().value()));
        auto idealSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
        idealSize.SetWidth(idealWidth);
        layoutWrapper->GetGeometryNode()->SetFrameSize(idealSize);
    }
}

void MultiMenuLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    BoxLayoutAlgorithm::PerformLayout(layoutWrapper);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    // translate each option by the height of previous options
    auto outPadding = static_cast<float>(theme->GetOutPadding().ConvertToPx());
    const auto& padding = layoutProperty->CreatePaddingAndBorder();
    OffsetF translate(padding.left.value_or(outPadding), padding.top.value_or(outPadding));
    for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
        child->GetGeometryNode()->SetMarginFrameOffset(translate);
        child->Layout();
        translate.AddY(child->GetGeometryNode()->GetMarginFrameSize().Height());
    }
}

void MultiMenuLayoutAlgorithm::UpdateConstraintBaseOnMenuItems(
    LayoutWrapper* layoutWrapper, LayoutConstraintF& constraint)
{
    // multiMenu children are menuItem or menuItemGroup, constrain width is same as the menu
    auto maxChildrenWidth = GetChildrenMaxWidth(layoutWrapper, constraint);
    constraint.selfIdealSize.SetWidth(maxChildrenWidth);
}

float MultiMenuLayoutAlgorithm::GetChildrenMaxWidth(
    LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint)
{
    float maxWidth = 0.0f;
    for (const auto& child : layoutWrapper->GetAllChildrenWithBuild()) {
        auto childConstraint = ResetLayoutConstraintMinWidth(child, layoutConstraint);
        child->Measure(childConstraint);
        auto childSize = child->GetGeometryNode()->GetFrameSize();
        maxWidth = std::max(maxWidth, childSize.Width());
    }
    return maxWidth;
}

LayoutConstraintF MultiMenuLayoutAlgorithm::ResetLayoutConstraintMinWidth(
    const RefPtr<LayoutWrapper>& child, const LayoutConstraintF& layoutConstraint)
{
    auto childLayoutProps = child->GetLayoutProperty();
    CHECK_NULL_RETURN(childLayoutProps, layoutConstraint);
    auto childConstraint = layoutConstraint;
    const auto& calcConstraint = childLayoutProps->GetCalcLayoutConstraint();
    if (calcConstraint && calcConstraint->selfIdealSize.has_value() &&
        calcConstraint->selfIdealSize.value().Width().has_value()) {
        childConstraint.minSize.Reset();
    }
    return childConstraint;
}
} // namespace OHOS::Ace::NG
