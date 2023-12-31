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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_OVERLAY_ROSEN_RENDER_TEXT_OVERLAY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_OVERLAY_ROSEN_RENDER_TEXT_OVERLAY_H

#ifndef USE_ROSEN_DRAWING
#include "include/core/SkCanvas.h"
#endif

#include "core/components/text_overlay/render_text_overlay.h"
#include "core/pipeline/base/rosen_render_context.h"

namespace OHOS::Ace {

class RosenRenderTextOverlay final : public RenderTextOverlay {
    DECLARE_ACE_TYPE(RosenRenderTextOverlay, RenderTextOverlay)

public:
    RosenRenderTextOverlay() = default;
    ~RosenRenderTextOverlay() override = default;

    void Paint(RenderContext& context, const Offset& offset) override;

private:
    void PaintHandles(RenderContext& context) const;
#ifndef USE_ROSEN_DRAWING
    void PaintHandle(SkCanvas* skCanvas, Offset centerOffset, bool isLeftHandle = false) const;
#else
    void PaintHandle(RSCanvas* canvas, Offset centerOffset, bool isLeftHandle = false) const;
#endif
    void PaintMagnifier(RenderContext& context);
#ifndef USE_ROSEN_DRAWING
    void PaintClipLine(SkCanvas* skCanvas, SkPaint paint);
#else
    void PaintClipLine(RSCanvas* canvas, RSPen pen);
#endif
    void PaintMore(RenderContext& context);
    void InitMoreButtonPaintColor();
    Offset CalculateMoreButtonCenter();
    bool CheckNeedPaintMore();
    bool NeedAdjustStartHandle();
    bool NeedAdjustEndHandle();
    void AdjustForAnimation();

    Color clipColor_ = Color::WHITE;
    Color arrowColor_ = Color::BLACK;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_OVERLAY_ROSEN_RENDER_TEXT_OVERLAY_H
