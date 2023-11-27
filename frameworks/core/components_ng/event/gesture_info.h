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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_GESTURE_INFO_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_GESTURE_INFO_H

#include <optional>

#include "base/memory/ace_type.h"
#include "core/components/common/layout/constants.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT GestureInfo : public virtual AceType {
    DECLARE_ACE_TYPE(GestureInfo, AceType);

public:
    GestureInfo() = default;
    GestureInfo(std::string tag, GestureTypeName type, bool isSystemGesture)
        : tag_(std::move(tag)), type_(type), isSystemGesture_(isSystemGesture)
    {}
    GestureInfo(GestureTypeName type, bool isSystemGesture) : type_(type), isSystemGesture_(isSystemGesture) {}
    explicit GestureInfo(std::string tag) : tag_(std::move(tag)) {}
    explicit GestureInfo(GestureTypeName type) : type_(type) {}
    explicit GestureInfo(bool isSystemGesture) : isSystemGesture_(isSystemGesture) {}
    ~GestureInfo() override = default;

    std::optional<std::string> GetTag() const
    {
        return tag_;
    }

    GestureTypeName GetType() const
    {
        return type_;
    }

    bool IsSystemGesture() const
    {
        return isSystemGesture_;
    }

    void SetTag(std::string tag)
    {
        tag_ = std::move(tag);
    }

    void SetType(GestureTypeName type)
    {
        type_ = type;
    }

    void SetIsSystemGesture(bool isSystemGesture)
    {
        isSystemGesture_ = isSystemGesture;
    }

private:
    std::optional<std::string> tag_;
    GestureTypeName type_;
    bool isSystemGesture_ = false;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_GESTURE_INFO_H
