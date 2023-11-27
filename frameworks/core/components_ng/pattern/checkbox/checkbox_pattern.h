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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_PATTERN_H

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/checkbox/checkbox_accessibility_property.h"
#include "core/components_ng/pattern/checkbox/checkbox_event_hub.h"
#include "core/components_ng/pattern/checkbox/checkbox_layout_algorithm.h"
#include "core/components_ng/pattern/checkbox/checkbox_paint_method.h"
#include "core/components_ng/pattern/checkbox/checkbox_paint_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

class CheckBoxPattern : public Pattern {
    DECLARE_ACE_TYPE(CheckBoxPattern, Pattern);

public:
    CheckBoxPattern() = default;
    ~CheckBoxPattern() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<CheckBoxPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<CheckBoxLayoutAlgorithm>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto paintProperty = host->GetPaintProperty<CheckBoxPaintProperty>();
        paintProperty->SetHost(host);
        auto isSelect = paintProperty->GetCheckBoxSelectValue(false);
        if (!checkboxModifier_) {
            auto pipeline = PipelineBase::GetCurrentContext();
            auto checkBoxTheme = pipeline->GetTheme<CheckboxTheme>();
            auto boardColor = isSelect ? paintProperty->GetCheckBoxSelectedColorValue(checkBoxTheme->GetActiveColor())
                                       : checkBoxTheme->GetInactivePointColor();
            auto checkColor = isSelect ? checkBoxTheme->GetPointColor() : Color::TRANSPARENT;
            auto borderColor = isSelect ? Color::TRANSPARENT : checkBoxTheme->GetInactiveColor();
            auto shadowColor = isSelect ? checkBoxTheme->GetShadowColor() : Color::TRANSPARENT;
            checkboxModifier_ =
                AceType::MakeRefPtr<CheckBoxModifier>(isSelect, boardColor, checkColor, borderColor, shadowColor);
        }
        CheckBoxStyle checkboxStyle = CheckBoxStyle::CIRCULAR_STYLE;
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
            checkboxStyle = CheckBoxStyle::CIRCULAR_STYLE;
        } else {
            checkboxStyle = CheckBoxStyle::SQUARE_STYLE;
        }
        if (paintProperty->HasCheckBoxSelectedStyle()) {
            checkboxStyle = paintProperty->GetCheckBoxSelectedStyleValue(CheckBoxStyle::CIRCULAR_STYLE);
        }
        checkboxModifier_->SetCheckboxStyle(checkboxStyle);
        host->SetCheckboxFlag(true);
        auto paintMethod = MakeRefPtr<CheckBoxPaintMethod>(checkboxModifier_);
        auto eventHub = host->GetEventHub<EventHub>();
        CHECK_NULL_RETURN(eventHub, nullptr);
        auto enabled = eventHub->IsEnabled();
        paintMethod->SetEnabled(enabled);
        paintMethod->SetTouchHoverAnimationType(touchHoverType_);
        return paintMethod;
    }

    bool OnDirtyLayoutWrapperSwap(
        const RefPtr<LayoutWrapper>& dirty, bool /*skipMeasure*/, bool /*skipLayout*/) override
    {
        auto geometryNode = dirty->GetGeometryNode();
        offset_ = geometryNode->GetContentOffset();
        size_ = geometryNode->GetContentSize();
        if (!isUserSetResponseRegion_) {
            AddHotZoneRect();
        }
        return true;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<CheckBoxEventHub>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<CheckBoxAccessibilityProperty>();
    }

    const std::optional<std::string>& GetPreName()
    {
        return preName_;
    }

    const std::optional<std::string>& GetPreGroup()
    {
        return preGroup_;
    }

    int32_t GetPrePageId() const
    {
        return prePageId_;
    }

    void SetPreName(const std::string& name)
    {
        preName_ = name;
    }

    void SetPreGroup(const std::string& group)
    {
        preGroup_ = group;
    }

    void SetPrePageId(int32_t pageId)
    {
        prePageId_ = pageId;
    }

    void MarkIsSelected(bool isSelected);
    void SetLastSelect(bool select)
    {
        lastSelect_ = select;
    }

    void SetIsUserSetResponseRegion(bool isUserSetResponseRegion)
    {
        isUserSetResponseRegion_ = isUserSetResponseRegion;
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        Pattern::ToJsonValue(json);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto checkBoxEventHub = host->GetEventHub<NG::CheckBoxEventHub>();
        auto name = checkBoxEventHub ? checkBoxEventHub->GetName() : "";
        auto group = checkBoxEventHub ? checkBoxEventHub->GetGroupName() : "";
        json->Put("name", name.c_str());
        json->Put("group", group.c_str());
        json->Put("type", "ToggleType.Checkbox");
        auto paintProperty = host->GetPaintProperty<CheckBoxPaintProperty>();
        auto select = paintProperty->GetCheckBoxSelectValue(false);
        json->Put("select", select ? "true" : "false");
    }

    FocusPattern GetFocusPattern() const override;
    void UpdateUIStatus(bool check);

    std::string ProvideRestoreInfo() override;
    void OnRestoreInfo(const std::string& restoreInfo) override;
    void OnColorConfigurationUpdate() override;

private:
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnModifyDone() override;
    void InitClickEvent();
    void InitTouchEvent();
    void InitMouseEvent();
    void OnClick();
    void OnTouchDown();
    void OnTouchUp();
    void HandleMouseEvent(bool isHover);
    void CheckPageNode();
    void UpdateState();
    void UpdateUnSelect();
    void UpdateCheckBoxGroupStatus(const RefPtr<FrameNode>& frameNode,
        std::unordered_map<std::string, std::list<WeakPtr<FrameNode>>>& checkBoxGroupMap, bool isSelected);
    void UpdateCheckBoxGroupStatusWhenDetach(const FrameNode* frameNode,
        std::unordered_map<std::string, std::list<WeakPtr<FrameNode>>>& checkBoxGroupMap);
    void CheckBoxGroupIsTrue();
    // Init key event
    void InitOnKeyEvent(const RefPtr<FocusHub>& focusHub);
    void GetInnerFocusPaintRect(RoundRect& paintRect);
    void AddHotZoneRect();
    void RemoveLastHotZoneRect() const;
    void SetAccessibilityAction();
    void UpdateSelectStatus(bool isSelected);

    std::optional<std::string> preName_;
    std::optional<std::string> preGroup_;
    int32_t prePageId_ = 0;
    bool lastSelect_ = false;

    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> mouseEvent_;
    bool isTouch_ = false;
    bool isHover_ = false;
    bool isFirstCreated_ = true;
    bool isUserSetResponseRegion_ = false;
    UIStatus uiStatus_ = UIStatus::UNSELECTED;
    Dimension hotZoneHorizontalPadding_;
    Dimension hotZoneVerticalPadding_;
    OffsetF offset_;
    SizeF size_;
    OffsetF hotZoneOffset_;
    SizeF hotZoneSize_;
    TouchHoverAnimationType touchHoverType_ = TouchHoverAnimationType::NONE;

    RefPtr<CheckBoxModifier> checkboxModifier_;

    ACE_DISALLOW_COPY_AND_MOVE(CheckBoxPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_PATTERN_H
