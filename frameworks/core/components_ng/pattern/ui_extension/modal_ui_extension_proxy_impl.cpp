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

#include "core/components_ng/pattern/ui_extension/modal_ui_extension_proxy_impl.h"

#include "session/host/include/extension_session.h"

namespace OHOS::Ace::NG {
ModalUIExtensionProxyImpl::ModalUIExtensionProxyImpl(const sptr<Rosen::Session>& session) : session_(session) {}

void ModalUIExtensionProxyImpl::SendData(const AAFwk::WantParams& params)
{
    auto session = session_.promote();
    if (session) {
        sptr<Rosen::ExtensionSession> extensionSession(static_cast<Rosen::ExtensionSession*>(session.GetRefPtr()));
        extensionSession->TransferComponentData(params);
    }
}
} // namespace OHOS::Ace::NG
