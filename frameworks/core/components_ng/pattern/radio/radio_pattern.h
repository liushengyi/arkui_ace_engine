/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_PATTERN_H

#include "base/memory/referenced.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/radio/radio_accessibility_property.h"
#include "core/components_ng/pattern/radio/radio_event_hub.h"
#include "core/components_ng/pattern/radio/radio_layout_algorithm.h"
#include "core/components_ng/pattern/radio/radio_paint_method.h"
#include "core/components_ng/pattern/radio/radio_paint_property.h"

namespace OHOS::Ace::NG {

class RadioPattern : public Pattern {
    DECLARE_ACE_TYPE(RadioPattern, Pattern);

public:
    RadioPattern() = default;
    ~RadioPattern() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<RadioPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<RadioLayoutAlgorithm>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        if (!radioModifier_) {
            radioModifier_ = AceType::MakeRefPtr<RadioModifier>();
        }
        auto paintMethod = MakeRefPtr<RadioPaintMethod>(radioModifier_);
        paintMethod->SetTotalScale(totalScale_);
        paintMethod->SetPointScale(pointScale_);
        paintMethod->SetRingPointScale(ringPointScale_);
        paintMethod->SetUIStatus(uiStatus_);
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto eventHub = host->GetEventHub<EventHub>();
        CHECK_NULL_RETURN(eventHub, nullptr);
        auto enabled = eventHub->IsEnabled();
        paintMethod->SetEnabled(enabled);
        paintMethod->SetIsOnAnimationFlag(isOnAnimationFlag_);
        paintMethod->SetTouchHoverAnimationType(touchHoverType_);
        paintMethod->SetIsFirstCreated(isFirstCreated_);
        paintMethod->SetShowHoverEffect(showHoverEffect_);
        isFirstCreated_ = false;
        return paintMethod;
    }

    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& /*config*/) override;

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<RadioEventHub>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<RadioAccessibilityProperty>();
    }

    const std::optional<std::string>& GetPreValue()
    {
        return preValue_;
    }

    const std::optional<std::string>& GetPreGroup()
    {
        return preGroup_;
    }

    int32_t GetPrePageId() const
    {
        return prePageId_;
    }

    void SetPreValue(const std::string& value)
    {
        preValue_ = value;
    }

    void SetPreGroup(const std::string& group)
    {
        preGroup_ = group;
    }

    void SetPrePageId(int32_t pageId)
    {
        prePageId_ = pageId;
    }

    void SetIsUserSetResponseRegion(bool isUserSetResponseRegion)
    {
        isUserSetResponseRegion_ = isUserSetResponseRegion;
    }

    void SetShowHoverEffect(bool showHoverEffect)
    {
        showHoverEffect_ = showHoverEffect;
    }

    FocusPattern GetFocusPattern() const override;

    void UpdateUncheckStatus(const RefPtr<FrameNode>& frameNode);

    void MarkIsSelected(bool isSelected);

    void SetSelected(bool isSelected)
    {
        preCheck_ = isSelected;
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        Pattern::ToJsonValue(json);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto radioEventHub = host->GetEventHub<NG::RadioEventHub>();
        auto value = radioEventHub ? radioEventHub->GetValue() : "";
        auto group = radioEventHub ? radioEventHub->GetGroup() : "";
        json->Put("value", value.c_str());
        json->Put("group", group.c_str());
    }
    std::string ProvideRestoreInfo() override;
    void OnRestoreInfo(const std::string& restoreInfo) override;

private:
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnModifyDone() override;
    void InitClickEvent();
    void InitTouchEvent();
    void InitMouseEvent();
    void OnClick();
    void UpdateState();
    void UpdateGroupCheckStatus(const RefPtr<FrameNode>& frameNode, const RefPtr<FrameNode>& pageNode, bool check);
    void OnTouchDown();
    void OnTouchUp();
    void CheckPageNode();
    void HandleMouseEvent(bool isHover);
    void UpdateUIStatus(bool check);
    // Init key event
    void InitOnKeyEvent(const RefPtr<FocusHub>& focusHub);
    void GetInnerFocusPaintRect(RoundRect& paintRect);
    void AddHotZoneRect();
    void RemoveLastHotZoneRect() const;
    void SetAccessibilityAction();
    void UpdateSelectStatus(bool isSelected);

    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> mouseEvent_;

    bool isFirstCreated_ = true;
    bool preCheck_ = false;
    std::optional<std::string> preValue_;
    std::optional<std::string> preGroup_;
    int32_t prePageId_ = 0;
    bool isTouch_ = false;
    bool isHover_ = false;
    float totalScale_ = 1.0f;
    float pointScale_ = 0.5f;
    float ringPointScale_ = 0.0f;
    UIStatus uiStatus_ = UIStatus::UNSELECTED;
    Dimension hotZoneHorizontalPadding_;
    Dimension hotZoneVerticalPadding_;
    OffsetF offset_;
    SizeF size_;
    OffsetF hotZoneOffset_;
    SizeF hotZoneSize_;
    bool isGroupChanged_ = false;
    TouchHoverAnimationType touchHoverType_ = TouchHoverAnimationType::NONE;
    bool isOnAnimationFlag_ = false;
    bool isUserSetResponseRegion_ = false;
    bool showHoverEffect_ = true;

    RefPtr<RadioModifier> radioModifier_;
    ACE_DISALLOW_COPY_AND_MOVE(RadioPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_PATTERN_H
