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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_MANAGER_H

#include <bitset>
#include <memory>
#include <mutex>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/want/want_wrap.h"
#include "core/common/container.h"

namespace OHOS::Rosen {
enum class WSError;
}

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t UI_EXTENSION_UNKNOW_ID = 0;
constexpr int32_t UI_EXTENSION_ID_FIRST_MAX = 210;
constexpr int32_t UI_EXTENSION_ID_OTHER_MAX = 9;
constexpr int32_t UI_EXTENSION_OFFSET_MAX = 10000000;
constexpr int32_t UI_EXTENSION_OFFSET_MIN = 100000;
constexpr int32_t UI_EXTENSION_ID_FACTOR = 10;
constexpr int32_t UI_EXTENSION_LEVEL_MAX = 3;
};

class UIExtensionPattern;
class UIExtensionManager : public AceType {
    DECLARE_ACE_TYPE(UIExtensionManager, AceType);
public:
    UIExtensionManager() = default;
    ~UIExtensionManager() override = default;

    void RegisterUIExtensionInFocus(const WeakPtr<UIExtensionPattern>& uiExtensionFocused);
    bool OnBackPressed();
    const RefPtr<FrameNode> GetFocusUiExtensionNode();
    bool IsWrapExtensionAbilityId(int32_t elementId);
    bool IsWindowTypeUIExtension(const RefPtr<PipelineBase>& pipeline);
    bool SendAccessibilityEventInfo(const Accessibility::AccessibilityEventInfo& eventInfo,
        std::vector<int32_t>& uiExtensionIdLevelList, const RefPtr<PipelineBase>& pipeline);
    std::pair<int32_t, int32_t> UnWrapExtensionAbilityId(int32_t extensionOffset, int32_t elementId);
    int32_t ApplyExtensionId();
    void RecycleExtensionId(int32_t id);

private:

    class UIExtensionIdUtility {
    public:
        UIExtensionIdUtility() = default;
        ~UIExtensionIdUtility() = default;
        UIExtensionIdUtility(const UIExtensionIdUtility&) = delete;
        UIExtensionIdUtility &operator=(const UIExtensionIdUtility&) = delete;

        int32_t ApplyExtensionId();
        void RecycleExtensionId(int32_t id);

    private:
        static std::bitset<UI_EXTENSION_ID_FIRST_MAX> idPool_;
        static std::mutex poolMutex_;
    };

    WeakPtr<UIExtensionPattern> uiExtensionFocused_;
    std::unique_ptr<UIExtensionIdUtility> extensionIdUtility_ = std::make_unique<UIExtensionIdUtility>();
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_MANAGER_H
