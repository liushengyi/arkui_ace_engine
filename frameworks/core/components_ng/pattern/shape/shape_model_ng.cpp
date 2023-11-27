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

#include "core/components_ng/pattern/shape/shape_model_ng.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/shape/shape_abstract_model_ng.h"
#include "core/components_ng/pattern/shape/shape_container_paint_property.h"
#include "core/components_ng/pattern/shape/shape_container_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void ShapeModelNG::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_SCOPED_TRACE("Create[%s][self:%d]", V2::SHAPE_CONTAINER_ETS_TAG, nodeId);
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::SHAPE_CONTAINER_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<ShapeContainerPattern>(); });
    stack->Push(frameNode);
}

void ShapeModelNG::SetBitmapMesh(std::vector<double>& mesh, int32_t column, int32_t row)
{
    ACE_UPDATE_PAINT_PROPERTY(ShapeContainerPaintProperty, ImageMesh, ImageMesh(mesh, (int32_t)column, (int32_t)row));
}

void ShapeModelNG::SetViewPort(
    const Dimension& dimLeft, const Dimension& dimTop, const Dimension& dimWidth, const Dimension& dimHeight)
{
    ShapeViewBox shapeViewBox;
    shapeViewBox.SetLeft(dimLeft);
    shapeViewBox.SetTop(dimTop);
    shapeViewBox.SetWidth(dimWidth);
    shapeViewBox.SetHeight(dimHeight);
    ACE_UPDATE_PAINT_PROPERTY(ShapeContainerPaintProperty, ShapeViewBox, shapeViewBox);
}

void ShapeModelNG::SetStroke(const Color& color)
{
    ShapeAbstractModelNG().SetStroke(color);
}

void ShapeModelNG::SetFill(const Color& color)
{
    ShapeAbstractModelNG().SetFill(color);
}

void ShapeModelNG::SetStrokeDashOffset(const Ace::Dimension& dashOffset)
{
    ShapeAbstractModelNG().SetStrokeDashOffset(dashOffset);
}

void ShapeModelNG::SetStrokeLineCap(int lineCapStyle)
{
    ShapeAbstractModelNG().SetStrokeLineCap(lineCapStyle);
}

void ShapeModelNG::SetStrokeLineJoin(int lineJoinStyle)
{
    ShapeAbstractModelNG().SetStrokeLineJoin(lineJoinStyle);
}

void ShapeModelNG::SetStrokeMiterLimit(double miterLimit)
{
    ShapeAbstractModelNG().SetStrokeMiterLimit(miterLimit);
}

void ShapeModelNG::SetStrokeOpacity(double opacity)
{
    ShapeAbstractModelNG().SetStrokeOpacity(opacity);
}

void ShapeModelNG::SetFillOpacity(double opacity)
{
    ShapeAbstractModelNG().SetFillOpacity(opacity);
}

void ShapeModelNG::SetStrokeWidth(const Ace::Dimension& lineWidth)
{
    ShapeAbstractModelNG().SetStrokeWidth(lineWidth);
}

void ShapeModelNG::SetStrokeDashArray(const std::vector<Ace::Dimension>& segments)
{
    ShapeAbstractModelNG().SetStrokeDashArray(segments);
}

void ShapeModelNG::SetAntiAlias(bool antiAlias)
{
    ShapeAbstractModelNG().SetAntiAlias(antiAlias);
}

} // namespace OHOS::Ace::NG
