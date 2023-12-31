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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PICKER_ROSEN_RENDER_PICKER_BASE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PICKER_ROSEN_RENDER_PICKER_BASE_H

#include "core/components/picker/render_picker_base.h"
#include "core/pipeline/base/rosen_render_context.h"

namespace OHOS::Ace {

class RosenRenderPickerBase : public RenderPickerBase {
    DECLARE_ACE_TYPE(RosenRenderPickerBase, RenderPickerBase);

public:
    RosenRenderPickerBase() = default;
    ~RosenRenderPickerBase() override = default;

    void Paint(RenderContext& context, const Offset& offset) override;

private:
#ifndef USE_ROSEN_DRAWING
    void PaintGradient(SkCanvas* canvas, const Offset& offset, const Rect& rect, const RefPtr<PickerTheme>& theme);
#else
    void PaintGradient(RSCanvas* canvas, const Offset& offset, const Rect& rect, const RefPtr<PickerTheme>& theme);
#endif
    Rect GetOptionsRect(const Offset& offset, const RefPtr<RenderPickerColumn>& pickerColumn);
#ifndef USE_ROSEN_DRAWING
    void PaintFocusOptionBorder(SkCanvas* canvas);
#else
    void PaintFocusOptionBorder(RSCanvas* canvas);
#endif
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PICKER_ROSEN_RENDER_PICKER_BASE_H
