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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_ROSEN_RENDER_SVG_POLYGON_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_ROSEN_RENDER_SVG_POLYGON_H

#ifndef USE_ROSEN_DRAWING
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#else
#include "core/components_ng/render/drawing.h"
#endif

#include "frameworks/core/components/svg/render_svg_polygon.h"

namespace OHOS::Ace {

class RosenRenderSvgPolygon : public RenderSvgPolygon {
    DECLARE_ACE_TYPE(RosenRenderSvgPolygon, RenderSvgPolygon);

public:
    void Paint(RenderContext& context, const Offset& offset) override;
    void PaintDirectly(RenderContext& context, const Offset& offset) override;

    bool HasEffectiveTransform() const override
    {
        return NeedTransform();
    }
    void UpdateMotion(const std::string& path, const std::string& rotate, double percent) override;

private:
    Rect GetPaintBounds(const Offset& offset) override;
#ifndef USE_ROSEN_DRAWING
    bool GetPath(SkPath* out);
    bool GetPathWithoutAnimate(SkPath* out);
    bool CreateSkPath(const std::string& pointsStr, std::vector<SkPoint>& skPoints);
    bool CreateSkPaths(const std::string& points1, const std::string& points2, double weight, SkPath* out);
#else
    bool GetPath(RSPath* out);
    bool GetPathWithoutAnimate(RSPath* out);
    bool CreateRSPath(const std::string& pointsStr, std::vector<RSPoint>& rsPoints);
    bool CreateRSPaths(const std::string& points1, const std::string& points2, double weight, RSPath* out);
#endif
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_ROSEN_RENDER_SVG_POLYGON_H
