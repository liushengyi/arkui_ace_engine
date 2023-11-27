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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_RECOGNIZERS_RECOGNIZER_GROUP_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_RECOGNIZERS_RECOGNIZER_GROUP_H

#include <list>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/gestures/gesture_info.h"
#include "core/components_ng/gestures/recognizers/multi_fingers_recognizer.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT RecognizerGroup : public MultiFingersRecognizer {
    DECLARE_ACE_TYPE(RecognizerGroup, MultiFingersRecognizer);

public:
    explicit RecognizerGroup(const std::vector<RefPtr<NGGestureRecognizer>>& recognizers)
    {
        for (const auto& recognizer : recognizers) {
            if (recognizer && recognizer->SetGestureGroup(AceType::WeakClaim(this))) {
                recognizers_.emplace_back(recognizer);
            }
        }
    }

    explicit RecognizerGroup(std::list<RefPtr<NGGestureRecognizer>>&& recognizers)
    {
        recognizers_.clear();
        for (const auto& recognizer : recognizers) {
            if (recognizer && recognizer->SetGestureGroup(AceType::WeakClaim(this))) {
                recognizers_.emplace_back(recognizer);
            }
        }
    }

    ~RecognizerGroup() override = default;

    void AddChildren(const std::list<RefPtr<NGGestureRecognizer>>& recognizers);

    void OnFlushTouchEventsBegin() override;
    void OnFlushTouchEventsEnd() override;
    RefereeState CheckStates(size_t touchId);
    void ForceReject();

    void RemainChildOnResetStatus()
    {
        remainChildOnResetStatus_ = true;
    }

    void AssignNodeId(int id) override
    {
        if (nodeId_ != -1) {
            return;
        }

        TouchEventTarget::AssignNodeId(id);
        auto recognizers = GetGroupRecognizer();
        for (const auto& recognizer : recognizers) {
            recognizer->AssignNodeId(id);
        }
    }

    void AttachFrameNode(const WeakPtr<NG::FrameNode>& node) override
    {
        TouchEventTarget::AttachFrameNode(node);
        auto recognizers = GetGroupRecognizer();
        for (const auto& recognizer : recognizers) {
            recognizer->AttachFrameNode(node);
        }
    }

    const std::list<RefPtr<NGGestureRecognizer>>& GetGroupRecognizer();

    Axis GetAxisDirection() override
    {
        uint8_t horizontalFlag = 0;
        uint8_t verticalFlag = 0;
        uint8_t freeFlag = 0;
        for (const auto& recognizer : recognizers_) {
            if (!recognizer) {
                continue;
            }
            auto direction = recognizer->GetAxisDirection();
            switch (direction) {
                case Axis::HORIZONTAL:
                    horizontalFlag = 0x1;
                    break;
                case Axis::VERTICAL:
                    verticalFlag = 0x2;
                    break;
                case Axis::FREE:
                    freeFlag = 0x3;
                    break;
                default:
                    break;
            }
        }
        uint8_t directionFlag = horizontalFlag | verticalFlag | freeFlag;
        switch (directionFlag) {
            case 0x1:
                return Axis::HORIZONTAL;
            case 0x2:
                return Axis::VERTICAL;
            case 0x3:
                return Axis::FREE;
            default:
                return Axis::NONE;
        }
    }

    void SetChildrenTargetComponent(const RefPtr<TargetComponent>& targetComponent)
    {
        for (const auto& child : recognizers_) {
            if (child) {
                child->SetTargetComponent(targetComponent);
            }
        }
    }

protected:
    void OnBeginGestureReferee(int32_t touchId, bool needUpdateChild = false) override;
    void OnFinishGestureReferee(int32_t touchId, bool isBlocked = false) override;
    void GroupAdjudicate(const RefPtr<NGGestureRecognizer>& recognizer, GestureDisposal disposal);

    bool Existed(const RefPtr<NGGestureRecognizer>& recognizer);
    bool CheckAllFailed();

    void OnResetStatus() override;

    std::list<RefPtr<NGGestureRecognizer>> recognizers_;
    bool remainChildOnResetStatus_ = false;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_GESTURES_RECOGNIZERS_RECOGNIZER_GROUP_H
