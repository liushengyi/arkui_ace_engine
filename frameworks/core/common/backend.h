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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_BACKEND_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_BACKEND_H

#include "base/memory/ace_type.h"
#include "base/resource/asset_manager.h"
#include "core/common/js_message_dispatcher.h"

namespace OHOS::Ace {

enum class BackendType { SERVICE, DATA, FORM };

enum class SrcLanguage { ETS, JS };

class ACE_EXPORT Backend : public AceType {
    DECLARE_ACE_TYPE(Backend, AceType);

public:
    Backend() = default;
    ~Backend() override = default;

    enum class State { ON_CREATE, ON_DESTROY };

    static RefPtr<Backend> Create();

    virtual bool Initialize(BackendType type, SrcLanguage language) = 0;

    virtual void LoadEngine(const char* libName, int32_t instanceId) = 0;

    virtual void SetAssetManager(const RefPtr<AssetManager> &assetManager) = 0;

    // inform the frontend that onCreate or onDestroy
    virtual void UpdateState(State) = 0;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_BACKEND_H