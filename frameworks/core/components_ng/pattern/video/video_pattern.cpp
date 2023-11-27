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

#include "core/components_ng/pattern/video/video_pattern.h"

#include "video_node.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/size_t.h"
#include "base/i18n/localization.h"
#include "base/json/json_util.h"
#include "base/thread/task_executor.h"
#include "base/utils/string_utils.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_view.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/declaration/button/button_declaration.h"
#include "core/components/video/video_theme.h"
#include "core/components/video/video_utils.h"
#include "core/components_ng/pattern/button/button_event_hub.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/slider/slider_event_hub.h"
#include "core/components_ng/pattern/slider/slider_layout_property.h"
#include "core/components_ng/pattern/slider/slider_paint_property.h"
#include "core/components_ng/pattern/slider/slider_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/video/video_event_hub.h"
#include "core/components_ng/pattern/video/video_full_screen_node.h"
#include "core/components_ng/pattern/video/video_full_screen_pattern.h"
#include "core/components_ng/pattern/video/video_layout_property.h"
#include "core/components_ng/pattern/video/video_node.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

#ifdef ENABLE_DRAG_FRAMEWORK
#include "core/common/udmf/udmf_client.h"
#endif
namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t SECONDS_PER_HOUR = 3600;
constexpr uint32_t SECONDS_PER_MINUTE = 60;
const std::string FORMAT_HH_MM_SS = "%02d:%02d:%02d";
const std::string FORMAT_MM_SS = "%02d:%02d";
constexpr int32_t MILLISECONDS_TO_SECONDS = 1000;
constexpr uint32_t CURRENT_POS = 1;
constexpr uint32_t SLIDER_POS = 2;
constexpr uint32_t DURATION_POS = 3;
constexpr uint32_t FULL_SCREEN_POS = 4;
constexpr int32_t AVERAGE_VALUE = 2;

// Default error, empty string.
const std::string ERROR = "";

enum SliderChangeMode {
    BEGIN = 0,
    MOVING,
    END,
};

std::string IntTimeToText(uint32_t time)
{
    auto minutes = (time % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
    auto seconds = time % SECONDS_PER_MINUTE;
    if (time >= SECONDS_PER_HOUR) {
        auto hours = time / SECONDS_PER_HOUR;
        return StringUtils::FormatString(FORMAT_HH_MM_SS.c_str(), hours, minutes, seconds);
    }
    return StringUtils::FormatString(FORMAT_MM_SS.c_str(), minutes, seconds);
}

SizeF CalculateFitContain(const SizeF& videoSize, const SizeF& layoutSize)
{
    double layoutRatio = NearZero(layoutSize.Height()) ? 0.0 : layoutSize.Width() / layoutSize.Height();
    double sourceRatio = NearZero(videoSize.Height()) ? layoutRatio : videoSize.Width() / videoSize.Height();

    if (NearZero(layoutRatio) || NearZero(sourceRatio)) {
        return layoutSize;
    }
    if (sourceRatio < layoutRatio) {
        return { static_cast<float>(sourceRatio) * layoutSize.Height(), layoutSize.Height() };
    }
    return { layoutSize.Width(), static_cast<float>(layoutSize.Width() / sourceRatio) };
}

SizeF CalculateFitFill(const SizeF& layoutSize)
{
    return layoutSize;
}

SizeF CalculateFitCover(const SizeF& videoSize, const SizeF& layoutSize)
{
    double layoutRatio = NearZero(layoutSize.Height()) ? 0.0 : layoutSize.Width() / layoutSize.Height();
    double sourceRatio = NearZero(videoSize.Height()) ? layoutRatio : videoSize.Width() / videoSize.Height();

    if (NearZero(layoutRatio) || NearZero(sourceRatio)) {
        return layoutSize;
    }
    if (sourceRatio < layoutRatio) {
        return { layoutSize.Width(), static_cast<float>(layoutSize.Width() / sourceRatio) };
    }
    return { static_cast<float>(layoutSize.Height() * sourceRatio), layoutSize.Height() };
}

SizeF CalculateFitNone(const SizeF& videoSize)
{
    return videoSize;
}

SizeF CalculateFitScaleDown(const SizeF& videoSize, const SizeF& layoutSize)
{
    if (layoutSize.Width() > videoSize.Width()) {
        return CalculateFitNone(videoSize);
    }
    return CalculateFitContain(videoSize, layoutSize);
}

SizeF MeasureVideoContentLayout(const SizeF& layoutSize, const RefPtr<VideoLayoutProperty>& layoutProperty)
{
    if (!layoutProperty || !layoutProperty->HasVideoSize()) {
        return layoutSize;
    }

    auto videoSize = layoutProperty->GetVideoSizeValue(SizeF(0, 0));
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Video original size before fitting is %{public}s", videoSize.ToString().c_str());
    auto imageFit = layoutProperty->GetObjectFitValue(ImageFit::COVER);
    SizeF contentSize = { 0.0, 0.0 };
    switch (imageFit) {
        case ImageFit::CONTAIN:
            contentSize = CalculateFitContain(videoSize, layoutSize);
            break;
        case ImageFit::FILL:
            contentSize = CalculateFitFill(layoutSize);
            break;
        case ImageFit::COVER:
            contentSize = CalculateFitCover(videoSize, layoutSize);
            break;
        case ImageFit::NONE:
            contentSize = CalculateFitNone(videoSize);
            break;
        case ImageFit::SCALE_DOWN:
            contentSize = CalculateFitScaleDown(videoSize, layoutSize);
            break;
        default:
            contentSize = CalculateFitContain(videoSize, layoutSize);
    }

    // Just return contentSize as the video frame area.
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Video contentSize after fitting is %{public}s", contentSize.ToString().c_str());
    return contentSize;
}
} // namespace

VideoPattern::VideoPattern(const RefPtr<VideoControllerV2>& videoController) : videoControllerV2_(videoController) {}

void VideoPattern::ResetStatus()
{
    isInitialState_ = true;
    isPlaying_ = false;
#ifndef OHOS_PLATFORM
    isStop_ = false;
#endif
}

void VideoPattern::ResetMediaPlayer()
{
    mediaPlayer_->ResetMediaPlayer();
    if (!SetSourceForMediaPlayer()) {
        TAG_LOGW(AceLogTag::ACE_VIDEO, "Video set source for mediaPlayer failed.");

        // It need post on ui thread.
        FireError();
        return;
    }

    RegisterMediaPlayerEvent();
    PrepareSurface();
    if (mediaPlayer_->PrepareAsync() != 0) {
        TAG_LOGE(AceLogTag::ACE_VIDEO, "Player prepare failed");
    }
}

void VideoPattern::UpdateMediaPlayerOnBg()
{
    PrepareMediaPlayer();
    UpdateSpeed();
    UpdateLooping();
    UpdateMuted();
    if (isInitialState_ && autoPlay_) {
        // When video is autoPlay, start playing the video when it is initial state.
        Start();
    }
}

void VideoPattern::PrepareMediaPlayer()
{
    auto videoLayoutProperty = GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_VOID(videoLayoutProperty);
    // src has not set/changed
    if (!videoLayoutProperty->HasVideoSource() || videoLayoutProperty->GetVideoSource().value() == src_) {
        TAG_LOGI(AceLogTag::ACE_VIDEO, "Video source is null or the source has not changed.");
        return;
    }
    src_ = videoLayoutProperty->GetVideoSource().value();
    if (!mediaPlayer_->IsMediaPlayerValid()) {
        mediaPlayer_->CreateMediaPlayer();
    }

    if (!mediaPlayer_->IsMediaPlayerValid()) {
        TAG_LOGD(AceLogTag::ACE_VIDEO, "Video media player is invalid.");
        // It need post on ui thread.
        FireError();
        return;
    }

    ResetStatus();
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto platformTask = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::BACKGROUND);
    platformTask.PostTask([weak = WeakClaim(this)] {
        auto video = weak.Upgrade();
        CHECK_NULL_VOID(video);
        auto targetPattern = video->GetTargetVideoPattern();
        CHECK_NULL_VOID(targetPattern);
        targetPattern->ResetMediaPlayer();
    });
}

bool VideoPattern::SetSourceForMediaPlayer()
{
    TAG_LOGI(AceLogTag::ACE_VIDEO, "Video Set src for media, it is : %{public}s", src_.c_str());
    CHECK_NULL_RETURN(mediaPlayer_, false);
    return mediaPlayer_->SetSource(src_);
}

void VideoPattern::RegisterMediaPlayerEvent()
{
    if (src_.empty()) {
        TAG_LOGW(AceLogTag::ACE_VIDEO, "Video src is empty, register mediaPlayerEvent fail");
        return;
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    auto uiTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    auto videoPattern = WeakClaim(this);

    auto&& positionUpdatedEvent = [videoPattern, uiTaskExecutor](uint32_t currentPos) {
        uiTaskExecutor.PostSyncTask([&videoPattern, currentPos] {
            auto video = videoPattern.Upgrade();
            CHECK_NULL_VOID(video);
            video->OnCurrentTimeChange(currentPos);
        });
    };

    auto&& stateChangedEvent = [videoPattern, uiTaskExecutor](PlaybackStatus status) {
        uiTaskExecutor.PostSyncTask([&videoPattern, status] {
            auto video = videoPattern.Upgrade();
            CHECK_NULL_VOID(video);
            video->OnPlayerStatus(status);
        });
    };

    auto&& errorEvent = [videoPattern, uiTaskExecutor]() {
        uiTaskExecutor.PostTask([&videoPattern] {
            auto video = videoPattern.Upgrade();
            CHECK_NULL_VOID(video);
            video->OnError("");
        });
    };

    auto&& resolutionChangeEvent = [videoPattern, uiTaskExecutor]() {
        uiTaskExecutor.PostSyncTask([&videoPattern] {
            auto video = videoPattern.Upgrade();
            CHECK_NULL_VOID(video);
            video->OnResolutionChange();
        });
    };

    auto&& startRenderFrameEvent = [videoPattern, uiTaskExecutor]() {
        uiTaskExecutor.PostSyncTask([&videoPattern] {
            auto video = videoPattern.Upgrade();
            CHECK_NULL_VOID(video);
            video->OnStartRenderFrameCb();
        });
    };

    mediaPlayer_->RegisterMediaPlayerEvent(
        positionUpdatedEvent, stateChangedEvent, errorEvent, resolutionChangeEvent, startRenderFrameEvent);

#ifdef VIDEO_TEXTURE_SUPPORTED
    auto&& textureRefreshEvent = [videoPattern, uiTaskExecutor](int32_t instanceId, int64_t textureId) {
        uiTaskExecutor.PostSyncTask([&videoPattern, instanceId, textureId] {
            auto video = videoPattern.Upgrade();
            CHECK_NULL_VOID(video);
            void* nativeWindow = video->GetNativeWindow(instanceId, textureId);
            if (!nativeWindow) {
                LOGE("the native window is nullptr.");
                return;
            }
            video->OnTextureRefresh(nativeWindow);
        });
    };
    mediaPlayer_->RegisterTextureEvent(textureRefreshEvent);
#endif
}

#ifdef VIDEO_TEXTURE_SUPPORTED
void* VideoPattern::GetNativeWindow(int32_t instanceId, int64_t textureId)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    CHECK_NULL_RETURN(container, nullptr);
    auto nativeView = static_cast<AceView*>(container->GetView());
    CHECK_NULL_RETURN(nativeView, nullptr);
    return const_cast<void*>(nativeView->GetNativeWindowById(textureId));
}

void VideoPattern::OnTextureRefresh(void* surface)
{
    CHECK_NULL_VOID(surface);
    auto renderContextForMediaPlayer = renderContextForMediaPlayerWeakPtr_.Upgrade();
    CHECK_NULL_VOID(renderContextForMediaPlayer);
    renderContextForMediaPlayer->MarkNewFrameAvailable(surface);
}
#endif

void VideoPattern::PrintPlayerStatus(PlaybackStatus status)
{
    switch (status) {
        case PlaybackStatus::ERROR:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is ERROR.");
            break;
        case PlaybackStatus::IDLE:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is IDLE.");
            break;
        case PlaybackStatus::PREPARED:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is PREPARED.");
            break;
        case PlaybackStatus::STARTED:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is STARTED.");
            break;
        case PlaybackStatus::PAUSED:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is PAUSED.");
            break;
        case PlaybackStatus::STOPPED:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is STOPPED.");
            break;
        case PlaybackStatus::PLAYBACK_COMPLETE:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is PLAYBACK_COMPLETE.");
            break;
        case PlaybackStatus::NONE:
            TAG_LOGI(AceLogTag::ACE_VIDEO, "Player current status is NONE.");
            break;
        default:
            TAG_LOGW(AceLogTag::ACE_VIDEO, "Invalid player status.");
            break;
    }
}

void VideoPattern::OnCurrentTimeChange(uint32_t currentPos)
{
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Video current position is %{public}d", currentPos);
    isInitialState_ = isInitialState_ ? currentPos == 0 : false;
    if (currentPos == currentPos_ || isStop_) {
        return;
    }

    if (duration_ == 0) {
        int32_t duration = 0;
        if (mediaPlayer_->GetDuration(duration) == 0) {
            duration_ = duration / MILLISECONDS_TO_SECONDS;
            OnUpdateTime(duration_, DURATION_POS);
        }
    }

    OnUpdateTime(currentPos, CURRENT_POS);
    currentPos_ = currentPos;
    auto eventHub = GetEventHub<VideoEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto json = JsonUtil::Create(true);
    json->Put("time", static_cast<double>(currentPos));
    auto param = json->ToString();
    eventHub->FireUpdateEvent(param);
}

void VideoPattern::OnPlayerStatus(PlaybackStatus status)
{
    PrintPlayerStatus(status);
    bool isPlaying = (status == PlaybackStatus::STARTED);
    if (isPlaying_ != isPlaying) {
        isPlaying_ = isPlaying;
        ChangePlayButtonTag();
    }

    if (isInitialState_) {
        isInitialState_ = !isPlaying;
    }

    if (isPlaying) {
        auto json = JsonUtil::Create(true);
        json->Put("start", "");
        auto param = json->ToString();
        auto eventHub = GetEventHub<VideoEventHub>();
        CHECK_NULL_VOID(eventHub);
        eventHub->FireStartEvent(param);
    }

    if (status == PlaybackStatus::PAUSED) {
        auto json = JsonUtil::Create(true);
        json->Put("pause", "");
        auto param = json->ToString();
        auto eventHub = GetEventHub<VideoEventHub>();
        CHECK_NULL_VOID(eventHub);
        eventHub->FirePauseEvent(param);
    }

    if (status == PlaybackStatus::PREPARED) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        if (!mediaPlayer_ || !mediaPlayer_->IsMediaPlayerValid()) {
            return;
        }
        auto uiTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        Size videoSize = Size(mediaPlayer_->GetVideoWidth(), mediaPlayer_->GetVideoHeight());
        int32_t milliSecondDuration = 0;
        mediaPlayer_->GetDuration(milliSecondDuration);
        OnPrepared(videoSize.Width(), videoSize.Height(), milliSecondDuration / MILLISECONDS_TO_SECONDS, 0, true);
        return;
    }

    if (status == PlaybackStatus::PLAYBACK_COMPLETE) {
        OnCompletion();
    }
}

void VideoPattern::OnError(const std::string& errorId)
{
    std::string errorcode = Localization::GetInstance()->GetErrorDescription(errorId);
    auto json = JsonUtil::Create(true);
    json->Put("error", "");
    auto param = json->ToString();
    auto eventHub = GetEventHub<VideoEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireErrorEvent(param);
}

void VideoPattern::OnResolutionChange() const
{
    if (!mediaPlayer_ || !mediaPlayer_->IsMediaPlayerValid()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    SizeF videoSize =
        SizeF(static_cast<float>(mediaPlayer_->GetVideoWidth()), static_cast<float>(mediaPlayer_->GetVideoHeight()));
    auto videoLayoutProperty = host->GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_VOID(videoLayoutProperty);
    videoLayoutProperty->UpdateVideoSize(videoSize);
    TAG_LOGD(AceLogTag::ACE_VIDEO, "OnResolutionChange video size: %{public}s", videoSize.ToString().c_str());
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void VideoPattern::OnStartRenderFrameCb() const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto video = AceType::DynamicCast<VideoNode>(host);
    CHECK_NULL_VOID(video);
    auto image = AceType::DynamicCast<FrameNode>(video->GetPreviewImage());
    CHECK_NULL_VOID(image);
    auto posterLayoutProperty = image->GetLayoutProperty<ImageLayoutProperty>();
    posterLayoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
    image->MarkModifyDone();
}

void VideoPattern::OnPrepared(double width, double height, uint32_t duration, uint32_t currentPos, bool needFireEvent)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto videoLayoutProperty = host->GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_VOID(videoLayoutProperty);
    videoLayoutProperty->UpdateVideoSize(SizeF(static_cast<float>(width), static_cast<float>(height)));
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    duration_ = duration;
    currentPos_ = currentPos;
    isInitialState_ = currentPos != 0 ? false : isInitialState_;
    isPlaying_ = mediaPlayer_->IsPlaying();
    OnUpdateTime(duration_, DURATION_POS);
    OnUpdateTime(currentPos_, CURRENT_POS);

    RefPtr<UINode> controlBar = nullptr;
    auto children = host->GetChildren();
    for (const auto& child : children) {
        if (child->GetTag() == V2::ROW_ETS_TAG) {
            controlBar = child;
            break;
        }
    }
    CHECK_NULL_VOID(controlBar);
    auto sliderNode = DynamicCast<FrameNode>(controlBar->GetChildAtIndex(SLIDER_POS));
    auto sliderPaintProperty = sliderNode->GetPaintProperty<SliderPaintProperty>();
    CHECK_NULL_VOID(sliderPaintProperty);
    sliderPaintProperty->UpdateMin(0.0f);
    sliderPaintProperty->UpdateMax(static_cast<float>(duration_));
    sliderNode->MarkModifyDone();
    auto playBtn = DynamicCast<FrameNode>(controlBar->GetChildAtIndex(0));
    ChangePlayButtonTag(playBtn);

    if (needFireEvent) {
        auto json = JsonUtil::Create(true);
        json->Put("duration", static_cast<double>(duration_));
        auto param = json->ToString();
        auto eventHub = GetEventHub<VideoEventHub>();
        CHECK_NULL_VOID(eventHub);
        eventHub->FirePreparedEvent(param);
    }
    UpdateLooping();
    UpdateSpeed();
    UpdateMuted();

    checkNeedAutoPlay();
}

void VideoPattern::checkNeedAutoPlay()
{
    if (isStop_) {
        isStop_ = false;
        Start();
    }
    if (dragEndAutoPlay_) {
        dragEndAutoPlay_ = false;
        Start();
    }
    if (autoPlay_) {
        Start();
    }
}

void VideoPattern::OnCompletion()
{
    isPlaying_ = false;
    currentPos_ = duration_;
    OnUpdateTime(currentPos_, CURRENT_POS);
    auto json = JsonUtil::Create(true);
    json->Put("finish", "");
    auto param = json->ToString();
    auto eventHub = GetEventHub<VideoEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireFinishEvent(param);
}

bool VideoPattern::HasPlayer() const
{
    return mediaPlayer_ != nullptr;
}

void VideoPattern::HiddenChange(bool hidden)
{
    if (isPlaying_ && hidden && HasPlayer()) {
        pastPlayingStatus_ = isPlaying_;
        Pause();
        return;
    }

    if (!hidden && pastPlayingStatus_) {
        pastPlayingStatus_ = false;
        Start();
    }
}

void VideoPattern::OnVisibleChange(bool isVisible)
{
    if (hiddenChangeEvent_) {
        hiddenChangeEvent_(!isVisible);
    }
}

void VideoPattern::UpdateLooping()
{
    if (mediaPlayer_->IsMediaPlayerValid()) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto platformTask = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::BACKGROUND);
        platformTask.PostTask([weak = WeakClaim(RawPtr(mediaPlayer_)), loop = loop_] {
            auto mediaPlayer = weak.Upgrade();
            CHECK_NULL_VOID(mediaPlayer);
            mediaPlayer->SetLooping(loop);
        });
    }
}

void VideoPattern::UpdateSpeed()
{
    if (mediaPlayer_->IsMediaPlayerValid()) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto platformTask = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::BACKGROUND);
        platformTask.PostTask([weak = WeakClaim(RawPtr(mediaPlayer_)), progress = progressRate_] {
            auto mediaPlayer = weak.Upgrade();
            CHECK_NULL_VOID(mediaPlayer);
            mediaPlayer->SetPlaybackSpeed(static_cast<float>(progress));
        });
    }
}

void VideoPattern::UpdateMuted()
{
    if (mediaPlayer_->IsMediaPlayerValid()) {
        float volume = muted_ ? 0.0f : 1.0f;
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto platformTask = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::BACKGROUND);
        platformTask.PostTask([weak = WeakClaim(RawPtr(mediaPlayer_)), videoVolume = volume] {
            auto mediaPlayer = weak.Upgrade();
            CHECK_NULL_VOID(mediaPlayer);
            mediaPlayer->SetVolume(videoVolume, videoVolume);
        });
    }
}

void VideoPattern::OnUpdateTime(uint32_t time, int pos) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    bool needControlBar = layoutProperty->GetControlsValue(true);
    if (!needControlBar) {
        return;
    }

    RefPtr<UINode> controlBar = nullptr;
    auto children = host->GetChildren();
    for (const auto& child : children) {
        if (child->GetTag() == V2::ROW_ETS_TAG) {
            controlBar = child;
            break;
        }
    }

    CHECK_NULL_VOID(controlBar);
    auto durationNode = DynamicCast<FrameNode>(controlBar->GetChildAtIndex(pos));
    CHECK_NULL_VOID(durationNode);
    auto textLayoutProperty = durationNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    std::string timeText = IntTimeToText(time);
    textLayoutProperty->UpdateContent(timeText);
    durationNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    durationNode->MarkModifyDone();
    if (pos == CURRENT_POS) {
        auto sliderNode = DynamicCast<FrameNode>(controlBar->GetChildAtIndex(SLIDER_POS));
        CHECK_NULL_VOID(sliderNode);
        auto sliderPattern = sliderNode->GetPattern<SliderPattern>();
        CHECK_NULL_VOID(sliderPattern);
        sliderPattern->UpdateValue(static_cast<float>(time));
        sliderNode->MarkModifyDone();
    }
}

void VideoPattern::PrepareSurface()
{
    if (renderSurface_->IsSurfaceValid()) {
        return;
    }
    if (!SystemProperties::GetExtSurfaceEnabled()) {
        renderSurface_->SetRenderContext(renderContextForMediaPlayer_);
    }
    renderSurface_->InitSurface();
    mediaPlayer_->SetRenderSurface(renderSurface_);
    if (mediaPlayer_->SetSurface() != 0) {
        TAG_LOGW(AceLogTag::ACE_VIDEO, "mediaPlayer renderSurface set failed");
    }
}

void VideoPattern::OnAttachToFrameNode()
{
    // full screen node is not supposed to register js controller event
    if (!InstanceOf<VideoFullScreenPattern>(this)) {
        SetMethodCall();
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    static RenderContext::ContextParam param = { RenderContext::ContextType::HARDWARE_SURFACE, "MediaPlayerSurface" };
    renderContextForMediaPlayer_->InitContext(false, param);

    if (SystemProperties::GetExtSurfaceEnabled()) {
        RegisterRenderContextCallBack();
    }

    renderContext->UpdateBackgroundColor(Color::BLACK);
    renderContextForMediaPlayer_->UpdateBackgroundColor(Color::BLACK);
    renderContext->SetClipToBounds(true);
}

void VideoPattern::RegisterRenderContextCallBack()
{
#ifndef VIDEO_TEXTURE_SUPPORTED
    auto isFullScreen = IsFullScreen();
    if (!isFullScreen) {
        auto OnAreaChangedCallBack = [weak = WeakClaim(this)](float x, float y, float w, float h) mutable {
            auto videoPattern = weak.Upgrade();
            CHECK_NULL_VOID(videoPattern);
            auto host = videoPattern->GetHost();
            CHECK_NULL_VOID(host);
            auto geometryNode = host->GetGeometryNode();
            CHECK_NULL_VOID(geometryNode);
            auto videoNodeSize = geometryNode->GetContentSize();
            auto layoutProperty = videoPattern->GetLayoutProperty<VideoLayoutProperty>();
            CHECK_NULL_VOID(layoutProperty);
            auto videoFrameSize = MeasureVideoContentLayout(videoNodeSize, layoutProperty);
            Rect rect = Rect(x + (videoNodeSize.Width() - videoFrameSize.Width()) / AVERAGE_VALUE,
                y + (videoNodeSize.Height() - videoFrameSize.Height()) / AVERAGE_VALUE, videoFrameSize.Width(),
                videoFrameSize.Height());
            if (videoPattern->renderSurface_) {
                if (videoPattern->renderSurface_->SetExtSurfaceBoundsSync(rect.Left(),
                    rect.Top(), rect.Width(), rect.Height())) {
                    videoPattern->lastBoundsRect_ = rect;
                }
            }
        };
        renderContextForMediaPlayer_->SetSurfaceChangedCallBack(OnAreaChangedCallBack);
    }
#else
    renderSurfaceWeakPtr_ = renderSurface_;
    renderContextForMediaPlayerWeakPtr_ = renderContextForMediaPlayer_;
    auto OnAttachCallBack = [weak = WeakClaim(this)](int64_t textureId, bool isAttach) mutable {
        LOGI("OnAttachCallBack.");
        auto videoPattern = weak.Upgrade();
        CHECK_NULL_VOID(videoPattern);
        if (auto renderSurface = videoPattern->renderSurfaceWeakPtr_.Upgrade(); renderSurface) {
            renderSurface->AttachToGLContext(textureId, isAttach);
        }
    };
    renderContextForMediaPlayer_->AddAttachCallBack(OnAttachCallBack);
    auto OnUpdateCallBack = [weak = WeakClaim(this)](std::vector<float>& matrix) mutable {
        auto videoPattern = weak.Upgrade();
        CHECK_NULL_VOID(videoPattern);
        if (auto renderSurface = videoPattern->renderSurfaceWeakPtr_.Upgrade(); renderSurface) {
            renderSurface->UpdateTextureImage(matrix);
        }
    };
    renderContextForMediaPlayer_->AddUpdateCallBack(OnUpdateCallBack);
#endif
}

void VideoPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    if (!hiddenChangeEvent_) {
        SetHiddenChangeEvent([weak = WeakClaim(this)](bool hidden) {
            auto videoPattern = weak.Upgrade();
            CHECK_NULL_VOID(videoPattern);
            auto fullScreenNode = videoPattern->GetFullScreenNode();
            if (fullScreenNode) {
                auto fullScreenPattern = AceType::DynamicCast<VideoFullScreenPattern>(fullScreenNode->GetPattern());
                CHECK_NULL_VOID(fullScreenPattern);
                fullScreenPattern->HiddenChange(hidden);
                return;
            }
            videoPattern->HiddenChange(hidden);
        });
    }

    // update full screen pattern state
    UpdateFsState();

    // Update the control bar and preview image.
    UpdatePreviewImage();
    UpdateControllerBar();

    // Update the media player when video node is not in full screen or current node is full screen node
    if (!fullScreenNodeId_.has_value() || InstanceOf<VideoFullScreenNode>(this)) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto uiTaskExecutor = SingleTaskExecutor::Make(pipelineContext->GetTaskExecutor(), TaskExecutor::TaskType::UI);
        uiTaskExecutor.PostTask([weak = WeakClaim(this)]() {
            auto videoPattern = weak.Upgrade();
            CHECK_NULL_VOID(videoPattern);
            TAG_LOGD(AceLogTag::ACE_VIDEO, "trigger modify media player");
            videoPattern->UpdateMediaPlayerOnBg();
        });
    }

    if (SystemProperties::GetExtSurfaceEnabled()) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        pipelineContext->AddOnAreaChangeNode(host->GetId());
    }
    EnableDrag();
}

void VideoPattern::UpdatePreviewImage()
{
    auto layoutProperty = GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (!layoutProperty->HasPosterImageInfo()) {
        return;
    }
    auto posterSourceInfo = layoutProperty->GetPosterImageInfo().value();
    auto imageFit = layoutProperty->GetObjectFitValue(ImageFit::COVER);
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto video = AceType::DynamicCast<VideoNode>(host);
    CHECK_NULL_VOID(video);
    auto image = AceType::DynamicCast<FrameNode>(video->GetPreviewImage());
    CHECK_NULL_VOID(image);
    if (!isInitialState_ || !layoutProperty->HasPosterImageInfo()) {
        auto posterLayoutProperty = image->GetLayoutProperty<ImageLayoutProperty>();
        posterLayoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
        image->MarkModifyDone();
        return;
    }

    if (!posterSourceInfo.IsValid()) {
        auto posterLayoutProperty = image->GetLayoutProperty<ImageLayoutProperty>();
        posterLayoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
        image->MarkModifyDone();
        TAG_LOGI(AceLogTag::ACE_VIDEO, "Src image is not valid.");
        return;
    }

    if (image) {
        image->SetDraggable(false);
        auto posterLayoutProperty = image->GetLayoutProperty<ImageLayoutProperty>();
        posterLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
        posterLayoutProperty->UpdateImageSourceInfo(posterSourceInfo);
        posterLayoutProperty->UpdateImageFit(imageFit);
        image->MarkModifyDone();
    }
}

void VideoPattern::UpdateControllerBar()
{
    auto layoutProperty = GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    if (layoutProperty->GetControlsValue(true)) {
        auto video = AceType::DynamicCast<VideoNode>(host);
        CHECK_NULL_VOID(video);
        auto controller = AceType::DynamicCast<FrameNode>(video->GetControllerRow());
        if (controller) {
            auto sliderNode = DynamicCast<FrameNode>(controller->GetChildAtIndex(SLIDER_POS));
            CHECK_NULL_VOID(sliderNode);
            auto sliderPattern = sliderNode->GetPattern<SliderPattern>();
            CHECK_NULL_VOID(sliderPattern);
            sliderPattern->UpdateValue(static_cast<float>(currentPos_));
            sliderNode->MarkModifyDone();

            auto textNode = DynamicCast<FrameNode>(controller->GetChildAtIndex(CURRENT_POS));
            CHECK_NULL_VOID(textNode);
            auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
            CHECK_NULL_VOID(textLayoutProperty);
            std::string label = IntTimeToText(currentPos_);
            textLayoutProperty->UpdateContent(label);

            auto durationNode = DynamicCast<FrameNode>(controller->GetChildAtIndex(DURATION_POS));
            CHECK_NULL_VOID(durationNode);
            auto durationTextLayoutProperty = durationNode->GetLayoutProperty<TextLayoutProperty>();
            CHECK_NULL_VOID(durationTextLayoutProperty);
            std::string durationText = IntTimeToText(duration_);
            durationTextLayoutProperty->UpdateContent(durationText);

            textNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            textNode->MarkModifyDone();
            durationNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            durationNode->MarkModifyDone();
            auto controllerLayoutProperty = controller->GetLayoutProperty<LinearLayoutProperty>();
            controllerLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
            controller->MarkModifyDone();
        }
    } else {
        auto video = AceType::DynamicCast<VideoNode>(host);
        CHECK_NULL_VOID(video);
        auto controller = AceType::DynamicCast<FrameNode>(video->GetControllerRow());
        CHECK_NULL_VOID(controller);
        if (controller) {
            auto controllerLayoutProperty = controller->GetLayoutProperty<LinearLayoutProperty>();
            controllerLayoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
            controller->MarkModifyDone();
        }
    }
}

void VideoPattern::UpdateVideoProperty()
{
    if (isInitialState_ && autoPlay_) {
        Start();
    }

    UpdateSpeed();
    UpdateLooping();
    UpdateMuted();
}

void VideoPattern::OnRebuildFrame()
{
    if (!renderSurface_ || !renderSurface_->IsSurfaceValid()) {
        TAG_LOGW(AceLogTag::ACE_VIDEO, "MediaPlayer surface is not valid");
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto video = AceType::DynamicCast<VideoNode>(host);
    CHECK_NULL_VOID(video);
    auto column = AceType::DynamicCast<FrameNode>(video->GetMediaColumn());
    CHECK_NULL_VOID(column);
    auto renderContext = column->GetRenderContext();
    renderContext->AddChild(renderContextForMediaPlayer_, 0);
}

bool VideoPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure || dirty->SkipMeasureContent()) {
        TAG_LOGD(AceLogTag::ACE_VIDEO, "Video skip measure.");
        return false;
    }
    auto geometryNode = dirty->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);
    auto videoNodeSize = geometryNode->GetContentSize();
    auto layoutProperty = GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, false);
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Video node size is %{public}s", videoNodeSize.ToString().c_str());
    auto videoFrameSize = MeasureVideoContentLayout(videoNodeSize, layoutProperty);
    // Change the surface layout for drawing video frames
    if (renderContextForMediaPlayer_) {
        renderContextForMediaPlayer_->SetBounds((videoNodeSize.Width() - videoFrameSize.Width()) / AVERAGE_VALUE,
            (videoNodeSize.Height() - videoFrameSize.Height()) / AVERAGE_VALUE, videoFrameSize.Width(),
            videoFrameSize.Height());
        TAG_LOGD(AceLogTag::ACE_VIDEO,
            "Video renderContext for mediaPlayer position x is %{public}lf,y is %{public}lf,width is "
            "%{public}lf,height "
            "is %{public}lf.",
            (videoNodeSize.Width() - videoFrameSize.Width()) / AVERAGE_VALUE,
            (videoNodeSize.Height() - videoFrameSize.Height()) / AVERAGE_VALUE, videoFrameSize.Width(),
            videoFrameSize.Height());
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkNeedSyncRenderTree();
    auto video = AceType::DynamicCast<VideoNode>(host);
    CHECK_NULL_RETURN(video, false);
    auto column = AceType::DynamicCast<FrameNode>(video->GetMediaColumn());
    CHECK_NULL_RETURN(column, false);
    column->GetRenderContext()->SetClipToBounds(true);
    return false;
}

void VideoPattern::OnAreaChangedInner()
{
#ifndef VIDEO_TEXTURE_SUPPORTED
    auto isFullScreen = IsFullScreen();
    if (SystemProperties::GetExtSurfaceEnabled() && isFullScreen) {
#else
    if (SystemProperties::GetExtSurfaceEnabled()) {
#endif
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto geometryNode = host->GetGeometryNode();
        CHECK_NULL_VOID(geometryNode);
        auto videoNodeSize = geometryNode->GetContentSize();
        auto layoutProperty = GetLayoutProperty<VideoLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        auto videoFrameSize = MeasureVideoContentLayout(videoNodeSize, layoutProperty);
        auto transformRelativeOffset = host->GetTransformRelativeOffset();

        Rect rect =
            Rect(transformRelativeOffset.GetX() + (videoNodeSize.Width() - videoFrameSize.Width()) / AVERAGE_VALUE,
                transformRelativeOffset.GetY() + (videoNodeSize.Height() - videoFrameSize.Height()) / AVERAGE_VALUE,
                videoFrameSize.Width(), videoFrameSize.Height());
        if (renderSurface_ && (rect != lastBoundsRect_)) {
            renderSurface_->SetExtSurfaceBounds(rect.Left(), rect.Top(), rect.Width(), rect.Height());
            lastBoundsRect_ = rect;
        }
    }
}

void VideoPattern::OnColorConfigurationUpdate()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto videoTheme = pipelineContext->GetTheme<VideoTheme>();
    CHECK_NULL_VOID(videoTheme);
    auto renderContext = controlBar_->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateBackgroundColor(videoTheme->GetBkgColor());
    for (const auto& child : controlBar_->GetChildren()) {
        if (child->GetTag() == V2::TEXT_ETS_TAG) {
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            if (frameNode) {
                auto textLayoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
                if (textLayoutProperty) {
                    auto textStyle = videoTheme->GetTimeTextStyle();
                    textLayoutProperty->UpdateTextColor(textStyle.GetTextColor());
                }
            }
        }
    }
    host->SetNeedCallChildrenUpdate(false);
    host->MarkModifyDone();
    host->MarkDirtyNode();
}

RefPtr<FrameNode> VideoPattern::CreateControlBar(int32_t nodeId)
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    auto videoTheme = pipelineContext->GetTheme<VideoTheme>();
    CHECK_NULL_RETURN(videoTheme, nullptr);
    auto controlBar = FrameNode::GetOrCreateFrameNode(
        V2::ROW_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<LinearLayoutPattern>(false); });
    CHECK_NULL_RETURN(controlBar, nullptr);
    controlBar_ = controlBar;

    auto playButton = CreateSVG();
    CHECK_NULL_RETURN(playButton, nullptr);
    ChangePlayButtonTag(playButton);
    controlBar->AddChild(playButton);

    auto currentPosText = CreateText(currentPos_);
    CHECK_NULL_RETURN(currentPosText, nullptr);
    controlBar->AddChild(currentPosText);

    auto slider = CreateSlider();
    CHECK_NULL_RETURN(currentPosText, nullptr);
    controlBar->AddChild(slider);

    auto durationText = CreateText(duration_);
    CHECK_NULL_RETURN(durationText, nullptr);
    controlBar->AddChild(durationText);

    auto fullScreenButton = CreateSVG();
    CHECK_NULL_RETURN(fullScreenButton, nullptr);
    SetFullScreenButtonCallBack(fullScreenButton);
    ChangeFullScreenButtonTag(InstanceOf<VideoFullScreenNode>(this), fullScreenButton);
    controlBar->AddChild(fullScreenButton);

    auto renderContext = controlBar->GetRenderContext();
    renderContext->UpdateBackgroundColor(videoTheme->GetBkgColor());
    auto controlBarLayoutProperty = controlBar->GetLayoutProperty<LinearLayoutProperty>();
    controlBarLayoutProperty->UpdateMainAxisAlign(FlexAlign::SPACE_BETWEEN);
    return controlBar;
}

RefPtr<FrameNode> VideoPattern::CreateSlider()
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    auto videoTheme = pipelineContext->GetTheme<VideoTheme>();
    CHECK_NULL_RETURN(videoTheme, nullptr);

    auto sliderNode = FrameNode::CreateFrameNode(V2::SLIDER_ETS_TAG, -1, AceType::MakeRefPtr<SliderPattern>());
    CHECK_NULL_RETURN(sliderNode, nullptr);
    auto sliderLayoutProperty = sliderNode->GetLayoutProperty<SliderLayoutProperty>();
    CHECK_NULL_RETURN(sliderLayoutProperty, nullptr);

    auto sliderEdge = videoTheme->GetSliderEdge();
    PaddingProperty padding;
    padding.left = CalcLength(sliderEdge.Left());
    padding.right = CalcLength(sliderEdge.Right());
    padding.top = CalcLength(sliderEdge.Top());
    padding.bottom = CalcLength(sliderEdge.Bottom());
    sliderLayoutProperty->UpdatePadding(padding);
    sliderLayoutProperty->UpdateLayoutWeight(1.0);

    SliderOnChangeEvent sliderOnChangeEvent = [weak = WeakClaim(this)](float value, int32_t mode) {
        auto videoPattern = weak.Upgrade();
        CHECK_NULL_VOID(videoPattern);
        videoPattern->OnSliderChange(value, mode);
    };
    auto sliderEventHub = sliderNode->GetEventHub<SliderEventHub>();
    sliderEventHub->SetOnChange(std::move(sliderOnChangeEvent));

    auto sliderPaintProperty = sliderNode->GetPaintProperty<SliderPaintProperty>();
    CHECK_NULL_RETURN(sliderPaintProperty, nullptr);
    sliderPaintProperty->UpdateMax(static_cast<float>(duration_));
    sliderPaintProperty->UpdateSelectColor(Color::BLACK);
    sliderPaintProperty->UpdateValue(static_cast<float>(currentPos_));
    sliderNode->MarkModifyDone();
    return sliderNode;
}

RefPtr<FrameNode> VideoPattern::CreateText(uint32_t time)
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    auto videoTheme = pipelineContext->GetTheme<VideoTheme>();
    CHECK_NULL_RETURN(videoTheme, nullptr);

    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    auto videoLayoutProperty = GetLayoutProperty<VideoLayoutProperty>();
    CHECK_NULL_RETURN(videoLayoutProperty, nullptr);
    std::string label = "";
    if (videoLayoutProperty->GetControlsValue(true)) {
        label = IntTimeToText(time);
    }
    textLayoutProperty->UpdateContent(label);
    auto textEdge = videoTheme->GetTextEdge();
    PaddingProperty padding;
    padding.left = CalcLength(textEdge.Left());
    padding.right = CalcLength(textEdge.Right());
    padding.top = CalcLength(textEdge.Top());
    padding.bottom = CalcLength(textEdge.Bottom());
    textLayoutProperty->UpdatePadding(padding);
    auto textStyle = videoTheme->GetTimeTextStyle();
    textLayoutProperty->UpdateFontSize(textStyle.GetFontSize());
    textLayoutProperty->UpdateTextColor(textStyle.GetTextColor());
    return textNode;
}

RefPtr<FrameNode> VideoPattern::CreateSVG()
{
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, nullptr);
    auto videoTheme = pipelineContext->GetTheme<VideoTheme>();
    CHECK_NULL_RETURN(videoTheme, nullptr);

    auto svgNode = FrameNode::CreateFrameNode(V2::IMAGE_ETS_TAG, -1, AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_RETURN(svgNode, nullptr);
    auto svgLayoutProperty = svgNode->GetLayoutProperty<ImageLayoutProperty>();

    auto btnEdge = videoTheme->GetBtnEdge();
    PaddingProperty padding;
    padding.left = CalcLength(btnEdge.Left());
    padding.right = CalcLength(btnEdge.Right());
    padding.top = CalcLength(btnEdge.Top());
    padding.bottom = CalcLength(btnEdge.Bottom());
    svgLayoutProperty->UpdatePadding(padding);

    auto btnSize = videoTheme->GetBtnSize();
    SizeF size { static_cast<float>(btnSize.Width()), static_cast<float>(btnSize.Height()) };
    svgLayoutProperty->UpdateMarginSelfIdealSize(size);
    auto width = Dimension(btnSize.Width(), DimensionUnit::VP).ConvertToPx();
    auto height = Dimension(btnSize.Height(), DimensionUnit::VP).ConvertToPx();
    CalcSize idealSize = { CalcLength(width), CalcLength(height) };
    MeasureProperty layoutConstraint;
    layoutConstraint.selfIdealSize = idealSize;
    layoutConstraint.maxSize = idealSize;
    svgNode->UpdateLayoutConstraint(layoutConstraint);
    return svgNode;
}

void VideoPattern::SetMethodCall()
{
    auto videoController = AceType::MakeRefPtr<VideoController>();
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto uiTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    videoController->SetStartImpl([weak = WeakClaim(this), uiTaskExecutor]() {
        uiTaskExecutor.PostTask([weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto targetPattern = pattern->GetTargetVideoPattern();
            CHECK_NULL_VOID(targetPattern);
            targetPattern->Start();
        });
    });
    videoController->SetPausetImpl([weak = WeakClaim(this), uiTaskExecutor]() {
        uiTaskExecutor.PostTask([weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto targetPattern = pattern->GetTargetVideoPattern();
            CHECK_NULL_VOID(targetPattern);
            targetPattern->Pause();
        });
    });
    videoController->SetStopImpl([weak = WeakClaim(this), uiTaskExecutor]() {
        uiTaskExecutor.PostTask([weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto targetPattern = pattern->GetTargetVideoPattern();
            CHECK_NULL_VOID(targetPattern);
            targetPattern->Stop();
        });
    });
    videoController->SetSeekToImpl([weak = WeakClaim(this), uiTaskExecutor](float pos, SeekMode seekMode) {
        uiTaskExecutor.PostTask([weak, pos, seekMode]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto targetPattern = pattern->GetTargetVideoPattern();
            CHECK_NULL_VOID(targetPattern);
            targetPattern->SetCurrentTime(pos, seekMode);
        });
    });
    videoController->SetRequestFullscreenImpl([weak = WeakClaim(this), uiTaskExecutor](bool isFullScreen) {
        uiTaskExecutor.PostTask([weak, isFullScreen]() {
            auto videoPattern = weak.Upgrade();
            CHECK_NULL_VOID(videoPattern);
            if (isFullScreen) {
                videoPattern->FullScreen();
            } else {
                videoPattern->ResetLastBoundsRect();
                auto targetPattern = videoPattern->GetTargetVideoPattern();
                CHECK_NULL_VOID(targetPattern);
                auto fullScreenPattern = AceType::DynamicCast<VideoFullScreenPattern>(targetPattern);
                CHECK_NULL_VOID(fullScreenPattern);
                fullScreenPattern->ExitFullScreen();
            }
        });
    });
    videoController->SetExitFullscreenImpl([weak = WeakClaim(this), uiTaskExecutor](bool isSync) {
        if (isSync) {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            auto targetPattern = pattern->GetTargetVideoPattern();
            CHECK_NULL_VOID(targetPattern);
            pattern->ResetLastBoundsRect();
            auto fullScreenPattern = AceType::DynamicCast<VideoFullScreenPattern>(targetPattern);
            fullScreenPattern->ExitFullScreen();
            return;
        }
        uiTaskExecutor.PostTask([weak]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->ResetLastBoundsRect();
            auto targetPattern = pattern->GetTargetVideoPattern();
            CHECK_NULL_VOID(targetPattern);
            auto fullScreenPattern = AceType::DynamicCast<VideoFullScreenPattern>(targetPattern);
            CHECK_NULL_VOID(fullScreenPattern);
            fullScreenPattern->ExitFullScreen();
        });
    });
    CHECK_NULL_VOID(videoControllerV2_);
    videoControllerV2_->AddVideoController(videoController);
}

void VideoPattern::Start()
{
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Video start to play");
    if (!mediaPlayer_ || !mediaPlayer_->IsMediaPlayerValid()) {
        return;
    }

    if (isStop_ && mediaPlayer_->PrepareAsync() != 0) {
        TAG_LOGW(AceLogTag::ACE_VIDEO, "Player has not prepared");
        return;
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    auto platformTask = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::BACKGROUND);
    platformTask.PostTask([weak = WeakClaim(RawPtr(mediaPlayer_))] {
        auto mediaPlayer = weak.Upgrade();
        CHECK_NULL_VOID(mediaPlayer);
        TAG_LOGI(AceLogTag::ACE_VIDEO, "trigger mediaPlayer play");
        mediaPlayer->Play();
    });
}

void VideoPattern::Pause()
{
    if (!mediaPlayer_ || !mediaPlayer_->IsMediaPlayerValid()) {
        return;
    }
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Video Pause");
    mediaPlayer_->Pause();
}

void VideoPattern::Stop()
{
    if (!mediaPlayer_ || !mediaPlayer_->IsMediaPlayerValid()) {
        return;
    }

    OnCurrentTimeChange(0);
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Video Stop");
    mediaPlayer_->Stop();
    isStop_ = true;
}

void VideoPattern::FireError()
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    // OnError function must be excuted on ui, so get the uiTaskExecutor.
    auto uiTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    uiTaskExecutor.PostTask([weak = WeakClaim(this)] {
        auto videoPattern = weak.Upgrade();
        CHECK_NULL_VOID(videoPattern);
        videoPattern->OnError("");
    });
}

void VideoPattern::ChangePlayButtonTag()
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    const auto& children = host->GetChildren();
    for (const auto& child : children) {
        if (child->GetTag() == V2::ROW_ETS_TAG) {
            auto playBtn = DynamicCast<FrameNode>(child->GetChildAtIndex(0));
            ChangePlayButtonTag(playBtn);
            break;
        }
    }
}

void VideoPattern::ChangePlayButtonTag(RefPtr<FrameNode>& playBtn)
{
    CHECK_NULL_VOID(playBtn);
    auto playClickCallback = [weak = WeakClaim(this), playing = isPlaying_](GestureEvent& /* info */) {
        auto videoPattern = weak.Upgrade();
        CHECK_NULL_VOID(videoPattern);
        if (playing) {
            videoPattern->Pause();
        } else {
            videoPattern->Start();
        }
    };
    auto playBtnEvent = playBtn->GetOrCreateGestureEventHub();
    playBtnEvent->SetUserOnClick(std::move(playClickCallback));
    auto svgLayoutProperty = playBtn->GetLayoutProperty<ImageLayoutProperty>();
    auto resourceId = isPlaying_ ? InternalResource::ResourceId::PAUSE_SVG : InternalResource::ResourceId::PLAY_SVG;
    auto svgSourceInfo = ImageSourceInfo("");
    svgSourceInfo.SetResourceId(resourceId);
    svgLayoutProperty->UpdateImageSourceInfo(svgSourceInfo);
    playBtn->MarkModifyDone();
    playBtn->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void VideoPattern::SetFullScreenButtonCallBack(RefPtr<FrameNode>& fullScreenBtn)
{
    CHECK_NULL_VOID(fullScreenBtn);
    auto fsClickCallback = [weak = WeakClaim(this)](GestureEvent& /* info */) {
        auto videoPattern = weak.Upgrade();
        CHECK_NULL_VOID(videoPattern);
        if (InstanceOf<VideoFullScreenPattern>(videoPattern)) {
            auto pattern = AceType::DynamicCast<VideoFullScreenPattern>(videoPattern);
            CHECK_NULL_VOID(pattern);
            videoPattern->ResetLastBoundsRect();
            pattern->ExitFullScreen();
        } else {
            videoPattern->FullScreen();
        }
    };
    auto fullScreenBtnEvent = fullScreenBtn->GetOrCreateGestureEventHub();
    fullScreenBtnEvent->SetUserOnClick(std::move(fsClickCallback));
}

void VideoPattern::ChangeFullScreenButtonTag(bool isFullScreen, RefPtr<FrameNode>& fullScreenBtn)
{
    CHECK_NULL_VOID(fullScreenBtn);
    auto svgLayoutProperty = fullScreenBtn->GetLayoutProperty<ImageLayoutProperty>();
    auto resourceId =
        isFullScreen ? InternalResource::ResourceId::QUIT_FULLSCREEN_SVG : InternalResource::ResourceId::FULLSCREEN_SVG;
    auto svgSourceInfo = ImageSourceInfo("");
    svgSourceInfo.SetResourceId(resourceId);
    svgLayoutProperty->UpdateImageSourceInfo(svgSourceInfo);
    fullScreenBtn->MarkModifyDone();
    fullScreenBtn->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void VideoPattern::SetCurrentTime(float currentPos, OHOS::Ace::SeekMode seekMode)
{
    TAG_LOGD(AceLogTag::ACE_VIDEO, "Set current pos: %{public}lf, mode: %{public}d", currentPos, seekMode);
    if (!mediaPlayer_ || !mediaPlayer_->IsMediaPlayerValid()) {
        return;
    }
    if (GreatOrEqual(currentPos, 0.0)) {
        TAG_LOGD(AceLogTag::ACE_VIDEO, "Video Seek");
        mediaPlayer_->Seek(static_cast<int32_t>(currentPos * MILLISECONDS_TO_SECONDS), seekMode);
    }
}

void VideoPattern::OnSliderChange(float posTime, int32_t mode)
{
    TAG_LOGD(AceLogTag::ACE_VIDEO, "posTime: %{public}lf, mode: %{public}d", posTime, mode);
    SetCurrentTime(posTime, OHOS::Ace::SeekMode::SEEK_PREVIOUS_SYNC);
    auto eventHub = GetEventHub<VideoEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto json = JsonUtil::Create(true);
    json->Put("time", static_cast<double>(posTime));
    auto param = json->ToString();
    CHECK_NULL_VOID(eventHub);
    if (mode == SliderChangeMode::BEGIN || mode == SliderChangeMode::MOVING) {
        eventHub->FireSeekingEvent(param);
    } else if (mode == SliderChangeMode::END) {
        eventHub->FireSeekedEvent(param);
    }
}

void VideoPattern::OnFullScreenChange(bool isFullScreen)
{
    auto json = JsonUtil::Create(true);
    json->Put("fullscreen", isFullScreen);
    auto param = json->ToString();
    auto eventHub = GetEventHub<VideoEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->FireFullScreenChangeEvent(param);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    const auto& children = host->GetChildren();
    for (const auto& child : children) {
        if (child->GetTag() == V2::ROW_ETS_TAG) {
            auto fsBtn = DynamicCast<FrameNode>(child->GetChildAtIndex(FULL_SCREEN_POS));
            ChangeFullScreenButtonTag(isFullScreen, fsBtn);
            break;
        }
    }
    if (!SystemProperties::GetExtSurfaceEnabled()) {
        return;
    }
    if (!fullScreenNodeId_.has_value()) {
        SetMediaFullScreen(isFullScreen);
        return;
    }
    auto fullScreenNode = FrameNode::GetFrameNode(V2::VIDEO_ETS_TAG, fullScreenNodeId_.value());
    CHECK_NULL_VOID(fullScreenNode);
    auto fullScreenPattern = AceType::DynamicCast<VideoFullScreenPattern>(fullScreenNode->GetPattern());
    CHECK_NULL_VOID(fullScreenPattern);
    fullScreenPattern->SetMediaFullScreen(isFullScreen);
}

void VideoPattern::FullScreen()
{
    if (fullScreenNodeId_.has_value()) {
        return;
    }
    ResetLastBoundsRect();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto videoNode = AceType::DynamicCast<VideoNode>(host);
    CHECK_NULL_VOID(videoNode);
    auto fullScreenPattern = AceType::MakeRefPtr<VideoFullScreenPattern>(videoControllerV2_);
    fullScreenPattern->InitFullScreenParam(
        AceType::Claim(this), renderSurface_, mediaPlayer_, renderContextForMediaPlayer_);
    fullScreenNodeId_ = ElementRegister::GetInstance()->MakeUniqueId();
    auto fullScreenNode =
        VideoFullScreenNode::CreateFullScreenNode(V2::VIDEO_ETS_TAG, fullScreenNodeId_.value(), fullScreenPattern);
    CHECK_NULL_VOID(fullScreenNode);
    fullScreenPattern->RequestFullScreen(videoNode);
}

void VideoPattern::EnableDrag()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = GetLayoutProperty<VideoLayoutProperty>();
    std::string videoSrcBefore = src_;
    std::string imageSrcBefore = "";
    if (layoutProperty->HasPosterImageInfo()) {
        imageSrcBefore = layoutProperty->GetPosterImageInfo().value().GetSrc();
    }
#ifdef ENABLE_DRAG_FRAMEWORK
    auto dragEnd = [this, videoSrcBefore](const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) {
        auto videoLayoutProperty = this->GetLayoutProperty<VideoLayoutProperty>();
        CHECK_NULL_VOID(videoLayoutProperty);
        auto unifiedData = event->GetData();
        std::string videoSrc = "";
        if (unifiedData != nullptr) {
            int ret = UdmfClient::GetInstance()->GetVideoRecordUri(unifiedData, videoSrc);
            if (ret != 0) {
                TAG_LOGW(AceLogTag::ACE_VIDEO, "unifiedRecords is empty");
                return;
            }
        } else {
            auto json = JsonUtil::ParseJsonString(extraParams);
            std::string key = "extraInfo";
            videoSrc = json->GetString(key);
        }

        if (videoSrc == videoSrcBefore) {
            return;
        }

        dragEndAutoPlay_ = true;
        videoLayoutProperty->UpdateVideoSource(videoSrc);

        auto frameNode = this->GetHost();
        frameNode->MarkModifyDone();
    };
#else
    auto dragEnd = [this, videoSrcBefore, imageSrcBefore](
                       const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) {
        if (extraParams.empty()) {
            TAG_LOGW(AceLogTag::ACE_VIDEO, "extraParams is empty");
            return;
        }
        auto videoLayoutProperty = this->GetLayoutProperty<VideoLayoutProperty>();
        CHECK_NULL_VOID(videoLayoutProperty);
        auto json = JsonUtil::ParseJsonString(extraParams);
        std::string key = "extraInfo";
        std::string extraInfo = json->GetString(key);
        if (extraInfo.empty()) {
            TAG_LOGW(AceLogTag::ACE_VIDEO, "extraInfo is empty");
            return;
        }
        int index = extraInfo.find("::");
        if (index < 0 || index == extraInfo.length() - 2) {
            TAG_LOGW(AceLogTag::ACE_VIDEO, "video source is empty");
            return;
        }
        std::string videoSrc = extraInfo.substr(index + 2); // 2 :the length of "::"
        std::string imageSrc = "";
        if (index != 0) {
            imageSrc = extraInfo.substr(0, index);
        }

        bool isInitialState = this->isInitialState_;
        if ((!isInitialState && videoSrc == videoSrcBefore) ||
            (isInitialState && videoSrc == videoSrcBefore && imageSrc == imageSrcBefore)) {
            return;
        }

        dragEndAutoPlay_ = true;
        videoLayoutProperty->UpdateVideoSource(videoSrc);
        ImageSourceInfo imageSourceInfo = ImageSourceInfo(imageSrc);
        videoLayoutProperty->UpdatePosterImageInfo(imageSourceInfo);

        if (!isInitialState) {
            this->SetIsStop(true);
        }
        auto frameNode = this->GetHost();
        frameNode->MarkModifyDone();
    };
#endif
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDrop(std::move(dragEnd));
}

VideoPattern::~VideoPattern()
{
    if (renderContextForMediaPlayer_) {
        renderContextForMediaPlayer_->RemoveSurfaceChangedCallBack();
    }
    if (!fullScreenNodeId_.has_value()) {
        return;
    }
    auto fullScreenNode = FrameNode::GetFrameNode(V2::VIDEO_ETS_TAG, fullScreenNodeId_.value());
    CHECK_NULL_VOID(fullScreenNode);
    auto parent = fullScreenNode->GetParent();
    CHECK_NULL_VOID(parent);
    parent->RemoveChild(fullScreenNode);
    parent->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void VideoPattern::RecoverState(const RefPtr<VideoPattern>& videoPattern)
{
    CHECK_NULL_VOID(videoPattern);
    currentPos_ = videoPattern->GetCurrentPos();
    if (mediaPlayer_->IsMediaPlayerValid() && mediaPlayer_->IsPlaying() != isPlaying_) {
        isPlaying_ = mediaPlayer_->IsPlaying();
        ChangePlayButtonTag();
    }
    isInitialState_ = videoPattern->GetInitialState();
    auto layoutProperty = videoPattern->GetLayoutProperty<VideoLayoutProperty>();
    src_ = layoutProperty->GetVideoSourceValue("");
    isStop_ = videoPattern->GetIsStop();
    muted_ = videoPattern->GetMuted();
    autoPlay_ = videoPattern->GetAutoPlay();
    loop_ = videoPattern->GetLoop();
    duration_ = videoPattern->GetDuration();
    progressRate_ = videoPattern->GetProgressRate();
    fullScreenNodeId_.reset();
    RegisterMediaPlayerEvent();
    auto videoNode = GetHost();
    CHECK_NULL_VOID(videoNode);
    // change event hub to the origin video node
    videoPattern->GetEventHub<VideoEventHub>()->AttachHost(videoNode);
    videoNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_CHILD);
}

void VideoPattern::UpdateFsState()
{
    if (!fullScreenNodeId_.has_value()) {
        return;
    }
    auto videoNode = FrameNode::GetFrameNode(V2::VIDEO_ETS_TAG, fullScreenNodeId_.value());
    CHECK_NULL_VOID(videoNode);
    auto videoPattern = AceType::DynamicCast<VideoFullScreenPattern>(videoNode->GetPattern());
    CHECK_NULL_VOID(videoPattern);
    // update full screen state
    videoPattern->UpdateState();
}

bool VideoPattern::IsFullScreen()
{
    return fullScreenNodeId_.has_value();
}

RefPtr<VideoPattern> VideoPattern::GetTargetVideoPattern()
{
    auto isFullScreen = IsFullScreen();
    auto patternIsFullScreen = AceType::InstanceOf<VideoFullScreenPattern>(this);
    if ((isFullScreen && patternIsFullScreen) || (!isFullScreen && !patternIsFullScreen)) {
        return AceType::Claim(this);
    }
    if (patternIsFullScreen) {
        // current is full screen,need to be released
        auto fullScreenPattern = AceType::DynamicCast<VideoFullScreenPattern>(this);
        return fullScreenPattern->GetVideoPattern();
    }
    // current node is origin video node, need to operate full screen node
    auto fullScreenNode = GetFullScreenNode();
    CHECK_NULL_RETURN(fullScreenNode, nullptr);
    return fullScreenNode->GetPattern<VideoPattern>();
}
} // namespace OHOS::Ace::NG
