/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FIELD_FLUTTER_RENDER_TEXT_FIELD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FIELD_FLUTTER_RENDER_TEXT_FIELD_H

#include <memory>
#include <string>

#include "core/components/common/properties/decoration.h"
#include "core/components/text_field/render_text_field.h"
#include "core/pipeline/layers/clip_layer.h"

#ifndef USE_GRAPHIC_TEXT_GINE
namespace txt {
class FontCollection;
class Paragraph;
class ParagraphStyle;
class TextStyle;
} // namespace txt
#else
namespace OHOS::Rosen {
class FontCollection;
class Typography;
struct TypographyStyle;
struct TextStyle;
} // namespace OHOS::Rosen
#endif

namespace flutter {
class Canvas;
}

namespace OHOS::Ace {

class Component;

class FlutterRenderTextField final : public RenderTextField {
    DECLARE_ACE_TYPE(FlutterRenderTextField, RenderTextField);

public:
    FlutterRenderTextField() = default;
    ~FlutterRenderTextField() override = default;

    void Paint(RenderContext& context, const Offset& offset) override;
    RenderLayer GetRenderLayer() override;

    const Rect& GetCaretRect() const
    {
        return caretRect_;
    }

    const Rect& GetStartRect() const
    {
        return startCaretRect_;
    }

    const SkBitmap& GetBitmap() const
    {
        return canvasCache_;
    }

protected:
    int32_t GetCursorPositionForMoveUp() override;
    int32_t GetCursorPositionForMoveDown() override;
    int32_t GetCursorPositionForClick(const Offset& offset) override;
    int32_t AdjustCursorAndSelection(int32_t currentCursorPosition) override;
    DirectionStatus GetDirectionStatusOfPosition(int32_t position) const override;
    Offset GetHandleOffset(int32_t extend) override;
    Size ComputeDeflateSizeOfErrorAndCountText() const override;
    void ResetStatus() override;

private:
#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::ParagraphStyle> CreateParagraphStyle(bool isErrorText = false);
    std::unique_ptr<txt::TextStyle> CreateTextStyle(const TextStyle& style, bool isPlaceholder = false);
#else
    std::unique_ptr<Rosen::TypographyStyle> CreateParagraphStyle(bool isErrorText = false);
    std::unique_ptr<Rosen::TextStyle> CreateTextStyle(const TextStyle& style, bool isPlaceholder = false);
#endif

    double PreferredLineHeight() override;
    void UpdateCaretProto();
    // Case of upstream affinity, which means the caret size should fit with the glyphs before [extent].
    bool ComputeOffsetForCaretUpstream(int32_t extent, CaretMetrics& result) const;
    bool ComputeOffsetForCaretDownstream(int32_t extent, CaretMetrics& result) const;
    bool ComputeOffsetForCaretCloserToClick(int32_t extent, CaretMetrics& result) const;
    // Make an offset when no text exists. The position of caret depends on the [textAlign_] && [textDirection_].
    Offset MakeEmptyOffset() const;
    Size Measure() override;
#ifndef USE_GRAPHIC_TEXT_GINE
    double MeasureParagraph(
        const std::unique_ptr<txt::ParagraphStyle>& paragraphStyle, std::unique_ptr<txt::TextStyle>& txtStyle);
#else
    double MeasureParagraph(
        const std::unique_ptr<Rosen::TypographyStyle>& paragraphStyle, std::unique_ptr<Rosen::TextStyle>& txtStyle);
#endif
    Size ComputeLayoutSize(const Size& size, double decorationHeight);

    Rect GetInnerRect(const Decoration& decoration, const Rect& outer, double dipScale) const;
    bool GetCaretRect(int32_t extent, Rect& caretRect, double caretHeightOffset = 0.0) const override;
    // Compute the offset of caret and text. Must be called after paragraph built.
    void ComputeOffsetAfterLayout();
    // Compute the offset to align text and icon to vertical center.
    Offset ComputeVerticalOffsetForCenter(double outerHeight, double innerHeight) const;
#ifndef USE_GRAPHIC_TEXT_GINE
    void SetShaderIfNeeded(std::unique_ptr<txt::ParagraphStyle> paragraphStyle,
        std::unique_ptr<txt::TextStyle> txtStyle, double textAreaWidth);
#else
    void SetShaderIfNeeded(std::unique_ptr<Rosen::TypographyStyle> paragraphStyle,
        std::unique_ptr<Rosen::TextStyle> txtStyle, double textAreaWidth);
#endif
    sk_sp<SkShader> MakeGradientShader(double shadeWidth) const;
#ifndef USE_GRAPHIC_TEXT_GINE
    std::shared_ptr<txt::FontCollection> GetFontCollection();
#else
    std::shared_ptr<Rosen::FontCollection> GetFontCollection();
#endif
    void ResetParagraphIfNeeded();
    void ComputeExtendHeight(double decorationHeight);
    double GetBoundaryOfParagraph(bool isLeftBoundary) const;
    bool AdjustCursorAndSelectionForLtr(bool isBeforeCharRtl, bool isAfterCharRtl,
        const std::wstring& textBeforeCursor, const std::wstring& textAfterCursor, int32_t& result);
    bool AdjustCursorAndSelectionForRtl(bool isBeforeCharRtl, bool isAfterCharRtl,
        const std::wstring& textBeforeCursor, const std::wstring& textAfterCursor, int32_t& result);
    bool NeedAdjustPosition(const std::wstring& textBeforeCursor);

    // Paint cursor at the extent position. When [affinity] supported, extends this function.
    void PaintCaret(SkCanvas& canvas, const Rect& caretRect);
    void PaintDecoration(const Offset& offset, SkCanvas* canvas, const Size& size, RenderContext& context);
    void PaintSelectCaret(SkCanvas* canvas);
    void PaintIcon(const Offset& offset, RenderContext& context);
    void PaintSelection(SkCanvas* canvas) const;
    void DrawSelection(unsigned start, unsigned end, SkCanvas* canvas) const;
#if defined(IOS_PLATFORM)
    void PaintCompose(SkCanvas* canvas) const;
#endif
    void PaintTextAndPlaceholder(SkCanvas* canvas) const;
    void PaintErrorText(SkCanvas* canvas) const;
    void PaintCountText(SkCanvas* canvas) const;
    void PaintOverlayForHoverAndPress(const Offset& offset, SkCanvas* canvas) const;
    void PaintTextField(const Offset& offset, RenderContext& context, SkCanvas* canvas, bool isMagnifier = false);
    SkVector GetSkRadii(const Radius& radius) const;
    void PaintFocus(const Offset& offset, const Size& widthHeight, RenderContext& context);

#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::Paragraph> paragraph_;
    std::unique_ptr<txt::Paragraph> errorParagraph_;
    std::unique_ptr<txt::Paragraph> countParagraph_;
    std::unique_ptr<txt::Paragraph> placeholderParagraph_;
#else
    std::unique_ptr<Rosen::Typography> paragraph_;
    std::unique_ptr<Rosen::Typography> errorParagraph_;
    std::unique_ptr<Rosen::Typography> countParagraph_;
    std::unique_ptr<Rosen::Typography> placeholderParagraph_;
#endif
    // Used to estimate size.
#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::Paragraph> template_;
#else
    std::unique_ptr<Rosen::Typography> template_;
#endif

    Rect caretRect_;
    Rect startCaretRect_;
    Size lastLayoutSize_;
    double originInnerWidth_ = 0.0;

    RefPtr<Flutter::ClipLayer> layer_;
    SkBitmap canvasCache_{};
    std::unique_ptr<SkCanvas> magnifierCanvas_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FIELD_FLUTTER_RENDER_TEXT_FIELD_H
