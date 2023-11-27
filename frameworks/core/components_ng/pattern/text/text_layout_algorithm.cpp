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

#include "core/components_ng/pattern/text/text_layout_algorithm.h"

#include <limits>

#include "text_layout_adapter.h"

#include "base/geometry/dimension.h"
#include "base/i18n/localization.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/font_manager.h"
#include "core/components/text/text_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/font_collection.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
/**
 * The baseline information needs to be calculated based on contentOffsetY.
 */
float GetContentOffsetY(LayoutWrapper* layoutWrapper)
{
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto offsetY = padding.top.value_or(0);
    auto align = Alignment::CENTER;
    if (layoutWrapper->GetLayoutProperty()->GetPositionProperty()) {
        align = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().value_or(align);
    }
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    if (content) {
        offsetY += Alignment::GetAlignPosition(size, content->GetRect().GetSize(), align).GetY();
    }
    return offsetY;
}
} // namespace

TextLayoutAlgorithm::TextLayoutAlgorithm() = default;

void TextLayoutAlgorithm::OnReset() {}

std::optional<SizeF> TextLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    if (!contentConstraint.maxSize.IsPositive()) {
        return std::nullopt;
    }

    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, std::nullopt);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_RETURN(pipeline, std::nullopt);
    auto textLayoutProperty = DynamicCast<TextLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(textLayoutProperty, std::nullopt);
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_RETURN(pattern, std::nullopt);
    auto contentModifier = pattern->GetContentModifier();

    TextStyle textStyle = CreateTextStyleUsingTheme(
        textLayoutProperty->GetFontStyle(), textLayoutProperty->GetTextLineStyle(), pipeline->GetTheme<TextTheme>());
    if (contentModifier) {
        SetPropertyToModifier(textLayoutProperty, contentModifier);
        contentModifier->ModifyTextStyle(textStyle);
        contentModifier->SetFontReady(false);
    }
    textStyle.SetHalfLeading(pipeline->GetHalfLeading());
    // Register callback for fonts.
    FontRegisterCallback(frameNode, textStyle);

    // Determines whether a foreground color is set or inherited.
    UpdateTextColorIfForeground(frameNode, textStyle);

    if (textStyle.GetTextOverflow() == TextOverflow::MARQUEE) {
        return BuildTextRaceParagraph(textStyle, textLayoutProperty, contentConstraint, pipeline, layoutWrapper);
    }

    if (!AddPropertiesAndAnimations(textStyle, textLayoutProperty, contentConstraint, pipeline, layoutWrapper)) {
        return std::nullopt;
    }

    textStyle_ = textStyle;

    auto height = static_cast<float>(paragraph_->GetHeight());
    double baselineOffset = 0.0;
    textStyle.GetBaselineOffset().NormalizeToPx(
        pipeline->GetDipScale(), pipeline->GetFontScale(), pipeline->GetLogicScale(), 0.0f, baselineOffset);

    baselineOffset_ = static_cast<float>(baselineOffset);
    auto heightFinal = static_cast<float>(height + std::fabs(baselineOffset));
    if (contentConstraint.selfIdealSize.Height().has_value()) {
        heightFinal = std::min(
            static_cast<float>(height + std::fabs(baselineOffset)), contentConstraint.selfIdealSize.Height().value());
    } else {
        heightFinal =
            std::min(static_cast<float>(height + std::fabs(baselineOffset)), contentConstraint.maxSize.Height());
    }
    if (frameNode->GetTag() == V2::TEXT_ETS_TAG && textLayoutProperty->GetContent().value_or("").empty() &&
        NonPositive(static_cast<double>(paragraph_->GetLongestLine()))) {
        // text content is empty
        return SizeF {};
    }
    return SizeF(paragraph_->GetMaxWidth(), heightFinal);
}

bool TextLayoutAlgorithm::AddPropertiesAndAnimations(TextStyle& textStyle,
    const RefPtr<TextLayoutProperty>& textLayoutProperty, const LayoutConstraintF& contentConstraint,
    const RefPtr<PipelineContext>& pipeline, LayoutWrapper* layoutWrapper)
{
    bool result = false;
    switch (textLayoutProperty->GetHeightAdaptivePolicyValue(TextHeightAdaptivePolicy::MAX_LINES_FIRST)) {
        case TextHeightAdaptivePolicy::MAX_LINES_FIRST:
            result = BuildParagraph(textStyle, textLayoutProperty, contentConstraint, pipeline, layoutWrapper);
            break;
        case TextHeightAdaptivePolicy::MIN_FONT_SIZE_FIRST:
            result = BuildParagraphAdaptUseMinFontSize(
                textStyle, textLayoutProperty, contentConstraint, pipeline, layoutWrapper);
            break;
        case TextHeightAdaptivePolicy::LAYOUT_CONSTRAINT_FIRST:
            result = BuildParagraphAdaptUseLayoutConstraint(
                textStyle, textLayoutProperty, contentConstraint, pipeline, layoutWrapper);
            break;
        default:
            break;
    }
    return result;
}

void TextLayoutAlgorithm::FontRegisterCallback(const RefPtr<FrameNode>& frameNode, const TextStyle& textStyle)
{
    auto callback = [weakNode = WeakPtr<FrameNode>(frameNode)] {
        auto frameNode = weakNode.Upgrade();
        CHECK_NULL_VOID(frameNode);
        frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        auto pattern = frameNode->GetPattern<TextPattern>();
        CHECK_NULL_VOID(pattern);
        auto modifier = DynamicCast<TextContentModifier>(pattern->GetContentModifier());
        CHECK_NULL_VOID(modifier);
        modifier->SetFontReady(true);
    };
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto fontManager = pipeline->GetFontManager();
    if (fontManager) {
        for (const auto& familyName : textStyle.GetFontFamilies()) {
            fontManager->RegisterCallbackNG(frameNode, familyName, callback);
        }
        fontManager->AddVariationNodeNG(frameNode);
    }
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetIsCustomFont(true);
    auto modifier = DynamicCast<TextContentModifier>(pattern->GetContentModifier());
    CHECK_NULL_VOID(modifier);
    modifier->SetIsCustomFont(true);
}

void TextLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    BoxLayoutAlgorithm::Measure(layoutWrapper);
    auto baselineDistance = 0.0f;
    if (paragraph_) {
        baselineDistance = paragraph_->GetAlphabeticBaseline() + std::max(GetBaselineOffset(), 0.0f);
    }
    if (!NearZero(baselineDistance, 0.0f)) {
        baselineDistance += GetContentOffsetY(layoutWrapper);
    }
    layoutWrapper->GetGeometryNode()->SetBaselineDistance(baselineDistance);
}

void TextLayoutAlgorithm::UpdateParagraph(LayoutWrapper* layoutWrapper)
{
    int32_t spanTextLength = GetPreviousLength();
    CHECK_NULL_VOID(layoutWrapper);
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto frameNode = layoutWrapper->GetHostNode();
    const auto& layoutConstrain = layoutProperty->CreateChildConstraint();
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_VOID(pattern);
    auto aiSpanMap = pattern->GetAISpanMap();
    for (const auto& child : spanItemChildren_) {
        if (!child) {
            continue;
        }
        auto imageSpanItem = AceType::DynamicCast<ImageSpanItem>(child);
        if (imageSpanItem) {
            int32_t targetId = imageSpanItem->imageNodeId;
            auto iterItems = children.begin();
            // find the Corresponding ImageNode for every ImageSpanItem
            while (iterItems != children.end() && (*iterItems) && (*iterItems)->GetHostNode()->GetId() != targetId) {
                iterItems++;
            }
            if (iterItems == children.end() || !(*iterItems)) {
                continue;
            }
            (*iterItems)->Measure(layoutConstrain);
            auto verticalAlign = VerticalAlign::BOTTOM;
            auto imageLayoutProperty = DynamicCast<ImageLayoutProperty>((*iterItems)->GetLayoutProperty());
            if (imageLayoutProperty) {
                verticalAlign = imageLayoutProperty->GetVerticalAlign().value_or(VerticalAlign::BOTTOM);
            }
            auto geometryNode = (*iterItems)->GetGeometryNode();
            if (!geometryNode) {
                iterItems++;
                continue;
            }
            auto width = geometryNode->GetMarginFrameSize().Width();
            auto height = geometryNode->GetMarginFrameSize().Height();
            child->placeholderIndex = child->UpdateParagraph(frameNode, paragraph_, width, height, verticalAlign);
            child->content = " ";
            child->position = spanTextLength + 1;
            spanTextLength += 1;
            iterItems++;
        } else if (AceType::InstanceOf<PlaceholderSpanItem>(child)) {
            auto placeholderSpanItem = AceType::DynamicCast<PlaceholderSpanItem>(child);
            if (!placeholderSpanItem) {
                continue;
            }
            int32_t targetId = placeholderSpanItem->placeholderSpanNodeId;
            auto iterItems = children.begin();
            // find the Corresponding ImageNode for every ImageSpanItem
            while (iterItems != children.end() && (*iterItems) && (*iterItems)->GetHostNode()->GetId() != targetId) {
                iterItems++;
            }
            if (iterItems == children.end() || !(*iterItems)) {
                continue;
            }
            (*iterItems)->Measure(layoutConstrain);
            auto geometryNode = (*iterItems)->GetGeometryNode();
            if (!geometryNode) {
                iterItems++;
                continue;
            }
            auto width = geometryNode->GetMarginFrameSize().Width();
            auto height = geometryNode->GetMarginFrameSize().Height();
            child->placeholderIndex = child->UpdateParagraph(frameNode, paragraph_, width, height, VerticalAlign::NONE);
            child->content = " ";
            child->position = spanTextLength + 1;
            spanTextLength += 1;
            iterItems++;
        } else {
            child->aiSpanMap = aiSpanMap;
            child->UpdateParagraph(frameNode, paragraph_);
            aiSpanMap = child->aiSpanMap;
            child->position = spanTextLength + StringUtils::ToWstring(child->content).length();
            spanTextLength += StringUtils::ToWstring(child->content).length();
        }
    }
}

void TextLayoutAlgorithm::UpdateParagraphForAISpan(const TextStyle& textStyle, LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_VOID(pattern);
    auto textCase = textStyle.GetTextCase();
    auto builder = paragraph_;
    auto updateTextAction = [builder, textCase](const std::string content, TextStyle textStyle) {
        if (content.empty()) {
            return;
        }
        auto displayText = content;
        StringUtils::TransformStrCase(displayText, static_cast<int32_t>(textCase));
        builder->PushStyle(textStyle);
        builder->AddText(StringUtils::Str8ToStr16(displayText));
        builder->PopStyle();
    };
    auto textForAI = pattern->GetTextForAI();
    auto wTextForAI = StringUtils::ToWstring(textForAI);
    int32_t wTextForAILength = static_cast<int32_t>(wTextForAI.length());
    int32_t preEnd = 0;
    for (auto kv : pattern->GetAISpanMap()) {
        auto aiSpan = kv.second;
        if (aiSpan.start <= preEnd) {
            TAG_LOGI(AceLogTag::ACE_TEXT, "Error prediction");
            continue;
        }
        if (preEnd < aiSpan.start) {
            auto beforeContent = StringUtils::ToString(wTextForAI.substr(preEnd, aiSpan.start - preEnd));
            updateTextAction(beforeContent, textStyle);
        }
        TextStyle aiSpanTextStyle = textStyle;
        aiSpanTextStyle.SetTextColor(Color::BLUE);
        aiSpanTextStyle.SetTextDecoration(TextDecoration::UNDERLINE);
        aiSpanTextStyle.SetTextDecorationColor(Color::BLUE);
        preEnd = aiSpan.end;
        updateTextAction(aiSpan.content, aiSpanTextStyle);
    }
    if (preEnd < wTextForAILength) {
        auto afterContent = StringUtils::ToString(wTextForAI.substr(preEnd, wTextForAILength - preEnd));
        updateTextAction(afterContent, textStyle);
    }
}

bool TextLayoutAlgorithm::CreateParagraph(const TextStyle& textStyle, std::string content, LayoutWrapper* layoutWrapper)
{
    auto paraStyle = GetParagraphStyle(textStyle, content);
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN) && spanItemChildren_.empty()) {
        paraStyle.fontSize = textStyle.GetFontSize().ConvertToPx();
    }
    paragraph_ = Paragraph::Create(paraStyle, FontCollection::Current());
    CHECK_NULL_RETURN(paragraph_, false);
    paragraph_->PushStyle(textStyle);

    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(frameNode, -1);
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_RETURN(pattern, -1);
    if (spanItemChildren_.empty()) {
        if (pattern->GetTextDetectEnable() && !pattern->GetAISpanMap().empty()) {
            UpdateParagraphForAISpan(textStyle, layoutWrapper);
        } else {
            StringUtils::TransformStrCase(content, static_cast<int32_t>(textStyle.GetTextCase()));
            paragraph_->AddText(StringUtils::Str8ToStr16(content));
        }
    } else {
        UpdateParagraph(layoutWrapper);
    }
    paragraph_->Build();
    return true;
}

bool TextLayoutAlgorithm::CreateParagraphAndLayout(const TextStyle& textStyle, const std::string& content,
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    if (!CreateParagraph(textStyle, content, layoutWrapper)) {
        return false;
    }
    CHECK_NULL_RETURN(paragraph_, false);
    auto maxSize = GetMaxMeasureSize(contentConstraint);
    ApplyIndent(textStyle, maxSize.Width());
    paragraph_->Layout(maxSize.Width());

    bool ret = CreateImageSpanAndLayout(textStyle, content, contentConstraint, layoutWrapper);
    return ret;
}

OffsetF TextLayoutAlgorithm::GetContentOffset(LayoutWrapper* layoutWrapper)
{
    OffsetF contentOffset(0.0, 0.0);
    CHECK_NULL_RETURN(layoutWrapper, contentOffset);

    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0);
    auto top = padding.top.value_or(0);
    auto paddingOffset = OffsetF(left, top);
    auto align = Alignment::CENTER;
    if (layoutWrapper->GetLayoutProperty()->GetPositionProperty()) {
        align = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().value_or(align);
    }

    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    if (content) {
        contentOffset = Alignment::GetAlignPosition(size, content->GetRect().GetSize(), align) + paddingOffset;
        content->SetOffset(contentOffset);
    }
    return contentOffset;
}

void TextLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto contentOffset = GetContentOffset(layoutWrapper);
    std::vector<int32_t> placeholderIndex;
    for (const auto& child : spanItemChildren_) {
        if (!child) {
            continue;
        }
        if (AceType::InstanceOf<ImageSpanItem>(child) || AceType::InstanceOf<PlaceholderSpanItem>(child)) {
            placeholderIndex.emplace_back(child->placeholderIndex);
        }
    }
    if (spanItemChildren_.empty() || placeholderIndex.empty()) {
        return;
    }

    size_t index = 0;
    std::vector<RectF> rectsForPlaceholders;
    GetPlaceholderRects(rectsForPlaceholders);
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    // children only contains the image span.
    for (const auto& child : children) {
        if (!child) {
            ++index;
            continue;
        }
        if (index >= placeholderIndex.size() ||
            (index >= rectsForPlaceholders.size() && child->GetHostTag() != V2::PLACEHOLDER_SPAN_ETS_TAG)) {
            return;
        }
        auto rect = rectsForPlaceholders.at(index);
        auto geometryNode = child->GetGeometryNode();
        if (!geometryNode) {
            ++index;
            continue;
        }
        geometryNode->SetMarginFrameOffset(contentOffset + OffsetF(rect.Left(), rect.Top()));
        child->Layout();
        ++index;
    }

#ifdef ENABLE_DRAG_FRAMEWORK
    auto frameNode = layoutWrapper->GetHostNode();
    CHECK_NULL_VOID(frameNode);
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->InitSpanImageLayout(placeholderIndex, rectsForPlaceholders, contentOffset);
#endif
}

bool TextLayoutAlgorithm::AdaptMinTextSize(TextStyle& textStyle, const std::string& content,
    const LayoutConstraintF& contentConstraint, const RefPtr<PipelineContext>& pipeline, LayoutWrapper* layoutWrapper)
{
    double maxFontSize = 0.0;
    double minFontSize = 0.0;
    if (!textStyle.GetAdaptMaxFontSize().NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(),
            pipeline->GetLogicScale(), contentConstraint.maxSize.Height(), maxFontSize)) {
        return false;
    }
    if (!textStyle.GetAdaptMinFontSize().NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(),
            pipeline->GetLogicScale(), contentConstraint.maxSize.Height(), minFontSize)) {
        return false;
    }
    if (LessNotEqual(maxFontSize, minFontSize) || LessOrEqual(minFontSize, 0.0)) {
        if (!CreateParagraphAndLayout(textStyle, content, contentConstraint, layoutWrapper)) {
            return false;
        }
        return true;
    }
    constexpr Dimension ADAPT_UNIT = 1.0_fp;
    Dimension step = ADAPT_UNIT;
    if (GreatNotEqual(textStyle.GetAdaptFontSizeStep().Value(), 0.0)) {
        step = textStyle.GetAdaptFontSizeStep();
    }
    double stepSize = 0.0;
    if (!step.NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(), pipeline->GetLogicScale(),
            contentConstraint.maxSize.Height(), stepSize)) {
        return false;
    }
    auto maxSize = GetMaxMeasureSize(contentConstraint);
    while (GreatOrEqual(maxFontSize, minFontSize)) {
        textStyle.SetFontSize(Dimension(maxFontSize));
        if (!CreateParagraphAndLayout(textStyle, content, contentConstraint, layoutWrapper)) {
            return false;
        }
        if (!DidExceedMaxLines(maxSize)) {
            break;
        }
        maxFontSize -= stepSize;
    }
    return true;
}

bool TextLayoutAlgorithm::DidExceedMaxLines(const SizeF& maxSize)
{
    CHECK_NULL_RETURN(paragraph_, false);
    bool didExceedMaxLines = paragraph_->DidExceedMaxLines();
    didExceedMaxLines = didExceedMaxLines || GreatNotEqual(paragraph_->GetHeight(), maxSize.Height());
    didExceedMaxLines = didExceedMaxLines || GreatNotEqual(paragraph_->GetLongestLine(), maxSize.Width());
    return didExceedMaxLines;
}

TextDirection TextLayoutAlgorithm::GetTextDirection(const std::string& content)
{
    TextDirection textDirection = TextDirection::LTR;
    auto showingTextForWString = StringUtils::ToWstring(content);
    for (const auto& charOfShowingText : showingTextForWString) {
        if (TextLayoutadapter::IsLeftToRight(charOfShowingText)) {
            return TextDirection::LTR;
        } else if (TextLayoutadapter::IsRightToLeft(charOfShowingText)) {
            return TextDirection::RTL;
        } else if (TextLayoutadapter::IsRightTOLeftArabic(charOfShowingText)) {
            return TextDirection::RTL;
        }
    }
    return textDirection;
}

float TextLayoutAlgorithm::GetTextWidth() const
{
    CHECK_NULL_RETURN(paragraph_, 0.0);
    return paragraph_->GetTextWidth();
}

const RefPtr<Paragraph>& TextLayoutAlgorithm::GetParagraph()
{
    return paragraph_;
}

float TextLayoutAlgorithm::GetBaselineOffset() const
{
    return baselineOffset_;
}

SizeF TextLayoutAlgorithm::GetMaxMeasureSize(const LayoutConstraintF& contentConstraint) const
{
    auto maxSize = contentConstraint.selfIdealSize;
    maxSize.UpdateIllegalSizeWithCheck(contentConstraint.maxSize);
    return maxSize.ConvertToSizeT();
}

std::list<RefPtr<SpanItem>>&& TextLayoutAlgorithm::GetSpanItemChildren()
{
    return std::move(spanItemChildren_);
}

bool TextLayoutAlgorithm::BuildParagraph(TextStyle& textStyle, const RefPtr<TextLayoutProperty>& layoutProperty,
    const LayoutConstraintF& contentConstraint, const RefPtr<PipelineContext>& pipeline, LayoutWrapper* layoutWrapper)
{
    if (!textStyle.GetAdaptTextSize()) {
        if (!CreateParagraphAndLayout(
                textStyle, layoutProperty->GetContent().value_or(""), contentConstraint, layoutWrapper)) {
            return false;
        }
    } else {
        if (!AdaptMinTextSize(
                textStyle, layoutProperty->GetContent().value_or(""), contentConstraint, pipeline, layoutWrapper)) {
            return false;
        }
    }

    // Confirmed specification: The width of the text paragraph covers the width of the component, so this code is
    // generally not allowed to be modified
    if (!contentConstraint.selfIdealSize.Width()) {
        float paragraphNewWidth = std::min(GetTextWidth(), paragraph_->GetMaxWidth());
        paragraphNewWidth =
            std::clamp(paragraphNewWidth, contentConstraint.minSize.Width(), contentConstraint.maxSize.Width());
        if (!NearEqual(paragraphNewWidth, paragraph_->GetMaxWidth())) {
            paragraph_->Layout(std::ceil(paragraphNewWidth));
        }
    }
    return true;
}

bool TextLayoutAlgorithm::BuildParagraphAdaptUseMinFontSize(TextStyle& textStyle,
    const RefPtr<TextLayoutProperty>& layoutProperty, const LayoutConstraintF& contentConstraint,
    const RefPtr<PipelineContext>& pipeline, LayoutWrapper* layoutWrapper)
{
    if (!AdaptMaxTextSize(
            textStyle, layoutProperty->GetContent().value_or(""), contentConstraint, pipeline, layoutWrapper)) {
        return false;
    }

    // Confirmed specification: The width of the text paragraph covers the width of the component, so this code is
    // generally not allowed to be modified
    if (!contentConstraint.selfIdealSize.Width()) {
        float paragraphNewWidth = std::min(GetTextWidth(), paragraph_->GetMaxWidth());
        paragraphNewWidth =
            std::clamp(paragraphNewWidth, contentConstraint.minSize.Width(), contentConstraint.maxSize.Width());
        if (!NearEqual(paragraphNewWidth, paragraph_->GetMaxWidth())) {
            paragraph_->Layout(std::ceil(paragraphNewWidth));
        }
    }

    return true;
}

bool TextLayoutAlgorithm::BuildParagraphAdaptUseLayoutConstraint(TextStyle& textStyle,
    const RefPtr<TextLayoutProperty>& layoutProperty, const LayoutConstraintF& contentConstraint,
    const RefPtr<PipelineContext>& pipeline, LayoutWrapper* layoutWrapper)
{
    // Create the paragraph and obtain the height.
    if (!BuildParagraph(textStyle, layoutProperty, contentConstraint, pipeline, layoutWrapper)) {
        return false;
    }
    auto height = static_cast<float>(paragraph_->GetHeight());
    double minTextSizeHeight = 0.0;
    textStyle.GetAdaptMinFontSize().NormalizeToPx(
        pipeline->GetDipScale(), pipeline->GetFontScale(), pipeline->GetLogicScale(), height, minTextSizeHeight);
    if (LessOrEqual(minTextSizeHeight, 0.0)) {
        textStyle.GetFontSize().NormalizeToPx(
            pipeline->GetDipScale(), pipeline->GetFontScale(), pipeline->GetLogicScale(), height, minTextSizeHeight);
    }
    if (textStyle.GetMaxLines() == UINT32_MAX) {
        double baselineOffset = 0.0;
        textStyle.GetBaselineOffset().NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(),
            pipeline->GetLogicScale(), contentConstraint.maxSize.Height(), baselineOffset);
        double lineHeight = minTextSizeHeight;
        if (textStyle.HasHeightOverride()) {
            textStyle.GetLineHeight().NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(),
                pipeline->GetLogicScale(), minTextSizeHeight, lineHeight);
        }
        uint32_t maxLines = (contentConstraint.maxSize.Height() - baselineOffset - minTextSizeHeight) / (lineHeight);
        textStyle.SetMaxLines(maxLines);
        textStyle.DisableAdaptTextSize();

        if (!BuildParagraph(textStyle, layoutProperty, contentConstraint, pipeline, layoutWrapper)) {
            return false;
        }
    }
    // Reducing the value of MaxLines to make sure the parapraph could be layout in the constraint of height.
    height = static_cast<float>(paragraph_->GetHeight());
    while (GreatNotEqual(height, contentConstraint.maxSize.Height())) {
        auto maxLines = textStyle.GetMaxLines();
        if (maxLines == 0) {
            break;
        } else {
            maxLines = textStyle.GetMaxLines() - 1;
            textStyle.SetMaxLines(maxLines);
        }
        if (!BuildParagraph(textStyle, layoutProperty, contentConstraint, pipeline, layoutWrapper)) {
            return false;
        }
        height = static_cast<float>(paragraph_->GetHeight());
    }
    return true;
}

std::optional<SizeF> TextLayoutAlgorithm::BuildTextRaceParagraph(TextStyle& textStyle,
    const RefPtr<TextLayoutProperty>& layoutProperty, const LayoutConstraintF& contentConstraint,
    const RefPtr<PipelineContext>& pipeline, LayoutWrapper* layoutWrapper)
{
    // create a paragraph with all text in 1 line
    textStyle.SetTextOverflow(TextOverflow::CLIP);
    textStyle.SetMaxLines(1);
    textStyle.SetTextAlign(TextAlign::START);
    if (!CreateParagraph(textStyle, layoutProperty->GetContent().value_or(""), layoutWrapper)) {
        return std::nullopt;
    }
    if (!paragraph_) {
        return std::nullopt;
    }

    // layout the paragraph to the width of text
    paragraph_->Layout(std::numeric_limits<float>::max());
    float paragraphWidth = paragraph_->GetMaxWidth();
    paragraph_->Layout(paragraphWidth);

    textStyle_ = textStyle;

    // calculate the content size
    auto height = static_cast<float>(paragraph_->GetHeight());
    double baselineOffset = 0.0;
    if (layoutProperty->GetBaselineOffsetValue(Dimension())
            .NormalizeToPx(
                pipeline->GetDipScale(), pipeline->GetFontScale(), pipeline->GetLogicScale(), height, baselineOffset)) {
        baselineOffset_ = static_cast<float>(baselineOffset);
    }
    float heightFinal =
        std::min(static_cast<float>(height + std::fabs(baselineOffset)), contentConstraint.maxSize.Height());

    float widthFinal = paragraphWidth;
    if (contentConstraint.selfIdealSize.Width().has_value()) {
        if (contentConstraint.selfIdealSize.Width().value() < paragraphWidth) {
            widthFinal = contentConstraint.selfIdealSize.Width().value();
        }
    } else if (contentConstraint.maxSize.Width() < paragraphWidth) {
        widthFinal = contentConstraint.maxSize.Width();
    }
    return SizeF(widthFinal, heightFinal);
}

void TextLayoutAlgorithm::SetPropertyToModifier(
    const RefPtr<TextLayoutProperty>& layoutProperty, RefPtr<TextContentModifier> modifier)
{
    auto fontFamily = layoutProperty->GetFontFamily();
    if (fontFamily.has_value()) {
        modifier->SetFontFamilies(fontFamily.value());
    }
    auto fontSize = layoutProperty->GetFontSize();
    if (fontSize.has_value()) {
        modifier->SetFontSize(fontSize.value());
    }
    auto fontWeight = layoutProperty->GetFontWeight();
    if (fontWeight.has_value()) {
        modifier->SetFontWeight(fontWeight.value());
    }
    auto textColor = layoutProperty->GetTextColor();
    if (textColor.has_value()) {
        modifier->SetTextColor(textColor.value());
    }
    auto textShadow = layoutProperty->GetTextShadow();
    if (textShadow.has_value()) {
        modifier->SetTextShadow(textShadow.value());
    }
    auto textDecorationColor = layoutProperty->GetTextDecorationColor();
    if (textDecorationColor.has_value()) {
        modifier->SetTextDecorationColor(textDecorationColor.value());
    }
    auto textDecorationStyle = layoutProperty->GetTextDecorationStyle();
    if (textDecorationStyle.has_value()) {
        modifier->SetTextDecorationStyle(textDecorationStyle.value());
    }
    auto textDecoration = layoutProperty->GetTextDecoration();
    if (textDecoration.has_value()) {
        modifier->SetTextDecoration(textDecoration.value());
    }
    auto baselineOffset = layoutProperty->GetBaselineOffset();
    if (baselineOffset.has_value()) {
        modifier->SetBaselineOffset(baselineOffset.value());
    }
}

bool TextLayoutAlgorithm::AdaptMaxTextSize(TextStyle& textStyle, const std::string& content,
    const LayoutConstraintF& contentConstraint, const RefPtr<PipelineContext>& pipeline, LayoutWrapper* layoutWrapper)
{
    double maxFontSize = 0.0;
    double minFontSize = 0.0;
    if (!textStyle.GetAdaptMaxFontSize().NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(),
            pipeline->GetLogicScale(), contentConstraint.maxSize.Height(), maxFontSize)) {
        return false;
    }
    if (!textStyle.GetAdaptMinFontSize().NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(),
            pipeline->GetLogicScale(), contentConstraint.maxSize.Height(), minFontSize)) {
        return false;
    }
    if (LessNotEqual(maxFontSize, minFontSize) || LessOrEqual(minFontSize, 0.0)) {
        // minFontSize or maxFontSize is invalid
        if (!CreateParagraphAndLayout(textStyle, content, contentConstraint, layoutWrapper)) {
            return false;
        }
        return true;
    }
    constexpr Dimension ADAPT_UNIT = 1.0_fp;
    Dimension step = ADAPT_UNIT;
    if (GreatNotEqual(textStyle.GetAdaptFontSizeStep().Value(), 0.0)) {
        step = textStyle.GetAdaptFontSizeStep();
    }
    double stepSize = 0.0;
    if (!step.NormalizeToPx(pipeline->GetDipScale(), pipeline->GetFontScale(), pipeline->GetLogicScale(),
            contentConstraint.maxSize.Height(), stepSize)) {
        return false;
    }
    RefPtr<Paragraph> paragraph;
    auto maxSize = GetMaxMeasureSize(contentConstraint);
    // Use the minFontSize to layout the paragraph. While using the minFontSize, if the paragraph could be layout in 1
    // line, then increase the font size and try to layout using the maximum available fontsize.
    while (LessOrEqual(minFontSize, maxFontSize)) {
        textStyle.SetFontSize(Dimension(minFontSize));
        if (!CreateParagraphAndLayout(textStyle, content, contentConstraint, layoutWrapper)) {
            return false;
        }
        if (paragraph_->GetLineCount() > 1 || paragraph_->DidExceedMaxLines() ||
            GreatNotEqual(paragraph_->GetLongestLine(), maxSize.Width())) {
            if (paragraph != nullptr) {
                paragraph_ = paragraph;
            }
            break;
        }
        minFontSize += stepSize;
        paragraph = paragraph_;
    }
    return true;
}

std::optional<TextStyle> TextLayoutAlgorithm::GetTextStyle() const
{
    return textStyle_;
}

void TextLayoutAlgorithm::UpdateTextColorIfForeground(const RefPtr<FrameNode>& frameNode, TextStyle& textStyle)
{
    auto renderContext = frameNode->GetRenderContext();
    if (renderContext->HasForegroundColor()) {
        if (renderContext->GetForegroundColorValue().GetValue() != textStyle.GetTextColor().GetValue()) {
            textStyle.SetTextColor(Color::FOREGROUND);
        }
    } else if (renderContext->HasForegroundColorStrategy()) {
        textStyle.SetTextColor(Color::FOREGROUND);
    }
}

void TextLayoutAlgorithm::ApplyIndent(const TextStyle& textStyle, double width)
{
    if (LessOrEqual(textStyle.GetTextIndent().Value(), 0.0)) {
        return;
    }
    // first line indent
    CHECK_NULL_VOID(paragraph_);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    double indent = 0.0;
    if (textStyle.GetTextIndent().Unit() != DimensionUnit::PERCENT) {
        if (!textStyle.GetTextIndent().NormalizeToPx(
                pipeline->GetDipScale(), pipeline->GetFontScale(), pipeline->GetLogicScale(), width, indent)) {
            return;
        }
    } else {
        indent = width * textStyle.GetTextIndent().Value();
    }
    std::vector<float> indents;
    // only indent first line
    indents.emplace_back(static_cast<float>(indent));
    indents.emplace_back(0.0);
    paragraph_->SetIndents(indents);
}

bool TextLayoutAlgorithm::IncludeImageSpan(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_RETURN(layoutWrapper, false);
    return (!layoutWrapper->GetAllChildrenWithBuild().empty());
}

void TextLayoutAlgorithm::GetSpanAndImageSpanList(std::list<RefPtr<SpanItem>>& spanList,
    std::map<int32_t, std::pair<RectF, RefPtr<PlaceholderSpanItem>>>& placeholderSpanList)
{
    std::vector<RectF> rectsForPlaceholders;
    paragraph_->GetRectsForPlaceholders(rectsForPlaceholders);
    for (const auto& child : spanItemChildren_) {
        if (!child) {
            continue;
        }
        auto imageSpanItem = AceType::DynamicCast<ImageSpanItem>(child);
        if (imageSpanItem) {
            int32_t index = child->placeholderIndex;
            if (index >= 0 && index < static_cast<int32_t>(rectsForPlaceholders.size())) {
                placeholderSpanList.emplace(index, std::make_pair(rectsForPlaceholders.at(index), imageSpanItem));
            }
        } else if (auto placeholderSpanItem = AceType::DynamicCast<PlaceholderSpanItem>(child); placeholderSpanItem) {
            int32_t index = child->placeholderIndex;
            if (index >= 0 && index < static_cast<int32_t>(rectsForPlaceholders.size())) {
                placeholderSpanList.emplace(index, std::make_pair(rectsForPlaceholders.at(index), placeholderSpanItem));
            }
        } else {
            spanList.emplace_back(child);
        }
    }
}

void TextLayoutAlgorithm::SplitSpanContentByLines(const TextStyle& textStyle,
    const std::list<RefPtr<SpanItem>>& spanList,
    std::map<int32_t, std::pair<RectF, std::list<RefPtr<SpanItem>>>>& spanContentLines)
{
    int32_t currentLine = 0;
    size_t currentLength = 0;
    for (const auto& child : spanList) {
        if (!child) {
            continue;
        }
        std::string textValue = child->content;
        std::vector<RectF> selectedRects;
        if (!textValue.empty()) {
            paragraph_->GetRectsForRange(currentLength, currentLength + textValue.size(), selectedRects);
        }
        currentLength += textValue.size();
        RectF currentRect;
        auto preLinetLastSpan = spanContentLines.rbegin();
        double preLineFontSize = textStyle.GetFontSize().Value();
        if (preLinetLastSpan != spanContentLines.rend()) {
            currentRect = preLinetLastSpan->second.first;
            if (preLinetLastSpan->second.second.back() && preLinetLastSpan->second.second.back()->fontStyle &&
                preLinetLastSpan->second.second.back()->fontStyle->GetFontSize().has_value()) {
                preLineFontSize = preLinetLastSpan->second.second.back()->fontStyle->GetFontSize().value().Value();
            }
        }
        for (const auto& rect : selectedRects) {
            if (!NearEqual(currentRect.GetOffset().GetY(), rect.GetOffset().GetY())) {
                currentRect = rect;
                ++currentLine;
            }

            auto iter = spanContentLines.find(currentLine);
            if (iter != spanContentLines.end()) {
                auto iterSecond = std::find(iter->second.second.begin(), iter->second.second.end(), child);
                if (iterSecond != iter->second.second.end()) {
                    continue;
                }
                if (NearEqual(rect.GetOffset().GetY(), currentRect.GetOffset().GetY()) && child->fontStyle &&
                    child->fontStyle->GetFontSize().has_value() &&
                    NearEqual(preLineFontSize, child->fontStyle->GetFontSize().value().Value())) {
                    continue;
                }
                iter->second.second.emplace_back(child);
            } else {
                std::list<RefPtr<SpanItem>> spanLineList;
                spanLineList.emplace_back(child);
                spanContentLines.emplace(currentLine, std::make_pair(currentRect, spanLineList));
            }
        }
    }
}

void TextLayoutAlgorithm::SetImageSpanTextStyleByLines(const TextStyle& textStyle,
    std::map<int32_t, std::pair<RectF, RefPtr<PlaceholderSpanItem>>>& placeholderSpanList,
    std::map<int32_t, std::pair<RectF, std::list<RefPtr<SpanItem>>>>& spanContentLines)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);

    auto placeholderItem = placeholderSpanList.begin();
    for (auto spanItem = spanContentLines.begin(); spanItem != spanContentLines.end(); spanItem++) {
        for (; placeholderItem != placeholderSpanList.end();) {
            auto placeholder = placeholderItem->second.second;
            if (!placeholder) {
                continue;
            }
            placeholder->textStyle = textStyle;

            auto offset = placeholderItem->second.first.GetOffset();
            auto placeholderItemRect = placeholderItem->second.first;
            placeholderItemRect.SetOffset(OffsetF(spanItem->second.first.GetOffset().GetX(), offset.GetY()));
            bool isIntersectWith = spanItem->second.first.IsIntersectWith(placeholderItemRect);
            if (!isIntersectWith) {
                break;
            }
            Dimension maxFontSize;
            TextStyle spanTextStyle = textStyle;
            for (const auto& child : spanItem->second.second) {
                if (!child || !child->fontStyle) {
                    continue;
                }
                if (!child->fontStyle->GetFontSize().has_value()) {
                    continue;
                }
                if (LessNotEqual(maxFontSize.Value(), child->fontStyle->GetFontSize().value().Value())) {
                    maxFontSize = child->fontStyle->GetFontSize().value();
                    spanTextStyle = CreateTextStyleUsingTheme(
                        child->fontStyle, child->textLineStyle, pipelineContext->GetTheme<TextTheme>());
                }
            }
            placeholder->textStyle = spanTextStyle;
            placeholderItem++;
        }
    }
}

void TextLayoutAlgorithm::SetImageSpanTextStyle(const TextStyle& textStyle)
{
    CHECK_NULL_VOID(paragraph_);

    std::list<RefPtr<SpanItem>> spanList;
    std::map<int32_t, std::pair<RectF, RefPtr<PlaceholderSpanItem>>> placeholderList;
    GetSpanAndImageSpanList(spanList, placeholderList);

    // split text content by lines
    std::map<int32_t, std::pair<RectF, std::list<RefPtr<SpanItem>>>> spanContentLines;
    SplitSpanContentByLines(textStyle, spanList, spanContentLines);

    // set imagespan textstyle
    SetImageSpanTextStyleByLines(textStyle, placeholderList, spanContentLines);
}

bool TextLayoutAlgorithm::CreateImageSpanAndLayout(const TextStyle& textStyle, const std::string& content,
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    bool includeImageSpan = IncludeImageSpan(layoutWrapper);
    if (includeImageSpan) {
        SetImageSpanTextStyle(textStyle);
        if (!CreateParagraph(textStyle, content, layoutWrapper)) {
            return false;
        }
        CHECK_NULL_RETURN(paragraph_, false);
        auto maxSize = GetMaxMeasureSize(contentConstraint);
        ApplyIndent(textStyle, maxSize.Width());
        paragraph_->Layout(maxSize.Width());
    }
    return true;
}

size_t TextLayoutAlgorithm::GetLineCount() const
{
    CHECK_NULL_RETURN(paragraph_, 0);
    return paragraph_->GetLineCount();
}

void TextLayoutAlgorithm::GetPlaceholderRects(std::vector<RectF>& rects)
{
    CHECK_NULL_VOID(paragraph_);
    paragraph_->GetRectsForPlaceholders(rects);
}

ParagraphStyle TextLayoutAlgorithm::GetParagraphStyle(const TextStyle& textStyle, const std::string& content) const
{
    return {
        .direction = GetTextDirection(content),
        .align = textStyle.GetTextAlign(),
        .maxLines = textStyle.GetMaxLines(),
        .fontLocale = Localization::GetInstance()->GetFontLocale(),
        .wordBreak = textStyle.GetWordBreak(),
        .ellipsisMode = textStyle.GetEllipsisMode(),
        .textOverflow = textStyle.GetTextOverflow(),
    };
}
} // namespace OHOS::Ace::NG
