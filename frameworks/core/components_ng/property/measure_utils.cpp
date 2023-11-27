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

#include "core/components_ng/property/measure_utils.h"

#include <memory>
#include <optional>

#include "base/geometry/ng/size_t.h"
#include "base/geometry/size.h"
#include "base/log/log.h"
#include "base/utils/utils.h"
#include "core/components_ng/property/measure_property.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
namespace {
const static int32_t PLATFORM_VERSION_TEN = 10;
}

SizeF ConvertToSize(const CalcSize& size, const ScaleProperty& scaleProperty, const SizeF& percentReference)
{
    auto width = ConvertToPx(size.Width(), scaleProperty, percentReference.Width());
    auto height = ConvertToPx(size.Height(), scaleProperty, percentReference.Height());
    return { width.value_or(-1.0f), height.value_or(-1.0f) };
}

OptionalSizeF ConvertToOptionalSize(
    const CalcSize& size, const ScaleProperty& scaleProperty, const SizeF& percentReference)
{
    auto width = ConvertToPx(size.Width(), scaleProperty, percentReference.Width());
    auto height = ConvertToPx(size.Height(), scaleProperty, percentReference.Height());
    return { width, height };
}

std::optional<float> ConvertToPx(const CalcLength& value, const ScaleProperty& scaleProperty, float percentReference)
{
    double result = -1.0;
    if (!value.NormalizeToPx(
            scaleProperty.vpScale, scaleProperty.fpScale, scaleProperty.lpxScale, percentReference, result)) {
        LOGE("fail to Convert CalcDimension To Px: %{public}f, %{public}f, %{public}f, %{public}f",
            scaleProperty.vpScale, scaleProperty.fpScale, scaleProperty.lpxScale, percentReference);
        return std::nullopt;
    }
    return static_cast<float>(result);
}

std::optional<float> ConvertToPx(
    const std::optional<CalcLength>& value, const ScaleProperty& scaleProperty, float percentReference)
{
    if (!value) {
        return std::nullopt;
    }
    double result = -1.0;
    if (!value.value().NormalizeToPx(
            scaleProperty.vpScale, scaleProperty.fpScale, scaleProperty.lpxScale, percentReference, result)) {
        LOGE("optional: fail to Convert CalcDimension To Px: %{public}f, %{public}f, %{public}f, %{public}f",
            scaleProperty.vpScale, scaleProperty.fpScale, scaleProperty.lpxScale, percentReference);
        return std::nullopt;
    }
    return static_cast<float>(result);
}

std::optional<float> ConvertToPx(const Dimension& dimension, const ScaleProperty& scaleProperty, float percentReference)
{
    double result = -1.0;
    if (!dimension.NormalizeToPx(
            scaleProperty.vpScale, scaleProperty.fpScale, scaleProperty.lpxScale, percentReference, result)) {
        LOGE("fail to Convert dimension To Px: %{public}f, %{public}f, %{public}f, %{public}f", scaleProperty.vpScale,
            scaleProperty.fpScale, scaleProperty.lpxScale, percentReference);
        return std::nullopt;
    }
    return static_cast<float>(result);
}

std::optional<float> ConvertToPx(
    const std::optional<Dimension>& dimension, const ScaleProperty& scaleProperty, float percentReference)
{
    if (!dimension) {
        return std::nullopt;
    }
    double result = -1.0;
    if (!dimension.value().NormalizeToPx(
            scaleProperty.vpScale, scaleProperty.fpScale, scaleProperty.lpxScale, percentReference, result)) {
        LOGE("fail to Convert dimension To Px: %{public}f, %{public}f, %{public}f, %{public}f", scaleProperty.vpScale,
            scaleProperty.fpScale, scaleProperty.lpxScale, percentReference);
        return std::nullopt;
    }
    return static_cast<float>(result);
}

SizeF ConstrainSize(const SizeF& size, const SizeF& minSize, const SizeF& maxSize)
{
    float height = std::max(minSize.Height(), size.Height());
    if (maxSize.Height() > 0) {
        height = std::min(maxSize.Height(), height);
    }
    float width = std::max(minSize.Width(), size.Width());
    if (maxSize.Width() > 0) {
        width = std::min(maxSize.Width(), width);
    }
    return { width, height };
}

PaddingPropertyF ConvertToPaddingPropertyF(
    const std::unique_ptr<PaddingProperty>& padding, const ScaleProperty& scaleProperty, float percentReference)
{
    if (!padding) {
        return {};
    }
    return ConvertToPaddingPropertyF(*padding, scaleProperty, percentReference);
}

PaddingPropertyF ConvertToPaddingPropertyF(
    const PaddingProperty& padding, const ScaleProperty& scaleProperty, float percentReference)
{
    auto left = ConvertToPx(padding.left, scaleProperty, percentReference);
    auto right = ConvertToPx(padding.right, scaleProperty, percentReference);
    auto top = ConvertToPx(padding.top, scaleProperty, percentReference);
    auto bottom = ConvertToPx(padding.bottom, scaleProperty, percentReference);
    return PaddingPropertyF { left, right, top, bottom };
}

MarginPropertyF ConvertToMarginPropertyF(
    const std::unique_ptr<MarginProperty>& margin, const ScaleProperty& scaleProperty, float percentReference)
{
    return ConvertToPaddingPropertyF(margin, scaleProperty, percentReference);
}

MarginPropertyF ConvertToMarginPropertyF(
    const MarginProperty& margin, const ScaleProperty& scaleProperty, float percentReference)
{
    return ConvertToPaddingPropertyF(margin, scaleProperty, percentReference);
}

BorderWidthPropertyF ConvertToBorderWidthPropertyF(
    const std::unique_ptr<BorderWidthProperty>& borderWidth, const ScaleProperty& scaleProperty, float percentReference)
{
    if (!borderWidth) {
        return {};
    }
    return ConvertToBorderWidthPropertyF(*borderWidth, scaleProperty, percentReference);
}

BorderWidthPropertyF ConvertToBorderWidthPropertyF(
    const BorderWidthProperty& borderWidth, const ScaleProperty& scaleProperty, float percentReference)
{
    auto left = ConvertToPx(borderWidth.leftDimen, scaleProperty, percentReference);
    auto right = ConvertToPx(borderWidth.rightDimen, scaleProperty, percentReference);
    auto top = ConvertToPx(borderWidth.topDimen, scaleProperty, percentReference);
    auto bottom = ConvertToPx(borderWidth.bottomDimen, scaleProperty, percentReference);
    return BorderWidthPropertyF { left, top, right, bottom };
}

void UpdatePaddingPropertyF(const PaddingProperty& padding, const ScaleProperty& scaleProperty, const SizeF& selfSize,
    PaddingPropertyF& paddingValue)
{
    auto left = ConvertToPx(padding.left, scaleProperty, selfSize.Width());
    auto right = ConvertToPx(padding.right, scaleProperty, selfSize.Width());
    auto top = ConvertToPx(padding.top, scaleProperty, selfSize.Height());
    auto bottom = ConvertToPx(padding.bottom, scaleProperty, selfSize.Height());
    if (left.has_value()) {
        paddingValue.left = left;
    }
    if (right.has_value()) {
        paddingValue.right = right;
    }
    if (top.has_value()) {
        paddingValue.top = top;
    }
    if (bottom.has_value()) {
        paddingValue.bottom = bottom;
    }
}

void AddPaddingToSize(const PaddingPropertyF& padding, SizeF& size)
{
    size.AddPadding(padding.left, padding.right, padding.top, padding.bottom);
}

void MinusPaddingToSize(const PaddingPropertyF& padding, SizeF& size)
{
    size.MinusPadding(padding.left, padding.right, padding.top, padding.bottom);
}

void MinusPaddingToNonNegativeSize(const PaddingPropertyF& padding, SizeF& size)
{
    size.MinusPaddingToNonNegative(padding.left, padding.right, padding.top, padding.bottom);
}

void AddPaddingToSize(const PaddingPropertyF& padding, OptionalSizeF& size)
{
    size.AddPadding(padding.left, padding.right, padding.top, padding.bottom);
}

void MinusPaddingToSize(const PaddingPropertyF& padding, OptionalSizeF& size)
{
    size.MinusPadding(padding.left, padding.right, padding.top, padding.bottom);
}

float GetMainAxisOffset(const OffsetF& offset, Axis axis)
{
    return axis == Axis::HORIZONTAL ? offset.GetX() : offset.GetY();
}

float GetMainAxisSize(const SizeF& size, Axis axis)
{
    return axis == Axis::HORIZONTAL ? size.Width() : size.Height();
}

float GetCrossAxisSize(const SizeF& size, Axis axis)
{
    return axis == Axis::HORIZONTAL ? size.Height() : size.Width();
}

void SetCrossAxisSize(float value, Axis axis, SizeF& size)
{
    if (axis == Axis::VERTICAL) {
        size.SetWidth(value);
        return;
    }
    size.SetHeight(value);
}

std::optional<float> GetMainAxisSize(const OptionalSizeF& size, Axis axis)
{
    return axis == Axis::HORIZONTAL ? size.Width() : size.Height();
}

std::optional<float> GetCrossAxisSize(const OptionalSizeF& size, Axis axis)
{
    return axis == Axis::HORIZONTAL ? size.Height() : size.Width();
}

void SetCrossAxisSize(float value, Axis axis, OptionalSizeF& size)
{
    if (axis == Axis::VERTICAL) {
        size.SetWidth(value);
        return;
    }
    size.SetHeight(value);
}

void SetMainAxisSize(float value, Axis axis, OptionalSizeF& size)
{
    if (axis == Axis::VERTICAL) {
        size.SetHeight(value);
        return;
    }
    size.SetWidth(value);
}

SizeF CreateIdealSize(const LayoutConstraintF& layoutConstraint, Axis axis, MeasureType measureType, bool usingMaxSize)
{
    auto optional = CreateIdealSize(layoutConstraint, axis, measureType);
    if (usingMaxSize) {
        optional.UpdateIllegalSizeWithCheck(layoutConstraint.maxSize);
    } else {
        optional.UpdateIllegalSizeWithCheck(layoutConstraint.minSize);
    }
    return optional.ConvertToSizeT();
}

OptionalSizeF CreateIdealSize(const LayoutConstraintF& layoutConstraint, Axis axis, MeasureType measureType)
{
    OptionalSizeF idealSize;
    do {
        // Use idea size first if it is valid.
        idealSize.UpdateSizeWithCheck(layoutConstraint.selfIdealSize);
        if (idealSize.IsValid()) {
            break;
        }

        if (measureType == MeasureType::MATCH_PARENT) {
            idealSize.UpdateIllegalSizeWithCheck(layoutConstraint.parentIdealSize);
            idealSize.UpdateIllegalSizeWithCheck(layoutConstraint.maxSize);
            break;
        }

        if (measureType == MeasureType::MATCH_PARENT_CROSS_AXIS) {
            auto selfSize = GetCrossAxisSize(idealSize, axis);
            if (!selfSize) {
                auto parentCrossSize = GetCrossAxisSize(layoutConstraint.parentIdealSize, axis);
                if (parentCrossSize) {
                    SetCrossAxisSize(parentCrossSize.value(), axis, idealSize);
                } else {
                    parentCrossSize = GetCrossAxisSize(layoutConstraint.maxSize, axis);
                    SetCrossAxisSize(parentCrossSize.value(), axis, idealSize);
                }
            }
            break;
        }

        if (measureType == MeasureType::MATCH_PARENT_MAIN_AXIS) {
            auto selfSize = GetMainAxisSize(idealSize, axis);
            auto parentMainSize = GetMainAxisSize(layoutConstraint.parentIdealSize, axis);
            if (!selfSize) {
                if (parentMainSize) {
                    SetMainAxisSize(parentMainSize.value(), axis, idealSize);
                } else {
                    parentMainSize = GetMainAxisSize(layoutConstraint.maxSize, axis);
                    SetMainAxisSize(parentMainSize.value(), axis, idealSize);
                }
            }
            break;
        }
    } while (false);
    return idealSize;
}

OptionalSizeF UpdateOptionSizeByCalcLayoutConstraint(const OptionalSize<float>& frameSize,
    const std::unique_ptr<MeasureProperty>& calcLayoutConstraint, const SizeT<float> percentReference)
{
    OptionalSizeF finalSize(frameSize.Width(), frameSize.Height());
    if (!calcLayoutConstraint) {
        return finalSize;
    } else {
        UpdateOptionSizeByMaxOrMinCalcLayoutConstraint(
            finalSize, calcLayoutConstraint->maxSize, percentReference, true);
        UpdateOptionSizeByMaxOrMinCalcLayoutConstraint(
            finalSize, calcLayoutConstraint->minSize, percentReference, false);
    }
    return finalSize;
}

void UpdateOptionSizeByMaxOrMinCalcLayoutConstraint(OptionalSizeF& frameSize,
    const std::optional<CalcSize>& calcLayoutConstraintMaxMinSize, const SizeT<float> percentReference, bool IsMaxSize)
{
    auto scaleProperty = ScaleProperty::CreateScaleProperty();
    if (!calcLayoutConstraintMaxMinSize.has_value()) {
        return;
    }
    if (calcLayoutConstraintMaxMinSize->Width().has_value()) {
        auto maxWidthPx = ConvertToPx(calcLayoutConstraintMaxMinSize->Width(), scaleProperty, percentReference.Width());
        if (IsMaxSize) {
            frameSize.SetWidth(std::min(maxWidthPx.value(), frameSize.Width().value_or(maxWidthPx.value())));
        } else {
            frameSize.SetWidth(std::max(maxWidthPx.value(), frameSize.Width().value_or(maxWidthPx.value())));
        }
    }
    if (calcLayoutConstraintMaxMinSize->Height().has_value()) {
        auto maxHeightPx =
            ConvertToPx(calcLayoutConstraintMaxMinSize->Height(), scaleProperty, percentReference.Height());
        if (IsMaxSize) {
            frameSize.SetHeight(std::min(maxHeightPx.value(), frameSize.Height().value_or(maxHeightPx.value())));
        } else {
            frameSize.SetHeight(std::max(maxHeightPx.value(), frameSize.Height().value_or(maxHeightPx.value())));
        }
    }
}

OptionalSizeF CreateIdealSizeByPercentRef(
    const LayoutConstraintF& layoutConstraint, Axis axis, MeasureType measureType, bool needToConstrain)
{
    OptionalSizeF idealSize;
    do {
        // Use idea size first if it is valid.
        idealSize.UpdateSizeWithCheck(layoutConstraint.selfIdealSize);
        if (idealSize.IsValid()) {
            break;
        }

        if (measureType == MeasureType::MATCH_PARENT) {
            idealSize.UpdateIllegalSizeWithCheck(layoutConstraint.parentIdealSize);
            idealSize.UpdateIllegalSizeWithCheck(layoutConstraint.percentReference);
            break;
        }

        if (measureType == MeasureType::MATCH_PARENT_CROSS_AXIS) {
            auto selfSize = GetCrossAxisSize(idealSize, axis);
            if (!selfSize) {
                auto parentCrossSize = GetCrossAxisSize(layoutConstraint.parentIdealSize, axis);
                if (parentCrossSize) {
                    SetCrossAxisSize(parentCrossSize.value(), axis, idealSize);
                } else {
                    parentCrossSize = GetCrossAxisSize(layoutConstraint.percentReference, axis);
                    SetCrossAxisSize(parentCrossSize.value(), axis, idealSize);
                }
            }
            break;
        }

        if (measureType == MeasureType::MATCH_PARENT_MAIN_AXIS) {
            auto selfSize = GetMainAxisSize(idealSize, axis);
            auto parentMainSize = GetMainAxisSize(layoutConstraint.parentIdealSize, axis);
            if (!selfSize) {
                if (parentMainSize) {
                    SetMainAxisSize(parentMainSize.value(), axis, idealSize);
                } else {
                    parentMainSize = GetMainAxisSize(layoutConstraint.percentReference, axis);
                    SetMainAxisSize(parentMainSize.value(), axis, idealSize);
                }
            }
            break;
        }
    } while (false);
    if (needToConstrain) {
        idealSize.Constrain(layoutConstraint.minSize, layoutConstraint.maxSize,
            PipelineBase::GetCurrentContext() &&
                PipelineBase::GetCurrentContext()->GetMinPlatformVersion() >= PLATFORM_VERSION_TEN);
    }
    return idealSize;
}

void CreateChildrenConstraint(SizeF& size, const PaddingPropertyF& padding)
{
    float width = 0;
    float height = 0;

    float paddingLeft = padding.left.has_value() ? padding.left.value() : 0;
    float paddingRight = padding.right.has_value() ? padding.right.value() : 0;
    float paddingTop = padding.top.has_value() ? padding.top.value() : 0;
    float paddingBottom = padding.bottom.has_value() ? padding.bottom.value() : 0;
    width += (paddingLeft + paddingRight);
    height += (paddingTop + paddingBottom);

    size.SetHeight(size.Height() - height);
    size.SetWidth(size.Width() - width);
}

PaddingProperty ConvertToCalcPaddingProperty(const std::optional<CalcDimension>& top,
    const std::optional<CalcDimension>& bottom, const std::optional<CalcDimension>& left,
    const std::optional<CalcDimension>& right)
{
    PaddingProperty paddings;
    if (top.has_value()) {
        if (top.value().Unit() == DimensionUnit::CALC) {
            paddings.top =
                NG::CalcLength(top.value().IsNonNegative() ? top.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.top = NG::CalcLength(top.value().IsNonNegative() ? top.value() : CalcDimension());
        }
    }
    if (bottom.has_value()) {
        if (bottom.value().Unit() == DimensionUnit::CALC) {
            paddings.bottom = NG::CalcLength(
                bottom.value().IsNonNegative() ? bottom.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.bottom = NG::CalcLength(bottom.value().IsNonNegative() ? bottom.value() : CalcDimension());
        }
    }
    if (left.has_value()) {
        if (left.value().Unit() == DimensionUnit::CALC) {
            paddings.left =
                NG::CalcLength(left.value().IsNonNegative() ? left.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.left = NG::CalcLength(left.value().IsNonNegative() ? left.value() : CalcDimension());
        }
    }
    if (right.has_value()) {
        if (right.value().Unit() == DimensionUnit::CALC) {
            paddings.right =
                NG::CalcLength(right.value().IsNonNegative() ? right.value().CalcValue() : CalcDimension().CalcValue());
        } else {
            paddings.right = NG::CalcLength(right.value().IsNonNegative() ? right.value() : CalcDimension());
        }
    }
    return paddings;
}
} // namespace OHOS::Ace::NG
