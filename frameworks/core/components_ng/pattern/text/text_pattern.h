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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_PATTERN_H

#include <optional>
#include <string>
#include <unordered_map>

#include "interfaces/inner_api/ace/ai/data_detector_interface.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "base/utils/utils.h"
#include "core/components_ng/event/long_press_event.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_accessibility_property.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text/text_content_modifier.h"
#include "core/components_ng/pattern/text/text_event_hub.h"
#include "core/components_ng/pattern/text/text_layout_algorithm.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_overlay_modifier.h"
#include "core/components_ng/pattern/text/text_paint_method.h"
#include "core/components_ng/pattern/text_drag/text_drag_base.h"
#include "core/components_ng/pattern/text_field/text_selector.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
// TextPattern is the base class for text render node to perform paint text.
class TextPattern : public ScrollablePattern, public TextDragBase, public TextBase {
    DECLARE_ACE_TYPE(TextPattern, ScrollablePattern, TextDragBase, TextBase);

public:
    TextPattern() = default;
    ~TextPattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<TextLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<TextLayoutAlgorithm>(spans_);
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<TextAccessibilityProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<TextEventHub>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    bool DefaultSupportDrag() override
    {
        return true;
    }

    void OnModifyDone() override;

    void PreCreateLayoutWrapper();

    void BeforeCreateLayoutWrapper() override;

    void AddChildSpanItem(const RefPtr<UINode>& child);

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, false };
    }

    void DumpAdvanceInfo() override;
    void DumpInfo() override;

    TextSelector GetTextSelector() const
    {
        return textSelector_;
    }

    std::string GetTextForDisplay() const
    {
        return textForDisplay_;
    }

    const OffsetF& GetStartOffset() const
    {
        return textSelector_.selectionBaseOffset;
    }

    const OffsetF& GetEndOffset() const
    {
        return textSelector_.selectionDestinationOffset;
    }

    double GetSelectHeight() const
    {
        return textSelector_.GetSelectHeight();
    }

    void GetGlobalOffset(Offset& offset);

    const RectF& GetTextContentRect() const override
    {
        return contentRect_;
    }

    float GetBaselineOffset() const
    {
        return baselineOffset_;
    }

    RefPtr<TextContentModifier> GetContentModifier()
    {
        return contentMod_;
    }

    void SetMenuOptionItems(std::vector<MenuOptionsParam>&& menuOptionItems)
    {
        menuOptionItems_ = std::move(menuOptionItems);
    }

    void SetTextDetectEnable(bool enable)
    {
        bool cache = textDetectEnable_;
        textDetectEnable_ = enable;
        if (cache != textDetectEnable_) {
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }

    bool GetTextDetectEnable()
    {
        return textDetectEnable_;
    }

    void SetTextDetectTypes(const std::string& types);

    std::string GetTextDetectTypes()
    {
        return textDetectTypes_;
    }

    void SetOnResult(std::function<void(const std::string&)>&& onResult)
    {
        onResult_ = std::move(onResult);
    }

    void FireOnResult(const std::string& value)
    {
        if (onResult_) {
            onResult_(value);
        }
    }

    void SetTextDetectResult(const TextDataDetectResult result)
    {
        textDetectResult_ = result;
    }

    std::optional<TextDataDetectResult> GetTextDetectResult()
    {
        return textDetectResult_;
    }

    const std::vector<MenuOptionsParam>&& GetMenuOptionItems() const
    {
        return std::move(menuOptionItems_);
    }

    void OnVisibleChange(bool isVisible) override;

    std::list<RefPtr<SpanItem>> GetSpanItemChildren()
    {
        return spans_;
    }

    int32_t GetDisplayWideTextLength()
    {
        return StringUtils::ToWstring(textForDisplay_).length();
    }

    // ===========================================================
    // TextDragBase implementations

    bool IsTextArea() const override
    {
        return false;
    }

    const RectF& GetTextRect() override
    {
        return contentRect_;
    }
    float GetLineHeight() const override;

    std::vector<RectF> GetTextBoxes() override;
    OffsetF GetParentGlobalOffset() const override;

    OffsetF GetTextPaintOffset() const;

    const RefPtr<FrameNode>& MoveDragNode() override
    {
        return dragNode_;
    }

    ParagraphT GetDragParagraph() const override
    {
        return { paragraph_ };
    }

    bool CloseKeyboard(bool /* forceClose */) override
    {
        return true;
    }
    virtual void CloseSelectOverlay() override;
    void CloseSelectOverlay(bool animation);
    void CreateHandles() override;

    bool BetweenSelectedPosition(const Offset& globalOffset) override;

    // end of TextDragBase implementations
    // ===========================================================

    void InitSurfaceChangedCallback();
    void InitSurfacePositionChangedCallback();
    virtual void HandleSurfaceChanged(int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight);
    virtual void HandleSurfacePositionChanged(int32_t posX, int32_t posY) {};
    bool HasSurfaceChangedCallback()
    {
        return surfaceChangedCallbackId_.has_value();
    }
    void UpdateSurfaceChangedCallbackId(int32_t id)
    {
        surfaceChangedCallbackId_ = id;
    }

    bool HasSurfacePositionChangedCallback()
    {
        return surfacePositionChangedCallbackId_.has_value();
    }
    void UpdateSurfacePositionChangedCallbackId(int32_t id)
    {
        surfacePositionChangedCallbackId_ = id;
    }

    void SetOnClickEvent(GestureEventFunc&& onClick)
    {
        onClick_ = std::move(onClick);
    }
    virtual void OnColorConfigurationUpdate() override;

#ifdef ENABLE_DRAG_FRAMEWORK
    DragDropInfo OnDragStart(const RefPtr<Ace::DragEvent>& event, const std::string& extraParams);
    void InitDragEvent();
    virtual std::function<void(Offset)> GetThumbnailCallback();
#endif

    void InitSpanImageLayout(const std::vector<int32_t>& placeholderIndex,
        const std::vector<RectF>& rectsForPlaceholders, OffsetF contentOffset) override
    {
        placeholderIndex_ = placeholderIndex;
        imageOffset_ = contentOffset;
        rectsForPlaceholders_ = rectsForPlaceholders;
    }

    const std::vector<int32_t>& GetPlaceHolderIndex()
    {
        return placeholderIndex_;
    }

    const std::vector<RectF>& GetRectsForPlaceholders()
    {
        return rectsForPlaceholders_;
    }

    OffsetF GetContentOffset() override
    {
        return imageOffset_;
    }

    bool IsMeasureBoundary() const override
    {
        return isMeasureBoundary_;
    }

    void SetIsMeasureBoundary(bool isMeasureBoundary)
    {
        isMeasureBoundary_ = isMeasureBoundary;
    }

    void SetIsCustomFont(bool isCustomFont)
    {
        isCustomFont_ = isCustomFont;
    }

    bool GetIsCustomFont()
    {
        return isCustomFont_;
    }
    void UpdateSelectOverlayOrCreate(SelectOverlayInfo selectInfo, bool animation = false);
    virtual void CheckHandles(SelectHandleInfo& handleInfo);
    OffsetF GetDragUpperLeftCoordinates() override;
    void SetTextSelection(int32_t selectionStart, int32_t selectionEnd);

#ifndef USE_GRAPHIC_TEXT_GINE
    static RSTypographyProperties::TextBox ConvertRect(const Rect& rect);
#else
    static RSTextRect ConvertRect(const Rect& rect);
#endif
    // override SelectOverlayClient methods
    void OnHandleMoveDone(const RectF& handleRect, bool isFirstHandle) override;
    void OnHandleMove(const RectF& handleRect, bool isFirstHandle) override;
    void OnSelectOverlayMenuClicked(SelectOverlayMenuId menuId) override
    {
        switch (menuId) {
            case SelectOverlayMenuId::COPY:
                HandleOnCopy();
                return;
            case SelectOverlayMenuId::SELECT_ALL:
                HandleOnSelectAll();
                return;
            default:
                return;
        }
    }

    RefPtr<FrameNode> GetClientHost() const override
    {
        return GetHost();
    }

    RefPtr<Paragraph> GetParagraph()
    {
        return paragraph_;
    }

    void OnAreaChangedInner() override;
    void RemoveAreaChangeInner();
    bool IsAtBottom() const override
    {
        return true;
    }

    bool IsAtTop() const override
    {
        return true;
    }

    bool UpdateCurrentOffset(float offset, int32_t source) override
    {
        return true;
    }

    virtual void UpdateScrollBarOffset() override {}

    const std::map<int32_t, AISpan>& GetAISpanMap()
    {
        return aiSpanMap_;
    }

    const std::string& GetTextForAI()
    {
        return textForAI_;
    }

protected:
    virtual void HandleOnCopy();
    void InitMouseEvent();
    void ResetSelection();
    virtual void HandleOnSelectAll();
    void InitSelection(const Offset& pos);
    void HandleLongPress(GestureEvent& info);
    void HandleClickEvent(GestureEvent& info);
    void HandleSingleClickEvent(GestureEvent& info);
    void HandleSpanSingleClickEvent(GestureEvent& info, RectF textContentRect, PointF textOffset, bool& isClickOnSpan);
    void HandleDoubleClickEvent(GestureEvent& info);
    void InitTextDetect(int32_t startPos, std::string detectText);
    void ShowUIExtensionMenu(AISpan aiSpan);
    bool ClickAISpan(GestureEvent& info, PointF textOffset);
    void ParseAIResult(const TextDataDetectResult& result, int32_t startPos);
    void ParseAIJson(const std::unique_ptr<JsonValue>& jsonValue, TextDataDetectType type, int32_t startPos,
        bool isMenuOption = false);
    void StartAITask();
    bool IsDraggable(const Offset& localOffset);
    virtual void InitClickEvent(const RefPtr<GestureEventHub>& gestureHub);
    void CalculateHandleOffsetAndShowOverlay(bool isUsingMouse = false);
    void ShowSelectOverlay(const RectF& firstHandle, const RectF& secondHandle);
    void ShowSelectOverlay(const RectF& firstHandle, const RectF& secondHandle, bool animation);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    bool IsSelectAll();
    virtual int32_t GetHandleIndex(const Offset& offset) const;
    std::wstring GetWideText() const;
    std::string GetSelectedText(int32_t start, int32_t end) const;
    void CalcCaretMetricsByPosition(
        int32_t extent, CaretMetricsF& caretCaretMetric, TextAffinity textAffinity = TextAffinity::DOWNSTREAM);

    bool showSelectOverlay_ = false;
    bool mouseEventInitialized_ = false;
    bool panEventInitialized_ = false;
    bool clickEventInitialized_ = false;
    bool touchEventInitialized_ = false;

    RectF contentRect_;
    RefPtr<FrameNode> dragNode_;
    RefPtr<LongPressEvent> longPressEvent_;
    RefPtr<SelectOverlayProxy> selectOverlayProxy_;
    RefPtr<Clipboard> clipboard_;
    RefPtr<TextContentModifier> contentMod_;
    RefPtr<TextOverlayModifier> overlayMod_;
    CopyOptions copyOption_ = CopyOptions::None;

    std::string textForDisplay_;
    std::optional<TextStyle> textStyle_;
    std::list<RefPtr<SpanItem>> spans_;
    float baselineOffset_ = 0.0f;
    int32_t imageCount_ = 0;
    SelectMenuInfo selectMenuInfo_;
    std::vector<RectF> dragBoxes_;

    // properties for AI
    bool textDetectEnable_ = false;
    bool aiDetectInitialized_ = false;
    bool aiDetectTypesChanged_ = false;
    std::string textDetectTypes_;
    std::set<std::string> textDetectTypesSet_;
    std::string textForAI_;
    std::optional<TextDataDetectResult> textDetectResult_;
    std::unordered_map<std::string, std::vector<std::string>> aiMenuOptionsMap_;
    std::function<void(const std::string&)> onResult_;
    std::map<int32_t, AISpan> aiSpanMap_;
    CancelableCallback<void()> aiDetectDelayTask_;

private:
    void OnDetachFromFrameNode(FrameNode* node) override;
    void OnAttachToFrameNode() override;
    void InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleMouseEvent(const MouseInfo& info);
    void OnHandleTouchUp();
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandlePanStart(const GestureEvent& info);
    void HandlePanUpdate(const GestureEvent& info);
    void HandlePanEnd(const GestureEvent& info);
    void InitTouchEvent();
    void HandleTouchEvent(const TouchEventInfo& info);
    void UpdateChildProperty(const RefPtr<SpanNode>& child) const;
    void ActSetSelection(int32_t start, int32_t end);
    void SetAccessibilityAction();
    void CollectSpanNodes(std::stack<RefPtr<UINode>> nodes, bool& isSpanHasClick);
    RefPtr<RenderContext> GetRenderContext();
    void ProcessBoundRectByTextShadow(RectF& rect);
    // to check if drag is in progress

    bool isMeasureBoundary_ = false;
    bool isMousePressed_ = false;
    bool isCustomFont_ = false;
    bool blockPress_ = false;
    bool hasClicked_ = false;
    bool isDoubleClick_ = false;
    TimeStamp lastClickTimeStamp_;

    RefPtr<Paragraph> paragraph_;
    std::vector<MenuOptionsParam> menuOptionItems_;
    std::vector<int32_t> placeholderIndex_;
    std::vector<RectF> rectsForPlaceholders_;
    OffsetF imageOffset_;

    OffsetF contentOffset_;
    OffsetF parentGlobalOffset_;
    GestureEventFunc onClick_;
    RefPtr<DragWindow> dragWindow_;
    RefPtr<DragDropProxy> dragDropProxy_;
    std::optional<int32_t> surfaceChangedCallbackId_;
    std::optional<int32_t> surfacePositionChangedCallbackId_;
    ACE_DISALLOW_COPY_AND_MOVE(TextPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_PATTERN_H
