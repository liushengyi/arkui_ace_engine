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

#include "core/components_ng/pattern/data_panel/data_panel_modifier.h"

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/rect.h"
#include "base/geometry/rrect.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/color.h"
#include "core/components/data_panel/data_panel_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/data_panel/data_panel_paint_property.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float FIXED_WIDTH = 1.0f;
constexpr float HALF_CIRCLE = 180.0f;
constexpr float WHOLE_CIRCLE = 360.0f;
constexpr float QUARTER_CIRCLE = 90.0f;
constexpr float PERCENT_HALF = 0.5f;
constexpr float DIAMETER_TO_THICKNESS_RATIO = 0.12f;
constexpr float FIXED_ANGLE = 2.0f;
constexpr float FIXED_DRAW_ANGLE = 4.0f;
constexpr float SHADOW_FILTER = 20.0f;
constexpr uint32_t SHADOW_ALPHA = 0.4 * 255;
} // namespace

DataPanelModifier::DataPanelModifier()
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto theme = pipelineContext->GetTheme<DataPanelTheme>();
    auto colors = theme->GetColorsArray();

    date_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(1.0);
    for (size_t i = 0; i < MAX_COUNT; ++i) {
        auto value = AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0);
        AttachProperty(value);
        values_.emplace_back(value);
    }
    max_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(DEFAULT_MAX_VALUE);
    trackBackgroundColor_ = AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor(theme->GetBackgroundColor()));
    strokeWidth_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetThickness().ConvertToPx());
    isEffect_ = AceType::MakeRefPtr<PropertyBool>(true);
    AttachProperty(date_);
    AttachProperty(max_);
    AttachProperty(trackBackgroundColor_);
    AttachProperty(strokeWidth_);
    AttachProperty(isEffect_);

    shadowRadiusFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetTrackShadowRadius().ConvertToPx());
    shadowOffsetXFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetTrackShadowOffsetX().ConvertToPx());
    shadowOffsetYFloat_ = AceType::MakeRefPtr<AnimatablePropertyFloat>(theme->GetTrackShadowOffsetY().ConvertToPx());
    AttachProperty(shadowRadiusFloat_);
    AttachProperty(shadowOffsetXFloat_);
    AttachProperty(shadowOffsetYFloat_);

    for (const auto& item : colors) {
        Gradient gradient;
        GradientColor gradientColorStart;
        gradientColorStart.SetLinearColor(LinearColor(item.first));
        gradientColorStart.SetDimension(Dimension(0.0));
        gradient.AddColor(gradientColorStart);
        GradientColor gradientColorEnd;
        gradientColorEnd.SetLinearColor(LinearColor(item.second));
        gradientColorEnd.SetDimension(Dimension(1.0));
        gradient.AddColor(gradientColorEnd);

        auto gradientColor = AceType::MakeRefPtr<AnimatablePropertyVectorColor>(GradientArithmetic(gradient));
        AttachProperty(gradientColor);
        valueColors_.emplace_back(gradientColor);

        auto gradientShadowColor = AceType::MakeRefPtr<AnimatablePropertyVectorColor>(GradientArithmetic(gradient));
        AttachProperty(gradientShadowColor);
        shadowColors_.emplace_back(gradientShadowColor);
    }
}

void DataPanelModifier::onDraw(DrawingContext& context)
{
    if (dataPanelType_ == 0) {
        PaintCircle(context, offset_);
    } else {
        PaintLinearProgress(context, offset_);
    }
}

void DataPanelModifier::UpdateDate()
{
    if (isEffect_->Get()) {
        // When the date update, the animation will repeat once.
        AnimationOption option = AnimationOption();
        option.SetDuration(0);
        option.SetDelay(ANIMATION_DELAY);
        option.SetIteration(ANIMATION_TIMES);
        AnimationUtils::Animate(option, [&]() { date_->Set(OHOS::Ace::NG::ANIMATION_START); });
        RefPtr<Curve> curve = AceType::MakeRefPtr<SpringCurve>(
            ANIMATION_CURVE_VELOCITY, ANIMATION_CURVE_MASS, ANIMATION_CURVE_STIFFNESS, ANIMATION_CURVE_DAMPING);
        option.SetDuration(ANIMATION_DURATION);
        option.SetDelay(ANIMATION_DELAY);
        option.SetCurve(curve);
        option.SetIteration(ANIMATION_TIMES);
        AnimationUtils::Animate(option, [&]() { date_->Set(ANIMATION_END); });
    }
}

void DataPanelModifier::PaintRainbowFilterMask(RSCanvas& canvas, ArcData arcData) const
{
    float thickness = arcData.thickness;
    float radius = arcData.radius;
    double totalValue = arcData.totalValue;
    float drawAngle = arcData.totalValue / arcData.maxValue * WHOLE_CIRCLE * date_->Get();
    drawAngle = std::min(drawAngle, WHOLE_CIRCLE);
    Offset center = arcData.center + Offset(shadowOffsetXFloat_->Get(), shadowOffsetYFloat_->Get());

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    float preItemPos = 0.0f;
    float drawValue = 0.0f;

    for (size_t i = 0; i < shadowColorsLastLength_; ++i) {
        float itemPos = 0.0f;
        drawValue += values_[i]->Get();
        arcData.shadowColor = SortGradientColorsOffset(shadowColors_[i]->Get().GetGradient());
        size_t length = arcData.shadowColor.GetColors().size();
        for (size_t j = 0; j < length; ++j) {
            itemPos = (values_[i]->Get() / totalValue) * arcData.shadowColor.GetColors().at(j).GetDimension().Value() +
                      preItemPos;
            pos.emplace_back(itemPos);
            colors.emplace_back(arcData.shadowColor.GetColors().at(j).GetLinearColor().GetValue());
        }
        preItemPos = itemPos;
    }

    RSPen gradientPaint;
    gradientPaint.SetWidth(thickness);
    gradientPaint.SetAntiAlias(true);
    gradientPaint.SetAlpha(SHADOW_ALPHA);
    RSFilter filter;
#ifndef USE_ROSEN_DRAWING
    filter.SetImageFilter(
        RSImageFilter::CreateBlurImageFilter(SHADOW_FILTER, SHADOW_FILTER, RSTileMode::DECAL, nullptr));
    gradientPaint.SetFilter(filter);
    RSPath path;
#else
    filter.SetImageFilter(
        RSRecordingImageFilter::CreateBlurImageFilter(SHADOW_FILTER, SHADOW_FILTER, RSTileMode::DECAL, nullptr));
    gradientPaint.SetFilter(filter);
    RSRecordingPath path;
#endif
    RSRect rRect(center.GetX() - radius + thickness * PERCENT_HALF, center.GetY() - radius + thickness * PERCENT_HALF,
        center.GetX() + radius - thickness * PERCENT_HALF, center.GetY() + radius - thickness * PERCENT_HALF);
    path.AddArc(rRect, START_ANGLE, drawAngle);

    RSBrush startCirclePaint;
    startCirclePaint.SetAntiAlias(true);
    startCirclePaint.SetColor(SortGradientColorsOffset(shadowColors_[0]->Get().GetGradient())
                                  .GetColors()
                                  .begin()
                                  ->GetLinearColor()
                                  .GetValue());
    startCirclePaint.SetAlpha(SHADOW_ALPHA);
    startCirclePaint.SetFilter(filter);

    RSBrush endCirclePaint;
    endCirclePaint.SetAntiAlias(true);
    endCirclePaint.SetColor(arcData.shadowColor.GetColors().rbegin()->GetLinearColor().GetValue());
    endCirclePaint.SetAlpha(SHADOW_ALPHA);
    endCirclePaint.SetFilter(filter);

#ifndef USE_ROSEN_DRAWING
    gradientPaint.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::DECAL, 0, drawAngle, nullptr));
#else
    gradientPaint.SetShaderEffect(RSRecordingShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::DECAL, 0, drawAngle, nullptr));
#endif
    RSRect edgeRect(center.GetX() - thickness * PERCENT_HALF, center.GetY() - radius,
        center.GetX() + thickness * PERCENT_HALF, center.GetY() - radius + thickness);

    canvas.Save();
    canvas.AttachBrush(startCirclePaint);
    canvas.DrawArc(edgeRect, QUARTER_CIRCLE - FIXED_ANGLE, HALF_CIRCLE + FIXED_DRAW_ANGLE);
    canvas.DetachBrush();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(-QUARTER_CIRCLE, center.GetX(), center.GetY());
    canvas.AttachPen(gradientPaint);
    canvas.DrawPath(path);
    canvas.DetachPen();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(drawAngle, center.GetX(), center.GetY());
    canvas.AttachBrush(endCirclePaint);
    canvas.DrawArc(edgeRect, -QUARTER_CIRCLE - FIXED_ANGLE, HALF_CIRCLE + FIXED_DRAW_ANGLE);
    canvas.DetachBrush();
    canvas.Restore();
}

void DataPanelModifier::PaintCircle(DrawingContext& context, OffsetF offset) const
{
    RSCanvas& canvas = context.canvas;
    canvas.Save();
    canvas.Translate(offset.GetX(), offset.GetY());

    auto defaultThickness = strokeWidth_->Get();
    ArcData arcData;
    arcData.center = Offset(context.width * PERCENT_HALF, context.height * PERCENT_HALF);

    // Here radius will minus defaultThickness, when there will be new api to set padding, use the new padding.
    arcData.radius = std::min(context.width, context.height) * PERCENT_HALF - defaultThickness;
    if (defaultThickness >= arcData.radius) {
        arcData.thickness = arcData.radius * DIAMETER_TO_THICKNESS_RATIO;
    } else {
        arcData.thickness = defaultThickness;
    }
    PaintTrackBackground(canvas, arcData, trackBackgroundColor_->Get().ToColor());
    arcData.maxValue = max_->Get();
    for (size_t i = 0; i < valuesLastLength_; ++i) {
        arcData.totalValue += values_[i]->Get();
    }
    if (NonPositive(arcData.totalValue)) {
        // all values are invalid
        return;
    }

    if ((isShadowVisible_ && (isHasShadowValue_ || isEffect_->Get()))) {
        PaintRainbowFilterMask(canvas, arcData);
    }

    for (int32_t i = valuesLastLength_ - 1; i >= 0; --i) {
        if (NearZero(values_[i]->Get())) {
            continue;
        }
        arcData.progressColors = SortGradientColorsOffset(valueColors_[i]->Get().GetGradient());
        auto totalValuePre = arcData.totalValue;
        arcData.progressValue = arcData.totalValue * date_->Get();
        arcData.drawAngle = arcData.progressValue / arcData.maxValue * WHOLE_CIRCLE;
        arcData.drawAngle = std::min(arcData.drawAngle, WHOLE_CIRCLE);
        arcData.totalValue -= values_[i]->Get();
        arcData.gradientPointBase = arcData.totalValue / totalValuePre;
        PaintProgress(canvas, arcData);
    }
    canvas.Restore();
}

void DataPanelModifier::PaintLinearProgress(DrawingContext& context, OffsetF offset) const
{
    auto& canvas = context.canvas;
    auto totalWidth = context.width;
    auto spaceWidth = SystemProperties::Vp2Px(FIXED_WIDTH);
    auto segmentWidthSum = 0.0f;
    auto segmentSize = 0.0;
    for (size_t i = 0; i < valuesLastLength_; ++i) {
        if (NearZero(values_[i]->Get())) {
            continue;
        }
        segmentWidthSum += values_[i]->Get();
        ++segmentSize;
    }

    float scaleMaxValue = 0.0f;
    scaleMaxValue = (totalWidth - segmentSize * spaceWidth) / max_->Get();

    auto widthSegment = offset.GetX();
    auto firstSegmentWidth = values_[0]->Get() * scaleMaxValue;
    PaintBackground(canvas, offset, totalWidth, context.height, firstSegmentWidth);
    float totalPaintWidth = 0.0f;
    float preWidthSegment = 0.0f;
    std::vector<LinearData> linearDataMap;
    bool isFirstValidDate = true;
    for (size_t i = 0; i < valuesLastLength_; ++i) {
        auto segmentWidth = values_[i]->Get();
        if (NonPositive(segmentWidth)) {
            continue;
        }
        LinearData segmentLinearData;
        segmentLinearData.offset = offset;
        segmentLinearData.height = context.height;

        if (isFirstValidDate) {
            segmentLinearData.isFirstData = true;
            isFirstValidDate = false;
        }

        segmentLinearData.segmentColor = SortGradientColorsOffset(valueColors_[i]->Get().GetGradient());
        segmentLinearData.segmentWidth = segmentWidth * scaleMaxValue;
        segmentLinearData.xSegment = widthSegment;
        preWidthSegment = widthSegment;
        if (GreatOrEqual(segmentLinearData.segmentWidth + segmentLinearData.xSegment, totalWidth)) {
            segmentLinearData.segmentWidth = totalWidth - preWidthSegment;
        }
        // mark last data or add space width
        widthSegment += values_[i]->Get() * scaleMaxValue;
        totalPaintWidth += segmentWidth;
        if (GreatOrEqual(totalPaintWidth, max_->Get())) {
            segmentLinearData.isEndData = true;
        } else {
            widthSegment += spaceWidth;
        }
        // draw the shadow at the bottom
        if ((isShadowVisible_ && (isHasShadowValue_ || isEffect_->Get())) && (i < shadowColorsLastLength_)) {
            segmentLinearData.segmentShadowColor = SortGradientColorsOffset(shadowColors_[i]->Get().GetGradient());
            PaintColorSegmentFilterMask(canvas, segmentLinearData);
        }

        linearDataMap.emplace_back(segmentLinearData);
    }
    // draw the data and the space after drawing the shadow
    for (size_t i = 0; i < linearDataMap.size(); ++i) {
        PaintColorSegment(canvas, linearDataMap[i]);
        if (!linearDataMap[i].isEndData) {
            PaintSpace(canvas, linearDataMap[i], spaceWidth);
        }
    }
}

void DataPanelModifier::PaintBackground(
    RSCanvas& canvas, OffsetF offset, float totalWidth, float height, float segmentWidth) const
{
    RSBrush brush;
    brush.SetColor(ToRSColor(trackBackgroundColor_->Get()));
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    RSRect rRect(offset.GetX(), offset.GetY(), totalWidth + offset.GetX(), height + offset.GetY());
    RSRoundRect rrRect;
    if (height <= segmentWidth) {
        rrRect = RSRoundRect(rRect, height, height);
    } else {
        rrRect = RSRoundRect(rRect, segmentWidth, segmentWidth);
    }
    canvas.DrawRoundRect(rrRect);
    canvas.DetachBrush();
}

void DataPanelModifier::PaintColorSegment(RSCanvas& canvas, const LinearData& segmentLinearData) const
{
    auto offset = segmentLinearData.offset;
    auto xSegment = segmentLinearData.xSegment;
    auto segmentWidth = segmentLinearData.segmentWidth;
    auto height = segmentLinearData.height;

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    size_t length = segmentLinearData.segmentColor.GetColors().size();
    for (size_t i = 0; i < length; ++i) {
        colors.emplace_back(segmentLinearData.segmentColor.GetColors().at(i).GetLinearColor().GetValue());
        pos.emplace_back(segmentLinearData.segmentColor.GetColors().at(i).GetDimension().Value());
    }

    RSRect rect(xSegment, offset.GetY(), xSegment + segmentWidth, offset.GetY() + height);
    RSRoundRect paintRect = RSRoundRect(rect, 0, 0);

    if (segmentLinearData.isFirstData) {
        paintRect.SetCornerRadius(RSRoundRect::TOP_LEFT_POS, height, height);
        paintRect.SetCornerRadius(RSRoundRect::BOTTOM_LEFT_POS, height, height);
    }

    if (segmentLinearData.isEndData) {
        paintRect.SetCornerRadius(RSRoundRect::TOP_RIGHT_POS, height, height);
        paintRect.SetCornerRadius(RSRoundRect::BOTTOM_RIGHT_POS, height, height);
    }

    RSPoint segmentStartPoint;
    segmentStartPoint.SetX(rect.GetLeft());
    segmentStartPoint.SetY(rect.GetTop());
    RSPoint segmentEndPoint;
    segmentEndPoint.SetX(rect.GetRight());
    segmentEndPoint.SetY(rect.GetBottom());
    canvas.Save();
    RSBrush brush;
#ifndef USE_ROSEN_DRAWING
    brush.SetShaderEffect(
        RSShaderEffect::CreateLinearGradient(segmentStartPoint, segmentEndPoint, colors, pos, RSTileMode::CLAMP));
#else
    brush.SetShaderEffect(RSRecordingShaderEffect::CreateLinearGradient(
        segmentStartPoint, segmentEndPoint, colors, pos, RSTileMode::CLAMP));
#endif
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(paintRect);
    canvas.DetachBrush();
    canvas.Restore();
}

void DataPanelModifier::PaintColorSegmentFilterMask(RSCanvas& canvas, const LinearData& segmentLinearData) const
{
    auto offset = segmentLinearData.offset;
    auto xSegment = segmentLinearData.xSegment;
    auto segmentWidth = segmentLinearData.segmentWidth;
    auto height = segmentLinearData.height;

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    size_t length = segmentLinearData.segmentShadowColor.GetColors().size();
    for (size_t i = 0; i < length; ++i) {
        colors.emplace_back(segmentLinearData.segmentShadowColor.GetColors().at(i).GetLinearColor().GetValue());
        pos.emplace_back(segmentLinearData.segmentShadowColor.GetColors().at(i).GetDimension().Value());
    }

    RSRect rect(xSegment + shadowOffsetXFloat_->Get(), offset.GetY() + shadowOffsetYFloat_->Get(),
        xSegment + segmentWidth + shadowOffsetXFloat_->Get(), offset.GetY() + height + shadowOffsetYFloat_->Get());
    RSRoundRect paintRect = RSRoundRect(rect, 0, 0);
    if (segmentLinearData.isFirstData) {
        paintRect.SetCornerRadius(RSRoundRect::TOP_LEFT_POS, height, height);
        paintRect.SetCornerRadius(RSRoundRect::BOTTOM_LEFT_POS, height, height);
    }

    if (segmentLinearData.isEndData) {
        paintRect.SetCornerRadius(RSRoundRect::TOP_RIGHT_POS, height, height);
        paintRect.SetCornerRadius(RSRoundRect::BOTTOM_RIGHT_POS, height, height);
    }

    RSPoint segmentStartPoint;
    segmentStartPoint.SetX(rect.GetLeft());
    segmentStartPoint.SetY(rect.GetTop());
    RSPoint segmentEndPoint;
    segmentEndPoint.SetX(rect.GetRight());
    segmentEndPoint.SetY(rect.GetBottom());
    canvas.Save();
    RSBrush brush;
    RSFilter filter;
#ifndef USE_ROSEN_DRAWING
    filter.SetMaskFilter(RSMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL, shadowRadiusFloat_->Get()));
#else
    filter.SetMaskFilter(RSRecordingMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL, shadowRadiusFloat_->Get()));
#endif
    brush.SetFilter(filter);
    brush.SetAlpha(SHADOW_ALPHA);
#ifndef USE_ROSEN_DRAWING
    brush.SetShaderEffect(
        RSShaderEffect::CreateLinearGradient(segmentStartPoint, segmentEndPoint, colors, pos, RSTileMode::CLAMP));
#else
    brush.SetShaderEffect(RSRecordingShaderEffect::CreateLinearGradient(
        segmentStartPoint, segmentEndPoint, colors, pos, RSTileMode::CLAMP));
#endif
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(paintRect);
    canvas.DetachBrush();
    canvas.Restore();
}

void DataPanelModifier::PaintSpace(RSCanvas& canvas, const LinearData& segmentLinearData, float spaceWidth) const
{
    float xSpace = segmentLinearData.xSegment + segmentLinearData.segmentWidth;
    auto offset = segmentLinearData.offset;
    RSBrush brush;
    RSRect rect(xSpace, offset.GetY(), xSpace + spaceWidth, offset.GetY() + segmentLinearData.height);
    brush.SetColor(ToRSColor(Color::WHITE));
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    canvas.DrawRect(rect);
    canvas.DetachBrush();
}

void DataPanelModifier::PaintTrackBackground(RSCanvas& canvas, ArcData arcData, const Color color) const
{
    RSPen backgroundTrackData;
#ifndef USE_ROSEN_DRAWING
    RSPath backgroundTrackPath;
#else
    RSRecordingPath backgroundTrackPath;
#endif
    auto center = arcData.center;
    float thickness = arcData.thickness;
    float radius = arcData.radius;

    RSRect rect(center.GetX() - radius + thickness * PERCENT_HALF, center.GetY() - radius + thickness * PERCENT_HALF,
        center.GetX() + radius - thickness * PERCENT_HALF, center.GetY() + radius - thickness * PERCENT_HALF);

    backgroundTrackPath.AddArc(rect, 0.0, WHOLE_CIRCLE);
    backgroundTrackData.SetColor(ToRSColor(color));
    backgroundTrackData.SetAntiAlias(true);
    backgroundTrackData.SetWidth(thickness);

    canvas.AttachPen(backgroundTrackData);
    canvas.DrawPath(backgroundTrackPath);
    canvas.DetachPen();
}

void DataPanelModifier::PaintProgress(RSCanvas& canvas, ArcData arcData) const
{
    float thickness = arcData.thickness;
    float radius = arcData.radius;
    float drawAngle = arcData.drawAngle;

    Offset center = arcData.center;

    std::vector<RSColorQuad> colors;
    std::vector<float> pos;
    size_t length = arcData.progressColors.GetColors().size();
    for (size_t i = 0; i < length; ++i) {
        colors.emplace_back(arcData.progressColors.GetColors().at(i).GetLinearColor().GetValue());
        if (NearZero(arcData.gradientPointBase)) {
            pos.emplace_back(arcData.progressColors.GetColors().at(i).GetDimension().Value());
        } else {
            auto itemPos =
                (1.0f - arcData.gradientPointBase) * arcData.progressColors.GetColors().at(i).GetDimension().Value() +
                arcData.gradientPointBase;
            pos.emplace_back(itemPos);
        }
    }

    RSPen gradientPaint;
    gradientPaint.SetWidth(thickness);
    gradientPaint.SetAntiAlias(true);
#ifndef USE_ROSEN_DRAWING
    RSPath path;
#else
    RSRecordingPath path;
#endif
    RSRect rRect(center.GetX() - radius + thickness * PERCENT_HALF, center.GetY() - radius + thickness * PERCENT_HALF,
        center.GetX() + radius - thickness * PERCENT_HALF, center.GetY() + radius - thickness * PERCENT_HALF);
    path.AddArc(rRect, START_ANGLE, drawAngle);

    RSBrush startCirclePaint;
    startCirclePaint.SetAntiAlias(true);
    startCirclePaint.SetColor(arcData.progressColors.GetColors().begin()->GetLinearColor().GetValue());

    RSBrush endCirclePaint;
    endCirclePaint.SetAntiAlias(true);
    endCirclePaint.SetColor(arcData.progressColors.GetColors().rbegin()->GetLinearColor().GetValue());

#ifndef USE_ROSEN_DRAWING
    gradientPaint.SetShaderEffect(RSShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::CLAMP, 0, drawAngle, nullptr));
#else
    gradientPaint.SetShaderEffect(RSRecordingShaderEffect::CreateSweepGradient(
        ToRSPoint(PointF(center.GetX(), center.GetY())), colors, pos, RSTileMode::CLAMP, 0, drawAngle, nullptr));
#endif

    canvas.Save();
    canvas.AttachBrush(startCirclePaint);
    RSRect edgeRect(center.GetX() - thickness * PERCENT_HALF, center.GetY() - radius,
        center.GetX() + thickness * PERCENT_HALF, center.GetY() - radius + thickness);
    canvas.DrawArc(edgeRect, QUARTER_CIRCLE - FIXED_ANGLE, HALF_CIRCLE + FIXED_DRAW_ANGLE);
    canvas.DetachBrush();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(-QUARTER_CIRCLE, center.GetX(), center.GetY());
    canvas.AttachPen(gradientPaint);
    canvas.DrawPath(path);
    canvas.DetachPen();
    canvas.Restore();

    canvas.Save();
    canvas.Rotate(drawAngle, center.GetX(), center.GetY());
    canvas.AttachBrush(endCirclePaint);
    canvas.DrawArc(edgeRect, -QUARTER_CIRCLE - FIXED_ANGLE, HALF_CIRCLE + FIXED_DRAW_ANGLE);
    canvas.DetachBrush();
    canvas.Restore();
}

Gradient DataPanelModifier::SortGradientColorsOffset(const Gradient& srcGradient) const
{
    auto srcGradientColors = srcGradient.GetColors();
    std::sort(
        srcGradientColors.begin(), srcGradientColors.end(), [](const GradientColor& left, const GradientColor& right) {
            return left.GetDimension().Value() < right.GetDimension().Value();
        });

    Gradient gradient;
    for (const auto& item : srcGradientColors) {
        gradient.AddColor(item);
    }

    return gradient;
}
} // namespace OHOS::Ace::NG
