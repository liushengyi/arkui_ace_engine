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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_PROXY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_PROXY_H

#include <refbase.h>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/want/want_wrap.h"
#include "ui_extension_pattern.h"

namespace OHOS::Rosen {
class Session;
enum class WSErrorCode : int32_t;
} // namespace OHOS::Rosen

namespace OHOS::Ace::NG {
class UIExtensionProxy : public AceType {
    DECLARE_ACE_TYPE(UIExtensionProxy, AceType);

public:
    UIExtensionProxy(const sptr<Rosen::Session>& session, const RefPtr<UIExtensionPattern>& pattern);
    void SendData(const RefPtr<WantParamsWrap>& params);
    OHOS::Rosen::WSErrorCode SendDataSync(const RefPtr<WantParamsWrap>& wantParams,
                                          AAFwk::WantParams& reWantParams);
    RefPtr<UIExtensionPattern> GetPattern() const;
private:
    wptr<Rosen::Session> session_;
    WeakPtr<UIExtensionPattern> pattern_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_PROXY_H
