/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/core/components/svg/flutter_render_svg_pattern.h"

#include "include/core/SkPicture.h"

#include "frameworks/core/components/common/painter/flutter_svg_painter.h"
#include "frameworks/core/components/svg/svg_transform.h"
#include "frameworks/core/components/transform/flutter_render_transform.h"
#include "frameworks/core/pipeline/base/flutter_render_context.h"

namespace OHOS::Ace {

using namespace Flutter;

RenderLayer FlutterRenderSvgPattern::GetRenderLayer()
{
    if (!transformLayer_) {
        transformLayer_ = AceType::MakeRefPtr<Flutter::TransformLayer>(Matrix4::CreateIdentity(), 0.0, 0.0);
    }
    return AceType::RawPtr(transformLayer_);
}

void FlutterRenderSvgPattern::Paint(RenderContext& context, const Offset& offset)
{
    return;
}

bool FlutterRenderSvgPattern::OnAsPaint(const Offset& offset, const Rect& paintRect, SkPaint& skPaint)
{
    Rect tileRect;
    SkMatrix skMatrix4;
    if (!FitAttribute(paintRect, tileRect, skMatrix4)) {
        LOGW("fit attribute fail.");
        return false;
    }

    FlutterRenderContext flutterContext;
    FitRenderContext(flutterContext, tileRect);
    PaintDirectly(flutterContext, offset);

    auto skPicture = flutterContext.FinishRecordingAsPicture();
    if (!skPicture) {
        return false;
    }
    SkRect skRect = SkRect::MakeXYWH(tileRect.Left(), tileRect.Top(), tileRect.Width(), tileRect.Height());
    skPaint.setShader(skPicture->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &skMatrix4, &skRect));
    return true;
}

bool FlutterRenderSvgPattern::FitAttribute(const Rect& paintRect, Rect& tileRect, SkMatrix& skMatrix4)
{
    if (LessOrEqual(width_.Value(), 0.0) || LessOrEqual(height_.Value(), 0.0)) {
        return false;
    }
    tileRect = Rect(ParseUnitsAttr(x_, paintRect.Width()), ParseUnitsAttr(y_, paintRect.Height()),
        ParseUnitsAttr(width_, paintRect.Width()), ParseUnitsAttr(height_, paintRect.Height()));

    if (NearZero(viewBox_.Width()) || NearZero(viewBox_.Height())) {
        ResetAttrOffset();
        return true;
    }
    scaleX_ = tileRect.Width() / viewBox_.Width();
    scaleY_ = tileRect.Height() / viewBox_.Height();
    scale_ = std::min(scaleX_, scaleY_);
    tx_ = tileRect.Width() * 0.5 - (viewBox_.Width() * 0.5 + viewBox_.Left()) * scale_;
    ty_ = tileRect.Height() * 0.5 - (viewBox_.Height() * 0.5 + viewBox_.Top()) * scale_;

    skMatrix4 = FlutterSvgPainter::ToSkMatrix(GetTransform(tileRect));
    return true;
}

const Matrix4 FlutterRenderSvgPattern::GetTransform(const Rect& patternRect) const
{
    auto transformInfo = (!animateTransformAttrs_.empty()) ? SvgTransform::CreateInfoFromMap(animateTransformAttrs_)
                                                           : SvgTransform::CreateInfoFromString(transform_);
    if (!NearZero(patternRect.Left()) || !NearZero(patternRect.Top())) {
        transformInfo.matrix4 =
            Matrix4::CreateTranslate(patternRect.Left(), patternRect.Top(), 0) * transformInfo.matrix4;
    }

    if (transformInfo.hasRotateCenter) {
        transformInfo.matrix4 =
            FlutterRenderTransform::GetTransformByOffset(transformInfo.matrix4, transformInfo.rotateCenter);
    }

    transformInfo.matrix4 = FlutterRenderTransform::GetTransformByOffset(transformInfo.matrix4, GetGlobalOffset());
    return transformInfo.matrix4;
}

void FlutterRenderSvgPattern::FitRenderContext(FlutterRenderContext& context, const Rect& patternRect)
{
    Rect rect(patternRect.Left(), patternRect.Top(), patternRect.Width() / scaleX_, patternRect.Height() / scaleY_);
    context.InitContext(GetRenderLayer(), rect);

    flutter::Canvas* canvas = context.GetCanvas();
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    SkCanvas* skCanvas = canvas->canvas();
    if (!skCanvas) {
        LOGE("Paint skCanvas is null");
        return;
    }
    skCanvas->scale(SkDoubleToScalar(scale_), SkDoubleToScalar(scale_));
}

void FlutterRenderSvgPattern::ResetAttrOffset()
{
    scaleY_ = 1.0f;
    scaleX_ = 1.0f;
    scale_ = 1.0f;
    tx_ = 0.0f;
    ty_ = 0.0f;
}

} // namespace OHOS::Ace
