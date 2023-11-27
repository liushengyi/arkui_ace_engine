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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_MODAL_PRESENTATION_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_MODAL_PRESENTATION_PATTERN_H

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/pattern/overlay/modal_style.h"
#include "core/components_ng/pattern/overlay/popup_base_pattern.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT ModalPresentationPattern : public PopupBasePattern {
    DECLARE_ACE_TYPE(ModalPresentationPattern, PopupBasePattern);

public:
    ModalPresentationPattern(int32_t targetId, ModalTransition type, std::function<void(const std::string&)>&& callback)
        : targetId_(targetId), type_(type), callback_(std::move(callback))
    {}
    ~ModalPresentationPattern() override = default;

    bool IsMeasureBoundary() const override
    {
        return true;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    int32_t GetTargetId() const override
    {
        return targetId_;
    }

    ModalTransition GetType() const
    {
        return type_;
    }

    void SetType(ModalTransition type)
    {
        type_ = type;
    }

    void FireCallback(const std::string& value)
    {
        if (callback_) {
            callback_(value);
        }
    }

    void UpdateOnDisappear(std::function<void()>&& onDisappear) {
        onDisappear_ = std::move(onDisappear);
        isExecuteOnDisappear_ = false;
    }

    void OnDisappear() {
        if (onDisappear_) {
            isExecuteOnDisappear_ = true;
            onDisappear_();
        }
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    bool IsExecuteOnDisappear() const
    {
        return isExecuteOnDisappear_;
    }

    bool AvoidKeyboard() const override
    {
        return false;
    }
private:
    void OnAttachToFrameNode() override;
    int32_t targetId_ = -1;
    ModalTransition type_ = ModalTransition::DEFAULT;
    std::function<void(const std::string&)> callback_;
    std::function<void()> onDisappear_;
    bool isExecuteOnDisappear_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(ModalPresentationPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_MODAL_PRESENTATION_PATTERN_H
