/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PLUGIN_ROSEN_RENDER_PLUGIN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PLUGIN_ROSEN_RENDER_PLUGIN_H

#include "base/geometry/offset.h"
#include "core/components/plugin/render_plugin.h"

namespace OHOS::Ace {
class RosenRenderPlugin : public RenderPlugin {
    DECLARE_ACE_TYPE(RosenRenderPlugin, RenderPlugin);

public:
    RosenRenderPlugin() = default;
    ~RosenRenderPlugin() override = default;

    std::unique_ptr<DrawDelegate> GetDrawDelegate() override;

    void RemoveChildren() override;

    void NotifyPaintFinish() override;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PLUGIN_ROSEN_RENDER_PLUGIN_H
