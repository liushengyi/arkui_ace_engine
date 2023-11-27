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

#include "core/components_ng/pattern/select/select_layout_algorithm.h"

#include "base/geometry/dimension.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
namespace {
    constexpr float MIN_SPACE = 8.0f;
    constexpr float MIN_CHAR_VAL = 2.0f;
} // namespace
void SelectLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);

    auto layoutProps = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProps);
    auto childConstraint = layoutProps->CreateChildConstraint();

    // Measure child row to get row height and width.
    auto rowWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(rowWrapper);
    auto spinnerSize = MeasureAndGetSize(rowWrapper->GetOrCreateChildByIndex(1), childConstraint);
    auto rowProps = DynamicCast<FlexLayoutProperty>(rowWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(rowProps);
    auto space = static_cast<float>(rowProps->GetSpaceValue(Dimension()).ConvertToPx());
    childConstraint.maxSize.MinusWidth(spinnerSize.Width() + space);
    auto textWrapper = rowWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(textWrapper);
    auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textLayoutProperty);
    auto textLayoutConstraint = textLayoutProperty->CreateContentConstraint();
    auto textSize = MeasureAndGetSize(textWrapper, childConstraint);
    if (childConstraint.parentIdealSize.Width().has_value()) {
        // Make the spinner icon layout at the right end
        textSize.SetWidth(childConstraint.parentIdealSize.Width().value() - spinnerSize.Width() - space);
    }

    auto textProps = DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textProps);
    auto fontSize = textProps->GetFontSize().value().ConvertToPx();
    if (textSize.Width() < fontSize * MIN_CHAR_VAL) {
        textSize.SetWidth(fontSize * MIN_CHAR_VAL);
    }

    textLayoutProperty->UpdateMarginSelfIdealSize(textSize);
    textLayoutConstraint.selfIdealSize = OptionalSize<float>(textSize.Width(), textSize.Height());
    textWrapper->Measure(textLayoutConstraint);

    auto rowGeometry = rowWrapper->GetGeometryNode();
    CHECK_NULL_VOID(rowGeometry);
    auto minSpace = Dimension(MIN_SPACE, DimensionUnit::VP);
    if (space < minSpace.ConvertToPx()) {
        space = minSpace.ConvertToPx();
        rowProps->UpdateSpace(minSpace);
    }

    auto rowWidth = textSize.Width() + space + spinnerSize.Width();
    auto rowHeight = std::max(textSize.Height(), spinnerSize.Height());
    rowGeometry->SetFrameSize(SizeF(rowWidth, rowHeight));
    rowWrapper->GetLayoutProperty()->UpdatePropertyChangeFlag(PROPERTY_UPDATE_LAYOUT);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto defaultHeight = static_cast<float>(theme->GetSelectDefaultHeight().ConvertToPx());
    layoutWrapper->GetGeometryNode()->SetContentSize(
        SizeF(rowWidth, rowHeight > defaultHeight ? rowHeight : defaultHeight));

    // Measure same as box, base on the child row.
    BoxLayoutAlgorithm::PerformMeasureSelf(layoutWrapper);
}

SizeF SelectLayoutAlgorithm::MeasureAndGetSize(
    const RefPtr<LayoutWrapper>& childLayoutWrapper, const LayoutConstraintF& constraint)
{
    CHECK_NULL_RETURN(childLayoutWrapper, SizeF());
    childLayoutWrapper->Measure(constraint);
    auto geometry = childLayoutWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(geometry, SizeF());
    return geometry->GetMarginFrameSize();
}
} // namespace OHOS::Ace::NG
