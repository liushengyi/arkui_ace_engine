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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_PRESENTATION_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_PRESENTATION_PATTERN_H

#include <functional>
#include <utility>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_algorithm.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/overlay/popup_base_pattern.h"
#include "core/components_ng/pattern/overlay/sheet_presentation_property.h"
#include "core/components_ng/pattern/overlay/sheet_style.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT SheetPresentationPattern : public LinearLayoutPattern, public PopupBasePattern {
    DECLARE_ACE_TYPE(SheetPresentationPattern, LinearLayoutPattern, PopupBasePattern);

public:
    SheetPresentationPattern(int32_t targetId, std::function<void(const std::string&)>&& callback)
        : LinearLayoutPattern(true)
    {
        targetId_ = targetId;
        callback_ = std::move(callback);
    }

    ~SheetPresentationPattern() override = default;

    bool IsMeasureBoundary() const override
    {
        return true;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<LinearLayoutAlgorithm>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<SheetPresentationProperty>();
    }

    int32_t GetTargetId() const override
    {
        return targetId_;
    }

    void FireCallback(const std::string& value)
    {
        if (callback_) {
            callback_(value);
        }
    }

    void UpdateOnDisappear(std::function<void()>&& onDisappear)
    {
        onDisappear_ = std::move(onDisappear);
        isExecuteOnDisappear_ = false;
    }

    void OnDisappear()
    {
        if (onDisappear_) {
            isExecuteOnDisappear_ = true;
            onDisappear_();
        }
    }

    void InitialLayoutProps();

    void AvoidSafeArea();
    float GetSheetHeightChange();
    void ScrollTo(float height);

    // initial drag gesture event
    void InitPanEvent();

    void HandleDragUpdate(const GestureEvent& info);

    void HandleDragEnd(float dragVelocity);

    void SheetTransition(bool isTransitionIn);

    void SetCurrentOffset(float currentOffset)
    {
        currentOffset_ = currentOffset;
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

    void OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) override;
private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void InitPageHeight();
    void TranslateTo(float height);
    void SetColumnMinSize(bool reset = false);

    uint32_t keyboardHeight_ = 0;
    int32_t targetId_ = -1;
    std::function<void(const std::string&)> callback_;
    std::function<void()> onDisappear_;
    RefPtr<PanEvent> panEvent_;
    float currentOffset_ = 0.0f;
    float height_ = 0.0f; // sheet height, start from the bottom
    float sheetHeight_ = 0.0f;
    float heightBoundary_ = 0.0f;
    float pageHeight_ = 0.0f;
    float scrollHeight_ = 0.0f;
    float statusBarHeight_ = .0f;
    bool isExecuteOnDisappear_ = false;
    bool windowRotate_ = false;
    bool firstMeasure_ = true;
    bool isScrolling_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(SheetPresentationPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_PRESENTATION_PATTERN_H
