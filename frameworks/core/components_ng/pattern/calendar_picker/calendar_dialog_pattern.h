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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CALENDAR_PICKER_CALENDAR_DIALOG_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CALENDAR_PICKER_CALENDAR_DIALOG_PATTERN_H

#include <optional>

#include "core/components/calendar/calendar_data_adapter.h"
#include "core/components/picker/picker_theme.h"
#include "core/components_ng/pattern/calendar/calendar_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_type_define.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"

namespace OHOS::Ace::NG {
class CalendarDialogPattern : public LinearLayoutPattern {
    DECLARE_ACE_TYPE(CalendarDialogPattern, LinearLayoutPattern);

public:
    CalendarDialogPattern() : LinearLayoutPattern(true) {};
    ~CalendarDialogPattern() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    void SetEntryNode(const WeakPtr<FrameNode>& node)
    {
        entryNode_ = node;
    }

    const OffsetF GetDialogOffset() const
    {
        return dialogOffset_;
    }

    void SetDialogOffset(const OffsetF& offset)
    {
        dialogOffset_ = offset;
    }

    bool OnDirtyLayoutWrapperSwap(
        const RefPtr<LayoutWrapper>& dirty, bool /* skipMeasure */, bool /* skipLayout */) override
    {
        if (isFirstAddhotZoneRect_) {
            AddHotZoneRect();
            isFirstAddhotZoneRect_ = false;
        }
        return true;
    }

    void HandleClickEvent(const GestureEvent& info);

    FocusPattern GetFocusPattern() const override
    {
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, FocusPattern());
        auto pickerTheme = pipeline->GetTheme<PickerTheme>();
        RefPtr<CalendarTheme> calendarTheme = pipeline->GetTheme<CalendarTheme>();
        CHECK_NULL_RETURN(pickerTheme, FocusPattern());
        CHECK_NULL_RETURN(calendarTheme, FocusPattern());
        auto focusColor = pickerTheme->GetFocusColor();
        FocusPaintParam focusPaintParams;
        focusPaintParams.SetPaintColor(focusColor);
        auto focusPaintWidth = calendarTheme->GetCalendarDayKeyFocusedPenWidth();
        focusPaintParams.SetPaintWidth(focusPaintWidth);
        return { FocusType::NODE, true, FocusStyleType::CUSTOM_REGION, focusPaintParams };
    }

    void SetHoverState(bool state)
    {
        hoverState_ = state;
    }

    bool GetHoverState() const
    {
        return hoverState_;
    }

    void GetCalendarMonthData(int32_t year, int32_t month, ObtainedMonth& calendarMonthData);
    CalendarMonth GetNextMonth(const CalendarMonth& calendarMonth);
    CalendarMonth GetLastMonth(const CalendarMonth& calendarMonth);

private:
    void OnModifyDone() override;
    void InitClickEvent();
    void InitOnKeyEvent();
    void InitOnTouchEvent();
    void InitTitleArrowsEvent();
    void InitEntryChangeEvent();
    void InitHoverEvent();
    void AddHotZoneRect();
    bool HandleKeyEvent(const KeyEvent& event);
    void GetInnerFocusPaintRect(RoundRect& paintRect);
    void PaintFocusState();
    void HandleTitleArrowsClickEvent(int32_t nodeIndex);
    void HandleEntryChange(const std::string& info);
    void HandleEntryLayoutChange();
    void ClearCalendarFocusedState();
    bool HandleCalendarNodeKeyEvent(const KeyEvent& event);
    bool ActClick(int32_t focusAreaID, int32_t focusAreaChildID);
    void PaintCurrentMonthFocusState();
    void PaintNonCurrentMonthFocusState(int32_t focusedDayIndex);
    void ChangeEntryState();
    void FocusedLastFocusedDay();
    int32_t GetIndexByFocusedDay();
    bool HandleTabKeyEvent(const KeyEvent& event);
    void FireChangeByKeyEvent(PickerDate& selectedDay);
    bool IsIndexInCurrentMonth(int32_t focusedDayIndex, const ObtainedMonth& currentMonthData);
    bool IsInEntryRegion(const Offset& globalLocation);
    void HandleEntryNodeHoverEvent(bool state, const Offset& globalLocation);
    void HandleEntryNodeTouchEvent(bool isPressed, const Offset& globalLocation);
    void UpdateDialogBackgroundColor();
    void UpdateTitleArrowsColor();
    void UpdateOptionsButtonColor();

    RefPtr<FrameNode> GetCalendarFrameNode();
    RefPtr<CalendarPattern> GetCalendarPattern();
    RefPtr<FrameNode> GetSwiperFrameNode();
    RefPtr<SwiperPattern> GetSwiperPattern();

    void UpdateSwiperNode(const ObtainedMonth& monthData, bool isPrev);
    void UpdateSwiperNodeFocusedDay(const CalendarDay& focusedDay, bool isPrev);

    int32_t focusAreaID_ = 0;
    int32_t focusAreaChildID_ = 0;
    CalendarDay focusedDay_ = { .day = -1 };
    bool isFirstAddhotZoneRect_ = true;
    bool isFocused_ = false;
    bool isCalendarFirstFocused_ = false;
    bool hoverState_ = false;
    OffsetF dialogOffset_;
    WeakPtr<FrameNode> entryNode_ = nullptr;
    RefPtr<TouchEventImpl> touchListener_ = nullptr;
    RefPtr<InputEvent> hoverListener_ = nullptr;
    ACE_DISALLOW_COPY_AND_MOVE(CalendarDialogPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CALENDAR_PICKER_CALENDAR_DIALOG_PATTERN_H
