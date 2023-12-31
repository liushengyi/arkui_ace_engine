/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SHAPE_ROSEN_RENDER_SHAPE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SHAPE_ROSEN_RENDER_SHAPE_H

#ifndef USE_ROSEN_DRAWING
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#endif

#include "core/components/shape/render_shape.h"
#ifdef USE_ROSEN_DRAWING
#include "core/components_ng/render/drawing.h"
#endif

namespace OHOS::Ace {

class RosenRenderShape : public RenderShape {
    DECLARE_ACE_TYPE(RosenRenderShape, RenderShape);

public:
    void Paint(RenderContext& context, const Offset& offset) override;
    Size CalcSize() override;
#ifndef USE_ROSEN_DRAWING
    void PaintOnCanvas(SkCanvas* skCanvas, const Offset& offset);
#else
    void PaintOnCanvas(RSCanvas* canvas, const Offset& offset);
#endif

private:
    Size CreateRect();
    Size CreateCircle();
    Size CreateEllipse();
    Size CreatePolygon(bool needClose);
    Size CreatePath();
#ifndef USE_ROSEN_DRAWING
    void DrawStroke(SkCanvas* skCanvas, const SkPath& path);
#else
    void DrawStroke(RSCanvas* canvas, const RSPath& path);
#endif
    float GetFloatRadiusValue(const Dimension& src, const Dimension& dest, bool isVertical);

#ifndef USE_ROSEN_DRAWING
    SkPath path_;
#else
    RSRecordingPath path_;
#endif
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SHAPE_ROSEN_RENDER_SHAPE_H
