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

#include "adapter/preview/entrance/ace_view_preview.h"

#include "base/log/dump_log.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/components/theme/theme_manager.h"
#include "core/event/mouse_event.h"
#include "core/event/touch_event.h"
#include "core/image/image_cache.h"

namespace OHOS::Ace::Platform {
AceViewPreview* AceViewPreview::CreateView(int32_t instanceId, bool useCurrentEventRunner, bool usePlatformThread)
{
    if (SystemProperties::GetFlutterDecouplingEnabled()) {
        auto* aceView =
            new AceViewPreview(instanceId, ThreadModelImpl::CreateThreadModel(useCurrentEventRunner, !usePlatformThread,
                !SystemProperties::GetRosenBackendEnabled()));
        if (aceView != nullptr) {
            aceView->IncRefCount();
        }
        return aceView;
    } else {
        auto* aceView =
            new AceViewPreview(instanceId, FlutterThreadModel::CreateThreadModel(useCurrentEventRunner,
                !usePlatformThread, !SystemProperties::GetRosenBackendEnabled()));
        if (aceView != nullptr) {
            aceView->IncRefCount();
        }
        return aceView;
    }
}

AceViewPreview::AceViewPreview(int32_t instanceId, std::unique_ptr<ThreadModelImpl> threadModelImpl)
    : instanceId_(instanceId), threadModelImpl_(std::move(threadModelImpl))
{}

AceViewPreview::AceViewPreview(int32_t instanceId, std::unique_ptr<FlutterThreadModel> threadModel)
    : instanceId_(instanceId), threadModel_(std::move(threadModel))
{}

void AceViewPreview::NotifySurfaceChanged(int32_t width, int32_t height,
    WindowSizeChangeReason type, const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    width_ = width;
    height_ = height;
    CHECK_NULL_VOID(viewChangeCallback_);
    viewChangeCallback_(width, height, type, rsTransaction);
}

bool AceViewPreview::HandleMouseEvent(const MouseEvent& mouseEvent)
{
    int32_t eventID = mouseEvent.GetId();
    auto markProcess = [eventID]() { LOGD("Mark %{public}d id Mouse Event Processed", eventID); };
    mouseEventCallback_(mouseEvent, markProcess);
    return true;
}

bool AceViewPreview::HandleAxisEvent(const AxisEvent& axisEvent)
{
    int32_t eventID = axisEvent.id;
    auto markProcess = [eventID]() { LOGD("Mark %{public}d id Axis Event Processed", eventID); };
    axisEventCallback_(axisEvent, markProcess);
    return true;
}

bool AceViewPreview::HandleTouchEvent(const TouchEvent& touchEvent)
{
    LOGD("Event: [x, y, size] = [%{public}lf, %{public}lf, %{public}lf]", touchEvent.x, touchEvent.y, touchEvent.size);
    if (touchEvent.type == TouchType::UNKNOWN) {
        LOGW("Unknown event.");
        return false;
    }
    CHECK_NULL_RETURN(touchEventCallback_, true);
    auto event = touchEvent.UpdatePointers();
    touchEventCallback_(event, nullptr);
    return true;
}

std::unique_ptr<DrawDelegate> AceViewPreview::GetDrawDelegate()
{
    auto drawDelegate = std::make_unique<DrawDelegate>();

#ifdef ENABLE_ROSEN_BACKEND
    drawDelegate->SetDrawRSFrameCallback([](std::shared_ptr<Rosen::RSNode>& node, const Rect& rect) {});
#else
    drawDelegate->SetDrawFrameCallback([this](RefPtr<Flutter::Layer>& layer, const Rect& dirty) {
        if (!layer) {
            return;
        }
        RefPtr<Flutter::FlutterSceneBuilder> flutterSceneBuilder = AceType::MakeRefPtr<Flutter::FlutterSceneBuilder>();
        layer->AddToScene(*flutterSceneBuilder, 0.0, 0.0);
        auto scene_ = flutterSceneBuilder->Build();
        if (!flutter::UIDartState::Current()) {
            return;
        }
        auto window = flutter::UIDartState::Current()->window();
        if (window != nullptr && window->client() != nullptr) {
            window->client()->Render(scene_.get());
        }
    });
#endif

    return drawDelegate;
}
} // namespace OHOS::Ace::Platform
