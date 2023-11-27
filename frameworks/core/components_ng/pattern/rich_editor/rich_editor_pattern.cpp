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
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iterator>
#include <string>
#include <utility>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/log/dump_log.h"
#include "base/memory/ace_type.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/common/ai/data_detector_mgr.h"
#include "core/common/clipboard/paste_data.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/long_press_event.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_event_hub.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_selection.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/pattern/text_field/text_input_ai_checker.h"
#include "core/components_ng/property/property.h"
#include "core/gestures/gesture_info.h"
#include "core/pipeline/base/element_register.h"

#if not defined(ACE_UNITTEST)
#if defined(ENABLE_STANDARD_INPUT)
#include "commonlibrary/c_utils/base/include/refbase.h"

#include "core/components_ng/pattern/text_field/on_text_changed_listener_impl.h"
#endif
#endif

#ifdef ENABLE_DRAG_FRAMEWORK
#include "core/common/ace_engine_ext.h"
#include "core/common/udmf/udmf_client.h"
#endif

namespace OHOS::Ace::NG {
namespace {
const std::string NEWLINE = "\n";
const std::wstring WIDE_NEWLINE = StringUtils::ToWstring(NEWLINE);
#if defined(ENABLE_STANDARD_INPUT)
// should be moved to theme
constexpr float CARET_WIDTH = 1.5f;
constexpr float DEFAULT_CARET_HEIGHT = 18.5f;
#endif
constexpr int32_t IMAGE_SPAN_LENGTH = 1;
constexpr int32_t RICH_EDITOR_TWINKLING_INTERVAL_MS = 500;
constexpr float DEFAULT_TEXT_SIZE = 16.0f;
constexpr int32_t AUTO_SCROLL_INTERVAL = 20;
constexpr Dimension AUTO_SCROLL_MOVE_THRESHOLD = 2.0_vp;
constexpr Dimension AUTO_SCROLL_EDGE_DISTANCE = 15.0_vp;
constexpr Dimension AUTO_SCROLL_DRAG_EDGE_DISTANCE = 25.0_vp;
constexpr float DOUBLE_CLICK_INTERVAL_MS = 300.0f;
constexpr float BOX_EPSILON = 0.5f;

const std::wstring lineSeparator = L"\n";
const std::wstring NUM_SYMBOL = L")!@#$%^&*(";
// hen do ai anaylsis, we should limit the left an right limit of the string
constexpr static int32_t AI_TEXT_RANGE_LEFT = 50;
constexpr static int32_t AI_TEXT_RANGE_RIGHT = 50;

const std::unordered_map<KeyCode, wchar_t> KEYBOARD_SYMBOL = {
    { KeyCode::KEY_GRAVE, L'`' },
    { KeyCode::KEY_MINUS, L'-' },
    { KeyCode::KEY_EQUALS, L'=' },
    { KeyCode::KEY_LEFT_BRACKET, L'[' },
    { KeyCode::KEY_RIGHT_BRACKET, L']' },
    { KeyCode::KEY_BACKSLASH, L'\\' },
    { KeyCode::KEY_SEMICOLON, L';' },
    { KeyCode::KEY_APOSTROPHE, L'\'' },
    { KeyCode::KEY_COMMA, L',' },
    { KeyCode::KEY_PERIOD, L'.' },
    { KeyCode::KEY_SLASH, L'/' },
    { KeyCode::KEY_SPACE, L' ' },
    { KeyCode::KEY_NUMPAD_DIVIDE, L'/' },
    { KeyCode::KEY_NUMPAD_MULTIPLY, L'*' },
    { KeyCode::KEY_NUMPAD_SUBTRACT, L'-' },
    { KeyCode::KEY_NUMPAD_ADD, L'+' },
    { KeyCode::KEY_NUMPAD_DOT, L'.' },
    { KeyCode::KEY_NUMPAD_COMMA, L',' },
    { KeyCode::KEY_NUMPAD_EQUALS, L'=' },
};
static const std::unordered_map<KeyCode, wchar_t> SHIFT_KEYBOARD_SYMBOL = {
    { KeyCode::KEY_GRAVE, L'~' },
    { KeyCode::KEY_MINUS, L'_' },
    { KeyCode::KEY_EQUALS, L'+' },
    { KeyCode::KEY_LEFT_BRACKET, L'{' },
    { KeyCode::KEY_RIGHT_BRACKET, L'}' },
    { KeyCode::KEY_BACKSLASH, L'|' },
    { KeyCode::KEY_SEMICOLON, L':' },
    { KeyCode::KEY_APOSTROPHE, L'\"' },
    { KeyCode::KEY_COMMA, L'<' },
    { KeyCode::KEY_PERIOD, L'>' },
    { KeyCode::KEY_SLASH, L'?' },
};
} // namespace
RichEditorPattern::RichEditorPattern() {}

RichEditorPattern::~RichEditorPattern()
{
    if (isCustomKeyboardAttached_) {
        CloseCustomKeyboard();
    }
}

void RichEditorPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    copyOption_ = layoutProperty->GetCopyOption().value_or(CopyOptions::Distributed);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->AddOnAreaChangeNode(host->GetId());
    if (!clipboard_ && context) {
        clipboard_ = ClipboardProxy::GetInstance()->GetClipboard(context->GetTaskExecutor());
    }
    instanceId_ = context->GetInstanceId();
    InitMouseEvent();
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitFocusEvent(focusHub);
    auto gestureEventHub = host->GetOrCreateGestureEventHub();
    InitClickEvent(gestureEventHub);
    InitLongPressEvent(gestureEventHub);
    InitTouchEvent();
    HandleEnabled();
    ProcessInnerPadding();
    InitScrollablePattern();
#ifdef ENABLE_DRAG_FRAMEWORK
    if (host->IsDraggable()) {
        InitDragDropEvent();
        AddDragFrameNodeToManager(host);
    }
#endif // ENABLE_DRAG_FRAMEWORK
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::HandleEnabled()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (IsDisabled()) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
        CHECK_NULL_VOID(richEditorTheme);
        auto disabledAlpha = richEditorTheme->GetDisabledAlpha();
        renderContext->OnOpacityUpdate(disabledAlpha);
    } else {
        auto opacity = renderContext->GetOpacity().value_or(1.0);
        renderContext->OnOpacityUpdate(opacity);
    }
}

void RichEditorPattern::BeforeCreateLayoutWrapper()
{
    TextPattern::PreCreateLayoutWrapper();
}

bool RichEditorPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }
    frameRect_ = dirty->GetGeometryNode()->GetFrameRect();
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto richEditorLayoutAlgorithm =
        DynamicCast<RichEditorLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(richEditorLayoutAlgorithm, false);
    parentGlobalOffset_ = richEditorLayoutAlgorithm->GetParentGlobalOffset();
    richTextRect_ = richEditorLayoutAlgorithm->GetTextRect();
    UpdateTextFieldManager(Offset(parentGlobalOffset_.GetX(), parentGlobalOffset_.GetY()), frameRect_.Height());
    // skip show selectoverlay in the TextPattern.
    auto restoreSelectOverlayProxy = selectOverlayProxy_;
    selectOverlayProxy_.Reset();
    bool ret = TextPattern::OnDirtyLayoutWrapperSwap(dirty, config);
    selectOverlayProxy_ = restoreSelectOverlayProxy;
    if (textSelector_.baseOffset != -1 && textSelector_.destinationOffset != -1 && SelectOverlayIsOn()) {
        CalculateHandleOffsetAndShowOverlay();
        ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
    }
    UpdateScrollStateAfterLayout(config.frameSizeChange);
    if (!isRichEditorInit_) {
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_RETURN(eventHub, ret);
        eventHub->FireOnReady();
        isFirstCallOnReady_ = true;
        isRichEditorInit_ = true;
    }
    MoveCaretAfterTextChange();
    MoveCaretToContentRect();
    UpdateCaretInfoToController();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, ret);
    auto context = host->GetRenderContext();
    CHECK_NULL_RETURN(context, ret);
    if (context->GetClipEdge().has_value()) {
        auto geometryNode = host->GetGeometryNode();
        auto frameOffset = geometryNode->GetFrameOffset();
        auto frameSize = geometryNode->GetFrameSize();
        auto height = static_cast<float>(paragraphs_.GetHeight() + std::fabs(baselineOffset_));
        if (!context->GetClipEdge().value() && LessNotEqual(frameSize.Height(), height)) {
            RectF boundsRect(frameOffset.GetX(), frameOffset.GetY(), frameSize.Width(), height);
            CHECK_NULL_RETURN(overlayMod_, ret);
            overlayMod_->SetBoundsRect(boundsRect);
        }
    }
    caretUpdateType_ = CaretUpdateType::NONE;
    return ret;
}

std::function<ImageSourceInfo()> RichEditorPattern::CreateImageSourceInfo(const ImageSpanOptions& options)
{
    std::string src;
    RefPtr<PixelMap> pixMap = nullptr;
    std::string bundleName;
    std::string moduleName;
    if (options.image.has_value()) {
        src = options.image.value();
    }
    if (options.imagePixelMap.has_value()) {
        pixMap = options.imagePixelMap.value();
    }
    if (options.bundleName.has_value()) {
        bundleName = options.bundleName.value();
    }
    if (options.moduleName.has_value()) {
        moduleName = options.moduleName.value();
    }
    auto createSourceInfoFunc = [src, noPixMap = !options.imagePixelMap.has_value(), pixMap, bundleName,
                                    moduleName]() -> ImageSourceInfo {
#if defined(PIXEL_MAP_SUPPORTED)
        if (noPixMap) {
            return { src, bundleName, moduleName };
        }
        return ImageSourceInfo(pixMap);
#else
        return { src, bundleName, moduleName };
#endif
    };
    return std::move(createSourceInfoFunc);
}

int32_t RichEditorPattern::AddImageSpan(const ImageSpanOptions& options, bool isPaste, int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);

    auto imageNode = ImageSpanNode::GetOrCreateSpanNode(V2::IMAGE_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();

    // Disable the image itself event
    imageNode->SetDraggable(false);
    auto gesture = imageNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(gesture, -1);
    // Masked the default drag behavior of node image
    gesture->SetDragEvent(nullptr, { PanDirection::DOWN }, 0, Dimension(0));
    if (options.userGestureOption.onClick) {
        auto tmpFunc = options.userGestureOption.onClick;
        gesture->SetUserOnClick(std::move(tmpFunc));
    }
    if (options.userGestureOption.onLongPress) {
        auto tmpFunc = options.userGestureOption.onLongPress;
        auto tmpFuncPtr = AceType::MakeRefPtr<LongPressEvent>(std::move(tmpFunc));
        gesture->SetLongPressEvent(tmpFuncPtr);
    }

    int32_t spanIndex = 0;
    int32_t offset = -1;
    if (options.offset.has_value()) {
        offset = TextSpanSplit(options.offset.value());
        if (offset == -1) {
            spanIndex = host->GetChildren().size();
        } else {
            spanIndex = offset;
        }
        imageNode->MountToParent(host, offset);
    } else if (index != -1) {
        imageNode->MountToParent(host, index);
        spanIndex = index;
    } else {
        spanIndex = host->GetChildren().size();
        imageNode->MountToParent(host);
    }
    std::function<ImageSourceInfo()> createSourceInfoFunc = CreateImageSourceInfo(options);
    imageLayoutProperty->UpdateImageSourceInfo(createSourceInfoFunc());
    if (options.imageAttribute.has_value()) {
        auto imgAttr = options.imageAttribute.value();
        if (imgAttr.size.has_value()) {
            imageLayoutProperty->UpdateUserDefinedIdealSize(
                CalcSize(CalcLength(imgAttr.size.value().width), CalcLength(imgAttr.size.value().height)));
        }
        if (imgAttr.verticalAlign.has_value()) {
            imageLayoutProperty->UpdateVerticalAlign(imgAttr.verticalAlign.value());
        }
        if (imgAttr.objectFit.has_value()) {
            imageLayoutProperty->UpdateImageFit(imgAttr.objectFit.value());
        }
        if (imgAttr.marginProp.has_value()) {
            imageLayoutProperty->UpdateMargin(imgAttr.marginProp.value());
        }
        if (imgAttr.borderRadius.has_value()) {
            auto imageRenderCtx = imageNode->GetRenderContext();
            imageRenderCtx->UpdateBorderRadius(imgAttr.borderRadius.value());
            imageRenderCtx->SetClipToBounds(true);
        }
    }
    if (isPaste) {
        isTextChange_ = true;
        moveDirection_ = MoveDirection::FORWARD;
        moveLength_ += 1;
    }
    imageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    auto spanItem = imageNode->GetSpanItem();
    // The length of the imageSpan defaults to the length of a character to calculate the position
    spanItem->content = " ";
    AddSpanItem(spanItem, offset);
    if (options.offset.has_value() && options.offset.value() <= GetCaretPosition()) {
        SetCaretPosition(options.offset.value() + 1);
    } else {
        SetCaretPosition(GetCaretPosition() + 1);
    }
    if (!isPaste && textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    return spanIndex;
}

void RichEditorPattern::AddSpanItem(const RefPtr<SpanItem>& item, int32_t offset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (offset == -1) {
        offset = host->GetChildren().size();
    }
    offset = std::clamp(offset, 0, static_cast<int32_t>(host->GetChildren().size()) - 1);
    auto it = spans_.begin();
    std::advance(it, offset);
    spans_.insert(it, item);
    int32_t spanTextLength = 0;
    for (auto& span : spans_) {
        span->position = spanTextLength + StringUtils::ToWstring(span->content).length();
        spanTextLength += StringUtils::ToWstring(span->content).length();
    }
}

int32_t RichEditorPattern::AddPlaceholderSpan(const RefPtr<UINode>& customNode, const SpanOptionBase& options)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0);
    auto placeholderSpanNode = PlaceholderSpanNode::GetOrCreateSpanNode(V2::PLACEHOLDER_SPAN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<PlaceholderSpanPattern>(); });
    CHECK_NULL_RETURN(placeholderSpanNode, 0);
    customNode->MountToParent(placeholderSpanNode);
    auto frameNode = DynamicCast<FrameNode>(customNode);
    if (frameNode) {
        frameNode->SetDraggable(false);
    }
    auto focusHub = placeholderSpanNode->GetOrCreateFocusHub();
    focusHub->SetFocusable(false);
    int32_t spanIndex = 0;
    int32_t offset = -1;
    auto optionalPosition = options.offset.value_or(-1);
    if (optionalPosition >= 0) {
        offset = TextSpanSplit(options.offset.value());
        if (offset == -1) {
            spanIndex = host->GetChildren().size();
        } else {
            spanIndex = offset;
        }
        placeholderSpanNode->MountToParent(host, offset);
    } else {
        spanIndex = host->GetChildren().size();
        placeholderSpanNode->MountToParent(host);
    }
    auto spanItem = placeholderSpanNode->GetSpanItem();
    spanItem->content = " ";
    AddSpanItem(spanItem, offset);
    if (options.offset.has_value() && options.offset.value() <= GetCaretPosition()) {
        SetCaretPosition(options.offset.value() + 1);
    } else {
        SetCaretPosition(GetCaretPosition() + 1);
    }
    if (textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    placeholderSpanNode->MarkModifyDone();
    placeholderSpanNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return spanIndex;
}

int32_t RichEditorPattern::AddTextSpan(const TextSpanOptions& options, bool isPaste, int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);

    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto spanNode = SpanNode::GetOrCreateSpanNode(nodeId);

    int32_t spanIndex = 0;
    int32_t offset = -1;
    if (options.offset.has_value()) {
        offset = TextSpanSplit(options.offset.value());
        if (offset == -1) {
            spanIndex = host->GetChildren().size();
        } else {
            spanIndex = offset;
        }
        spanNode->MountToParent(host, offset);
    } else if (index != -1) {
        spanNode->MountToParent(host, index);
        spanIndex = index;
    } else {
        spanIndex = host->GetChildren().size();
        spanNode->MountToParent(host);
    }
    spanNode->UpdateContent(options.value);
    spanNode->AddPropertyInfo(PropertyInfo::NONE);
    if (options.style.has_value()) {
        spanNode->UpdateTextColor(options.style.value().GetTextColor());
        spanNode->AddPropertyInfo(PropertyInfo::FONTCOLOR);
        spanNode->UpdateFontSize(options.style.value().GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
        spanNode->UpdateItalicFontStyle(options.style.value().GetFontStyle());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSTYLE);
        spanNode->UpdateFontWeight(options.style.value().GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
        spanNode->UpdateFontFamily(options.style.value().GetFontFamilies());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFAMILY);
        spanNode->UpdateTextDecoration(options.style.value().GetTextDecoration());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
        spanNode->UpdateTextDecorationColor(options.style.value().GetTextDecorationColor());
        spanNode->AddPropertyInfo(PropertyInfo::NONE);
        spanNode->UpdateTextShadow(options.style.value().GetTextShadows());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTSHADOW);
    }
    auto spanItem = spanNode->GetSpanItem();
    spanItem->content = options.value;
    spanItem->SetTextStyle(options.style);
    AddSpanItem(spanItem, offset);
    if (options.paraStyle) {
        int32_t start = 0;
        int32_t end = 0;
        spanItem->GetIndex(start, end);
        UpdateParagraphStyle(start, end, *options.paraStyle);
    }
    if (options.userGestureOption.onClick) {
        auto tmpFunc = options.userGestureOption.onClick;
        spanItem->SetOnClickEvent(std::move(tmpFunc));
    }
    if (options.userGestureOption.onLongPress) {
        auto tmpFunc = options.userGestureOption.onLongPress;
        spanItem->SetLongPressEvent(std::move(tmpFunc));
    }
    if (!isPaste && textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    SpanNodeFission(spanNode);
    return spanIndex;
}

void RichEditorPattern::SpanNodeFission(RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    auto content = StringUtils::ToWstring(spanItem->content);
    auto contentLen = content.length();
    auto spanStart = spanItem->position - contentLen;
    for (size_t i = 0; i < content.length(); i++) {
        auto character = content[i];
        if (character == '\n') {
            auto charPosition = spanStart + i;
            TextSpanSplit(static_cast<int32_t>(charPosition + 1));
        }
    }
}

void RichEditorPattern::DeleteSpans(const RangeOptions& options)
{
    int32_t start = 0;
    int32_t end = 0;
    auto length = GetTextContentLength();
    start = (!options.start.has_value()) ? 0 : options.start.value();
    end = (!options.end.has_value()) ? length : options.end.value();
    if (start > end) {
        auto value = start;
        start = end;
        end = value;
    }
    start = std::max(0, start);
    end = std::min(length, end);
    if (start > length || end < 0 || start == end) {
        return;
    }

    auto startInfo = GetSpanPositionInfo(start);
    auto endInfo = GetSpanPositionInfo(end - 1);
    if (startInfo.spanIndex_ == endInfo.spanIndex_) {
        DeleteSpanByRange(start, end, startInfo);
    } else {
        DeleteSpansByRange(start, end, startInfo, endInfo);
    }
    if (textSelector_.IsValid()) {
        SetCaretPosition(textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    SetCaretOffset(start);
    ResetSelection();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    if (childrens.empty() || GetTextContentLength() == 0) {
        SetCaretPosition(0);
    }
}

void RichEditorPattern::DeleteSpanByRange(int32_t start, int32_t end, SpanPositionInfo info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    auto it = childrens.begin();
    std::advance(it, info.spanIndex_);
    if (start == info.spanStart_ && end == info.spanEnd_) {
        ClearContent(*it);
        host->RemoveChild(*it);
    } else {
        auto spanNode = DynamicCast<SpanNode>(*it);
        CHECK_NULL_VOID(spanNode);
        auto spanItem = spanNode->GetSpanItem();
        auto beforStr = StringUtils::ToWstring(spanItem->content).substr(0, start - info.spanStart_);
        auto endStr = StringUtils::ToWstring(spanItem->content).substr(end - info.spanStart_);
        std::wstring result = beforStr + endStr;
        auto str = StringUtils::ToString(result);
        spanNode->UpdateContent(str);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

void RichEditorPattern::DeleteSpansByRange(
    int32_t start, int32_t end, SpanPositionInfo startInfo, SpanPositionInfo endInfo)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    auto itStart = childrens.begin();
    std::advance(itStart, startInfo.spanIndex_);
    auto saveStartSpan = (start == startInfo.spanStart_) ? 0 : 1;
    if (saveStartSpan) {
        auto spanNodeStart = DynamicCast<SpanNode>(*itStart);
        CHECK_NULL_VOID(spanNodeStart);
        auto spanItemStart = spanNodeStart->GetSpanItem();
        auto beforStr = StringUtils::ToWstring(spanItemStart->content).substr(0, start - startInfo.spanStart_);
        auto strStart = StringUtils::ToString(beforStr);
        spanNodeStart->UpdateContent(strStart);
    }
    auto itEnd = childrens.begin();
    std::advance(itEnd, endInfo.spanIndex_);
    auto delEndSpan = (end == endInfo.spanEnd_) ? 1 : 0;
    if (!delEndSpan) {
        auto spanNodeEnd = DynamicCast<SpanNode>(*itEnd);
        CHECK_NULL_VOID(spanNodeEnd);
        auto spanItemEnd = spanNodeEnd->GetSpanItem();
        auto endStr =
            StringUtils::ToWstring(spanItemEnd->content).substr(end - endInfo.spanStart_, endInfo.spanEnd_ - end);
        auto strEnd = StringUtils::ToString(endStr);
        spanNodeEnd->UpdateContent(strEnd);
    }
    auto startIter = childrens.begin();
    std::advance(startIter, startInfo.spanIndex_ + saveStartSpan);
    auto endIter = childrens.begin();
    std::advance(endIter, endInfo.spanIndex_);
    for (auto iter = startIter; iter != endIter; ++iter) {
        ClearContent(*iter);
        host->RemoveChild(*iter);
    }
    if (delEndSpan) {
        ClearContent(*endIter);
        host->RemoveChild(*endIter);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

std::u16string RichEditorPattern::GetLeftTextOfCursor(int32_t number)
{
    if (number > caretPosition_) {
        number = caretPosition_;
    }
    auto start = caretPosition_;
    if (IsSelected()) {
        start = std::min(textSelector_.GetStart(), textSelector_.GetEnd());
    }
    auto stringText = GetSelectedText(start - number, start);
    return StringUtils::Str8ToStr16(stringText);
}

std::u16string RichEditorPattern::GetRightTextOfCursor(int32_t number)
{
    auto end = caretPosition_;
    if (IsSelected()) {
        end = std::max(textSelector_.GetStart(), textSelector_.GetEnd());
    }
    auto stringText = GetSelectedText(end, end + number);
    return StringUtils::Str8ToStr16(stringText);
}

int32_t RichEditorPattern::GetTextIndexAtCursor()
{
    return caretPosition_;
}

void RichEditorPattern::ClearContent(const RefPtr<UINode>& child)
{
    CHECK_NULL_VOID(child);
    if (child->GetTag() == V2::SPAN_ETS_TAG) {
        auto spanNode = DynamicCast<SpanNode>(child);
        if (spanNode) {
            spanNode->UpdateContent("");
            spanNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }
}

SpanPositionInfo RichEditorPattern::GetSpanPositionInfo(int32_t position)
{
    SpanPositionInfo spanPositionInfo(-1, -1, -1, -1);
    CHECK_NULL_RETURN(!spans_.empty(), spanPositionInfo);
    position = std::clamp(position, 0, GetTextContentLength());
    // find the spanItem where the position is
    auto it = std::find_if(spans_.begin(), spans_.end(), [position](const RefPtr<SpanItem>& spanItem) {
        return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                   position) &&
               (position < spanItem->position);
    });
    // the position is at the end
    if (it == spans_.end()) {
        return spanPositionInfo;
    }

    spanPositionInfo.spanIndex_ = std::distance(spans_.begin(), it);
    auto contentLen = StringUtils::ToWstring((*it)->content).length();
    spanPositionInfo.spanStart_ = (*it)->position - contentLen;
    spanPositionInfo.spanEnd_ = (*it)->position;
    spanPositionInfo.spanOffset_ = position - spanPositionInfo.spanStart_;
    return spanPositionInfo;
}

void RichEditorPattern::CopyTextSpanStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target)
{
    CHECK_NULL_VOID(source);
    CHECK_NULL_VOID(target);

    if (source->HasFontSize()) {
        target->UpdateFontSize(source->GetFontSizeValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }

    if (source->HasTextColor()) {
        target->UpdateTextColor(source->GetTextColorValue(Color::BLACK));
        target->AddPropertyInfo(PropertyInfo::FONTCOLOR);
    }

    if (source->HasItalicFontStyle()) {
        target->UpdateItalicFontStyle(source->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
        target->AddPropertyInfo(PropertyInfo::FONTSTYLE);
    }

    if (source->HasFontWeight()) {
        target->UpdateFontWeight(source->GetFontWeightValue(FontWeight::NORMAL));
        target->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
    }

    if (source->HasFontFamily()) {
        target->UpdateFontFamily(source->GetFontFamilyValue({ "HarmonyOS Sans" }));
        target->AddPropertyInfo(PropertyInfo::FONTFAMILY);
    }

    if (source->HasTextDecoration()) {
        target->UpdateTextDecoration(source->GetTextDecorationValue(TextDecoration::NONE));
        target->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
    }

    if (source->HasTextDecorationColor()) {
        target->UpdateTextDecorationColor(source->GetTextDecorationColorValue(Color::BLACK));
        target->AddPropertyInfo(PropertyInfo::NONE);
    }

    if (source->HasTextCase()) {
        target->UpdateTextCase(source->GetTextCaseValue(TextCase::NORMAL));
        target->AddPropertyInfo(PropertyInfo::TEXTCASE);
    }

    if (source->HasLetterSpacing()) {
        target->UpdateLetterSpacing(source->GetLetterSpacingValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::LETTERSPACE);
    }

    if (source->HasLineHeight()) {
        target->UpdateLineHeight(source->GetLineHeightValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::LINEHEIGHT);
    }
}

int32_t RichEditorPattern::TextSpanSplit(int32_t position)
{
    int32_t spanIndex = 0;
    int32_t spanStart = 0;
    int32_t spanOffset = 0;

    if (spans_.empty()) {
        return -1;
    }

    auto positionInfo = GetSpanPositionInfo(position);
    spanIndex = positionInfo.spanIndex_;
    spanStart = positionInfo.spanStart_;
    spanOffset = positionInfo.spanOffset_;

    if (spanOffset == 0 || spanOffset == -1) {
        return spanIndex;
    }

    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);
    auto it = host->GetChildren().begin();
    std::advance(it, spanIndex);

    auto spanNode = DynamicCast<SpanNode>(*it);
    CHECK_NULL_RETURN(spanNode, -1);
    auto spanItem = spanNode->GetSpanItem();
    auto newContent = StringUtils::ToWstring(spanItem->content).substr(spanOffset);
    auto deleteContent = StringUtils::ToWstring(spanItem->content).substr(0, spanOffset);

    auto* stack = ViewStackProcessor::GetInstance();
    CHECK_NULL_RETURN(stack, -1);
    auto nodeId = stack->ClaimNodeId();
    auto newSpanNode = SpanNode::GetOrCreateSpanNode(nodeId);
    CHECK_NULL_RETURN(newSpanNode, -1);

    auto newSpanItem = newSpanNode->GetSpanItem();
    newSpanItem->position = spanStart + spanOffset;
    auto spanIter = spans_.begin();
    std::advance(spanIter, spanIndex);
    spans_.insert(spanIter, newSpanItem);

    spanNode->UpdateContent(StringUtils::ToString(newContent));
    newSpanNode->UpdateContent(StringUtils::ToString(deleteContent));

    CopyTextSpanStyle(spanNode, newSpanNode);
    newSpanNode->MountToParent(host, spanIndex);

    return spanIndex + 1;
}

int32_t RichEditorPattern::GetTextContentLength()
{
    if (!spans_.empty()) {
        auto it = spans_.rbegin();
        return (*it)->position;
    }
    return 0;
}

int32_t RichEditorPattern::GetCaretPosition()
{
    return caretPosition_;
}

bool RichEditorPattern::SetCaretOffset(int32_t caretPosition)
{
    bool success = false;
    success = SetCaretPosition(caretPosition);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    if (focusHub->IsCurrentFocus()) {
        StartTwinkling();
    }
    CloseSelectOverlay();
    ResetSelection();
    return success;
}

OffsetF RichEditorPattern::CalcCursorOffsetByPosition(int32_t position, float& selectLineHeight, bool downStreamFirst)
{
    selectLineHeight = 0.0f;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, OffsetF(0, 0));
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, OffsetF(0, 0));
    auto rootOffset = pipeline->GetRootRect().GetOffset();
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0f, std::min(baselineOffset_, 0.0f));
    auto startOffset = paragraphs_.ComputeCursorOffset(position, selectLineHeight, downStreamFirst);
    auto children = host->GetChildren();
    if (NearZero(selectLineHeight)) {
        if (children.empty() || GetTextContentLength() == 0) {
            CHECK_NULL_RETURN(overlayMod_, OffsetF(0, 0));
            float caretHeight = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretHeight();
            return textPaintOffset - rootOffset - OffsetF(0.0f, caretHeight / 2.0f);
        }
        if (std::all_of(children.begin(), children.end(), [](RefPtr<UINode>& node) {
                CHECK_NULL_RETURN(node, false);
                return (node->GetTag() == V2::IMAGE_ETS_TAG || node->GetTag() == V2::PLACEHOLDER_SPAN_ETS_TAG);
            })) {
            bool isTail = false;
            auto it = children.begin();
            if (position >= static_cast<int32_t>(children.size())) {
                std::advance(it, (static_cast<int32_t>(children.size()) - 1));
                isTail = true;
            } else {
                std::advance(it, position);
            }
            if (it == children.end()) {
                return startOffset;
            }
            auto imageNode = DynamicCast<FrameNode>(*it);
            if (imageNode) {
                auto geometryNode = imageNode->GetGeometryNode();
                CHECK_NULL_RETURN(geometryNode, OffsetF(0.0f, 0.0f));
                startOffset = geometryNode->GetMarginFrameOffset();
                selectLineHeight = geometryNode->GetMarginFrameSize().Height();
                startOffset += isTail ? OffsetF(geometryNode->GetMarginFrameSize().Width(), 0.0f) : OffsetF(0.0f, 0.0f);
            }
            return startOffset;
        }
    }
    auto caretOffset = startOffset + textPaintOffset + rootOffset;
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, caretOffset);
    auto frameSize = geometryNode->GetFrameRect().GetSize();
    CHECK_NULL_RETURN(overlayMod_, caretOffset);
    float caretWidth = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretWidth();
    caretOffset.SetX(std::clamp(caretOffset.GetX(), 0.0f, static_cast<float>(frameSize.Width()) - caretWidth));
    return caretOffset;
}

bool RichEditorPattern::SetCaretPosition(int32_t pos)
{
    auto lastCaretPosition = caretPosition_;
    caretPosition_ = std::clamp(pos, 0, GetTextContentLength());
    ResetLastClickOffset();
    if (caretPosition_ == pos) {
        return true;
    }
    caretPosition_ = lastCaretPosition;
    return false;
}

bool RichEditorPattern::GetCaretVisible() const
{
    return caretVisible_;
}

void RichEditorPattern::SetUpdateSpanStyle(struct UpdateSpanStyle updateSpanStyle)
{
    updateSpanStyle_ = updateSpanStyle;
}

void RichEditorPattern::SetTypingStyle(struct UpdateSpanStyle typingStyle, TextStyle textStyle)
{
    typingStyle_ = typingStyle;
    typingTextStyle_ = textStyle;
}

void RichEditorPattern::UpdateTextStyle(
    RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (updateSpanStyle.updateTextColor.has_value()) {
        spanNode->UpdateTextColor(textStyle.GetTextColor());
        spanNode->AddPropertyInfo(PropertyInfo::FONTCOLOR);
    }
    if (updateSpanStyle.updateFontSize.has_value()) {
        spanNode->UpdateFontSize(textStyle.GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }
    if (updateSpanStyle.updateItalicFontStyle.has_value()) {
        spanNode->UpdateItalicFontStyle(textStyle.GetFontStyle());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSTYLE);
    }
    if (updateSpanStyle.updateFontWeight.has_value()) {
        spanNode->UpdateFontWeight(textStyle.GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
    }
    if (updateSpanStyle.updateFontFamily.has_value()) {
        spanNode->UpdateFontFamily(textStyle.GetFontFamilies());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFAMILY);
    }
    if (updateSpanStyle.updateTextDecoration.has_value()) {
        spanNode->UpdateTextDecoration(textStyle.GetTextDecoration());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
    }
    if (updateSpanStyle.updateTextDecorationColor.has_value()) {
        spanNode->UpdateTextDecorationColor(textStyle.GetTextDecorationColor());
        spanNode->AddPropertyInfo(PropertyInfo::NONE);
    }
    if (updateSpanStyle.updateTextShadows.has_value()) {
        spanNode->UpdateTextShadow(textStyle.GetTextShadows());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTSHADOW);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

bool RichEditorPattern::HasSameTypingStyle(const RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_RETURN(spanItem, false);
    auto spanTextStyle = spanItem->GetTextStyle();
    if (spanTextStyle.has_value() && typingTextStyle_.has_value()) {
        return spanTextStyle.value() == typingTextStyle_.value();
    } else {
        return !(spanTextStyle.has_value() || typingTextStyle_.has_value());
    }
}

void RichEditorPattern::UpdateImageStyle(RefPtr<FrameNode>& imageNode, const ImageSpanAttribute& imageStyle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    if (updateSpanStyle_.updateImageWidth.has_value() || updateSpanStyle_.updateImageHeight.has_value()) {
        imageLayoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(imageStyle.size.value().width), CalcLength(imageStyle.size.value().height)));
    }
    if (updateSpanStyle_.updateImageFit.has_value()) {
        imageLayoutProperty->UpdateImageFit(imageStyle.objectFit.value());
    }
    if (updateSpanStyle_.updateImageVerticalAlign.has_value()) {
        imageLayoutProperty->UpdateVerticalAlign(imageStyle.verticalAlign.value());
    }
    if (updateSpanStyle_.borderRadius.has_value()) {
        auto imageRenderCtx = imageNode->GetRenderContext();
        imageRenderCtx->UpdateBorderRadius(imageStyle.borderRadius.value());
        imageRenderCtx->SetClipToBounds(true);
    }
    if (updateSpanStyle_.marginProp.has_value()) {
        imageLayoutProperty->UpdateMargin(imageStyle.marginProp.value());
    }

    imageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

void RichEditorPattern::UpdateSpanStyle(
    int32_t start, int32_t end, const TextStyle& textStyle, const ImageSpanAttribute& imageStyle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t spanStart = 0;
    int32_t spanEnd = 0;
    for (auto it = host->GetChildren().begin(); it != host->GetChildren().end(); ++it) {
        auto spanNode = DynamicCast<SpanNode>(*it);
        auto imageNode = DynamicCast<FrameNode>(*it);
        if (!spanNode) {
            if (spanEnd != 0) {
                spanStart = spanEnd;
            }
            spanEnd = spanStart + 1;
        } else {
            auto spanItem = spanNode->GetSpanItem();
            spanItem->GetIndex(spanStart, spanEnd);
        }
        if (spanEnd < start) {
            continue;
        }

        if (spanStart >= start && spanEnd <= end) {
            if (spanNode) {
                UpdateTextStyle(spanNode, updateSpanStyle_, textStyle);
            } else {
                UpdateImageStyle(imageNode, imageStyle);
            }
            if (spanEnd == end) {
                break;
            }
            continue;
        }
        if (spanStart < start && start < spanEnd) {
            TextSpanSplit(start);
            --it;
            continue;
        }
        if (spanStart < end && end < spanEnd) {
            TextSpanSplit(end);
            --(--it);
            continue;
        }
        if (spanStart >= end) {
            break;
        }
    }

    // Custom menus do not actively close.
    if (!(SelectOverlayIsOn() && selectOverlayProxy_->GetSelectOverlayMangerInfo().menuInfo.menuBuilder != nullptr)) {
        CloseSelectOverlay();
        ResetSelection();
    }
}

std::vector<ParagraphInfo> RichEditorPattern::GetParagraphInfo(int32_t start, int32_t end)
{
    std::vector<ParagraphInfo> res;
    auto spanNodes = GetParagraphNodes(start, end);

    auto&& firstSpan = spanNodes.front()->GetSpanItem();
    auto paraStart = firstSpan->position - StringUtils::ToWstring(firstSpan->content).length();

    for (auto it = spanNodes.begin(); it != spanNodes.end(); ++it) {
        if (it == std::prev(spanNodes.end()) || StringUtils::ToWstring((*it)->GetSpanItem()->content).back() == L'\n') {
            ParagraphInfo info;
            auto lm = (*it)->GetLeadingMarginValue({});

            res.emplace_back(ParagraphInfo {
                .leadingMarginPixmap = lm.pixmap,
                .leadingMarginSize = { Dimension(lm.size.Width()).ConvertToVp(),
                    Dimension(lm.size.Height()).ConvertToVp() },
                .textAlign = static_cast<int32_t>((*it)->GetTextAlignValue(TextAlign::START)),
                .range = { paraStart, (*it)->GetSpanItem()->position },
            });
            paraStart = (*it)->GetSpanItem()->position;
        }
    }

    return res;
}

int32_t RichEditorPattern::GetParagraphLength(const std::list<RefPtr<UINode>>& spans) const
{
    if (spans.empty()) {
        return 0;
    }
    int32_t imageSpanCnt = 0;
    for (auto it = spans.rbegin(); it != spans.rend(); ++it) {
        auto spanNode = DynamicCast<SpanNode>(*it);
        if (spanNode) {
            return spanNode->GetSpanItem()->position + imageSpanCnt;
        }
        ++imageSpanCnt;
    }
    return imageSpanCnt;
}

std::vector<RefPtr<SpanNode>> RichEditorPattern::GetParagraphNodes(int32_t start, int32_t end) const
{
    CHECK_NULL_RETURN(start != end, {});
    auto host = GetHost();
    CHECK_NULL_RETURN(host, {});
    CHECK_NULL_RETURN(!host->GetChildren().empty(), {});

    const auto& spans = host->GetChildren();
    int32_t length = GetParagraphLength(spans);
    std::vector<RefPtr<SpanNode>> res;

    // start >= all content
    if (start >= length) {
        for (const auto& span : spans) {
            auto spanNode = DynamicCast<SpanNode>(span);
            if (spanNode) {
                res.emplace_back(spanNode);
            }
        }
        return res;
    }

    auto headIt = spans.begin();
    auto flagNode = headIt;
    bool isEnd = false;
    int32_t spanEnd = -1;
    while (flagNode != spans.end()) {
        auto spanNode = DynamicCast<SpanNode>(*flagNode);
        if (spanNode) {
            auto&& info = spanNode->GetSpanItem();
            spanEnd = info->position;
            isEnd = StringUtils::ToWstring(info->content).back() == '\n';
        } else {
            ++spanEnd;
            isEnd = false;
        }
        flagNode++;
        if (spanEnd > start) {
            break;
        }
        if (isEnd) {
            headIt = flagNode;
        }
    }
    while (headIt != flagNode) {
        auto spanNode = DynamicCast<SpanNode>(*headIt);
        if (spanNode) {
            res.emplace_back(spanNode);
        }
        headIt++;
    }
    while (flagNode != spans.end() && (spanEnd < end || !isEnd)) {
        auto spanNode = DynamicCast<SpanNode>(*flagNode);
        if (spanNode) {
            res.emplace_back(spanNode);
            auto&& info = spanNode->GetSpanItem();
            spanEnd = info->position;
            isEnd = StringUtils::ToWstring(info->content).back() == '\n';
        } else {
            ++spanEnd;
            isEnd = false;
        }
        flagNode++;
    }

    return res;
}

void RichEditorPattern::UpdateParagraphStyle(int32_t start, int32_t end, const struct UpdateParagraphStyle& style)
{
    auto spanNodes = GetParagraphNodes(start, end);
    for (const auto& spanNode : spanNodes) {
        if (style.textAlign.has_value()) {
            spanNode->UpdateTextAlign(*style.textAlign);
        }
        if (style.leadingMargin.has_value()) {
            spanNode->GetSpanItem()->leadingMargin = *style.leadingMargin;
            spanNode->UpdateLeadingMargin(*style.leadingMargin);
        }
    }
}

void RichEditorPattern::ScheduleCaretTwinkling()
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    if (!context->GetTaskExecutor()) {
        return;
    }

    auto weak = WeakClaim(this);
    caretTwinklingTask_.Reset([weak] {
        auto client = weak.Upgrade();
        CHECK_NULL_VOID(client);
        client->OnCaretTwinkling();
    });
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostDelayedTask(caretTwinklingTask_, TaskExecutor::TaskType::UI, RICH_EDITOR_TWINKLING_INTERVAL_MS);
}

void RichEditorPattern::StartTwinkling()
{
    caretTwinklingTask_.Cancel();
    caretVisible_ = true;
    auto tmpHost = GetHost();
    CHECK_NULL_VOID(tmpHost);
    tmpHost->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    ScheduleCaretTwinkling();
}

void RichEditorPattern::OnCaretTwinkling()
{
    caretTwinklingTask_.Cancel();
    caretVisible_ = !caretVisible_;
    GetHost()->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    ScheduleCaretTwinkling();
}

void RichEditorPattern::StopTwinkling()
{
    caretTwinklingTask_.Cancel();
    if (caretVisible_) {
        caretVisible_ = false;
        GetHost()->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void RichEditorPattern::HandleClickEvent(GestureEvent& info)
{
    if (hasClicked_) {
        hasClicked_ = false;
        TimeStamp clickTimeStamp = info.GetTimeStamp();
        std::chrono::duration<float, std::ratio<1, InputAIChecker::SECONDS_TO_MILLISECONDS>> timeout =
            clickTimeStamp - lastClickTimeStamp_;
        lastClickTimeStamp_ = info.GetTimeStamp();
        if (timeout.count() < DOUBLE_CLICK_INTERVAL_MS) {
            HandleDoubleClickEvent(info);
            return;
        }
    }
    HandleSingleClickEvent(info);
}

void RichEditorPattern::HandleSingleClickEvent(OHOS::Ace::GestureEvent& info)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "in handleSingleClickEvent");
    hasClicked_ = true;
    lastClickTimeStamp_ = info.GetTimeStamp();

    HandleUserClickEvent(info);
    if (textSelector_.IsValid() && !isMouseSelect_) {
        CloseSelectOverlay();
        ResetSelection();
    }

    caretUpdateType_ = CaretUpdateType::PRESSED;
    UseHostToUpdateTextFieldManager();

    auto textRect = GetTextRect();
    textRect.SetTop(textRect.GetY() - std::min(baselineOffset_, 0.0f));
    textRect.SetHeight(textRect.Height() - std::max(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - textRect.GetX(),
        info.GetLocalLocation().GetY() - textRect.GetY() };

    auto position = paragraphs_.GetIndex(textOffset);
    AdjustCursorPosition(position);

    auto focusHub = GetHost()->GetOrCreateFocusHub();
    if (focusHub) {
        if (focusHub->RequestFocusImmediately()) {
            float caretHeight = 0.0f;
            SetCaretPosition(position);
            OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
            MoveCaretToContentRect();
            CHECK_NULL_VOID(overlayMod_);
            DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(caretOffset, caretHeight);
            StartTwinkling();
            if (overlayMod_) {
                RequestKeyboard(false, true, true);
            }
        }
    }
    UseHostToUpdateTextFieldManager();
    CalcCaretInfoByClick(info);
}

void RichEditorPattern::HandleDoubleClickEvent(OHOS::Ace::GestureEvent& info)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "in double HandleDoubleClickEvent");
    if (!IsUsingMouse()) {
        caretUpdateType_ = CaretUpdateType::DOUBLE_CLICK;
        HandleDoubleClickOrLongPress(info);
    }
}

bool RichEditorPattern::HandleUserGestureEvent(
    GestureEvent& info, std::function<bool(RefPtr<SpanItem> item, GestureEvent& info)>&& gestureFunc)
{
    RectF textContentRect = contentRect_;
    textContentRect.SetTop(contentRect_.GetY() - std::min(baselineOffset_, 0.0f));
    textContentRect.SetHeight(contentRect_.Height() - std::max(baselineOffset_, 0.0f));
    if (!textContentRect.IsInRegion(PointF(info.GetLocalLocation().GetX(), info.GetLocalLocation().GetY())) ||
        spans_.empty()) {
        return false;
    }
    PointF textOffset = { info.GetLocalLocation().GetX() - GetTextRect().GetX(),
        info.GetLocalLocation().GetY() - GetTextRect().GetY() };
    int32_t start = 0;
    bool isParagraphHead = true;
    for (const auto& item : spans_) {
        if (!item) {
            continue;
        }
        std::vector<RectF> selectedRects = paragraphs_.GetRects(start, item->position);
        start = item->position;
        if (isParagraphHead && !selectedRects.empty() && item->leadingMargin.has_value()) {
            auto addWidth = item->leadingMargin.value().size.Width();
            selectedRects[0].SetLeft(selectedRects[0].GetX() - addWidth);
            selectedRects[0].SetWidth(selectedRects[0].GetSize().Width() + addWidth);
            isParagraphHead = false;
        } else if (!isParagraphHead && item->content.back() == '\n') {
            isParagraphHead = true;
        }
        for (auto&& rect : selectedRects) {
            if (!rect.IsInRegion(textOffset)) {
                continue;
            }
            return gestureFunc(item, info);
        }
    }
    return false;
}

bool RichEditorPattern::HandleUserClickEvent(GestureEvent& info)
{
    auto clickFunc = [](RefPtr<SpanItem> item, GestureEvent& info) -> bool {
        if (item && item->onClick) {
            item->onClick(info);
            return true;
        }
        return false;
    };
    return HandleUserGestureEvent(info, std::move(clickFunc));
}

void RichEditorPattern::CalcCaretInfoByClick(GestureEvent& info)
{
    auto textRect = GetTextRect();
    auto touchOffset = info.GetLocalLocation();
    textRect.SetTop(textRect.GetY() - std::min(baselineOffset_, 0.0f));
    textRect.SetHeight(textRect.Height() - std::max(baselineOffset_, 0.0f));
    Offset textOffset = { touchOffset.GetX() - textRect.GetX(), touchOffset.GetY() - textRect.GetY() };
    // get the caret position
    auto position = paragraphs_.GetIndex(textOffset);
    // get the caret offset when click
    float selectLineHeight = 0.0f;
    auto lastClickOffset = paragraphs_.ComputeCursorInfoByClick(position, selectLineHeight,
        OffsetF(static_cast<float>(textOffset.GetX()), static_cast<float>(textOffset.GetY())));

    lastClickOffset.AddX(textRect.GetX());
    lastClickOffset.AddY(textRect.GetY());

    SetCaretPosition(position);
    CHECK_NULL_VOID(overlayMod_);
    DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(lastClickOffset, selectLineHeight);
    SetLastClickOffset(lastClickOffset);
}

void RichEditorPattern::InitClickEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!clickEventInitialized_);
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleClickEvent(info);
    };
    auto clickListener = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gestureHub->AddClickEvent(clickListener);
    clickEventInitialized_ = true;
}

void RichEditorPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    CHECK_NULL_VOID(!focusEventInitialized_);
    auto focusTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleFocusEvent();
    };
    focusHub->SetOnFocusInternal(focusTask);
    auto blurTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent();
    };
    focusHub->SetOnBlurInternal(blurTask);
    focusEventInitialized_ = true;
    auto keyTask = [weak = WeakClaim(this)](const KeyEvent& keyEvent) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnKeyEvent(keyEvent);
    };
    focusHub->SetOnKeyEventInternal(std::move(keyTask));
}

void RichEditorPattern::HandleBlurEvent()
{
    StopTwinkling();
    if (textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
}

void RichEditorPattern::HandleFocusEvent()
{
    UseHostToUpdateTextFieldManager();
    StartTwinkling();
    if (!usingMouseRightButton_ && !isLongPress_) {
        RequestKeyboard(false, true, true);
    }
}

void RichEditorPattern::UseHostToUpdateTextFieldManager()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto globalOffset = host->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    UpdateTextFieldManager(Offset(globalOffset.GetX(), globalOffset.GetY()), frameRect_.Height());
}

void RichEditorPattern::OnVisibleChange(bool isVisible)
{
    TextPattern::OnVisibleChange(isVisible);
    StopTwinkling();
    CloseKeyboard(true);
}

bool RichEditorPattern::CloseKeyboard(bool forceClose)
{
    if (forceClose) {
        if (customKeyboardBuilder_ && isCustomKeyboardAttached_) {
            return CloseCustomKeyboard();
        }
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "Request close soft keyboard.");
#if defined(ENABLE_STANDARD_INPUT)
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        if (!imeAttached_) {
            return false;
        }
#endif
        auto inputMethod = MiscServices::InputMethodController::GetInstance();
        CHECK_NULL_RETURN(inputMethod, false);
        inputMethod->HideTextInput();
        inputMethod->Close();
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        imeAttached_ = false;
#endif
#else
        if (HasConnection()) {
            connection_->Close(GetInstanceId());
            connection_ = nullptr;
        }
#endif
        return true;
    }
    return false;
}

bool RichEditorPattern::JudgeDraggable(GestureEvent& info)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(hub, false);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    if (BetweenSelectedPosition(info.GetGlobalLocation())) {
        dragBoxes_ = GetTextBoxes();
        // prevent long press event from being triggered when dragging
#ifdef ENABLE_DRAG_FRAMEWORK
        gestureHub->SetIsTextDraggable(true);
#endif
        return true;
    }
#ifdef ENABLE_DRAG_FRAMEWORK
    gestureHub->SetIsTextDraggable(false);
#endif
    return false;
}

void RichEditorPattern::HandleLongPress(GestureEvent& info)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "handle long press!");
    caretUpdateType_ = CaretUpdateType::LONG_PRESSED;
    HandleDoubleClickOrLongPress(info);
}

void RichEditorPattern::HandleDoubleClickOrLongPress(GestureEvent& info)
{
    HandleUserLongPressEvent(info);
    if (JudgeDraggable(info)) {
        return;
    }
    if (isMousePressed_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    isLongPress_ = true;
    if (!focusHub->IsCurrentFocus()) {
        CalcCaretInfoByClick(info);
        focusHub->RequestFocusImmediately();
        return;
    }
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
        info.GetLocalLocation().GetY() - textPaintOffset.GetY() };
    InitSelection(textOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    if (!BetweenSelectedPosition(info.GetGlobalLocation()) && caretUpdateType_ == CaretUpdateType::LONG_PRESSED) {
        if (selectStart == 0) {
            textSelector_.Update(selectStart, selectStart);
        } else {
            textSelector_.Update(selectEnd, selectEnd);
            selectStart = selectEnd;
        }
    }
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    UpdateSelectionType(textSelectInfo);
    CalculateHandleOffsetAndShowOverlay();
    CloseSelectOverlay();
    selectionMenuOffset_ = info.GetGlobalLocation();
    ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    if (!textSelectInfo.GetSelection().resultObjects.empty()) {
        eventHub->FireOnSelect(&textSelectInfo);
    }
    SetCaretPosition(std::min(selectEnd, GetTextContentLength()));
    focusHub->RequestFocusImmediately();
    if (overlayMod_) {
        RequestKeyboard(false, true, true);
    }
    StopTwinkling();
}

bool RichEditorPattern::HandleUserLongPressEvent(GestureEvent& info)
{
    auto longPressFunc = [](RefPtr<SpanItem> item, GestureEvent& info) -> bool {
        if (item && item->onLongPress) {
            item->onLongPress(info);
            return true;
        }
        return false;
    };
    return HandleUserGestureEvent(info, std::move(longPressFunc));
}

void RichEditorPattern::HandleOnSelectAll()
{
    auto textSize = static_cast<int32_t>(GetWideText().length()) + imageCount_;
    textSelector_.Update(0, textSize);
    CalculateHandleOffsetAndShowOverlay();
    CloseSelectOverlay();
    auto responseType = selectOverlayProxy_
                            ? static_cast<RichEditorResponseType>(
                                  selectOverlayProxy_->GetSelectOverlayMangerInfo().menuInfo.responseType.value_or(0))
                            : RichEditorResponseType::LONG_PRESS;
    ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle, true, responseType);
    selectMenuInfo_.showCopyAll = false;
    selectOverlayProxy_->UpdateSelectMenuInfo(selectMenuInfo_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    FireOnSelect(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    SetCaretPosition(textSize);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!longPressEvent_);
    auto longPressCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleLongPress(info);
    };
    longPressEvent_ = MakeRefPtr<LongPressEvent>(std::move(longPressCallback));
    gestureHub->SetLongPressEvent(longPressEvent_);

    auto onTextSelectorChange = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto frameNode = pattern->GetHost();
        CHECK_NULL_VOID(frameNode);
        frameNode->OnAccessibilityEvent(AccessibilityEventType::TEXT_SELECTION_UPDATE);
    };
    textSelector_.SetOnAccessibility(std::move(onTextSelectorChange));
}

#ifdef ENABLE_DRAG_FRAMEWORK
void RichEditorPattern::InitDragDropEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->InitDragDropEvent();
    gestureHub->SetThumbnailCallback(GetThumbnailCallback());
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto onDragStart = [weakPtr = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& event,
                           const std::string& extraParams) -> NG::DragDropInfo {
        NG::DragDropInfo itemInfo;
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_RETURN(pattern, itemInfo);
        pattern->timestamp_ = std::chrono::system_clock::now().time_since_epoch().count();
        auto eventHub = pattern->GetEventHub<RichEditorEventHub>();
        eventHub->SetTimestamp(pattern->GetTimestamp());
        CHECK_NULL_RETURN(eventHub, itemInfo);
        return pattern->OnDragStart(event);
    };
    eventHub->SetOnDragStart(std::move(onDragStart));
    auto onDragMove = [weakPtr = WeakClaim(this)](
                          const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) {
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnDragMove(event);
    };
    eventHub->SetOnDragMove(std::move(onDragMove));
    auto onDragEnd = [weakPtr = WeakClaim(this), scopeId = Container::CurrentId()](
                         const RefPtr<OHOS::Ace::DragEvent>& event) {
        ContainerScope scope(scopeId);
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnDragEnd();
    };
    eventHub->SetOnDragEnd(std::move(onDragEnd));
    // OnDragMove will only execute if OnDrop is set.
    auto onDragDrop = [weakPtr = WeakClaim(this), scopeId = Container::CurrentId()](
                          const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& value) {};
    eventHub->SetOnDrop(std::move(onDragDrop));
    auto onDragDragLeave = [weakPtr = WeakClaim(this), scopeId = Container::CurrentId()](
                               const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& value) {
        ContainerScope scope(scopeId);
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->StopAutoScroll();
    };
    eventHub->SetOnDragLeave(onDragDragLeave);
}

NG::DragDropInfo RichEditorPattern::OnDragStart(const RefPtr<OHOS::Ace::DragEvent>& event)
{
    NG::DragDropInfo itemInfo;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, itemInfo);
    auto selectStart = textSelector_.GetTextStart();
    auto selectEnd = textSelector_.GetTextEnd();
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    dragResultObjects_ = textSelectInfo.GetSelection().resultObjects;
    if (dragResultObjects_.empty()) {
        return itemInfo;
    }
    RefPtr<UnifiedData> unifiedData = UdmfClient::GetInstance()->CreateUnifiedData();
    auto resultProcessor = [unifiedData, weak = WeakClaim(this)](const ResultObject& result) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (result.type == RichEditorSpanType::TYPESPAN) {
            auto data = pattern->GetSelectedSpanText(StringUtils::ToWstring(result.valueString),
                result.offsetInSpan[RichEditorSpanRange::RANGESTART],
                result.offsetInSpan[RichEditorSpanRange::RANGEEND]);
            UdmfClient::GetInstance()->AddPlainTextRecord(unifiedData, data);
            return;
        }
        if (result.type == RichEditorSpanType::TYPEIMAGE) {
            if (result.valuePixelMap) {
                const uint8_t* pixels = result.valuePixelMap->GetPixels();
                CHECK_NULL_VOID(pixels);
                int32_t length = result.valuePixelMap->GetByteCount();
                std::vector<uint8_t> data(pixels, pixels + length);
                PixelMapRecordDetails details = { result.valuePixelMap->GetWidth(), result.valuePixelMap->GetHeight(),
                    result.valuePixelMap->GetPixelFormat(), result.valuePixelMap->GetAlphaType() };
                UdmfClient::GetInstance()->AddPixelMapRecord(unifiedData, data, details);
            } else {
                UdmfClient::GetInstance()->AddImageRecord(unifiedData, result.valueString);
            }
        }
    };
    for (const auto& resultObj : dragResultObjects_) {
        resultProcessor(resultObj);
    }
    event->SetData(unifiedData);

    AceEngineExt::GetInstance().DragStartExt();

    StopTwinkling();
    CloseKeyboard(true);
    CloseSelectOverlay();
    ResetSelection();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return itemInfo;
}

void RichEditorPattern::OnDragEnd()
{
    StopAutoScroll();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (dragResultObjects_.empty()) {
        return;
    }
    UpdateSpanItemDragStatus(dragResultObjects_, false);
    dragResultObjects_.clear();
    StartTwinkling();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void RichEditorPattern::OnDragMove(const RefPtr<OHOS::Ace::DragEvent>& event)
{
    auto focusHub = GetHost()->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    auto touchX = event->GetX();
    auto touchY = event->GetY();
    auto textRect = GetTextRect();
    textRect.SetTop(textRect.GetY() - std::min(baselineOffset_, 0.0f));
    Offset textOffset = { touchX - textRect.GetX() - GetParentGlobalOffset().GetX(),
        touchY - textRect.GetY() - GetParentGlobalOffset().GetY() };
    auto position = paragraphs_.GetIndex(textOffset);
    float caretHeight = 0.0f;
    SetCaretPosition(position);
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    CHECK_NULL_VOID(overlayMod_);
    DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(caretOffset, caretHeight);
    StartTwinkling();

    AutoScrollParam param = { .autoScrollEvent = AutoScrollEvent::DRAG, .showScrollbar = true };
    auto localOffset = OffsetF(touchX, touchY) - parentGlobalOffset_;
    AutoScrollByEdgeDetection(param, localOffset, EdgeDetectionStrategy::IN_BOUNDARY);
}

void RichEditorPattern::UpdateSpanItemDragStatus(const std::list<ResultObject>& resultObjects, bool isDragging)
{
    if (resultObjects.empty()) {
        return;
    }
    auto dragStatusUpdateAction = [weakPtr = WeakClaim(this), isDragging](const ResultObject& resultObj) {
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto it = pattern->spans_.begin();
        std::advance(it, resultObj.spanPosition.spanIndex);
        auto spanItem = *it;
        CHECK_NULL_VOID(spanItem);
        if (resultObj.type == RichEditorSpanType::TYPESPAN) {
            if (isDragging) {
                spanItem->StartDrag(resultObj.offsetInSpan[RichEditorSpanRange::RANGESTART],
                    resultObj.offsetInSpan[RichEditorSpanRange::RANGEEND]);
            } else {
                spanItem->EndDrag();
            }
            return;
        }

        if (resultObj.type == RichEditorSpanType::TYPEIMAGE) {
            auto imageNode = DynamicCast<FrameNode>(pattern->GetChildByIndex(resultObj.spanPosition.spanIndex));
            CHECK_NULL_VOID(imageNode);
            auto renderContext = imageNode->GetRenderContext();
            CHECK_NULL_VOID(renderContext);
            renderContext->UpdateOpacity(isDragging ? (double)DRAGGED_TEXT_OPACITY / 255 : 1);
            imageNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
    };
    for (const auto& resultObj : resultObjects) {
        dragStatusUpdateAction(resultObj);
    }
}
#endif // ENABLE_DRAG_FRAMEWORK

bool RichEditorPattern::SelectOverlayIsOn()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    CHECK_NULL_RETURN(selectOverlayProxy_, false);
    auto overlayId = selectOverlayProxy_->GetSelectOverlayId();
    return pipeline->GetSelectOverlayManager()->HasSelectOverlay(overlayId);
}

void RichEditorPattern::UpdateEditingValue(const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent)
{
    InsertValue(value->text);
}

void RichEditorPattern::PerformAction(TextInputAction action, bool forceCloseKeyboard)
{
    InsertValue("\n");
}

void RichEditorPattern::InitMouseEvent()
{
    CHECK_NULL_VOID(!mouseEventInitialized_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleMouseEvent(info);
    };
    auto mouseEvent = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent);
    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->OnHover(isHover);
        }
    };
    auto hoverEvent = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent);
    mouseEventInitialized_ = true;
}

void RichEditorPattern::OnHover(bool isHover)
{
    auto frame = GetHost();
    CHECK_NULL_VOID(frame);
    auto frameId = frame->GetId();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    if (isHover) {
        pipeline->SetMouseStyleHoldNode(frameId);
        pipeline->ChangeMouseStyle(frameId, MouseFormat::TEXT_CURSOR);
    } else {
        pipeline->ChangeMouseStyle(frameId, MouseFormat::DEFAULT);
        pipeline->FreeMouseStyleHoldNode(frameId);
    }
}

bool RichEditorPattern::RequestKeyboard(bool isFocusViewChanged, bool needStartTwinkling, bool needShowSoftKeyboard)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    CHECK_NULL_RETURN(needShowSoftKeyboard, false);
    if (needShowSoftKeyboard && customKeyboardBuilder_) {
        return RequestCustomKeyboard();
    }
#if defined(ENABLE_STANDARD_INPUT)
    if (!EnableStandardInput(needShowSoftKeyboard)) {
        return false;
    }
#else
    if (!UnableStandardInput(isFocusViewChanged)) {
        return false;
    }
#endif
    return true;
}

#if defined(ENABLE_STANDARD_INPUT)
bool RichEditorPattern::EnableStandardInput(bool needShowSoftKeyboard)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    MiscServices::Configuration configuration;
    configuration.SetEnterKeyType(static_cast<MiscServices::EnterKeyType>(static_cast<int32_t>(TextInputAction::DONE)));
    configuration.SetTextInputType(
        static_cast<MiscServices::TextInputType>(static_cast<int32_t>(TextInputType::UNSPECIFIED)));
    MiscServices::InputMethodController::GetInstance()->OnConfigurationChange(configuration);
    if (richEditTextChangeListener_ == nullptr) {
        richEditTextChangeListener_ = new OnTextChangedListenerImpl(WeakClaim(this));
    }
    auto inputMethod = MiscServices::InputMethodController::GetInstance();
    CHECK_NULL_RETURN(inputMethod, false);
    auto miscTextConfig = GetMiscTextConfig();
    CHECK_NULL_RETURN(miscTextConfig.has_value(), false);
    TAG_LOGI(
        AceLogTag::ACE_RICH_TEXT, "RequestKeyboard set calling window id is : %{public}u", miscTextConfig->windowId);
    inputMethod->Attach(richEditTextChangeListener_, needShowSoftKeyboard, miscTextConfig.value());
    if (context) {
        inputMethod->SetCallingWindow(context->GetWindowId());
    }
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    imeAttached_ = true;
#endif
    return true;
}

std::optional<MiscServices::TextConfig> RichEditorPattern::GetMiscTextConfig()
{
    auto pipeline = GetHost()->GetContext();
    CHECK_NULL_RETURN(pipeline, {});
    auto windowRect = pipeline->GetCurrentWindowRect();
    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    if (NearZero(caretHeight)) {
        auto overlayModifier = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
        caretHeight = overlayModifier ? overlayModifier->GetCaretHeight() : DEFAULT_CARET_HEIGHT;
    }
    MiscServices::CursorInfo cursorInfo { .left = caretOffset.GetX() + windowRect.Left() + parentGlobalOffset_.GetX(),
        .top = caretOffset.GetY() + windowRect.Top() + parentGlobalOffset_.GetY(),
        .width = CARET_WIDTH,
        .height = caretHeight };
    MiscServices::InputAttribute inputAttribute = { .inputPattern = (int32_t)TextInputType::UNSPECIFIED,
        .enterKeyType = (int32_t)TextInputAction::DONE };
    MiscServices::TextConfig textConfig = { .inputAttribute = inputAttribute,
        .cursorInfo = cursorInfo,
        .range = { .start = textSelector_.GetStart(), .end = textSelector_.GetEnd() },
        .windowId = pipeline->GetFocusWindowId(),
        .positionY = (GetHost()->GetPaintRectOffset() - pipeline->GetRootRect().GetOffset()).GetY(),
        .height = frameRect_.Height() };
    return textConfig;
}
#else
bool RichEditorPattern::UnableStandardInput(bool isFocusViewChanged)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    if (HasConnection()) {
        connection_->Show(isFocusViewChanged, GetInstanceId());
        return true;
    }
    TextInputConfiguration config;
    config.type = TextInputType::UNSPECIFIED;
    config.action = TextInputAction::DONE;
    config.obscureText = false;
    connection_ =
        TextInputProxy::GetInstance().Attach(WeakClaim(this), config, context->GetTaskExecutor(), GetInstanceId());
    if (!HasConnection()) {
        return false;
    }
    TextEditingValue value;
    if (spans_.empty()) {
        value.text = textForDisplay_;
    } else {
        for (auto it = spans_.begin(); it != spans_.end(); it++) {
            if ((*it)->placeholderIndex < 0) {
                value.text.append((*it)->content);
            } else {
                value.text.append(" ");
            }
        }
    }
    value.selection.Update(caretPosition_, caretPosition_);
    connection_->SetEditingState(value, GetInstanceId());
    connection_->Show(isFocusViewChanged, GetInstanceId());
    return true;
}
#endif

void RichEditorPattern::UpdateCaretInfoToController()
{
    CHECK_NULL_VOID(HasFocus());
    auto selectionResult = GetSpansInfo(0, GetTextContentLength(), GetSpansMethod::ONSELECT);
    auto resultObjects = selectionResult.GetSelection().resultObjects;
    std::string text = "";
    if (!resultObjects.empty()) {
        for (const auto& resultObj : resultObjects) {
            if (resultObj.type == RichEditorSpanType::TYPESPAN) {
                text += resultObj.valueString;
            }
        }
    }
#if defined(ENABLE_STANDARD_INPUT)
    auto miscTextConfig = GetMiscTextConfig();
    CHECK_NULL_VOID(miscTextConfig.has_value());
    MiscServices::CursorInfo cursorInfo = miscTextConfig.value().cursorInfo;
    MiscServices::InputMethodController::GetInstance()->OnCursorUpdate(cursorInfo);
    MiscServices::InputMethodController::GetInstance()->OnSelectionChange(
        StringUtils::Str8ToStr16(text), textSelector_.GetStart(), textSelector_.GetEnd());
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT,
        "Caret position update, left %{public}f, top %{public}f, width %{public}f, height %{public}f; textSelector_ "
        "Start "
        "%{public}d, end %{public}d",
        cursorInfo.left, cursorInfo.top, cursorInfo.width, cursorInfo.height, textSelector_.GetStart(),
        textSelector_.GetEnd());
#else
    if (HasConnection()) {
        TextEditingValue editingValue;
        editingValue.text = text;
        editingValue.hint = "";
        editingValue.selection.Update(textSelector_.baseOffset, textSelector_.destinationOffset);
        connection_->SetEditingState(editingValue, GetInstanceId());
    }
#endif
}

bool RichEditorPattern::HasConnection() const
{
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    return imeAttached_;
#else
    return connection_ != nullptr;
#endif
}

bool RichEditorPattern::RequestCustomKeyboard()
{
    if (isCustomKeyboardAttached_) {
        return true;
    }
    CHECK_NULL_RETURN(customKeyboardBuilder_, false);
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, false);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, false);
    overlayManager->BindKeyboard(customKeyboardBuilder_, frameNode->GetId());
    isCustomKeyboardAttached_ = true;
    return true;
}

bool RichEditorPattern::CloseCustomKeyboard()
{
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, false);

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, false);
    overlayManager->CloseKeyboard(frameNode->GetId());
    isCustomKeyboardAttached_ = false;
    return true;
}

void RichEditorPattern::InsertValue(const std::string& insertValue)
{
    if (SystemProperties::GetDebugEnabled()) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "Insert value '%{public}s'", insertValue.c_str());
    }
    bool isSelector = false;
    if (textSelector_.IsValid()) {
        SetCaretPosition(textSelector_.GetTextStart());
        isSelector = true;
    }

    std::string insertValueTemp = insertValue;
    bool isLineSeparator = false;
    if (insertValueTemp == std::string("\n")) {
        isLineSeparator = true;
    }

    auto isInsert = BeforeIMEInsertValue(insertValueTemp);
    CHECK_NULL_VOID(isInsert);
    TextInsertValueInfo info;
    CalcInsertValueObj(info);
    if (isSelector) {
        DeleteForward(textSelector_.GetTextEnd() - textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    RefPtr<SpanNode> spanNode = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex()));
    if (spanNode == nullptr && info.GetSpanIndex() == 0) {
        CreateTextSpanNode(spanNode, info, insertValueTemp);
        return;
    }
    if (spanNode == nullptr && info.GetSpanIndex() != 0) {
        auto spanNodeBefore = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex() - 1));
        if (spanNodeBefore == nullptr) {
            CreateTextSpanNode(spanNode, info, insertValueTemp);
            return;
        }
        if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
            CreateTextSpanNode(spanNode, info, insertValueTemp);
            return;
        }
        auto spanNodeGet = InsertValueToBeforeSpan(spanNodeBefore, insertValueTemp);
        bool isCreate = spanNodeBefore->GetId() != spanNodeGet->GetId();
        AfterIMEInsertValue(
            spanNodeGet, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), isCreate);
        return;
    }
    if (info.GetOffsetInSpan() == 0) {
        auto spanNodeBefore = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex() - 1));
        if (spanNodeBefore != nullptr && !IsLineSeparatorInLast(spanNodeBefore)) {
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
                CreateTextSpanNode(spanNode, info, insertValueTemp);
                return;
            }
            auto spanNodeGet = InsertValueToBeforeSpan(spanNodeBefore, insertValueTemp);
            bool isCreate = spanNodeBefore->GetId() != spanNodeGet->GetId();
            AfterIMEInsertValue(
                spanNodeGet, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), isCreate);
            return;
        }
    }
    if (typingStyle_.has_value() && !HasSameTypingStyle(spanNode)) {
        TextSpanOptions options;
        options.value = insertValueTemp;
        options.offset = caretPosition_;
        options.style = typingTextStyle_;
        AddTextSpan(options);
        AfterIMEInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), true);
        return;
    }
    if (!isLineSeparator) {
        InsertValueToSpanNode(spanNode, insertValueTemp, info);
    } else {
        SpanNodeFission(spanNode, insertValueTemp, info);
    }
    AfterIMEInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), false);
}

bool RichEditorPattern::IsLineSeparatorInLast(RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        if (textAfter.empty()) {
            return true;
        }
    }
    return false;
}

void RichEditorPattern::InsertValueToSpanNode(
    RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_VOID(spanItem);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.insert(info.GetOffsetInSpan(), insertValueTemp);
    text = StringUtils::ToString(textTemp);
    spanNode->UpdateContent(text);
    spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
}

void RichEditorPattern::SpanNodeFission(
    RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_VOID(spanItem);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.insert(info.GetOffsetInSpan(), insertValueTemp);

    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        text = StringUtils::ToString(textBefore);
        spanNode->UpdateContent(text);
        spanItem->position += 1 - static_cast<int32_t>(textAfter.length());
        if (!textAfter.empty()) {
            TextInsertValueInfo infoAfter;
            infoAfter.SetSpanIndex(info.GetSpanIndex() + 1);
            infoAfter.SetOffsetInSpan(0);
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
            RefPtr<SpanNode> spanNodeAfter = SpanNode::GetOrCreateSpanNode(nodeId);
            spanNodeAfter->MountToParent(host, infoAfter.GetSpanIndex());
            spanNodeAfter->UpdateContent(StringUtils::ToString(textAfter));
            CopyTextSpanStyle(spanNode, spanNodeAfter);
        }
    } else {
        text = StringUtils::ToString(textTemp);
        spanNode->UpdateContent(text);
        spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    }
}

RefPtr<SpanNode> RichEditorPattern::InsertValueToBeforeSpan(
    RefPtr<SpanNode>& spanNodeBefore, const std::string& insertValue)
{
    auto spanItem = spanNodeBefore->GetSpanItem();
    CHECK_NULL_RETURN(spanItem, spanNodeBefore);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.append(insertValueTemp);

    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        text = StringUtils::ToString(textBefore);
        spanNodeBefore->UpdateContent(text);
        spanItem->position += 1 - static_cast<int32_t>(textAfter.length());
        if (!textAfter.empty()) {
            auto host = GetHost();
            CHECK_NULL_RETURN(spanItem, spanNodeBefore);
            TextInsertValueInfo infoAfter;
            infoAfter.SetSpanIndex(host->GetChildIndex(spanNodeBefore) + 1);
            infoAfter.SetOffsetInSpan(0);
            auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
            RefPtr<SpanNode> spanNodeAfter = SpanNode::GetOrCreateSpanNode(nodeId);
            spanNodeAfter->MountToParent(host, infoAfter.GetSpanIndex());
            spanNodeAfter->UpdateContent(StringUtils::ToString(textAfter));
            CopyTextSpanStyle(spanNodeBefore, spanNodeAfter);
            return spanNodeAfter;
        }
    } else {
        text = StringUtils::ToString(textTemp);
        spanNodeBefore->UpdateContent(text);
        spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    }
    return spanNodeBefore;
}

void RichEditorPattern::CreateTextSpanNode(
    RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    spanNode = SpanNode::GetOrCreateSpanNode(nodeId);
    spanNode->MountToParent(host, info.GetSpanIndex());
    auto spanItem = spanNode->GetSpanItem();
    AddSpanItem(spanItem, info.GetSpanIndex());
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        UpdateTextStyle(spanNode, typingStyle_.value(), typingTextStyle_.value());
        auto spanItem = spanNode->GetSpanItem();
        spanItem->SetTextStyle(typingTextStyle_);
    } else {
        spanNode->UpdateFontSize(Dimension(DEFAULT_TEXT_SIZE, DimensionUnit::FP));
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }
    spanNode->UpdateContent(insertValue);
    if (isIME) {
        AfterIMEInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValue).length()), true);
    }
}

bool RichEditorPattern::BeforeIMEInsertValue(const std::string& insertValue)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    RichEditorInsertValue insertValueInfo;
    insertValueInfo.SetInsertOffset(caretPosition_);
    insertValueInfo.SetInsertValue(insertValue);
    return eventHub->FireAboutToIMEInput(insertValueInfo);
}

void RichEditorPattern::AfterIMEInsertValue(const RefPtr<SpanNode>& spanNode, int32_t insertValueLength, bool isCreate)
{
    RichEditorAbstractSpanResult retInfo;
    isTextChange_ = true;
    moveDirection_ = MoveDirection::FORWARD;
    moveLength_ += insertValueLength;
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    retInfo.SetSpanIndex(host->GetChildIndex(spanNode));
    retInfo.SetEraseLength(insertValueLength);
    retInfo.SetValue(spanNode->GetSpanItem()->content);
    auto contentLength = StringUtils::ToWstring(spanNode->GetSpanItem()->content).length();
    if (isCreate) {
        auto spanStart = 0;
        auto spanEnd = static_cast<int32_t>(contentLength);
        retInfo.SetSpanRangeStart(spanStart);
        retInfo.SetSpanRangeEnd(spanEnd);
        retInfo.SetOffsetInSpan(0);
    } else {
        auto spanEnd = spanNode->GetSpanItem()->position;
        auto spanStart = spanEnd - static_cast<int32_t>(contentLength);
        retInfo.SetSpanRangeStart(spanStart);
        retInfo.SetSpanRangeEnd(spanEnd);
        retInfo.SetOffsetInSpan(GetCaretPosition() - retInfo.GetSpanRangeStart());
    }
    retInfo.SetFontColor(spanNode->GetTextColorValue(Color::BLACK).ColorToString());
    retInfo.SetFontSize(spanNode->GetFontSizeValue(Dimension(16.0f, DimensionUnit::VP)).ConvertToVp());
    retInfo.SetFontStyle(spanNode->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
    retInfo.SetFontWeight(static_cast<int32_t>(spanNode->GetFontWeightValue(FontWeight::NORMAL)));
    std::string fontFamilyValue;
    auto fontFamily = spanNode->GetFontFamilyValue({ "HarmonyOS Sans" });
    for (const auto& str : fontFamily) {
        fontFamilyValue += str;
    }
    retInfo.SetFontFamily(fontFamilyValue);
    retInfo.SetTextDecoration(spanNode->GetTextDecorationValue(TextDecoration::NONE));
    retInfo.SetColor(spanNode->GetTextDecorationColorValue(Color::BLACK).ColorToString());
    eventHub->FireOnIMEInputComplete(retInfo);
    int32_t spanTextLength = 0;
    for (auto& span : spans_) {
        spanTextLength += StringUtils::ToWstring(span->content).length();
        span->position = spanTextLength;
    }
}

void RichEditorPattern::ResetFirstNodeStyle()
{
    auto tmpHost = GetHost();
    CHECK_NULL_VOID(tmpHost);
    auto spans = tmpHost->GetChildren();
    if (!spans.empty()) {
        auto&& firstNode = DynamicCast<SpanNode>(*(spans.begin()));
        if (firstNode) {
            firstNode->ResetTextAlign();
            firstNode->ResetLeadingMargin();
            tmpHost->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }
}

void RichEditorPattern::FireOnDeleteComplete(const RichEditorDeleteValue& info)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto isDelete = eventHub->FireAboutToDelete(info);
    if (isDelete) {
        DeleteByDeleteValueInfo(info);
        eventHub->FireOnDeleteComplete();
    }
}

void RichEditorPattern::DeleteBackward(int32_t length)
{
    if (textSelector_.IsValid()) {
        length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        SetCaretPosition(textSelector_.GetTextEnd());
        CloseSelectOverlay();
        ResetSelection();
    }
    RichEditorDeleteValue info;
    info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::BACKWARD);
    if (caretPosition_ == 0) {
        info.SetLength(0);
        ResetFirstNodeStyle();
        FireOnDeleteComplete(info);
        return;
    }
    if (length == spans_.back()->position) {
        ResetFirstNodeStyle();
        textForDisplay_.clear();
    }
    info.SetOffset(caretPosition_ - 1);
    info.SetLength(length);
    int32_t currentPosition = std::clamp((caretPosition_ - length), 0, static_cast<int32_t>(GetTextContentLength()));
    if (!spans_.empty()) {
        CalcDeleteValueObj(currentPosition, length, info);
        FireOnDeleteComplete(info);
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
}

void RichEditorPattern::DeleteForward(int32_t length)
{
    if (textSelector_.IsValid()) {
        length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        SetCaretPosition(textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    if (caretPosition_ == GetTextContentLength()) {
        return;
    }
    RichEditorDeleteValue info;
    info.SetOffset(caretPosition_);
    info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::FORWARD);
    info.SetLength(length);
    int32_t currentPosition = caretPosition_;
    if (!spans_.empty()) {
        CalcDeleteValueObj(currentPosition, length, info);
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto isDelete = eventHub->FireAboutToDelete(info);
        if (isDelete) {
            DeleteByDeleteValueInfo(info);
            eventHub->FireOnDeleteComplete();
        }
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
}

bool RichEditorPattern::OnBackPressed()
{
    auto tmpHost = GetHost();
    CHECK_NULL_RETURN(tmpHost, false);
    TAG_LOGI(AceLogTag::ACE_TEXT_FIELD, "RichEditor %{public}d receives back press event", tmpHost->GetId());
    if (SelectOverlayIsOn()) {
        CloseSelectOverlay();
        textSelector_.Update(textSelector_.destinationOffset);
        StartTwinkling();
        return true;
    }
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    if (!imeShown_ && !isCustomKeyboardAttached_) {
#else
    if (!isCustomKeyboardAttached_) {
#endif
        return false;
    }
    tmpHost->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    CloseKeyboard(true);
#if defined(ANDROID_PLATFORM)
    return false;
#else
    return true;
#endif
}

void RichEditorPattern::SetInputMethodStatus(bool keyboardShown)
{
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    imeShown_ = keyboardShown;
#endif
}

bool RichEditorPattern::CursorMoveLeft()
{
    CloseSelectOverlay();
    ResetSelection();
    auto caretPosition = std::clamp((caretPosition_ - 1), 0, static_cast<int32_t>(GetTextContentLength()));
    if (caretPosition_ == caretPosition) {
        return false;
    }
    SetCaretPosition(caretPosition);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveRight()
{
    CloseSelectOverlay();
    ResetSelection();
    auto caretPosition = std::clamp((caretPosition_ + 1), 0, static_cast<int32_t>(GetTextContentLength()));
    if (caretPosition_ == caretPosition) {
        return false;
    }
    SetCaretPosition(caretPosition);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveUp()
{
    CloseSelectOverlay();
    ResetSelection();
    if (static_cast<int32_t>(GetTextContentLength()) > 1) {
        float caretHeight = 0.0f;
        OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
        auto minDet = paragraphs_.minParagraphFontSize.value() / 2.0;
        int32_t caretPosition = paragraphs_.GetIndex(
            Offset(caretOffset.GetX() - GetTextRect().GetX(), caretOffset.GetY() - GetTextRect().GetY() - minDet));
        caretPosition = std::clamp(caretPosition, 0, static_cast<int32_t>(GetTextContentLength()));
        if (caretPosition_ == caretPosition) {
            return false;
        }
        SetCaretPosition(caretPosition);
        MoveCaretToContentRect();
    }
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveDown()
{
    CloseSelectOverlay();
    ResetSelection();
    if (static_cast<int32_t>(GetTextContentLength()) > 1) {
        float caretHeight = 0.0f;
        OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
        auto minDet = paragraphs_.minParagraphFontSize.value() / 2.0;
        int32_t caretPosition = paragraphs_.GetIndex(Offset(caretOffset.GetX() - GetTextRect().GetX(),
            caretOffset.GetY() - GetTextRect().GetY() + caretHeight + minDet / 2.0));
        caretPosition = std::clamp(caretPosition, 0, static_cast<int32_t>(GetTextContentLength()));
        if (caretPosition_ == caretPosition) {
            return false;
        }
        SetCaretPosition(caretPosition);
        MoveCaretToContentRect();
    }
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

void RichEditorPattern::CalcInsertValueObj(TextInsertValueInfo& info)
{
    if (spans_.empty()) {
        info.SetSpanIndex(0);
        info.SetOffsetInSpan(0);
        return;
    }
    auto it = std::find_if(
        spans_.begin(), spans_.end(), [caretPosition = caretPosition_ + moveLength_](const RefPtr<SpanItem>& spanItem) {
            return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                       caretPosition) &&
                   (caretPosition < spanItem->position);
        });
    info.SetSpanIndex(std::distance(spans_.begin(), it));
    if (it == spans_.end()) {
        info.SetOffsetInSpan(0);
        return;
    }
    info.SetOffsetInSpan(
        caretPosition_ + moveLength_ - ((*it)->position - StringUtils::ToWstring((*it)->content).length()));
}

void RichEditorPattern::CalcDeleteValueObj(int32_t currentPosition, int32_t length, RichEditorDeleteValue& info)
{
    auto it =
        std::find_if(spans_.begin(), spans_.end(), [caretPosition = currentPosition](const RefPtr<SpanItem>& spanItem) {
            return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                       caretPosition) &&
                   (caretPosition < spanItem->position);
        });
    while (it != spans_.end() && length > 0) {
        if ((*it)->placeholderIndex >= 0) {
            RichEditorAbstractSpanResult spanResult;
            spanResult.SetSpanIndex(std::distance(spans_.begin(), it));
            auto eraseLength = DeleteValueSetImageSpan(*it, spanResult);
            currentPosition += eraseLength;
            length -= eraseLength;
            info.SetRichEditorDeleteSpans(spanResult);
        } else {
            RichEditorAbstractSpanResult spanResult;
            spanResult.SetSpanIndex(std::distance(spans_.begin(), it));
            auto eraseLength = DeleteValueSetTextSpan(*it, currentPosition, length, spanResult);
            length -= eraseLength;
            currentPosition += eraseLength;
            info.SetRichEditorDeleteSpans(spanResult);
        }
        std::advance(it, 1);
    }
}

int32_t RichEditorPattern::DeleteValueSetImageSpan(
    const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::IMAGE);
    spanResult.SetSpanRangeEnd(spanItem->position);
    spanResult.SetSpanRangeStart(spanItem->position - 1);
    spanResult.SetEraseLength(1);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, IMAGE_SPAN_LENGTH);
    auto uiNode = host->GetChildAtIndex(spanResult.GetSpanIndex());
    CHECK_NULL_RETURN(uiNode, IMAGE_SPAN_LENGTH);
    auto imageNode = AceType::DynamicCast<FrameNode>(uiNode);
    CHECK_NULL_RETURN(imageNode, IMAGE_SPAN_LENGTH);
    auto geometryNode = imageNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, IMAGE_SPAN_LENGTH);
    auto imageLayoutProperty = DynamicCast<ImageLayoutProperty>(imageNode->GetLayoutProperty());
    CHECK_NULL_RETURN(imageLayoutProperty, IMAGE_SPAN_LENGTH);
    spanResult.SetSizeWidth(geometryNode->GetMarginFrameSize().Width());
    spanResult.SetSizeHeight(geometryNode->GetMarginFrameSize().Height());
    if (!imageLayoutProperty->GetImageSourceInfo()->GetPixmap()) {
        spanResult.SetValueResourceStr(imageLayoutProperty->GetImageSourceInfo()->GetSrc());
    } else {
        spanResult.SetValuePixelMap(imageLayoutProperty->GetImageSourceInfo()->GetPixmap());
    }
    if (imageLayoutProperty->HasImageFit()) {
        spanResult.SetImageFit(imageLayoutProperty->GetImageFitValue());
    }
    if (imageLayoutProperty->HasVerticalAlign()) {
        spanResult.SetVerticalAlign(imageLayoutProperty->GetVerticalAlignValue());
    }
    return IMAGE_SPAN_LENGTH;
}

int32_t RichEditorPattern::DeleteValueSetTextSpan(
    const RefPtr<SpanItem>& spanItem, int32_t currentPosition, int32_t length, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::TEXT);
    auto contentStartPosition = spanItem->position - StringUtils::ToWstring(spanItem->content).length();
    spanResult.SetSpanRangeStart(contentStartPosition);
    int32_t eraseLength = 0;
    if (spanItem->position - currentPosition >= length) {
        eraseLength = length;
    } else {
        eraseLength = spanItem->position - currentPosition;
    }
    spanResult.SetSpanRangeEnd(spanItem->position);
    spanResult.SetValue(spanItem->content);
    spanResult.SetOffsetInSpan(currentPosition - contentStartPosition);
    spanResult.SetEraseLength(eraseLength);
    return eraseLength;
}

void RichEditorPattern::DeleteByDeleteValueInfo(const RichEditorDeleteValue& info)
{
    auto deleteSpans = info.GetRichEditorDeleteSpans();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<UINode>> deleteNode;
    std::set<int32_t, std::greater<int32_t>> deleteNodes;
    for (const auto& it : deleteSpans) {
        switch (it.GetType()) {
            case SpanResultType::TEXT: {
                auto ui_node = host->GetChildAtIndex(it.GetSpanIndex());
                CHECK_NULL_VOID(ui_node);
                auto spanNode = DynamicCast<SpanNode>(ui_node);
                CHECK_NULL_VOID(spanNode);
                auto spanItem = spanNode->GetSpanItem();
                CHECK_NULL_VOID(spanItem);
                auto text = spanItem->content;
                std::wstring textTemp = StringUtils::ToWstring(text);
                textTemp.erase(it.OffsetInSpan(), it.GetEraseLength());
                if (textTemp.size() == 0) {
                    deleteNodes.emplace(it.GetSpanIndex());
                }
                text = StringUtils::ToString(textTemp);
                spanNode->UpdateContent(text);
                spanItem->position -= it.GetEraseLength();
                break;
            }
            case SpanResultType::IMAGE:
                deleteNodes.emplace(it.GetSpanIndex());
                break;
            default:
                break;
        }
    }
    for (auto index : deleteNodes) {
        host->RemoveChildAtIndex(index);
    }
    if (info.GetRichEditorDeleteDirection() == RichEditorDeleteDirection::BACKWARD) {
        SetCaretPosition(
            std::clamp(caretPosition_ - info.GetLength(), 0, static_cast<int32_t>(GetTextContentLength())));
    }
    int32_t spanTextLength = 0;
    for (auto& span : spans_) {
        span->position = spanTextLength + StringUtils::ToWstring(span->content).length();
        spanTextLength += StringUtils::ToWstring(span->content).length();
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    OnModifyDone();
}

bool RichEditorPattern::OnKeyEvent(const KeyEvent& keyEvent)
{
    if (keyEvent.action == KeyAction::DOWN) {
        if (keyEvent.code == KeyCode::KEY_TAB) {
            return false;
        }
        std::string appendElement;
        if (keyEvent.code == KeyCode::KEY_ENTER || keyEvent.code == KeyCode::KEY_NUMPAD_ENTER ||
            keyEvent.code == KeyCode::KEY_DPAD_CENTER) {
            InsertValue("\n");
            return true;
        } else if (HandleShiftPressedEvent(keyEvent)) {
            return true;
        } else if (keyEvent.IsDirectionalKey()) {
            HandleDirectionalKey(keyEvent);
            return true;
        } else if (keyEvent.IsNumberKey() && !keyEvent.IsCombinationKey()) {
            appendElement = keyEvent.ConvertCodeToString();
        } else if (keyEvent.IsLetterKey()) {
            if (keyEvent.IsKey({ KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_A }) ||
                keyEvent.IsKey({ KeyCode::KEY_CTRL_RIGHT, KeyCode::KEY_A })) {
                HandleOnSelectAll();
            } else if (keyEvent.IsKey({ KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_C }) ||
                       keyEvent.IsKey({ KeyCode::KEY_CTRL_RIGHT, KeyCode::KEY_C })) {
                HandleOnCopy();
            } else if (keyEvent.IsKey({ KeyCode::KEY_CTRL_LEFT, KeyCode::KEY_V }) ||
                       keyEvent.IsKey({ KeyCode::KEY_CTRL_RIGHT, KeyCode::KEY_V })) {
                HandleOnPaste();
            }
        }
        if (keyEvent.code == KeyCode::KEY_DEL) {
#if defined(PREVIEW)
            DeleteForward(1);
#else
            DeleteBackward(1);
#endif
            return true;
        }
        if (keyEvent.code == KeyCode::KEY_FORWARD_DEL) {
#if defined(PREVIEW)
            DeleteBackward(1);
#else
            DeleteForward(1);
#endif
            return true;
        }
        ParseAppendValue(keyEvent.code, appendElement);
        if (!appendElement.empty()) {
            InsertValue(appendElement);
            return true;
        }
    }
    return true;
}

bool RichEditorPattern::HandleShiftPressedEvent(const KeyEvent& event)
{
    const static size_t maxKeySizes = 2;
    wchar_t keyChar;

    auto iterCode = KEYBOARD_SYMBOL.find(event.code);
    if (event.pressedCodes.size() == 1 && iterCode != KEYBOARD_SYMBOL.end()) {
        if (iterCode != KEYBOARD_SYMBOL.end()) {
            keyChar = iterCode->second;
        } else {
            return false;
        }
    } else if (event.pressedCodes.size() == maxKeySizes && (event.pressedCodes[0] == KeyCode::KEY_SHIFT_LEFT ||
                                                               event.pressedCodes[0] == KeyCode::KEY_SHIFT_RIGHT)) {
        iterCode = SHIFT_KEYBOARD_SYMBOL.find(event.code);
        if (iterCode != SHIFT_KEYBOARD_SYMBOL.end()) {
            keyChar = iterCode->second;
        } else if (KeyCode::KEY_A <= event.code && event.code <= KeyCode::KEY_Z) {
            keyChar = static_cast<wchar_t>(event.code) - static_cast<wchar_t>(KeyCode::KEY_A) + UPPER_CASE_A;
        } else if (KeyCode::KEY_0 <= event.code && event.code <= KeyCode::KEY_9) {
            keyChar = NUM_SYMBOL[static_cast<int32_t>(event.code) - static_cast<int32_t>(KeyCode::KEY_0)];
        } else {
            return false;
        }
    } else {
        return false;
    }
    std::wstring appendElement(1, keyChar);
    InsertValue(StringUtils::ToString(appendElement));
    return true;
}

bool RichEditorPattern::HandleDirectionalKey(const KeyEvent& keyEvent)
{
    switch (keyEvent.code) {
        case KeyCode::KEY_DPAD_UP:
            return CursorMoveUp();
        case KeyCode::KEY_DPAD_DOWN:
            return CursorMoveDown();
        case KeyCode::KEY_DPAD_LEFT:
            return CursorMoveLeft();
        case KeyCode::KEY_DPAD_RIGHT:
            return CursorMoveRight();
        default:
            return false;
    }
    return false;
}

void RichEditorPattern::ParseAppendValue(KeyCode keyCode, std::string& appendElement)
{
    switch (keyCode) {
        case KeyCode::KEY_SPACE:
            appendElement = " ";
            break;
        default:
            break;
    }
}

void RichEditorPattern::MoveCaretAfterTextChange()
{
    CHECK_NULL_VOID(isTextChange_);
    isTextChange_ = false;
    switch (moveDirection_) {
        case MoveDirection::BACKWARD:
            SetCaretPosition(
                std::clamp((caretPosition_ - moveLength_), 0, static_cast<int32_t>(GetTextContentLength())));
            break;
        case MoveDirection::FORWARD:
            SetCaretPosition(
                std::clamp((caretPosition_ + moveLength_), 0, static_cast<int32_t>(GetTextContentLength())));
            break;
        default:
            break;
    }
    moveLength_ = 0;
}

void RichEditorPattern::InitTouchEvent()
{
    CHECK_NULL_VOID(!touchListener_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleTouchEvent(info);
    };
    touchListener_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gesture->AddTouchEvent(touchListener_);
}

void RichEditorPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    if (SelectOverlayIsOn()) {
        return;
    }
    auto touchType = info.GetTouches().front().GetTouchType();
    if (touchType == TouchType::DOWN) {
    } else if (touchType == TouchType::UP) {
        isMousePressed_ = false;
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    if (isLongPress_) {
        RequestKeyboard(false, true, true);
        isLongPress_ = false;
    }
#endif
    }
}

void RichEditorPattern::HandleMouseLeftButton(const MouseInfo& info)
{
    if (info.GetAction() == MouseAction::MOVE) {
        if (blockPress_ || !leftMousePress_) {
            return;
        }
        auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
        Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
            info.GetLocalLocation().GetY() - textPaintOffset.GetY() };

        mouseStatus_ = MouseStatus::MOVE;
        if (isFirstMouseSelect_) {
            int32_t extend = paragraphs_.GetIndex(textOffset);
            int32_t extendEnd = extend + GetGraphemeClusterLength(GetWideText(), extend);
            textSelector_.Update(extend, extendEnd);
            isFirstMouseSelect_ = false;
        } else {
            int32_t extend = paragraphs_.GetIndex(textOffset);
            textSelector_.Update(textSelector_.baseOffset, extend);
            SetCaretPosition(std::max(textSelector_.baseOffset, extend));
            AutoScrollParam param = {
                .autoScrollEvent = AutoScrollEvent::MOUSE, .showScrollbar = true, .eventOffset = info.GetLocalLocation()
            };
            AutoScrollByEdgeDetection(param, OffsetF(info.GetLocalLocation().GetX(), info.GetLocalLocation().GetY()),
                EdgeDetectionStrategy::OUT_BOUNDARY);
        }

        isMouseSelect_ = true;
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    } else if (info.GetAction() == MouseAction::PRESS) {
        isMousePressed_ = true;
        if (BetweenSelectedPosition(info.GetGlobalLocation())) {
            blockPress_ = true;
            return;
        }
        leftMousePress_ = true;
        mouseStatus_ = MouseStatus::PRESSED;
        blockPress_ = false;
    } else if (info.GetAction() == MouseAction::RELEASE) {
        blockPress_ = false;
        leftMousePress_ = false;
        auto oldMouseStatus = mouseStatus_;
        mouseStatus_ = MouseStatus::RELEASED;
        isMouseSelect_ = false;
        isMousePressed_ = false;
        isFirstMouseSelect_ = true;
        auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
        auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
        FireOnSelect(selectStart, selectEnd);
        StopAutoScroll();
        if (textSelector_.IsValid() && !textSelector_.StartEqualToDest() && IsSelectedBindSelectionMenu() &&
            oldMouseStatus == MouseStatus::MOVE) {
            selectionMenuOffsetByMouse_ = OffsetF(static_cast<float>(info.GetGlobalLocation().GetX()),
                static_cast<float>(info.GetGlobalLocation().GetY()));
            ShowSelectOverlay(RectF(), RectF(), false, RichEditorResponseType::SELECTED_BY_MOUSE);
        }
    }
}

void RichEditorPattern::HandleMouseRightButton(const MouseInfo& info)
{
    if (info.GetAction() == MouseAction::PRESS) {
        isMousePressed_ = true;
        usingMouseRightButton_ = true;
        CloseSelectionMenu();
    } else if (info.GetAction() == MouseAction::RELEASE) {
        selectionMenuOffsetByMouse_ = OffsetF(
            static_cast<float>(info.GetGlobalLocation().GetX()), static_cast<float>(info.GetGlobalLocation().GetY()));
        if (textSelector_.IsValid() && BetweenSelectedPosition(info.GetGlobalLocation())) {
            ShowSelectOverlay(RectF(), RectF(), IsSelectAll(), RichEditorResponseType::RIGHT_CLICK);
            isMousePressed_ = false;
            usingMouseRightButton_ = false;
            return;
        }
        if (textSelector_.IsValid()) {
            CloseSelectOverlay();
            ResetSelection();
        }
        MouseRightFocus(info);
        ShowSelectOverlay(RectF(), RectF(), IsSelectAll(), RichEditorResponseType::RIGHT_CLICK);
        isMousePressed_ = false;
        usingMouseRightButton_ = false;
    }
}

void RichEditorPattern::MouseRightFocus(const MouseInfo& info)
{
    auto textRect = GetTextRect();
    textRect.SetTop(textRect.GetY() - std::min(baselineOffset_, 0.0f));
    textRect.SetHeight(textRect.Height() - std::max(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - textRect.GetX(),
        info.GetLocalLocation().GetY() - textRect.GetY() };
    InitSelection(textOffset);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    SetCaretPosition(selectEnd);

    TextInsertValueInfo spanInfo;
    CalcInsertValueObj(spanInfo);
    auto spanNode = DynamicCast<FrameNode>(GetChildByIndex(spanInfo.GetSpanIndex() - 1));
    if (spanNode && spanNode->GetTag() == V2::IMAGE_ETS_TAG && spanInfo.GetOffsetInSpan() == 0 &&
        selectEnd == selectStart + 1 && BetweenSelectedPosition(info.GetGlobalLocation())) {
        selectedType_ = RichEditorType::IMAGE;
        FireOnSelect(selectStart, selectEnd);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        return;
    }
    if (textSelector_.IsValid()) {
        ResetSelection();
    }
    auto position = paragraphs_.GetIndex(textOffset);
    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    SetCaretPosition(position);
    selectedType_ = RichEditorType::TEXT;
    CHECK_NULL_VOID(overlayMod_);
    DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(caretOffset, caretHeight);
    StartTwinkling();
}

void RichEditorPattern::FireOnSelect(int32_t selectStart, int32_t selectEnd)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    if (!textSelectInfo.GetSelection().resultObjects.empty()) {
        eventHub->FireOnSelect(&textSelectInfo);
    }
    UpdateSelectionType(textSelectInfo);
}

void RichEditorPattern::HandleMouseEvent(const MouseInfo& info)
{
    caretUpdateType_ = CaretUpdateType::NONE;
    if (info.GetButton() == MouseButton::LEFT_BUTTON) {
        HandleMouseLeftButton(info);
    } else if (info.GetButton() == MouseButton::RIGHT_BUTTON) {
        HandleMouseRightButton(info);
    }
}

void RichEditorPattern::OnHandleMoveDone(const RectF& handleRect, bool isFirstHandle)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    if (!textSelectInfo.GetSelection().resultObjects.empty()) {
        eventHub->FireOnSelect(&textSelectInfo);
    }
    UpdateSelectionType(textSelectInfo);
    SetCaretPosition(selectEnd);
    CalculateHandleOffsetAndShowOverlay();
    StopAutoScroll();
    if (selectOverlayProxy_) {
        SelectHandleInfo handleInfo;
        if (isFirstHandle) {
            handleInfo.paintRect = textSelector_.firstHandle;
            selectOverlayProxy_->UpdateFirstSelectHandleInfo(handleInfo);
        } else {
            handleInfo.paintRect = textSelector_.secondHandle;
            selectOverlayProxy_->UpdateSecondSelectHandleInfo(handleInfo);
        }

        if (IsSelectAll() && selectMenuInfo_.showCopyAll == true) {
            selectMenuInfo_.showCopyAll = false;
            selectOverlayProxy_->UpdateSelectMenuInfo(selectMenuInfo_);
        } else if (!IsSelectAll() && selectMenuInfo_.showCopyAll == false) {
            selectMenuInfo_.showCopyAll = true;
            selectOverlayProxy_->UpdateSelectMenuInfo(selectMenuInfo_);
        }
        selectOverlayProxy_ = nullptr;
        ShowSelectOverlay(
            textSelector_.firstHandle, textSelector_.secondHandle, IsSelectAll(), RichEditorResponseType::LONG_PRESS);
        return;
    }
    ShowSelectOverlay(
        textSelector_.firstHandle, textSelector_.secondHandle, IsSelectAll(), RichEditorResponseType::LONG_PRESS);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

RefPtr<UINode> RichEditorPattern::GetChildByIndex(int32_t index) const
{
    auto host = GetHost();
    const auto& children = host->GetChildren();
    int32_t size = static_cast<int32_t>(children.size());
    if (index < 0 || index >= size) {
        return nullptr;
    }
    auto pos = children.begin();
    std::advance(pos, index);
    return *pos;
}

std::string RichEditorPattern::GetSelectedSpanText(std::wstring value, int32_t start, int32_t end) const
{
    if (start < 0 || end > static_cast<int32_t>(value.length()) || start >= end) {
        return "";
    }
    auto min = std::min(start, end);
    auto max = std::max(start, end);

    return StringUtils::ToString(value.substr(min, max - min));
}

TextStyleResult RichEditorPattern::GetTextStyleObject(const RefPtr<SpanNode>& node)
{
    TextStyleResult textStyle;
    textStyle.fontColor = node->GetTextColorValue(Color::BLACK).ColorToString();
    textStyle.fontSize = node->GetFontSizeValue(Dimension(16.0f, DimensionUnit::VP)).ConvertToVp();
    textStyle.fontStyle = static_cast<int32_t>(node->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
    textStyle.fontWeight = static_cast<int32_t>(node->GetFontWeightValue(FontWeight::NORMAL));
    std::string fontFamilyValue;
    const std::vector<std::string> defaultFontFamily = { "HarmonyOS Sans" };
    auto fontFamily = node->GetFontFamilyValue(defaultFontFamily);
    for (const auto& str : fontFamily) {
        fontFamilyValue += str;
        fontFamilyValue += ",";
    }
    fontFamilyValue = fontFamilyValue.substr(0, fontFamilyValue.size() - 1);
    textStyle.fontFamily = !fontFamilyValue.empty() ? fontFamilyValue : defaultFontFamily.front();
    textStyle.decorationType = static_cast<int32_t>(node->GetTextDecorationValue(TextDecoration::NONE));
    textStyle.decorationColor = node->GetTextDecorationColorValue(Color::BLACK).ColorToString();
    return textStyle;
}

ResultObject RichEditorPattern::GetTextResultObject(RefPtr<UINode> uinode, int32_t index, int32_t start, int32_t end)
{
    bool selectFlag = false;
    ResultObject resultObject;
    if (!DynamicCast<SpanNode>(uinode)) {
        return resultObject;
    }
    auto spanItem = DynamicCast<SpanNode>(uinode)->GetSpanItem();
    int32_t itemLength = StringUtils::ToWstring(spanItem->content).length();
    int32_t endPosition = std::min(GetTextContentLength(), spanItem->position);
    int32_t startPosition = endPosition - itemLength;

    if (startPosition >= start && endPosition <= end) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
    } else if (startPosition < start && endPosition <= end && endPosition > start) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = start - startPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
    } else if (startPosition >= start && startPosition < end && endPosition >= end) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = end - startPosition;
    } else if (startPosition <= start && endPosition >= end) {
        selectFlag = true;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = start - startPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = end - startPosition;
    }
    if (selectFlag == true) {
        resultObject.spanPosition.spanIndex = index;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGESTART] = startPosition;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGEEND] = endPosition;
        resultObject.type = RichEditorSpanType::TYPESPAN;
        resultObject.valueString = spanItem->content;
        auto spanNode = DynamicCast<SpanNode>(uinode);
        resultObject.textStyle = GetTextStyleObject(spanNode);
    }
    return resultObject;
}

ResultObject RichEditorPattern::GetImageResultObject(RefPtr<UINode> uinode, int32_t index, int32_t start, int32_t end)
{
    int32_t itemLength = 1;
    ResultObject resultObject;
    if (!DynamicCast<FrameNode>(uinode) || !GetSpanItemByIndex(index)) {
        return resultObject;
    }
    int32_t endPosition = std::min(GetTextContentLength(), GetSpanItemByIndex(index)->position);
    int32_t startPosition = endPosition - itemLength;
    if ((start <= startPosition) && (end >= endPosition)) {
        auto imageNode = DynamicCast<FrameNode>(uinode);
        auto imageLayoutProperty = DynamicCast<ImageLayoutProperty>(imageNode->GetLayoutProperty());
        resultObject.spanPosition.spanIndex = index;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGESTART] = startPosition;
        resultObject.spanPosition.spanRange[RichEditorSpanRange::RANGEEND] = endPosition;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGESTART] = 0;
        resultObject.offsetInSpan[RichEditorSpanRange::RANGEEND] = itemLength;
        resultObject.type = RichEditorSpanType::TYPEIMAGE;
        if (!imageLayoutProperty->GetImageSourceInfo()->GetPixmap()) {
            resultObject.valueString = imageLayoutProperty->GetImageSourceInfo()->GetSrc();
        } else {
            resultObject.valuePixelMap = imageLayoutProperty->GetImageSourceInfo()->GetPixmap();
        }
        auto geometryNode = imageNode->GetGeometryNode();
        resultObject.imageStyle.size[RichEditorImageSize::SIZEWIDTH] = geometryNode->GetMarginFrameSize().Width();
        resultObject.imageStyle.size[RichEditorImageSize::SIZEHEIGHT] = geometryNode->GetMarginFrameSize().Height();
        if (imageLayoutProperty->HasImageFit()) {
            resultObject.imageStyle.verticalAlign = static_cast<int32_t>(imageLayoutProperty->GetImageFitValue());
        }
        if (imageLayoutProperty->HasVerticalAlign()) {
            resultObject.imageStyle.objectFit = static_cast<int32_t>(imageLayoutProperty->GetVerticalAlignValue());
        }
    }
    return resultObject;
}

RichEditorSelection RichEditorPattern::GetSpansInfo(int32_t start, int32_t end, GetSpansMethod method)
{
    int32_t index = 0;
    std::int32_t realEnd = 0;
    std::int32_t realStart = 0;
    RichEditorSelection selection;
    std::list<ResultObject> resultObjects;
    auto length = GetTextContentLength();
    if (method == GetSpansMethod::GETSPANS) {
        realStart = (start == -1) ? 0 : start;
        realEnd = (end == -1) ? length : end;
        if (realStart > realEnd) {
            std::swap(realStart, realEnd);
        }
        realStart = std::max(0, realStart);
        realEnd = std::min(length, realEnd);
    } else if (method == GetSpansMethod::ONSELECT) {
        realEnd = std::min(length, end);
        realStart = std::min(length, start);
    }
    selection.SetSelectionEnd(realEnd);
    selection.SetSelectionStart(realStart);
    if (realStart > length || realEnd < 0 || spans_.empty() || (start > length && end > length)) {
        selection.SetResultObjectList(resultObjects);
        return selection;
    }
    auto host = GetHost();
    const auto& children = host->GetChildren();
    for (const auto& uinode : children) {
        if (uinode->GetTag() == V2::IMAGE_ETS_TAG) {
            ResultObject resultObject = GetImageResultObject(uinode, index, realStart, realEnd);
            if (!resultObject.valueString.empty() || resultObject.valuePixelMap) {
                resultObjects.emplace_back(resultObject);
            }
        } else if (uinode->GetTag() == V2::SPAN_ETS_TAG) {
            ResultObject resultObject = GetTextResultObject(uinode, index, realStart, realEnd);
            if (!resultObject.valueString.empty()) {
                resultObjects.emplace_back(resultObject);
            }
        }
        index++;
    }
    selection.SetResultObjectList(resultObjects);
    return selection;
}

bool RichEditorPattern::IsSelectedBindSelectionMenu()
{
    bool result = false;
    auto selectType = selectedType_.value_or(RichEditorType::TEXT);
    if (selectType == RichEditorType::TEXT) {
        result = GetMenuParams(RichEditorResponseType::SELECTED_BY_MOUSE, RichEditorType::TEXT) != nullptr;
    } else if (selectType == RichEditorType::IMAGE) {
        result = GetMenuParams(RichEditorResponseType::SELECTED_BY_MOUSE, RichEditorType::IMAGE) != nullptr;
    } else if (selectType == RichEditorType::MIXED) {
        result = GetMenuParams(RichEditorResponseType::SELECTED_BY_MOUSE, RichEditorType::MIXED) != nullptr;
    }
    return result;
}

void RichEditorPattern::CopySelectionMenuParams(SelectOverlayInfo& selectInfo, RichEditorResponseType responseType)
{
    auto selectType = selectedType_.value_or(RichEditorType::NONE);
    std::shared_ptr<SelectionMenuParams> menuParams = nullptr;
    if (selectType == RichEditorType::TEXT) {
        menuParams = GetMenuParams(responseType, RichEditorType::TEXT);
    } else if (selectType == RichEditorType::IMAGE) {
        menuParams = GetMenuParams(responseType, RichEditorType::IMAGE);
    } else if (selectType == RichEditorType::MIXED) {
        menuParams = GetMenuParams(responseType, RichEditorType::MIXED);
    }

    if (menuParams == nullptr) {
        return;
    }

    // long pressing on the image needs to set the position of the pop-up menu following the long pressing position
    if (selectType == RichEditorType::IMAGE && !selectInfo.isUsingMouse) {
        selectInfo.menuInfo.menuOffset = OffsetF(selectionMenuOffset_.GetX(), selectionMenuOffset_.GetY());
    }

    selectInfo.menuInfo.menuBuilder = menuParams->buildFunc;
    if (menuParams->onAppear) {
        auto weak = AceType::WeakClaim(this);
        auto callback = [weak, menuParams]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            CHECK_NULL_VOID(menuParams->onAppear);

            auto& textSelector = pattern->textSelector_;
            auto selectStart = std::min(textSelector.baseOffset, textSelector.destinationOffset);
            auto selectEnd = std::max(textSelector.baseOffset, textSelector.destinationOffset);
            menuParams->onAppear(selectStart, selectEnd);
        };
        selectInfo.menuCallback.onAppear = std::move(callback);
    }
    selectInfo.menuCallback.onDisappear = menuParams->onDisappear;
}

void RichEditorPattern::ShowSelectOverlay(
    const RectF& firstHandle, const RectF& secondHandle, bool isCopyAll, RichEditorResponseType responseType)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto hasDataCallback = [weak = WeakClaim(this), pipeline, firstHandle, secondHandle, isCopyAll, responseType](
                               bool hasData) mutable {
        auto pattern = weak.Upgrade();
        SelectOverlayInfo selectInfo;
        bool usingMouse = pattern->IsUsingMouse();
        if (!pattern->IsUsingMouse() && responseType == RichEditorResponseType::LONG_PRESS) {
            selectInfo.firstHandle.paintRect = firstHandle;
            selectInfo.secondHandle.paintRect = secondHandle;
        } else {
            if (responseType == RichEditorResponseType::LONG_PRESS) {
                responseType = RichEditorResponseType::RIGHT_CLICK;
            }
            selectInfo.isUsingMouse = true;
            selectInfo.rightClickOffset = pattern->GetSelectionMenuOffset();
            pattern->ResetIsMousePressed();
        }
        selectInfo.menuInfo.responseType = static_cast<int32_t>(responseType);
        selectInfo.onHandleMove = [weak](const RectF& handleRect, bool isFirst) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnHandleMove(handleRect, isFirst);
        };
        selectInfo.onHandleMoveDone = [weak](const RectF& handleRect, bool isFirst) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnHandleMoveDone(handleRect, isFirst);
        };

        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);

        pattern->UpdateSelectMenuInfo(hasData, selectInfo, isCopyAll);

        selectInfo.menuCallback.onCopy = [weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleOnCopy();
            pattern->CloseSelectOverlay();
            pattern->ResetSelection();
        };

        selectInfo.menuCallback.onCut = [weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleOnCut();
        };

        selectInfo.menuCallback.onPaste = [weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->HandleOnPaste();
        };
        selectInfo.menuCallback.onSelectAll = [weak, usingMouse]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->isMousePressed_ = usingMouse;
            pattern->HandleOnSelectAll();
        };

        selectInfo.callerFrameNode = host;
        pattern->CopySelectionMenuParams(selectInfo, responseType);
        pattern->UpdateSelectOverlayOrCreate(selectInfo);
    };
    CHECK_NULL_VOID(clipboard_);
    clipboard_->HasData(hasDataCallback);
}

void RichEditorPattern::HandleOnCopy()
{
    CHECK_NULL_VOID(clipboard_);
    auto selectStart = textSelector_.GetTextStart();
    auto selectEnd = textSelector_.GetTextEnd();
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    auto copyResultObjects = textSelectInfo.GetSelection().resultObjects;
    caretUpdateType_ = CaretUpdateType::NONE;
    if (copyResultObjects.empty()) {
        return;
    }
    RefPtr<PasteDataMix> pasteData = clipboard_->CreatePasteDataMix();
    auto resultProcessor = [weak = WeakClaim(this), pasteData, selectStart, selectEnd, clipboard = clipboard_](
                               const ResultObject& result) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (result.type == RichEditorSpanType::TYPESPAN) {
            auto data = pattern->GetSelectedSpanText(StringUtils::ToWstring(result.valueString),
                result.offsetInSpan[RichEditorSpanRange::RANGESTART],
                result.offsetInSpan[RichEditorSpanRange::RANGEEND]);
            clipboard->AddTextRecord(pasteData, data);
            return;
        }
        if (result.type == RichEditorSpanType::TYPEIMAGE) {
            if (result.valuePixelMap) {
                clipboard->AddPixelMapRecord(pasteData, result.valuePixelMap);
            } else {
                clipboard->AddImageRecord(pasteData, result.valueString);
            }
        }
    };
    for (auto resultObj = copyResultObjects.rbegin(); resultObj != copyResultObjects.rend(); ++resultObj) {
        resultProcessor(*resultObj);
    }
    clipboard_->SetData(pasteData, copyOption_);
    StartTwinkling();
}

void RichEditorPattern::ResetAfterPaste()
{
    SetCaretSpanIndex(-1);
    StartTwinkling();
    CloseSelectOverlay();
    InsertValueByPaste(GetPasteStr());
    ClearPasteStr();
    if (textSelector_.IsValid()) {
        SetCaretPosition(textSelector_.GetTextStart());
        auto length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        textSelector_.Update(-1, -1);
        DeleteForward(length);
        ResetSelection();
    }
}

void RichEditorPattern::HandleOnPaste()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    TextCommonEvent event;
    eventHub->FireOnPaste(event);
    if (event.IsPreventDefault()) {
        return;
    }
    CHECK_NULL_VOID(clipboard_);
    auto pasteCallback = [weak = WeakClaim(this)](const std::string& data) {
        if (data.empty()) {
            return;
        }
        auto richEditor = weak.Upgrade();
        CHECK_NULL_VOID(richEditor);
        richEditor->AddPasteStr(data);
        richEditor->ResetAfterPaste();
    };
    clipboard_->GetData(pasteCallback);
}

void RichEditorPattern::InsertValueByPaste(const std::string& insertValue)
{
    RefPtr<UINode> child;
    TextInsertValueInfo info;
    CalcInsertValueObj(info);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT,
        "InsertValueByPaste spanIndex: %{public}d,  offset inspan:  %{public}d, caretPosition: %{public}d",
        info.GetSpanIndex(), info.GetOffsetInSpan(), caretPosition_);
    TextSpanOptions options;
    options.value = insertValue;
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        options.style = typingTextStyle_.value();
    }
    auto newSpanOffset = caretPosition_ + moveLength_;
    isTextChange_ = true;
    moveDirection_ = MoveDirection::FORWARD;
    moveLength_ += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    if (caretSpanIndex_ == -1) {
        child = GetChildByIndex(info.GetSpanIndex());
        if (child && child->GetTag() == V2::SPAN_ETS_TAG) {
            auto spanNode = DynamicCast<SpanNode>(child);
            CHECK_NULL_VOID(spanNode);
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNode)) {
                options.offset = newSpanOffset;
                caretSpanIndex_ = AddTextSpan(options, true);
            } else {
                InsertValueToSpanNode(spanNode, insertValue, info);
            }
            return;
        } else if (!child) {
            auto spanNodeBefore = DynamicCast<SpanNode>(GetChildByIndex(info.GetSpanIndex() - 1));
            if (spanNodeBefore == nullptr) {
                caretSpanIndex_ = AddTextSpan(options, true);
                return;
            }
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
                auto spanNode = DynamicCast<SpanNode>(child);
                CreateTextSpanNode(spanNode, info, insertValue, false);
                caretSpanIndex_ = info.GetSpanIndex();
            } else {
                InsertValueToBeforeSpan(spanNodeBefore, insertValue);
                caretSpanIndex_ = info.GetSpanIndex() - 1;
            }
            return;
        }
    } else {
        child = GetChildByIndex(caretSpanIndex_);
        if (child && child->GetTag() == V2::SPAN_ETS_TAG) {
            auto spanNode = DynamicCast<SpanNode>(child);
            CHECK_NULL_VOID(spanNode);
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNode)) {
                options.offset = newSpanOffset;
                caretSpanIndex_ = AddTextSpan(options, true);
            } else {
                InsertValueToBeforeSpan(spanNode, insertValue);
            }
            return;
        }
    }
    if (child && child->GetTag() == V2::IMAGE_ETS_TAG) {
        auto spanNodeBefore = DynamicCast<SpanNode>(GetChildByIndex(info.GetSpanIndex() - 1));
        if (spanNodeBefore != nullptr && caretSpanIndex_ == -1) {
            if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
                options.offset = newSpanOffset;
                caretSpanIndex_ = AddTextSpan(options, true);
            } else {
                InsertValueToBeforeSpan(spanNodeBefore, insertValue);
                caretSpanIndex_ = info.GetSpanIndex() - 1;
            }
        } else {
            auto imageNode = DynamicCast<FrameNode>(child);
            if (imageNode && caretSpanIndex_ == -1) {
                caretSpanIndex_ = AddTextSpan(options, true, info.GetSpanIndex());
            } else {
                caretSpanIndex_ = AddTextSpan(options, true, caretSpanIndex_ + 1);
            }
        }
    } else {
        caretSpanIndex_ = AddTextSpan(options, true);
    }
}

void RichEditorPattern::SetCaretSpanIndex(int32_t index)
{
    caretSpanIndex_ = index;
}

void RichEditorPattern::HandleOnCut()
{
    caretUpdateType_ = CaretUpdateType::NONE;
    HandleOnCopy();
    DeleteBackward();
}

void RichEditorPattern::OnHandleMove(const RectF& handleRect, bool isFirstHandle)
{
    TextPattern::OnHandleMove(handleRect, isFirstHandle);
    if (!isFirstHandle) {
        SetCaretPosition(textSelector_.destinationOffset);
    }
    auto localOffset = handleRect.GetOffset() - parentGlobalOffset_;
    AutoScrollParam param = { .autoScrollEvent = AutoScrollEvent::HANDLE,
        .handleRect = handleRect,
        .isFirstHandle = isFirstHandle,
        .showScrollbar = true };
    AutoScrollByEdgeDetection(param, localOffset, EdgeDetectionStrategy::OUT_BOUNDARY);
}

#ifdef ENABLE_DRAG_FRAMEWORK
std::function<void(Offset)> RichEditorPattern::GetThumbnailCallback()
{
    return [wk = WeakClaim(this)](const Offset& point) {
        auto pattern = wk.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (pattern->BetweenSelectedPosition(point)) {
            auto host = pattern->GetHost();
            auto children = host->GetChildren();
            std::list<RefPtr<FrameNode>> imageChildren;
            for (const auto& child : children) {
                auto node = DynamicCast<FrameNode>(child);
                if (!node) {
                    continue;
                }
                auto image = node->GetPattern<ImagePattern>();
                if (image) {
                    imageChildren.emplace_back(node);
                }
            }
            pattern->dragNode_ = RichEditorDragPattern::CreateDragNode(host, imageChildren);
            FrameNode::ProcessOffscreenNode(pattern->dragNode_);
        }
    };
}
#endif

void RichEditorPattern::CreateHandles()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    float startSelectHeight = 0.0f;
    float endSelectHeight = 0.0f;
    auto firstHandlePosition = CalcCursorOffsetByPosition(textSelector_.GetStart(), startSelectHeight);
    OffsetF firstHandleOffset(firstHandlePosition.GetX() + parentGlobalOffset_.GetX(),
        firstHandlePosition.GetY() + parentGlobalOffset_.GetY());
    textSelector_.firstHandleOffset_ = firstHandleOffset;
    auto secondHandlePosition = CalcCursorOffsetByPosition(textSelector_.GetEnd(), endSelectHeight);
    OffsetF secondHandleOffset(secondHandlePosition.GetX() + parentGlobalOffset_.GetX(),
        secondHandlePosition.GetY() + parentGlobalOffset_.GetY());
    textSelector_.secondHandleOffset_ = secondHandleOffset;
    SizeF firstHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), startSelectHeight };
    SizeF secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), endSelectHeight };
    RectF firstHandle = RectF(firstHandleOffset, firstHandlePaintSize);
    RectF secondHandle = RectF(secondHandleOffset, secondHandlePaintSize);
    ShowSelectOverlay(firstHandle, secondHandle, IsSelectAll(), RichEditorResponseType::LONG_PRESS);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void RichEditorPattern::OnAreaChangedInner()
{
    float selectLineHeight = 0.0f;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto parentGlobalOffset = host->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    if (parentGlobalOffset != parentGlobalOffset_) {
        parentGlobalOffset_ = parentGlobalOffset;
        UpdateTextFieldManager(Offset(parentGlobalOffset_.GetX(), parentGlobalOffset_.GetY()), frameRect_.Height());
        CHECK_NULL_VOID(SelectOverlayIsOn());
        textSelector_.selectionBaseOffset.SetX(
            CalcCursorOffsetByPosition(textSelector_.GetStart(), selectLineHeight).GetX());
        textSelector_.selectionDestinationOffset.SetX(
            CalcCursorOffsetByPosition(textSelector_.GetEnd(), selectLineHeight).GetX());
        CreateHandles();
    }
}

void RichEditorPattern::CloseSelectionMenu()
{
    CloseSelectOverlay();
}

void RichEditorPattern::CloseSelectOverlay()
{
    TextPattern::CloseSelectOverlay(true);
}

void RichEditorPattern::CalculateHandleOffsetAndShowOverlay(bool isUsingMouse)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto rootOffset = pipeline->GetRootRect().GetOffset();
    auto offset = host->GetPaintRectOffset();
    auto textPaintOffset = offset - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    float startSelectHeight = 0.0f;
    float endSelectHeight = 0.0f;
    auto startOffset = CalcCursorOffsetByPosition(textSelector_.baseOffset, startSelectHeight);
    auto endOffset =
        CalcCursorOffsetByPosition(std::min(textSelector_.destinationOffset, GetTextContentLength()), endSelectHeight);
    SizeF firstHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), startSelectHeight };
    SizeF secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), endSelectHeight };
    OffsetF firstHandleOffset = startOffset + textPaintOffset - rootOffset;
    OffsetF secondHandleOffset = endOffset + textPaintOffset - rootOffset;
    if (GetTextContentLength() == 0) {
        float caretHeight = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretHeight();
        firstHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), caretHeight / 2 };
        secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), caretHeight / 2 };
        firstHandleOffset = OffsetF(firstHandleOffset.GetX(), firstHandleOffset.GetY() + caretHeight / 2);
        secondHandleOffset = OffsetF(secondHandleOffset.GetX(), secondHandleOffset.GetY() + caretHeight);
    }
    textSelector_.selectionBaseOffset = firstHandleOffset;
    textSelector_.selectionDestinationOffset = secondHandleOffset;
    RectF firstHandle;
    firstHandle.SetOffset(firstHandleOffset);
    firstHandle.SetSize(firstHandlePaintSize);
    textSelector_.firstHandle = firstHandle;
    RectF secondHandle;
    secondHandle.SetOffset(secondHandleOffset);
    secondHandle.SetSize(secondHandlePaintSize);
    textSelector_.secondHandle = secondHandle;
}

void RichEditorPattern::ResetSelection()
{
    if (textSelector_.IsValid()) {
        textSelector_.Update(-1, -1);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto eventHub = host->GetEventHub<RichEditorEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto textSelectInfo = GetSpansInfo(-1, -1, GetSpansMethod::ONSELECT);
        eventHub->FireOnSelect(&textSelectInfo);
        UpdateSelectionType(textSelectInfo);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

RefPtr<SpanItem> RichEditorPattern::GetSpanItemByIndex(int32_t index) const
{
    int32_t size = static_cast<int32_t>(spans_.size());
    if (index < 0 || index >= size) {
        return nullptr;
    }
    auto pos = spans_.begin();
    std::advance(pos, index);
    return *pos;
}

bool RichEditorPattern::BetweenSelectedPosition(const Offset& globalOffset)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto offset = host->GetPaintRectOffset();
    auto localOffset = globalOffset - Offset(offset.GetX(), offset.GetY());
    auto eventHub = host->GetEventHub<EventHub>();
    if (GreatNotEqual(textSelector_.GetTextEnd(), textSelector_.GetTextStart())) {
        // Determine if the pan location is in the selected area
        auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
        auto panOffset = OffsetF(localOffset.GetX(), localOffset.GetY()) - GetTextRect().GetOffset() +
                         OffsetF(0.0, std::min(baselineOffset_, 0.0f));
        for (const auto& selectedRect : selectedRects) {
            if (selectedRect.IsInRegion(PointF(panOffset.GetX(), panOffset.GetY()))) {
                return true;
            }
        }
    }
    return false;
}

void RichEditorPattern::HandleSurfaceChanged(int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight)
{
    if (newWidth != prevWidth || newHeight != prevHeight) {
        TextPattern::CloseSelectOverlay();
    }
    UpdateCaretInfoToController();
}

void RichEditorPattern::HandleSurfacePositionChanged(int32_t posX, int32_t posY)
{
    UpdateCaretInfoToController();
}

void RichEditorPattern::DumpInfo()
{
    if (customKeyboardBuilder_) {
        DumpLog::GetInstance().AddDesc(std::string("CustomKeyboard: true")
                                           .append(", Attached: ")
                                           .append(std::to_string(isCustomKeyboardAttached_)));
    }
    auto context = GetHost()->GetContext();
    CHECK_NULL_VOID(context);
    auto richEditorTheme = context->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richEditorTheme);
    DumpLog::GetInstance().AddDesc(std::string("caret offset: ").append(GetCaretRect().GetOffset().ToString()));
    DumpLog::GetInstance().AddDesc(
        std::string("caret height: ")
            .append(std::to_string(NearZero(GetCaretRect().Height())
                                       ? richEditorTheme->GetDefaultCaretHeight().ConvertToPx()
                                       : GetCaretRect().Height())));
}

bool RichEditorPattern::HasFocus() const
{
    auto focusHub = GetHost()->GetOrCreateFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->IsCurrentFocus();
}

void RichEditorPattern::UpdateTextFieldManager(const Offset& offset, float height)
{
    if (!HasFocus()) {
        return;
    }
    auto context = GetHost()->GetContext();
    CHECK_NULL_VOID(context);
    auto richEditorTheme = context->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richEditorTheme);
    auto textFieldManager = DynamicCast<TextFieldManagerNG>(context->GetTextFieldManager());
    CHECK_NULL_VOID(textFieldManager);
    textFieldManager->SetClickPosition(
        { offset.GetX() + GetCaretRect().GetX(), offset.GetY() + GetCaretRect().GetY() });
    textFieldManager->SetHeight(NearZero(GetCaretRect().Height())
                                    ? richEditorTheme->GetDefaultCaretHeight().ConvertToPx()
                                    : GetCaretRect().Height());
    textFieldManager->SetOnFocusTextField(WeakClaim(this));

    if (!isTextChange_) {
        return;
    }
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [weak = WeakClaim(this)] {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->ScrollToSafeArea();
        },
        TaskExecutor::TaskType::UI);
}

bool RichEditorPattern::IsDisabled() const
{
    auto eventHub = GetHost()->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    return !eventHub->IsEnabled();
}

void RichEditorPattern::InitSelection(const Offset& pos)
{
    int32_t currentPosition = paragraphs_.GetIndex(pos);
    currentPosition = std::min(currentPosition, GetTextContentLength());
    int32_t nextPosition = currentPosition + GetGraphemeClusterLength(GetWideText(), currentPosition);
    // if \n char is between current and next position, it's necessary to move selection
    // range one char ahead to reserve handle at the current line
    if ((currentPosition < GetTextContentLength() && currentPosition > 0 &&
            GetWideText().substr(currentPosition, 1) == WIDE_NEWLINE)) {
        nextPosition = std::max(currentPosition - GetGraphemeClusterLength(GetWideText(), currentPosition, true), 0);
        std::swap(currentPosition, nextPosition);
    } else if (currentPosition == 0 && GetWideText().substr(currentPosition, 1) == WIDE_NEWLINE) {
        nextPosition = 0;
    } else if (currentPosition == GetTextContentLength() && currentPosition > 0 &&
               GetWideText().substr(currentPosition - 1, 1) == WIDE_NEWLINE &&
               LessOrEqual(pos.GetY(), contentRect_.Height() + contentRect_.GetY())) {
        // if caret at last position and prev char is \n, set selection to the char before \n
        currentPosition--;
        nextPosition = std::max(currentPosition - GetGraphemeClusterLength(GetWideText(), currentPosition, true), 0);
        std::swap(currentPosition, nextPosition);
    }
    nextPosition = std::min(nextPosition, GetTextContentLength());
    AdjustWordSelection(currentPosition, nextPosition);
    textSelector_.Update(currentPosition, nextPosition);
    auto selectedRects = paragraphs_.GetRects(currentPosition, nextPosition);
    if (selectedRects.empty() && !spans_.empty()) {
        auto it = std::find_if(
            spans_.begin(), spans_.end(), [caretPosition = currentPosition](const RefPtr<SpanItem>& spanItem) {
                return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                           caretPosition) &&
                       (caretPosition < spanItem->position);
            });
        auto spanIndex = std::distance(spans_.begin(), it);
        auto spanNode = DynamicCast<FrameNode>(GetChildByIndex(spanIndex - 1));
        if (spanNode &&
            (spanNode->GetTag() == V2::IMAGE_ETS_TAG || spanNode->GetTag() == V2::PLACEHOLDER_SPAN_ETS_TAG)) {
            textSelector_.Update(currentPosition - 1, currentPosition);
            return;
        }
    }
    bool selectedSingle =
        selectedRects.size() == 1 && (pos.GetX() < selectedRects[0].Left() || pos.GetY() < selectedRects[0].Top());
    bool selectedLast = selectedRects.empty() && currentPosition == GetTextContentLength();
    if (selectedSingle || selectedLast) {
        if (selectedLast) {
            nextPosition = currentPosition + 1;
        }
        auto selectedNextRects = paragraphs_.GetRects(currentPosition - 1, nextPosition - 1);
        if (selectedNextRects.size() == 1) {
            bool isInRange = pos.GetX() >= selectedNextRects[0].Left() && pos.GetX() <= selectedNextRects[0].Right() &&
                             pos.GetY() >= selectedNextRects[0].Top() && pos.GetY() <= selectedNextRects[0].Bottom();
            if (isInRange) {
                textSelector_.Update(currentPosition - 1, nextPosition - 1);
            }
        }
    }
}

void RichEditorPattern::SetSelection(int32_t start, int32_t end)
{
    CHECK_NULL_VOID(HasFocus());
    bool changeSelected = false;
    if (start > end) {
        changeSelected = textSelector_.IsValid();
        ResetSelection();
    } else {
        if (start == -1 && end == -1) {
            start = 0;
            end = GetTextContentLength();
        } else {
            start = std::min(std::max(0, start), GetTextContentLength());
            end = std::min(std::max(0, end), GetTextContentLength());
        }
        changeSelected = start != textSelector_.GetTextStart() || end != textSelector_.GetTextEnd();
        textSelector_.Update(start, end);
    }

    auto oldSelectedType = selectedType_;
    if (textSelector_.IsValid() && !textSelector_.StartEqualToDest()) {
        StopTwinkling();
        if (changeSelected) {
            FireOnSelect(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
        }
    }
    if (SelectOverlayIsOn()) {
        isMousePressed_ = selectOverlayProxy_->GetSelectOverlayMangerInfo().isUsingMouse;
        auto selectedTypeChange = (oldSelectedType.has_value() && selectedType_.has_value() &&
                                      oldSelectedType.value() != selectedType_.value()) ||
                                  (!oldSelectedType.has_value() && selectedType_.has_value());
        if (!isMousePressed_ || selectedTypeChange) {
            CalculateHandleOffsetAndShowOverlay();
            CloseSelectOverlay();
            auto responseType = static_cast<RichEditorResponseType>(
                selectOverlayProxy_->GetSelectOverlayMangerInfo().menuInfo.responseType.value_or(0));
            ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle, IsSelectAll(), responseType);
        }
    }
    SetCaretPosition(textSelector_.GetTextEnd());
    MoveCaretToContentRect();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::BindSelectionMenu(RichEditorResponseType type, RichEditorType richEditorType,
    std::function<void()>& menuBuilder, std::function<void(int32_t, int32_t)>& onAppear,
    std::function<void()>& onDisappear)
{
    auto key = std::make_pair(richEditorType, type);
    auto it = selectionMenuMap_.find(key);
    if (it != selectionMenuMap_.end()) {
        if (menuBuilder == nullptr) {
            selectionMenuMap_.erase(it);
            return;
        }
        it->second->buildFunc = menuBuilder;
        it->second->onAppear = onAppear;
        it->second->onDisappear = onDisappear;
        return;
    }

    auto selectionMenuParams =
        std::make_shared<SelectionMenuParams>(richEditorType, menuBuilder, onAppear, onDisappear, type);
    selectionMenuMap_[key] = selectionMenuParams;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

RefPtr<NodePaintMethod> RichEditorPattern::CreateNodePaintMethod()
{
    if (!contentMod_) {
        contentMod_ = MakeRefPtr<RichEditorContentModifier>(textStyle_, &paragraphs_, WeakClaim(this));
    }
    if (!overlayMod_) {
        auto scrollBar = GetScrollBar();
        if (scrollBar) {
            auto scrollBarModifier = AceType::MakeRefPtr<ScrollBarOverlayModifier>();
            scrollBarModifier->SetRect(scrollBar->GetActiveRect());
            scrollBarModifier->SetPositionMode(scrollBar->GetPositionMode());
            SetScrollBarOverlayModifier(scrollBarModifier);
        }
        SetEdgeEffect(EdgeEffect::FADE);
        overlayMod_ = AceType::MakeRefPtr<RichEditorOverlayModifier>(
            WeakClaim(this), GetScrollBarOverlayModifier(), GetScrollEdgeEffect());
    }

    if (GetIsCustomFont()) {
        contentMod_->SetIsCustomFont(true);
    }
    return MakeRefPtr<RichEditorPaintMethod>(WeakClaim(this), &paragraphs_, baselineOffset_, contentMod_, overlayMod_);
}

int32_t RichEditorPattern::GetHandleIndex(const Offset& offset) const
{
    return paragraphs_.GetIndex(Offset(offset.GetX() + contentRect_.GetX() - richTextRect_.GetX(),
        offset.GetY() + contentRect_.GetY() - richTextRect_.GetY()));
}

std::vector<RectF> RichEditorPattern::GetTextBoxes()
{
    auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    std::vector<RectF> res;
    res.reserve(selectedRects.size());
    for (auto&& rect : selectedRects) {
        if (NearZero(rect.Width())) {
            continue;
        }
        res.emplace_back(rect);
    }
    return res;
}

float RichEditorPattern::GetLineHeight() const
{
    auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    CHECK_NULL_RETURN(selectedRects.size(), 0.0f);
    return selectedRects.front().Height();
}

void RichEditorPattern::UpdateSelectionType(RichEditorSelection& selection)
{
    selectedType_ = RichEditorType::NONE;
    auto list = selection.GetSelection().resultObjects;
    bool imageSelected = false;
    bool textSelected = false;
    for (const auto& obj : list) {
        if (obj.type == RichEditorSpanType::TYPEIMAGE) {
            imageSelected = true;
        } else if (obj.type == RichEditorSpanType::TYPESPAN) {
            textSelected = true;
        }
        if (imageSelected && textSelected) {
            selectedType_ = RichEditorType::MIXED;
            return;
        }
    }
    if (imageSelected) {
        selectedType_ = RichEditorType::IMAGE;
    } else if (textSelected) {
        selectedType_ = RichEditorType::TEXT;
    }
}

std::shared_ptr<SelectionMenuParams> RichEditorPattern::GetMenuParams(
    RichEditorResponseType responseType, RichEditorType type)
{
    auto key = std::make_pair(type, responseType);
    auto it = selectionMenuMap_.find(key);
    if (it != selectionMenuMap_.end()) {
        return it->second;
    }
    return nullptr;
}

RectF RichEditorPattern::GetCaretRect() const
{
    RectF rect;
    CHECK_NULL_RETURN(overlayMod_, rect);
    auto richEditorOverlay = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    CHECK_NULL_RETURN(richEditorOverlay, rect);
    rect.SetOffset(richEditorOverlay->GetCaretOffset());
    rect.SetHeight(richEditorOverlay->GetCaretHeight());
    return rect;
}

void RichEditorPattern::ScrollToSafeArea() const
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textFieldManager = DynamicCast<TextFieldManagerNG>(pipeline->GetTextFieldManager());
    CHECK_NULL_VOID(textFieldManager);
    textFieldManager->ScrollTextFieldToSafeArea();
}

void RichEditorPattern::InitScrollablePattern()
{
    if (GetScrollableEvent()) {
        return;
    }
    SetAxis(Axis::VERTICAL);
    AddScrollEvent();
    SetScrollBar(DisplayMode::AUTO);
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
        CHECK_NULL_VOID(richEditorTheme);
        scrollBar->SetMinHeight(richEditorTheme->GetScrollbarMinHeight());
    }
    if (overlayMod_) {
        UpdateScrollBarOffset();
    }
    auto layoutProperty = GetLayoutProperty<RichEditorLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto& paddingProperty = layoutProperty->GetPaddingProperty();
    if (paddingProperty) {
        auto offsetY = paddingProperty->top.has_value() ? paddingProperty->top->GetDimension().ConvertToPx() : 0.0f;
        auto offsetX = paddingProperty->left.has_value() ? paddingProperty->left->GetDimension().ConvertToPx() : 0.0f;
        richTextRect_.SetOffset(OffsetF(offsetX, offsetY));
    }
}

void RichEditorPattern::ProcessInnerPadding()
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto theme = context->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(theme);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<RichEditorLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto themePadding = theme->GetPadding();
    auto& paddingProp = layoutProperty->GetPaddingProperty();
    auto left = !paddingProp ? CalcLength(themePadding.Left()).GetDimension()
                             : paddingProp->left.value_or(CalcLength(themePadding.Left())).GetDimension();
    auto top = !paddingProp ? CalcLength(themePadding.Top()).GetDimension()
                            : paddingProp->top.value_or(CalcLength(themePadding.Top())).GetDimension();
    auto bottom = !paddingProp ? CalcLength(themePadding.Bottom()).GetDimension()
                               : paddingProp->bottom.value_or(CalcLength(themePadding.Bottom())).GetDimension();
    auto right = !paddingProp ? CalcLength(themePadding.Right()).GetDimension()
                              : paddingProp->right.value_or(CalcLength(themePadding.Right())).GetDimension();
    PaddingProperty paddings;
    paddings.top = NG::CalcLength(top);
    paddings.bottom = NG::CalcLength(bottom);
    paddings.left = NG::CalcLength(left);
    paddings.right = NG::CalcLength(right);
    layoutProperty->UpdatePadding(paddings);
}

void RichEditorPattern::UpdateScrollStateAfterLayout(bool shouldDisappear)
{
    bool hasTextOffsetChanged = false;
    if (GreatNotEqual(richTextRect_.GetY(), contentRect_.GetY())) {
        auto offset = richTextRect_.GetOffset();
        offset.AddY(contentRect_.GetY() - richTextRect_.GetY());
        richTextRect_.SetOffset(offset);
        hasTextOffsetChanged = true;
    }
    if (GreatNotEqual(richTextRect_.Height(), contentRect_.Height()) &&
        LessNotEqual(richTextRect_.Bottom(), contentRect_.Bottom())) {
        auto offset = richTextRect_.GetOffset();
        offset.AddY(contentRect_.Bottom() - richTextRect_.Bottom());
        richTextRect_.SetOffset(offset);
        hasTextOffsetChanged = true;
    }
    if (LessOrEqual(richTextRect_.Height(), contentRect_.Height()) &&
        LessNotEqual(richTextRect_.GetY(), contentRect_.GetY())) {
        richTextRect_.SetOffset(contentRect_.GetOffset());
        hasTextOffsetChanged = true;
    }
    if (hasTextOffsetChanged) {
        UpdateChildrenOffset();
    }
    StopScrollable();
    CheckScrollable();
    if (overlayMod_) {
        UpdateScrollBarOffset();
    }
    if (!GetScrollBar()) {
        return;
    }
    if (isFirstCallOnReady_) {
        isFirstCallOnReady_ = false;
        GetScrollBar()->ScheduleDisappearDelayTask();
        return;
    }
    if (shouldDisappear) {
        GetScrollBar()->ScheduleDisappearDelayTask();
    }
}

bool RichEditorPattern::OnScrollCallback(float offset, int32_t source)
{
    if (source == SCROLL_FROM_START) {
        auto scrollBar = GetScrollBar();
        if (scrollBar) {
            scrollBar->PlayScrollBarAppearAnimation();
        }
        return true;
    }
    if (IsReachedBoundary(offset)) {
        return false;
    }
    auto newOffset = MoveTextRect(offset);
    MoveFirstHandle(newOffset);
    MoveSecondHandle(newOffset);
    return true;
}

float RichEditorPattern::MoveTextRect(float offset)
{
    if (GreatNotEqual(richTextRect_.Height(), contentRect_.Height())) {
        if (GreatNotEqual(richTextRect_.GetY() + offset, contentRect_.GetY())) {
            offset = contentRect_.GetY() - richTextRect_.GetY();
        } else if (LessNotEqual(richTextRect_.Bottom() + offset, contentRect_.Bottom())) {
            offset = contentRect_.Bottom() - richTextRect_.Bottom();
        }
    } else if (!NearEqual(richTextRect_.GetY(), contentRect_.GetY())) {
        offset = contentRect_.GetY() - richTextRect_.GetY();
    } else {
        return 0.0f;
    }
    if (NearEqual(offset, 0.0f)) {
        return offset;
    }
    scrollOffset_ = richTextRect_.GetY() + offset;
    richTextRect_.SetOffset(OffsetF(richTextRect_.GetX(), scrollOffset_));
    UpdateScrollBarOffset();
    UpdateChildrenOffset();
    return offset;
}

void RichEditorPattern::MoveFirstHandle(float offset)
{
    if (SelectOverlayIsOn() && !NearEqual(offset, 0.0f)) {
        textSelector_.selectionBaseOffset.AddY(offset);
        auto firstHandleOffset = textSelector_.firstHandle.GetOffset();
        firstHandleOffset.AddY(offset);
        textSelector_.firstHandle.SetOffset(firstHandleOffset);
        SelectHandleInfo firstHandleInfo;
        firstHandleInfo.paintRect = textSelector_.firstHandle;
        firstHandleInfo.needLayout = true;
        CheckHandles(firstHandleInfo);
        selectOverlayProxy_->UpdateFirstSelectHandleInfo(firstHandleInfo);
    }
}

void RichEditorPattern::MoveSecondHandle(float offset)
{
    if (SelectOverlayIsOn() && !NearEqual(offset, 0.0f)) {
        textSelector_.selectionDestinationOffset.AddY(offset);
        auto secondHandleOffset = textSelector_.secondHandle.GetOffset();
        secondHandleOffset.AddY(offset);
        textSelector_.secondHandle.SetOffset(secondHandleOffset);
        SelectHandleInfo secondHandleInfo;
        secondHandleInfo.paintRect = textSelector_.secondHandle;
        secondHandleInfo.needLayout = true;
        CheckHandles(secondHandleInfo);
        selectOverlayProxy_->UpdateSecondSelectHandleInfo(secondHandleInfo);
    }
}

bool RichEditorPattern::MoveCaretToContentRect()
{
    auto contentRect = GetTextContentRect();
    auto textRect = GetTextRect();
    if (LessOrEqual(textRect.Height(), contentRect.Height())) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT,
            "MoveCaretToContentRect text height is smaller than content height, no need to move caret");
        return true;
    }
    float caretHeight = 0.0f;
    auto caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight, true);
    if (LessNotEqual(caretOffset.GetY(), contentRect.GetY())) {
        MoveTextRect(contentRect.GetY() - caretOffset.GetY());
        return true;
    }
    if (GreatNotEqual(caretOffset.GetY() + caretHeight, contentRect.Bottom())) {
        MoveTextRect(contentRect.Bottom() - caretOffset.GetY() - caretHeight);
        return true;
    }
    return false;
}

void RichEditorPattern::UpdateScrollBarOffset()
{
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    Size size(frameRect_.Width(), frameRect_.Height());
    auto verticalGap = frameRect_.Height() - contentRect_.Height();
    UpdateScrollBarRegion(contentRect_.GetY() - richTextRect_.GetY(), richTextRect_.Height() + verticalGap,
        size, Offset(0.0, 0.0));
    auto tmpHost = GetHost();
    CHECK_NULL_VOID(tmpHost);
    tmpHost->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::OnScrollEndCallback()
{
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        scrollBar->ScheduleDisappearDelayTask();
    }
}

bool RichEditorPattern::IsReachedBoundary(float offset)
{
    return (NearEqual(richTextRect_.GetY(), contentRect_.GetY()) && GreatNotEqual(offset, 0.0f)) ||
           (NearEqual(richTextRect_.GetY() + richTextRect_.Height(), contentRect_.GetY() + contentRect_.Height()) &&
               LessNotEqual(offset, 0.0f));
}

void RichEditorPattern::CheckScrollable()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    scrollable_ = GetTextContentLength() > 0 && GreatNotEqual(richTextRect_.Height(), contentRect_.Height());
    SetScrollEnable(scrollable_);
}

void RichEditorPattern::UpdateChildrenOffset()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::vector<int32_t> placeholderIndex;
    for (const auto& child : spans_) {
        if (!child) {
            continue;
        }
        auto imageSpanItem = AceType::DynamicCast<ImageSpanItem>(child);
        if (imageSpanItem) {
            placeholderIndex.emplace_back(child->placeholderIndex);
        }
    }
    if (spans_.empty() || placeholderIndex.empty()) {
        return;
    }
    size_t index = 0;
    std::vector<RectF> rectsForPlaceholders = paragraphs_.GetPlaceholderRects();
    auto childrenNodes = host->GetChildren();
    auto textOffset = GetTextRect().GetOffset();
    for (const auto& child : childrenNodes) {
        auto childNode = AceType::DynamicCast<FrameNode>(child);
        if (!childNode) {
            continue;
        }
        auto pattern = childNode->GetPattern<ImagePattern>();
        if (!pattern) {
            continue;
        }
        if (index >= rectsForPlaceholders.size()) {
            break;
        }
        auto rect = rectsForPlaceholders.at(index);
        auto geometryNode = childNode->GetGeometryNode();
        if (geometryNode) {
            geometryNode->SetMarginFrameOffset(textOffset + OffsetF(rect.Left(), rect.Top()));
            childNode->ForceSyncGeometryNode();
            childNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
        ++index;
    }
}

void RichEditorPattern::AutoScrollByEdgeDetection(AutoScrollParam param, OffsetF offset, EdgeDetectionStrategy strategy)
{
    auto deltaOffset = offset - prevAutoScrollOffset_;
    auto thresholdDistance = AUTO_SCROLL_MOVE_THRESHOLD.ConvertToPx();
    if (std::abs(deltaOffset.GetY()) < thresholdDistance) {
        return;
    }
    prevAutoScrollOffset_ = offset;
    auto contentRect = GetTextContentRect();
    float edgeThreshold = param.autoScrollEvent == AutoScrollEvent::DRAG ? AUTO_SCROLL_DRAG_EDGE_DISTANCE.ConvertToPx()
                                                                         : AUTO_SCROLL_EDGE_DISTANCE.ConvertToPx();
    if (GreatOrEqual(edgeThreshold, contentRect.Height())) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "AutoScrollByEdgeDetection: Content height is too small.");
        return;
    }
    float topEdgeThreshold = edgeThreshold + contentRect.GetY();
    float bottomThreshold = contentRect.Bottom() - edgeThreshold;

    if (param.autoScrollEvent == AutoScrollEvent::HANDLE) {
        auto handleTopOffset = offset;
        auto handleBottomOffset = OffsetF(offset.GetX(), offset.GetY() + param.handleRect.Height());
        if (GreatNotEqual(handleBottomOffset.GetY(), bottomThreshold)) {
            param.offset = bottomThreshold - handleBottomOffset.GetY();
            ScheduleAutoScroll(param);
        } else if (LessNotEqual(handleTopOffset.GetY(), topEdgeThreshold)) {
            param.offset = topEdgeThreshold - handleTopOffset.GetY();
            ScheduleAutoScroll(param);
        } else {
            StopAutoScroll();
        }
        return;
    }

    // drag and mouse
    if (GreatNotEqual(offset.GetY(), bottomThreshold)) {
        param.offset = bottomThreshold - offset.GetY();
        ScheduleAutoScroll(param);
    } else if (LessNotEqual(offset.GetY(), topEdgeThreshold)) {
        param.offset = topEdgeThreshold - offset.GetY();
        ScheduleAutoScroll(param);
    } else {
        StopAutoScroll();
    }
}

void RichEditorPattern::ScheduleAutoScroll(AutoScrollParam param)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (!context->GetTaskExecutor()) {
        return;
    }
    autoScrollTask_.Reset([weak = WeakClaim(this), param]() {
        auto client = weak.Upgrade();
        CHECK_NULL_VOID(client);
        client->OnAutoScroll(param);
    });

    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostDelayedTask(autoScrollTask_, TaskExecutor::TaskType::UI, AUTO_SCROLL_INTERVAL);
}

void RichEditorPattern::OnAutoScroll(AutoScrollParam param)
{
    if (param.showScrollbar) {
        auto scrollBar = GetScrollBar();
        if (scrollBar) {
            scrollBar->PlayScrollBarAppearAnimation();
        }
        param.showScrollbar = false;
    }

    if (param.autoScrollEvent == AutoScrollEvent::HANDLE) {
        auto newOffset = MoveTextRect(param.offset);
        if (param.isFirstHandle) {
            MoveSecondHandle(newOffset);
        } else {
            MoveFirstHandle(newOffset);
        }
        TextPattern::OnHandleMove(param.handleRect, param.isFirstHandle);
        if (NearEqual(newOffset, 0.0f)) {
            return;
        }
        ScheduleAutoScroll(param);
        return;
    }

    if (param.autoScrollEvent == AutoScrollEvent::DRAG) {
        auto newOffset = MoveTextRect(param.offset);
        if (NearEqual(newOffset, 0.0f)) {
            return;
        }
        ScheduleAutoScroll(param);
        return;
    }

    if (param.autoScrollEvent == AutoScrollEvent::MOUSE) {
        auto newOffset = MoveTextRect(param.offset);
        auto textOffset =
            Offset(param.eventOffset.GetX() - GetTextRect().GetX(), param.eventOffset.GetY() - GetTextRect().GetY());
        int32_t extend = paragraphs_.GetIndex(textOffset);
        textSelector_.Update(textSelector_.baseOffset, extend);
        SetCaretPosition(std::max(textSelector_.baseOffset, extend));
        if (NearEqual(newOffset, 0.0f)) {
            return;
        }
        ScheduleAutoScroll(param);
    }
}

void RichEditorPattern::StopAutoScroll()
{
    autoScrollTask_.Cancel();
    prevAutoScrollOffset_ = OffsetF(0.0f, 0.0f);
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        scrollBar->PlayScrollBarDisappearAnimation();
    }
}

bool RichEditorPattern::NeedAiAnalysis(
    const CaretUpdateType targeType, const int32_t pos, const int32_t& spanStart, const std::string& content)
{
    if (spanStart < 0) {
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "NeedAiAnalysis -spanStart%{public}d,return!", spanStart);
        return false;
    }

    if (!InputAIChecker::NeedAIAnalysis(content, targeType, lastClickTimeStamp_ - lastAiPosTimeStamp_)) {
        return false;
    }

    if (IsClickBoundary(pos) && targeType == CaretUpdateType::PRESSED) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "NeedAiAnalysis IsClickBoundary,return!");
        return false;
    }
    return false;
}

void RichEditorPattern::AdjustCursorPosition(int32_t& pos)
{
    // the rich text has some spans, the pos is belong to the whole richtext content, should use (pos - spanStarint)
    int32_t spanStart = -1;
    // get the span text by the position, maybe text is empty
    std::string content = GetPositionSpansText(pos, spanStart);

    if (NeedAiAnalysis(CaretUpdateType::PRESSED, pos, spanStart, content)) {
        int32_t aiPos = pos - spanStart;
        DataDetectorMgr::GetInstance().AdjustCursorPosition(aiPos, content, lastAiPosTimeStamp_, lastClickTimeStamp_);
        if (aiPos < 0) {
            return;
        }
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get ai pos:%{public}d--spanStart%{public}d", aiPos, spanStart);
        pos = aiPos + spanStart;
    }
}

void RichEditorPattern::AdjustWordSelection(int32_t& start, int32_t& end)
{
    // the rich text has some spans, the pos is belong to the whole richtext content, should use (pos - spanStarint)
    int32_t spanStart = -1;
    // get the span text by the position, maybe text is empty
    std::string content = GetPositionSpansText(start, spanStart);
    if (NeedAiAnalysis(CaretUpdateType::DOUBLE_CLICK, start, spanStart, content)) {
        int32_t aiPosStart = start - spanStart;
        int32_t aiPosEnd = end - spanStart;
        DataDetectorMgr::GetInstance().AdjustWordSelection(aiPosStart, content, aiPosStart, aiPosEnd);
        if (aiPosStart < 0 || aiPosEnd < 0) {
            return;
        }

        start = std::min(aiPosStart + spanStart, GetTextContentLength());
        end = std::min(aiPosEnd + spanStart, GetTextContentLength());
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get ai selector [%{public}d--%{public}d", start, end);
    }
}

bool RichEditorPattern::IsClickBoundary(const int32_t position)
{
    if (InputAIChecker::IsSingleClickAtBoundary(position, GetTextContentLength())) {
        return true;
    }

    float height = 0;
    auto handleOffset = CalcCursorOffsetByPosition(position, height);
    if (InputAIChecker::IsMultiClickAtBoundary(handleOffset, TextPattern::GetTextRect())) {
        return true;
    }
    return false;
}

std::string RichEditorPattern::GetPositionSpansText(int32_t position, int32_t& startSpan)
{
    int32_t start = position - AI_TEXT_RANGE_LEFT;
    int32_t end = position + AI_TEXT_RANGE_RIGHT;

    start = std::clamp(start, 0, GetTextContentLength());
    end = std::clamp(end, 0, GetTextContentLength());

    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get span pos:%{public}d-start:%{public}d-end:%{public}d", position, start, end);

    // get all the spans between start and end, then filter the valid text
    auto infos = GetSpansInfo(start, end, GetSpansMethod::ONSELECT);
    if (infos.GetSelection().resultObjects.empty()) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get spans text is null pos:%{public}d,return", position);
        return "";
    }
    auto list = infos.GetSelection().resultObjects;

    std::stringstream sstream;
    for (const auto& obj : list) {
        if (obj.type == RichEditorSpanType::TYPEIMAGE) {
            if (obj.spanPosition.spanRange[1] <= position) {
                sstream.str("");
                startSpan = -1;
            } else {
                break;
            }
        } else if (obj.type == RichEditorSpanType::TYPESPAN) {
            if (startSpan < 0) {
                startSpan = obj.spanPosition.spanRange[0] + obj.offsetInSpan[0];
            }
            // we should use the wide string deal to avoid crash
            auto wideText = StringUtils::ToWstring(obj.valueString);
            sstream << StringUtils::ToString(
                wideText.substr(obj.offsetInSpan[0], obj.offsetInSpan[1] - obj.offsetInSpan[0]));
        }
    }

    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get spans text ret spanStart:%{public}d", startSpan);
    return sstream.str();
}

void RichEditorPattern::CheckHandles(SelectHandleInfo& handleInfo)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto offset = host->GetPaintRectOffset() + contentRect_.GetOffset() - pipeline->GetRootRect().GetOffset();
    RectF contentGlobalRect(offset, contentRect_.GetSize());
    contentGlobalRect = GetVisibleContentRect(host->GetAncestorNodeOfFrame(), contentGlobalRect);
    auto handleOffset = handleInfo.paintRect.GetOffset();
    handleInfo.isShow = contentGlobalRect.IsInRegion(PointF(handleOffset.GetX(), handleOffset.GetY() + BOX_EPSILON));
}
} // namespace OHOS::Ace::NG
