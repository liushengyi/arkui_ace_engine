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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SYSTEM_WINDOW_SCENE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SYSTEM_WINDOW_SCENE_H

#include "common/rs_vector4.h"
#include "session/host/include/session.h"

#include "core/common/container.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"

namespace OHOS::Ace::NG {
class SystemWindowScene : public StackPattern {
    DECLARE_ACE_TYPE(SystemWindowScene, StackPattern);

public:
    explicit SystemWindowScene(const sptr<Rosen::Session>& session);
    ~SystemWindowScene() override = default;

    std::optional<RenderContext::ContextParam> GetContextParam() const override
    {
        return RenderContext::ContextParam { RenderContext::ContextType::EXTERNAL };
    }

private:
    void OnAttachToFrameNode() override;
    void OnBoundsChanged(const Rosen::Vector4f& bounds);
    void RegisterFocusCallback();

    int32_t instanceId_ = Container::CurrentId();
    sptr<Rosen::Session> session_;
    std::function<void(const Rosen::Vector4f&)> boundsChangedCallback_;

    ACE_DISALLOW_COPY_AND_MOVE(SystemWindowScene);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SYSTEM_WINDOW_SCENE_H
