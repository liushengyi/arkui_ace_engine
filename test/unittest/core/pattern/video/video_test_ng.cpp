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

#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock-actions.h"
#include "gtest/gtest.h"

#define private public
#define protected public
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/render/mock_media_player.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/render/mock_render_surface.h"

#include "base/geometry/ng/size_t.h"
#include "base/json/json_util.h"
#include "base/memory/ace_type.h"
#include "base/resource/internal_resource.h"
#include "core/components/common/layout/constants.h"
#include "core/components/video/video_theme.h"
#include "core/components/video/video_utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/drag_event.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/video/video_full_screen_node.h"
#include "core/components_ng/pattern/video/video_full_screen_pattern.h"
#include "core/components_ng/pattern/video/video_layout_algorithm.h"
#include "core/components_ng/pattern/video/video_layout_property.h"
#include "core/components_ng/pattern/video/video_model_ng.h"
#include "core/components_ng/pattern/video/video_node.h"
#include "core/components_ng/pattern/video/video_pattern.h"
#include "core/components_ng/pattern/video/video_styles.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/image/image_source_info.h"
#include "test/mock/core/pipeline/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
struct TestProperty {
    std::optional<std::string> src;
    std::optional<double> progressRate;
    std::optional<std::string> posterUrl;
    std::optional<bool> muted;
    std::optional<bool> autoPlay;
    std::optional<bool> controls;
    std::optional<bool> loop;
    std::optional<ImageFit> objectFit;
    std::optional<RefPtr<VideoControllerV2>> videoController;
};
namespace {
constexpr double VIDEO_PROGRESS_RATE = 1.0;
constexpr bool MUTED_VALUE = false;
constexpr bool AUTO_PLAY = false;
constexpr bool CONTROL_VALUE = true;
constexpr bool LOOP_VALUE = false;
const ImageFit VIDEO_IMAGE_FIT = ImageFit::COVER;
const std::string VIDEO_SRC = "common/video.mp4";
const std::string VIDEO_POSTER_URL = "common/img2.png";
const std::string VIDEO_START_EVENT = "start";
const std::string VIDEO_PAUSE_EVENT = "pause";
const std::string VIDEO_FINISH_EVENT = "finish";
const std::string VIDEO_ERROR_EVENT = "error";
const std::string VIDEO_PREPARED_EVENT = "prepared";
const std::string VIDEO_SEEKING_EVENT = "seeking";
const std::string VIDEO_SEEKED_EVENT = "seeked";
const std::string VIDEO_UPDATE_EVENT = "update";
const std::string VIDEO_FULLSCREEN_EVENT = "fullScreen";
const std::string EXTRA_INFO_KEY = "extraInfo";
const std::string VIDEO_ERROR_ID = "";
const std::string VIDEO_CALLBACK_RESULT = "result_ok";
constexpr float MAX_WIDTH = 400.0f;
constexpr float MAX_HEIGHT = 400.0f;
constexpr float VIDEO_WIDTH = 300.0f;
constexpr float VIDEO_HEIGHT = 300.0f;
constexpr float SCREEN_WIDTH_SMALL = 500.0f;
constexpr float SCREEN_HEIGHT_SMALL = 1000.0f;
constexpr float SCREEN_WIDTH_MEDIUM = 1000.0f;
constexpr float SCREEN_HEIGHT_MEDIUM = 2000.0f;
constexpr float SCREEN_WIDTH_LARGE = 1500.0f;
constexpr float SCREEN_HEIGHT_LARGE = 2500.0f;
const SizeF MAX_SIZE(MAX_WIDTH, MAX_HEIGHT);
const SizeF SCREEN_SIZE_SMALL(SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL);
const SizeF SCREEN_SIZE_MEDIUM(SCREEN_WIDTH_MEDIUM, SCREEN_HEIGHT_MEDIUM);
const SizeF SCREEN_SIZE_LARGE(SCREEN_WIDTH_LARGE, SCREEN_HEIGHT_LARGE);
const SizeF VIDEO_SIZE(VIDEO_WIDTH, VIDEO_HEIGHT);
const SizeF LAYOUT_SIZE_RATIO_GREATER_THAN_1(MAX_WIDTH, VIDEO_HEIGHT);
const SizeF LAYOUT_SIZE_RATIO_LESS_THAN_1(VIDEO_WIDTH, MAX_HEIGHT);
const SizeF INVALID_SIZE(MAX_WIDTH, 0.0f);
constexpr uint32_t VIDEO_CHILDREN_NUM = 3;
constexpr uint32_t DURATION = 100;
constexpr uint32_t CURRENT_TIME = 100;
constexpr int32_t SLIDER_INDEX = 2;
constexpr int32_t VIDEO_NODE_ID_1 = 1;
constexpr int32_t VIDEO_NODE_ID_2 = 2;
TestProperty testProperty;
} // namespace

class VideoTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown() {}

protected:
    static RefPtr<FrameNode> CreateVideoNode(TestProperty& testProperty);
};

void VideoTestNg::SetUpTestSuite()
{
    testProperty.progressRate = VIDEO_PROGRESS_RATE;
    testProperty.muted = MUTED_VALUE;
    testProperty.autoPlay = AUTO_PLAY;
    testProperty.controls = CONTROL_VALUE;
    testProperty.loop = LOOP_VALUE;
    testProperty.objectFit = VIDEO_IMAGE_FIT;
    MockPipelineBase::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    MockPipelineBase::GetCurrent()->rootNode_ = FrameNode::CreateFrameNodeWithTree(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<RootPattern>());
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<VideoTheme>()));
}

void VideoTestNg::TearDownTestSuite()
{
    MockPipelineBase::TearDown();
}
void VideoTestNg::SetUp()
{
    ViewStackProcessor::GetInstance()->ClearStack();
}

RefPtr<FrameNode> VideoTestNg::CreateVideoNode(TestProperty& testProperty)
{
    if (testProperty.videoController.has_value()) {
        VideoModelNG().Create(testProperty.videoController.value());
    } else {
        auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
        VideoModelNG().Create(videoController);
    }
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto videoPattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    CHECK_NULL_RETURN(videoPattern, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));

    if (testProperty.src.has_value()) {
        VideoModelNG().SetSrc(testProperty.src.value());
    }
    if (testProperty.progressRate.has_value()) {
        VideoModelNG().SetProgressRate(testProperty.progressRate.value());
    }
    if (testProperty.posterUrl.has_value()) {
        VideoModelNG().SetPosterSourceInfo(testProperty.posterUrl.value());
    }
    if (testProperty.muted.has_value()) {
        VideoModelNG().SetMuted(testProperty.muted.value());
    }
    if (testProperty.autoPlay.has_value()) {
        VideoModelNG().SetAutoPlay(testProperty.autoPlay.value());
    }
    if (testProperty.controls.has_value()) {
        VideoModelNG().SetControls(testProperty.controls.value());
    }
    if (testProperty.loop.has_value()) {
        VideoModelNG().SetLoop(testProperty.loop.value());
    }
    if (testProperty.objectFit.has_value()) {
        VideoModelNG().SetObjectFit(testProperty.objectFit.value());
    }

    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    return AceType::DynamicCast<FrameNode>(element);
}

/**
 * @tc.name: VideoPropertyTest001
 * @tc.desc: Create Video.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPropertyTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    EXPECT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
}

/**
 * @tc.name: VideoPropertyTest002
 * @tc.desc: Create Vdeo, and set its properties.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPropertyTest002, TestSize.Level1)
{
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    video.Create(videoController);

    auto frameNodeTemp = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNodeTemp);
    auto videoPatternTemp = AceType::DynamicCast<VideoPattern>(frameNodeTemp->GetPattern());
    CHECK_NULL_VOID(videoPatternTemp);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPatternTemp->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    video.SetSrc(VIDEO_SRC);
    video.SetProgressRate(VIDEO_PROGRESS_RATE);
    video.SetPosterSourceInfo(VIDEO_POSTER_URL);
    video.SetMuted(MUTED_VALUE);
    video.SetAutoPlay(AUTO_PLAY);
    video.SetControls(CONTROL_VALUE);
    video.SetLoop(LOOP_VALUE);
    video.SetObjectFit(VIDEO_IMAGE_FIT);

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    EXPECT_TRUE(frameNode != nullptr && frameNode->GetTag() == V2::VIDEO_ETS_TAG);
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_NE(videoLayoutProperty, nullptr);
    auto videoPattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    EXPECT_EQ(videoLayoutProperty->GetVideoSource().value_or(""), VIDEO_SRC);
    EXPECT_EQ(videoPattern->GetProgressRate(), VIDEO_PROGRESS_RATE);
    EXPECT_EQ(videoLayoutProperty->GetPosterImageInfoValue(ImageSourceInfo("")), ImageSourceInfo(VIDEO_POSTER_URL));
    EXPECT_EQ(videoPattern->GetMuted(), MUTED_VALUE);
    EXPECT_EQ(videoPattern->GetAutoPlay(), AUTO_PLAY);
    EXPECT_EQ(videoLayoutProperty->GetControlsValue(true), CONTROL_VALUE);
    EXPECT_EQ(videoPattern->GetLoop(), LOOP_VALUE);
    EXPECT_EQ(videoLayoutProperty->GetObjectFitValue(ImageFit::COVER), VIDEO_IMAGE_FIT);
}

/**
 * @tc.name: VideoEventTest003
 * @tc.desc: Create Video, and set its callback functions.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoEventTest003, TestSize.Level1)
{
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    video.Create(videoController);

    auto frameNodeTemp = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNodeTemp);
    auto videoPatternTemp = AceType::DynamicCast<VideoPattern>(frameNodeTemp->GetPattern());
    CHECK_NULL_VOID(videoPatternTemp);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPatternTemp->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    std::string unknownVideoEvent;
    auto videoEvent = [&unknownVideoEvent](const std::string& videoEvent) { unknownVideoEvent = videoEvent; };

    video.SetOnStart(videoEvent);
    video.SetOnPause(videoEvent);
    video.SetOnFinish(videoEvent);
    video.SetOnError(videoEvent);
    video.SetOnPrepared(videoEvent);
    video.SetOnSeeking(videoEvent);
    video.SetOnSeeked(videoEvent);
    video.SetOnUpdate(videoEvent);
    video.SetOnFullScreenChange(videoEvent);

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    EXPECT_TRUE(frameNode != nullptr && frameNode->GetTag() == V2::VIDEO_ETS_TAG);
    auto videoEventHub = frameNode->GetEventHub<VideoEventHub>();
    EXPECT_TRUE(videoEventHub != nullptr);

    videoEventHub->FireStartEvent(VIDEO_START_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_START_EVENT);
    videoEventHub->FirePauseEvent(VIDEO_PAUSE_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_PAUSE_EVENT);
    videoEventHub->FireFinishEvent(VIDEO_FINISH_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_FINISH_EVENT);
    videoEventHub->FireErrorEvent(VIDEO_ERROR_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_ERROR_EVENT);
    videoEventHub->FirePreparedEvent(VIDEO_PREPARED_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_PREPARED_EVENT);
    videoEventHub->FireSeekingEvent(VIDEO_SEEKING_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_SEEKING_EVENT);
    videoEventHub->FireSeekedEvent(VIDEO_SEEKED_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_SEEKED_EVENT);
    videoEventHub->FireUpdateEvent(VIDEO_UPDATE_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_UPDATE_EVENT);
    videoEventHub->FireFullScreenChangeEvent(VIDEO_FULLSCREEN_EVENT);
    EXPECT_EQ(unknownVideoEvent, VIDEO_FULLSCREEN_EVENT);
}

/**
 * @tc.name: VideoMeasureContentTest004
 * @tc.desc: Create Video, and invoke its MeasureContent function to calculate the content size
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoMeasureContentTest004, TestSize.Level1)
{
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    video.Create(videoController);

    auto frameNodeTemp = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNodeTemp, nullptr);
    auto videoPatternTemp = AceType::DynamicCast<VideoPattern>(frameNodeTemp->GetPattern());
    ASSERT_NE(videoPatternTemp, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPatternTemp->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    EXPECT_NE(videoLayoutProperty, nullptr);

    // Create LayoutWrapper and set videoLayoutAlgorithm.
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    LayoutWrapperNode layoutWrapper = LayoutWrapperNode(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto videoPattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);
    auto videoLayoutAlgorithm = videoPattern->CreateLayoutAlgorithm();
    EXPECT_NE(videoLayoutAlgorithm, nullptr);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(videoLayoutAlgorithm));

    // Test MeasureContent.
    /**
    //     corresponding ets code:
    //         Video({ previewUri: this.previewUri, controller: this.controller })
    */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = MAX_SIZE;
    auto videoDefaultSize =
        videoLayoutAlgorithm->MeasureContent(layoutConstraint, &layoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(videoDefaultSize, MAX_SIZE);

    /**
    //     corresponding ets code:
    //         Video({ src: this.videoSrc, previewUri: this.previewUri, controller: this.controller })
    //             .height(400).width(400)
    */
    // Set layout size.
    layoutConstraint.selfIdealSize.SetSize(VIDEO_SIZE);
    auto videoSize1 =
        videoLayoutAlgorithm->MeasureContent(layoutConstraint, &layoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(videoSize1, VIDEO_SIZE);
}

/**
 * @tc.name: VideoMeasureTest005
 * @tc.desc: Create Video, and invoke its Measure and layout function, and test its child/children layout algorithm.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoMeasureTest005, TestSize.Level1)
{
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    video.Create(videoController);

    auto frameNodeTemp = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNodeTemp);
    auto videoPatternTemp = AceType::DynamicCast<VideoPattern>(frameNodeTemp->GetPattern());
    CHECK_NULL_VOID(videoPatternTemp);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPatternTemp->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    // when video set preview image and control, it will contains two children which are image and row respectively.
    video.SetPosterSourceInfo(VIDEO_POSTER_URL);
    video.SetControls(CONTROL_VALUE);

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    EXPECT_TRUE(frameNode != nullptr && frameNode->GetTag() == V2::VIDEO_ETS_TAG);
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    EXPECT_FALSE(videoLayoutProperty == nullptr);

    // Create LayoutWrapper and set videoLayoutAlgorithm.
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    LayoutWrapperNode layoutWrapper = LayoutWrapperNode(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto videoPattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);
    auto videoLayoutAlgorithm = videoPattern->CreateLayoutAlgorithm();
    ASSERT_NE(videoLayoutAlgorithm, nullptr);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(videoLayoutAlgorithm));

    // Set video source size and layout size.
    LayoutConstraintF layoutConstraint;
    videoLayoutProperty->UpdateVideoSize(VIDEO_SIZE);
    layoutConstraint.selfIdealSize.SetSize(VIDEO_SIZE);
    auto videoSize1 =
        videoLayoutAlgorithm->MeasureContent(layoutConstraint, &layoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(videoSize1, SizeF(VIDEO_WIDTH, VIDEO_WIDTH));
    layoutWrapper.GetGeometryNode()->SetContentSize(videoSize1);

    const auto& children = frameNode->GetChildren();
    for (const auto& child : children) {
        auto frameNodeChild = AceType::DynamicCast<FrameNode>(child);
        RefPtr<GeometryNode> geometryNodeChild = AceType::MakeRefPtr<GeometryNode>();
        auto childLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapperNode>(AceType::WeakClaim(AceType::RawPtr(frameNodeChild)),
                geometryNodeChild, frameNodeChild->GetLayoutProperty());
        layoutWrapper.AppendChild(childLayoutWrapper);
    }

    videoLayoutAlgorithm->Measure(&layoutWrapper);
    videoLayoutAlgorithm->Layout(&layoutWrapper);

    auto layoutWrapperChildren = layoutWrapper.GetAllChildrenWithBuild();
    EXPECT_EQ(layoutWrapperChildren.size(), VIDEO_CHILDREN_NUM);
    for (auto&& child : layoutWrapperChildren) {
        if (child->GetHostTag() == V2::IMAGE_ETS_TAG) {
            EXPECT_EQ(child->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0, 0.0));
        } else if (child->GetHostTag() == V2::ROW_ETS_TAG) {
            // controlBarHeight is 40 defined in theme, but pipeline context is null in UT which cannot get the value
            // when invoking video Measure function. So we assume the value is 0.0 here.
            float const controlBarHeight = 0.0;
            EXPECT_EQ(child->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0, VIDEO_WIDTH - controlBarHeight));
        }
    }
}

/**
 * @tc.name: VideoPatternTest006
 * @tc.desc: Test AddPreviewNode
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest006, TestSize.Level1)
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<VideoTheme>()));

    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    /**
     * @tc.steps: step2. check the children size.
     */
    auto children = frameNode->GetChildren();
    auto childrenSize = static_cast<int32_t>(children.size());
    EXPECT_EQ(childrenSize, VIDEO_CHILDREN_NUM);
    auto image = frameNode->GetChildAtIndex(1);
    EXPECT_EQ(image->GetTag(), V2::IMAGE_ETS_TAG);
}

/**
 * @tc.name: VideoPatternTest007
 * @tc.desc: Test AddControlBarNode
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    /**
     * @tc.steps: step2. Add a child, in order to go to some branches
     */
    auto children = frameNode->GetChildren();
    auto childrenSize = static_cast<int32_t>(children.size());
    EXPECT_EQ(childrenSize, VIDEO_CHILDREN_NUM);
    auto row = frameNode->GetChildAtIndex(2);
    EXPECT_EQ(row->GetTag(), V2::ROW_ETS_TAG);
}

/**
 * @tc.name: VideoPatternTest008
 * @tc.desc: Test UpdateMediaPlayerOnBg
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    /**
     * @tc.steps: step2. Call UpdateMediaPlayerOnBg
     *            case: IsMediaPlayerValid is always false
     * @tc.expected: step2. IsMediaPlayerValid will be called 3 times
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(3)
        .WillRepeatedly(Return(false));
    pattern->UpdateMediaPlayerOnBg();

    /**
     * @tc.steps: step3. Call UpdateMediaPlayerOnBg
     *            case: IsMediaPlayerValid is always true & has not set VideoSource
     * @tc.expected: step3. IsMediaPlayerValid will be called 3 times.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(3)
        .WillRepeatedly(Return(true));
    pattern->UpdateMediaPlayerOnBg();

    /**
     * @tc.steps: step4. Call UpdateMediaPlayerOnBg
     *            case: IsMediaPlayerValid is always true & has set VideoSource
     * @tc.expected: step4. IsMediaPlayerValid will be called 5 times
     */
    auto videoLayoutProperty = pattern->GetLayoutProperty<VideoLayoutProperty>();
    videoLayoutProperty->UpdateVideoSource(VIDEO_SRC);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(5)
        .WillRepeatedly(Return(true));

    pattern->UpdateMediaPlayerOnBg();

    /**
     * @tc.steps: step5. Call UpdateMediaPlayerOnBg
     *            case: IsMediaPlayerValid is always true & has set VideoSource & has set src_
     * @tc.expected: step5. IsMediaPlayerValid will be called 3 times.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(3)
        .WillRepeatedly(Return(true));
    pattern->UpdateMediaPlayerOnBg();

    /**
     * @tc.steps: step6. Call UpdateMediaPlayerOnBg
     *            case: first prepare and UpdateMediaPlayerOnBg successfully
     * @tc.expected: step6. IsMediaPlayerValid will be called 5 times
     *                      other function will be called once and return right value when preparing MediaPlayer
     *                      firstly
     */
    pattern->src_.clear();
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(5)
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    pattern->UpdateMediaPlayerOnBg();

    /**
     * @tc.steps: step7. Call UpdateMediaPlayerOnBg several times
     *            cases: first prepare and UpdateMediaPlayerOnBg fail
     * @tc.expected: step7. IsMediaPlayerValid will be called 5 + 5 + 5 times totally.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(15)
        // 1st time.
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))

        // 2nd time.
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))

        // 3rd time.
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    pattern->src_.clear();
    pattern->UpdateMediaPlayerOnBg();
    pattern->src_.clear();
    pattern->UpdateMediaPlayerOnBg();
    pattern->src_.clear();
    pattern->UpdateMediaPlayerOnBg();

    // CreateMediaPlayer success but PrepareMediaPlayer fail for mediaPlayer is invalid
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(5)
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(false));
    pattern->src_.clear();
    pattern->UpdateMediaPlayerOnBg();
}

/**
 * @tc.name: VideoPatternTest009
 * @tc.desc: Test functions in UpdateMediaPlayerOnBg
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    /**
     * @tc.steps: step2. Call UpdateSpeed
     *            cases: MediaPlayer is valid and MediaPlayer is invalid
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(2)
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    pattern->UpdateSpeed();
    pattern->UpdateSpeed();

    /**
     * @tc.steps: step3. Call UpdateLooping
     *            cases: MediaPlayer is valid and MediaPlayer is invalid
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(2)
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    pattern->UpdateLooping();
    pattern->UpdateLooping();

    /**
     * @tc.steps: step4. Call UpdateMuted
     *            cases: MediaPlayer is valid (with muted and not muted) and MediaPlayer is invalid
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(3)
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    pattern->UpdateMuted();
    pattern->muted_ = false;
    pattern->UpdateMuted();
    pattern->muted_ = true;
    pattern->UpdateMuted();
}

/**
 * @tc.name: VideoPatternTest010
 * @tc.desc: Test OnPlayerStatus
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest010, TestSize.Level1)
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<VideoTheme>()));
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    /**
     * @tc.steps: step2. Prepare the childNode & videoEvent
     */
    auto controlBar = frameNode->GetChildAtIndex(2);
    ASSERT_TRUE(controlBar);

    auto playBtn = AceType::DynamicCast<FrameNode>(controlBar->GetChildAtIndex(0));
    ASSERT_TRUE(playBtn);
    auto playBtnGestureEventHub = playBtn->GetOrCreateGestureEventHub();
    ASSERT_TRUE(playBtnGestureEventHub);

    // set videoEvent
    auto videoEventHub = frameNode->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(videoEventHub);
    std::string startCheck;
    EventCallback onStart = [&startCheck](const std::string& /* param */) { startCheck = VIDEO_START_EVENT; };
    std::string pauseCheck;
    EventCallback onPause = [&pauseCheck](const std::string& /* param */) { pauseCheck = VIDEO_PAUSE_EVENT; };
    std::string finishCheck;
    EventCallback onFinish = [&finishCheck](const std::string& /* param */) { finishCheck = VIDEO_FINISH_EVENT; };
    videoEventHub->SetOnStart(std::move(onStart));
    videoEventHub->SetOnPause(std::move(onPause));
    videoEventHub->SetOnFinish(std::move(onFinish));

    /**
     * @tc.steps: step3. Call OnPlayerStatus status == STARTED
     * @tc.expected: step3. FireStartEvent has called and playBtn event will call pattern->Pause()
     */
    pattern->OnPlayerStatus(PlaybackStatus::STARTED);
    EXPECT_EQ(startCheck, VIDEO_START_EVENT);
    // will call pattern->Pause()
    EXPECT_TRUE(pattern->isPlaying_);
    // case1: MediaPlayer is invalid
    auto flag = playBtnGestureEventHub->ActClick();
    EXPECT_TRUE(flag);
    // case2: MediaPlayer is valid & isPlaying = true
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), Pause()).Times(2).WillOnce(Return(0));
    flag = playBtnGestureEventHub->ActClick();
    EXPECT_TRUE(flag);
    // case3: MediaPlayer is valid & isPlaying = false
    pattern->isPlaying_ = false;
    flag = playBtnGestureEventHub->ActClick();
    EXPECT_TRUE(flag);

    /**
     * @tc.steps: step4. Call OnPlayerStatus status == PREPARED
     * @tc.expected: step4. FirePauseEvent & mediaPlayer->GetDuration() has called
     */
    // case1: MediaPlayer is invalid
    pattern->OnPlayerStatus(PlaybackStatus::PAUSED);
    EXPECT_EQ(pauseCheck, VIDEO_PAUSE_EVENT);

    // case1: MediaPlayer is valid
    pauseCheck.clear();
    pattern->OnPlayerStatus(PlaybackStatus::PAUSED);
    EXPECT_EQ(pauseCheck, VIDEO_PAUSE_EVENT);

    /**
     * @tc.steps: step5. Call OnPlayerStatus status == PLAYBACK_COMPLETE
     * @tc.expected: step5. FireFinishEvent & OnUpdateTime(pos = CURRENT_POS) will be called
     */
    pattern->OnPlayerStatus(PlaybackStatus::PLAYBACK_COMPLETE); // case1: controls = true
    EXPECT_EQ(finishCheck, VIDEO_FINISH_EVENT);
    auto videoLayoutProperty = pattern->GetLayoutProperty<VideoLayoutProperty>();
    videoLayoutProperty->UpdateControls(false);
    pattern->OnPlayerStatus(PlaybackStatus::PLAYBACK_COMPLETE); // case2: controls = false
    EXPECT_EQ(finishCheck, VIDEO_FINISH_EVENT);
    pattern->OnPlayerStatus(PlaybackStatus::ERROR);
    pattern->OnPlayerStatus(PlaybackStatus::IDLE);
    pattern->OnPlayerStatus(PlaybackStatus::PREPARED);
    pattern->OnPlayerStatus(PlaybackStatus::PAUSED);
    pattern->OnPlayerStatus(PlaybackStatus::STOPPED);
    pattern->OnPlayerStatus(PlaybackStatus::NONE);
}

/**
 * @tc.name: VideoPatternTest011
 * @tc.desc: Test OnPrepared
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    // set videoEvent
    auto videoEventHub = frameNode->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(videoEventHub);
    std::string preparedCheck;
    EventCallback onPrepared = [&preparedCheck](
                                   const std::string& /* param */) { preparedCheck = VIDEO_PREPARED_EVENT; };
    videoEventHub->SetOnPrepared(std::move(onPrepared));
    auto videoLayoutProperty = pattern->GetLayoutProperty<VideoLayoutProperty>();

    /**
     * @tc.steps: step2. Call OnPrepared
     *            case1: needControlBar & needFireEvent = true, isStop_ & autoPlay_ = false
     * @tc.expected: step2. FirePreparedEvent will be called & duration_ has changed
     */
    EXPECT_TRUE(videoLayoutProperty->GetControlsValue(true));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(3)
        .WillRepeatedly(Return(true));
    pattern->OnPrepared(VIDEO_WIDTH, VIDEO_HEIGHT, DURATION, 0, true);
    EXPECT_EQ(pattern->duration_, DURATION);
    EXPECT_EQ(preparedCheck, VIDEO_PREPARED_EVENT);

    /**
     * @tc.steps: step3. Call OnPrepared
     *            case2: needControlBar & needFireEvent = false, isStop_ & autoPlay_ = true
     * @tc.expected: step3. FirePreparedEvent will not be called & duration_ has changed
     */
    preparedCheck.clear();
    pattern->duration_ = 0;
    pattern->isStop_ = true;
    pattern->autoPlay_ = true;
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .Times(10)
        .WillRepeatedly(Return(true));
    pattern->OnPrepared(VIDEO_WIDTH, VIDEO_HEIGHT, DURATION, 0, false);
    EXPECT_EQ(pattern->duration_, DURATION);
    EXPECT_TRUE(preparedCheck.empty());
    pattern->isStop_ = false;
    pattern->dragEndAutoPlay_ = true;
    pattern->OnPrepared(VIDEO_WIDTH, VIDEO_HEIGHT, DURATION, 0, false);
    EXPECT_EQ(pattern->duration_, DURATION);
    EXPECT_TRUE(preparedCheck.empty());
    EXPECT_FALSE(pattern->dragEndAutoPlay_);
}

/**
 * @tc.name: VideoPatternTest012
 * @tc.desc: Test Start & Stop
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    auto imageFrameNode = frameNode->GetChildAtIndex(0);
    auto rawChildNum = static_cast<int32_t>(frameNode->GetChildren().size());

    // set video event
    auto videoEventHub = pattern->GetEventHub<VideoEventHub>();
    std::string pauseCheck;
    EventCallback onPause = [&pauseCheck](const std::string& /* param */) { pauseCheck = VIDEO_PAUSE_EVENT; };
    videoEventHub->SetOnPause(std::move(onPause));
    std::string updateCheck;
    EventCallback onUpdate = [&updateCheck](const std::string& /* param */) { updateCheck = VIDEO_UPDATE_EVENT; };
    videoEventHub->SetOnUpdate(std::move(onUpdate));

    /**
     * @tc.steps: step2. Call Start
     * @tc.expected: step2. relevant functions called correctly
     */
    bool isStops[2] { true, false };
    int32_t prepareReturns[2] { -1, 0 };
    for (bool isStop : isStops) {
        for (int prepareReturn : prepareReturns) {
            pattern->isStop_ = isStop;
            if (isStop) {
                EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), PrepareAsync())
                    .WillOnce(Return(prepareReturn));
            }
            if (isStop && prepareReturn != 0) {
                pattern->Start();
            } else if (isStop && prepareReturn == 0) {
                pattern->isPlaying_ = true;
                pattern->Start();
                auto childNum = static_cast<int32_t>(frameNode->GetChildren().size());
                EXPECT_EQ(childNum, rawChildNum);
            }
        }
    }

    /**
     * @tc.steps: step3. Call Stop when the mediaplayer is not valid.
     * @tc.expected: step3. relevant functions called correctly
     */
    pattern->currentPos_ = 10;
    pattern->isStop_ = false;
    pattern->Stop();
    EXPECT_EQ(static_cast<int32_t>(pattern->currentPos_), 0);
    EXPECT_TRUE(pattern->isStop_);

    /**
     * @tc.steps: step3. Call Stop when the mediaplayer is valid.
     * @tc.expected: step3. relevant functions called correctly
     */
    pattern->currentPos_ = 10;
    pattern->isStop_ = false;
    pattern->duration_ = 20;
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), Stop()).WillOnce(Return(0));
    pattern->Stop();
    EXPECT_EQ(static_cast<int32_t>(pattern->currentPos_), 0);
    EXPECT_EQ(pattern->isStop_, true);

    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), Stop()).WillOnce(Return(0));
    EXPECT_EQ(static_cast<int32_t>(pattern->currentPos_), 0);
    pattern->Stop(); // case2: media player is valid & currentPos = currentPos_ = 0
    EXPECT_EQ(pattern->isStop_, true);

    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), Stop())
        .Times(2)
        .WillRepeatedly(Return(0));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), GetDuration(_)).Times(0);
    updateCheck.clear();
    pattern->currentPos_ = 1;
    pattern->Stop(); // case3: media player is valid & currentPos != currentPos_ & duration_ = 0 &
                     // mediaPlayer_->GetDuration return ok
                     // this will call OnUpdateTime(pos=DURATION_POS)
    EXPECT_EQ(static_cast<int32_t>(pattern->currentPos_), 1);
    EXPECT_EQ(updateCheck, "");
    EXPECT_EQ(pattern->isStop_, true);
    updateCheck.clear();
    pattern->currentPos_ = 1;
    pattern->Stop(); // case4: media player is valid & currentPos != currentPos_ & duration_ = 0 &
                     // mediaPlayer_->GetDuration return err
    EXPECT_EQ(static_cast<int32_t>(pattern->currentPos_), 1);
    EXPECT_EQ(updateCheck, "");
    EXPECT_EQ(pattern->isStop_, true);
}

/**
 * @tc.name: VideoPatternTest013
 * @tc.desc: Test function related to full screen and slider
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    testProperty.videoController = videoController;
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    // Add a redundant node to go other branch
    auto tempFrameNode = AceType::MakeRefPtr<FrameNode>("TEMP", -1, AceType::MakeRefPtr<Pattern>());
    frameNode->AddChild(tempFrameNode, 0);

    // set video event
    auto videoEventHub = frameNode->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(videoEventHub);
    std::string seekingCheck;
    EventCallback onSeeking = [&seekingCheck](const std::string& /* param */) { seekingCheck = VIDEO_SEEKING_EVENT; };
    videoEventHub->SetOnSeeking(std::move(onSeeking));
    std::string seekedCheck;
    EventCallback onSeeked = [&seekedCheck](const std::string& /* param */) { seekedCheck = VIDEO_SEEKED_EVENT; };
    videoEventHub->SetOnSeeked(std::move(onSeeked));

    /**
     * @tc.steps: step2. call OnSliderChange
     * @tc.expected: step2. onSeeking/onSeeked & SetCurrentTime  will be called
     */
    std::vector<int32_t> sliderChangeModes { 0, 1, 2 };
    for (int i = 0; i < 3; i++) {
        auto sliderChangeMode = sliderChangeModes[i];
        if (i == 1) {
            EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), Seek(_, _))
                .Times(2)
                .WillOnce(Return(0)); // 0 <= currentPos(0) <= duration_(0) will call mediaPlayer's Seek()
        }
        if (i < 2) {
            seekingCheck.clear();
            pattern->OnSliderChange(0, sliderChangeMode);
            EXPECT_EQ(seekingCheck, VIDEO_SEEKING_EVENT);
        } else {
            seekedCheck.clear();
            pattern->OnSliderChange(-1, sliderChangeMode); // currentPos(-1) < 0
            EXPECT_EQ(seekedCheck, VIDEO_SEEKED_EVENT);

            seekedCheck.clear();
            pattern->OnSliderChange(1, sliderChangeMode); // currentPos(1) > duration_(0)
            EXPECT_EQ(seekedCheck, VIDEO_SEEKED_EVENT);
        }
    }
}

/**
 * @tc.name: VideoPatternTest014
 * @tc.desc: Test OnResolutionChange
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    /**
     * @tc.steps: step2. Call OnResolutionChange
     * @tc.expected: step2. related functions will be called
     */
    pattern->OnResolutionChange();

    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    EXPECT_TRUE(videoLayoutProperty->HasVideoSize());
    EXPECT_EQ(videoLayoutProperty->GetVideoSizeValue(SizeF(0, 0)).Width(), 100);
    EXPECT_EQ(videoLayoutProperty->GetVideoSizeValue(SizeF(0, 0)).Height(), 100);
}

/**
 * @tc.name: VideoFullScreenTest015
 * @tc.desc: Create Video, and invoke its MeasureContent function to calculate the content size when it is fullscreen.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoFullScreenTest015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    MockPipelineBase::GetCurrent()->SetRootSize(SCREEN_WIDTH_MEDIUM, SCREEN_HEIGHT_MEDIUM);
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    video.Create(videoController);

    auto frameNodeTemp = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNodeTemp);
    auto videoPatternTemp = AceType::DynamicCast<VideoPattern>(frameNodeTemp->GetPattern());
    CHECK_NULL_VOID(videoPatternTemp);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPatternTemp->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    EXPECT_TRUE(frameNode != nullptr && frameNode->GetTag() == V2::VIDEO_ETS_TAG);
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    EXPECT_FALSE(videoLayoutProperty == nullptr);

    // Create LayoutWrapper and set videoLayoutAlgorithm.
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    LayoutWrapperNode layoutWrapper = LayoutWrapperNode(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto videoPattern = frameNode->GetPattern<VideoPattern>();
    EXPECT_FALSE(videoPattern == nullptr);
    auto videoLayoutAlgorithm = videoPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(videoLayoutAlgorithm == nullptr);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(videoLayoutAlgorithm));
    videoPattern->FullScreen(); // will called onFullScreenChange(true)
    EXPECT_TRUE(videoPattern->GetFullScreenNode() != nullptr);

    // Test MeasureContent when it is fullScreen.
    /**
        corresponding ets code:
            Video({ src: this.videoSrc, previewUri: this.previewUri, controller: this.controller })
                .height(400).width(400)
    */

    // Set layout size.
    LayoutConstraintF layoutConstraint;
    videoLayoutProperty->UpdateVideoSize(VIDEO_SIZE);
    layoutConstraint.selfIdealSize.SetSize(VIDEO_SIZE);

    /**
     * @tc.steps: step2. change to full screen, check size.
     * @tc.expected: step2. Video size is same to rootsize.
     */
    auto fullScreenNode = videoPattern->GetFullScreenNode();
    auto fullScreenPattern = fullScreenNode->GetPattern();
    EXPECT_FALSE(fullScreenPattern == nullptr);
    auto fullScreenLayout = fullScreenPattern->CreateLayoutAlgorithm();
    auto fullScreenGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode fullScreenLayoutWrapper =
        LayoutWrapperNode(fullScreenNode, fullScreenGeometryNode, fullScreenNode->GetLayoutProperty());
    fullScreenLayoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(fullScreenLayout));
    auto videoSize =
        fullScreenLayout->MeasureContent(layoutConstraint, &fullScreenLayoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(videoSize, SCREEN_SIZE_MEDIUM);

    // Change the root size to small
    MockPipelineBase::GetCurrent()->SetRootSize(SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL);
    auto size =
        fullScreenLayout->MeasureContent(layoutConstraint, &fullScreenLayoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(size, SCREEN_SIZE_SMALL);

    // Change the root size to large
    MockPipelineBase::GetCurrent()->SetRootSize(SCREEN_WIDTH_LARGE, SCREEN_HEIGHT_LARGE);
    videoSize =
        fullScreenLayout->MeasureContent(layoutConstraint, &fullScreenLayoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(videoSize, SCREEN_SIZE_LARGE);

    /**
     * @tc.steps: step3. change from full screen to normal size, check size.
     * @tc.expected: step2. Video size is same to origional size.
     */
    auto videoFullScreenNode1 = videoPattern->GetFullScreenNode();
    EXPECT_FALSE(videoFullScreenNode1 == nullptr);
    auto fullScreenPattern1 = AceType::DynamicCast<VideoFullScreenPattern>(videoFullScreenNode1->GetPattern());
    EXPECT_FALSE(fullScreenPattern1 == nullptr);
    fullScreenPattern1->ExitFullScreen(); // will called onFullScreenChange(true)
    EXPECT_TRUE(videoPattern->GetFullScreenNode() == nullptr);

    videoSize = videoLayoutAlgorithm->MeasureContent(layoutConstraint, &layoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(videoSize, SizeF(VIDEO_WIDTH, VIDEO_HEIGHT));
}

/**
 * @tc.name: VideoPropertyTest016
 * @tc.desc: Create Video, and check the pixelmap.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPropertyTest016, TestSize.Level1)
{
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    video.Create(videoController);

    /**
     * @tc.steps: step1. Create a video.
     * @tc.expected: step1. Create successfully.
     */
    auto frameNodeTemp = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNodeTemp);
    auto videoPatternTemp = AceType::DynamicCast<VideoPattern>(frameNodeTemp->GetPattern());
    CHECK_NULL_VOID(videoPatternTemp);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPatternTemp->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    /**
     * @tc.steps: step3. Set the preview by pixelmap.
     * @tc.expected: step2. Set the pixelmap successfully.
     */
    void* voidPtr = static_cast<void*>(new char[0]);
    void* secondVoidPtr = static_cast<void*>(new char[0]);
    RefPtr<PixelMap> pixelMap = PixelMap::CreatePixelMap(voidPtr);
    RefPtr<PixelMap> secondPixelMap = PixelMap::CreatePixelMap(secondVoidPtr);

    // Default image and pixelmap image and url image.
    ImageSourceInfo defaultImage = ImageSourceInfo("");
    ImageSourceInfo pixelMapImage = ImageSourceInfo(pixelMap);
    ImageSourceInfo secondPixelMapImage = ImageSourceInfo(secondPixelMap);
    ImageSourceInfo urlImage = ImageSourceInfo(VIDEO_POSTER_URL);

    video.SetSrc(VIDEO_SRC);
    video.SetProgressRate(VIDEO_PROGRESS_RATE);
    video.SetPosterSourceByPixelMap(pixelMap);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    EXPECT_TRUE(frameNode != nullptr && frameNode->GetTag() == V2::VIDEO_ETS_TAG);
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    EXPECT_FALSE(videoLayoutProperty == nullptr);

    EXPECT_EQ(videoLayoutProperty->GetVideoSource().value_or(""), VIDEO_SRC);
    EXPECT_EQ(videoLayoutProperty->GetPosterImageInfoValue(defaultImage), pixelMapImage);

    /**
     * @tc.steps: step3. Reset preview by another pixelmap.
     * @tc.expected: step2. Reset the pixelmap successfully.
     */
    video.SetPosterSourceByPixelMap(secondPixelMap);
    EXPECT_EQ(videoLayoutProperty->GetPosterImageInfoValue(defaultImage), secondPixelMapImage);

    video.SetPosterSourceInfo(VIDEO_POSTER_URL);
    EXPECT_EQ(videoLayoutProperty->GetPosterImageInfoValue(defaultImage), urlImage);

    video.SetPosterSourceByPixelMap(pixelMap);
    EXPECT_EQ(videoLayoutProperty->GetPosterImageInfoValue(defaultImage), pixelMapImage);
}

/**
 * @tc.name: VideoPatternTest017
 * @tc.desc: Test VideoPattern OnDirtyLayoutWrapperSwap
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a video and get the videoPattern.
     * @tc.expected: step1. Create and get successfully.
     */
    VideoModelNG videoModelNG;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    videoModelNG.Create(videoController);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto videoPattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(videoPattern, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    /**
     * @tc.steps: step2. Set skipMeasure property of drawSwapConfig.
     * @tc.expected: step2. LayoutWrapper swap failed.
     */
    DirtySwapConfig config;
    config.skipMeasure = true;
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_NE(videoLayoutProperty, nullptr);
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, videoLayoutProperty);
    layoutWrapper->skipMeasureContent_ = true;
    EXPECT_FALSE(videoPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));
    config.skipMeasure = false;
    EXPECT_FALSE(videoPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));

    /**
     * @tc.steps: step3. Set skipMeasure property of layoutAlgorithm.
     * @tc.expected: step3. LayoutWrapper swap failed.
     */
    config.skipMeasure = true;
    layoutWrapper->skipMeasureContent_ = false;
    auto layoutAlgorithm = AceType::MakeRefPtr<LayoutAlgorithmWrapper>(AceType::MakeRefPtr<LayoutAlgorithm>());
    layoutWrapper->layoutAlgorithm_ = layoutAlgorithm;
    layoutWrapper->layoutAlgorithm_->skipMeasure_ = false;
    EXPECT_FALSE(videoPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));

    /**
     * @tc.steps: step4. Call function while videoSize is null.
     * @tc.expected: step4. LayoutWrapper swap failed.
     */
    config.skipMeasure = false;
    std::optional<SizeF> videoSize;
    std::unique_ptr<VideoStyle> tempPtr = std::make_unique<VideoStyle>();
    videoLayoutProperty->propVideoStyle_ = std::move(tempPtr);
    videoLayoutProperty->propVideoStyle_->propVideoSize = videoSize;
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL));
    auto mockRenderContext = AceType::MakeRefPtr<MockRenderContext>();
    videoPattern->renderContextForMediaPlayer_ = mockRenderContext;
    EXPECT_CALL(*AceType::DynamicCast<MockRenderContext>(videoPattern->renderContextForMediaPlayer_),
        SetBounds(0.0f, 0.0f, SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL))
        .WillOnce(Return());

    EXPECT_FALSE(videoPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, config));
}

/**
 * @tc.name: VideoPatternTest018
 * @tc.desc: Test VideoPattern OnAreaChangedInner
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a video and get the videoPattern.
     * @tc.expected: step1. Create and get successfully.
     */
    VideoModelNG videoModelNG;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    videoModelNG.Create(videoController);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto videoPattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Set lastBoundsRect property.
     * @tc.expected: step1. Property set successfully.
     */
    SystemProperties::SetExtSurfaceEnabled(false);
    videoPattern->OnAreaChangedInner();
    videoPattern->fullScreenNodeId_ = std::make_optional<int32_t>(1);
    videoPattern->OnAreaChangedInner();
    SystemProperties::SetExtSurfaceEnabled(true);
    videoPattern->fullScreenNodeId_ = std::make_optional<int32_t>();
    videoPattern->OnAreaChangedInner();
    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_NE(videoLayoutProperty, nullptr);
    auto geometryNode = frameNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    videoPattern->fullScreenNodeId_ = std::make_optional<int32_t>(1);
    videoLayoutProperty->UpdateObjectFit(ImageFit::CONTAIN);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL, 0.0f));
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH, 0.0f);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), 0.0f);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), 0.0f);

    /**
     * @tc.steps: step3. Call function with different objectFit status.
     * @tc.expected: step3. VideoPattern lastBoundsRect_'s width and height set correctly.
     */
    videoLayoutProperty->UpdateObjectFit(ImageFit::CONTAIN);
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH, VIDEO_HEIGHT);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), 0.0f);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL));
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH, 0.0f);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_HEIGHT_SMALL);
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH * 2, VIDEO_HEIGHT);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_WIDTH_SMALL / 2);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL * 6, SCREEN_HEIGHT_SMALL));
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_HEIGHT_SMALL * 2);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_HEIGHT_SMALL);

    videoLayoutProperty->UpdateObjectFit(ImageFit::FILL);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL));
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_HEIGHT_SMALL);

    videoLayoutProperty->UpdateObjectFit(ImageFit::COVER);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL, 0.0f));
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH, VIDEO_HEIGHT);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), 0.0f);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL));
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH, 0.0f);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_HEIGHT_SMALL);
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH * 2, VIDEO_HEIGHT);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_HEIGHT_SMALL * 2);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_HEIGHT_SMALL);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL * 6, SCREEN_HEIGHT_SMALL));
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL * 6);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_WIDTH_SMALL * 3);

    videoLayoutProperty->UpdateObjectFit(ImageFit::NONE);
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH, VIDEO_HEIGHT);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), VIDEO_WIDTH);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), VIDEO_HEIGHT);

    videoLayoutProperty->UpdateObjectFit(ImageFit::SCALE_DOWN);
    geometryNode->SetContentSize(SizeF(SCREEN_WIDTH_SMALL, SCREEN_HEIGHT_SMALL));
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), VIDEO_WIDTH);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), VIDEO_HEIGHT);
    videoLayoutProperty->propVideoStyle_->propVideoSize = SizeF(VIDEO_WIDTH * 2, VIDEO_HEIGHT);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_WIDTH_SMALL / 2);

    videoLayoutProperty->UpdateObjectFit(ImageFit::FITHEIGHT);
    videoPattern->OnAreaChangedInner();
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Width(), SCREEN_WIDTH_SMALL);
    EXPECT_FLOAT_EQ(videoPattern->lastBoundsRect_.Height(), SCREEN_WIDTH_SMALL / 2);
}

/**
 * @tc.name: VideoPatternTest019
 * @tc.desc: Test VideoPattern requestFullscreenImpl
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a video and get the videoPattern.
     * @tc.expected: step1. Create and get successfully.
     */
    VideoModelNG videoModelNG;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    videoModelNG.Create(videoController);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto videoPattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Set controlBar Property.
     * @tc.expected: step2. Property set successfully.
     */
    frameNode->children_.clear();
    videoPattern->isPlaying_ = false;
    auto controlBar = videoPattern->CreateControlBar();
    auto playButton = AceType::DynamicCast<FrameNode>(controlBar->GetFirstChild());
    auto playBtnEvent = playButton->GetOrCreateGestureEventHub();
    auto playClickCallback = playBtnEvent->clickEventActuator_->userCallback_->callback_;
    GestureEvent gestureEvent;
    playClickCallback(gestureEvent);

    auto pipelineContext = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipelineContext, nullptr);
    auto videoTheme = pipelineContext->GetTheme<VideoTheme>();
    ASSERT_NE(videoTheme, nullptr);
    EXPECT_EQ(controlBar->GetRenderContext()->GetBackgroundColorValue(), videoTheme->GetBkgColor());
    auto controlBarLayoutProperty = controlBar->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_EQ(controlBarLayoutProperty->GetMainAxisAlignValue(FlexAlign::AUTO), FlexAlign::SPACE_BETWEEN);
}

/**
 * @tc.name: VideoPatternTest020
 * @tc.desc: Test VideoPattern dragEnd
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a video and get the videoPattern.
     * @tc.expected: step1. Create and get successfully.
     */
    VideoModelNG videoModelNG;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    videoModelNG.Create(videoController);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto videoPattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(videoPattern, nullptr);

    videoPattern->src_ = "test";
    videoPattern->EnableDrag();
    auto eventHub = frameNode->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    auto dragEnd = eventHub->onDrop_;
    std::string extraParams;
    dragEnd(nullptr, extraParams);

    /**
     * @tc.steps: step2. Call dragEnd in different wrong situation.
     * @tc.expected: step2. Log error message.
     */
    auto json = JsonUtil::Create(true);
    json->Put(EXTRA_INFO_KEY.c_str(), "");
    dragEnd(nullptr, json->ToString());
    json->Delete(EXTRA_INFO_KEY.c_str());
    json->Put(EXTRA_INFO_KEY.c_str(), "test");
    dragEnd(nullptr, json->ToString());
    json->Delete(EXTRA_INFO_KEY.c_str());
    json->Put(EXTRA_INFO_KEY.c_str(), "test::");
    dragEnd(nullptr, json->ToString());
    json->Delete(EXTRA_INFO_KEY.c_str());
    json->Put(EXTRA_INFO_KEY.c_str(), "::test");
    dragEnd(nullptr, json->ToString());

    videoPattern->isInitialState_ = false;
    json->Delete(EXTRA_INFO_KEY.c_str());
    json->Put(EXTRA_INFO_KEY.c_str(), "test::test");
    dragEnd(nullptr, json->ToString());
    videoPattern->isInitialState_ = true;
    dragEnd(nullptr, json->ToString());

    /**
     * @tc.steps: step3. Call dragEnd while use new video src.
     * @tc.expected: step3. VideoPattern property set correctly.
     */
    json->Delete(EXTRA_INFO_KEY.c_str());
    json->Put(EXTRA_INFO_KEY.c_str(), "test::newTest");
    videoPattern->SetIsStop(false);
    videoPattern->isInitialState_ = false;
    dragEnd(nullptr, json->ToString());
    EXPECT_TRUE(videoPattern->isStop_);
}

/**
 * @tc.name: VideoFocusTest001
 * @tc.desc: Create Video, and test focus.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoFocusTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    EXPECT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);
    EXPECT_EQ(pattern->GetFocusPattern().focusType_, FocusType::SCOPE);
}

/**
 * @tc.name: VideoFocusTest002
 * @tc.desc: Create Video, and test focus and children node focus.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoFocusTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto frameNode = CreateVideoNode(testProperty);
    EXPECT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    frameNode->GetOrCreateFocusHub()->currentFocus_ = true;
    auto videoPattern = frameNode->GetPattern<VideoPattern>();
    CHECK_NULL_VOID(videoPattern);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));

    auto videoLayoutProperty = frameNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_NE(videoLayoutProperty, nullptr);

    /**
     * @tc.steps: step2. Create LayoutWrapper and set videoLayoutAlgorithm.
     * @tc.expected: step2. Create video pattern and node successfully.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    LayoutWrapperNode layoutWrapper = LayoutWrapperNode(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto videoLayoutAlgorithm = videoPattern->CreateLayoutAlgorithm();
    ASSERT_NE(videoLayoutAlgorithm, nullptr);
    layoutWrapper.SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(videoLayoutAlgorithm));

    /**
     * @tc.steps: step3. Set source size and layout size.
     * @tc.expected: step3. Set successfully.
     */
    LayoutConstraintF layoutConstraint;
    videoLayoutProperty->UpdateVideoSize(VIDEO_SIZE);
    layoutConstraint.selfIdealSize.SetSize(VIDEO_SIZE);
    auto videoSize1 =
        videoLayoutAlgorithm->MeasureContent(layoutConstraint, &layoutWrapper).value_or(SizeF(0.0f, 0.0f));
    EXPECT_EQ(videoSize1, SizeF(VIDEO_WIDTH, VIDEO_WIDTH));
    layoutWrapper.GetGeometryNode()->SetContentSize(videoSize1);

    /**
     * @tc.steps: step4. Set the framenode tree, and test the focus.
     * @tc.expected: step4. Test focus on child successfully.
     */
    frameNode->GetOrCreateFocusHub()->RequestFocusImmediately();

    for (const auto& child : frameNode->GetChildren()) {
        if (child->GetTag() == V2::ROW_ETS_TAG) {
            auto slider = AceType::DynamicCast<FrameNode>(child->GetChildAtIndex(SLIDER_INDEX));
            EXPECT_EQ(slider->GetOrCreateFocusHub()->IsFocusable(), true);
            break;
        }
    }
}

/**
 * @tc.name: VideoAccessibilityPropertyTest001
 * @tc.desc: Test the text property of VideoAccessibilityProperty.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoAccessibilityPropertyTest001, TestSize.Level1)
{
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    ASSERT_NE(videoController, nullptr);
    video.Create(videoController);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(pattern, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    auto videoAccessibilitProperty = frameNode->GetAccessibilityProperty<VideoAccessibilityProperty>();
    ASSERT_NE(videoAccessibilitProperty, nullptr);
    EXPECT_EQ(videoAccessibilitProperty->GetText(), "");

    video.SetSrc(VIDEO_SRC);
    EXPECT_EQ(videoAccessibilitProperty->GetText(), VIDEO_SRC);
}

/**
 * @tc.name: VideoAccessibilityPropertyTest002
 * @tc.desc: Test the rangeInfo property of VideoAccessibilityProperty.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoAccessibilityPropertyTest002, TestSize.Level1)
{
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_NE(frameNode, nullptr);
    auto videoAccessibilitProperty = frameNode->GetAccessibilityProperty<VideoAccessibilityProperty>();
    ASSERT_NE(videoAccessibilitProperty, nullptr);
    EXPECT_TRUE(videoAccessibilitProperty->HasRange());
    auto accessibilityValue = videoAccessibilitProperty->GetAccessibilityValue();
    EXPECT_EQ(accessibilityValue.min, 0);
    EXPECT_EQ(accessibilityValue.max, 0);
    EXPECT_EQ(accessibilityValue.current, 0);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));
    pattern->OnPrepared(VIDEO_WIDTH, VIDEO_HEIGHT, DURATION, 0, true);
    EXPECT_EQ(pattern->duration_, DURATION);
    pattern->currentPos_ = CURRENT_TIME;
    accessibilityValue = videoAccessibilitProperty->GetAccessibilityValue();
    EXPECT_EQ(accessibilityValue.min, 0);
    EXPECT_EQ(accessibilityValue.max, DURATION);
    EXPECT_EQ(accessibilityValue.current, CURRENT_TIME);

    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), Stop()).WillOnce(Return(0));
    pattern->Stop();
    accessibilityValue = videoAccessibilitProperty->GetAccessibilityValue();
    EXPECT_EQ(accessibilityValue.current, 0);
}

/**
 * @tc.name: VideoPatternTest016
 * @tc.desc: Test full screen button click event
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    testProperty.videoController = videoController;
    auto frameNode = CreateVideoNode(testProperty);
    ASSERT_TRUE(frameNode);
    EXPECT_EQ(frameNode->GetTag(), V2::VIDEO_ETS_TAG);
    auto pattern = frameNode->GetPattern<VideoPattern>();
    ASSERT_TRUE(pattern);

    // Add a redundant node to go other branch
    auto tempFrameNode = AceType::MakeRefPtr<FrameNode>("TEMP", -1, AceType::MakeRefPtr<Pattern>());
    frameNode->AddChild(tempFrameNode, 0);

    // set video event
    auto videoEventHub = frameNode->GetEventHub<VideoEventHub>();
    ASSERT_TRUE(videoEventHub);

    std::string fullScreenCheck;
    EventCallback onFullScreenChange = [&fullScreenCheck](const std::string& param) { fullScreenCheck = param; };
    videoEventHub->SetOnFullScreenChange(std::move(onFullScreenChange));

    /**
     * @tc.steps: step2. call FullScreen & ExitFullScreen
     * @tc.expected: step3. onFullScreenChange(true / false)  will be called
     */
    auto json = JsonUtil::Create(true);
    json->Put("fullscreen", true);
    auto fullScreenTrue = json->ToString();
    pattern->FullScreen(); // will called onFullScreenChange(true)
    EXPECT_TRUE(pattern->GetFullScreenNode() != nullptr);
    EXPECT_EQ(fullScreenCheck, fullScreenTrue);

    fullScreenCheck.clear();
    pattern->FullScreen(); // call again, nothing will happen
    EXPECT_TRUE(pattern->GetFullScreenNode() != nullptr);
    EXPECT_TRUE(fullScreenCheck.empty());

    // get the full screen svg node & get its gestureEventHub
    auto fullScreenNode = pattern->GetFullScreenNode();
    const auto& children = fullScreenNode->GetChildren();
    RefPtr<UINode> controlBar = nullptr;
    for (const auto& child : children) {
        if (child->GetTag() == V2::ROW_ETS_TAG) {
            controlBar = child;
        }
    }
    ASSERT_TRUE(controlBar);
    auto fsBtn = AceType::DynamicCast<FrameNode>(controlBar->GetChildAtIndex(4));
    ASSERT_TRUE(fsBtn);
    auto fsEvent = fsBtn->GetOrCreateGestureEventHub();

    fsEvent->ActClick(); // this will call ExitFullScreen()
    json = JsonUtil::Create(true);
    json->Put("fullscreen", false);
    auto fullScreenFalse = json->ToString();
    EXPECT_TRUE(pattern->GetFullScreenNode() == nullptr);
    EXPECT_EQ(fullScreenCheck, fullScreenFalse);

    fullScreenCheck.clear();

    const auto& videoChildren = frameNode->GetChildren();
    RefPtr<UINode> videoControlBar = nullptr;
    for (const auto& child : videoChildren) {
        if (child->GetTag() == V2::ROW_ETS_TAG) {
            videoControlBar = child;
        }
    }
    ASSERT_TRUE(videoControlBar);
    auto videoBtn = AceType::DynamicCast<FrameNode>(videoControlBar->GetChildAtIndex(4));
    ASSERT_TRUE(videoBtn);
    auto videoEvent = videoBtn->GetOrCreateGestureEventHub();
    videoEvent->ActClick(); // this will call FullScreen()
    EXPECT_TRUE(pattern->GetFullScreenNode() != nullptr);

    /**
     * @tc.steps: step4. call OnBackPressed
     * @tc.expected: step4. ExitFullScreen() will be called
     */
    // construct a FullScreenManager
    auto rootNode = MockPipelineBase::GetCurrent()->rootNode_;
    auto fullScreenManager = AceType::MakeRefPtr<FullScreenManager>(rootNode);

    auto flag = fullScreenManager->OnBackPressed(); // will on videoPattern->OnBackPressed()
    EXPECT_TRUE(flag);
    EXPECT_TRUE(pattern->GetFullScreenNode() == nullptr);
    EXPECT_EQ(fullScreenCheck, fullScreenFalse);

    rootNode->AddChild(tempFrameNode);
    flag = fullScreenManager->OnBackPressed(); // call again, nothing happen
    EXPECT_FALSE(flag);

    pattern->OnBackPressed(); // nothing will happen
    EXPECT_TRUE(pattern->GetFullScreenNode() == nullptr);

    /**
     * @tc.steps: step5. call FullScreen & ExitFullScreen in videoController
     *                   note: just test ExitFullscreen(issync = true), other functions are async
     * @tc.expected: step5. onFullScreenChange(true / false)  will be called
     */
    pattern->FullScreen();
    EXPECT_TRUE(pattern->GetFullScreenNode() != nullptr);
    EXPECT_EQ(fullScreenCheck, fullScreenTrue);
    videoController->ExitFullscreen(false); // nothing will happen for it's async
    EXPECT_TRUE(pattern->GetFullScreenNode() != nullptr);
    videoController->ExitFullscreen(true);
    EXPECT_FALSE(pattern->GetFullScreenNode() != nullptr);
    EXPECT_EQ(fullScreenCheck, fullScreenFalse);
}

/**
 * @tc.name: VideoNodeTest001
 * @tc.desc: Create Video.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoNodeTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    auto videoNode = VideoNode::GetOrCreateVideoNode(V2::VIDEO_ETS_TAG, VIDEO_NODE_ID_1,
        [videoController]() { return AceType::MakeRefPtr<VideoPattern>(videoController); });
    EXPECT_TRUE(videoNode);
    EXPECT_EQ(videoNode->GetTag(), V2::VIDEO_ETS_TAG);

    auto secondVideoNode = VideoNode::GetOrCreateVideoNode(V2::VIDEO_ETS_TAG, VIDEO_NODE_ID_1,
        [videoController]() { return AceType::MakeRefPtr<VideoPattern>(videoController); });
    EXPECT_TRUE(secondVideoNode);
    EXPECT_EQ(videoNode->GetTag(), secondVideoNode->GetTag());
    EXPECT_EQ(videoNode->GetId(), secondVideoNode->GetId());

    /**
     * @tc.steps: step2. Create Video again
     * @tc.expected: step2. Create Video node successfully
     */
    auto thirdVideoNode = VideoNode::GetOrCreateVideoNode(V2::VIDEO_ETS_TAG, VIDEO_NODE_ID_2,
        [videoController]() { return AceType::MakeRefPtr<VideoPattern>(videoController); });
    EXPECT_TRUE(secondVideoNode);
    EXPECT_EQ(videoNode->GetTag(), thirdVideoNode->GetTag());
    EXPECT_NE(videoNode->GetId(), thirdVideoNode->GetId());
}

/**
 * @tc.name: VideoPatternEventTest001
 * @tc.desc: Create Video and test onerror event.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternEventTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    ASSERT_NE(videoController, nullptr);
    video.Create(videoController);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(pattern, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    /**
     * @tc.steps: step2. Execute onerror
     * @tc.expected: step1. Execute onerror successfully
     */
    std::string result;
    auto errorCallback = [&result](const std::string& error) { result = VIDEO_CALLBACK_RESULT; };
    auto eventHub = pattern->GetEventHub<VideoEventHub>();
    eventHub->SetOnError(std::move(errorCallback));
    pattern->OnError(VIDEO_ERROR_ID);
    EXPECT_EQ(result, VIDEO_CALLBACK_RESULT);
}

/**
 * @tc.name: VideoPatternEventTest002
 * @tc.desc: Create Video and test onvisiblechange event.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternEventTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    ASSERT_NE(videoController, nullptr);
    video.Create(videoController);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(pattern, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));

    bool result;

    /**
     * @tc.steps: step2. Make video visible
     * @tc.expected: step2. Video callback result is false.
     */
    auto visibleChangeCallback = [&result](bool visible) { result = visible; };
    pattern->SetHiddenChangeEvent(std::move(visibleChangeCallback));
    pattern->OnVisibleChange(true);
    EXPECT_FALSE(result);

    /**
     * @tc.steps: step3. Make video visible false
     * @tc.expected: step3. Video callback result is true.
     */
    pattern->OnVisibleChange(false);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: VideoPatternPlayerTest001
 * @tc.desc: Create Video and test HasPlayer() func.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternPlayerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video
     * @tc.expected: step1. Create Video successfully
     */
    VideoModelNG video;
    auto videoController = AceType::MakeRefPtr<VideoControllerV2>();
    ASSERT_NE(videoController, nullptr);
    video.Create(videoController);

    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    auto pattern = AceType::DynamicCast<VideoPattern>(frameNode->GetPattern());
    ASSERT_NE(pattern, nullptr);
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(pattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));

    /**
     * @tc.steps: step2. Make video
     * @tc.expected: step2. Video HasPlayer Func result is true.
     */
    bool result = pattern->HasPlayer();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: VideoFullScreenTest001
 * @tc.desc: Test VideoFullScreenPattern UpdateState.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoFullScreenTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get fullScreenPattern.
     * @tc.expected: Create Video successfully.
     */
    MockPipelineBase::GetCurrent()->SetRootSize(SCREEN_WIDTH_MEDIUM, SCREEN_HEIGHT_MEDIUM);
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);
    videoPattern->FullScreen();
    auto fullScreenNode = videoPattern->GetFullScreenNode();
    ASSERT_NE(fullScreenNode, nullptr);
    auto fullScreenPattern = fullScreenNode->GetPattern<VideoFullScreenPattern>();
    ASSERT_NE(fullScreenPattern, nullptr);

    /**
     * @tc.steps: step2. Call UpdateState when videoLayout's properties is equal or not equal to fullScreenLayout.
     * @tc.expected: FullScreenLayout's properties is set same to videoLayout.
     */
    auto videoLayout = videoNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_NE(videoLayout, nullptr);
    auto fullScreenLayout = fullScreenNode->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_NE(fullScreenLayout, nullptr);
    fullScreenPattern->UpdateState();
    EXPECT_FALSE(videoLayout->HasObjectFit());
    EXPECT_FALSE(videoLayout->HasVideoSource());
    EXPECT_FALSE(videoLayout->HasPosterImageInfo());
    EXPECT_FALSE(videoLayout->HasControls());
    videoLayout->UpdateObjectFit(ImageFit::COVER);
    fullScreenLayout->UpdateObjectFit(ImageFit::CONTAIN);
    videoLayout->UpdateVideoSource(VIDEO_SRC);
    fullScreenLayout->UpdateVideoSource("");
    videoLayout->UpdatePosterImageInfo(ImageSourceInfo(VIDEO_POSTER_URL));
    fullScreenLayout->UpdatePosterImageInfo(ImageSourceInfo(""));
    videoLayout->UpdateControls(true);
    fullScreenLayout->UpdateControls(false);
    fullScreenPattern->UpdateState();
    EXPECT_EQ(fullScreenLayout->GetObjectFit().value(), ImageFit::COVER);
    EXPECT_EQ(fullScreenLayout->GetVideoSource().value(), VIDEO_SRC);
    EXPECT_EQ(fullScreenLayout->GetPosterImageInfo().value(), ImageSourceInfo(VIDEO_POSTER_URL));
    EXPECT_TRUE(fullScreenLayout->GetControls().value());
    fullScreenPattern->UpdateState();
}

/**
 * @tc.name: VideoPatternTest021
 * @tc.desc: Test VideoPattern ResetMediaPlayer.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Call ResetMediaPlayer when mediaPlayer_ in different status.
     * @tc.expected: mediaPlayer_'s functions is called.
     */
    videoPattern->src_ = VIDEO_SRC;
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), SetSource(_))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), PrepareAsync())
        .WillOnce(Return(0))
        .WillOnce(Return(-1));
    videoPattern->ResetMediaPlayer();
    videoPattern->ResetMediaPlayer();
    videoPattern->ResetMediaPlayer();
    videoPattern->mediaPlayer_ = nullptr;
    videoPattern->SetSourceForMediaPlayer();
}

/**
 * @tc.name: VideoPatternTest022
 * @tc.desc: Test VideoPattern UpdateMediaPlayerOnBg.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Call ResetMediaPlayer while autoPlay_ and isInitialState_ in different status.
     * @tc.expected: PrepareAsync is called only once.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), PrepareAsync())
        .WillOnce(Return(-1));
    videoPattern->isStop_ = true;
    videoPattern->isInitialState_ = false;
    videoPattern->UpdateMediaPlayerOnBg();
    videoPattern->autoPlay_ = true;
    videoPattern->UpdateMediaPlayerOnBg();
    videoPattern->isInitialState_ = true;
    videoPattern->UpdateMediaPlayerOnBg();
}

/**
 * @tc.name: VideoPatternTest023
 * @tc.desc: Test VideoPattern OnCurrentTimeChange.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Call OnCurrentTimeChange with different duration.
     * @tc.expected: GetDuration is called twice.
     */
    videoPattern->duration_ = 0;
    videoPattern->currentPos_ = 0;
    videoPattern->isStop_ = false;
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), GetDuration(_))
        .WillOnce(Return(-1))
        .WillOnce(Return(0));
    videoPattern->OnCurrentTimeChange(1);
    videoPattern->duration_ = 0;
    videoPattern->currentPos_ = 0;
    videoPattern->isStop_ = false;
    videoPattern->OnCurrentTimeChange(1);
}

/**
 * @tc.name: VideoPatternTest024
 * @tc.desc: Test VideoPattern HiddenChange.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Call OnCurrentTimeChange in different status.
     * @tc.expected: pastPlayingStatus_ is set.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), Pause()).Times(1);
    videoPattern->isPlaying_ = false;
    videoPattern->pastPlayingStatus_ = false;
    videoPattern->HiddenChange(true);
    videoPattern->pastPlayingStatus_ = true;
    videoPattern->HiddenChange(true);
    videoPattern->pastPlayingStatus_ = false;
    videoPattern->HiddenChange(false);
    videoPattern->pastPlayingStatus_ = true;
    videoPattern->HiddenChange(false);
    EXPECT_FALSE(videoPattern->pastPlayingStatus_);

    videoPattern->isPlaying_ = true;
    videoPattern->HiddenChange(false);
    videoPattern->HiddenChange(true);
    EXPECT_TRUE(videoPattern->pastPlayingStatus_);
    videoPattern->mediaPlayer_ = nullptr;
    videoPattern->HiddenChange(false);
    videoPattern->HiddenChange(true);
    videoPattern->isPlaying_ = false;
    videoPattern->HiddenChange(false);
    videoPattern->HiddenChange(true);
}

/**
 * @tc.name: VideoPatternTest025
 * @tc.desc: Test VideoPattern PrepareSurface.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Call PrepareSurface in different status.
     * @tc.expected: SetSurface function is called.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockRenderSurface>(videoPattern->renderSurface_)), IsSurfaceValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), SetSurface())
        .WillOnce(Return(-1));
    videoPattern->PrepareSurface();
    SystemProperties::SetExtSurfaceEnabled(false);
    videoPattern->PrepareSurface();
}

/**
 * @tc.name: VideoPatternTest026
 * @tc.desc: Test VideoPattern OnModifyDone.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Call hiddenChangeEvent while video not in full screen mode.
     * @tc.expected: pastPlayingStatus_ is set.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), Pause()).Times(1);
    EXPECT_CALL(*(AceType::DynamicCast<MockRenderSurface>(videoPattern->renderSurface_)), IsSurfaceValid())
        .WillRepeatedly(Return(false));
    videoPattern->OnModifyDone();
    auto hiddenChangeEvent = videoPattern->hiddenChangeEvent_;
    ASSERT_NE(hiddenChangeEvent, nullptr);
    videoPattern->isPlaying_ = true;
    hiddenChangeEvent(true);
    EXPECT_TRUE(videoPattern->pastPlayingStatus_);

    /**
     * @tc.steps: step2. Call hiddenChangeEvent while video in full screen mode.
     * @tc.expected: pastPlayingStatus_ is set.
     */
    videoPattern->FullScreen();
    videoPattern->isPlaying_ = true;
    hiddenChangeEvent(true);
    EXPECT_TRUE(videoPattern->pastPlayingStatus_);
    videoPattern->OnModifyDone();
}

/**
 * @tc.name: VideoPatternTest027
 * @tc.desc: Test VideoPattern UpdateControllerBar.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<VideoNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);
    auto layoutProperty = videoPattern->GetLayoutProperty<VideoLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step2. Update Video controllerBar while controllerBar is show or not.
     * @tc.expected: Visibility value is changed.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockRenderSurface>(videoPattern->renderSurface_)), IsSurfaceValid())
        .WillOnce(Return(false));
    layoutProperty->UpdateControls(false);
    auto controller = AceType::DynamicCast<FrameNode>(videoNode->GetControllerRow());
    ASSERT_NE(controller, nullptr);
    auto controllerLayoutProperty = controller->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(controllerLayoutProperty, nullptr);
    videoPattern->UpdateControllerBar();
    EXPECT_EQ(controllerLayoutProperty->GetVisibilityValue(), VisibleType::INVISIBLE);
    layoutProperty->UpdateControls(true);
    videoNode->children_.clear();
    videoPattern->UpdateControllerBar();
}

/**
 * @tc.name: VideoPatternTest028
 * @tc.desc: Test VideoPattern UpdateVideoProperty and OnRebuildFrame.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest028, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<VideoNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Update Video property while isInitialState_ and autoPlay_ changes.
     * @tc.expected: PrepareAsync function is called only once.
     */
    videoPattern->isStop_ = true;
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), PrepareAsync())
        .Times(1)
        .WillOnce(Return(-1));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(true));
    videoPattern->isInitialState_ = false;
    videoPattern->autoPlay_ = false;
    videoPattern->UpdateVideoProperty();
    videoPattern->autoPlay_ = true;
    videoPattern->UpdateVideoProperty();
    videoPattern->isInitialState_ = true;
    videoPattern->autoPlay_ = false;
    videoPattern->UpdateVideoProperty();
    videoPattern->autoPlay_ = true;
    videoPattern->UpdateVideoProperty();

    /**
     * @tc.steps: step2. Call OnRebuildFrame while renderSurface_ in different status.
     * @tc.expected: IsSurfaceValid function is called only once.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockRenderSurface>(videoPattern->renderSurface_)), IsSurfaceValid())
        .Times(1)
        .WillOnce(Return(true));
    videoPattern->OnRebuildFrame();
    videoPattern->renderSurface_ = nullptr;
    videoPattern->OnRebuildFrame();
}

/**
 * @tc.name: VideoPatternTest029
 * @tc.desc: Test VideoPattern OnColorConfigurationUpdate.
 * @tc.type: FUNC
 */
HWTEST_F(VideoTestNg, VideoPatternTest029, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Video and get videoPattern.
     */
    VideoModelNG video;
    video.Create(AceType::MakeRefPtr<VideoControllerV2>());
    auto videoNode = AceType::DynamicCast<VideoNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(videoNode, nullptr);
    auto videoPattern = videoNode->GetPattern<VideoPattern>();
    ASSERT_NE(videoPattern, nullptr);

    /**
     * @tc.steps: step2. Call OnColorConfigurationUpdate with different childNode in controlBar_.
     * @tc.expected: BackgroundColor of renderContext is set.
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockRenderSurface>(videoPattern->renderSurface_)), IsSurfaceValid())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*(AceType::DynamicCast<MockMediaPlayer>(videoPattern->mediaPlayer_)), IsMediaPlayerValid())
        .WillRepeatedly(Return(false));
    ASSERT_NE(videoPattern->controlBar_, nullptr);
    auto renderContext = videoPattern->controlBar_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto videoTheme = PipelineBase::GetCurrentContext()->GetTheme<VideoTheme>();
    ASSERT_NE(videoTheme, nullptr);
    auto textNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<TextPattern>(); });
    textNode->layoutProperty_ = nullptr;
    videoPattern->controlBar_->children_.emplace_back(textNode);
    videoPattern->OnColorConfigurationUpdate();
    EXPECT_FALSE(videoNode->needCallChildrenUpdate_);
    EXPECT_EQ(renderContext->GetBackgroundColorValue(), videoTheme->GetBkgColor());
}
} // namespace OHOS::Ace::NG
