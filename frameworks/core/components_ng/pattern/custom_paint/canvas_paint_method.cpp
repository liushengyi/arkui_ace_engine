/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/custom_paint/canvas_paint_method.h"

#include "drawing/engine_adapter/skia_adapter/skia_canvas.h"
#ifndef USE_GRAPHIC_TEXT_GINE
#include "txt/paragraph_builder.h"
#include "txt/paragraph_style.h"
#else
#include "rosen_text/typography_create.h"
#include "rosen_text/typography_style.h"
#endif
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/utils/SkBase64.h"

#include "base/i18n/localization.h"
#include "base/image/pixel_map.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/common/painter/rosen_decoration_painter.h"
#include "core/components/font/constants_converter.h"
#include "core/components/font/rosen_font_collection.h"
#include "core/components_ng/image_provider/image_object.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/components_ng/render/drawing.h"
#include "core/image/sk_image_cache.h"

namespace OHOS::Ace::NG {
namespace {
constexpr double HANGING_PERCENT = 0.8;
constexpr double DEFAULT_QUALITY = 0.92;
constexpr int32_t MAX_LENGTH = 2048 * 2048;
const std::string UNSUPPORTED = "data:image/png";
const std::string URL_PREFIX = "data:";
const std::string URL_SYMBOL = ";base64,";
const std::string IMAGE_PNG = "image/png";
const std::string IMAGE_JPEG = "image/jpeg";
const std::string IMAGE_WEBP = "image/webp";

std::string GetMimeType(const std::string& args)
{
    // Args example: ["image/png"]
    std::vector<std::string> values;
    StringUtils::StringSplitter(args, ',', values);
    if (values.size() > 2) {
        return IMAGE_PNG;
    }
    // Convert to lowercase string.
    for (size_t i = 0; i < values[0].size(); ++i) {
        values[0][i] = static_cast<uint8_t>(tolower(values[0][i]));
    }
    return values[0];
}

// Quality need between 0.0 and 1.0 for MimeType jpeg and webp
double GetQuality(const std::string& args)
{
    // Args example: ["image/jpeg", 0.8]
    std::vector<std::string> values;
    StringUtils::StringSplitter(args, ',', values);
    if (values.size() < 2) {
        return DEFAULT_QUALITY;
    }
    auto mimeType = GetMimeType(args);
    if (mimeType != IMAGE_JPEG && mimeType != IMAGE_WEBP) {
        return DEFAULT_QUALITY;
    }
    double quality = StringUtils::StringToDouble(values[1]);
    if (quality < 0.0 || quality > 1.0) {
        return DEFAULT_QUALITY;
    }
    return quality;
}
} // namespace

void CanvasPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    ACE_SCOPED_TRACE("CanvasPaintMethod::UpdateContentModifier");
    CHECK_NULL_VOID(paintWrapper);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto viewScale = context->GetViewScale();
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    if (lastLayoutSize_ != frameSize) {
        lastLayoutSize_.SetSizeT(frameSize);
    }
#ifndef USE_ROSEN_DRAWING
    if (!skCanvas_) {
        return;
    }
#else
    if (!rsCanvas_) {
        return;
    }
#endif

    if (tasks_.empty()) {
        return;
    }

#ifndef USE_ROSEN_DRAWING
    skCanvas_->scale(viewScale, viewScale);
#else
    rsCanvas_->Scale(viewScale, viewScale);
#endif
    for (const auto& task : tasks_) {
        task(*this, paintWrapper);
    }
    tasks_.clear();
    CHECK_NULL_VOID(contentModifier_);
    contentModifier_->MarkModifierDirty();
}

void CanvasPaintMethod::ImageObjReady(const RefPtr<Ace::ImageObject>& imageObj)
{
    imageObj_ = imageObj;
    if (!imageObj_->IsSvg()) {
        return;
    }
    skiaDom_ = AceType::DynamicCast<SvgSkiaImageObject>(imageObj_)->GetSkiaDom();
    currentSource_ = loadingSource_;
    Ace::CanvasImage canvasImage = canvasImage_;
    TaskFunc func = [canvasImage](CanvasPaintMethod& paintMethod, PaintWrapper* paintWrapper) {
        paintMethod.DrawImage(paintWrapper, canvasImage, 0, 0);
    };
    tasks_.emplace_back(func);
}

void CanvasPaintMethod::ImageObjFailed()
{
    imageObj_ = nullptr;
    skiaDom_ = nullptr;
}

void CanvasPaintMethod::DrawImage(
    PaintWrapper* paintWrapper, const Ace::CanvasImage& canvasImage, double width, double height)
{
    std::string::size_type tmp = canvasImage.src.find(".svg");
    if (tmp != std::string::npos) {
        DrawSvgImage(paintWrapper, canvasImage);
        return;
    }

    auto image = GetImage(canvasImage.src);
    CHECK_NULL_VOID(image);
#ifndef USE_ROSEN_DRAWING

    InitImagePaint(imagePaint_, sampleOptions_);
    if (globalState_.HasGlobalAlpha()) {
        imagePaint_.setAlphaf(globalState_.GetAlpha());
    }

    const auto skCanvas = skCanvas_.get();
    if (HasShadow()) {
        double shadowWidth = (canvasImage.flag == 0) ? static_cast<double>(image->width()) : canvasImage.dWidth;
        double shadowHeight = (canvasImage.flag == 0) ? static_cast<double>(image->height()) : canvasImage.dHeight;
        SkRect skRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, shadowWidth, shadowHeight);
        SkPath path;
        path.addRect(skRect);
        PaintShadow(path, shadow_, skCanvas, &imagePaint_);
    }

    switch (canvasImage.flag) {
        case 0: {
            if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
                skCanvas_->drawImage(image, canvasImage.dx, canvasImage.dy, sampleOptions_, &imagePaint_);
            } else {
                SkPaint compositeOperationpPaint;
                InitPaintBlend(compositeOperationpPaint);
                skCanvas_->saveLayer(SkRect::MakeXYWH(0, 0, lastLayoutSize_.Width(), lastLayoutSize_.Height()),
                    &compositeOperationpPaint);
                skCanvas_->drawImage(image, canvasImage.dx, canvasImage.dy, sampleOptions_, &imagePaint_);
                skCanvas_->restore();
            }
            break;
        }
        case 1: {
            SkRect rect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);

            if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
                skCanvas_->drawImageRect(image, rect, sampleOptions_, &imagePaint_);
            } else {
                SkPaint compositeOperationpPaint;
                InitPaintBlend(compositeOperationpPaint);
                skCanvas_->saveLayer(SkRect::MakeXYWH(0, 0, lastLayoutSize_.Width(), lastLayoutSize_.Height()),
                    &compositeOperationpPaint);
                skCanvas_->drawImageRect(image, rect, sampleOptions_, &imagePaint_);
                skCanvas_->restore();
            }
            break;
        }
        case 2: {
            SkRect dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            SkRect srcRect = SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);

            if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
                skCanvas_->drawImageRect(
                    image, srcRect, dstRect, sampleOptions_, &imagePaint_, SkCanvas::kStrict_SrcRectConstraint);
            } else {
                SkPaint compositeOperationpPaint;
                InitPaintBlend(compositeOperationpPaint);
                skCanvas_->saveLayer(SkRect::MakeXYWH(0, 0, lastLayoutSize_.Width(), lastLayoutSize_.Height()),
                    &compositeOperationpPaint);
                skCanvas_->drawImageRect(
                    image, srcRect, dstRect, sampleOptions_, &imagePaint_, SkCanvas::kStrict_SrcRectConstraint);
                skCanvas_->restore();
            }
            break;
        }
        default:
            break;
    }
#else
    InitImagePaint(nullptr, &imageBrush_, sampleOptions_);
    if (globalState_.HasGlobalAlpha()) {
        imageBrush_.setAlphaf(globalState_.GetAlpha());
    }

    const auto rsCanvas = rsCanvas_.get();
    if (HasImageShadow()) {
        RSRect rsRect = RSRect(
            canvasImage.dx, canvasImage.dy, canvasImage.dWidth + canvasImage.dx, canvasImage.dHeight + canvasImage.dy);
        RSPath path;
        path.AddRect(rsRect);
        PaintShadow(path, *imageShadow_, rsCanvas);
    }

    RSRect bounds = RSRect(0, 0, lastLayoutSize_.Width(), lastLayoutSize_.Height());
    rosen::SaveLayerOps layerOps(&bounds, &imageBrush_);
    switch (canvasImage.flag) {
        case 0: {
            if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
                rsCanvas_->AttachBrush(imageBrush_);
                rsCanvas_->DrawImage(*image, canvasImage.dx, canvasImage.dy, sampleOptions_);
                rsCanvas_->DetachBrush();
            } else {
                InitPaintBlend(imageBrush_);
                rsCanvas_->SaveLayer(layerOps);
                rsCanvas_->AttachBrush(imageBrush_);
                rsCanvas_->DrawImage(*image, canvasImage.dx, canvasImage.dy, sampleOptions_);
                rsCanvas_->DetachBrush();
                rsCanvas_->Restore();
            }
            break;
        }
        case 1: {
            RSRect rect = RSRect(canvasImage.dx, canvasImage.dy, canvasImage.dWidth + canvasImage.dx,
                canvasImage.dHeight + canvasImage.dy);

            if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
                rsCanvas_->AttachBrush(imageBrush_);
                rsCanvas_->DrawImageRect(*image, rect, sampleOptions_);
                rsCanvas_->DetachBrush();
            } else {
                InitPaintBlend(imageBrush_);
                rsCanvas_->SaveLayer(layerOps);
                rsCanvas_->AttachBrush(imageBrush_);
                rsCanvas_->DrawImageRect(*image, rect, sampleOptions_);
                rsCanvas_->DetachBrush();
                rsCanvas_->Restore();
            }
            break;
        }
        case 2: {
            RSRect dstRect = RSRect(canvasImage.dx, canvasImage.dy, canvasImage.dWidth + canvasImage.dx,
                canvasImage.dHeight + canvasImage.dy);
            RSRect srcRect = RSRect(canvasImage.sx, canvasImage.sy, canvasImage.sWidth + canvasImage.sx,
                canvasImage.sHeight + canvasImage.sy);

            if (globalState_.GetType() == CompositeOperation::SOURCE_OVER) {
                rsCanvas_->AttachBrush(imageBrush_);
                rsCanvas_->DrawImageRect(*image, srcRect, dstRect, sampleOptions_);
                rsCanvas_->DetachBrush();
            } else {
                InitPaintBlend(imageBrush_);
                rsCanvas_->SaveLayer(layerOps);
                rsCanvas_->AttachBrush(imageBrush_);
                rsCanvas_->DrawImageRect(*image, srcRect, dstRect, sampleOptions_);
                rsCanvas_->DetachBrush();
                rsCanvas_->Restore();
            }
            break;
        }
        default:
            break;
    }
#endif
}

void CanvasPaintMethod::DrawPixelMap(RefPtr<PixelMap> pixelMap, const Ace::CanvasImage& canvasImage)
{
#ifndef USE_ROSEN_DRAWING
    InitImagePaint(imagePaint_, sampleOptions_);

    SkPaint compositeOperationpPaint;
    InitPaintBlend(compositeOperationpPaint);
    if (globalState_.GetType() != CompositeOperation::SOURCE_OVER) {
        skCanvas_->saveLayer(
            SkRect::MakeXYWH(0, 0, lastLayoutSize_.Width(), lastLayoutSize_.Height()), &compositeOperationpPaint);
    }

    if (globalState_.HasGlobalAlpha()) {
        imagePaint_.setAlphaf(globalState_.GetAlpha());
    }

    const auto skCanvas = skCanvas_.get();
    CHECK_NULL_VOID(skCanvas);
    if (HasShadow()) {
        SkRect skRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
        SkPath path;
        path.addRect(skRect);
        PaintShadow(path, shadow_, skCanvas, &imagePaint_);
    }

    auto recordingCanvas = static_cast<OHOS::Rosen::RSRecordingCanvas*>(skCanvas);
    CHECK_NULL_VOID(recordingCanvas);
    const std::shared_ptr<Media::PixelMap> tempPixelMap = pixelMap->GetPixelMapSharedPtr();
    CHECK_NULL_VOID(tempPixelMap);
    switch (canvasImage.flag) {
        case 0:
            recordingCanvas->DrawPixelMap(tempPixelMap, canvasImage.dx, canvasImage.dy, sampleOptions_, &imagePaint_);
            break;
        case 1: {
            SkRect rect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, rect, sampleOptions_, &imagePaint_);
            break;
        }
        case 2: {
            SkRect dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            SkRect srcRect = SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
            recordingCanvas->DrawPixelMapRect(
                tempPixelMap, srcRect, dstRect, sampleOptions_, &imagePaint_, SkCanvas::kStrict_SrcRectConstraint);
            break;
        }
        default:
            break;
    }

    if (globalState_.GetType() != CompositeOperation::SOURCE_OVER) {
        skCanvas_->restore();
    }
#endif
}

void CanvasPaintMethod::DrawPixelMapWithoutGlobalState(
    const RefPtr<PixelMap>& pixelMap, const Ace::CanvasImage& canvasImage)
{
#ifndef USE_ROSEN_DRAWING
    InitImagePaint(imagePaint_, sampleOptions_);

    const auto skCanvas = skCanvas_.get();
    CHECK_NULL_VOID(skCanvas);
    auto recordingCanvas = static_cast<OHOS::Rosen::RSRecordingCanvas*>(skCanvas);
    CHECK_NULL_VOID(recordingCanvas);
    const std::shared_ptr<Media::PixelMap> tempPixelMap = pixelMap->GetPixelMapSharedPtr();
    CHECK_NULL_VOID(tempPixelMap);
    switch (canvasImage.flag) {
        case 0:
            recordingCanvas->DrawPixelMap(tempPixelMap, canvasImage.dx, canvasImage.dy, sampleOptions_, &imagePaint_);
            break;
        case 1: {
            SkRect rect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            recordingCanvas->DrawPixelMapRect(tempPixelMap, rect, sampleOptions_, &imagePaint_);
            break;
        }
        case 2: {
            SkRect dstRect = SkRect::MakeXYWH(canvasImage.dx, canvasImage.dy, canvasImage.dWidth, canvasImage.dHeight);
            SkRect srcRect = SkRect::MakeXYWH(canvasImage.sx, canvasImage.sy, canvasImage.sWidth, canvasImage.sHeight);
            recordingCanvas->DrawPixelMapRect(
                tempPixelMap, srcRect, dstRect, sampleOptions_, &imagePaint_, SkCanvas::kStrict_SrcRectConstraint);
            break;
        }
        default:
            break;
    }
#endif
}

void CanvasPaintMethod::CloseImageBitmap(const std::string& src)
{
    CHECK_NULL_VOID(imageCache_);
    auto cacheImage = imageCache_->GetCacheImage(src);
    if (cacheImage && cacheImage->imagePtr) {
        imageCache_->ClearCacheImage(src);
    }
}

std::unique_ptr<Ace::ImageData> CanvasPaintMethod::GetImageData(
    RefPtr<RosenRenderContext> renderContext, double left, double top, double width, double height)
{
    CHECK_NULL_RETURN(renderContext, nullptr);
    double viewScale = 1.0;
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, nullptr);
    viewScale = context->GetViewScale();
    double dirtyWidth = std::abs(width);
    double dirtyHeight = std::abs(height);
    double scaledLeft = left * viewScale;
    double scaledTop = top * viewScale;
    if (Negative(width)) {
        scaledLeft += width * viewScale;
    }
    if (Negative(height)) {
        scaledTop += height * viewScale;
    }
    // copy the bitmap to tempCanvas
#ifndef USE_ROSEN_DRAWING
    SkBitmap currentBitmap;
    if (!DrawBitmap(renderContext, currentBitmap)) {
        return nullptr;
    }

    SkBitmap tempCache;
    tempCache.allocPixels(SkImageInfo::Make(
        dirtyWidth, dirtyHeight, SkColorType::kBGRA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType));
    SkCanvas tempCanvas(tempCache);
    int32_t size = dirtyWidth * dirtyHeight;
    const uint8_t* pixels = nullptr;
    auto srcRect = SkRect::MakeXYWH(scaledLeft, scaledTop, dirtyWidth * viewScale, dirtyHeight * viewScale);
    auto dstRect = SkRect::MakeXYWH(0.0, 0.0, dirtyWidth, dirtyHeight);

    tempCanvas.drawImageRect(
        currentBitmap.asImage(), srcRect, dstRect, SkSamplingOptions(), nullptr, SkCanvas::kStrict_SrcRectConstraint);
    pixels = tempCache.pixmap().addr8();
#else
    RSBitmap currentBitmap;
    if (!DrawBitmap(renderContext, currentBitmap)) {
        return nullptr;
    }

    RSBitmapFormat format { RSColorType::COLORTYPE_BGRA_8888, RSAlphaType::ALPHATYPE_OPAQUE };
    RSBitmap tempCache;
    tempCache.Build(dirtyWidth, dirtyHeight, format);
    int32_t size = dirtyWidth * dirtyHeight;

    RSCanvas tempCanvas;
    tempCanvas.Bind(tempCache);
    auto srcRect =
        RSRect(scaledLeft, scaledTop, dirtyWidth * viewScale + scaledLeft, dirtyHeight * viewScale + scaledTop);
    auto dstRect = RSRect(0.0, 0.0, dirtyWidth, dirtyHeight);
    RSImage rsImage;
    rsImage.BuildFromBitmap(currentBitmap);
    tempCanvas.DrawImageRect(rsImage, srcRect, dstRect, RSSamplingOptions());
    const uint8_t* pixels = static_cast<const uint8_t*>(tempCache.GetPixels());
#endif
    CHECK_NULL_RETURN(pixels, nullptr);
    std::unique_ptr<Ace::ImageData> imageData = std::make_unique<Ace::ImageData>();
    imageData->dirtyWidth = dirtyWidth;
    imageData->dirtyHeight = dirtyHeight;
    // a pixel include 4 data(blue, green, red, alpha)
    for (int i = 0; i < size * 4; i += 4) {
        auto blue = pixels[i];
        auto green = pixels[i + 1];
        auto red = pixels[i + 2];
        auto alpha = pixels[i + 3];
        imageData->data.emplace_back(Color::FromARGB(alpha, red, green, blue));
    }
    return imageData;
}

void CanvasPaintMethod::GetImageData(
    const RefPtr<RenderContext>& renderContext, const std::shared_ptr<Ace::ImageData>& imageData)
{
    auto rosenRenderContext = AceType::DynamicCast<RosenRenderContext>(renderContext);
    CHECK_NULL_VOID(rosenRenderContext);
    CHECK_NULL_VOID(imageData);
    auto viewScale = 1.0f;
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    viewScale = context->GetViewScale();
    auto dirtyWidth = std::abs(imageData->dirtyWidth);
    auto dirtyHeight = std::abs(imageData->dirtyHeight);
    auto scaledLeft = imageData->dirtyX * viewScale;
    auto scaledTop = imageData->dirtyY * viewScale;
    auto dx = 0.0f;
    auto dy = 0.0f;
    if (Negative(imageData->dirtyWidth)) {
        scaledLeft += imageData->dirtyWidth * viewScale;
    }
    if (Negative(imageData->dirtyHeight)) {
        scaledTop += imageData->dirtyHeight * viewScale;
    }
    if (Negative(scaledLeft)) {
        dx = scaledLeft;
    }
    if (Negative(scaledTop)) {
        dy = scaledTop;
    }

#ifndef USE_ROSEN_DRAWING
    CHECK_NULL_VOID(rsRecordingCanvas_);
    auto drawCmdList = rsRecordingCanvas_->GetDrawCmdList();
    SkRect rect = SkRect::MakeXYWH(scaledLeft, scaledTop, dirtyWidth * viewScale, dirtyHeight * viewScale);
    auto pixelMap = imageData->pixelMap;
    CHECK_NULL_VOID(pixelMap);
    auto sharedPixelMap = pixelMap->GetPixelMapSharedPtr();
    auto ret = rosenRenderContext->GetPixelMap(sharedPixelMap, drawCmdList, &rect);
    if (!ret) {
        if (!drawCmdList || drawCmdList->GetSize() == 0) {
            return;
        }
        SkBitmap bitmap;
        SkImageInfo info = SkImageInfo::Make(rect.width(), rect.height(), kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        bitmap.installPixels(info, pixelMap->GetWritablePixels(), pixelMap->GetRowBytes());
        SkCanvas canvas(bitmap);
        canvas.translate(-rect.x(), -rect.y());
        drawCmdList->Playback(canvas, &rect);
    }
#else
    CHECK_NULL_VOID(rsRecordingCanvas_);
    auto drawCmdList = rsRecordingCanvas_->GetDrawCmdList();
    auto rect = RSRect(scaledLeft, scaledTop,
        dirtyWidth * viewScale + scaledLeft, dirtyHeight * viewScale + scaledTop);
    auto pixelMap = imageData->pixelMap;
    CHECK_NULL_VOID(pixelMap);
    auto sharedPixelMap = pixelMap->GetPixelMapSharedPtr();
    auto ret = rosenRenderContext->GetPixelMap(sharedPixelMap, drawCmdList, &rect);
    if (!ret) {
        if (!drawCmdList || drawCmdList->IsEmpty()) {
            return;
        }
        RSBitmap bitmap;
        RSImageInfo info = RSImageInfo(rect.GetWidth(), rect.GetHeight(),
            RSColorType::COLORTYPE_BRGBA_8888, RSAlphaType::ALPHATYPE_PREMUL);
        bitmap.InstallPixels(info, pixelMap->GetWritablePixels(), pixelMap->GetRowBytes());
        RSCanvas canvas;
        canvas.Bind(bitmap);
        canvas.Translate(-rect.GetLeft(), -rect.GetTop());
        drawCmdList->Playback(canvas, &rect);
    }
#endif
}

void CanvasPaintMethod::TransferFromImageBitmap(
    PaintWrapper* paintWrapper, const RefPtr<OffscreenCanvasPattern>& offscreenCanvas)
{
    CHECK_NULL_VOID(offscreenCanvas);
#ifdef PIXEL_MAP_SUPPORTED
    OHOS::Media::InitializationOptions options;
    options.alphaType = OHOS::Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    options.pixelFormat = OHOS::Media::PixelFormat::RGBA_8888;
    options.scaleMode = OHOS::Media::ScaleMode::CENTER_CROP;
    auto width = offscreenCanvas->GetWidth();
    auto height = offscreenCanvas->GetHeight();
    options.size.width = width;
    options.size.height = height;
    options.editable = true;
    auto pixelMap = Ace::PixelMap::Create(OHOS::Media::PixelMap::Create(options));
    if (pixelMap) {
        std::shared_ptr<Ace::ImageData> imageData = std::make_shared<Ace::ImageData>();
        imageData->pixelMap = pixelMap;
        imageData->dirtyX = 0.0f;
        imageData->dirtyY = 0.0f;
        imageData->dirtyWidth = width;
        imageData->dirtyHeight = height;
        offscreenCanvas->GetImageData(imageData);
        Ace::CanvasImage canvasImage;
        canvasImage.flag = 0;
        DrawPixelMapWithoutGlobalState(pixelMap, canvasImage);
    }
#else
    std::unique_ptr<Ace::ImageData> imageData =
        offscreenCanvas->GetImageData(0, 0, offscreenCanvas->GetWidth(), offscreenCanvas->GetHeight());
    CHECK_NULL_VOID(imageData);
    PutImageData(paintWrapper, *imageData);
#endif
}

void CanvasPaintMethod::FillText(
    PaintWrapper* paintWrapper, const std::string& text, double x, double y, std::optional<double> maxWidth)
{
    CHECK_NULL_VOID(paintWrapper);
    OffsetF offset = GetContentOffset(paintWrapper);
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();

    auto success = UpdateParagraph(offset, text, false, HasShadow());
    CHECK_NULL_VOID(success);
    PaintText(offset, frameSize, x, y, maxWidth, false, HasShadow());
}

void CanvasPaintMethod::StrokeText(
    PaintWrapper* paintWrapper, const std::string& text, double x, double y, std::optional<double> maxWidth)
{
    CHECK_NULL_VOID(paintWrapper);
    OffsetF offset = GetContentOffset(paintWrapper);
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();

    if (HasShadow()) {
        auto success = UpdateParagraph(offset, text, true, true);
        CHECK_NULL_VOID(success);
        PaintText(offset, frameSize, x, y, maxWidth, true, true);
    }

    auto success = UpdateParagraph(offset, text, true);
    CHECK_NULL_VOID(success);
    PaintText(offset, frameSize, x, y, maxWidth, true);
}

double CanvasPaintMethod::MeasureText(const std::string& text, const PaintState& state)
{
    using namespace Constants;
#ifndef USE_GRAPHIC_TEXT_GINE
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
#else
    Rosen::TypographyStyle style;
    style.textAlign = ConvertTxtTextAlign(state.GetTextAlign());
#endif

    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
#else
    std::unique_ptr<Rosen::TypographyCreate> builder = Rosen::TypographyCreate::Create(style, fontCollection);
    Rosen::TextStyle txtStyle;
#endif
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
#ifndef USE_GRAPHIC_TEXT_GINE
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
#else
    txtStyle.fontSize = state.GetTextStyle().GetFontSize().Value();
#endif
    builder->PushStyle(txtStyle);
#ifndef USE_GRAPHIC_TEXT_GINE
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
#else
    builder->AppendText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->CreateTypography();
#endif
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetMaxIntrinsicWidth();
}

double CanvasPaintMethod::MeasureTextHeight(const std::string& text, const PaintState& state)
{
    using namespace Constants;
#ifndef USE_GRAPHIC_TEXT_GINE
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
#else
    Rosen::TypographyStyle style;
    style.textAlign = ConvertTxtTextAlign(state.GetTextAlign());
#endif

    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, 0.0);
#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
#else
    std::unique_ptr<Rosen::TypographyCreate> builder = Rosen::TypographyCreate::Create(style, fontCollection);
    Rosen::TextStyle txtStyle;
#endif
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
#ifndef USE_GRAPHIC_TEXT_GINE
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
#else
    txtStyle.fontSize = state.GetTextStyle().GetFontSize().Value();
#endif
    builder->PushStyle(txtStyle);
#ifndef USE_GRAPHIC_TEXT_GINE
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
#else
    builder->AppendText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->CreateTypography();
#endif
    paragraph->Layout(Size::INFINITE_SIZE);
    return paragraph->GetHeight();
}

TextMetrics CanvasPaintMethod::MeasureTextMetrics(const std::string& text, const PaintState& state)
{
    using namespace Constants;
    TextMetrics textMetrics = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
#ifndef USE_GRAPHIC_TEXT_GINE
    txt::ParagraphStyle style;
    style.text_align = ConvertTxtTextAlign(state.GetTextAlign());
#else
    Rosen::TypographyStyle style;
    style.textAlign = ConvertTxtTextAlign(state.GetTextAlign());
#endif

    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, textMetrics);
#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.font_size = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->Build();
#else
    std::unique_ptr<Rosen::TypographyCreate> builder = Rosen::TypographyCreate::Create(style, fontCollection);
    Rosen::TextStyle txtStyle;
    ConvertTxtStyle(state.GetTextStyle(), context_, txtStyle);
    txtStyle.fontSize = state.GetTextStyle().GetFontSize().Value();
    builder->PushStyle(txtStyle);
    builder->AppendText(StringUtils::Str8ToStr16(text));
    auto paragraph = builder->CreateTypography();
#endif
    paragraph->Layout(Size::INFINITE_SIZE);

    auto textAlign = state.GetTextAlign();
    auto textBaseLine = state.GetTextStyle().GetTextBaseline();

    textMetrics.width = paragraph->GetMaxIntrinsicWidth();
    textMetrics.height = paragraph->GetHeight();
    textMetrics.actualBoundingBoxLeft = -GetAlignOffset(textAlign, paragraph);
    textMetrics.actualBoundingBoxRight = textMetrics.width - textMetrics.actualBoundingBoxLeft;
    textMetrics.actualBoundingBoxAscent = -GetBaselineOffset(textBaseLine, paragraph);
    textMetrics.actualBoundingBoxDescent = textMetrics.height - textMetrics.actualBoundingBoxAscent;
    return textMetrics;
}

void CanvasPaintMethod::PaintText(const OffsetF& offset, const SizeF& frameSize, double x, double y,
    std::optional<double> maxWidth, bool isStroke, bool hasShadow)
{
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        paragraph_->Layout(FLT_MAX);
    } else {
        paragraph_->Layout(frameSize.Width());
    }
    auto width = paragraph_->GetMaxIntrinsicWidth();
    if (frameSize.Width() > width) {
        paragraph_->Layout(std::ceil(width));
    }
    auto align = isStroke ? strokeState_.GetTextAlign() : fillState_.GetTextAlign();
#ifndef USE_ROSEN_DRAWING
    double dx = offset.GetX() + x + GetAlignOffset(align, paragraph_);
    auto baseline =
        isStroke ? strokeState_.GetTextStyle().GetTextBaseline() : fillState_.GetTextStyle().GetTextBaseline();
    double dy = offset.GetY() + y + GetBaselineOffset(baseline, paragraph_);

    std::optional<double> scale = CalcTextScale(paragraph_->GetMaxIntrinsicWidth(), maxWidth);
    if (hasShadow) {
        skCanvas_->save();
        auto shadowOffsetX = shadow_.GetOffset().GetX();
        auto shadowOffsetY = shadow_.GetOffset().GetY();
        if (scale.has_value()) {
            if (!NearZero(scale.value())) {
                dx /= scale.value();
                shadowOffsetX /= scale.value();
            }
            skCanvas_->scale(scale.value(), 1.0);
        }
        paragraph_->Paint(skCanvas_.get(), dx + shadowOffsetX, dy + shadowOffsetY);
        skCanvas_->restore();
        return;
    }
#else
    double dx = offset.GetX() + x + GetAlignOffset(align, paragraph_);
    auto baseline =
        isStroke ? strokeState_.GetTextStyle().GetTextBaseline() : fillState_.GetTextStyle().GetTextBaseline();
    double dy = offset.GetY() + y + GetBaselineOffset(baseline, paragraph_);

    std::optional<double> scale = CalcTextScale(paragraph_->GetMaxIntrinsicWidth(), maxWidth);
    if (hasShadow) {
        rsCanvas_->Save();
        auto shadowOffsetX = shadow_.GetOffset().GetX();
        auto shadowOffsetY = shadow_.GetOffset().GetY();
        if (scale.has_value()) {
            if (!NearZero(scale.value())) {
                dx /= scale.value();
                shadowOffsetX /= scale.value();
            }
            rsCanvas_->Scale(scale.value(), 1.0);
        }
        paragraph_->Paint(rsCanvas_.get(), dx + shadowOffsetX, dy + shadowOffsetY);
        rsCanvas_->Restore();
        return;
    }
#endif
    if (scale.has_value()) {
        if (!NearZero(scale.value())) {
            dx /= scale.value();
        }
#ifndef USE_ROSEN_DRAWING
        skCanvas_->save();
        skCanvas_->scale(scale.value(), 1.0);
        paragraph_->Paint(skCanvas_.get(), dx, dy);
        skCanvas_->restore();
    } else {
        paragraph_->Paint(skCanvas_.get(), dx, dy);
    }
#else
        rsCanvas_->Save();
        rsCanvas_->Scale(scale.value(), 1.0);
        paragraph_->Paint(rsCanvas_.get(), dx, dy);
        rsCanvas_->Restore();
    } else {
        paragraph_->Paint(rsCanvas_.get(), dx, dy);
    }
#endif
}

#ifndef USE_GRAPHIC_TEXT_GINE
double CanvasPaintMethod::GetBaselineOffset(TextBaseline baseline, std::unique_ptr<txt::Paragraph>& paragraph)
#else
double CanvasPaintMethod::GetBaselineOffset(TextBaseline baseline, std::unique_ptr<Rosen::Typography>& paragraph)
#endif
{
    double y = 0.0;
    switch (baseline) {
        case TextBaseline::ALPHABETIC:
            y = -paragraph->GetAlphabeticBaseline();
            break;
        case TextBaseline::IDEOGRAPHIC:
            y = -paragraph->GetIdeographicBaseline();
            break;
        case TextBaseline::BOTTOM:
            y = -paragraph->GetHeight();
            break;
        case TextBaseline::TOP:
            y = 0.0;
            break;
        case TextBaseline::MIDDLE:
            y = -paragraph->GetHeight() / 2;
            break;
        case TextBaseline::HANGING:
            y = -HANGING_PERCENT * (paragraph->GetHeight() - paragraph->GetAlphabeticBaseline());
            break;
        default:
            y = -paragraph->GetAlphabeticBaseline();
            break;
    }
    return y;
}

bool CanvasPaintMethod::UpdateParagraph(const OffsetF& offset, const std::string& text, bool isStroke, bool hasShadow)
{
    using namespace Constants;
#ifndef USE_GRAPHIC_TEXT_GINE
    txt::ParagraphStyle style;
#else
    Rosen::TypographyStyle style;
#endif
    if (isStroke) {
#ifndef USE_GRAPHIC_TEXT_GINE
        style.text_align = ConvertTxtTextAlign(strokeState_.GetTextAlign());
#else
        style.textAlign = ConvertTxtTextAlign(strokeState_.GetTextAlign());
#endif
    } else {
#ifndef USE_GRAPHIC_TEXT_GINE
        style.text_align = ConvertTxtTextAlign(fillState_.GetTextAlign());
#else
        style.textAlign = ConvertTxtTextAlign(fillState_.GetTextAlign());
#endif
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    style.text_direction = ConvertTxtTextDirection(fillState_.GetOffTextDirection());
    style.text_align = GetEffectiveAlign(style.text_align, style.text_direction);
#else
    style.textDirection = ConvertTxtTextDirection(fillState_.GetOffTextDirection());
    style.textAlign = GetEffectiveAlign(style.textAlign, style.textDirection);
#endif

    auto fontCollection = RosenFontCollection::GetInstance().GetFontCollection();
    CHECK_NULL_RETURN(fontCollection, false);
#ifndef USE_GRAPHIC_TEXT_GINE
    std::unique_ptr<txt::ParagraphBuilder> builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
    txt::TextStyle txtStyle;
#else
    std::unique_ptr<Rosen::TypographyCreate> builder = Rosen::TypographyCreate::Create(style, fontCollection);
    Rosen::TextStyle txtStyle;
#endif
    if (!isStroke && hasShadow) {
#ifndef USE_GRAPHIC_TEXT_GINE
        txt::TextShadow txtShadow;
        txtShadow.color = shadow_.GetColor().GetValue();
#ifndef USE_ROSEN_DRAWING
        txtShadow.offset.fX = shadow_.GetOffset().GetX();
        txtShadow.offset.fY = shadow_.GetOffset().GetY();
#else
        txtShadow.offset.SetX(shadow_.GetOffset().GetX());
        txtShadow.offset.SetY(shadow_.GetOffset().GetY());
#endif
        txtShadow.blur_sigma = shadow_.GetBlurRadius();
        txtStyle.text_shadows.emplace_back(txtShadow);
#else
        Rosen::TextShadow txtShadow;
        txtShadow.color = shadow_.GetColor().GetValue();
        txtShadow.offset.SetX(shadow_.GetOffset().GetX());
        txtShadow.offset.SetY(shadow_.GetOffset().GetY());
        txtShadow.blurRadius = shadow_.GetBlurRadius();
        txtStyle.shadows.emplace_back(txtShadow);
#endif
    }
    txtStyle.locale = Localization::GetInstance()->GetFontLocale();
    UpdateTextStyleForeground(offset, isStroke, txtStyle, hasShadow);
    builder->PushStyle(txtStyle);
#ifndef USE_GRAPHIC_TEXT_GINE
    builder->AddText(StringUtils::Str8ToStr16(text));
    paragraph_ = builder->Build();
#else
    builder->AppendText(StringUtils::Str8ToStr16(text));
    paragraph_ = builder->CreateTypography();
#endif
    return true;
}

#ifndef USE_ROSEN_DRAWING
#ifndef USE_GRAPHIC_TEXT_GINE
void CanvasPaintMethod::UpdateTextStyleForeground(
    const OffsetF& offset, bool isStroke, txt::TextStyle& txtStyle, bool hasShadow)
#else
void CanvasPaintMethod::UpdateTextStyleForeground(
    const OffsetF& offset, bool isStroke, Rosen::TextStyle& txtStyle, bool hasShadow)
#endif
{
    using namespace Constants;
    if (!isStroke) {
        txtStyle.color = ConvertSkColor(fillState_.GetColor());
#ifndef USE_GRAPHIC_TEXT_GINE
        txtStyle.font_size = fillState_.GetTextStyle().GetFontSize().Value();
#else
        txtStyle.fontSize = fillState_.GetTextStyle().GetFontSize().Value();
#endif
        ConvertTxtStyle(fillState_.GetTextStyle(), context_, txtStyle);
        if (fillState_.GetGradient().IsValid() && fillState_.GetPaintStyle() == PaintStyle::Gradient) {
            SkPaint paint;

            SkSamplingOptions options;
            InitImagePaint(paint, options);
            paint.setStyle(SkPaint::Style::kFill_Style);
            UpdatePaintShader(offset, paint, fillState_.GetGradient());
            txtStyle.foreground = paint;
#ifndef USE_GRAPHIC_TEXT_GINE
            txtStyle.has_foreground = true;
#endif
        }
        if (globalState_.HasGlobalAlpha()) {
#ifndef USE_GRAPHIC_TEXT_GINE
            if (txtStyle.has_foreground) {
                txtStyle.foreground.setColor(fillState_.GetColor().GetValue());
                txtStyle.foreground.setAlphaf(globalState_.GetAlpha()); // set alpha after color
#else
            if (txtStyle.foreground.has_value()) {
                txtStyle.foreground->setColor(fillState_.GetColor().GetValue());
                txtStyle.foreground->setAlphaf(globalState_.GetAlpha()); // set alpha after color
#endif
            } else {
                SkPaint paint;

                SkSamplingOptions options;
                InitImagePaint(paint, options);
                paint.setColor(fillState_.GetColor().GetValue());
                paint.setAlphaf(globalState_.GetAlpha()); // set alpha after color
                InitPaintBlend(paint);
                txtStyle.foreground = paint;
#ifndef USE_GRAPHIC_TEXT_GINE
                txtStyle.has_foreground = true;
#endif
            }
        }
    } else {
        // use foreground to draw stroke
        SkPaint paint;

        SkSamplingOptions options;
        GetStrokePaint(paint, options);
        InitPaintBlend(paint);
        ConvertTxtStyle(strokeState_.GetTextStyle(), context_, txtStyle);
#ifndef USE_GRAPHIC_TEXT_GINE
        txtStyle.font_size = strokeState_.GetTextStyle().GetFontSize().Value();
        if (strokeState_.GetGradient().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::Gradient) {
#else
        txtStyle.fontSize = strokeState_.GetTextStyle().GetFontSize().Value();
        if (strokeState_.GetGradient().IsValid()) {
#endif
            UpdatePaintShader(offset, paint, strokeState_.GetGradient());
        }
        if (hasShadow) {
            paint.setColor(shadow_.GetColor().GetValue());
            paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle,
                RosenDecorationPainter::ConvertRadiusToSigma(shadow_.GetBlurRadius())));
        }
        txtStyle.foreground = paint;
#ifndef USE_GRAPHIC_TEXT_GINE
        txtStyle.has_foreground = true;
#endif
    }
}
#else
void CanvasPaintMethod::UpdateTextStyleForeground(
    const OffsetF& offset, bool isStroke, txt::TextStyle& txtStyle, bool hasShadow)
{
    using namespace Constants;
    if (!isStroke) {
        txtStyle.foreground_pen.Reset();
        txtStyle.has_foreground_pen = false;
        txtStyle.color = ConvertSkColor(fillState_.GetColor());
        txtStyle.font_size = fillState_.GetTextStyle().GetFontSize().Value();
        ConvertTxtStyle(fillState_.GetTextStyle(), context_, txtStyle);
        if (fillState_.GetGradient().IsValid() && fillState_.GetPaintStyle() == PaintStyle::Gradient) {
            RSBrush brush;
            RSSamplingOptions options;
            InitImagePaint(nullptr, &brush, options);
            UpdatePaintShader(offset, nullptr, &brush, fillState_.GetGradient());
            txtStyle.foreground_brush = brush;
            txtStyle.has_foreground_brush = true;
        }
        if (globalState_.HasGlobalAlpha()) {
            if (txtStyle.has_foreground_brush) {
                txtStyle.foreground_brush.SetColor(fillState_.GetColor().GetValue());
                txtStyle.foreground_brush.SetAlphaF(globalState_.GetAlpha()); // set alpha after color
            } else {
                RSBrush brush;
                RSSamplingOptions options;
                InitImagePaint(nullptr, &brush, options);
                brush.SetColor(fillState_.GetColor().GetValue());
                brush.SetAlphaF(globalState_.GetAlpha()); // set alpha after color
                InitPaintBlend(brush);
                txtStyle.foreground_brush = brush;
                txtStyle.has_foreground_brush = true;
            }
        }
    } else {
        // use foreground to draw stroke
        txtStyle.foreground_pen.Reset();
        txtStyle.has_foreground_pen = false;
        RSPen pen;
        RSSamplingOptions options;
        GetStrokePaint(pen, options);
        InitPaintBlend(pen);
        ConvertTxtStyle(strokeState_.GetTextStyle(), context_, txtStyle);
        txtStyle.font_size = strokeState_.GetTextStyle().GetFontSize().Value();
        if (strokeState_.GetGradient().IsValid() && strokeState_.GetPaintStyle() == PaintStyle::Gradient) {
            UpdatePaintShader(offset, &pen, nullptr, strokeState_.GetGradient());
        }
        if (hasShadow) {
            pen.SetColor(shadow_.GetColor().GetValue());
            RSFilter filter;
            filter.SetMaskFilter(RSRecordingMaskFilter::CreateBlurMaskFilter(RSBlurType::NORMAL,
                RosenDecorationPainter::ConvertRadiusToSigma(shadow_.GetBlurRadius())));
            pen.SetFilter(filter);
        }
        txtStyle.foreground_pen = pen;
        txtStyle.has_foreground_pen = true;
    }
}
#endif

#ifndef USE_ROSEN_DRAWING
void CanvasPaintMethod::PaintShadow(const SkPath& path, const Shadow& shadow, SkCanvas* canvas, const SkPaint* paint)
{
#ifdef ENABLE_ROSEN_BACKEND
    RosenDecorationPainter::PaintShadow(path, shadow, canvas, paint);
#endif
}
#else
void CanvasPaintMethod::PaintShadow(const RSPath& path, const Shadow& shadow, RSCanvas* canvas)
{
#ifdef ENABLE_ROSEN_BACKEND
    RosenDecorationPainter::PaintShadow(path, shadow, canvas);
#endif
}
#endif

void CanvasPaintMethod::Path2DRect(const OffsetF& offset, const PathArgs& args)
{
    double left = args.para1 + offset.GetX();
    double top = args.para2 + offset.GetY();
    double right = args.para3 + args.para1 + offset.GetX();
    double bottom = args.para4 + args.para2 + offset.GetY();
#ifndef USE_ROSEN_DRAWING
    skPath2d_.addRect(SkRect::MakeLTRB(left, top, right, bottom));
#else
    rsPath2d_.AddRect(RSRect(left, top, right, bottom));
#endif
}

void CanvasPaintMethod::SetTransform(const TransformParam& param)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    double viewScale = context->GetViewScale();
#ifndef USE_ROSEN_DRAWING
    SkMatrix skMatrix;
    skMatrix.setAll(param.scaleX * viewScale, param.skewX * viewScale, param.translateX * viewScale,
        param.skewY * viewScale, param.scaleY * viewScale, param.translateY * viewScale, 0, 0, 1);
    skCanvas_->setMatrix(skMatrix);
#else
    RSMatrix rsMatrix;
    rsMatrix.SetMatrix(param.scaleX * viewScale, param.skewX * viewScale, param.translateX * viewScale,
        param.skewY * viewScale, param.scaleY * viewScale, param.translateY * viewScale, 0, 0, 1);
    rsCanvas_->SetMatrix(rsMatrix);
#endif
}

std::string CanvasPaintMethod::ToDataURL(RefPtr<RosenRenderContext> renderContext, const std::string& args)
{
    CHECK_NULL_RETURN(renderContext, UNSUPPORTED);
    std::string mimeType = GetMimeType(args);
    double quality = GetQuality(args);
    double width = lastLayoutSize_.Width();
    double height = lastLayoutSize_.Height();
    auto imageInfo = SkImageInfo::Make(width, height, SkColorType::kBGRA_8888_SkColorType,
        (mimeType == IMAGE_JPEG) ? SkAlphaType::kOpaque_SkAlphaType : SkAlphaType::kUnpremul_SkAlphaType);
#ifndef USE_ROSEN_DRAWING
    SkBitmap tempCache;
    tempCache.allocPixels(imageInfo);
    SkBitmap currentBitmap;
    if (!DrawBitmap(renderContext, currentBitmap)) {
        return UNSUPPORTED;
    }
    bool success = currentBitmap.pixmap().scalePixels(
        tempCache.pixmap(), SkSamplingOptions(SkCubicResampler { 1 / 3.0f, 1 / 3.0f }));
    CHECK_NULL_RETURN(success, UNSUPPORTED);
    SkPixmap src = tempCache.pixmap();
#else
    RSBitmapFormat format { RSColorType::COLORTYPE_BGRA_8888,
        (mimeType == IMAGE_JPEG) ? RSAlphaType::ALPHATYPE_OPAQUE : RSAlphaType::ALPHATYPE_UNPREMUL };
    RSBitmap tempCache;
    tempCache.Build(width, height, format);
    RSBitmap currentBitmap;
    if (!DrawBitmap(renderContext, currentBitmap)) {
        return UNSUPPORTED;
    }
    bool success = currentBitmap.GetPixelMap().ScalePixels(
        tempCache.GetPixelMap(), RSSamplingOptions(RSCubicResampler { 1 / 3.0f, 1 / 3.0f }));
    CHECK_NULL_RETURN(success, UNSUPPORTED);
    RSPixmap rsSrc = tempCache.GetPixelMap();
    SkPixmap src { imageInfo, rsSrc.GetAddr(), rsSrc.GetRowBytes() };
#endif
    SkDynamicMemoryWStream dst;
    if (mimeType == IMAGE_JPEG) {
        SkJpegEncoder::Options options;
        options.fQuality = quality * 100;
        success = SkJpegEncoder::Encode(&dst, src, options);
    } else if (mimeType == IMAGE_WEBP) {
        SkWebpEncoder::Options options;
        options.fQuality = quality * 100.0;
        success = SkWebpEncoder::Encode(&dst, src, options);
    } else {
        mimeType = IMAGE_PNG;
        SkPngEncoder::Options options;
        success = SkPngEncoder::Encode(&dst, src, options);
    }
    CHECK_NULL_RETURN(success, UNSUPPORTED);
    auto result = dst.detachAsData();
    CHECK_NULL_RETURN(result, UNSUPPORTED);
    size_t len = SkBase64::Encode(result->data(), result->size(), nullptr);
    if (len > MAX_LENGTH) {
        return UNSUPPORTED;
    }
    SkString info(len);
    SkBase64::Encode(result->data(), result->size(), info.writable_str());
    return std::string(URL_PREFIX).append(mimeType).append(URL_SYMBOL).append(info.c_str());
}

#ifndef USE_ROSEN_DRAWING
bool CanvasPaintMethod::DrawBitmap(RefPtr<RosenRenderContext> renderContext, SkBitmap& currentBitmap)
{
    CHECK_NULL_RETURN(rsRecordingCanvas_, false);
    auto drawCmdList = rsRecordingCanvas_->GetDrawCmdList();
    bool res = renderContext->GetBitmap(currentBitmap, drawCmdList);
    if (res) {
        return true;
    }
    if (!drawCmdList) {
        return false;
    }
    if (drawCmdList->GetSize() == 0) {
        return false;
    }
    currentBitmap.reset();
    auto imageInfo = SkImageInfo::Make(lastLayoutSize_.Width(), lastLayoutSize_.Height(),
        SkColorType::kBGRA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType);
    // tryAllocPixels is more safe than allocPixels
    currentBitmap.allocPixels(imageInfo);
    SkCanvas currentCanvas(currentBitmap);
    drawCmdList->Playback(currentCanvas);
    return true;
}
#else
bool CanvasPaintMethod::DrawBitmap(RefPtr<RosenRenderContext> renderContext, RSBitmap& currentBitmap)
{
    CHECK_NULL_RETURN(rsRecordingCanvas_, false);
    auto drawCmdList = rsRecordingCanvas_->GetDrawCmdList();
    bool res = renderContext->GetBitmap(currentBitmap, drawCmdList);
    if (res) {
        return true;
    }
    if (!drawCmdList) {
        return false;
    }
    if (drawCmdList->IsEmpty()) {
        return false;
    }
    currentBitmap.Free();
    RSBitmapFormat format { RSColorType::COLORTYPE_BGRA_8888, RSAlphaType::ALPHATYPE_OPAQUE };
    currentBitmap.Build(lastLayoutSize_.Width(), lastLayoutSize_.Height(), format);

    RSCanvas currentCanvas;
    currentCanvas.Bind(currentBitmap);
    drawCmdList->Playback(currentCanvas);
    return true;
}
#endif

std::string CanvasPaintMethod::GetJsonData(const std::string& path)
{
    AssetImageLoader imageLoader;
    return imageLoader.LoadJsonData(path, context_);
}
} // namespace OHOS::Ace::NG
