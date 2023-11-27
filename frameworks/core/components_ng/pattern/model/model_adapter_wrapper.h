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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MODEL_MODEL_ADAPTER_WRAPPER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MODEL_MODEL_ADAPTER_WRAPPER_H

#include <memory>

#include "custom/shader_input_buffer.h"
#include "ohos/texture_layer.h"
#include "widget_adapter.h"

#include "base/geometry/animatable_float.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/geometry/quaternion.h"
#include "base/geometry/vec3.h"
#include "core/components_ng/pattern/model/model_paint_property.h"
#include "core/components_ng/pattern/model/model_touch_handler.h"

namespace OHOS::Ace::NG {

struct CameraProperty {
    Render3D::Position position_;
    Render3D::Vec3 lookAt_ { 0.0f, 0.0f, 0.0f };
    Render3D::Vec3 up_ { 0.0f, 1.0f, 0.0f };
    Render3D::Quaternion rotation_ { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    float near_ = 0.5f;
    float far_ = 50.0f;
    float fov_ = 60.0f;
};

class ModelAdapterWrapper : public virtual AceType {
    DECLARE_ACE_TYPE(ModelAdapterWrapper, AceType)
public:
    using PaintFinishCallback = std::function<void()>;

    ModelAdapterWrapper(uint32_t key, Render3D::SurfaceType surfaceType, const std::string& bundleName,
        const std::string& moduleName);
    ~ModelAdapterWrapper() override;

    void SetPaintFinishCallback(PaintFinishCallback callback);
    bool NeedsRepaint();
    bool HandleTouchEvent(const TouchEventInfo& info, const RefPtr<ModelPaintProperty>& modelPaintProperty);

    void OnPaint3D(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void OnPaintFinish();
    void OnRebuildFrame(RefPtr<RenderContext>& context);
    void OnAttachToFrameNode(const RefPtr<RenderContext>& context);
    void OnDirtyLayoutWrapperSwap(float offsetX, float offsetY, float width, float height, float scale,
        bool recreateWindow);
    void OnDirtyLayoutWrapperSwap(const Render3D::WindowChangeInfo& windowChangeInfo);
    void OnPaint3DSceneTexture(SkCanvas* skCanvas);

    OHOS::Render3D::SurfaceType GetSurfaceType()
    {
        return surfaceType_;
    }

private:
    void CreateTextureLayer();
    void CreateWidgetAdapter();

    uint32_t GetKey();
    void Initialize();
    void UnloadSceneAndBackground();
    void DrawFrame();
    void UpdateCamera(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateLights(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateGLTFAnimations(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateGeometries(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateScene(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateEnviroment(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateCustomRender(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateShaderPath(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateImageTexturePaths(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void UpdateShaderInputBuffers(const RefPtr<ModelPaintProperty>& modelPaintProperty);
    void HandleCameraMove(const Render3D::PointerEvent& event);
    Render3D::HapInfo SetHapInfo();

    uint32_t key_ = UINT32_MAX;
    PaintFinishCallback callback_ = nullptr;
    bool needsSyncPaint_ = true;
    bool needsRepaint_ = true;

    std::shared_ptr<Render3D::WidgetAdapter> widgetAdapter_;
    std::unique_ptr<Render3D::TextureLayer> textureLayer_;
    RefPtr<ModelTouchHandler> touchHandler_;
    Render3D::SurfaceType surfaceType_;

    std::string bundleName_;
    std::string moduleName_;
    ACE_DISALLOW_COPY_AND_MOVE(ModelAdapterWrapper);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MODEL_MODEL_ADAPTER_WRAPPER_H
