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

#include "core/components_ng/pattern/custom_paint/rendering_context2d_modifier.h"
#include "core/components_ng/render/drawing.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "pipeline/rs_recording_canvas.h"
#include "pipeline/rs_paint_filter_canvas.h"
#endif

namespace OHOS::Ace::NG {
RenderingContext2DModifier::RenderingContext2DModifier()
{
    needRender_ = AceType::MakeRefPtr<PropertyBool>(true);
    AttachProperty(needRender_);
}

void RenderingContext2DModifier::onDraw(DrawingContext& drawingContext)
{
    ACE_SCOPED_TRACE("RenderingContext2DModifier::onDraw");
#ifndef USE_ROSEN_DRAWING
    auto skCanvas = drawingContext.canvas.GetImpl<Rosen::Drawing::SkiaCanvas>()->ExportSkCanvas();

    auto recordingCanvas = static_cast<OHOS::Rosen::RSRecordingCanvas*>(skCanvas);
    if (!recordingCanvas || !rsRecordingCanvas_) {
        return;
    }

    auto drawCmdList = rsRecordingCanvas_->GetDrawCmdList();
    if (!drawCmdList) {
        return;
    }
    if (drawCmdList->GetSize() == 0) {
        return;
    }
    recordingCanvas->GetDrawCmdList()->SetWidth(drawCmdList->GetWidth());
    recordingCanvas->GetDrawCmdList()->SetHeight(drawCmdList->GetHeight());
    auto canvas = std::make_shared<OHOS::Rosen::RSPaintFilterCanvas>(recordingCanvas);
    CHECK_NULL_VOID(canvas);
    canvas->SetRecordingState(true);
    drawCmdList->Playback(*canvas);
    rsRecordingCanvas_->Clear();
#else
    if (!rsRecordingCanvas_) {
        return;
    }

    auto& recordingCanvas = drawingContext.canvas;
    auto drawCmdList = rsRecordingCanvas_->GetDrawCmdList();
    if (!drawCmdList) {
        return;
    }
    if (drawCmdList->IsEmpty()) {
        return;
    }
    if (recordingCanvas.GetDrawingType() == Rosen::Drawing::Drawing::Type::RECORDING) {
        static_cast<RSRecordingCanvas&>(recordingCanvas).GetDrawCmdList()->SetWidth(drawCmdList->GetWidth());
        static_cast<RSRecordingCanvas&>(recordingCanvas).GetDrawCmdList()->SetHeight(drawCmdList->GetHeight());
    }
    drawCmdList->Playback(recordingCanvas);
    rsRecordingCanvas_->Clear();
#endif
}
} // namespace OHOS::Ace::NG