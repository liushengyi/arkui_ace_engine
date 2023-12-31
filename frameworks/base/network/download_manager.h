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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_NETWORK_DOWNLOAD_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_NETWORK_DOWNLOAD_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS::Ace {

class DownloadManager {
public:
    static DownloadManager& GetInstance();

    virtual ~DownloadManager() = default;
    virtual bool Download(const std::string& url, std::vector<uint8_t>& dataOut) = 0;

    struct ProxyInfo {
        std::string host;
        int32_t port = 0;
        std::string exclusions;
    };
    static bool GetProxy(ProxyInfo& proxy);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_NETWORK_DOWNLOAD_MANAGER_H
