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

#include "core/components/multimodal/flutter_render_multimodal.h"

#include <cstdint>

#ifndef USE_GRAPHIC_TEXT_GINE
#include "txt/paragraph_builder.h"
#include "txt/paragraph_style.h"
#else
#include "rosen_text/typography_create.h"
#include "rosen_text/typography_style.h"
#endif

#include "base/i18n/localization.h"
#include "base/utils/string_utils.h"
#include "core/components/calendar/flutter_render_calendar.h"
#include "core/components/font/constants_converter.h"
#include "core/components/font/flutter_font_collection.h"
#include "core/pipeline/base/scoped_canvas_state.h"

namespace OHOS::Ace {
namespace {

constexpr double DEFAULT_SIZE = 20.0;
constexpr double LIMIT_WIDTH = 18.0;
constexpr double CORNER_RADIUS = 8.0;
constexpr double FONT_SIZE = 14;
const char ELLIPSIS[] = "...";

} // namespace

void FlutterRenderMultimodal::Paint(RenderContext& context, const Offset& offset)
{
    RenderNode::Paint(context, offset);

    if (!isSubscriptShow_) {
        return;
    }

    auto canvas = ScopedCanvas::Create(context);
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    SkCanvas* skCanvas = canvas->canvas();
    if (skCanvas == nullptr) {
        LOGE("skCanvas fetch failed");
        return;
    }

    auto width = NormalizeToPx(Dimension(DEFAULT_SIZE, DimensionUnit::VP));
    auto height = NormalizeToPx(Dimension(DEFAULT_SIZE, DimensionUnit::VP));
    auto corner = NormalizeToPx(Dimension(CORNER_RADIUS, DimensionUnit::VP));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(Color::FromARGB(230, 0, 0, 0).GetValue());

    UpdateParagraph(offset, subscript_.GetVoiceContent());
    paragraph_->Layout(GetLayoutSize().Width());
#ifndef USE_GRAPHIC_TEXT_GINE
    if (paragraph_->GetLongestLine() > NormalizeToPx(Dimension(LIMIT_WIDTH, DimensionUnit::VP))) {
        width = height + (paragraph_->GetLongestLine() - NormalizeToPx(Dimension(LIMIT_WIDTH, DimensionUnit::VP)));
    }
#else
    if (paragraph_->GetActualWidth() > NormalizeToPx(Dimension(LIMIT_WIDTH, DimensionUnit::VP))) {
        width = height + (paragraph_->GetActualWidth() - NormalizeToPx(Dimension(LIMIT_WIDTH, DimensionUnit::VP)));
    }
#endif

    SkVector radii[] = { { corner, corner }, { 0, 0 }, { corner, corner }, { 0, 0 } };
    SkRRect rrect;
    rrect.setRectRadii(SkRect::MakeXYWH(offset.GetX(), offset.GetY(), width, height), radii);
    skCanvas->drawRRect(rrect, paint);

#ifndef USE_GRAPHIC_TEXT_GINE
    auto leftOffset = paragraph_->GetLongestLine() / 2;
#else
    auto leftOffset = paragraph_->GetActualWidth() / 2;
#endif
    auto centerX = offset.GetX() + width / 2;
    auto centerY = offset.GetY() + height / 2;
    paragraph_->Paint(skCanvas, centerX - leftOffset, centerY - paragraph_->GetHeight() / 2);
}

void FlutterRenderMultimodal::UpdateParagraph(const Offset& offset, const std::string& text)
{
    using namespace Constants;
#ifndef USE_GRAPHIC_TEXT_GINE
    txt::ParagraphStyle style;
    style.max_lines = 1;
    style.ellipsis = StringUtils::Str8ToStr16(ELLIPSIS);
#else
    Rosen::TypographyStyle style;
    style.maxLines = 1;
    style.ellipsis= StringUtils::Str8ToStr16(ELLIPSIS);
#endif
    auto fontCollection = FlutterFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("UpdateParagraph: fontCollection is null");
        return;
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::ParagraphBuilder> builder =
        txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    txtStyle.font_size = NormalizeToPx(Dimension(FONT_SIZE, DimensionUnit::FP));
#else
    std::unique_ptr<Rosen::TypographyCreate> builder =
        Rosen::TypographyCreate::Create(style, fontCollection);
    Rosen::TextStyle txtStyle;
    txtStyle.fontSize = NormalizeToPx(Dimension(FONT_SIZE, DimensionUnit::FP));
#endif
    txtStyle.color = Color::FromARGB(255, 255, 255, 255).GetValue();
    txtStyle.locale = Localization::GetInstance()->GetFontLocale();
    builder->PushStyle(txtStyle);
#ifndef USE_GRAPHIC_TEXT_GINE
    builder->AddText(StringUtils::Str8ToStr16(text));
    paragraph_ = builder->Build();
#else
    builder->AppendText(StringUtils::Str8ToStr16(text));
    paragraph_ = builder->CreateTypography();
#endif
}

} // namespace OHOS::Ace
