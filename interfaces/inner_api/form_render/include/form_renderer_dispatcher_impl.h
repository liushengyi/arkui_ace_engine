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

#ifndef FOUNDATION_ACE_INTERFACE_INNERKITS_FORM_RENDERER_DISPATCHER_IMPL_H
#define FOUNDATION_ACE_INTERFACE_INNERKITS_FORM_RENDERER_DISPATCHER_IMPL_H

#include "form_renderer_dispatcher_stub.h"

#include "base/utils/macros.h"
#include "ui_content.h"
#include "event_handler.h"

namespace OHOS {
namespace AppExecFwk {
class EventHandler;
}
namespace Ace {
class FormRenderer;
/**
 * @class FormRendererDispatcher
 * FormRendererDispatcher interface is used to form renderer dispatcher.
 */
class ACE_EXPORT FormRendererDispatcherImpl : public FormRendererDispatcherStub {
public:
    FormRendererDispatcherImpl(const std::shared_ptr<UIContent> uiContent,
        const std::shared_ptr<FormRenderer> formRenderer,
        std::weak_ptr<OHOS::AppExecFwk::EventHandler> eventHandler);
    ~FormRendererDispatcherImpl() override = default;
    /**
     * @brief Dispatcher pointer event.
     * @param pointerEvent The pointer event info.
     */
    void DispatchPointerEvent(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent) override;
    /**
     * @brief Set AllowUpdate.
     * @param allowUpdate The allowUpdate.
     */
    void SetAllowUpdate(bool allowUpdate) override;
    bool IsAllowUpdate();

    void DispatchSurfaceChangeEvent(float width, float height) override;
    void SetVisibleChange(bool isVisible) override;
private:
    std::weak_ptr<UIContent> uiContent_;
    std::weak_ptr<FormRenderer> formRenderer_;
    std::weak_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_;
    bool allowUpdate_ = true;
};
} // namespace Ace
} // namespace OHOS
#endif  // FOUNDATION_ACE_INTERFACE_INNERKITS_FORM_RENDERER_DISPATCHER_IMPL_H
