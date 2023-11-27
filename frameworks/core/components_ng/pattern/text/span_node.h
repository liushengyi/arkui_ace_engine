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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_SPAN_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_SPAN_NODE_H

#include <list>
#include <memory>
#include <optional>
#include <string>

#include "base/memory/referenced.h"
#include "core/common/ai/data_detector_mgr.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "core/components_ng/render/paragraph.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_v2/inspector/utils.h"

#define DEFINE_SPAN_FONT_STYLE_ITEM(name, type)                              \
public:                                                                      \
    std::optional<type> Get##name() const                                    \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Get##name();                        \
        }                                                                    \
        return std::nullopt;                                                 \
    }                                                                        \
    bool Has##name() const                                                   \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Has##name();                        \
        }                                                                    \
        return false;                                                        \
    }                                                                        \
    type Get##name##Value(const type& defaultValue) const                    \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Get##name().value_or(defaultValue); \
        }                                                                    \
        return defaultValue;                                                 \
    }                                                                        \
    void Update##name(const type& value)                                     \
    {                                                                        \
        if (!spanItem_->fontStyle) {                                         \
            spanItem_->fontStyle = std::make_unique<FontStyle>();            \
        }                                                                    \
        if (spanItem_->fontStyle->Check##name(value)) {                      \
            return;                                                          \
        }                                                                    \
        spanItem_->fontStyle->Update##name(value);                           \
        RequestTextFlushDirty();                                             \
    }                                                                        \
    void Reset##name()                                                       \
    {                                                                        \
        if (spanItem_->fontStyle) {                                          \
            return spanItem_->fontStyle->Reset##name();                      \
        }                                                                    \
    }                                                                        \
    void Update##name##WithoutFlushDirty(const type& value)                  \
    {                                                                        \
        if (!spanItem_->fontStyle) {                                         \
            spanItem_->fontStyle = std::make_unique<FontStyle>();            \
        }                                                                    \
        if (spanItem_->fontStyle->Check##name(value)) {                      \
            return;                                                          \
        }                                                                    \
        spanItem_->fontStyle->Update##name(value);                           \
    }

#define DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(name, type)                             \
public:                                                                          \
    std::optional<type> Get##name() const                                        \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Get##name();                        \
        }                                                                        \
        return std::nullopt;                                                     \
    }                                                                            \
    bool Has##name() const                                                       \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Has##name();                        \
        }                                                                        \
        return false;                                                            \
    }                                                                            \
    type Get##name##Value(const type& defaultValue) const                        \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Get##name().value_or(defaultValue); \
        }                                                                        \
        return defaultValue;                                                     \
    }                                                                            \
    void Update##name(const type& value)                                         \
    {                                                                            \
        if (!spanItem_->textLineStyle) {                                         \
            spanItem_->textLineStyle = std::make_unique<TextLineStyle>();        \
        }                                                                        \
        if (spanItem_->textLineStyle->Check##name(value)) {                      \
            return;                                                              \
        }                                                                        \
        spanItem_->textLineStyle->Update##name(value);                           \
        RequestTextFlushDirty();                                                 \
    }                                                                            \
    void Reset##name()                                                           \
    {                                                                            \
        if (spanItem_->textLineStyle) {                                          \
            return spanItem_->textLineStyle->Reset##name();                      \
        }                                                                        \
    }                                                                            \
    void Update##name##WithoutFlushDirty(const type& value)                      \
    {                                                                            \
        if (!spanItem_->textLineStyle) {                                         \
            spanItem_->textLineStyle = std::make_unique<TextLineStyle>();        \
        }                                                                        \
        if (spanItem_->textLineStyle->Check##name(value)) {                      \
            return;                                                              \
        }                                                                        \
        spanItem_->textLineStyle->Update##name(value);                           \
    }

namespace OHOS::Ace::NG {

class Paragraph;

struct SpanItem : public AceType {
    DECLARE_ACE_TYPE(SpanItem, AceType);

public:
    SpanItem() = default;
    virtual ~SpanItem()
    {
        children.clear();
    }
    // position of last char + 1
    int32_t position = -1;
    int32_t imageNodeId = -1;
    std::string content;
    std::unique_ptr<FontStyle> fontStyle = std::make_unique<FontStyle>();
    std::unique_ptr<TextLineStyle> textLineStyle = std::make_unique<TextLineStyle>();
    GestureEventFunc onClick;
    GestureEventFunc onLongPress;
    [[deprecated]] std::list<RefPtr<SpanItem>> children;
    std::map<int32_t, AISpan> aiSpanMap;
    int32_t placeholderIndex = -1;
    // when paragraph ends with a \n, it causes the paragraph height to gain an extra line
    // to have normal spacing between paragraphs, remove \n from every paragraph except the last one.
    bool needRemoveNewLine = false;
    std::optional<LeadingMargin> leadingMargin;
#ifdef ENABLE_DRAG_FRAMEWORK
    int32_t selectedStart = -1;
    int32_t selectedEnd = -1;
#endif // ENABLE_DRAG_FRAMEWORK
    virtual int32_t UpdateParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder,
        double width = 0.0f, double height = 0.0f, VerticalAlign verticalAlign = VerticalAlign::BASELINE);
    virtual void UpdateTextStyleForAISpan(
        const std::string& content, const RefPtr<Paragraph>& builder, const std::optional<TextStyle>& textStyle);
    virtual void UpdateTextStyle(
        const std::string& content, const RefPtr<Paragraph>& builder, const std::optional<TextStyle>& textStyle);
    virtual void GetIndex(int32_t& start, int32_t& end) const;
    virtual void FontRegisterCallback(const RefPtr<FrameNode>& frameNode, const TextStyle& textStyle);
    virtual void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
    std::string GetFont() const;
#ifdef ENABLE_DRAG_FRAMEWORK
    virtual void StartDrag(int32_t start, int32_t end);
    virtual void EndDrag();
    virtual bool IsDragging();
#endif // ENABLE_DRAG_FRAMEWORK
    std::optional<TextStyle> GetTextStyle() const
    {
        return textStyle_;
    }
    void SetTextStyle(const std::optional<TextStyle>& textStyle)
    {
        textStyle_ = textStyle;
    }
    void MarkNeedRemoveNewLine(bool value)
    {
        needRemoveNewLine = value;
    }
    void SetOnClickEvent(GestureEventFunc&& onClick_)
    {
        onClick = std::move(onClick_);
    }
    void SetLongPressEvent(GestureEventFunc&& onLongPress_)
    {
        onLongPress = std::move(onLongPress_);
    }
    std::string GetSpanContent();

private:
    std::optional<TextStyle> textStyle_;
};


enum class PropertyInfo {
    FONTSIZE = 0,
    FONTCOLOR,
    FONTSTYLE,
    FONTWEIGHT,
    FONTFAMILY,
    TEXTDECORATION,
    TEXTCASE,
    LETTERSPACE,
    LINEHEIGHT,
    TEXT_ALIGN,
    LEADING_MARGIN,
    NONE,
    TEXTSHADOW,
};

class ACE_EXPORT SpanNode : public UINode {
    DECLARE_ACE_TYPE(SpanNode, UINode);

public:
    static RefPtr<SpanNode> GetOrCreateSpanNode(int32_t nodeId);

    explicit SpanNode(int32_t nodeId) : UINode(V2::SPAN_ETS_TAG, nodeId) {}
    ~SpanNode() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    const RefPtr<SpanItem>& GetSpanItem() const
    {
        return spanItem_;
    }

    void UpdateContent(const std::string& content)
    {
        if (spanItem_->content == content) {
            return;
        }
        spanItem_->content = content;
        RequestTextFlushDirty();
    }

    void UpdateOnClickEvent(GestureEventFunc&& onClick)
    {
        spanItem_->onClick = std::move(onClick);
    }

    DEFINE_SPAN_FONT_STYLE_ITEM(FontSize, Dimension);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextColor, Color);
    DEFINE_SPAN_FONT_STYLE_ITEM(ItalicFontStyle, Ace::FontStyle);
    DEFINE_SPAN_FONT_STYLE_ITEM(FontWeight, FontWeight);
    DEFINE_SPAN_FONT_STYLE_ITEM(FontFamily, std::vector<std::string>);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextDecoration, TextDecoration);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextDecorationStyle, TextDecorationStyle);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextDecorationColor, Color);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextCase, TextCase);
    DEFINE_SPAN_FONT_STYLE_ITEM(TextShadow, std::vector<Shadow>);
    DEFINE_SPAN_FONT_STYLE_ITEM(LetterSpacing, Dimension);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(LineHeight, Dimension);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(TextAlign, TextAlign);
    DEFINE_SPAN_TEXT_LINE_STYLE_ITEM(LeadingMargin, LeadingMargin);

    // Mount to the previous Span node or Text node.
    void MountToParagraph();

    void AddChildSpanItem(const RefPtr<SpanNode>& child)
    {
        spanItem_->children.emplace_back(child->GetSpanItem());
    }

    void CleanSpanItemChildren()
    {
        spanItem_->children.clear();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        spanItem_->ToJsonValue(json);
    }

    void RequestTextFlushDirty();
    // The function is only used for fast preview.
    void FastPreviewUpdateChildDone() override
    {
        RequestTextFlushDirty();
    }

    void AddPropertyInfo(PropertyInfo value)
    {
        propertyInfo_.insert(value);
    }

    void CleanPropertyInfo()
    {
        propertyInfo_.clear();
    }

    std::set<PropertyInfo> CalculateInheritPropertyInfo()
    {
        std::set<PropertyInfo> inheritPropertyInfo;
        const std::set<PropertyInfo> propertyInfoContainer = { PropertyInfo::FONTSIZE, PropertyInfo::FONTCOLOR,
            PropertyInfo::FONTSTYLE, PropertyInfo::FONTWEIGHT, PropertyInfo::FONTFAMILY, PropertyInfo::TEXTDECORATION,
            PropertyInfo::TEXTCASE, PropertyInfo::LETTERSPACE, PropertyInfo::LINEHEIGHT, PropertyInfo::TEXT_ALIGN,
            PropertyInfo::LEADING_MARGIN, PropertyInfo::TEXTSHADOW };
        set_difference(propertyInfoContainer.begin(), propertyInfoContainer.end(), propertyInfo_.begin(),
            propertyInfo_.end(), inserter(inheritPropertyInfo, inheritPropertyInfo.begin()));
        return inheritPropertyInfo;
    }

private:
    std::list<RefPtr<SpanNode>> spanChildren_;
    std::set<PropertyInfo> propertyInfo_;

    RefPtr<SpanItem> spanItem_ = MakeRefPtr<SpanItem>();

    ACE_DISALLOW_COPY_AND_MOVE(SpanNode);
};

struct PlaceholderSpanItem : public SpanItem {
    DECLARE_ACE_TYPE(PlaceholderSpanItem, SpanItem);

public:
    int32_t placeholderSpanNodeId = -1;
    TextStyle textStyle;
    PlaceholderSpanItem() = default;
    ~PlaceholderSpanItem() override = default;
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override {};
    int32_t UpdateParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder, double width,
        double height, VerticalAlign verticalAlign) override;
    ACE_DISALLOW_COPY_AND_MOVE(PlaceholderSpanItem);
};

class PlaceholderSpanPattern : public Pattern {
    DECLARE_ACE_TYPE(PlaceholderSpanPattern, Pattern);

public:
    PlaceholderSpanPattern() = default;
    ~PlaceholderSpanPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }
};

class ACE_EXPORT PlaceholderSpanNode : public FrameNode {
    DECLARE_ACE_TYPE(PlaceholderSpanNode, FrameNode);

public:
    static RefPtr<PlaceholderSpanNode> GetOrCreateSpanNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
    {
        auto frameNode = GetFrameNode(tag, nodeId);
        CHECK_NULL_RETURN(!frameNode, AceType::DynamicCast<PlaceholderSpanNode>(frameNode));
        auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
        auto placeholderSpanNode = AceType::MakeRefPtr<PlaceholderSpanNode>(tag, nodeId, pattern);
        placeholderSpanNode->InitializePatternAndContext();
        ElementRegister::GetInstance()->AddUINode(placeholderSpanNode);
        return placeholderSpanNode;
    }

    PlaceholderSpanNode(const std::string& tag, int32_t nodeId) : FrameNode(tag, nodeId, AceType::MakeRefPtr<Pattern>())
    {}
    PlaceholderSpanNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : FrameNode(tag, nodeId, pattern)
    {}
    ~PlaceholderSpanNode() override = default;

    const RefPtr<PlaceholderSpanItem>& GetSpanItem() const
    {
        return placeholderSpanItem_;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

private:
    RefPtr<PlaceholderSpanItem> placeholderSpanItem_ = MakeRefPtr<PlaceholderSpanItem>();

    ACE_DISALLOW_COPY_AND_MOVE(PlaceholderSpanNode);
};

struct ImageSpanItem : public PlaceholderSpanItem {
    DECLARE_ACE_TYPE(ImageSpanItem, PlaceholderSpanItem);

public:
    ImageSpanItem() = default;
    ~ImageSpanItem() override = default;
    int32_t UpdateParagraph(const RefPtr<FrameNode>& frameNode, const RefPtr<Paragraph>& builder, double width,
        double height, VerticalAlign verticalAlign) override;
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override {};
    ACE_DISALLOW_COPY_AND_MOVE(ImageSpanItem);
};

class ACE_EXPORT ImageSpanNode : public FrameNode {
    DECLARE_ACE_TYPE(ImageSpanNode, FrameNode);

public:
    static RefPtr<ImageSpanNode> GetOrCreateSpanNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
    {
        auto frameNode = GetFrameNode(tag, nodeId);
        CHECK_NULL_RETURN(!frameNode, AceType::DynamicCast<ImageSpanNode>(frameNode));
        auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
        auto imageSpanNode = AceType::MakeRefPtr<ImageSpanNode>(tag, nodeId, pattern);
        imageSpanNode->InitializePatternAndContext();
        ElementRegister::GetInstance()->AddUINode(imageSpanNode);
        return imageSpanNode;
    }

    ImageSpanNode(const std::string& tag, int32_t nodeId) : FrameNode(tag, nodeId, AceType::MakeRefPtr<ImagePattern>())
    {}
    ImageSpanNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : FrameNode(tag, nodeId, pattern)
    {}
    ~ImageSpanNode() override = default;

    const RefPtr<ImageSpanItem>& GetSpanItem() const
    {
        return imageSpanItem_;
    }

private:
    RefPtr<ImageSpanItem> imageSpanItem_ = MakeRefPtr<ImageSpanItem>();

    ACE_DISALLOW_COPY_AND_MOVE(ImageSpanNode);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_FOR_EACH_NODE_H
