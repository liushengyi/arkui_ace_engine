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

#include "gtest/gtest.h"

#define protected public
#define private public
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_property.h"
#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/core/pipeline/mock_pipeline_base.h"
#include "mock_navigation_stack.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const std::string NAVIGATION_TITLE = "NavdestinationTestNg";
const std::string NAVIGATION_SUBTITLE = "NavdestinationSubtitle";
} // namespace

class NavdestinationTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void MockPipelineContextGetTheme();
};

void NavdestinationTestNg::SetUpTestSuite()
{
    MockPipelineBase::SetUp();
}

void NavdestinationTestNg::TearDownTestSuite()
{
    MockPipelineBase::TearDown();
}

void NavdestinationTestNg::MockPipelineContextGetTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<NavigationBarTheme>()));
}

/**
 * @tc.name: NavDestinationTest001
 * @tc.desc: Test Create.
 * @tc.type: FUNC
 */
HWTEST_F(NavdestinationTestNg, NavdestinationTest001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavDestinationModelNG NavDestinationModelNG;
    NavDestinationModelNG.Create();
    NavDestinationModelNG.SetTitleHeight(0);
    NavDestinationModelNG.SetHideTitleBar(true);
    NavDestinationModelNG.SetTitle(NAVIGATION_TITLE, true);
    NavDestinationModelNG.SetSubtitle(NAVIGATION_SUBTITLE);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(frameNode, nullptr);
    auto navigationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto navDestinationPattern = navigationGroupNode->GetPattern<NavDestinationPattern>();
    ASSERT_NE(navDestinationPattern, nullptr);
    auto navDestinationLayoutProperty = navDestinationPattern->GetLayoutProperty<NavDestinationLayoutProperty>();
    ASSERT_NE(navDestinationLayoutProperty, nullptr);
}

/**
 * @tc.name: NavDestinationTest002
 * @tc.desc: Test NavDestinationModelNG SetTitle & SetSubTitle.
 * @tc.type: FUNC
 */
HWTEST_F(NavdestinationTestNg, NavdestinationTest002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavDestinationModelNG NavDestinationModelNG;
    NavDestinationModelNG.Create();
    NavDestinationModelNG.SetHideTitleBar(true);
    NavDestinationModelNG.SetTitle(NAVIGATION_TITLE, true);
    NavDestinationModelNG.SetSubtitle(NAVIGATION_SUBTITLE);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(frameNode, nullptr);
    auto navigationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
    auto titleNode = navigationGroupNode->GetTitle();
    ASSERT_NE(titleNode, nullptr);
    auto subTitleNode = navigationGroupNode->GetSubtitle();
    ASSERT_NE(subTitleNode, nullptr);
    NavDestinationModelNG.SetTitle(NAVIGATION_TITLE, false);
    auto newSubTitleNode = navigationGroupNode->GetSubtitle();
    ASSERT_EQ(newSubTitleNode, nullptr);
}

/**
 * @tc.name: NavDestinationTest003
 * @tc.desc: Test Create.
 * @tc.type: FUNC
 */
HWTEST_F(NavdestinationTestNg, NavdestinationTest003, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavDestinationModelNG NavDestinationModelNG;
    auto builderFunc = []() {};
    NavDestinationModelNG.Create(std::move(builderFunc));
    NavDestinationModelNG.SetHideTitleBar(false);
    NavDestinationModelNG.SetTitle(NAVIGATION_TITLE, false);
    NavDestinationModelNG.SetSubtitle(NAVIGATION_SUBTITLE);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(frameNode, nullptr);
    auto navigationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
}

/**
 * @tc.name: NavDestinationTest004
 * @tc.desc: Test CustomTitle.
 * @tc.type: FUNC
 */
HWTEST_F(NavdestinationTestNg, NavdestinationTest004, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    NavDestinationModelNG NavDestinationModelNG;
    auto builderFunc = []() {};
    auto onBackPressed = []() -> bool { return true; };
    NavDestinationModelNG.Create(std::move(builderFunc));
    NavDestinationModelNG.SetHideTitleBar(false);
    NavDestinationModelNG.SetTitle(NAVIGATION_TITLE, false);
    NavDestinationModelNG.SetSubtitle(NAVIGATION_SUBTITLE);
    NavDestinationModelNG.SetOnShown(std::move(builderFunc));
    NavDestinationModelNG.SetOnHidden(std::move(builderFunc));
    NavDestinationModelNG.SetOnBackPressed(std::move(onBackPressed));

    RefPtr<NG::UINode> customNode;
    NavDestinationModelNG.SetCustomTitle(customNode);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->GetMainFrameNode());
    ASSERT_NE(frameNode, nullptr);
    auto navigationGroupNode = AceType::DynamicCast<NavDestinationGroupNode>(frameNode);
    ASSERT_NE(navigationGroupNode, nullptr);
}
} // namespace OHOS::Ace::NG
