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

#include "core/components_ng/pattern/badge/badge_layout_algorithm.h"

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/components/badge/badge_theme.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/badge/badge_layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

void BadgeLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    if (children.empty()) {
        return;
    }
    auto childrenSize = children.size();
    auto layoutProperty = AceType::DynamicCast<BadgeLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();

    auto textFirstLayoutConstraint = childLayoutConstraint;
    textFirstLayoutConstraint.maxSize = { Infinity<float>(), Infinity<float>() };

    auto textWrapper = layoutWrapper->GetOrCreateChildByIndex(childrenSize - 1);
    CHECK_NULL_VOID(textWrapper);
    auto textLayoutProperty = AceType::DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textLayoutProperty);
    auto textGeometryNode = textWrapper->GetGeometryNode();
    CHECK_NULL_VOID(textGeometryNode);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto badgeTheme = pipeline->GetTheme<BadgeTheme>();
    CHECK_NULL_VOID(badgeTheme);

    auto badgeFontSize = layoutProperty->GetBadgeFontSize();
    if (badgeFontSize.has_value() && GreatOrEqual(badgeFontSize.value().ConvertToPx(), 0)) {
        hasFontSize_ = true;
        textLayoutProperty->UpdateFontSize(badgeFontSize.value());
    } else {
        hasFontSize_ = false;
        auto badgeThemeFontSize = badgeTheme->GetBadgeFontSize();
        textLayoutProperty->UpdateFontSize(badgeThemeFontSize);
    }
    if (textWrapper) {
        textWrapper->Measure(textFirstLayoutConstraint);
    }

    auto badgeCircleSize = badgeTheme->GetBadgeCircleSize();
    auto badgeValue = layoutProperty->GetBadgeValue();
    if (badgeValue.has_value() && badgeValue.value().empty()) {
        badgeCircleSize = badgeTheme->GetLittleBadgeCircleSize();
    }
    auto circleSize = layoutProperty->GetBadgeCircleSize();
    auto badgeCircleDiameter = circleSize.has_value() ? (circleSize->IsValid() ? circleSize->ConvertToPx() : 0)
                                                      : badgeCircleSize.ConvertToPx();

    auto badgeWidth = 0.0;
    auto badgeHeight = badgeCircleDiameter;
    auto countLimit = layoutProperty->GetBadgeMaxCountValue();
    auto badgeCircleRadius = badgeCircleDiameter / 2;

    std::string textData;
    if (textLayoutProperty->HasContent()) {
        textData = textLayoutProperty->GetContentValue();
    }

    auto messageCount = textData.size();
    auto textSize = textGeometryNode->GetContentSize();
    if (!textData.empty() || messageCount > 0) {
        if ((textData.size() <= 1 && !textData.empty()) ||
            ((messageCount < 10 && messageCount <= countLimit) && textData.empty())) {
            if (hasFontSize_) {
                badgeCircleDiameter = std::max(static_cast<double>(textSize.Height()), badgeCircleDiameter);
                badgeHeight = std::max(badgeCircleDiameter, badgeHeight);
            }
            badgeCircleRadius = badgeCircleDiameter / 2;
            badgeWidth = badgeCircleDiameter;
        } else if (textData.size() > 1 || messageCount > countLimit) {
            if (hasFontSize_) {
                badgeCircleDiameter = std::max(static_cast<double>(textSize.Height()), badgeCircleDiameter);
                badgeHeight = std::max(badgeCircleDiameter, badgeHeight);
            }
            badgeWidth = textSize.Width() + badgeTheme->GetNumericalBadgePadding().ConvertToPx() * 2;
            badgeWidth = badgeCircleDiameter > badgeWidth ? badgeCircleDiameter : badgeWidth;
            badgeCircleRadius = badgeCircleDiameter / 2;
        }
    }
    if (LessOrEqual(circleSize->ConvertToPx(), 0)) {
        badgeWidth = 0;
        badgeHeight = 0;
    }
    textLayoutProperty->UpdateMarginSelfIdealSize(SizeF(badgeWidth, badgeHeight));
    auto textLayoutConstraint = textFirstLayoutConstraint;
    textLayoutConstraint.selfIdealSize = OptionalSize<float>(badgeWidth, badgeHeight);

    textWrapper->Measure(textLayoutConstraint);
    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(childrenSize - 2);
    CHECK_NULL_VOID(childWrapper);
    childWrapper->Measure(childLayoutConstraint);

    PerformMeasureSelf(layoutWrapper);
}

void BadgeLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto host = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    if (children.empty()) {
        return;
    }
    auto childrenSize = children.size();

    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto offset = geometryNode->GetFrameOffset();

    auto layoutProperty = DynamicCast<BadgeLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto badgeTheme = pipeline->GetTheme<BadgeTheme>();
    CHECK_NULL_VOID(badgeTheme);
    auto badgeCircleSize = badgeTheme->GetBadgeCircleSize();
    auto circleSize = layoutProperty->GetBadgeCircleSize();
    auto badgeCircleDiameter = circleSize.has_value() ? (circleSize->IsValid() ? circleSize->ConvertToPx() : 0)
                                                      : badgeCircleSize.ConvertToPx();

    auto badgeWidth = 0.0;
    auto badgeCircleRadius = badgeCircleDiameter / 2;
    auto countLimit =
        layoutProperty->HasBadgeMaxCount() ? layoutProperty->GetBadgeMaxCountValue() : badgeTheme->GetMaxCount();

    auto textWrapper = layoutWrapper->GetOrCreateChildByIndex(childrenSize - 1);
    CHECK_NULL_VOID(textWrapper);
    auto textLayoutProperty = DynamicCast<TextLayoutProperty>(textWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(textLayoutProperty);
    auto textGeometryNode = textWrapper->GetGeometryNode();
    CHECK_NULL_VOID(textGeometryNode);

    std::string textData;
    if (textLayoutProperty->HasContent()) {
        textData = textLayoutProperty->GetContentValue();
    }

    auto messageCount = textData.size();
    auto textSize = textGeometryNode->GetContentSize();

    if (!textData.empty() || messageCount > 0) {
        if ((textData.size() <= 1 && !textData.empty()) ||
            ((messageCount < 10 && messageCount <= countLimit) && textData.empty())) {
            if (hasFontSize_) {
                badgeCircleDiameter = std::max(static_cast<double>(textSize.Height()), badgeCircleDiameter);
            }
            badgeCircleRadius = badgeCircleDiameter / 2;
            badgeWidth = badgeCircleDiameter;
        } else if (textData.size() > 1 || messageCount > countLimit) {
            if (hasFontSize_) {
                badgeCircleDiameter = std::max(static_cast<double>(textSize.Height()), badgeCircleDiameter);
            }
            badgeWidth = textSize.Width() + badgeTheme->GetNumericalBadgePadding().ConvertToPx() * 2;
            badgeWidth = badgeCircleDiameter > badgeWidth ? badgeCircleDiameter : badgeWidth;
            badgeCircleRadius = badgeCircleDiameter / 2;
        }
    }

    BorderRadiusProperty radius;
    auto borderWidth = layoutProperty->GetBadgeBorderWidthValue(badgeTheme->GetBadgeBorderWidth());
    OffsetF borderOffset(borderWidth.ConvertToPx(), borderWidth.ConvertToPx());
    radius.SetRadius(Dimension(badgeCircleRadius + borderWidth.ConvertToPx()));
    auto textFrameNode = textWrapper->GetHostNode();
    CHECK_NULL_VOID(textFrameNode);
    auto textRenderContext = textFrameNode->GetRenderContext();
    CHECK_NULL_VOID(textRenderContext);
    textRenderContext->UpdateBorderRadius(radius);

    textLayoutProperty->UpdateAlignment(Alignment::CENTER);

    OffsetF textOffset;
    if (!layoutProperty->GetIsPositionXy().value()) {
        auto parentSize = geometryNode->GetFrameSize();
        auto width = parentSize.Width();
        auto height = parentSize.Height();
        auto badgePosition = layoutProperty->GetBadgePosition();
        if (textData == " ") {
            if (badgePosition == BadgePosition::RIGHT_TOP) {
                textOffset = OffsetF(offset.GetX() + width - badgeCircleDiameter, offset.GetY());
            } else if (badgePosition == BadgePosition::RIGHT) {
                textOffset = OffsetF(
                    offset.GetX() + width - badgeCircleDiameter, offset.GetY() + height / 2 - badgeCircleRadius);
            } else {
                textOffset = OffsetF(offset.GetX(), offset.GetY() + height / 2 - badgeCircleRadius);
            }
        } else {
            if (badgePosition == BadgePosition::RIGHT_TOP) {
                textOffset = OffsetF(
                    width - badgeCircleDiameter + Dimension(2.0_vp).ConvertToPx(), 0 - Dimension(2.0_vp).ConvertToPx());
                textOffset = OffsetF(offset.GetX() + textOffset.GetX(), offset.GetY() + textOffset.GetY());
            } else if (badgePosition == BadgePosition::RIGHT) {
                textOffset = OffsetF(width - badgeCircleDiameter, height / 2 - badgeCircleRadius);
                textOffset = OffsetF(offset.GetX() + textOffset.GetX(), offset.GetY() + textOffset.GetY());
            } else {
                textOffset = OffsetF(0, height / 2 - badgeCircleRadius);
                textOffset = OffsetF(offset.GetX(), offset.GetY() + textOffset.GetY());
            }
        }
    } else {
        auto badgePositionX = layoutProperty->GetBadgePositionX();
        auto badgePositionY = layoutProperty->GetBadgePositionY();
        textOffset =
            OffsetF(offset.GetX() + badgePositionX->ConvertToPx(), offset.GetY() + badgePositionY->ConvertToPx());
    }
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        textGeometryNode->SetMarginFrameOffset(textOffset - geometryNode->GetFrameOffset());
    } else {
        textGeometryNode->SetMarginFrameOffset(textOffset - geometryNode->GetFrameOffset() - borderOffset);
    }
    auto textFrameSize = textGeometryNode->GetFrameSize();
    if (GreatNotEqual(circleSize->ConvertToPx(), 0) && Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        textFrameSize += SizeF(borderWidth.ConvertToPx() * 2, borderWidth.ConvertToPx() * 2);
    }
    textGeometryNode->SetFrameSize(textFrameSize);
    textWrapper->Layout();

    auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(childrenSize - 2);
    CHECK_NULL_VOID(childWrapper);
    childWrapper->Layout();
}

void BadgeLayoutAlgorithm::PerformMeasureSelf(LayoutWrapper* layoutWrapper)
{
    const auto& layoutConstraint = layoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    const auto& minSize = layoutConstraint->minSize;
    const auto& maxSize = layoutConstraint->maxSize;
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    OptionalSizeF frameSize;
    do {
        // Use idea size first if it is valid.
        frameSize.UpdateSizeWithCheck(layoutConstraint->selfIdealSize);
        if (frameSize.IsValid()) {
            break;
        }
        // use the last child size.
        auto host = layoutWrapper->GetHostNode();
        CHECK_NULL_VOID(host);
        auto children = host->GetChildren();
        auto childrenSize = children.size();
        auto childFrame =
            layoutWrapper->GetOrCreateChildByIndex(childrenSize - 2)->GetGeometryNode()->GetMarginFrameSize();
        AddPaddingToSize(padding, childFrame);
        frameSize.UpdateIllegalSizeWithCheck(childFrame);
        frameSize.Constrain(minSize, maxSize);
        frameSize.UpdateIllegalSizeWithCheck(SizeF { 0.0f, 0.0f });
    } while (false);

    layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize.ConvertToSizeT());
}

} // namespace OHOS::Ace::NG
