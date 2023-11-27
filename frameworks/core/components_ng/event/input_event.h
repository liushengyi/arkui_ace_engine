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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_ON_MOUSE_EVENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_ON_MOUSE_EVENT_H

#include "core/event/mouse_event.h"

namespace OHOS::Ace::NG {

class InputEventHub;
class FrameNode;

class InputEvent : public virtual AceType {
    DECLARE_ACE_TYPE(InputEvent, AceType)
public:
    explicit InputEvent(OnMouseEventFunc&& callback) : onMouseCallback_(std::move(callback)) {}

    explicit InputEvent(OnHoverEventFunc&& callback) : onHoverCallback_(std::move(callback)) {}

    explicit InputEvent(OnHoverFunc&& callback) : onHoverEventCallback_(std::move(callback)) {}
    
    explicit InputEvent(OnAxisEventFunc&& callback) : onAxisCallback_(std::move(callback)) {}

    ~InputEvent() override = default;

    const OnMouseEventFunc& GetOnMouseEventFunc() const
    {
        return onMouseCallback_;
    }

    const OnHoverEventFunc& GetOnHoverEventFunc() const
    {
        return onHoverCallback_;
    }

    const OnHoverFunc& GetOnHoverFunc() const
    {
        return onHoverEventCallback_;
    }

    const OnAxisEventFunc& GetOnAxisEventFunc() const
    {
        return onAxisCallback_;
    }

    void operator()(MouseInfo& info) const
    {
        if (onMouseCallback_) {
            onMouseCallback_(info);
        }
    }

    void operator()(bool state, HoverInfo& info) const
    {
        if (onHoverEventCallback_) {
            onHoverEventCallback_(state, info);
        }
    }
    void operator()(bool state) const
    {
        if (onHoverCallback_) {
            onHoverCallback_(state);
        }
    }

    void operator()(AxisInfo& info) const
    {
        if (onAxisCallback_) {
            onAxisCallback_(info);
        }
    }

private:
    OnMouseEventFunc onMouseCallback_;
    OnHoverEventFunc onHoverCallback_;
    OnHoverFunc onHoverEventCallback_;
    OnAxisEventFunc onAxisCallback_;
};

class ACE_EXPORT InputEventActuator : public virtual AceType {
    DECLARE_ACE_TYPE(InputEventActuator, AceType)
public:
    explicit InputEventActuator(const WeakPtr<InputEventHub>& inputEventHub);
    ~InputEventActuator() override = default;

    void ClearUserCallback()
    {
        if (userCallback_) {
            userCallback_.Reset();
        }
    }

    void ReplaceInputEvent(OnMouseEventFunc&& callback)
    {
        if (userCallback_) {
            userCallback_.Reset();
        }
        userCallback_ = MakeRefPtr<InputEvent>(std::move(callback));
    }
    void ReplaceInputEvent(OnHoverFunc&& callback)
    {
        if (userCallback_) {
            userCallback_.Reset();
        }
        userCallback_ = MakeRefPtr<InputEvent>(std::move(callback));
    }

    void ReplaceInputEvent(OnAxisEventFunc&& callback)
    {
        if (userCallback_) {
            userCallback_.Reset();
        }
        userCallback_ = MakeRefPtr<InputEvent>(std::move(callback));
    }

    void AddInputEvent(const RefPtr<InputEvent>& inputEvent)
    {
        if (inputEvents_.empty()) {
            inputEvents_.emplace_back(inputEvent);
            return;
        }
        if (std::find(inputEvents_.begin(), inputEvents_.end(), inputEvent) == inputEvents_.end()) {
            inputEvents_.emplace_back(inputEvent);
        }
    }

    void RemoveInputEvent(const RefPtr<InputEvent>& inputEvent)
    {
        inputEvents_.remove(inputEvent);
    }

    void OnCollectMouseEvent(const OffsetF& coordinateOffset, const GetEventTargetImpl& getEventTargetImpl,
        TouchTestResult& result);

    void OnCollectHoverEvent(
        const OffsetF& coordinateOffset, const GetEventTargetImpl& getEventTargetImpl, TouchTestResult& result);

    void OnCollectHoverEffect(const OffsetF& coordinateOffset, const GetEventTargetImpl& getEventTargetImpl,
        TouchTestResult& result);

    void OnCollectAxisEvent(
        const OffsetF& coordinateOffset, const GetEventTargetImpl& getEventTargetImpl, AxisTestResult& onAxisResult);

private:
    WeakPtr<InputEventHub> inputEventHub_;
    RefPtr<MouseEventTarget> mouseEventTarget_;
    RefPtr<HoverEventTarget> hoverEventTarget_;
    RefPtr<HoverEffectTarget> hoverEffectTarget_;
    RefPtr<AxisEventTarget> axisEventTarget_;
    std::list<RefPtr<InputEvent>> inputEvents_;
    RefPtr<InputEvent> userCallback_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_INPUT_EVENT_H
