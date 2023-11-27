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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_RICH_EDITOR_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_RICH_EDITOR_PATTERN_H

#include "core/common/ime/text_edit_controller.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_client.h"
#include "core/common/ime/text_input_configuration.h"
#include "core/common/ime/text_input_connection.h"
#include "core/common/ime/text_input_formatter.h"
#include "core/common/ime/text_input_proxy.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/ime/text_selection.h"
#include "core/components_ng/pattern/rich_editor/paragraph_manager.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_content_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_controller.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_event_hub.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_algorithm.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_paint_method.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_selection.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_pattern.h"

#if not defined(ACE_UNITTEST)
#if defined(ENABLE_STANDARD_INPUT)
#include "commonlibrary/c_utils/base/include/refbase.h"

namespace OHOS::MiscServices {
class OnTextChangedListener;
struct TextConfig;
} // namespace OHOS::MiscServices
#endif
#endif

namespace OHOS::Ace::NG {
// TextPattern is the base class for text render node to perform paint text.
enum class MoveDirection { FORWARD, BACKWARD };

enum class AutoScrollEvent { HANDLE, DRAG, MOUSE, NONE };
enum class EdgeDetectionStrategy { OUT_BOUNDARY, IN_BOUNDARY, DISABLE };
struct AutoScrollParam {
    AutoScrollEvent autoScrollEvent = AutoScrollEvent::NONE;
    RectF handleRect;
    bool isFirstHandle = false;
    float offset = 0.0f;
    bool showScrollbar = false;
    Offset eventOffset;
};

struct SelectionMenuParams {
    RichEditorType type;
    std::function<void()> buildFunc;
    std::function<void(int32_t, int32_t)> onAppear;
    std::function<void()> onDisappear;
    RichEditorResponseType responseType;

    SelectionMenuParams(RichEditorType _type, std::function<void()> _buildFunc,
        std::function<void(int32_t, int32_t)> _onAppear, std::function<void()> _onDisappear,
        RichEditorResponseType _responseType)
        : type(_type), buildFunc(_buildFunc), onAppear(_onAppear), onDisappear(_onDisappear),
          responseType(_responseType)
    {}
};

class RichEditorPattern : public TextPattern, public TextInputClient {
    DECLARE_ACE_TYPE(RichEditorPattern, TextPattern, TextInputClient);

public:
    RichEditorPattern();
    ~RichEditorPattern() override;
    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<RichEditorEventHub>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<RichEditorLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<RichEditorLayoutAlgorithm>(spans_, &paragraphs_);
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true, FocusStyleType::INNER_BORDER };
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    const RefPtr<RichEditorController>& GetRichEditorController()
    {
        return richEditorController_;
    }

    void SetRichEditorController(const RefPtr<RichEditorController>& controller)
    {
        richEditorController_ = controller;
    }

    long long GetTimestamp() const
    {
        return timestamp_;
    }

    bool NeedSoftKeyboard() const override
    {
        return true;
    }

    void ResetBeforePaste();
    void ResetAfterPaste();

    void OnVisibleChange(bool isVisible) override;
    void OnModifyDone() override;
    void BeforeCreateLayoutWrapper() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void UpdateEditingValue(const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent = true) override;
    void PerformAction(TextInputAction action, bool forceCloseKeyboard = true) override;
    void InsertValue(const std::string& insertValue) override;
    void InsertValueByPaste(const std::string& insertValue);
    bool IsLineSeparatorInLast(RefPtr<SpanNode>& spanNode);
    void InsertValueToSpanNode(
        RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info);
    void SpanNodeFission(RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info);
    void SpanNodeFission(RefPtr<SpanNode>& spanNode);
    void CreateTextSpanNode(
        RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME = true);
    void DeleteBackward(int32_t length = 0) override;
    void DeleteForward(int32_t length) override;
    void SetInputMethodStatus(bool keyboardShown) override;
    void NotifyKeyboardClosedByUser() override
    {
        FocusHub::LostFocusToViewRoot();
    }
    bool CursorMoveLeft() override;
    bool CursorMoveRight() override;
    bool CursorMoveUp() override;
    bool CursorMoveDown() override;
    bool SetCaretPosition(int32_t pos);
    int32_t GetCaretPosition();
    int32_t GetTextContentLength();
    bool GetCaretVisible() const;
    OffsetF CalcCursorOffsetByPosition(int32_t position, float& selectLineHeight, bool downStreamFirst = false);
    void CopyTextSpanStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target);
    int32_t TextSpanSplit(int32_t position);
    SpanPositionInfo GetSpanPositionInfo(int32_t position);
    std::function<ImageSourceInfo()> CreateImageSourceInfo(const ImageSpanOptions& options);
    void DeleteSpans(const RangeOptions& options);
    void DeleteSpanByRange(int32_t start, int32_t end, SpanPositionInfo info);
    void DeleteSpansByRange(int32_t start, int32_t end, SpanPositionInfo startInfo, SpanPositionInfo endInfo);
    void ClearContent(const RefPtr<UINode>& child);
    void CloseSelectionMenu();
    bool SetCaretOffset(int32_t caretPosition);
    void ResetFirstNodeStyle();
    void FireOnDeleteComplete(const RichEditorDeleteValue& info);

    void UpdateSpanStyle(int32_t start, int32_t end, const TextStyle& textStyle, const ImageSpanAttribute& imageStyle);
    void SetUpdateSpanStyle(struct UpdateSpanStyle updateSpanStyle);
    void UpdateParagraphStyle(int32_t start, int32_t end, const UpdateParagraphStyle& style);
    std::vector<ParagraphInfo> GetParagraphInfo(int32_t start, int32_t end);
    void SetTypingStyle(struct UpdateSpanStyle typingStyle, TextStyle textStyle);
    int32_t AddImageSpan(const ImageSpanOptions& options, bool isPaste = false, int32_t index = -1);
    int32_t AddTextSpan(const TextSpanOptions& options, bool isPaste = false, int32_t index = -1);
    void AddSpanItem(const RefPtr<SpanItem>& item, int32_t offset);
    int32_t AddPlaceholderSpan(const RefPtr<UINode>& customNode, const SpanOptionBase& options);
    RichEditorSelection GetSpansInfo(int32_t start, int32_t end, GetSpansMethod method);
    void SetSelection(int32_t start, int32_t end);
    void OnHandleMoveDone(const RectF& handleRect, bool isFirstHandle) override;
    std::u16string GetLeftTextOfCursor(int32_t number) override;
    std::u16string GetRightTextOfCursor(int32_t number) override;
    int32_t GetTextIndexAtCursor() override;
    void ShowSelectOverlay(const RectF& firstHandle, const RectF& secondHandle, bool isCopyAll = false,
        RichEditorResponseType responseType = RichEditorResponseType::LONG_PRESS);
    void OnHandleMove(const RectF& handleRect, bool isFirstHandle) override;
    int32_t GetHandleIndex(const Offset& offset) const override;
    void OnAreaChangedInner() override;
    void CreateHandles() override;
    void HandleOnSelectAll() override;
    void HandleOnCopy() override;
    bool JudgeDraggable(GestureEvent& info);

    bool IsUsingMouse() const
    {
        return isMousePressed_;
    }

    void ResetIsMousePressed()
    {
        isMousePressed_ = false;
    }

    OffsetF GetSelectionMenuOffset() const
    {
        return selectionMenuOffsetByMouse_;
    }

    OffsetF GetLastClickOffset() const
    {
        return lastClickOffset_;
    }

    void SetLastClickOffset(const OffsetF& lastClickOffset)
    {
        lastClickOffset_ = lastClickOffset;
    }

    void ResetLastClickOffset()
    {
        lastClickOffset_.Reset();
    }

    int32_t GetCaretSpanIndex()
    {
        return caretSpanIndex_;
    }

    const std::list<ParagraphManager::ParagraphInfo>& GetParagraphs() const
    {
        return paragraphs_.GetParagraphs();
    }

    RectF GetCaretRect() const override;
    void CloseSelectOverlay() override;
    void CalculateHandleOffsetAndShowOverlay(bool isUsingMouse = false);
    void CopySelectionMenuParams(SelectOverlayInfo& selectInfo, RichEditorResponseType responseType);
#ifdef ENABLE_DRAG_FRAMEWORK
    std::function<void(Offset)> GetThumbnailCallback() override;
#endif
    void ResetSelection();
    bool BetweenSelectedPosition(const Offset& globalOffset) override;
    void HandleSurfaceChanged(int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight) override;
    void HandleSurfacePositionChanged(int32_t posX, int32_t posY) override;
    bool RequestCustomKeyboard();
    bool CloseCustomKeyboard();
    const std::string& GetPasteStr() const
    {
        return pasteStr_;
    }
    void AddPasteStr(const std::string& addedStr)
    {
        pasteStr_.append(addedStr);
    }
    void ClearPasteStr()
    {
        pasteStr_.clear();
    }
    void SetCustomKeyboard(const std::function<void()>&& keyboardBuilder)
    {
        if (customKeyboardBuilder_ && isCustomKeyboardAttached_ && !keyboardBuilder) {
            CloseCustomKeyboard();
        }
        customKeyboardBuilder_ = keyboardBuilder;
    }
    void BindSelectionMenu(RichEditorResponseType type, RichEditorType richEditorType,
        std::function<void()>& menuBuilder, std::function<void(int32_t, int32_t)>& onAppear,
        std::function<void()>& onDisappear);
    void ClearSelectionMenu()
    {
        selectionMenuMap_.clear();
    }
    void DumpInfo() override;
    void InitSelection(const Offset& pos);
    bool HasFocus() const;
    void OnColorConfigurationUpdate() override {}
    bool IsDisabled() const;
    float GetLineHeight() const override;
    std::vector<RectF> GetTextBoxes() override;
    bool OnBackPressed() override;

    // Add for Scroll
    const RectF& GetTextRect() override
    {
        return richTextRect_;
    }

    float GetScrollOffset() const
    {
        return scrollOffset_;
    }

    RefPtr<ScrollBar> GetScrollControllerBar()
    {
        return GetScrollBar();
    }

    bool OnScrollCallback(float offset, int32_t source) override;
    void OnScrollEndCallback() override;
    bool IsScrollable() const override
    {
        return scrollable_;
    }

    bool IsAtomicNode() const override
    {
        return true;
    }

    void CheckHandles(SelectHandleInfo& handleInfo) override;

private:
    void UpdateSelectMenuInfo(bool hasData, SelectOverlayInfo& selectInfo, bool isCopyAll)
    {
        auto hasValue = (static_cast<int32_t>(GetWideText().length()) + imageCount_) > 0;
        bool isShowItem = copyOption_ != CopyOptions::None;
        selectInfo.menuInfo.showCopy = isShowItem && hasValue && textSelector_.IsValid() &&
                                       !textSelector_.StartEqualToDest();
        selectInfo.menuInfo.showCut = isShowItem && hasValue && textSelector_.IsValid() &&
                                      !textSelector_.StartEqualToDest();
        selectInfo.menuInfo.showCopyAll = !isCopyAll && hasValue;
        selectInfo.menuInfo.showPaste = hasData;
        selectInfo.menuInfo.menuIsShow = hasValue || hasData;
        selectMenuInfo_ = selectInfo.menuInfo;
    }
    void UpdateSelectionType(RichEditorSelection& selection);
    std::shared_ptr<SelectionMenuParams> GetMenuParams(RichEditorResponseType responseType, RichEditorType type);
    void HandleOnPaste();
    void HandleOnCut();
    void InitClickEvent(const RefPtr<GestureEventHub>& gestureHub) override;
    void InitFocusEvent(const RefPtr<FocusHub>& focusHub);
    void HandleBlurEvent();
    void HandleFocusEvent();
    void HandleClickEvent(GestureEvent& info);
    void HandleSingleClickEvent(GestureEvent& info);
    void HandleDoubleClickEvent(GestureEvent& info);
    bool HandleUserClickEvent(GestureEvent& info);
    bool HandleUserLongPressEvent(GestureEvent& info);
    bool HandleUserGestureEvent(
        GestureEvent& info, std::function<bool(RefPtr<SpanItem> item, GestureEvent& info)>&& gestureFunc);
    void CalcCaretInfoByClick(GestureEvent& info);
    void HandleEnabled();
    void InitMouseEvent();
    void ScheduleCaretTwinkling();
    void OnCaretTwinkling();
    void StartTwinkling();
    void StopTwinkling();
    void UpdateTextStyle(RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle);
    void UpdateImageStyle(RefPtr<FrameNode>& imageNode, const ImageSpanAttribute& imageStyle);
    void InitTouchEvent();
    bool SelectOverlayIsOn();
    void HandleLongPress(GestureEvent& info);
    void HandleDoubleClickOrLongPress(GestureEvent& info);
    std::string GetPositionSpansText(int32_t position, int32_t& startSpan);
    void FireOnSelect(int32_t selectStart, int32_t selectEnd);
    void MouseRightFocus(const MouseInfo& info);
    void HandleMouseLeftButton(const MouseInfo& info);
    void HandleMouseRightButton(const MouseInfo& info);
    void HandleMouseEvent(const MouseInfo& info);
    void HandleTouchEvent(const TouchEventInfo& info);
    void InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub);
    void UseHostToUpdateTextFieldManager();
    void UpdateTextFieldManager(const Offset& offset, float height);
    void ScrollToSafeArea() const override;
#ifdef ENABLE_DRAG_FRAMEWORK
    void InitDragDropEvent();
    void UpdateSpanItemDragStatus(const std::list<ResultObject>& resultObjects, bool IsDragging);
    NG::DragDropInfo OnDragStart(const RefPtr<OHOS::Ace::DragEvent>& event);
    void OnDragEnd();
    void OnDragMove(const RefPtr<OHOS::Ace::DragEvent>& event);

    void AddDragFrameNodeToManager(const RefPtr<FrameNode>& frameNode)
    {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto dragDropManager = context->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->AddDragFrameNode(frameNode->GetId(), AceType::WeakClaim(AceType::RawPtr(frameNode)));
    }
#endif // ENABLE_DRAG_FRAMEWORK

    int32_t GetParagraphLength(const std::list<RefPtr<UINode>>& spans) const;
    // REQUIRES: 0 <= start < end
    std::vector<RefPtr<SpanNode>> GetParagraphNodes(int32_t start, int32_t end) const;
    RefPtr<UINode> GetChildByIndex(int32_t index) const;
    RefPtr<SpanItem> GetSpanItemByIndex(int32_t index) const;
    std::string GetSelectedSpanText(std::wstring value, int32_t start, int32_t end) const;
    TextStyleResult GetTextStyleObject(const RefPtr<SpanNode>& node);
    ResultObject GetTextResultObject(RefPtr<UINode> uinode, int32_t index, int32_t start, int32_t end);
    ResultObject GetImageResultObject(RefPtr<UINode> uinode, int32_t index, int32_t start, int32_t end);
    void OnHover(bool isHover);
    bool RequestKeyboard(bool isFocusViewChanged, bool needStartTwinkling, bool needShowSoftKeyboard);
    void UpdateCaretInfoToController();
#if defined(ENABLE_STANDARD_INPUT)
    bool EnableStandardInput(bool needShowSoftKeyboard);
    std::optional<MiscServices::TextConfig> GetMiscTextConfig();
#else
    bool UnableStandardInput(bool isFocusViewChanged);
#endif

    bool HasConnection() const;
    bool CloseKeyboard(bool forceClose) override;
    void CalcInsertValueObj(TextInsertValueInfo& info);
    void CalcDeleteValueObj(int32_t currentPosition, int32_t length, RichEditorDeleteValue& info);
    int32_t DeleteValueSetImageSpan(const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult);
    int32_t DeleteValueSetTextSpan(const RefPtr<SpanItem>& spanItem, int32_t currentPosition, int32_t length,
        RichEditorAbstractSpanResult& spanResult);
    void DeleteByDeleteValueInfo(const RichEditorDeleteValue& info);
    bool OnKeyEvent(const KeyEvent& keyEvent);
    bool HandleShiftPressedEvent(const KeyEvent& event);
    bool HandleDirectionalKey(const KeyEvent& keyEvent);
    void ParseAppendValue(KeyCode keyCode, std::string& appendElement);
    void MoveCaretAfterTextChange();
    bool BeforeIMEInsertValue(const std::string& insertValue);
    void AfterIMEInsertValue(const RefPtr<SpanNode>& spanNode, int32_t moveLength, bool isCreate);
    RefPtr<SpanNode> InsertValueToBeforeSpan(RefPtr<SpanNode>& spanNodeBefore, const std::string& insertValue);
    void SetCaretSpanIndex(int32_t index);
    bool HasSameTypingStyle(const RefPtr<SpanNode>& spanNode);
    bool IsSelectedBindSelectionMenu();

    // add for scroll.
    void UpdateChildrenOffset();
    void MoveFirstHandle(float offset);
    void MoveSecondHandle(float offset);
    void InitScrollablePattern();
    bool IsReachedBoundary(float offset);
    void UpdateScrollBarOffset() override;
    void CheckScrollable();
    void UpdateScrollStateAfterLayout(bool shouldDisappear);
    void ScheduleAutoScroll(AutoScrollParam param);
    void OnAutoScroll(AutoScrollParam param);
    void StopAutoScroll();
    void AutoScrollByEdgeDetection(AutoScrollParam param, OffsetF offset, EdgeDetectionStrategy strategy);
    float MoveTextRect(float offset);
    bool MoveCaretToContentRect();
    bool IsTextArea() const override
    {
        return true;
    }
    void ProcessInnerPadding();

    // ai analysis fun
    bool NeedAiAnalysis(
        const CaretUpdateType targeType, const int32_t pos, const int32_t& spanStart, const std::string& content);
    void AdjustCursorPosition(int32_t& pos);
    void AdjustWordSelection(int32_t& start, int32_t& end);
    bool IsClickBoundary(const int32_t position);

#if defined(ENABLE_STANDARD_INPUT)
    sptr<OHOS::MiscServices::OnTextChangedListener> richEditTextChangeListener_;
#else
    RefPtr<TextInputConnection> connection_ = nullptr;
#endif
    bool isMouseSelect_ = false;
    bool isMousePressed_ = false;
    bool isFirstMouseSelect_ = true;
    bool leftMousePress_ = false;
    bool isLongPress_ = false;
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    bool imeAttached_ = false;
    bool imeShown_ = false;
#endif

    bool isTextChange_ = false;
    bool caretVisible_ = false;
    bool isRichEditorInit_ = false;
    bool clickEventInitialized_ = false;
    bool focusEventInitialized_ = false;
    bool blockPress_ = false;
    bool isCustomKeyboardAttached_ = false;
    bool usingMouseRightButton_ = false;

    int32_t moveLength_ = 0;
    int32_t caretPosition_ = 0;
    int32_t caretSpanIndex_ = -1;
    long long timestamp_ = 0;
    OffsetF parentGlobalOffset_;
    OffsetF selectionMenuOffsetByMouse_;
    OffsetF lastClickOffset_;
    std::string pasteStr_;

    // still in progress
    ParagraphManager paragraphs_;

    RefPtr<TouchEventImpl> touchListener_;
    struct UpdateSpanStyle updateSpanStyle_;
    CancelableCallback<void()> caretTwinklingTask_;
    RefPtr<RichEditorController> richEditorController_;
    MoveDirection moveDirection_ = MoveDirection::FORWARD;
    RectF frameRect_;
    std::optional<struct UpdateSpanStyle> typingStyle_;
    std::optional<TextStyle> typingTextStyle_;
#ifdef ENABLE_DRAG_FRAMEWORK
    std::list<ResultObject> dragResultObjects_;
#endif // ENABLE_DRAG_FRAMEWORK
    std::map<std::pair<RichEditorType, RichEditorResponseType>, std::shared_ptr<SelectionMenuParams>> selectionMenuMap_;
    std::optional<RichEditorType> selectedType_;

    std::function<void()> customKeyboardBuilder_;
    Offset selectionMenuOffset_;
    // add for scroll
    RectF richTextRect_;
    float scrollOffset_ = 0.0f;
    bool isFirstCallOnReady_ = false;
    bool scrollable_ = true;
    CancelableCallback<void()> autoScrollTask_;
    OffsetF prevAutoScrollOffset_;
    // add for ai input analysis
    bool hasClicked_ = false;
    CaretUpdateType caretUpdateType_ = CaretUpdateType::NONE;
    TimeStamp lastClickTimeStamp_;
    TimeStamp lastAiPosTimeStamp_;

    ACE_DISALLOW_COPY_AND_MOVE(RichEditorPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_RICH_EDITOR_PATTERN_H
