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

#include "web_configuration_observer.h"

namespace OHOS::Ace {
void WebConfigurationObserver::OnConfigurationUpdated(const OHOS::AppExecFwk::Configuration& configuration)
{
    std::string colorMode = configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    auto delegate = delegate_.Upgrade();
    CHECK_NULL_VOID(delegate);
    auto nweb = delegate->GetNweb();
    if (nweb) {
        if (colorMode == "dark") {
            nweb->GetPreference()->PutDarkSchemeEnabled(true);
            if (delegate->GetForceDarkMode()) {
                nweb->GetPreference()->PutForceDarkModeEnabled(true);
            }
            return;
        }
        if (colorMode == "light") {
            nweb->GetPreference()->PutDarkSchemeEnabled(false);
            nweb->GetPreference()->PutForceDarkModeEnabled(false);
        }
    }
}
} // namespace OHOS::ACE