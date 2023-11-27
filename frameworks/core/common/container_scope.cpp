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

#include "core/common/container_scope.h"
#include "core/common/container.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {
namespace {
// preview not support multi-instance, always using default instance id 0.
#if defined(PREVIEW)
thread_local int32_t currentId_ = 0;
#else
thread_local int32_t currentId_ = INSTANCE_ID_UNDEFINED;
#endif
}

int32_t ContainerScope::CurrentId()
{
    return currentId_;
}

void ContainerScope::UpdateCurrent(int32_t id)
{
    currentId_ = id;
}

} // namespace OHOS::Ace
