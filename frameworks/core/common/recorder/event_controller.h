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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_RECORDER_EVENT_CONTROLLER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_RECORDER_EVENT_CONTROLLER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "interfaces/inner_api/ace/ui_event_observer.h"
#include "core/common/recorder/event_config.h"

namespace OHOS::Ace::Recorder {
struct UIEventClient {
    std::shared_ptr<UIEventObserver> observer;
    EventConfig config;
};

class EventController final {
public:
    ~EventController() = default;

    static EventController& Get();

    void Register(const std::string& config, const std::shared_ptr<UIEventObserver>& observer);
    void Unregister(const std::shared_ptr<UIEventObserver>& observer);

    void NotifyEvent(EventCategory category, int32_t eventType,
        const std::shared_ptr<std::unordered_map<std::string, std::string>>& eventParams);

    void NotifyEventSync(EventCategory category, int32_t eventType,
        const std::shared_ptr<std::unordered_map<std::string, std::string>>& eventParams);

private:
    EventController() = default;
    void NotifyConfigChange();

    std::mutex cacheLock_;
    std::vector<UIEventClient> clientList_;
};
} // namespace OHOS::Ace::Recorder
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_RECORDER_EVENT_CONTROLLER_H
