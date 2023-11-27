/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_GESTURE_INFO_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_GESTURE_INFO_H

#include <functional>
#include <string>

#include "base/memory/ace_type.h"
#include "base/utils/macros.h"
#include "core/components_ng/event/gesture_info.h"
#include "core/gestures/gesture_event.h"
#include "core/gestures/velocity.h"
#include "core/gestures/velocity_tracker.h"

namespace OHOS::Ace::NG {

class NGGestureRecognizer;

class ACE_EXPORT Gesture : public virtual AceType {
    DECLARE_ACE_TYPE(Gesture, AceType);

public:
    Gesture() = default;
    explicit Gesture(int32_t fingers) : fingers_(fingers) {}
    ~Gesture() override = default;

    void SetOnActionId(const GestureEventFunc& onActionId)
    {
        onActionId_ = std::make_unique<GestureEventFunc>(onActionId);
    }
    void SetOnActionStartId(const GestureEventFunc& onActionStartId)
    {
        onActionStartId_ = std::make_unique<GestureEventFunc>(onActionStartId);
    }
    void SetOnActionUpdateId(const GestureEventFunc& onActionUpdateId)
    {
        onActionUpdateId_ = std::make_unique<GestureEventFunc>(onActionUpdateId);
    }
    void SetOnActionEndId(const GestureEventFunc& onActionEndId)
    {
        onActionEndId_ = std::make_unique<GestureEventFunc>(onActionEndId);
    }
    void SetOnActionCancelId(const GestureEventNoParameter& onActionCancelId)
    {
        onActionCancelId_ = std::make_unique<GestureEventNoParameter>(onActionCancelId);
    }
    void SetPriority(GesturePriority priority)
    {
        priority_ = priority;
    }
    void SetGestureMask(GestureMask gestureMask)
    {
        gestureMask_ = gestureMask;
    }

    GesturePriority GetPriority() const
    {
        return priority_;
    }

    GestureMask GetGestureMask() const
    {
        return gestureMask_;
    }

    void SetTag(std::string tag)
    {
        if (gestureInfo_) {
            gestureInfo_->SetTag(std::move(tag));
        } else {
            gestureInfo_ = MakeRefPtr<GestureInfo>(tag);
        }
    }

    virtual RefPtr<NGGestureRecognizer> CreateRecognizer() = 0;

protected:
    int32_t fingers_ = 1;
    GesturePriority priority_ = GesturePriority::Low;
    GestureMask gestureMask_ = GestureMask::Normal;
    std::unique_ptr<GestureEventFunc> onActionId_;
    std::unique_ptr<GestureEventFunc> onActionStartId_;
    std::unique_ptr<GestureEventFunc> onActionUpdateId_;
    std::unique_ptr<GestureEventFunc> onActionEndId_;
    std::unique_ptr<GestureEventNoParameter> onActionCancelId_;
    RefPtr<GestureInfo> gestureInfo_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_GESTURE_INFO_H
