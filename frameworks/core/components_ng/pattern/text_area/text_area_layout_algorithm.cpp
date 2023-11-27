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

#include "core/components_ng/pattern/text_area/text_area_layout_algorithm.h"

#include <optional>

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float PARAGRAPH_SAVE_BOUNDARY = 1.0f;
} // namespace
std::optional<SizeF> TextAreaLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, std::nullopt);
    auto textFieldLayoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, std::nullopt);

    // Construct text style.
    TextStyle textStyle;
    ConstructTextStyles(frameNode, textStyle, textContent_, showPlaceHolder_);

    auto isInlineStyle = pattern->IsNormalInlineState();

    // Create paragraph.
    if (pattern->IsDragging() && !showPlaceHolder_) {
        CreateParagraph(textStyle, pattern->GetDragContents(), textContent_, false);
    } else {
        CreateParagraph(textStyle, textContent_, false, pattern->GetNakedCharPosition());
    }

    autoWidth_ = textFieldLayoutProperty->GetWidthAutoValue(false);

    if (textContent_.empty()) {
        // Used for empty text.
        preferredHeight_ = pattern->PreferredLineHeight(true);
    }
    auto textFieldContentConstraint = CalculateContentMaxSizeWithCalculateConstraint(contentConstraint, layoutWrapper);
    // Paragraph layout.}
    if (isInlineStyle) {
        return InlineMeasureContent(textFieldContentConstraint, layoutWrapper);
    } else if (showPlaceHolder_) {
        return PlaceHolderMeasureContent(textFieldContentConstraint, layoutWrapper);
    } else {
        return TextAreaMeasureContent(textFieldContentConstraint, layoutWrapper);
    }
}

void TextAreaLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    OptionalSizeF frameSize;
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    float contentWidth = 0.0f;
    float contentHeight = 0.0f;
    if (content) {
        auto contentSize = content->GetRect().GetSize();
        contentWidth = contentSize.Width();
        contentHeight = contentSize.Height();
    }
    // Add children height;
    auto counterNode = pattern->GetCounterNode().Upgrade();
    if (counterNode && !pattern->IsNormalInlineState()) {
        auto counterSize = counterNode->GetGeometryNode()->GetFrameSize();
        contentHeight += counterSize.Height();
    }

    if (pattern->IsNormalInlineState() && pattern->HasFocus()) {
        frameSize.SetWidth(contentWidth + pattern->GetHorizontalPaddingAndBorderSum() + PARAGRAPH_SAVE_BOUNDARY);
        frameSize.SetHeight(contentHeight + pattern->GetVerticalPaddingAndBorderSum() + PARAGRAPH_SAVE_BOUNDARY);
    } else {
        // The width after MeasureContent is already optimal, but the height needs to be constrained in Measure.
        frameSize.SetWidth(contentWidth + pattern->GetHorizontalPaddingAndBorderSum());
        auto textFieldLayoutProperty = pattern->GetLayoutProperty<TextFieldLayoutProperty>();
        CHECK_NULL_VOID(textFieldLayoutProperty);
        auto contentConstraint = layoutWrapper->GetLayoutProperty()->CreateContentConstraint();
        auto textFieldContentConstraint =
            CalculateContentMaxSizeWithCalculateConstraint(contentConstraint, layoutWrapper);
        if (textFieldContentConstraint.selfIdealSize.Height().has_value()) {
            frameSize.SetHeight(
                textFieldContentConstraint.maxSize.Height() + pattern->GetVerticalPaddingAndBorderSum());
        } else {
            frameSize.SetHeight(contentHeight + pattern->GetVerticalPaddingAndBorderSum());
        }
        // Height is constrained by the CalcLayoutConstraint.
        if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
            frameSize.Constrain(layoutConstraint->minSize, layoutConstraint->maxSize);
        } else {
            auto finalSize = UpdateOptionSizeByCalcLayoutConstraint(frameSize,
                layoutWrapper->GetLayoutProperty()->GetCalcLayoutConstraint(),
                layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()->percentReference);
            frameSize.SetHeight(finalSize.Height());
        }
    }
    layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize.ConvertToSizeT());
}

void TextAreaLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    // update child position.
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize() -
                SizeF(pattern->GetHorizontalPaddingAndBorderSum(), pattern->GetVerticalPaddingAndBorderSum());
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    CHECK_NULL_VOID(content);
    auto layoutProperty = DynamicCast<TextFieldLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto context = layoutWrapper->GetHostNode()->GetContext();
    CHECK_NULL_VOID(context);
    parentGlobalOffset_ = layoutWrapper->GetHostNode()->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    auto align = Alignment::TOP_CENTER;

    auto offsetBase = OffsetF(
        pattern->GetPaddingLeft() + pattern->GetBorderLeft(), pattern->GetPaddingTop() + pattern->GetBorderTop());
    if (layoutWrapper->GetLayoutProperty()->GetPositionProperty()) {
        align = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().value_or(align);
    }
    // Update text position.
    if (LessOrEqual(textRect_.Height(), content->GetRect().Height())) {
        OffsetF textRectOffSet = Alignment::GetAlignPosition(size, textRect_.GetSize(), align);
        textRect_.SetOffset(OffsetF(0.0f, textRectOffSet.GetY()) + offsetBase);
        content->SetOffset(OffsetF(0.0f, textRectOffSet.GetY()) + offsetBase);
    } else {
        textRect_.SetOffset(OffsetF(offsetBase.GetX(), pattern->GetTextRect().GetOffset().GetY()));
        content->SetOffset(offsetBase);
    }
    // CounterNode Layout.
    auto isInlineStyle = pattern->IsNormalInlineState();
    if (layoutProperty->GetShowCounterValue(false) && layoutProperty->HasMaxLength() && !isInlineStyle) {
        TextFieldLayoutAlgorithm::CounterLayout(layoutWrapper);
    }
}
} // namespace OHOS::Ace::NG
