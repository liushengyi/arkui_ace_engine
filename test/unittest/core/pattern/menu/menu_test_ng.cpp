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

#include "gtest/gtest.h"

#define private public
#define protected public

#include "core/components/common/layout/constants.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_model_ng.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_view.h"
#include "core/components_ng/pattern/menu/menu_model_ng.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/multi_menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_layout_algorithm.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_pattern.h"
#include "core/components_ng/pattern/menu/sub_menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/syntax/lazy_for_each_model.h"
#include "core/components_ng/syntax/lazy_layout_wrapper_builder.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/mock/core/rosen/testing_canvas.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "core/event/touch_event.h"
#include "test/mock/core/pipeline/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TARGET_ID = 3;
constexpr MenuType TYPE = MenuType::MENU;
constexpr int32_t SELECTED_INDEX = 10;
constexpr float CURRENT_OFFSET = -0.5f;
const std::string EMPTY_TEXT = "";
const std::string TEXT_TAG = "text";
const std::string MENU_TAG = "menu";
const std::string MENU_ITEM_TEXT = "menuItem";
const std::string MENU_ITEM_GROUP_TEXT = "menuItemGroup";
const std::string MENU_TOUCH_EVENT_TYPE = "1";
const DirtySwapConfig configDirtySwap = { false, false, false, false, true, false };
constexpr Color ITEM_DISABLED_COLOR = Color(0x0c182431);

constexpr float FULL_SCREEN_WIDTH = 720.0f;
constexpr float FULL_SCREEN_HEIGHT = 1136.0f;
constexpr float POSITION_OFFSET = 100.0f;
constexpr float TARGET_SIZE_WIDTH = 100.0f;
constexpr float TARGET_SIZE_HEIGHT = 100.0f;
constexpr float MENU_SIZE_WIDTH = 150.0f;
constexpr float MENU_SIZE_HEIGHT = 150.0f;
constexpr double MENU_OFFSET_X = 10.0;
constexpr double MENU_OFFSET_Y = 10.0;
constexpr float MENU_ITEM_SIZE_WIDTH = 100.0f;
constexpr float MENU_ITEM_SIZE_HEIGHT = 50.0f;

constexpr int CHILD_SIZE_X = 1;
constexpr int CHILD_SIZE_Y = 2;
constexpr int TOP_POSITION_X = 10;
constexpr int TOP_POSITION_Y = 20;
constexpr int BOTTOM_POSITION_X = 30;
constexpr int BOTTOM_POSITION_Y = 40;
constexpr float OFFSET_FIRST = 20.0f;
constexpr float OFFSET_SECOND = 5.0f;
constexpr float OFFSET_THIRD = 200.0f;
constexpr float OFFSET_FORTH = 300.0f;
constexpr float OFFSET_FIFTH = 50.0f;
constexpr float OFFSET_SIXTH = 60.0f;
constexpr int SIZE_X_SECOND = 20;
constexpr int SIZE_Y_SECOND = 20;
constexpr int OFFSET_X_THIRD = 100;
constexpr int OFFSET_Y_THIRD = 20;
constexpr int NODEID = 1;
constexpr int TOP_LEFT_X = 100;
constexpr int TOP_LEFT_Y = 18;
constexpr int TOP_RIGHT_X = 119;
constexpr int TOP_RIGHT_Y = 18;
constexpr int BOTTOM_LEFT_X = 100;
constexpr int BOTTOM_LEFT_Y = 40;
constexpr int BOTTOM_RIGHT_X = 119;
constexpr int BOTTOM_RIGHT_Y = 40;
constexpr int PLACEMENT_LEFT_X = 99;
constexpr int PLACEMENT_LEFT_Y = 29;
constexpr int PLACEMENT_LEFT_TOP_X = 99;
constexpr int PLACEMENT_LEFT_TOP_Y = 20;
constexpr int PLACEMENT_LEFT_BOTTOM_X = 99;
constexpr int PLACEMENT_LEFT_BOTTOM_Y = 38;
constexpr int PLACEMENT_RIGHT_X = 120;
constexpr int PLACEMENT_RIGHT_Y = 29;
constexpr int PLACEMENT_RIGHT_TOP_X = 120;
constexpr int PLACEMENT_RIGHT_TOP_Y = 20;
constexpr int PLACEMENT_RIGHT_BOTTOM_X = 120;
constexpr int PLACEMENT_RIGHT_BOTTOM_Y = 38;
const SizeF FULL_SCREEN_SIZE(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
const Dimension CONTAINER_OUTER_RADIUS = 16.0_vp;
constexpr float PREVIEW_WIDTH = 684.0f;
constexpr float PREVIEW_HEIGHT = 125.0f;
constexpr float GREATER_WINDOW_MENU_HEIGHT = 1190.0f;
constexpr float GREATER_WINDOW_PREVIEW_WIDTH = 800.0f;
constexpr float GREATER_WINDOW_PREVIEW_WIDTH_SECOND = 900.0f;
constexpr float GREATER_WINDOW_PREVIEW_HEIGHT = 1165.0f;
constexpr float GREATER_WINDOW_PREVIEW_HEIGHT_SECOND = 1376.0f;
constexpr float GREATER_HALF_PREVIEW_MENUITEM_HEIGHT = 700.0f;
constexpr float SCALE_PREVIEW_WIDTH = 547.2f;
constexpr float SCALE_PREVIEW_WIDTH_SECOND = 513.0f;
constexpr float SCALE_PREVIEW_WIDTH_THIRD = 342.0f;
constexpr float SCALE_PREVIEW_WIDTH_FORTH = 576.0f;
constexpr float SCALE_PREVIEW_HEIGHT = 270.0f;
constexpr float SCALE_PREVIEW_HEIGHT_SECOND = 688.0f;
constexpr float SCALE_PREVIEW_HEIGHT_THIRD = 932.0f;
constexpr float SCALE_PREVIEW_HEIGHT_FORTH = 80.0f;
constexpr float SCALE_OFFSET = 80.0f;
constexpr float SCALE_MENU_HEIGHT = 952.0f;
constexpr float SCALE_MENU_HEIGHT_SECOND = 150.0f;
constexpr float SCALE_MENU_HEIGHT_THIRD = 344.0f;
constexpr float MENU_SIZE_WIDTH_SECOND = 199.0f;
constexpr double DIP_SCALE = 1.5;
} // namespace
class MenuTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void InitMenuTestNg();
    void InitMenuItemTestNg();
    PaintWrapper* GetPaintWrapper(RefPtr<MenuPaintProperty> paintProperty);
    RefPtr<FrameNode> GetPreviewMenuWrapper(SizeF itemSize = SizeF(0.0f, 0.0f));
    RefPtr<FrameNode> menuFrameNode_;
    RefPtr<MenuAccessibilityProperty> menuAccessibilityProperty_;
    RefPtr<FrameNode> menuItemFrameNode_;
    RefPtr<MenuItemPattern> menuItemPattern_;
    RefPtr<MenuItemAccessibilityProperty> menuItemAccessibilityProperty_;
};

void MenuTestNg::SetUpTestCase() {}

void MenuTestNg::TearDownTestCase() {}

void MenuTestNg::SetUp()
{
    MockPipelineBase::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
}

void MenuTestNg::TearDown()
{
    MockPipelineBase::TearDown();
    menuFrameNode_ = nullptr;
    menuAccessibilityProperty_ = nullptr;
    menuItemFrameNode_ = nullptr;
    menuItemPattern_ = nullptr;
    menuItemAccessibilityProperty_ = nullptr;
    SystemProperties::SetDeviceType(DeviceType::PHONE);
    ScreenSystemManager::GetInstance().dipScale_ = 1.0;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
}

void MenuTestNg::InitMenuTestNg()
{
    menuFrameNode_ = FrameNode::GetOrCreateFrameNode(V2::MENU_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(),
        []() { return AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE); });
    ASSERT_NE(menuFrameNode_, nullptr);

    menuAccessibilityProperty_ = menuFrameNode_->GetAccessibilityProperty<MenuAccessibilityProperty>();
    ASSERT_NE(menuAccessibilityProperty_, nullptr);
}

void MenuTestNg::InitMenuItemTestNg()
{
    menuItemFrameNode_ = FrameNode::GetOrCreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ViewStackProcessor::GetInstance()->ClaimNodeId(), []() { return AceType::MakeRefPtr<MenuItemPattern>(); });
    ASSERT_NE(menuItemFrameNode_, nullptr);

    menuItemPattern_ = menuItemFrameNode_->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern_, nullptr);

    menuItemAccessibilityProperty_ = menuItemFrameNode_->GetAccessibilityProperty<MenuItemAccessibilityProperty>();
    ASSERT_NE(menuItemAccessibilityProperty_, nullptr);
}

PaintWrapper* MenuTestNg::GetPaintWrapper(RefPtr<MenuPaintProperty> paintProperty)
{
    WeakPtr<RenderContext> renderContext;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper* paintWrapper = new PaintWrapper(renderContext, geometryNode, paintProperty);
    return paintWrapper;
}

RefPtr<FrameNode> MenuTestNg::GetPreviewMenuWrapper(SizeF itemSize)
{
    auto rootNode = FrameNode::CreateFrameNode(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<RootPattern>());
    CHECK_NULL_RETURN(rootNode, nullptr);
    auto targetNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(targetNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    if (!NearZero(itemSize.Width()) || !NearZero(itemSize.Height())) {
        auto itemGeometryNode = textNode->GetGeometryNode();
        CHECK_NULL_RETURN(itemGeometryNode, nullptr);
        itemGeometryNode->SetFrameSize(itemSize);
    }
    targetNode->MountToParent(rootNode);
    MenuParam menuParam;
    menuParam.type = MenuType::CONTEXT_MENU;
    menuParam.previewMode = MenuPreviewMode::CUSTOM;
    auto customNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(customNode, nullptr);
    auto customGeometryNode = customNode->GetGeometryNode();
    CHECK_NULL_RETURN(customGeometryNode, nullptr);
    customGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto menuWrapperNode =
        MenuView::Create(textNode, targetNode->GetId(), V2::TEXT_ETS_TAG, menuParam, true, customNode);
    return menuWrapperNode;
}

RefPtr<FrameNode> GetImagePreviewMenuWrapper()
{
    auto rootNode = FrameNode::CreateFrameNode(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<RootPattern>());
    CHECK_NULL_RETURN(rootNode, nullptr);
    auto menuWrapperNode = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(1));
    CHECK_NULL_RETURN(menuWrapperNode, nullptr);
    auto menuNode = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    CHECK_NULL_RETURN(menuNode, nullptr);
    auto previewNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    CHECK_NULL_RETURN(previewNode, nullptr);
    menuNode->MountToParent(menuWrapperNode);
    previewNode->MountToParent(menuWrapperNode);
    menuWrapperNode->MountToParent(rootNode);

    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    menuPattern->SetPreviewMode(MenuPreviewMode::IMAGE);
    menuPattern->SetType(MenuType::CONTEXT_MENU);

    return menuWrapperNode;
}

/**
 * @tc.name: MenuWrapperPatternTestNg001
 * @tc.desc: Verify HideMenu(Menu).
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper and child menu
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenuFirst = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 3, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    auto subMenuSecond = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 4, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    mainMenu->MountToParent(wrapperNode);
    subMenuFirst->MountToParent(wrapperNode);
    subMenuSecond->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. excute HideMenu
     * @tc.expected: wrapper child size is 3
     */
    wrapperPattern->HideMenu(mainMenu);
    wrapperPattern->OnModifyDone();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 3);
}

/**
 * @tc.name: MenuWrapperPatternTestNg002
 * @tc.desc: Verify HideMenu().
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg002, TestSize.Level1)
{
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    mainMenu->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->HideMenu();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 1);
}

/**
 * @tc.name: MenuWrapperPatternTestNg003
 * @tc.desc: Verify HideSubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    wrapperPattern->HideSubMenu();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. add submenu to wrapper
     * @tc.expected: wrapper child size is 1
     */
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    mainMenu->MountToParent(wrapperNode);
    wrapperPattern->HideSubMenu();
    subMenu->MountToParent(wrapperNode);
    wrapperPattern->HideSubMenu();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 1);
}

/**
 * @tc.name: MenuWrapperPatternTestNg004
 * @tc.desc: Verify HandleMouseEvent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create menuItem and mouseInfo
     * @tc.expected: menuItem and mouseInfo function result as expected
     */
    MouseInfo mouseInfo;
    mouseInfo.SetAction(MouseAction::PRESS);
    mouseInfo.SetGlobalLocation(Offset(200, 200));
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 100, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->SetIsSubMenuShowed(true);
    menuItemPattern->AddHoverRegions(OffsetF(0, 0), OffsetF(100, 100));
    const auto& mousePosition = mouseInfo.GetGlobalLocation();
    EXPECT_TRUE(!menuItemPattern->IsInHoverRegions(mousePosition.GetX(), mousePosition.GetY()));
    EXPECT_TRUE(menuItemPattern->IsSubMenuShowed());
    /**
     * @tc.steps: step2. Create menuWrapper
     * @tc.expected: wrapperPattern is not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    wrapperPattern->HandleMouseEvent(mouseInfo, menuItemPattern);
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step3. add submenu to wrapper
     * @tc.expected: wrapper child size is 2
     */
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    auto currentMenuItemNode =
        FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 101, AceType::MakeRefPtr<MenuItemPattern>());
    auto currentMenuItemPattern = currentMenuItemNode->GetPattern<MenuItemPattern>();
    auto subMenuPattern = subMenu->GetPattern<MenuPattern>();
    currentMenuItemPattern->SetIsSubMenuShowed(true);
    mainMenu->MountToParent(wrapperNode);
    subMenu->MountToParent(wrapperNode);
    EXPECT_EQ(wrapperNode->GetChildren().size(), 2);
    /**
     * @tc.steps: step4. excute HandleMouseEvent
     * @tc.expected: menuItemPattern IsSubMenuShowed as expected
     */
    subMenuPattern->SetParentMenuItem(currentMenuItemNode);
    wrapperPattern->HandleMouseEvent(mouseInfo, menuItemPattern);
    EXPECT_TRUE(currentMenuItemPattern->IsSubMenuShowed());
    subMenuPattern->SetParentMenuItem(menuItemNode);
    wrapperPattern->HandleMouseEvent(mouseInfo, menuItemPattern);
    EXPECT_FALSE(menuItemPattern->IsSubMenuShowed());
}

/**
 * @tc.name: MenuWrapperPatternTestNg005
 * @tc.desc: Verify OnTouchEvent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create touchEventInfo
     * @tc.expected: touchEventInfo size is 1
     */
    TouchEventInfo contextMenuTouchUpEventInfo(MENU_TOUCH_EVENT_TYPE);
    TouchLocationInfo upLocationInfo(TARGET_ID);
    Offset touchUpGlobalLocation(80, 80);
    upLocationInfo.SetTouchType(TouchType::MOVE);
    auto touchUpLocationInfo = upLocationInfo.SetGlobalLocation(touchUpGlobalLocation);
    contextMenuTouchUpEventInfo.touches_.emplace_back(touchUpLocationInfo);
    EXPECT_EQ(contextMenuTouchUpEventInfo.touches_.size(), 1);
    /**
     * @tc.steps: step2. update touchEventInfo, excute OnTouchEvent
     * @tc.expected: touchEventInfo size is 1
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    upLocationInfo.SetTouchType(TouchType::DOWN);
    touchUpLocationInfo = upLocationInfo.SetGlobalLocation(touchUpGlobalLocation);
    contextMenuTouchUpEventInfo.touches_.clear();
    contextMenuTouchUpEventInfo.touches_.emplace_back(touchUpLocationInfo);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    wrapperPattern->isHided_ = true;
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    wrapperPattern->isHided_ = false;
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    EXPECT_EQ(contextMenuTouchUpEventInfo.touches_.size(), 1);
    /**
     * @tc.steps: step3. add submenu to wrapper,excute OnTouchEvent
     * @tc.expected: wrapper child size is 2
     */
    auto topMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto bottomMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 3, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 4, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    topMenu->GetGeometryNode()->SetFrameSize(SizeF(100, 100));
    topMenu->MountToParent(wrapperNode);
    bottomMenu->GetGeometryNode()->SetFrameSize(SizeF(70, 70));
    bottomMenu->MountToParent(wrapperNode);
    subMenu->GetGeometryNode()->SetFrameSize(SizeF(70, 70));
    subMenu->MountToParent(wrapperNode);
    wrapperPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    wrapperPattern->HideMenu();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 2);
}

/**
 * @tc.name: MenuWrapperPatternTestNg006
 * @tc.desc: Verify HideSubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper
     * @tc.expected: wrapper pattern not null
     */
    auto menuItemGroupPattern = AceType::MakeRefPtr<MenuItemGroupPattern>();
    menuItemGroupPattern->hasSelectIcon_ = true;
    auto menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    menuItemGroup->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->OnModifyDone();
    auto algorithm = AceType::MakeRefPtr<MenuItemGroupLayoutAlgorithm>(-1, -1, 0);
    ASSERT_TRUE(algorithm);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto* layoutWrapperNode = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);
    RefPtr<LayoutWrapper> layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, nullptr);
    /**
     * @tc.steps: step2. create firstChildLayoutWrapper and append it to layoutWrapper
     */
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, itemPattern);
    auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
    itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto firstChildLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);
    layoutWrapperNode->AppendChild(firstChildLayoutWrapper);
    layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, firstChildLayoutWrapper);
    /**
     * @tc.steps: step3. add submenu to wrapper
     * @tc.expected: wrapper child size is 1
     */
    wrapperPattern->isFirstShow_ = true;
    EXPECT_FALSE(wrapperPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, configDirtySwap));
    wrapperPattern->SetHotAreas(layoutWrapper);
}

/**
 * @tc.name: MenuWrapperPatternTestNg006
 * @tc.desc: Verify HideSubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto menuItemGroupPattern = AceType::MakeRefPtr<MenuItemGroupPattern>();
    auto menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    auto* layoutWrapperNode = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);
    RefPtr<LayoutWrapper> layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, nullptr);
    /**
     * @tc.steps: step2. create firstChildLayoutWrapper and append it to layoutWrapper
     */
    RefPtr<MenuPattern> menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE);
    menuPattern->isSelectMenu_ = true;
    menuPattern->SetType(MenuType::CONTEXT_MENU);
    auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, menuPattern);
    menuItem->MountToParent(wrapperNode);
    auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
    itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto firstChildLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);

    layoutWrapperNode->AppendChild(firstChildLayoutWrapper);
    layoutWrapper = layoutWrapperNode->GetOrCreateChildByIndex(0, false);
    EXPECT_EQ(layoutWrapper, firstChildLayoutWrapper);
    /**
     * @tc.steps: step3. add submenu to wrapper
     * @tc.expected: wrapper child size is 1
     */
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->isFirstShow_ = true;
    EXPECT_FALSE(wrapperPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, configDirtySwap));
    wrapperPattern->GetMenu()->GetPattern<MenuPattern>()->SetType(MenuType::CONTEXT_MENU);
    wrapperPattern->SetHotAreas(layoutWrapper);
}

/**
 * @tc.name: MenuWrapperPatternTestNg008
 * @tc.desc: CallMenuStateChangeCallback (Menu).
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuWrapperPatternTestNg008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create wrapper and child menu
     * @tc.expected: wrapper pattern not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    
    mainMenu->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    /**
     * @tc.steps: step2. excute HideMenu
     * @tc.expected: wrapper child size is 3
     */
    int32_t callNum = 0;
    std::function<void(const std::string&)> callback = [&](const std::string& param) {
        callNum++;
    };
    wrapperPattern->RegisterMenuStateChangeCallback(callback);
    wrapperPattern->CallMenuStateChangeCallback("false");
    EXPECT_EQ(callNum, 1);
}

/**
 * @tc.name: MenuPatternTestNg001
 * @tc.desc: Verify RegisterOnTouch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg001, TestSize.Level1)
{
    RefPtr<MenuPattern> menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE);
    std::string type = "1";
    TouchEventInfo info(type);
    menuPattern->RegisterOnTouch();
    EXPECT_TRUE(info.GetTouches().empty());
}

/**
 * @tc.name: MenuPatternTestNg002
 * @tc.desc: Verify RegisterOnTouch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg002, TestSize.Level1)
{
    RefPtr<MenuPattern> menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE);
    std::string type = "1";
    TouchType touchType = TouchType::UP;
    TouchEventInfo info(type);
    TouchLocationInfo locationInfo(TARGET_ID);
    Offset globalLocation(1, 1);
    locationInfo.SetTouchType(touchType);
    auto touchLocationInfo = locationInfo.SetGlobalLocation(globalLocation);
    info.touches_.emplace_back(touchLocationInfo);
    menuPattern->RegisterOnTouch();
    EXPECT_FALSE(info.GetTouches().empty());
    EXPECT_TRUE(info.GetTouches().front().GetTouchType() == TouchType::UP);
}

/**
 * @tc.name: MenuPatternTestNg003
 * @tc.desc: Verify RegisterOnTouch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg003, TestSize.Level1)
{
    RefPtr<MenuPattern> menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE);
    std::string type = "1";
    TouchType touchType = TouchType::UP;
    TouchEventInfo info(type);
    TouchLocationInfo locationInfo(TARGET_ID);
    Offset globalLocation(1, 1);
    locationInfo.SetTouchType(touchType);
    auto touchLocationInfo = locationInfo.SetGlobalLocation(globalLocation);
    info.touches_.emplace_back(touchLocationInfo);
    menuPattern->type_ = MenuType::CONTEXT_MENU;
    menuPattern->RegisterOnTouch();
    EXPECT_FALSE(info.GetTouches().empty());
    EXPECT_TRUE(info.GetTouches().front().GetTouchType() == TouchType::UP);
}

/**
 * @tc.name: MenuPatternTestNg004
 * @tc.desc: Verify RegisterOnTouch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg004, TestSize.Level1)
{
    RefPtr<MenuPattern> menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE);
    std::string type = "1";
    TouchEventInfo info(type);
    TouchLocationInfo locationInfo(TARGET_ID);
    Offset globalLocation(1, 1);
    auto touchLocationInfo = locationInfo.SetGlobalLocation(globalLocation);
    info.touches_.emplace_back(touchLocationInfo);
    menuPattern->RegisterOnTouch();
    EXPECT_FALSE(info.GetTouches().empty());
    EXPECT_TRUE(info.GetTouches().front().GetTouchType() == TouchType::UNKNOWN);
}

/**
 * @tc.name: MenuPatternTestNg005
 * @tc.desc: Verify RegisterOnTouch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg005, TestSize.Level1)
{
    RefPtr<MenuPattern> menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE);
    std::string type = "1";
    TouchEventInfo info(type);
    TouchType touchType = TouchType::UP;
    TouchLocationInfo locationInfo(TARGET_ID);
    locationInfo.SetTouchType(touchType);
    menuPattern->RegisterOnTouch();
    EXPECT_TRUE(info.GetTouches().empty());
    EXPECT_FALSE(info.GetTouches().front().GetTouchType() == TouchType::UP);
}

/**
 * @tc.name: MenuPatternTestNg006
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg006, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MneuModelInstance.Create();
    MneuModelInstance.SetFontSize(Dimension(25.0));
    MneuModelInstance.SetFontColor(Color::RED);
    MneuModelInstance.SetFontWeight(FontWeight::BOLD);
    MneuModelInstance.SetFontStyle(Ace::FontStyle::ITALIC);

    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    // call UpdateMenuItemChildren
    menuPattern->OnModifyDone();

    ASSERT_TRUE(layoutProperty->GetFontSize().has_value());
    EXPECT_EQ(layoutProperty->GetFontSize().value(), Dimension(25.0));
    ASSERT_TRUE(layoutProperty->GetFontWeight().has_value());
    EXPECT_EQ(layoutProperty->GetFontWeight().value(), FontWeight::BOLD);
    ASSERT_TRUE(layoutProperty->GetFontColor().has_value());
    EXPECT_EQ(layoutProperty->GetFontColor().value(), Color::RED);
    ASSERT_TRUE(layoutProperty->GetItalicFontStyle().has_value());
    EXPECT_EQ(layoutProperty->GetItalicFontStyle().value(), Ace::FontStyle::ITALIC);
}

/**
 * @tc.name: MenuPatternTestNg007
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg007, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    MneuModelInstance.SetFontSize(Dimension(25.0));
    MneuModelInstance.SetFontColor(Color::RED);
    MneuModelInstance.SetFontWeight(FontWeight::BOLD);

    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    MenuItemProperties itemOption;
    itemOption.content = "content";
    itemOption.labelInfo = "label";
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->OnModifyDone();
    itemNode->MountToParent(menuNode);
    itemNode->OnMountToParentDone();

    // call UpdateMenuItemChildren
    menuPattern->OnModifyDone();

    auto contentNode = itemPattern->GetContentNode();
    ASSERT_NE(contentNode, nullptr);
    auto textProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);
    ASSERT_TRUE(textProperty->GetContent().has_value());
    EXPECT_EQ(textProperty->GetContent().value(), "content");
    ASSERT_TRUE(textProperty->GetFontSize().has_value());
    EXPECT_EQ(textProperty->GetFontSize().value(), Dimension(25.0));
    ASSERT_TRUE(textProperty->GetFontWeight().has_value());
    EXPECT_EQ(textProperty->GetFontWeight().value(), FontWeight::BOLD);
    ASSERT_TRUE(textProperty->GetTextColor().has_value());
    EXPECT_EQ(textProperty->GetTextColor().value(), Color::RED);

    auto labelNode = itemPattern->GetLabelNode();
    ASSERT_NE(labelNode, nullptr);
    auto labelProperty = labelNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(labelProperty, nullptr);
    ASSERT_TRUE(labelProperty->GetContent().has_value());
    EXPECT_EQ(labelProperty->GetContent().value(), "label");
    ASSERT_TRUE(labelProperty->GetFontSize().has_value());
    EXPECT_EQ(labelProperty->GetFontSize().value(), Dimension(25.0));
    ASSERT_TRUE(labelProperty->GetFontWeight().has_value());
    EXPECT_EQ(labelProperty->GetFontWeight().value(), FontWeight::BOLD);
    ASSERT_TRUE(labelProperty->GetTextColor().has_value());
    EXPECT_EQ(labelProperty->GetTextColor().value(), Color::RED);
}

/**
 * @tc.name: MenuPatternTestNg008
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg008, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    MneuModelInstance.SetFontSize(Dimension(25.0));
    MneuModelInstance.SetFontColor(Color::RED);
    MneuModelInstance.SetFontWeight(FontWeight::BOLD);

    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    MenuItemProperties itemOption;
    itemOption.content = "content";
    itemOption.labelInfo = "label";
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontSize(Dimension(35.0));
    MneuItemModelInstance.SetFontColor(Color::BLUE);
    MneuItemModelInstance.SetFontWeight(FontWeight::LIGHTER);
    MneuItemModelInstance.SetLabelFontSize(Dimension(40.0));
    MneuItemModelInstance.SetLabelFontColor(Color::GRAY);
    MneuItemModelInstance.SetLabelFontWeight(FontWeight::LIGHTER);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->OnModifyDone();
    itemNode->MountToParent(menuNode);
    itemNode->OnMountToParentDone();

    // call UpdateMenuItemChildren
    menuPattern->OnModifyDone();

    auto contentNode = itemPattern->GetContentNode();
    ASSERT_NE(contentNode, nullptr);
    auto textProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);
    ASSERT_TRUE(textProperty->GetContent().has_value());
    EXPECT_EQ(textProperty->GetContent().value(), "content");
    ASSERT_TRUE(textProperty->GetFontSize().has_value());
    EXPECT_EQ(textProperty->GetFontSize().value(), Dimension(35.0));
    ASSERT_TRUE(textProperty->GetFontWeight().has_value());
    EXPECT_EQ(textProperty->GetFontWeight().value(), FontWeight::LIGHTER);
    ASSERT_TRUE(textProperty->GetTextColor().has_value());
    EXPECT_EQ(textProperty->GetTextColor().value(), Color::BLUE);

    auto labelNode = itemPattern->GetLabelNode();
    ASSERT_NE(labelNode, nullptr);
    auto labelProperty = labelNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(labelProperty, nullptr);
    ASSERT_TRUE(labelProperty->GetContent().has_value());
    EXPECT_EQ(labelProperty->GetContent().value(), "label");
    ASSERT_TRUE(labelProperty->GetFontSize().has_value());
    EXPECT_EQ(labelProperty->GetFontSize().value(), Dimension(40.0));
    ASSERT_TRUE(labelProperty->GetFontWeight().has_value());
    EXPECT_EQ(labelProperty->GetFontWeight().value(), FontWeight::LIGHTER);
    ASSERT_TRUE(labelProperty->GetTextColor().has_value());
    EXPECT_EQ(labelProperty->GetTextColor().value(), Color::GRAY);
}

/**
 * @tc.name: MenuPatternTestNg009
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg009, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    MneuModelInstance.SetFontSize(Dimension(25.0));
    MneuModelInstance.SetFontColor(Color::RED);
    MneuModelInstance.SetFontWeight(FontWeight::BOLD);

    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    MenuItemGroupView::Create();
    auto groupNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    MenuItemGroupView::Create();
    auto groupNode2 = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(groupNode, nullptr);
    auto groupPattern = groupNode->GetPattern<MenuItemGroupPattern>();

    MenuItemProperties itemOption;
    itemOption.content = "content";
    itemOption.labelInfo = "label";
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->OnModifyDone();
    itemNode->MountToParent(groupNode);
    itemNode->OnMountToParentDone();
    groupPattern->OnModifyDone();
    groupNode->MountToParent(menuNode);
    groupNode->OnMountToParentDone();
    groupNode2->MountToParent(menuNode);
    groupNode2->OnMountToParentDone();

    // call UpdateMenuItemChildren
    menuPattern->OnModifyDone();

    auto contentNode = itemPattern->GetContentNode();
    ASSERT_NE(contentNode, nullptr);
    auto textProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);
    ASSERT_TRUE(textProperty->GetContent().has_value());
    EXPECT_EQ(textProperty->GetContent().value(), "content");
    ASSERT_TRUE(textProperty->GetFontSize().has_value());
    EXPECT_EQ(textProperty->GetFontSize().value(), Dimension(25.0));
    ASSERT_TRUE(textProperty->GetFontWeight().has_value());
    EXPECT_EQ(textProperty->GetFontWeight().value(), FontWeight::BOLD);
    ASSERT_TRUE(textProperty->GetTextColor().has_value());
    EXPECT_EQ(textProperty->GetTextColor().value(), Color::RED);

    auto labelNode = itemPattern->GetLabelNode();
    ASSERT_NE(labelNode, nullptr);
    auto labelProperty = labelNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(labelProperty, nullptr);
    ASSERT_TRUE(labelProperty->GetContent().has_value());
    EXPECT_EQ(labelProperty->GetContent().value(), "label");
    ASSERT_TRUE(labelProperty->GetFontSize().has_value());
    EXPECT_EQ(labelProperty->GetFontSize().value(), Dimension(25.0));
    ASSERT_TRUE(labelProperty->GetFontWeight().has_value());
    EXPECT_EQ(labelProperty->GetFontWeight().value(), FontWeight::BOLD);
    ASSERT_TRUE(labelProperty->GetTextColor().has_value());
    EXPECT_EQ(labelProperty->GetTextColor().value(), Color::RED);
}

/**
 * @tc.name: MenuPatternTestNg010
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg010, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    MenuItemGroupView::Create();
    auto groupNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(groupNode, nullptr);
    auto groupPattern = groupNode->GetPattern<MenuItemGroupPattern>();

    MenuItemProperties itemOption;
    itemOption.content = "content";
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->OnModifyDone();
    itemNode->MountToParent(groupNode);
    itemNode->OnMountToParentDone();
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto textPattern = textNode->GetPattern<TextPattern>();
    textPattern->OnModifyDone();
    textNode->MountToParent(groupNode);
    textNode->OnMountToParentDone();
    groupPattern->OnModifyDone();
    groupNode->MountToParent(menuNode);
    groupNode->OnMountToParentDone();

    // call UpdateMenuItemChildren
    menuPattern->OnModifyDone();

    auto contentNode = itemPattern->GetContentNode();
    ASSERT_NE(contentNode, nullptr);
    auto textProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);
    ASSERT_TRUE(textProperty->GetContent().has_value());
    EXPECT_EQ(textProperty->GetContent().value(), "content");
    ASSERT_TRUE(textProperty->GetFontSize().has_value());
    ASSERT_TRUE(textProperty->GetFontWeight().has_value());
    EXPECT_EQ(textProperty->GetFontWeight().value(), FontWeight::REGULAR);
    ASSERT_TRUE(textProperty->GetTextColor().has_value());

    auto labelNode = itemPattern->GetLabelNode();
    ASSERT_EQ(labelNode, nullptr);
}

/**
 * @tc.name: MenuPatternTestNg011
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg011, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    MneuModelInstance.SetFontSize(Dimension(25.0));
    MneuModelInstance.SetFontColor(Color::RED);
    MneuModelInstance.SetFontWeight(FontWeight::BOLD);
    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    MenuItemProperties itemOption;
    itemOption.content = "content";
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontStyle(Ace::FontStyle::ITALIC);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->OnModifyDone();
    itemNode->MountToParent(menuNode);
    itemNode->OnMountToParentDone();

    // call UpdateMenuItemChildren
    menuPattern->OnModifyDone();
    itemPattern->OnModifyDone();
    auto labelNode = itemPattern->GetLabelNode();
    ASSERT_EQ(labelNode, nullptr);
}

/**
 * @tc.name: MenuPatternTestNg012
 * @tc.desc: Verify UpdateSelectParam.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg012, TestSize.Level1)
{
    std::vector<SelectParam> params;
    params.emplace_back("content1", "icon1");
    params.emplace_back("content2", "");
    params.emplace_back("", "icon3");
    params.emplace_back("", "");
    auto wrapperNode = MenuView::Create(params, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(wrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(wrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);

    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    ASSERT_EQ(menuPattern->GetOptions().size(), 4);
    const auto& children = menuPattern->GetOptions();
    auto childIt = children.begin();
    for (size_t i = 0; i < children.size(); i++, childIt++) {
        const auto& childNode = AceType::DynamicCast<FrameNode>(*childIt);
        ASSERT_NE(childNode, nullptr);
        auto optionPattern = childNode->GetPattern<OptionPattern>();
        ASSERT_NE(optionPattern, nullptr);
        ASSERT_NE(optionPattern->text_, nullptr);
        auto textProps = optionPattern->text_->GetLayoutProperty<TextLayoutProperty>();
        ASSERT_NE(textProps, nullptr);
        auto param = params.at(i);
        EXPECT_EQ(textProps->GetContent().value_or(""), param.first);
        if (param.second.empty()) {
            ASSERT_EQ(optionPattern->icon_, nullptr);
        } else {
            ASSERT_NE(optionPattern->icon_, nullptr);
            auto imageProps = optionPattern->icon_->GetLayoutProperty<ImageLayoutProperty>();
            ASSERT_NE(imageProps, nullptr);
            auto imageSrcInfo = imageProps->GetImageSourceInfo();
            ASSERT_TRUE(imageSrcInfo.has_value());
            ASSERT_EQ(imageSrcInfo->GetSrc(), param.second);
        }
    }

    params.clear();
    menuPattern->UpdateSelectParam(params);
    ASSERT_EQ(menuPattern->GetOptions().size(), 0);
}

/**
 * @tc.name: MenuPatternTestNg013
 * @tc.desc: Verify UpdateSelectParam.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg013, TestSize.Level1)
{
    std::vector<SelectParam> params;
    auto wrapperNode = MenuView::Create(params, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(wrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(wrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    ASSERT_EQ(menuNode->GetChildren().size(), 1);

    params.emplace_back("content1", "icon1");
    params.emplace_back("content2", "");
    params.emplace_back("", "icon3");
    params.emplace_back("", "");
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    menuPattern->UpdateSelectParam(params);

    ASSERT_EQ(menuPattern->GetOptions().size(), 4);
    const auto& children = menuPattern->GetOptions();
    auto childIt = children.begin();
    for (size_t i = 0; i < children.size(); i++, childIt++) {
        const auto& childNode = AceType::DynamicCast<FrameNode>(*childIt);
        ASSERT_NE(childNode, nullptr);
        auto optionPattern = childNode->GetPattern<OptionPattern>();
        ASSERT_NE(optionPattern, nullptr);
        ASSERT_NE(optionPattern->text_, nullptr);
        auto textProps = optionPattern->text_->GetLayoutProperty<TextLayoutProperty>();
        ASSERT_NE(textProps, nullptr);
        auto param = params.at(i);
        EXPECT_EQ(textProps->GetContent().value_or(""), param.first);
        if (param.second.empty()) {
            ASSERT_EQ(optionPattern->icon_, nullptr);
        } else {
            ASSERT_NE(optionPattern->icon_, nullptr);
            auto imageProps = optionPattern->icon_->GetLayoutProperty<ImageLayoutProperty>();
            ASSERT_NE(imageProps, nullptr);
            auto imageSrcInfo = imageProps->GetImageSourceInfo();
            ASSERT_TRUE(imageSrcInfo.has_value());
            ASSERT_EQ(imageSrcInfo->GetSrc(), param.second);
        }
    }
}

/**
 * @tc.name: MenuPatternTestNg014
 * @tc.desc: Verify UpdateSelectParam.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg014, TestSize.Level1)
{
    std::vector<SelectParam> params;
    auto wrapperNode = MenuView::Create(params, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(wrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(wrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    ASSERT_EQ(menuPattern->GetOptions().size(), 0);

    menuPattern->UpdateSelectParam(params);
    ASSERT_EQ(menuPattern->GetOptions().size(), 0);
}

/**
 * @tc.name: MenuPatternTestNg015
 * @tc.desc: Verify UpdateSelectParam.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg015, TestSize.Level1)
{
    std::vector<SelectParam> params;
    params.emplace_back("content1", "icon1");
    params.emplace_back("content2", "");
    params.emplace_back("", "icon3");
    params.emplace_back("", "");
    auto wrapperNode = MenuView::Create(params, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(wrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(wrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);

    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    ASSERT_EQ(menuPattern->GetOptions().size(), 4);

    params.clear();
    params.emplace_back("content1_new", "");
    params.emplace_back("content2_new", "icon2_new");
    params.emplace_back("", "");
    params.emplace_back("", "icon4_new");
    menuPattern->UpdateSelectParam(params);
    ASSERT_EQ(menuPattern->GetOptions().size(), 4);

    const auto& children = menuPattern->GetOptions();
    auto childIt = children.begin();
    for (size_t i = 0; i < children.size(); i++, childIt++) {
        const auto& childNode = AceType::DynamicCast<FrameNode>(*childIt);
        ASSERT_NE(childNode, nullptr);
        auto optionPattern = childNode->GetPattern<OptionPattern>();
        ASSERT_NE(optionPattern, nullptr);
        ASSERT_NE(optionPattern->text_, nullptr);
        auto textProps = optionPattern->text_->GetLayoutProperty<TextLayoutProperty>();
        ASSERT_NE(textProps, nullptr);
        auto param = params.at(i);
        EXPECT_EQ(textProps->GetContent().value_or(""), param.first);
        if (param.second.empty()) {
            ASSERT_EQ(optionPattern->icon_, nullptr);
        } else {
            ASSERT_NE(optionPattern->icon_, nullptr);
            auto imageProps = optionPattern->icon_->GetLayoutProperty<ImageLayoutProperty>();
            ASSERT_NE(imageProps, nullptr);
            auto imageSrcInfo = imageProps->GetImageSourceInfo();
            ASSERT_TRUE(imageSrcInfo.has_value());
            ASSERT_EQ(imageSrcInfo->GetSrc(), param.second);
        }
    }
}

/**
 * @tc.name: MenuPatternTestNg016
 * @tc.desc: Verify UpdateSelectParam.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg016, TestSize.Level1)
{
    std::vector<SelectParam> params;
    params.emplace_back("content1", "icon1");
    params.emplace_back("content2", "");
    params.emplace_back("", "icon3");
    params.emplace_back("", "");
    auto wrapperNode = MenuView::Create(params, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(wrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(wrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);

    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    ASSERT_EQ(menuPattern->GetOptions().size(), 4);

    params.clear();
    params.emplace_back("content1_new", "");
    params.emplace_back("content2_new", "icon2_new");
    params.emplace_back("", "");
    menuPattern->UpdateSelectParam(params);
    ASSERT_EQ(menuPattern->GetOptions().size(), 3);

    const auto& children = menuPattern->GetOptions();
    auto childIt = children.begin();
    for (size_t i = 0; i < children.size(); i++, childIt++) {
        const auto& childNode = AceType::DynamicCast<FrameNode>(*childIt);
        ASSERT_NE(childNode, nullptr);
        auto optionPattern = childNode->GetPattern<OptionPattern>();
        ASSERT_NE(optionPattern, nullptr);
        ASSERT_NE(optionPattern->text_, nullptr);
        auto textProps = optionPattern->text_->GetLayoutProperty<TextLayoutProperty>();
        ASSERT_NE(textProps, nullptr);
        auto param = params.at(i);
        EXPECT_EQ(textProps->GetContent().value_or(""), param.first);
        if (param.second.empty()) {
            ASSERT_EQ(optionPattern->icon_, nullptr);
        } else {
            ASSERT_NE(optionPattern->icon_, nullptr);
            auto imageProps = optionPattern->icon_->GetLayoutProperty<ImageLayoutProperty>();
            ASSERT_NE(imageProps, nullptr);
            auto imageSrcInfo = imageProps->GetImageSourceInfo();
            ASSERT_TRUE(imageSrcInfo.has_value());
            ASSERT_EQ(imageSrcInfo->GetSrc(), param.second);
        }
    }
}

/**
 * @tc.name: MenuPatternTestNg017
 * @tc.desc: Verify UpdateSelectParam.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg017, TestSize.Level1)
{
    std::vector<SelectParam> params;
    params.emplace_back("content1", "icon1");
    params.emplace_back("content2", "");
    params.emplace_back("", "icon3");
    auto wrapperNode = MenuView::Create(params, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(wrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(wrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);

    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    ASSERT_EQ(menuPattern->GetOptions().size(), 3);

    params.clear();
    params.emplace_back("content1_new", "");
    params.emplace_back("content2_new", "icon2_new");
    params.emplace_back("", "");
    params.emplace_back("", "icon4_new");
    menuPattern->UpdateSelectParam(params);
    ASSERT_EQ(menuPattern->GetOptions().size(), 4);

    const auto& children = menuPattern->GetOptions();
    auto childIt = children.begin();
    for (size_t i = 0; i < children.size(); i++, childIt++) {
        const auto& childNode = AceType::DynamicCast<FrameNode>(*childIt);
        ASSERT_NE(childNode, nullptr);
        auto optionPattern = childNode->GetPattern<OptionPattern>();
        ASSERT_NE(optionPattern, nullptr);
        ASSERT_NE(optionPattern->text_, nullptr);
        auto textProps = optionPattern->text_->GetLayoutProperty<TextLayoutProperty>();
        ASSERT_NE(textProps, nullptr);
        auto param = params.at(i);
        EXPECT_EQ(textProps->GetContent().value_or(""), param.first);
        if (param.second.empty()) {
            ASSERT_EQ(optionPattern->icon_, nullptr);
        } else {
            ASSERT_NE(optionPattern->icon_, nullptr);
            auto imageProps = optionPattern->icon_->GetLayoutProperty<ImageLayoutProperty>();
            ASSERT_NE(imageProps, nullptr);
            auto imageSrcInfo = imageProps->GetImageSourceInfo();
            ASSERT_TRUE(imageSrcInfo.has_value());
            ASSERT_EQ(imageSrcInfo->GetSrc(), param.second);
        }
    }
}

/**
 * @tc.name: MenuPatternTestNg018
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg018, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MneuModelInstance.Create();
    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    ASSERT_EQ(menuPattern->GetOptions().size(), 0);
    std::vector<SelectParam> params;
    params.emplace_back("content1", "icon1");
    menuPattern->UpdateSelectParam(params);
    ASSERT_EQ(menuPattern->GetOptions().size(), 0);
}

/**
 * @tc.ame: MenuPatternTestNg019
 * @tc.desc: Test MultiMenu and outer Menu container.
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg019, TestSize.Level1)
{
    MenuModelNG model;
    model.Create();
    auto multiMenu = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    auto menuWrapper = MenuView::Create(multiMenu, -1, EMPTY_TEXT);
    ASSERT_NE(menuWrapper, nullptr);
    auto outerMenu = AceType::DynamicCast<FrameNode>(menuWrapper->GetFirstChild());
    ASSERT_NE(outerMenu, nullptr);

    // backgroundColor should be reset
    ASSERT_EQ(outerMenu->GetRenderContext()->GetBackgroundColorValue(), Color::TRANSPARENT);

    // padding should be moved to inner multi menu
    auto scroll = AceType::DynamicCast<FrameNode>(outerMenu->GetFirstChild());
    ASSERT_NE(scroll, nullptr);
    auto&& padding = scroll->GetLayoutProperty()->GetPaddingProperty();
    // should have empty padding
    ASSERT_EQ(padding->ToString(), PaddingProperty().ToString());

    multiMenu->GetPattern()->BeforeCreateLayoutWrapper();
    // inner multi menu should have backgroundColor and padding set up
    ASSERT_NE(multiMenu->GetLayoutProperty()->GetPaddingProperty()->ToString(), PaddingProperty().ToString());
    ASSERT_NE(multiMenu->GetRenderContext()->GetBackgroundColor(), std::nullopt);
    // inner menu should have no shadow
    ASSERT_EQ(multiMenu->GetRenderContext()->GetBackShadow(), ShadowConfig::NoneShadow);

    // MultiMenu should have its own layout algorithm
    auto layoutAlgorithm = multiMenu->GetPattern<MenuPattern>()->CreateLayoutAlgorithm();
    ASSERT_NE(AceType::DynamicCast<MultiMenuLayoutAlgorithm>(layoutAlgorithm), nullptr);
}

/**
 * @tc.name: MenuPatternTestNg020
 * @tc.desc: Verify UpdateMenuItemChildren.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create MenuModelNG and MenuItemModelNG object and set FontStyle properties of MenuModelNG.
     */
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    MneuModelInstance.SetFontStyle(Ace::FontStyle::ITALIC);

    /**
     * @tc.steps: step2. get the frameNode, MenuPattern and MenuLayoutProperty.
     * @tc.expected: step2. check whether the objects is available.
     */
    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    /**
     * @tc.steps: step3. not set FontStyle properties of MenuModelNG.
     */
    MenuItemProperties itemOption;
    itemOption.content = "content";
    itemOption.labelInfo = "label";
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->OnModifyDone();
    itemNode->MountToParent(menuNode);
    itemNode->OnMountToParentDone();

    /**
     * @tc.steps: step4. call OnModifyDone of MenuPattern to call UpdateMenuItemChildren
     */
    menuPattern->OnModifyDone();

    /**
     * @tc.steps: step5. get the FontStyle properties of menuItemLayoutProperty.
     * @tc.expected: step5. check whether the FontStyle properties is is correct.
     */
    auto contentNode = itemPattern->GetContentNode();
    ASSERT_NE(contentNode, nullptr);
    auto textProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);
    ASSERT_TRUE(textProperty->GetItalicFontStyle().has_value());
    EXPECT_EQ(textProperty->GetItalicFontStyle().value(), Ace::FontStyle::ITALIC);
}

/**
 * @tc.name: MenuPatternTest021
 * @tc.desc: Verify RegisterOnTouch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg021, TestSize.Level1)
{
    MenuPattern* menuPattern = new MenuPattern(TARGET_ID, "", TYPE);
    const std::string tag = "tag";
    int32_t nodeId = 1;
    RefPtr<Pattern> pattern = AceType::MakeRefPtr<Pattern>();
    bool isRoot = false;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<FrameNode> frameNode = AceType::MakeRefPtr<FrameNode>(tag, nodeId, pattern, isRoot);
    const RefPtr<LayoutWrapperNode> layoutWrapper;
    std::string type = "1";
    TouchEventInfo info(type);
    TouchType touchType = TouchType::UP;
    TouchLocationInfo locationInfo(TARGET_ID);
    Offset globalLocation(1, 1);
    locationInfo.SetTouchType(touchType);
    auto touchLocationInfo = locationInfo.SetGlobalLocation(globalLocation);
    info.touches_.emplace_back(touchLocationInfo);
    menuPattern->OnTouchEvent(info);
    menuPattern->RegisterOnTouch();
    KeyEvent keyEvent(KeyCode::KEY_ESCAPE, KeyAction::UP);
    EXPECT_FALSE(menuPattern->IsMultiMenu());
    EXPECT_FALSE(menuPattern->OnKeyEvent(keyEvent));
    menuPattern->HideMenu();
    menuPattern->HideSubMenu();
    EXPECT_FALSE(menuPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, configDirtySwap));
    menuPattern->type_ = MenuType::SUB_MENU;
    EXPECT_FALSE(menuPattern->OnKeyEvent(keyEvent));
    menuPattern->HideMenu();
    menuPattern->HideSubMenu();
    menuPattern->RemoveParentHoverStyle();
    delete menuPattern;
    menuPattern = nullptr;
}

/**
 * @tc.name: MenuPatternTestNg022
 * @tc.desc: Verify OnTouchEvent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. multi menu touch event
     * @tc.expected: menu type is multi
     */
    auto multiMenuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MULTI_MENU);
    TouchEventInfo multiMenuTouchEventInfo(MENU_TOUCH_EVENT_TYPE);
    multiMenuPattern->OnTouchEvent(multiMenuTouchEventInfo);
    EXPECT_TRUE(multiMenuPattern->IsMultiMenu());
    /**
     * @tc.steps: step2. select menu touch event
     * @tc.expected: menu options nums as expected
     */
    std::vector<SelectParam> selectParams;
    selectParams.emplace_back("content", "icon");
    auto selectWrapperNode = MenuView::Create(selectParams, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(selectWrapperNode, nullptr);
    auto selectMenuNode = AceType::DynamicCast<FrameNode>(selectWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(selectMenuNode, nullptr);
    TouchEventInfo selectMenuEventInfo(MENU_TOUCH_EVENT_TYPE);
    auto selectMenuPattern = selectMenuNode->GetPattern<MenuPattern>();
    ASSERT_NE(selectMenuPattern, nullptr);
    selectMenuPattern->OnTouchEvent(selectMenuEventInfo);
    ASSERT_EQ(selectMenuPattern->GetOptions().size(), 1);
    /**
     * @tc.steps: step3. contextMenu touch event
     * @tc.expected: menu type is context
     */
    auto contextMenuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", MenuType::CONTEXT_MENU);
    // excute touch down event
    TouchEventInfo contextMenuTouchDownEventInfo(MENU_TOUCH_EVENT_TYPE);
    TouchLocationInfo downLocationInfo(TARGET_ID);
    Offset touchDownGlobalLocation(1, 1);
    downLocationInfo.SetTouchType(TouchType::DOWN);
    auto touchDownLocationInfo = downLocationInfo.SetGlobalLocation(touchDownGlobalLocation);
    contextMenuTouchDownEventInfo.touches_.emplace_back(touchDownLocationInfo);
    contextMenuPattern->OnTouchEvent(contextMenuTouchDownEventInfo);
    // excute touch up event
    TouchEventInfo contextMenuTouchUpEventInfo(MENU_TOUCH_EVENT_TYPE);
    TouchLocationInfo upLocationInfo(TARGET_ID);
    Offset touchUpGlobalLocation(3, 3);
    upLocationInfo.SetTouchType(TouchType::UP);
    auto touchUpLocationInfo = upLocationInfo.SetGlobalLocation(touchUpGlobalLocation);
    contextMenuTouchUpEventInfo.touches_.emplace_back(touchUpLocationInfo);
    contextMenuPattern->OnTouchEvent(contextMenuTouchUpEventInfo);
    EXPECT_TRUE(contextMenuPattern->IsContextMenu());
}

/**
 * @tc.name: MenuPatternTestNg023
 * @tc.desc: Verify OnKeyEvent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu wrapper
     * @tc.expected: wrapper node is not null and has 1 child
     */
    std::vector<SelectParam> selectParams;
    selectParams.emplace_back(std::make_pair("MenuItem1", "Icon1"));
    auto menuWrapperNode = MenuView::Create(std::move(selectParams), 1, EMPTY_TEXT);
    ASSERT_NE(menuWrapperNode, nullptr);
    EXPECT_EQ(menuWrapperNode->GetChildren().size(), 1);
    /**
     * @tc.steps: step2. excute OnKeyEvent
     * @tc.expected: result as expected
     */
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    menuPattern->type_ = MenuType::SUB_MENU;
    KeyEvent escapeKeyEvent(KeyCode::KEY_ESCAPE, KeyAction::DOWN);
    EXPECT_TRUE(menuPattern->OnKeyEvent(escapeKeyEvent));
    KeyEvent upKeyEvent(KeyCode::KEY_PLUS, KeyAction::DOWN);
    EXPECT_FALSE(menuPattern->OnKeyEvent(upKeyEvent));
}

/**
 * @tc.name: MenuPatternTestNg024
 * @tc.desc: Verify RemoveParentHoverStyle.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuItem and menu
     * @tc.expected: menuItem not null
     */
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MENU);
    auto menuItemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, menuItemPattern);
    ASSERT_NE(menuItem, nullptr);
    /**
     * @tc.steps: step2. excute RemoveParentHoverStyle
     * @tc.expected: item IsSubMenuShowed as expected
     */
    menuPattern->SetParentMenuItem(menuItem);
    menuPattern->RemoveParentHoverStyle();
    auto itemPattern = menuItem->GetPattern<MenuItemPattern>();
    EXPECT_FALSE(itemPattern->IsSubMenuShowed());
    menuPattern->type_ = MenuType::SUB_MENU;
    menuPattern->RemoveParentHoverStyle();
    EXPECT_FALSE(itemPattern->IsSubMenuShowed());
}

/**
 * @tc.name: MenuPatternTestNg025
 * @tc.desc: Verify HideMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg025, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::CONTEXT_MENU);
    menuPattern->HideMenu(true);
    EXPECT_TRUE(menuPattern->IsContextMenu());
}

/**
 * @tc.name: MenuPatternTestNg026
 * @tc.desc: Verify HideSubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu wrapper
     * @tc.expected: wrapper node is not null and has 1 child
     */
    std::vector<SelectParam> selectParams;
    selectParams.emplace_back(std::make_pair("MenuItem1", "Icon1"));
    auto menuWrapperNode = MenuView::Create(std::move(selectParams), 1, EMPTY_TEXT);
    ASSERT_NE(menuWrapperNode, nullptr);
    EXPECT_EQ(menuWrapperNode->GetChildren().size(), 1);
    /**
     * @tc.steps: step2. excute HideSubMenu
     * @tc.expected: result as expected
     */
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    auto menuItemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, menuItemPattern);
    menuPattern->SetParentMenuItem(menuItem);
    menuPattern->type_ = MenuType::SUB_MENU;
    menuPattern->SetShowedSubMenu(menuNode);
    menuPattern->HideSubMenu();
    auto itemPattern = menuItem->GetPattern<MenuItemPattern>();
    EXPECT_FALSE(itemPattern->IsSubMenuShowed());
    ASSERT_EQ(menuPattern->GetShowedSubMenu(), nullptr);
}

/**
 * @tc.name: MenuPatternTestNg027
 * @tc.desc: Verify GetMenuWrapper,GetMainMenuPattern.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPatternTestNg027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu wrapper and menu
     * @tc.expected: wrapper node is not null and has 1 child
     */
    std::vector<SelectParam> selectParams;
    selectParams.emplace_back(std::make_pair("MenuItem1", "Icon1"));
    auto menuWrapperNode = MenuView::Create(std::move(selectParams), 1, EMPTY_TEXT);
    ASSERT_NE(menuWrapperNode, nullptr);
    EXPECT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    /**
     * @tc.steps: step2. create child to menu, excute GetMenuWrapper,GetMainMenuPattern,GetInnerMenuCount
     * @tc.expected: result as expected
     */
    auto multiMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, "", MenuType::MULTI_MENU));
    multiMenu->MountToParent(menuNode);
    auto multiPattern = multiMenu->GetPattern<MenuPattern>();
    ASSERT_NE(multiPattern, nullptr);
    ASSERT_NE(multiPattern->GetMenuWrapper(), nullptr);
    ASSERT_NE(multiPattern->GetMainMenuPattern(), nullptr);
    EXPECT_EQ(multiPattern->GetInnerMenuCount(), 0);
    /**
     * @tc.steps: step3. create cascade node
     * @tc.expected: wrapper node relate to child node pattern is null
     */
    auto singleMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, "", MenuType::MULTI_MENU));
    auto singleParentMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(2, "", MenuType::MENU));
    singleMenu->MountToParent(singleParentMenu);
    auto singleMenuPattern = singleMenu->GetPattern<MenuPattern>();
    ASSERT_EQ(singleMenuPattern->GetMenuWrapper(), nullptr);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg0
 * @tc.desc: Verify positionOffset of Layout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg0, TestSize.Level1)
{
    std::function<void()> action = [] {};
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "", action);
    optionParams.emplace_back("MenuItem2", "", action);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), 1, "", MenuType::MENU, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ASSERT_TRUE(property->GetPositionOffset().has_value());
    EXPECT_EQ(property->GetPositionOffset().value(), OffsetF());
    RefPtr<MenuLayoutAlgorithm> layoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode layoutWrapper(menuNode, geometryNode, menuNode->GetLayoutProperty());
    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->position_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->positionOffset_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->wrapperSize_, SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->isSelectMenu_ = true;
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(geometryNode->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg1
 * @tc.desc: Verify positionOffset of measure.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg1, TestSize.Level1)
{
    std::function<void()> action = [] {};
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "", action);
    optionParams.emplace_back("MenuItem2", "", action);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), 1, "", MenuType::CONTEXT_MENU, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ASSERT_TRUE(property->GetPositionOffset().has_value());
    EXPECT_EQ(property->GetPositionOffset().value(), OffsetF());
    RefPtr<MenuLayoutAlgorithm> layoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode layoutWrapper(menuNode, geometryNode, menuNode->GetLayoutProperty());
    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();
    layoutAlgorithm->GetPaintProperty(&layoutWrapper)->UpdateEnableArrow(true);
    RefPtr<LayoutWrapperNode> wrapperChild =
        AceType::MakeRefPtr<LayoutWrapperNode>(menuNode, geometryNode, menuNode->GetLayoutProperty());
    layoutWrapper.cachedList_.push_back(wrapperChild);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->position_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->positionOffset_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->wrapperSize_, SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    Placement placements[] = { Placement::TOP, Placement::BOTTOM, Placement::RIGHT, Placement::LEFT,
        Placement::TOP_LEFT, Placement::BOTTOM_LEFT, Placement::LEFT_BOTTOM, Placement::LEFT_TOP,
        Placement::RIGHT_BOTTOM, Placement::RIGHT_TOP, Placement::TOP_RIGHT, Placement::BOTTOM_RIGHT };
    for (Placement placementValue : placements) {
        layoutAlgorithm->arrowPlacement_ = placementValue;
        layoutAlgorithm->UpdatePropArrowOffset();
    }
    layoutAlgorithm->propArrowOffset_.value_ = 1.0;
    layoutAlgorithm->UpdatePropArrowOffset();
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->isSelectMenu_ = true;
    layoutAlgorithm->targetTag_ = EMPTY_TEXT;
    layoutAlgorithm->Layout(&layoutWrapper);
    layoutAlgorithm->LayoutArrow(&layoutWrapper);
    EXPECT_EQ(geometryNode->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: PerformActionTest001
 * @tc.desc: MenuItem Accessibility PerformAction test Select and ClearSelection.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, PerformActionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create menu, get menu frameNode and pattern, set callback function.
     * @tc.expected: FrameNode and pattern is not null, related function is called.
     */
    MenuItemProperties itemOption;
    itemOption.content = "content";
    MenuItemModelNG MneuItemModelInstance;
    MneuItemModelInstance.Create(itemOption);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto menuItemPattern = frameNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    auto menuItemEventHub = frameNode->GetEventHub<MenuItemEventHub>();
    ASSERT_NE(menuItemEventHub, nullptr);
    auto menuItemAccessibilityProperty = frameNode->GetAccessibilityProperty<MenuItemAccessibilityProperty>();
    ASSERT_NE(menuItemAccessibilityProperty, nullptr);
    menuItemPattern->SetAccessibilityAction();

    /**
     * @tc.steps: step2. When selectedChangeEvent onChange and subBuilder is null, call the callback function in
     *                   menuItemAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    EXPECT_TRUE(menuItemAccessibilityProperty->ActActionSelect());

    /**
     * @tc.steps: step3. When selectedChangeEvent onChange and subBuilder is not null, call the callback function in
     *                   menuItemAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    bool isSelected = false;
    auto changeEvent = [&isSelected](bool select) { isSelected = select; };
    menuItemEventHub->SetSelectedChangeEvent(changeEvent);
    menuItemEventHub->SetOnChange(changeEvent);
    auto subBuilder = []() {};
    menuItemPattern->SetSubBuilder(subBuilder);
    EXPECT_TRUE(menuItemAccessibilityProperty->ActActionSelect());
}

/**
 * @tc.name: PerformActionTest002
 * @tc.desc: Menu Accessibility PerformAction test ScrollForward and ScrollBackward.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, PerformActionTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create menu, get menu frameNode and pattern, set callback function.
     * @tc.expected: FrameNode and pattern is not null, related function is called.
     */
    MenuModelNG model;
    model.Create();
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto menuPattern = frameNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto menuAccessibilityProperty = frameNode->GetAccessibilityProperty<MenuAccessibilityProperty>();
    ASSERT_NE(menuAccessibilityProperty, nullptr);
    menuPattern->SetAccessibilityAction();

    /**
     * @tc.steps: step2. When firstChild is null, call the callback function in menuAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    EXPECT_TRUE(menuAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(menuAccessibilityProperty->ActActionScrollBackward());

    /**
     * @tc.steps: step3. When firstChild is not null and firstChild tag is SCROLL_ETS_TAG, call the callback function in
     *                   menuAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    auto scrollPattern = AceType::MakeRefPtr<ScrollPattern>();
    ASSERT_NE(scrollPattern, nullptr);
    scrollPattern->SetAxis(Axis::VERTICAL);
    scrollPattern->scrollableDistance_ = 1.0f;
    auto scroll =
        FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), scrollPattern);
    ASSERT_NE(scroll, nullptr);
    scroll->MountToParent(frameNode, 0);
    scroll->MarkModifyDone();
    EXPECT_TRUE(menuAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(menuAccessibilityProperty->ActActionScrollBackward());

    /**
     * @tc.steps: step4. When firstChild is not null and firstChild tag is not SCROLL_ETS_TAG, call the callback
     *                   function in menuAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    textNode->MountToParent(frameNode, 0);
    textNode->MarkModifyDone();
    EXPECT_TRUE(menuAccessibilityProperty->ActActionScrollForward());
    EXPECT_TRUE(menuAccessibilityProperty->ActActionScrollBackward());
}

/**
 * @tc.name: PerformActionTest003
 * @tc.desc: MenuItem Accessibility PerformAction test Select and ClearSelection.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, PerformActionTest003, TestSize.Level1)
{
    MenuItemProperties itemOption;
    itemOption.content = "content";
    MenuItemModelNG MneuItemModelInstance;
    MneuItemModelInstance.Create(itemOption);
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(frameNode, nullptr);
    auto menuItemPattern = frameNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    auto menuItemAccessibilityProperty = frameNode->GetAccessibilityProperty<MenuItemAccessibilityProperty>();
    ASSERT_NE(menuItemAccessibilityProperty, nullptr);
    bool isSelected = true;
    menuItemPattern->isSelected_ = false;
    menuItemPattern->MarkIsSelected(isSelected);
    EXPECT_TRUE(menuItemAccessibilityProperty->ActActionSelect());
}

/**
 * @tc.name: MenuAccessibilityEventTestNg001
 * @tc.desc: Test Click Event for Option of Menu.
 */
HWTEST_F(MenuTestNg, MenuAccessibilityEventTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Option for Menu.
     */
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::OPTION_ETS_TAG,
        ViewStackProcessor::GetInstance()->ClaimNodeId(), []() { return AceType::MakeRefPtr<OptionPattern>(0); });
    ASSERT_NE(frameNode, nullptr);
    auto optionPattern = frameNode->GetPattern<OptionPattern>();
    ASSERT_NE(optionPattern, nullptr);

    /**
     * @tc.steps: step2. set callback function.
     */
    int testIndex = SELECTED_INDEX;
    auto selectFunc = [optionPattern, testIndex](int index) { optionPattern->index_ = testIndex; };
    auto optionEventHub = frameNode->GetEventHub<OptionEventHub>();
    optionEventHub->SetOnSelect(selectFunc);
    optionPattern->RegisterOnClick();

    /**
     * @tc.steps: step3. call callback function.
     * @tc.expected: index_ is SELECTED_INDEX.
     */
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    auto clickEventActuator = gestureHub->clickEventActuator_;
    ASSERT_NE(clickEventActuator, nullptr);
    auto event = clickEventActuator->GetClickEvent();
    ASSERT_NE(event, nullptr);
    GestureEvent gestureEvent;
    event(gestureEvent);
    EXPECT_EQ(optionPattern->index_, SELECTED_INDEX);
}

/**
 * @tc.name: DesktopMenuPattern001
 * @tc.desc: Test MenuPattern onModifyDone, switch between DesktopMenu and regular menu.
 */
HWTEST_F(MenuTestNg, DesktopMenuPattern001, TestSize.Level1)
{
    MenuModelNG model;
    model.Create();
    auto menu1 = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menu1, nullptr);
    model.Create();
    auto menu2 = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    auto container = FrameNode::CreateFrameNode("", -1, AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MENU));
    auto mockScroll = FrameNode::CreateFrameNode("", -1, AceType::MakeRefPtr<Pattern>());
    mockScroll->MountToParent(container);
    menu1->MountToParent(mockScroll);
    menu2->MountToParent(mockScroll);

    auto pattern1 = menu1->GetPattern<InnerMenuPattern>();
    auto pattern2 = menu2->GetPattern<InnerMenuPattern>();
    auto containerPattern = container->GetPattern<MenuPattern>();
    containerPattern->OnModifyDone();
    pattern1->OnModifyDone();
    pattern2->OnModifyDone();
    pattern1->BeforeCreateLayoutWrapper();
    pattern2->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern1->type_, MenuType::DESKTOP_MENU);
    EXPECT_EQ(pattern2->type_, MenuType::DESKTOP_MENU);
    EXPECT_EQ(container->GetRenderContext()->GetBackShadow(), ShadowConfig::NoneShadow);

    mockScroll->RemoveChildAtIndex(1);
    pattern1->OnModifyDone();
    pattern1->BeforeCreateLayoutWrapper();
    containerPattern->OnModifyDone();
    EXPECT_EQ(pattern1->type_, MenuType::MULTI_MENU);
    EXPECT_EQ(container->GetRenderContext()->GetBackShadow(), ShadowConfig::DefaultShadowM);
}

/**
 * @tc.name: MenuAccessibilityPropertyIsScrollable001
 * @tc.desc: Test IsScrollable of menuAccessibilityProperty.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuAccessibilityPropertyIsScrollable001, TestSize.Level1)
{
    InitMenuTestNg();

    EXPECT_FALSE(menuAccessibilityProperty_->IsScrollable());

    auto scrollPattern = AceType::MakeRefPtr<ScrollPattern>();
    ASSERT_NE(scrollPattern, nullptr);
    scrollPattern->SetAxis(Axis::VERTICAL);
    scrollPattern->scrollableDistance_ = 1.0f;
    auto scroll =
        FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), scrollPattern);
    ASSERT_NE(scroll, nullptr);
    scroll->MountToParent(menuFrameNode_, 0);
    scroll->MarkModifyDone();
    EXPECT_TRUE(menuAccessibilityProperty_->IsScrollable());
}

/**
 * @tc.name: MenuAccessibilityPropertyGetSupportAction001
 * @tc.desc: Test GetSupportAction of menuAccessibilityProperty.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuAccessibilityPropertyGetSupportAction001, TestSize.Level1)
{
    InitMenuTestNg();

    auto scrollPattern = AceType::MakeRefPtr<ScrollPattern>();
    ASSERT_NE(scrollPattern, nullptr);
    scrollPattern->SetAxis(Axis::VERTICAL);
    scrollPattern->scrollableDistance_ = 1.0f;
    scrollPattern->currentOffset_ = CURRENT_OFFSET;
    auto scroll =
        FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), scrollPattern);
    ASSERT_NE(scroll, nullptr);
    scroll->MountToParent(menuFrameNode_, 0);

    menuAccessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = menuAccessibilityProperty_->GetSupportAction();
    uint64_t actions = 0, expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, expectActions);
}

/**
 * @tc.name: MenuItemAccessibilityPropertyGetText001
 * @tc.desc: Test GetText of menuItem.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemAccessibilityPropertyGetText001, TestSize.Level1)
{
    InitMenuItemTestNg();

    EXPECT_EQ(menuItemAccessibilityProperty_->GetText(), EMPTY_TEXT);

    auto menuItemLayoutProperty = menuItemFrameNode_->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(menuItemLayoutProperty, nullptr);
    menuItemLayoutProperty->UpdateContent(MENU_ITEM_TEXT);

    EXPECT_EQ(menuItemAccessibilityProperty_->GetText(), MENU_ITEM_TEXT);
}

/**
 * @tc.name: MenuItemAccessibilityPropertyIsSelected001
 * @tc.desc: Test IsSelected of menuitem.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemAccessibilityPropertyIsSelected001, TestSize.Level1)
{
    InitMenuItemTestNg();

    EXPECT_FALSE(menuItemAccessibilityProperty_->IsSelected());

    menuItemPattern_->SetSelected(true);
    EXPECT_TRUE(menuItemAccessibilityProperty_->IsSelected());
}

/**
 * @tc.name: MenuItemAccessibilityPropertyGetSupportAction001
 * @tc.desc: Test GetSupportAction of menuitem.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemAccessibilityPropertyGetSupportAction001, TestSize.Level1)
{
    InitMenuItemTestNg();

    menuItemAccessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = menuItemAccessibilityProperty_->GetSupportAction();
    uint64_t actions = 0, expectActions = 0;
    expectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, expectActions);
}

/**
 * @tc.name: MenuItemGroupAccessibilityPropertyGetText001
 * @tc.desc: Test GetText of menuItemGroup.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemGroupAccessibilityPropertyGetText001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG,
        ViewStackProcessor::GetInstance()->ClaimNodeId(), []() { return AceType::MakeRefPtr<MenuItemGroupPattern>(); });
    ASSERT_NE(frameNode, nullptr);

    auto menuItemGroupPattern = frameNode->GetPattern<MenuItemGroupPattern>();
    ASSERT_NE(menuItemGroupPattern, nullptr);

    auto menuItemGroupAccessibilityProperty = frameNode->GetAccessibilityProperty<MenuItemGroupAccessibilityProperty>();
    ASSERT_NE(menuItemGroupAccessibilityProperty, nullptr);
    EXPECT_EQ(menuItemGroupAccessibilityProperty->GetText(), EMPTY_TEXT);

    auto content = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(content, nullptr);
    menuItemGroupPattern->AddHeaderContent(content);

    auto textLayoutProperty = content->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);
    textLayoutProperty->UpdateContent(MENU_ITEM_GROUP_TEXT);
    EXPECT_EQ(menuItemGroupAccessibilityProperty->GetText(), MENU_ITEM_GROUP_TEXT);
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg001
 * @tc.desc: Verify GetStartIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg001, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetStartIcon().has_value());
    property.UpdateStartIcon("test.png");
    ASSERT_TRUE(property.GetStartIcon().has_value());
    EXPECT_EQ(property.GetStartIcon().value(), "test.png");
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg002
 * @tc.desc: Verify GetContent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg002, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetContent().has_value());
    property.UpdateContent("content");
    ASSERT_TRUE(property.GetContent().has_value());
    EXPECT_EQ(property.GetContent().value(), "content");
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg003
 * @tc.desc: Verify GetEndIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg003, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetEndIcon().has_value());
    property.UpdateEndIcon("test.png");
    ASSERT_TRUE(property.GetEndIcon().has_value());
    EXPECT_EQ(property.GetEndIcon().value(), "test.png");
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg004
 * @tc.desc: Verify GetLabel.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg004, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetLabel().has_value());
    property.UpdateLabel("label");
    ASSERT_TRUE(property.GetLabel().has_value());
    EXPECT_EQ(property.GetLabel().value(), "label");
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg005
 * @tc.desc: Verify GetSelectIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg005, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetSelectIcon().has_value());
    property.UpdateSelectIcon(true);
    ASSERT_TRUE(property.GetSelectIcon().has_value());
    EXPECT_TRUE(property.GetSelectIcon().value());
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg006
 * @tc.desc: Verify GetSelectIconSrc.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg006, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetSelectIconSrc().has_value());
    property.UpdateSelectIconSrc("test.png");
    ASSERT_TRUE(property.GetSelectIconSrc().has_value());
    EXPECT_EQ(property.GetSelectIconSrc().value(), "test.png");
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg007
 * @tc.desc: Verify GetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg007, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetFontSize().has_value());
    property.UpdateFontSize(Dimension(25.0f));
    ASSERT_TRUE(property.GetFontSize().has_value());
    EXPECT_EQ(property.GetFontSize().value(), Dimension(25.0f));
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg008
 * @tc.desc: Verify GetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg008, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetFontColor().has_value());
    property.UpdateFontColor(Color::RED);
    ASSERT_TRUE(property.GetFontColor().has_value());
    EXPECT_EQ(property.GetFontColor().value(), Color::RED);
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg009
 * @tc.desc: Verify GetFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg009, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetFontWeight().has_value());
    property.UpdateFontWeight(FontWeight::BOLD);
    ASSERT_TRUE(property.GetFontWeight().has_value());
    EXPECT_EQ(property.GetFontWeight().value(), FontWeight::BOLD);
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg0010
 * @tc.desc: Verify GetLabelFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg0010, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetLabelFontSize().has_value());
    property.UpdateLabelFontSize(Dimension(25.0f));
    ASSERT_TRUE(property.GetLabelFontSize().has_value());
    EXPECT_EQ(property.GetLabelFontSize().value(), Dimension(25.0f));
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg011
 * @tc.desc: Verify GetLabelFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg011, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetLabelFontColor().has_value());
    property.UpdateLabelFontColor(Color::RED);
    ASSERT_TRUE(property.GetLabelFontColor().has_value());
    EXPECT_EQ(property.GetLabelFontColor().value(), Color::RED);
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg012
 * @tc.desc: Verify GetLabelFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg012, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    EXPECT_FALSE(property.GetLabelFontWeight().has_value());
    property.UpdateLabelFontWeight(FontWeight::BOLD);
    ASSERT_TRUE(property.GetLabelFontWeight().has_value());
    EXPECT_EQ(property.GetLabelFontWeight().value(), FontWeight::BOLD);
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg013
 * @tc.desc: Verify Reset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg013, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    property.UpdateStartIcon("test.png");
    property.UpdateContent("content");
    property.UpdateEndIcon("test.png");
    property.UpdateLabel("label");
    property.UpdateSelectIcon(true);
    property.UpdateSelectIconSrc("test.png");
    property.UpdateFontSize(Dimension(25.0f));
    property.UpdateFontColor(Color::RED);
    property.UpdateFontWeight(FontWeight::BOLD);
    property.UpdateLabelFontSize(Dimension(25.0f));
    property.UpdateLabelFontColor(Color::RED);
    property.UpdateLabelFontWeight(FontWeight::BOLD);
    EXPECT_TRUE(property.GetStartIcon().has_value());
    EXPECT_TRUE(property.GetContent().has_value());
    EXPECT_TRUE(property.GetLabel().has_value());
    EXPECT_TRUE(property.GetEndIcon().has_value());
    EXPECT_TRUE(property.GetSelectIcon().has_value());
    EXPECT_TRUE(property.GetSelectIconSrc().has_value());
    EXPECT_TRUE(property.GetFontSize().has_value());
    EXPECT_TRUE(property.GetFontColor().has_value());
    EXPECT_TRUE(property.GetFontWeight().has_value());
    EXPECT_TRUE(property.GetLabelFontSize().has_value());
    EXPECT_TRUE(property.GetLabelFontColor().has_value());
    EXPECT_TRUE(property.GetLabelFontWeight().has_value());
    property.Reset();
    EXPECT_FALSE(property.GetStartIcon().has_value());
    EXPECT_FALSE(property.GetContent().has_value());
    EXPECT_FALSE(property.GetLabel().has_value());
    EXPECT_FALSE(property.GetEndIcon().has_value());
    EXPECT_FALSE(property.GetSelectIcon().has_value());
    EXPECT_FALSE(property.GetSelectIconSrc().has_value());
    EXPECT_FALSE(property.GetFontSize().has_value());
    EXPECT_FALSE(property.GetFontColor().has_value());
    EXPECT_FALSE(property.GetFontWeight().has_value());
    EXPECT_FALSE(property.GetLabelFontSize().has_value());
    EXPECT_FALSE(property.GetLabelFontColor().has_value());
    EXPECT_FALSE(property.GetLabelFontWeight().has_value());
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg014
 * @tc.desc: Verify Clone.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg014, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    property.UpdateStartIcon("start.png");
    property.UpdateContent("content");
    property.UpdateEndIcon("end.png");
    property.UpdateLabel("label");
    property.UpdateSelectIcon(true);
    property.UpdateSelectIconSrc("select.png");
    property.UpdateFontSize(Dimension(25.0f));
    property.UpdateFontColor(Color::RED);
    property.UpdateFontWeight(FontWeight::BOLD);
    property.UpdateLabelFontSize(Dimension(35.0f));
    property.UpdateLabelFontColor(Color::BLUE);
    property.UpdateLabelFontWeight(FontWeight::LIGHTER);

    auto cloneProperty = AceType::DynamicCast<MenuItemLayoutProperty>(property.Clone());
    ASSERT_NE(cloneProperty, nullptr);
    EXPECT_EQ(property.GetStartIcon().value(), cloneProperty->GetStartIcon().value());
    EXPECT_EQ(property.GetContent().value(), cloneProperty->GetContent().value());
    EXPECT_EQ(property.GetEndIcon().value(), cloneProperty->GetEndIcon().value());
    EXPECT_EQ(property.GetLabel().value(), cloneProperty->GetLabel().value());
    EXPECT_EQ(property.GetSelectIcon().value(), cloneProperty->GetSelectIcon().value());
    EXPECT_EQ(property.GetSelectIconSrc().value(), cloneProperty->GetSelectIconSrc().value());
    EXPECT_EQ(property.GetFontSize().value(), cloneProperty->GetFontSize().value());
    EXPECT_EQ(property.GetFontColor().value(), cloneProperty->GetFontColor().value());
    EXPECT_EQ(property.GetFontWeight().value(), cloneProperty->GetFontWeight().value());
    EXPECT_EQ(property.GetLabelFontSize().value(), cloneProperty->GetLabelFontSize().value());
    EXPECT_EQ(property.GetLabelFontColor().value(), cloneProperty->GetLabelFontColor().value());
    EXPECT_EQ(property.GetLabelFontWeight().value(), cloneProperty->GetLabelFontWeight().value());
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg015
 * @tc.desc: Verify ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg015, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    property.UpdateStartIcon("start.png");
    property.UpdateContent("content");
    property.UpdateEndIcon("end.png");
    property.UpdateLabel("label");
    property.UpdateSelectIcon(true);
    property.UpdateSelectIconSrc("select.png");
    property.UpdateFontSize(Dimension(25.0f));
    property.UpdateFontColor(Color::RED);
    property.UpdateFontWeight(FontWeight::BOLD);
    property.UpdateLabelFontSize(Dimension(35.0f));
    property.UpdateLabelFontColor(Color::BLUE);
    property.UpdateLabelFontWeight(FontWeight::LIGHTER);

    auto json = JsonUtil::Create(true);
    property.ToJsonValue(json);
    auto labelFontJson = json->GetObject("labelFont");
    auto contentFontJson = json->GetObject("contentFont");
    EXPECT_EQ(json->GetString("startIcon"), "start.png");
    EXPECT_EQ(json->GetString("content"), "content");
    EXPECT_EQ(json->GetString("endIcon"), "end.png");
    EXPECT_EQ(json->GetString("labelInfo"), "label");
    EXPECT_EQ(json->GetString("selectIcon"), "select.png");
    EXPECT_EQ(contentFontJson->GetString("size"), Dimension(25.0f).ToString());
    EXPECT_EQ(json->GetString("contentFontColor"), Color::RED.ColorToString());
    EXPECT_EQ(contentFontJson->GetString("weight"), V2::ConvertWrapFontWeightToStirng(FontWeight::BOLD));
    EXPECT_EQ(labelFontJson->GetString("size"), Dimension(35.0f).ToString());
    EXPECT_EQ(json->GetString("labelFontColor"), Color::BLUE.ColorToString());
    EXPECT_EQ(labelFontJson->GetString("weight"), V2::ConvertWrapFontWeightToStirng(FontWeight::LIGHTER));
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg016
 * @tc.desc: Verify ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg016, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    property.UpdateSelectIcon(false);
    property.UpdateSelectIconSrc("select.png");

    auto json = JsonUtil::Create(true);
    property.ToJsonValue(json);
    EXPECT_EQ(json->GetString("selectIcon"), "false");
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg017
 * @tc.desc: Verify ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg017, TestSize.Level1)
{
    MenuItemLayoutProperty property;
    property.UpdateSelectIcon(true);

    auto json = JsonUtil::Create(true);
    property.ToJsonValue(json);
    EXPECT_EQ(json->GetString("selectIcon"), "true");
}

/**
 * @tc.name: MenuItemLayoutPropertyTestNg018
 * @tc.desc: Verify ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemLayoutPropertyTestNg018, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetSelectIcon(true);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    auto json = JsonUtil::Create(true);
    itemProperty->ToJsonValue(json);
    EXPECT_EQ(json->GetString("selectIcon"), "true");
}

/**
 * @tc.name: MenuItemPatternTestNgAddSelectIcon001
 * @tc.desc: Verify AddSelectIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgAddSelectIcon001, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    MenuItemProperties itemOption;
    MenuItemModelInstance.Create(itemOption);
    MenuItemModelInstance.SetSelectIcon(false);
    MenuItemModelInstance.SetSelectIconSrc("selectIcon.png");
    MenuItemModelInstance.SetSelected(true);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    EXPECT_EQ(leftRow->GetChildren().size(), 0u);
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));
    EXPECT_EQ(rightRow->GetChildren().size(), 0u);

    // call AddSelectIcon
    itemPattern->OnModifyDone();

    EXPECT_EQ(leftRow->GetChildren().size(), 0u);
    EXPECT_EQ(rightRow->GetChildren().size(), 0u);
}

/**
 * @tc.name: MenuItemPatternTestNgAddSelectIcon002
 * @tc.desc: Verify AddSelectIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgAddSelectIcon002, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
    MenuItemProperties itemOption;
    MenuItemModelInstance.Create(itemOption);
    MenuItemModelInstance.SetSelectIcon(true);
    MenuItemModelInstance.SetSelectIconSrc("");
    MenuItemModelInstance.SetSelected(true);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    EXPECT_EQ(leftRow->GetChildren().size(), 0u);
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));
    EXPECT_EQ(rightRow->GetChildren().size(), 0u);

    EXPECT_CALL(*themeManager, GetTheme(_))
        .WillOnce(Return(AceType::MakeRefPtr<IconTheme>()))
        .WillOnce(Return(AceType::MakeRefPtr<SelectTheme>()));
    // call AddSelectIcon
    itemPattern->OnModifyDone();

    EXPECT_EQ(rightRow->GetChildren().size(), 0u);
    ASSERT_EQ(leftRow->GetChildren().size(), 1u);
    auto selectIconNode = AceType::DynamicCast<FrameNode>(leftRow->GetChildAtIndex(0));
    ASSERT_NE(selectIconNode, nullptr);
    EXPECT_EQ(selectIconNode->GetTag(), V2::IMAGE_ETS_TAG);
    auto imagePattern = selectIconNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    auto imageLayoutProperty = selectIconNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_NE(imageLayoutProperty, nullptr);
    auto sourceInfo = imageLayoutProperty->GetImageSourceInfo();
    EXPECT_TRUE(sourceInfo.has_value());
}

/**
 * @tc.name: MenuItemPatternTestNgAddSelectIcon003
 * @tc.desc: Verify AddSelectIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgAddSelectIcon003, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
    MenuItemProperties itemOption;
    MenuItemModelInstance.Create(itemOption);
    MenuItemModelInstance.SetSelectIcon(true);
    MenuItemModelInstance.SetSelectIconSrc("selectIcon.png");
    MenuItemModelInstance.SetSelected(false);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    EXPECT_EQ(leftRow->GetChildren().size(), 0u);
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));
    EXPECT_EQ(rightRow->GetChildren().size(), 0u);

    EXPECT_CALL(*themeManager, GetTheme(_))
        .WillOnce(Return(AceType::MakeRefPtr<IconTheme>()))
        .WillOnce(Return(AceType::MakeRefPtr<SelectTheme>()));
    // call AddSelectIcon
    itemPattern->OnModifyDone();

    EXPECT_EQ(rightRow->GetChildren().size(), 0u);
    ASSERT_EQ(leftRow->GetChildren().size(), 1u);
    auto selectIconNode = AceType::DynamicCast<FrameNode>(leftRow->GetChildAtIndex(0));
    ASSERT_NE(selectIconNode, nullptr);
    EXPECT_EQ(selectIconNode->GetTag(), V2::IMAGE_ETS_TAG);
    auto imagePattern = selectIconNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    auto imageLayoutProperty = selectIconNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_NE(imageLayoutProperty, nullptr);
    auto sourceInfo = imageLayoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo.has_value());
    EXPECT_EQ(sourceInfo.value().GetSrc(), "selectIcon.png");
}

/**
 * @tc.name: MenuItemPatternTestNgAddSelectIcon004
 * @tc.desc: Verify AddSelectIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgAddSelectIcon004, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
    MenuItemProperties itemOption;
    MenuItemModelInstance.Create(itemOption);
    MenuItemModelInstance.SetSelectIcon(true);
    MenuItemModelInstance.SetSelectIconSrc("selectIcon.png");
    MenuItemModelInstance.SetSelected(false);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    EXPECT_EQ(leftRow->GetChildren().size(), 0u);
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));
    EXPECT_EQ(rightRow->GetChildren().size(), 0u);
    EXPECT_CALL(*themeManager, GetTheme(_))
        .WillOnce(Return(AceType::MakeRefPtr<IconTheme>()))
        .WillOnce(Return(AceType::MakeRefPtr<SelectTheme>()));
    // call AddSelectIcon
    itemPattern->OnModifyDone();

    EXPECT_EQ(rightRow->GetChildren().size(), 0u);
    ASSERT_EQ(leftRow->GetChildren().size(), 1u);
    auto selectIconNode = AceType::DynamicCast<FrameNode>(leftRow->GetChildAtIndex(0));
    ASSERT_NE(selectIconNode, nullptr);
    EXPECT_EQ(selectIconNode->GetTag(), V2::IMAGE_ETS_TAG);

    itemProperty->UpdateSelectIcon(false);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
    // call AddSelectIcon
    itemPattern->OnModifyDone();
    ASSERT_EQ(leftRow->GetChildren().size(), 0u);
}

/**
 * @tc.name: MenuItemPatternTestNgUpdateIcon001
 * @tc.desc: Verify UpdateIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgUpdateIcon001, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    MenuItemProperties itemOption;
    itemOption.startIcon = "startIcon.png";
    MenuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));

    // call UpdateIcon
    itemPattern->OnModifyDone();

    EXPECT_EQ(rightRow->GetChildren().size(), 0u);
    ASSERT_EQ(leftRow->GetChildren().size(), 1u);
    auto startIconNode = AceType::DynamicCast<FrameNode>(leftRow->GetChildAtIndex(0));
    ASSERT_NE(startIconNode, nullptr);
    EXPECT_EQ(startIconNode->GetTag(), V2::IMAGE_ETS_TAG);
    auto imagePattern = startIconNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    auto imageLayoutProperty = startIconNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_NE(imageLayoutProperty, nullptr);
    auto sourceInfo = imageLayoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo.has_value());
    EXPECT_EQ(sourceInfo.value().GetSrc(), "startIcon.png");
}

/**
 * @tc.name: MenuItemPatternTestNgUpdateIcon002
 * @tc.desc: Verify UpdateIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgUpdateIcon002, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    MenuItemProperties itemOption;
    itemOption.endIcon = "endIcon.png";
    MenuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));

    // call UpdateIcon
    itemPattern->OnModifyDone();

    EXPECT_EQ(leftRow->GetChildren().size(), 0u);
    ASSERT_EQ(rightRow->GetChildren().size(), 1u);
    auto endIconNode = AceType::DynamicCast<FrameNode>(rightRow->GetChildAtIndex(0));
    ASSERT_NE(endIconNode, nullptr);
    EXPECT_EQ(endIconNode->GetTag(), V2::IMAGE_ETS_TAG);
    auto imagePattern = endIconNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    auto imageLayoutProperty = endIconNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_NE(imageLayoutProperty, nullptr);
    auto sourceInfo = imageLayoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo.has_value());
    EXPECT_EQ(sourceInfo.value().GetSrc(), "endIcon.png");

    itemProperty->UpdateEndIcon("endIcon2.png");
    // call UpdateIcon
    itemPattern->OnModifyDone();
    imagePattern = endIconNode->GetPattern<ImagePattern>();
    ASSERT_NE(imagePattern, nullptr);
    imageLayoutProperty = endIconNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_NE(imageLayoutProperty, nullptr);
    sourceInfo = imageLayoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo.has_value());
    EXPECT_EQ(sourceInfo.value().GetSrc(), "endIcon2.png");
}

/**
 * @tc.name: MenuItemPatternTestNgUpdateText001
 * @tc.desc: Verify UpdateText.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgUpdateText001, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    MenuItemProperties itemOption;
    itemOption.content = "content";
    MenuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));

    // call UpdateText
    itemPattern->OnModifyDone();

    EXPECT_EQ(rightRow->GetChildren().size(), 0u);
    ASSERT_EQ(leftRow->GetChildren().size(), 1u);
    auto contentNode = AceType::DynamicCast<FrameNode>(leftRow->GetChildAtIndex(0));
    ASSERT_NE(contentNode, nullptr);
    EXPECT_EQ(contentNode->GetTag(), V2::TEXT_ETS_TAG);
    auto textLayoutProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);
    auto content = textLayoutProperty->GetContent();
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "content");
}

/**
 * @tc.name: MenuItemPatternTestNgUpdateText002
 * @tc.desc: Verify UpdateText.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgUpdateText002, TestSize.Level1)
{
    MenuItemModelNG MenuItemModelInstance;
    MenuItemProperties itemOption;
    itemOption.labelInfo = "label";
    MenuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));

    // call UpdateText
    itemPattern->OnModifyDone();

    EXPECT_EQ(leftRow->GetChildren().size(), 0u);
    ASSERT_EQ(rightRow->GetChildren().size(), 1u);
    auto labelNode = AceType::DynamicCast<FrameNode>(rightRow->GetChildAtIndex(0));
    ASSERT_NE(labelNode, nullptr);
    EXPECT_EQ(labelNode->GetTag(), V2::TEXT_ETS_TAG);
    auto textLayoutProperty = labelNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);
    auto content = textLayoutProperty->GetContent();
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "label");
}

/**
 * @tc.name: MenuItemPatternTestNgUpdateText003
 * @tc.desc: Verify update text when item is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNgUpdateText003, TestSize.Level1)
{
    // mock theme
    auto selectTheme = MockPipelineBase::GetCurrent()->GetTheme<SelectTheme>();
    selectTheme->SetDisabledMenuFontColor(ITEM_DISABLED_COLOR);

    // create menu item
    MenuItemModelNG MenuItemModelInstance;
    MenuItemProperties itemOption;
    itemOption.startIcon = "startIcon.png";
    itemOption.content = "item content";
    MenuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemEventHub = itemNode->GetEventHub<MenuItemEventHub>();
    ASSERT_NE(itemEventHub, nullptr);
    itemEventHub->SetEnabled(false);
    // update item
    itemPattern->OnModifyDone();

    EXPECT_FALSE(itemEventHub->IsEnabled());
    ASSERT_EQ(itemNode->GetChildren().size(), 2u);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    EXPECT_EQ(leftRow->GetChildren().size(), 2u);
    auto startIconNode = AceType::DynamicCast<FrameNode>(leftRow->GetChildAtIndex(0));
    ASSERT_NE(startIconNode, nullptr);
    EXPECT_EQ(startIconNode->GetTag(), V2::IMAGE_ETS_TAG);
    auto imageLayoutProperty = startIconNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_NE(imageLayoutProperty, nullptr);
    auto sourceInfo = imageLayoutProperty->GetImageSourceInfo();
    ASSERT_TRUE(sourceInfo.has_value());
    EXPECT_EQ(sourceInfo.value().GetSrc(), "startIcon.png");

    auto contentNode = AceType::DynamicCast<FrameNode>(leftRow->GetChildAtIndex(1));
    ASSERT_NE(contentNode, nullptr);
    EXPECT_EQ(contentNode->GetTag(), V2::TEXT_ETS_TAG);
    auto textLayoutProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);
    auto content = textLayoutProperty->GetContent();
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "item content");
    auto textRenderContext = contentNode->GetRenderContext();
    EXPECT_EQ(textRenderContext->GetForegroundColor(), ITEM_DISABLED_COLOR);
}

/**
 * @tc.name: MenuItemPatternTestEvent001
 * @tc.desc: Test Click Event
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestEvent001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create menuItem.
     */
    MenuItemModelNG MenuItemModelInstance;
    MenuItemProperties itemOption;
    MenuItemModelInstance.Create(itemOption);
    MenuItemModelInstance.SetSelected(true);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);

    /**
     * @tc.steps: step2. set callback function.
     */
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto gestureHub = itemNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    auto clickEventActuator = gestureHub->clickEventActuator_;
    ASSERT_NE(clickEventActuator, nullptr);
    auto event = clickEventActuator->GetClickEvent();
    ASSERT_NE(event, nullptr);

    /**
     * @tc.steps: step3. call callback function.
     * @tc.expected: isSelected_ is false.
     */
    GestureEvent gestureEvent;
    event(gestureEvent);
    EXPECT_FALSE(itemPattern->isSelected_);
}

/**
 * @tc.name: CustomMenuItemPattern001
 * @tc.desc: Test CustomMenuItem creation
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, CustomMenuItemPattern001, TestSize.Level1)
{
    MenuItemModelNG model;
    auto customNode = FrameNode::CreateFrameNode("", -1, AceType::MakeRefPtr<Pattern>());
    model.Create(customNode);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_TRUE(itemNode);
    auto pattern = itemNode->GetPattern<CustomMenuItemPattern>();
    ASSERT_TRUE(pattern);
    ASSERT_TRUE(itemNode->GetEventHub<EventHub>());
    auto touch = itemNode->GetOrCreateGestureEventHub()->touchEventActuator_;
    ASSERT_TRUE(touch);
    ASSERT_FALSE(touch->touchEvents_.empty());
}

/**
 * @tc.name: MenuItemPatternTestNg001
 * @tc.desc: Verify GetMenuWrapper.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare wrapperNode, menuNode, itemNode
     * @tc.expected: itemPattern is not null
     */
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 3, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    /**
     * @tc.steps: step2. excute GetMenuWrapper
     * @tc.expected: result as expected
     */
    ASSERT_EQ(menuItemPattern->GetMenuWrapper(), nullptr);
    menuItemNode->MountToParent(wrapperNode);
    ASSERT_NE(menuItemPattern->GetMenuWrapper(), nullptr);
    wrapperNode->RemoveChildAtIndex(0);
    EXPECT_EQ(wrapperNode->GetChildren().size(), 0);
    menuItemNode->MountToParent(mainMenu);
    mainMenu->MountToParent(wrapperNode);
    ASSERT_NE(menuItemPattern->GetMenuWrapper(), nullptr);
}

/**
 * @tc.name: MenuItemPatternTestNg002
 * @tc.desc: Verify ShowSubMenu, CloseMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create cascade menu condition
     * @tc.expected: wrapper and child pattern is not null
     */
    std::function<void()> buildFun = []() {
        MenuModelNG MenuModelInstance;
        MenuModelInstance.Create();
    };
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto subMenu = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 3, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::SUB_MENU));
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto subMenuParent = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 5, AceType::MakeRefPtr<MenuItemPattern>());
    menuItemNode->MountToParent(mainMenu);
    mainMenu->MountToParent(wrapperNode);
    subMenu->MountToParent(wrapperNode);
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->ShowSubMenu();
    menuItemPattern->SetSubBuilder(buildFun);
    menuItemPattern->SetIsSubMenuShowed(false);
    auto mainMenuPattern = mainMenu->GetPattern<MenuPattern>();
    ASSERT_NE(mainMenuPattern, nullptr);
    mainMenuPattern->SetShowedSubMenu(subMenu);
    auto subMenuPattern = subMenu->GetPattern<MenuPattern>();
    ASSERT_NE(subMenuPattern, nullptr);
    subMenuPattern->SetParentMenuItem(subMenuParent);
    /**
     * @tc.steps: step2. prepare wrapperNode, menuNode, itemNode
     * @tc.expected: itemPattern is not null
     */
    menuItemPattern->ShowSubMenu();
    menuItemPattern->CloseMenu();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 2);
}

/**
 * @tc.name: MenuItemPatternTestNg003
 * @tc.desc: Verify RegisterOnClick.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create item node and eventhub
     * @tc.expected: pattern and eventhub is not null
     */
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    auto menuItemEventHub = menuItemNode->GetEventHub<MenuItemEventHub>();
    ASSERT_NE(menuItemEventHub, nullptr);
    /**
     * @tc.steps: step2. excute RegisterOnClick
     * @tc.expected: result as expected
     */
    bool isSelected = false;
    auto changeEvent = [&isSelected](bool select) { isSelected = !select; };
    menuItemEventHub->SetSelectedChangeEvent(changeEvent);
    menuItemEventHub->SetOnChange(changeEvent);
    menuItemPattern->RegisterOnClick();
    // trigger click
    auto gestureHub = menuItemNode->GetOrCreateGestureEventHub();
    auto clickEventActuator = gestureHub->clickEventActuator_;
    ASSERT_NE(clickEventActuator, nullptr);
    auto event = clickEventActuator->GetClickEvent();
    ASSERT_NE(event, nullptr);
    GestureEvent gestureEvent;
    event(gestureEvent);
    EXPECT_TRUE(isSelected);
    // update item pattern subbuilder， click item
    std::function<void()> buildFun = [] {};
    menuItemPattern->SetSubBuilder(buildFun);
    menuItemPattern->RegisterOnClick();
    event(gestureEvent);
    EXPECT_FALSE(isSelected);
}

/**
 * @tc.name: MenuItemPatternTestNg004
 * @tc.desc: Verify OnTouch.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create item node
     * @tc.expected: pattern is not null
     */
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    /**
     * @tc.steps: step2. excute OnTouch
     * @tc.expected: result as expected
     */
    // excute touch down event
    TouchEventInfo itemTouchDownEventInfo(MENU_TOUCH_EVENT_TYPE);
    TouchLocationInfo downLocationInfo(TARGET_ID);
    Offset touchDownGlobalLocation(1, 1);
    downLocationInfo.SetTouchType(TouchType::DOWN);
    auto touchDownLocationInfo = downLocationInfo.SetGlobalLocation(touchDownGlobalLocation);
    itemTouchDownEventInfo.touches_.emplace_back(touchDownLocationInfo);
    menuItemPattern->OnTouch(itemTouchDownEventInfo);
    EXPECT_EQ(itemTouchDownEventInfo.touches_.size(), 1);
    // excute touch up event
    TouchEventInfo itemTouchUpEventInfo(MENU_TOUCH_EVENT_TYPE);
    TouchLocationInfo upLocationInfo(TARGET_ID);
    Offset touchUpGlobalLocation(3, 3);
    upLocationInfo.SetTouchType(TouchType::UP);
    auto touchUpLocationInfo = upLocationInfo.SetGlobalLocation(touchUpGlobalLocation);
    itemTouchUpEventInfo.touches_.emplace_back(touchUpLocationInfo);
    menuItemPattern->OnTouch(itemTouchUpEventInfo);
    EXPECT_EQ(itemTouchUpEventInfo.touches_.size(), 1);
    // excute touch move event
    upLocationInfo.SetTouchType(TouchType::MOVE);
    touchUpLocationInfo = upLocationInfo.SetGlobalLocation(touchUpGlobalLocation);
    itemTouchUpEventInfo.touches_.clear();
    itemTouchUpEventInfo.touches_.emplace_back(touchUpLocationInfo);
    menuItemPattern->OnTouch(itemTouchUpEventInfo);
    EXPECT_EQ(itemTouchUpEventInfo.touches_.size(), 1);
}

/**
 * @tc.name: MenuItemPatternTestNg005
 * @tc.desc: Verify OnHover.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg005, TestSize.Level1)
{
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->OnHover(true);
    menuItemPattern->OnHover(false);
    EXPECT_EQ(menuItemPattern->GetBgBlendColor(), Color::TRANSPARENT);
}

/**
 * @tc.name: MenuItemPatternTestNg006
 * @tc.desc: Verify OnKeyEvent,PlayBgColorAnimation.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create item node
     * @tc.expected: pattern is not null
     */
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    /**
     * @tc.steps: step2. update event type, excute OnKeyEvent
     * @tc.expected: result as expected
     */
    // use longPressEvent
    KeyEvent longPressEvent(KeyCode::KEY_ESCAPE, KeyAction::LONG_PRESS);
    EXPECT_FALSE(menuItemPattern->OnKeyEvent(longPressEvent));
    // use enterEvent
    KeyEvent enterEvent(KeyCode::KEY_ENTER, KeyAction::DOWN);
    EXPECT_TRUE(menuItemPattern->OnKeyEvent(enterEvent));
    // use rightEvent
    KeyEvent rightEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN);
    std::function<void()> buildFun = []() {};
    menuItemPattern->SetSubBuilder(buildFun);
    menuItemPattern->SetIsSubMenuShowed(false);
    EXPECT_TRUE(menuItemPattern->OnKeyEvent(rightEvent));
    // use fnEvent
    KeyEvent fnEvent(KeyCode::KEY_FN, KeyAction::DOWN);
    menuItemPattern->PlayBgColorAnimation(false);
    menuItemPattern->PlayBgColorAnimation(true);
    EXPECT_FALSE(menuItemPattern->OnKeyEvent(fnEvent));
}

/**
 * @tc.name: MenuItemPatternTestNg007
 * @tc.desc: Verify RegisterWrapperMouseEvent.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg007, TestSize.Level1)
{
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    menuItemNode->MountToParent(mainMenu);
    mainMenu->MountToParent(wrapperNode);
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->RegisterWrapperMouseEvent();
    EXPECT_EQ(wrapperNode->GetChildren().size(), 1);
}

/**
 * @tc.name: MenuItemPatternTestNg008
 * @tc.desc: Verify AddSelfHoverRegion, IsInHoverRegions.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg008, TestSize.Level1)
{
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    mainMenu->GetGeometryNode()->SetFrameSize(SizeF(100, 100));
    menuItemPattern->AddSelfHoverRegion(mainMenu);
    EXPECT_EQ(menuItemPattern->hoverRegions_.size(), 1);
    EXPECT_TRUE(menuItemPattern->IsInHoverRegions(40, 40));
    EXPECT_FALSE(menuItemPattern->IsInHoverRegions(200, 200));
}

/**
 * @tc.name: MenuItemPatternTestNg009
 * @tc.desc: Verify AddSelfHoverRegion.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemPatternTestNg009, TestSize.Level1)
{
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    auto mainMenu =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    mainMenu->GetGeometryNode()->SetFrameSize(SizeF(100, 100));
    auto position = menuItemPattern->GetSubMenuPostion(mainMenu);
    EXPECT_EQ(position, OffsetF(100, 0));
}

/**
 * @tc.name: MenuItemViewTestNgCreate001
 * @tc.desc: Verify Create.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgCreate001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_EQ(itemNode->GetChildren().size(), 2);
    auto leftRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(0));
    EXPECT_EQ(leftRow->GetChildren().size(), 0);
    auto rightRow = AceType::DynamicCast<FrameNode>(itemNode->GetChildAtIndex(1));
    EXPECT_EQ(rightRow->GetChildren().size(), 0);

    ASSERT_TRUE(itemProperty->GetStartIcon().has_value());
    EXPECT_EQ(itemProperty->GetStartIcon().value(), "");
    ASSERT_TRUE(itemProperty->GetEndIcon().has_value());
    EXPECT_EQ(itemProperty->GetEndIcon().value(), "");
    ASSERT_TRUE(itemProperty->GetContent().has_value());
    EXPECT_EQ(itemProperty->GetContent().value(), "");
    ASSERT_TRUE(itemProperty->GetLabel().has_value());
    EXPECT_EQ(itemProperty->GetLabel().value(), "");
}

/**
 * @tc.name: MenuItemViewTestNgCreate002
 * @tc.desc: Verify Create.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgCreate002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    itemOption.content = "content";
    itemOption.startIcon = "startIcon";
    itemOption.endIcon = "endIcon";
    itemOption.labelInfo = "label";
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetStartIcon().has_value());
    EXPECT_EQ(itemProperty->GetStartIcon().value(), "startIcon");
    ASSERT_TRUE(itemProperty->GetEndIcon().has_value());
    EXPECT_EQ(itemProperty->GetEndIcon().value(), "endIcon");
    ASSERT_TRUE(itemProperty->GetContent().has_value());
    EXPECT_EQ(itemProperty->GetContent().value(), "content");
    ASSERT_TRUE(itemProperty->GetLabel().has_value());
    EXPECT_EQ(itemProperty->GetLabel().value(), "label");
}

/**
 * @tc.name: MenuItemViewTestNgSetSelectIcon001
 * @tc.desc: Verify SetSelectIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetSelectIcon001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetSelectIcon(true);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    EXPECT_EQ(itemProperty->GetSelectIcon().value_or(false), true);
}

/**
 * @tc.name: MenuItemViewTestNgSetSelectIcon002
 * @tc.desc: Verify SetSelectIcon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetSelectIcon002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetSelectIcon(true);
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    EXPECT_FALSE(itemProperty->GetSelectIcon().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetSelectIconSrc001
 * @tc.desc: Verify SetSelectIconSrc.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetSelectIconSrc001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetSelectIconSrc("selectIcon.png");
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    EXPECT_EQ(itemProperty->GetSelectIconSrc().value_or(""), "selectIcon.png");
}

/**
 * @tc.name: MenuItemViewTestNgSetSelectIconSrc002
 * @tc.desc: Verify SetSelectIconSrc.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetSelectIconSrc002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetSelectIconSrc("selectIcon.png");
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    EXPECT_FALSE(itemProperty->GetSelectIconSrc().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetFontSize001
 * @tc.desc: Verify SetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontSize001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontSize(Dimension());
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    EXPECT_FALSE(itemProperty->GetFontSize().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetFontSize002
 * @tc.desc: Verify SetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontSize002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontSize(Dimension(40.0));
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetFontSize().has_value());
    EXPECT_EQ(itemProperty->GetFontSize().value(), Dimension(40.0));
}

/**
 * @tc.name: MenuItemViewTestNgSetFontSize003
 * @tc.desc: Verify SetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontSize003, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetFontSize(Dimension(40.0));
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    ASSERT_FALSE(itemProperty->GetFontSize().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetFontWeight001
 * @tc.desc: Verify SetFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontWeight001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontWeight(FontWeight::BOLD);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetFontWeight().has_value());
    EXPECT_EQ(itemProperty->GetFontWeight().value(), FontWeight::BOLD);
}

/**
 * @tc.name: MenuItemViewTestNgSetFontWeight002
 * @tc.desc: Verify SetFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontWeight002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetFontWeight(FontWeight::BOLD);
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    ASSERT_FALSE(itemProperty->GetFontWeight().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetFontColor001
 * @tc.desc: Verify SetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontColor001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontColor(Color::RED);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetFontColor().has_value());
    EXPECT_EQ(itemProperty->GetFontColor().value(), Color::RED);
}

/**
 * @tc.name: MenuItemViewTestNgSetFontColor002
 * @tc.desc: Verify SetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontColor002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetFontColor(Color::RED);
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    ASSERT_FALSE(itemProperty->GetFontColor().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetFontColor003
 * @tc.desc: Verify SetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontColor003, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontColor(Color::RED);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetFontColor().has_value());
    EXPECT_EQ(itemProperty->GetFontColor().value(), Color::RED);

    ViewStackProcessor::GetInstance()->Push(itemNode);
    MneuItemModelInstance.SetFontColor(std::nullopt);
    ASSERT_FALSE(itemProperty->GetFontColor().has_value());
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontSize001
 * @tc.desc: Verify SetLabelFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontSize001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetLabelFontSize(Dimension());
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    EXPECT_FALSE(itemProperty->GetLabelFontSize().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontSize002
 * @tc.desc: Verify SetLabelFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontSize002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetLabelFontSize(Dimension(40.0));
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetLabelFontSize().has_value());
    EXPECT_EQ(itemProperty->GetLabelFontSize().value(), Dimension(40.0));
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontSize003
 * @tc.desc: Verify SetLabelFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontSize003, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetLabelFontSize(Dimension(40.0));
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    ASSERT_FALSE(itemProperty->GetLabelFontSize().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontWeight001
 * @tc.desc: Verify SetLabelFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontWeight001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetLabelFontWeight(FontWeight::BOLD);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetLabelFontWeight().has_value());
    EXPECT_EQ(itemProperty->GetLabelFontWeight().value(), FontWeight::BOLD);
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontWeight002
 * @tc.desc: Verify SetLabelFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontWeight002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetLabelFontWeight(FontWeight::BOLD);
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    ASSERT_FALSE(itemProperty->GetLabelFontWeight().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontColor001
 * @tc.desc: Verify SetLabelFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontColor001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetLabelFontColor(Color::RED);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetLabelFontColor().has_value());
    EXPECT_EQ(itemProperty->GetLabelFontColor().value(), Color::RED);
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontColor002
 * @tc.desc: Verify SetLabelFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontColor002, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.SetLabelFontColor(Color::RED);
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);
    ASSERT_FALSE(itemProperty->GetLabelFontColor().has_value());
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontColor003
 * @tc.desc: Verify SetLabelFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontColor003, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetLabelFontColor(Color::RED);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    ASSERT_TRUE(itemProperty->GetLabelFontColor().has_value());
    EXPECT_EQ(itemProperty->GetLabelFontColor().value(), Color::RED);

    ViewStackProcessor::GetInstance()->Push(itemNode);
    MneuItemModelInstance.SetLabelFontColor(std::nullopt);
    ASSERT_FALSE(itemProperty->GetLabelFontColor().has_value());
    ViewStackProcessor::GetInstance()->Finish();
}
/**
 * @tc.name: MenuItemSetSelectedChangeEvent001
 * @tc.desc: Verify SetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemSetSelectedChangeEvent001, TestSize.Level1)
{
    MenuItemModelNG MneuItemModelInstance;
    bool isSelected = false;
    auto changeEvent = [&isSelected](bool select) { isSelected = select; };
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetSelectedChangeEvent(changeEvent);

    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);

    auto itemProperty = itemNode->GetEventHub<NG::MenuItemEventHub>();
    ASSERT_NE(itemProperty, nullptr);
    EXPECT_TRUE(itemProperty->GetSelectedChangeEvent());
}

/**
 * @tc.name: MenuItemViewTestNgSetLabelFontStyle001
 * @tc.desc: Verify SetLabelFontStyle.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetLabelFontStyle001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create MenuItemModelNG object and set LabelFontStyle properties.
     */
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetLabelFontStyle(Ace::FontStyle::ITALIC);

    /**
     * @tc.steps: step2. get the frameNode, menuItemPattern and menuItemLayoutProperty.
     * @tc.expected: step2. check whether the objects is available.
     */
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    /**
     * @tc.steps: step3. get the labelFontStyle properties of menuItemLayoutProperty.
     * @tc.expected: step3. check whether the labelFontStyle properties is is correct.
     */
    ASSERT_TRUE(itemProperty->GetLabelItalicFontStyle().has_value());
    EXPECT_EQ(itemProperty->GetLabelItalicFontStyle().value(), Ace::FontStyle::ITALIC);
}

/**
 * @tc.name: MenuItemViewTestNgSetFontStyle001
 * @tc.desc: Verify SetFontStyle.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemViewTestNgSetFontStyle001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create MenuItemModelNG object and set FontStyle properties.
     */
    MenuItemModelNG MneuItemModelInstance;
    MenuItemProperties itemOption;
    MneuItemModelInstance.Create(itemOption);
    MneuItemModelInstance.SetFontStyle(Ace::FontStyle::ITALIC);

    /**
     * @tc.steps: step2. get the frameNode, menuItemPattern and menuItemLayoutProperty.
     * @tc.expected: step2. check whether the objects is available.
     */
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    auto itemProperty = itemNode->GetLayoutProperty<MenuItemLayoutProperty>();
    ASSERT_NE(itemProperty, nullptr);

    /**
     * @tc.steps: step3. get the FontStyle properties of menuItemLayoutProperty.
     * @tc.expected: step3. check whether the FontStyle properties is is correct.
     */
    ASSERT_TRUE(itemProperty->GetItalicFontStyle().has_value());
    EXPECT_EQ(itemProperty->GetItalicFontStyle().value(), Ace::FontStyle::ITALIC);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg001
 * @tc.desc: Verify HorizontalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg001, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(50, 100);
    SizeF size_f(100, 200);
    float clickPosition = 50.0f;
    menuLayoutAlgorithm->wrapperSize_ = size_f;
    auto result = menuLayoutAlgorithm->HorizontalLayout(size, clickPosition);
    EXPECT_EQ(result, clickPosition);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg002
 * @tc.desc: Verify HorizontalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg002, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(50, 100);
    SizeF size_f(100, 200);
    float clickPosition = 60.0f;
    menuLayoutAlgorithm->wrapperSize_ = size_f;
    auto result = menuLayoutAlgorithm->HorizontalLayout(size, clickPosition);
    EXPECT_EQ(result, size_f.Width() - size.Width());
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg003
 * @tc.desc: Verify HorizontalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg003, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(70, 100);
    SizeF size_f(100, 200);
    float clickPosition = 60.0f;
    menuLayoutAlgorithm->wrapperSize_ = size_f;
    auto result = menuLayoutAlgorithm->HorizontalLayout(size, clickPosition);
    EXPECT_EQ(result, menuLayoutAlgorithm->wrapperSize_.Width() - size.Width());
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg004
 * @tc.desc: Verify HorizontalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg004, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(100, 100);
    SizeF size_f(100, 200);
    float clickPosition = 60.0f;
    menuLayoutAlgorithm->wrapperSize_ = size_f;
    auto result = menuLayoutAlgorithm->HorizontalLayout(size, clickPosition);
    EXPECT_EQ(result, 0.0);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg005
 * @tc.desc: Verify VerticalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg005, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(100, 100);
    SizeF size_f(100, 200);
    float clickPosition = 100.0f;
    menuLayoutAlgorithm->wrapperSize_ = size_f;
    auto result = menuLayoutAlgorithm->VerticalLayout(size, clickPosition);
    EXPECT_EQ(result, clickPosition);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg006
 * @tc.desc: Verify VerticalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg006, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(100, 100);
    float clickPosition = 150.0f;
    menuLayoutAlgorithm->topSpace_ = 200.0f;
    auto result = menuLayoutAlgorithm->VerticalLayout(size, clickPosition);
    EXPECT_EQ(result, menuLayoutAlgorithm->topSpace_ - size.Height());
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg007
 * @tc.desc: Verify VerticalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg007, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(100, 150);
    SizeF size_f(100, 200);
    float clickPosition = 100.0f;
    menuLayoutAlgorithm->wrapperSize_ = size_f;
    auto result = menuLayoutAlgorithm->VerticalLayout(size, clickPosition);
    EXPECT_EQ(result, menuLayoutAlgorithm->wrapperSize_.Height() - size.Height());
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg008
 * @tc.desc: Verify VerticalLayout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg008, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF size(100, 200);
    SizeF size_f(100, 200);
    float clickPosition = 100.0f;
    menuLayoutAlgorithm->wrapperSize_ = size_f;
    auto result = menuLayoutAlgorithm->VerticalLayout(size, clickPosition);
    EXPECT_EQ(result, 0.0);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg009
 * @tc.desc: Verify Initialize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg009, TestSize.Level1)
{
    std::function<void()> action = [] {};
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "", action);
    optionParams.emplace_back("MenuItem2", "", action);
    MenuParam menuParam;
    auto menuWrapperNode =
        MenuView::Create(std::move(optionParams), 1, "", MenuType::SELECT_OVERLAY_EXTENSION_MENU, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ASSERT_TRUE(property->GetPositionOffset().has_value());
    EXPECT_EQ(property->GetPositionOffset().value(), OffsetF());
    RefPtr<MenuLayoutAlgorithm> layoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode layoutWrapper(menuNode, geometryNode, menuNode->GetLayoutProperty());

    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();

    layoutAlgorithm->Initialize(&layoutWrapper);
    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->position_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->positionOffset_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->wrapperSize_, SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(geometryNode->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg010
 * @tc.desc: Verify Layout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg010, TestSize.Level1)
{
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    OffsetF offset(10, 10);
    menuLayoutAlgorithm->position_ = offset;
    const std::string tag = "tag";
    int32_t nodeId = 1;
    RefPtr<Pattern> pattern = AceType::MakeRefPtr<Pattern>();
    bool isRoot = false;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<FrameNode> frameNode = AceType::MakeRefPtr<FrameNode>(tag, nodeId, pattern, isRoot);
    LayoutWrapperNode* layoutWrapper = new LayoutWrapperNode(frameNode, geometryNode, frameNode->GetLayoutProperty());
    RefPtr<LazyForEachActuator> actuator = AceType::MakeRefPtr<LazyForEachActuator>();
    auto builder = AceType::DynamicCast<LazyForEachBuilder>(actuator);
    RefPtr<LazyForEachNode> host_ = AceType::MakeRefPtr<LazyForEachNode>(nodeId, builder);
    WeakPtr<LazyForEachNode> host(host_);
    RefPtr<LazyLayoutWrapperBuilder> wrapperBuilder = AceType::MakeRefPtr<LazyLayoutWrapperBuilder>(builder, host);
    wrapperBuilder->OnGetOrCreateWrapperByIndex(nodeId);
    auto children = wrapperBuilder->OnExpandChildLayoutWrapper();
    auto layoutWrapper_ = wrapperBuilder->GetOrCreateWrapperByIndex(nodeId);
    menuLayoutAlgorithm->Layout(layoutWrapper);
    EXPECT_EQ(menuLayoutAlgorithm->position_.GetX(), 10);
    delete layoutWrapper;
    layoutWrapper = nullptr;
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg011
 * @tc.desc: Verify positionOffset of Layout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg011, TestSize.Level1)
{
    std::function<void()> action = [] {};
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "", action);
    optionParams.emplace_back("MenuItem2", "", action);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), 1, "", MenuType::MENU, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ASSERT_TRUE(property->GetPositionOffset().has_value());
    EXPECT_EQ(property->GetPositionOffset().value(), OffsetF());
    RefPtr<MenuLayoutAlgorithm> layoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode layoutWrapper(menuNode, geometryNode, menuNode->GetLayoutProperty());

    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();

    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->position_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->positionOffset_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->wrapperSize_, SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(geometryNode->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg012
 * @tc.desc: Verify positionOffset of Layout.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg012, TestSize.Level1)
{
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "", nullptr);
    optionParams.emplace_back("MenuItem2", "", nullptr);
    MenuParam menuParam = { "", { POSITION_OFFSET, POSITION_OFFSET } };
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), 1, "", MenuType::MENU, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ASSERT_TRUE(property->GetPositionOffset().has_value());
    EXPECT_EQ(property->GetPositionOffset().value(), OffsetF(POSITION_OFFSET, POSITION_OFFSET));

    RefPtr<MenuLayoutAlgorithm> layoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode layoutWrapper(menuNode, geometryNode, menuNode->GetLayoutProperty());
    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();

    layoutAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(layoutAlgorithm->position_, OffsetF());
    EXPECT_EQ(layoutAlgorithm->positionOffset_, OffsetF(POSITION_OFFSET, POSITION_OFFSET));
    EXPECT_EQ(layoutAlgorithm->wrapperSize_, SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutAlgorithm->Layout(&layoutWrapper);
    EXPECT_EQ(geometryNode->GetMarginFrameOffset(), OffsetF(POSITION_OFFSET, POSITION_OFFSET));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg013
 * @tc.desc: Verify ComputeMenuPositionByAlignType.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg013, TestSize.Level1)
{
    std::vector<SelectParam> selectParams;
    selectParams.emplace_back(std::make_pair("MenuItem1", "Icon1"));
    selectParams.emplace_back(std::make_pair("MenuItem2", "Icon2"));
    // create select menu
    auto menuWrapperNode = MenuView::Create(std::move(selectParams), 1, EMPTY_TEXT);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    SizeF targetSize(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    property->UpdateTargetSize(targetSize);

    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    ASSERT_NE(menuLayoutAlgorithm, nullptr);

    /**
     * @tc.cases: case1. the menu align type is start.
     */
    SizeF menuSize(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT);
    menuLayoutAlgorithm->position_ = OffsetF(0, 0);
    property->UpdateAlignType(MenuAlignType::START);
    menuLayoutAlgorithm->ComputeMenuPositionByAlignType(property, menuSize);
    EXPECT_EQ(menuLayoutAlgorithm->position_.GetX(), 0);

    /**
     * @tc.cases: case2. the menu align type is center.
     */
    menuLayoutAlgorithm->position_ = OffsetF(0, 0);
    property->UpdateAlignType(MenuAlignType::CENTER);
    menuLayoutAlgorithm->ComputeMenuPositionByAlignType(property, menuSize);
    float expectResult = -25.0f;
    EXPECT_EQ(menuLayoutAlgorithm->position_.GetX(), expectResult);

    /**
     * @tc.cases: case3. the menu align type is end.
     */
    menuLayoutAlgorithm->position_ = OffsetF(0, 0);
    property->UpdateAlignType(MenuAlignType::END);
    menuLayoutAlgorithm->ComputeMenuPositionByAlignType(property, menuSize);
    expectResult = -50.0f;
    EXPECT_EQ(menuLayoutAlgorithm->position_.GetX(), expectResult);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg014
 * @tc.desc: Verify ComputeMenuPositionByOffset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg014, TestSize.Level1)
{
    std::vector<SelectParam> selectParams;
    selectParams.emplace_back(std::make_pair("MenuItem1", "Icon1"));
    selectParams.emplace_back(std::make_pair("MenuItem2", "Icon2"));
    // create select menu
    auto menuWrapperNode = MenuView::Create(std::move(selectParams), 1, EMPTY_TEXT);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    property->UpdateLayoutConstraint(parentLayoutConstraint);

    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT));

    /**
     * @tc.cases: case1. parameter is valid, return the valid offset.
     */
    property->UpdateOffset(
        DimensionOffset(Dimension(MENU_OFFSET_X, DimensionUnit::VP), Dimension(MENU_OFFSET_Y, DimensionUnit::VP)));
    auto resultOffset = menuLayoutAlgorithm->ComputeMenuPositionByOffset(property, geometryNode);
    EXPECT_EQ(resultOffset, OffsetF(MENU_OFFSET_X, MENU_OFFSET_Y));

    /**
     * @tc.cases: case2. parameter property is nullptr, return OffsetF(0.0, 0.0).
     */
    resultOffset = menuLayoutAlgorithm->ComputeMenuPositionByOffset(nullptr, geometryNode);
    EXPECT_EQ(resultOffset, OffsetF(0.0, 0.0));

    /**
     * @tc.cases: case3. parameter geometryNode is nullptr, return OffsetF(0.0, 0.0).
     */
    resultOffset = menuLayoutAlgorithm->ComputeMenuPositionByOffset(property, nullptr);
    EXPECT_EQ(resultOffset, OffsetF(0.0, 0.0));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg015
 * @tc.desc: Verify ComputeMenuPositionByOffset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg015, TestSize.Level1)
{
    std::vector<SelectParam> selectParams;
    selectParams.emplace_back(std::make_pair("MenuItem1", "Icon1"));
    selectParams.emplace_back(std::make_pair("MenuItem2", "Icon2"));
    // create select menu
    auto menuWrapperNode = MenuView::Create(std::move(selectParams), 1, EMPTY_TEXT);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    property->UpdateLayoutConstraint(parentLayoutConstraint);

    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODEID, "menu");
    ASSERT_NE(menuLayoutAlgorithm, nullptr);

    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    menuPattern->isSelectMenu_ = true;
    SizeF size(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT);

    /**
     * @tc.cases: case1. parameter property is nullptr, return OffsetF(0.0, 0.0).
     */
    auto resultOffset = menuLayoutAlgorithm->MenuLayoutAvoidAlgorithm(nullptr, menuPattern, size);
    EXPECT_EQ(resultOffset, OffsetF(0.0, 0.0));

    /**
     * @tc.cases: case2. parameter menuPattern is nullptr, return OffsetF(0.0, 0.0).
     */
    resultOffset = menuLayoutAlgorithm->MenuLayoutAvoidAlgorithm(property, nullptr, size);
    EXPECT_EQ(resultOffset, OffsetF(0.0, 0.0));

    /**
     * @tc.cases: case3. menu property has placement value and has targetSize.
     */
    property->UpdateMenuPlacement(Placement::RIGHT);
    menuLayoutAlgorithm->placement_ = Placement::RIGHT;
    menuLayoutAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuLayoutAlgorithm->targetOffset_ = OffsetF(POSITION_OFFSET, POSITION_OFFSET);
    menuLayoutAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    resultOffset = menuLayoutAlgorithm->MenuLayoutAvoidAlgorithm(property, menuPattern, size);
    float expectOffsetX = POSITION_OFFSET + TARGET_SIZE_WIDTH + TARGET_SECURITY.ConvertToPx();
    float expectOffsetY = MENU_SIZE_HEIGHT / 2;
    EXPECT_EQ(resultOffset, OffsetF(expectOffsetX, expectOffsetY));

    /**
     * @tc.cases: case4. target size is (0.0, 0.0)
     */
    menuLayoutAlgorithm->targetSize_ = SizeF(0.0f, 0.0f);
    resultOffset = menuLayoutAlgorithm->MenuLayoutAvoidAlgorithm(property, menuPattern, size);
    EXPECT_EQ(resultOffset, OffsetF(FULL_SCREEN_WIDTH - MENU_SIZE_WIDTH, FULL_SCREEN_HEIGHT - MENU_SIZE_HEIGHT));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg016
 * @tc.desc: Test MultiMenu layout algorithm.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg016, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MULTI_MENU);
    auto multiMenu = AceType::MakeRefPtr<FrameNode>("", -1, menuPattern);
    auto algorithm = AceType::MakeRefPtr<MultiMenuLayoutAlgorithm>();
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    geometryNode->SetFrameSize(SizeF(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT));
    auto layoutProp = AceType::MakeRefPtr<MenuLayoutProperty>();
    auto* wrapper = new LayoutWrapperNode(multiMenu, geometryNode, layoutProp);

    for (int32_t i = 0; i < 3; ++i) {
        auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
        auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, itemPattern);
        auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeoNode->SetFrameSize(SizeF(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT / 3));
        auto childWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);
        wrapper->AppendChild(childWrapper);
    }

    algorithm->Layout(wrapper);
    // default padding from theme is zero, so the offset on the first child is zero.
    OffsetF offset;
    for (auto&& child : wrapper->GetAllChildrenWithBuild()) {
        EXPECT_EQ(child->GetGeometryNode()->GetMarginFrameOffset(), offset);
        offset.AddY(MENU_SIZE_HEIGHT / 3);
    }
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg017
 * @tc.desc: Verify GetPositionWithPlacement.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    auto result = menuLayoutAlgorithm->GetPositionWithPlacement(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), 0);
    EXPECT_EQ(result.GetY(), 0);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg018
 * @tc.desc: Verify GetPositionWithPlacement with targetNodeId, targetTag
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetNodeId, targetTag
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    const std::string tag = "tag";
    MenuLayoutAlgorithm menu(NODEID, tag);
    auto result = menu.GetPositionWithPlacement(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), 0);
    EXPECT_EQ(result.GetY(), 0);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg019
 * @tc.desc: Verify AddTargetSpace with placement
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, placement is BOTTOM_LEFT
     * @tc.expected: result y offset is add 8vp space
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    menuLayoutAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    auto result = menuLayoutAlgorithm->AddTargetSpace(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST, OFFSET_FIRST + TARGET_SECURITY.ConvertToPx()));

    /**
     * @tc.steps: step2. create menuLayoutAlgorithm, placement is TOP_LEFT
     * @tc.expected: result y offset is reduce 8vp space
     */
    menuLayoutAlgorithm->placement_ = Placement::TOP_LEFT;
    result = menuLayoutAlgorithm->AddTargetSpace(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST, OFFSET_FIRST - TARGET_SECURITY.ConvertToPx()));

    /**
     * @tc.steps: step3. create menuLayoutAlgorithm, placement is RIGHT_TOP
     * @tc.expected: result x offset is add 8vp space
     */
    menuLayoutAlgorithm->placement_ = Placement::RIGHT_TOP;
    result = menuLayoutAlgorithm->AddTargetSpace(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST + TARGET_SECURITY.ConvertToPx(), OFFSET_FIRST));

    /**
     * @tc.steps: step4. create menuLayoutAlgorithm, placement is LEFT_TOP
     * @tc.expected: result x offset is reduce 8vp space
     */
    menuLayoutAlgorithm->placement_ = Placement::LEFT_TOP;
    result = menuLayoutAlgorithm->AddTargetSpace(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST - TARGET_SECURITY.ConvertToPx(), OFFSET_FIRST));

    /**
     * @tc.steps: step5. create menuLayoutAlgorithm, placement is NONE
     * @tc.expected: the offset of the result is consistent with when the placement is BOTTOM_LEFT
     */
    menuLayoutAlgorithm->placement_ = Placement::NONE;
    result = menuLayoutAlgorithm->AddTargetSpace(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST, OFFSET_FIRST + TARGET_SECURITY.ConvertToPx()));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg020
 * @tc.desc: Verify AddOffset with placement
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, placement is BOTTOM_LEFT
     * @tc.expected: result x and y offset is add position offset
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    menuLayoutAlgorithm->positionOffset_ = OffsetF(OFFSET_SECOND, OFFSET_SECOND);
    menuLayoutAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    auto result = menuLayoutAlgorithm->AddOffset(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST + OFFSET_SECOND, OFFSET_FIRST + OFFSET_SECOND));

    /**
     * @tc.steps: step2. create menuLayoutAlgorithm, placement is TOP_LEFT
     * @tc.expected: result x offset add position offset and y offset is reduce position offset
     */
    menuLayoutAlgorithm->placement_ = Placement::TOP_LEFT;
    result = menuLayoutAlgorithm->AddOffset(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST + OFFSET_SECOND, OFFSET_FIRST - OFFSET_SECOND));

    /**
     * @tc.steps: step3. create menuLayoutAlgorithm, placement is RIGHT_TOP
     * @tc.expected: result x offset add position offset and y offset is add position offset
     */
    menuLayoutAlgorithm->placement_ = Placement::RIGHT_TOP;
    result = menuLayoutAlgorithm->AddOffset(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST + OFFSET_SECOND, OFFSET_FIRST + OFFSET_SECOND));

    /**
     * @tc.steps: step4. create menuLayoutAlgorithm, placement is LEFT_TOP
     * @tc.expected: result x offset reduce position offset and y offset is add position offset
     */
    menuLayoutAlgorithm->placement_ = Placement::LEFT_TOP;
    result = menuLayoutAlgorithm->AddOffset(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST - OFFSET_SECOND, OFFSET_FIRST + OFFSET_SECOND));

    /**
     * @tc.steps: step5. create menuLayoutAlgorithm, placement is NONE
     * @tc.expected: the offset of the result is consistent with when the placement is BOTTOM_LEFT
     */
    menuLayoutAlgorithm->placement_ = Placement::NONE;
    result = menuLayoutAlgorithm->AddOffset(OffsetF(OFFSET_FIRST, OFFSET_FIRST));
    EXPECT_EQ(result, OffsetF(OFFSET_FIRST + OFFSET_SECOND, OFFSET_FIRST + OFFSET_SECOND));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg021
 * @tc.desc: Verify GetPositionWithPlacementTop.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition
     * @tc.expected: step1. position is topPosition
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementTop(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), TOP_POSITION_X);
    EXPECT_EQ(result.GetY(), TOP_POSITION_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg022
 * @tc.desc: Verify GetPositionWithPlacementTopLeft.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition and targetOffset
     * @tc.expected: step1. position is (targetOffset_.GetX(), targetOffset_.GetY() - childSize.Height())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementTopLeft(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), TOP_LEFT_X);
    EXPECT_EQ(result.GetY(), TOP_LEFT_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg023
 * @tc.desc: Verify GetPositionWithPlacementTopRight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition and targetOffset
     * @tc.expected: step1. position is (targetOffset_.GetX() + targetSize_.Width() - childSize.Width(),
     * targetOffset_.GetY() - childSize.Height())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementTopRight(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), TOP_RIGHT_X);
    EXPECT_EQ(result.GetY(), TOP_RIGHT_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg024
 * @tc.desc: Verify GetPositionWithPlacementBottom.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition
     * @tc.expected: step1. position is bottomPosition
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementBottom(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), BOTTOM_POSITION_X);
    EXPECT_EQ(result.GetY(), BOTTOM_POSITION_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg025
 * @tc.desc: Verify GetPositionWithPlacementBottomLeft.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX(), targetOffset_.GetY() + targetSize_.Height())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementBottomLeft(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), BOTTOM_LEFT_X);
    EXPECT_EQ(result.GetY(), BOTTOM_LEFT_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg026
 * @tc.desc: Verify GetPositionWithPlacementBottomRight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX() + targetSize_.Width() - childSize.Width(),
     * targetOffset_.GetY() + targetSize_.Height())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementBottomRight(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), BOTTOM_RIGHT_X);
    EXPECT_EQ(result.GetY(), BOTTOM_RIGHT_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg027
 * @tc.desc: Verify GetPositionWithPlacementLeft.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX() - childSize.Width(),
     * targetOffset_.GetY() + targetSize_.Height() / 2.0 - childSize.Height() / 2.0)
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementLeft(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), PLACEMENT_LEFT_X);
    EXPECT_EQ(result.GetY(), PLACEMENT_LEFT_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg028
 * @tc.desc: Verify GetPositionWithPlacementLeftTop.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg028, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX() - childSize.Width(),
     * targetOffset_.GetY())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementLeftTop(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), PLACEMENT_LEFT_TOP_X);
    EXPECT_EQ(result.GetY(), PLACEMENT_LEFT_TOP_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg029
 * @tc.desc: Verify GetPositionWithPlacementLeftBottom.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg029, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX() - childSize.Width(),
     * targetOffset_.GetY() + targetSize_.Height() - childSize.Height())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementLeftBottom(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), PLACEMENT_LEFT_BOTTOM_X);
    EXPECT_EQ(result.GetY(), PLACEMENT_LEFT_BOTTOM_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg030
 * @tc.desc: Verify GetPositionWithPlacementRight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg030, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX() + targetSize_.Width(),
     * targetOffset_.GetY() + targetSize_.Height() / 2.0 - childSize.Height() / 2.0)
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementRight(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), PLACEMENT_RIGHT_X);
    EXPECT_EQ(result.GetY(), PLACEMENT_RIGHT_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg031
 * @tc.desc: Verify GetPositionWithPlacementRightTop.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg031, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX() + targetSize_.Width(),
     * targetOffset_.GetY())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementRightTop(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), PLACEMENT_RIGHT_TOP_X);
    EXPECT_EQ(result.GetY(), PLACEMENT_RIGHT_TOP_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg032
 * @tc.desc: Verify GetPositionWithPlacementRightBottom.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg032, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm, set childSize, topPosition, bottomPosition, targetOffset and
     * targetSize
     * @tc.expected: step1. position is (targetOffset_.GetX() + targetSize_.Width(),
     * targetOffset_.GetY() + targetSize_.Height() - childSize.Height())
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>();
    SizeF childSize(CHILD_SIZE_X, CHILD_SIZE_Y);
    OffsetF topPosition(TOP_POSITION_X, TOP_POSITION_Y);
    OffsetF bottomPosition(BOTTOM_POSITION_X, BOTTOM_POSITION_Y);
    OffsetF offset(OFFSET_X_THIRD, OFFSET_Y_THIRD);
    menuLayoutAlgorithm->targetOffset_ = offset;
    SizeF size(SIZE_X_SECOND, SIZE_Y_SECOND);
    menuLayoutAlgorithm->targetSize_ = size;
    auto result = menuLayoutAlgorithm->GetPositionWithPlacementRightBottom(childSize, topPosition, bottomPosition);
    EXPECT_EQ(result.GetX(), PLACEMENT_RIGHT_BOTTOM_X);
    EXPECT_EQ(result.GetY(), PLACEMENT_RIGHT_BOTTOM_Y);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg033
 * @tc.desc: Test SubMenu layout algorithm.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg033, TestSize.Level1)
{
    // create parent menu item
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    // set parent item size
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));

    // create submenu
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SUB_MENU);
    auto subMenu = AceType::MakeRefPtr<FrameNode>("", -1, menuPattern);
    auto algorithm = AceType::DynamicCast<SubMenuLayoutAlgorithm>(menuPattern->CreateLayoutAlgorithm());
    ASSERT_TRUE(algorithm);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    geometryNode->SetFrameSize(SizeF(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT));
    auto layoutProp = AceType::MakeRefPtr<MenuLayoutProperty>();
    auto* wrapper = new LayoutWrapperNode(subMenu, geometryNode, layoutProp);
    // link parent menu item and sub menu
    ASSERT_TRUE(menuPattern);
    menuPattern->SetParentMenuItem(item);
    item->GetGeometryNode()->SetFrameOffset(OffsetF(MENU_OFFSET_X, MENU_OFFSET_Y));
    algorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);

    // @tc.cases: case1. sub menu show on the right side of item
    algorithm->position_ = OffsetF(MENU_OFFSET_X + MENU_ITEM_SIZE_WIDTH, MENU_OFFSET_Y);
    algorithm->Layout(wrapper);

    EXPECT_NE(wrapper->GetGeometryNode()->GetMarginFrameOffset().GetX(), MENU_ITEM_SIZE_WIDTH);

    // @tc.cases: case2. sub menu show on the left side of item
    algorithm->position_ = OffsetF(FULL_SCREEN_WIDTH, MENU_OFFSET_Y);
    algorithm->Layout(wrapper);

    EXPECT_NE(wrapper->GetGeometryNode()->GetMarginFrameOffset().GetX(), MENU_ITEM_SIZE_WIDTH);
    EXPECT_EQ(wrapper->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg034
 * @tc.desc: Test MultiMenu measure algorithm.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg034, TestSize.Level1)
{
    // set screen width for grid column
    ScreenSystemManager::GetInstance().SetWindowInfo(FULL_SCREEN_WIDTH, 1.0, 1.0);
    // create multi menu
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MULTI_MENU);
    auto multiMenu = AceType::MakeRefPtr<FrameNode>("", -1, menuPattern);
    auto algorithm = AceType::MakeRefPtr<MultiMenuLayoutAlgorithm>();
    ASSERT_TRUE(algorithm);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<MenuLayoutProperty>();
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    layoutProp->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutProp->UpdateContentConstraint();
    auto* wrapper = new LayoutWrapperNode(multiMenu, geometryNode, layoutProp);
    // create menu item
    for (int32_t i = 0; i < 3; ++i) {
        auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
        auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, itemPattern);
        auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
        auto childWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);
        wrapper->AppendChild(childWrapper);
    }

    algorithm->Measure(wrapper);
    // @tc.expected: menu content width = item width, height = sum(item height)
    auto expectedSize = SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT * 3);
    EXPECT_EQ(wrapper->GetGeometryNode()->GetContentSize().Height(), expectedSize.Height());
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg035
 * @tc.desc: Test GetChildPosition
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg035, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm
     * @tc.expected: menuLayoutAlgorithm is not null
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODEID, "menu");
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    SizeF size(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT);

    /**
     * @tc.steps: step2. the placement of menuLayoutAlgorithm is Placement::NONE
     * @tc.expected: GetChildPosition get result offset is menu defaultPositon
     */
    menuLayoutAlgorithm->placement_ = Placement::NONE;
    menuLayoutAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuLayoutAlgorithm->targetOffset_ = OffsetF(POSITION_OFFSET, POSITION_OFFSET);
    auto resultOffset = menuLayoutAlgorithm->GetChildPosition(size, false);
    float expectOffsetX = MENU_SIZE_WIDTH / 2;
    float expectOffsetY = MENU_SIZE_HEIGHT / 2;
    EXPECT_EQ(resultOffset, OffsetF(expectOffsetX, expectOffsetY));

    /**
     * @tc.steps: step3. the placement of menuLayoutAlgorithm is Placement::BOTTOM_LEFT
     * @tc.expected: GetChildPosition get result offset is adjust position
     */
    menuLayoutAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    menuLayoutAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH / 2, TARGET_SIZE_HEIGHT);
    menuLayoutAlgorithm->wrapperSize_ = SizeF(OFFSET_THIRD, OFFSET_FORTH);
    menuLayoutAlgorithm->targetOffset_ = OffsetF(OFFSET_SIXTH, 0.0f);

    resultOffset = menuLayoutAlgorithm->GetChildPosition(size);
    EXPECT_EQ(resultOffset, OffsetF(OFFSET_FIFTH, POSITION_OFFSET + TARGET_SECURITY.ConvertToPx()));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg036
 * @tc.desc: Test FitToScreen
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg036, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm
     * @tc.expected: menuLayoutAlgorithm is not null
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODEID, "menu");
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    SizeF size(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT);

    /**
     * @tc.steps: step2. the placement of menuLayoutAlgorithm is Placement::NONE and need arrow
     * @tc.expected: FitToScreen get result offset is (0.0f, 0.0f)
     */
    OffsetF position = OffsetF(POSITION_OFFSET, POSITION_OFFSET);
    menuLayoutAlgorithm->positionOffset_ = OffsetF(OFFSET_FIRST, OFFSET_FIRST);
    menuLayoutAlgorithm->arrowPlacement_ = Placement::NONE;
    auto resultOffset = menuLayoutAlgorithm->FitToScreen(position, size, true);
    EXPECT_EQ(resultOffset, OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg037
 * @tc.desc: Test CheckPosition
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg037, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm
     * @tc.expected: menuLayoutAlgorithm is not null
     */
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODEID, "menu");
    ASSERT_NE(menuLayoutAlgorithm, nullptr);
    SizeF size(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT);

    /**
     * @tc.steps: step2. the placement of menuLayoutAlgorithm is Placement::BOTTOM_LEFT and menu position is normal
     * @tc.expected: CheckPosition result is true
     */
    menuLayoutAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    menuLayoutAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuLayoutAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    OffsetF position = OffsetF(POSITION_OFFSET, OFFSET_FORTH);
    menuLayoutAlgorithm->targetOffset_ = OffsetF(OFFSET_THIRD, OFFSET_THIRD);
    auto result = menuLayoutAlgorithm->CheckPosition(position, size);
    EXPECT_TRUE(result);

    /**
     * @tc.steps: step3. the placement of menuLayoutAlgorithm is Placement::TOP_LEFT and menu position is normal
     * @tc.expected: CheckPosition result is true
     */
    position = OffsetF(POSITION_OFFSET, OFFSET_FIFTH);
    menuLayoutAlgorithm->placement_ = Placement::TOP_LEFT;
    result = menuLayoutAlgorithm->CheckPosition(position, size);
    EXPECT_TRUE(result);

    /**
     * @tc.steps: step4. the placement of menuLayoutAlgorithm is Placement::LEFT_TOP and menu position is normal
     * @tc.expected: CheckPosition result is true
     */
    position = OffsetF(OFFSET_FIFTH, OFFSET_THIRD);
    menuLayoutAlgorithm->placement_ = Placement::LEFT_TOP;
    result = menuLayoutAlgorithm->CheckPosition(position, size);
    EXPECT_TRUE(result);

    /**
     * @tc.steps: step5. the placement of menuLayoutAlgorithm is Placement::NONE
     * @tc.expected: CheckPosition result is false
     */
    menuLayoutAlgorithm->placement_ = Placement::NONE;
    result = menuLayoutAlgorithm->CheckPosition(position, size);
    EXPECT_FALSE(result);

    /**
     * @tc.steps: step6. the placement of menuLayoutAlgorithm is Placement::BOTTOM_LEFT and menu position is not normal
     * @tc.expected: CheckPosition result is false
     */
    menuLayoutAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    position = OffsetF(POSITION_OFFSET, FULL_SCREEN_HEIGHT);
    result = menuLayoutAlgorithm->CheckPosition(position, size);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg038
 * @tc.desc: Test InitTargetSizeAndPosition
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg038, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm and target is null
     * @tc.expected: menuLayoutAlgorithm is not null
     */
    auto nodeId =  ElementRegister::GetInstance()->MakeUniqueId();
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(nodeId, TEXT_TAG, MenuType::CONTEXT_MENU);
    RefPtr<MenuLayoutAlgorithm> menuLayoutAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(nodeId, "menu");
    ASSERT_NE(menuLayoutAlgorithm, nullptr);

    menuLayoutAlgorithm->InitTargetSizeAndPosition(nullptr, true, menuPattern);
    menuLayoutAlgorithm->targetNodeId_ = nodeId;
    menuLayoutAlgorithm->targetTag_ = "text";
    auto target = FrameNode::GetOrCreateFrameNode("text", nodeId, []() { return AceType::MakeRefPtr<Pattern>(); });
    ASSERT_NE(target, nullptr);

    /**
     * @tc.steps: step2. target is null but the geometry node of target is null
     */
    menuLayoutAlgorithm->InitTargetSizeAndPosition(nullptr, true, menuPattern);

    /**
     * @tc.steps: step3. layoutWrapper, target node and the geometry node of target is not null, isContextMenu is false
     * @tc.expected: targetOffset_ is OffsetF(0.0f, 0.0f)
     */
    std::vector<SelectParam> params;
    params.emplace_back(std::make_pair("MenuItem", "Icon"));
    auto frameNode = MenuView::Create(params, 1, EMPTY_TEXT);
    ASSERT_NE(frameNode, nullptr);
    auto menuGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    LayoutWrapperNode* layoutWrapper =
        new LayoutWrapperNode(frameNode, menuGeometryNode, frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    GeometryProperty geometryProperty;
    geometryProperty.rect_ = RectF(0.0f, 0.0f, 0.0f, 0.0f);
    geometryNode->frame_ = geometryProperty;
    target->geometryNode_ = geometryNode;
    menuLayoutAlgorithm->InitTargetSizeAndPosition(layoutWrapper, false, menuPattern);
    EXPECT_EQ(menuLayoutAlgorithm->targetOffset_, OffsetF(0.0f, 0.0f));

    /**
     * @tc.steps: step4. layoutWrapper, target and the geometry node of target is not null, isContextMenu and
     * isContainerModal is true
     * @tc.expected: targetOffset_ is OffsetF(-5.0f, -38.0f)
     */
    MockPipelineBase::GetCurrent()->SetWindowModal(WindowModal::CONTAINER_MODAL);
    MockPipelineBase::GetCurrent()->windowManager_ = AceType::MakeRefPtr<WindowManager>();
    MockPipelineBase::GetCurrent()->windowManager_->SetWindowGetModeCallBack(
        []() -> WindowMode { return WindowMode::WINDOW_MODE_FLOATING; });

    menuLayoutAlgorithm->InitTargetSizeAndPosition(layoutWrapper, true, menuPattern);
    auto pipelineContext = menuLayoutAlgorithm->GetCurrentPipelineContext();
    auto windowGlobalRect = pipelineContext->GetDisplayWindowRectInfo();
    float windowsOffsetX = static_cast<float>(windowGlobalRect.GetOffset().GetX());
    float windowsOffsetY = static_cast<float>(windowGlobalRect.GetOffset().GetY());
    OffsetF offset = OffsetF(windowsOffsetX, windowsOffsetY);
    OffsetF offset2 = menuLayoutAlgorithm->GetMenuWrapperOffset(layoutWrapper);
    offset -= offset2;
    EXPECT_EQ(menuLayoutAlgorithm->targetOffset_, offset);

    /**
     * @tc.steps: step5. layoutWrapper, target and the geometry node of target is not null, isContextMenu is false
     * @tc.expected: targetOffset_ is OffsetF(-5.0f, -38.0f)
     */
    menuLayoutAlgorithm->InitTargetSizeAndPosition(layoutWrapper, false, menuPattern);
    EXPECT_EQ(menuLayoutAlgorithm->targetOffset_,
        OffsetF(-static_cast<float>(CONTAINER_BORDER_WIDTH.ConvertToPx() + CONTENT_PADDING.ConvertToPx()),
            -static_cast<float>(CONTAINER_TITLE_HEIGHT.ConvertToPx() + CONTAINER_BORDER_WIDTH.ConvertToPx())));
    delete layoutWrapper;
    layoutWrapper = nullptr;
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg039
 * @tc.desc: Test GetIfNeedArrow
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg039, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm
     * @tc.expected: menuLayoutAlgorithm is not null
     */
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(NODEID, TEXT_TAG, MenuType::CONTEXT_MENU);
    auto contextMenu = AceType::MakeRefPtr<FrameNode>(MENU_TAG, -1, menuPattern);
    auto menuAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODEID, TEXT_TAG);
    ASSERT_TRUE(menuAlgorithm);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<MenuLayoutProperty>();
    auto* layoutWrapper = new LayoutWrapperNode(contextMenu, geometryNode, layoutProp);
    const SizeF menuSize = SizeF(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT);
    /**
     * @tc.steps: step2. excute GetIfNeedArrow
     * @tc.expected: ifNeedArrow is as expected.
     */
    menuAlgorithm->GetPaintProperty(layoutWrapper)->UpdateEnableArrow(true);
    auto result = menuAlgorithm->GetIfNeedArrow(layoutWrapper, menuSize);
    EXPECT_FALSE(result);
    menuAlgorithm->placement_ = Placement::LEFT;
    result = menuAlgorithm->GetIfNeedArrow(layoutWrapper, menuSize);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg040
 * @tc.desc: Test UpdatePropArrowOffset
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg040, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm
     * @tc.expected: menuLayoutAlgorithm is not null
     */
    auto menuAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODEID, TEXT_TAG);
    ASSERT_TRUE(menuAlgorithm);
    std::unordered_set<Placement> placements = { Placement::TOP, Placement::TOP_LEFT, Placement::TOP_RIGHT,
        Placement::BOTTOM, Placement::BOTTOM_LEFT, Placement::BOTTOM_RIGHT, Placement::LEFT, Placement::LEFT_TOP,
        Placement::LEFT_BOTTOM, Placement::RIGHT, Placement::RIGHT_TOP, Placement::RIGHT_BOTTOM };
    std::unordered_set<Placement> offsetCondition1 = { Placement::LEFT, Placement::RIGHT, Placement::TOP,
        Placement::BOTTOM };
    std::unordered_set<Placement> offsetCondition2 = { Placement::TOP_LEFT, Placement::BOTTOM_LEFT, Placement::LEFT_TOP,
        Placement::RIGHT_TOP };
    std::unordered_set<Placement> offsetCondition3 = { Placement::TOP_RIGHT, Placement::BOTTOM_RIGHT,
        Placement::LEFT_BOTTOM, Placement::RIGHT_BOTTOM };
    /**
     * @tc.steps: step2. input unit is px, excute UpdatePropArrowOffset
     * @tc.expected: propArrowOffset_ is as expected.
     */
    menuAlgorithm->propArrowOffset_ = Dimension(10, DimensionUnit::PX);
    menuAlgorithm->UpdatePropArrowOffset();
    EXPECT_EQ(menuAlgorithm->propArrowOffset_, Dimension(10, DimensionUnit::PX));
    /**
     * @tc.steps: step3. input unit is percent, excute UpdatePropArrowOffset
     * @tc.expected: propArrowOffset_ is as expected.
     */
    menuAlgorithm->propArrowOffset_ = Dimension(0.5, DimensionUnit::PERCENT);
    menuAlgorithm->UpdatePropArrowOffset();
    EXPECT_EQ(menuAlgorithm->propArrowOffset_, Dimension(0.5, DimensionUnit::PERCENT));
    /**
     * @tc.steps: step4. intput is invalid, excute UpdatePropArrowOffset
     * @tc.expected: propArrowOffset_ is as expected.
     */
    for (Placement placementValue : placements) {
        menuAlgorithm->propArrowOffset_ = Dimension(-1, DimensionUnit::PX);
        menuAlgorithm->arrowPlacement_ = placementValue;
        menuAlgorithm->UpdatePropArrowOffset();
        if (offsetCondition1.find(placementValue) != offsetCondition1.end()) {
            EXPECT_EQ(menuAlgorithm->propArrowOffset_, ARROW_HALF_PERCENT_VALUE);
        }
        if (offsetCondition2.find(placementValue) != offsetCondition2.end()) {
            EXPECT_EQ(menuAlgorithm->propArrowOffset_, ARROW_ZERO_PERCENT_VALUE);
        }
        if (offsetCondition3.find(placementValue) != offsetCondition3.end()) {
            EXPECT_EQ(menuAlgorithm->propArrowOffset_, ARROW_ONE_HUNDRED_PERCENT_VALUE);
        }
    }
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg041
 * @tc.desc: Test GetArrowPositionWithPlacement
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg041, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuLayoutAlgorithm
     * @tc.expected: menuLayoutAlgorithm is not null
     */
    auto menuAlgorithm = AceType::MakeRefPtr<MenuLayoutAlgorithm>(NODEID, TEXT_TAG);
    ASSERT_TRUE(menuAlgorithm);
    const SizeF menuSize = SizeF(MENU_SIZE_WIDTH, MENU_SIZE_HEIGHT);
    std::unordered_set<Placement> placements = { Placement::TOP, Placement::TOP_LEFT, Placement::TOP_RIGHT,
        Placement::BOTTOM, Placement::BOTTOM_LEFT, Placement::BOTTOM_RIGHT, Placement::LEFT, Placement::LEFT_TOP,
        Placement::LEFT_BOTTOM, Placement::RIGHT, Placement::RIGHT_TOP, Placement::RIGHT_BOTTOM };
    std::unordered_set<Placement> positionCondition1 = { Placement::TOP, Placement::TOP_LEFT, Placement::TOP_RIGHT };
    std::unordered_set<Placement> positionCondition2 = { Placement::BOTTOM, Placement::BOTTOM_LEFT,
        Placement::BOTTOM_RIGHT };
    std::unordered_set<Placement> positionCondition3 = { Placement::LEFT, Placement::LEFT_TOP, Placement::LEFT_BOTTOM };
    std::unordered_set<Placement> positionCondition4 = { Placement::RIGHT, Placement::RIGHT_TOP,
        Placement::RIGHT_BOTTOM };
    /**
     * @tc.steps: step2. excute GetArrowPositionWithPlacement
     * @tc.expected: arrowPosition is as expected.
     */
    float arrowOffsetMax = 0.0f;
    for (Placement placementValue : placements) {
        if (menuAlgorithm->setHorizontal_.find(placementValue) != menuAlgorithm->setHorizontal_.end()) {
            arrowOffsetMax = menuSize.Height() - menuAlgorithm->menuRadius_ * 2 - menuAlgorithm->arrowWidth_;
        }
        if (menuAlgorithm->setVertical_.find(placementValue) != menuAlgorithm->setVertical_.end()) {
            arrowOffsetMax = menuSize.Width() - menuAlgorithm->menuRadius_ * 2 - menuAlgorithm->arrowWidth_;
        }

        menuAlgorithm->propArrowOffset_ = Dimension(0.5, DimensionUnit::PX);
        menuAlgorithm->arrowPlacement_ = placementValue;
        auto result = menuAlgorithm->GetArrowPositionWithPlacement(menuSize);
        EXPECT_EQ(menuAlgorithm->propArrowOffset_, Dimension(0.5, DimensionUnit::PX));
        auto arrowOffsetValue = (menuAlgorithm->propArrowOffset_).ConvertToPx();
        EXPECT_EQ(menuAlgorithm->arrowOffset_, arrowOffsetValue);
        if (positionCondition1.find(placementValue) != positionCondition1.end()) {
            EXPECT_EQ(result, OffsetF(arrowOffsetValue, menuSize.Height() + ARROW_HIGHT.ConvertToPx()));
        }
        if (positionCondition2.find(placementValue) != positionCondition2.end()) {
            EXPECT_EQ(result, OffsetF(arrowOffsetValue, -ARROW_HIGHT.ConvertToPx()));
        }
        if (positionCondition3.find(placementValue) != positionCondition3.end()) {
            EXPECT_EQ(result, OffsetF(menuSize.Width() + ARROW_HIGHT.ConvertToPx(), arrowOffsetValue));
        }
        if (positionCondition4.find(placementValue) != positionCondition4.end()) {
            EXPECT_EQ(result, OffsetF(-ARROW_HIGHT.ConvertToPx(), arrowOffsetValue));
        }
    }
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4200
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4200, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, OFFSET_THIRD);
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the window can accommodate preview, placement is default, layout preview and menu
     * @tc.expected: preview size is fixed [PREVIEW_WIDTH, TARGET_SIZE_HEIGHT], menu and preview bottom border
     * distance TARGET_SECURITY, align the menu with the left border of the preview
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM_LEFT);
    menuAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2, OFFSET_THIRD);
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(PREVIEW_WIDTH, TARGET_SIZE_HEIGHT));
    auto expectMenuOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2,
        OFFSET_THIRD + TARGET_SIZE_HEIGHT + TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));

    /**
     * @tc.steps: step3. the window can accommodate preview, placement is BOTTOM, layout preview and menu
     * @tc.expected: menu and preview bottom border distance TARGET_SECURITY, align the menu with the preview in the
     * center
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    RefPtr<MenuTheme> menuTheme = AceType::MakeRefPtr<MenuTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(menuTheme));
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM);
    menuAlgorithm->placement_ = Placement::BOTTOM;
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    expectMenuOffset = OffsetF(OFFSET_FORTH, OFFSET_THIRD + TARGET_SIZE_HEIGHT + TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    EXPECT_EQ(menuPattern->endOffset_, expectMenuOffset);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4300
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4300, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, GREATER_WINDOW_MENU_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);
    /**
     * @tc.steps: step2. the height of window can't accommodate preview and menu, placement is default,
     * layout preview and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH, SCALE_OFFSET], scale the height of the menu
     * to SCALE_MENU_HEIGHT
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM_LEFT);
    menuAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset =
        OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH, SCALE_OFFSET));
    auto expectMenuOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2,
        SCALE_OFFSET + TARGET_SECURITY.ConvertToPx() + PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, SCALE_MENU_HEIGHT));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4400
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4400, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, OFFSET_FORTH);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the window can accommodate preview, placement is TOP_LEFT, layout preview and menu
     * @tc.expected: preview size is fixed [PREVIEW_WIDTH, TARGET_SIZE_HEIGHT], menu and preview top border
     * distance TARGET_SECURITY, align the menu with the left border of the preview
     */
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    menuAlgorithm->placement_ = Placement::TOP_LEFT;
    layoutProperty->UpdateMenuPlacement(Placement::TOP_LEFT);
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2, OFFSET_FORTH);
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(PREVIEW_WIDTH, TARGET_SIZE_HEIGHT));
    auto expectMenuOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2,
        OFFSET_FORTH - TARGET_SIZE_HEIGHT - TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));

    /**
     * @tc.steps: step3. the window can accommodate preview, placement is TOP, layout preview and menu
     * @tc.expected: menu and preview bottom border distance TARGET_SECURITY, align the menu with the preview in the
     * center
     */
    layoutProperty->UpdateMenuPlacement(Placement::TOP);
    menuAlgorithm->placement_ = Placement::TOP;
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    expectMenuOffset = OffsetF(OFFSET_FORTH, OFFSET_FORTH - TARGET_SIZE_HEIGHT - TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4500
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4500, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, GREATER_WINDOW_MENU_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the height of window can't accommodate preview and menu, placement is TOP_LEFT,
     * layout preview and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH, SCALE_OFFSET], scale the height of the menu
     * to SCALE_MENU_HEIGHT
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::TOP_LEFT);
    menuAlgorithm->placement_ = Placement::TOP_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2,
        SCALE_MENU_HEIGHT + TARGET_SECURITY.ConvertToPx() + PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH, SCALE_OFFSET));
    auto expectMenuOffset =
        OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, SCALE_MENU_HEIGHT));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4600
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4600, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device type is TABLET, create menu node, preview node and menuLayoutAlgorithm, then set the
     * initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_HEIGHT;
    SystemProperties::SetDeviceType(DeviceType::TABLET);
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, OFFSET_THIRD));
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_HEIGHT, FULL_SCREEN_WIDTH);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_THIRD, OFFSET_THIRD);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the window can accommodate preview, placement is RIGHT_TOP, layout preview and menu
     * @tc.expected: menu and preview right border distance TARGET_SECURITY, align the menu with the top border of the
     * preview
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::RIGHT_TOP);
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_HEIGHT, FULL_SCREEN_WIDTH));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(0.0f, OFFSET_THIRD + (TARGET_SIZE_HEIGHT - OFFSET_THIRD) / 2);
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(PREVIEW_WIDTH, OFFSET_THIRD));
    auto expectMenuOffset =
        OffsetF(PREVIEW_WIDTH + TARGET_SECURITY.ConvertToPx(), OFFSET_THIRD + (TARGET_SIZE_HEIGHT - OFFSET_THIRD) / 2);
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));

    /**
     * @tc.steps: step3. the window can accommodate preview, placement is LEFT_TOP, layout preview and menu
     * @tc.expected: menu and preview left border distance TARGET_SECURITY, align the menu with the top border of the
     * preview
     */
    layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::LEFT_TOP);
    menuAlgorithm->placement_ = Placement::LEFT_TOP;
    menuAlgorithm->targetOffset_ = OffsetF(FULL_SCREEN_HEIGHT - OFFSET_THIRD, OFFSET_THIRD);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    expectMenuOffset =
        OffsetF(FULL_SCREEN_HEIGHT - PREVIEW_WIDTH - TARGET_SECURITY.ConvertToPx() - TARGET_SIZE_WIDTH, OFFSET_THIRD);
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4700
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4700, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device type is TABLET, create menu node, preview node and menuLayoutAlgorithm, then set the
     * initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::SetDeviceType(DeviceType::TABLET);
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(MENU_SIZE_WIDTH_SECOND, TARGET_SIZE_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, OFFSET_THIRD));
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_THIRD, OFFSET_THIRD);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the window can not accommodate preview, placement is default, layout preview and menu
     * @tc.expected: preview size is fixed [SCALE_PREVIEW_WIDTH_SECOND, SCALE_MENU_HEIGHT_SECOND], menu and preview
     * right border distance TARGET_SECURITY, align the menu with the top border of the preview
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::RIGHT_TOP);
    menuAlgorithm->placement_ = Placement::RIGHT_TOP;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(0.0f, OFFSET_THIRD + (TARGET_SIZE_HEIGHT - SCALE_MENU_HEIGHT_SECOND) / 2);
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH_SECOND, SCALE_MENU_HEIGHT_SECOND));
    auto expectMenuOffset = OffsetF(SCALE_PREVIEW_WIDTH_SECOND + TARGET_SECURITY.ConvertToPx(),
        OFFSET_THIRD + (TARGET_SIZE_HEIGHT - SCALE_MENU_HEIGHT_SECOND) / 2);
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(MENU_SIZE_WIDTH_SECOND, TARGET_SIZE_HEIGHT));

    /**
     * @tc.steps: step3. the window can not accommodate preview, placement is RIGHT, layout preview and menu
     * @tc.expected: menu and preview bottom right distance TARGET_SECURITY, align the menu with the preview in the
     * center
     */
    layoutProperty->UpdateMenuPlacement(Placement::RIGHT);
    menuAlgorithm->placement_ = Placement::RIGHT;
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    menuGeometryNode->SetFrameSize(SizeF(MENU_SIZE_WIDTH_SECOND, TARGET_SIZE_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    expectMenuOffset = OffsetF(SCALE_PREVIEW_WIDTH_SECOND + TARGET_SECURITY.ConvertToPx(), OFFSET_THIRD);
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4800
 * @tc.desc: Test Measure with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4800, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device type is PHONE, create menu node, preview node and menuLayoutAlgorithm, then set the
     * initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);

    LayoutWrapperNode layoutWrapper(menuNode, menuGeometryNode, menuLayoutProperty);
    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    RefPtr<MenuTheme> menuTheme = AceType::MakeRefPtr<MenuTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(menuTheme));

    /**
     * @tc.steps: step2. call Measure function
     * @tc.expected: the placement of the menu defaults to BOTTOM_LEFT
     */
    menuAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(menuAlgorithm->placement_, OHOS::Ace::Placement::BOTTOM_LEFT);
    ASSERT_TRUE(menuLayoutProperty->GetMenuPlacement().has_value());
    EXPECT_EQ(menuLayoutProperty->GetMenuPlacement().value(), OHOS::Ace::Placement::BOTTOM_LEFT);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg4900
 * @tc.desc: Test Measure with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg4900, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device type is TABLET, create menu node, preview node and menuLayoutAlgorithm, then set the
     * initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::SetDeviceType(DeviceType::TABLET);
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    LayoutWrapperNode layoutWrapper(menuNode, menuGeometryNode, menuLayoutProperty);
    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();

    /**
     * @tc.steps: step2. call Measure function
     * @tc.expected: the placement of the menu defaults to RIGHT_TOP
     */
    menuAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(menuAlgorithm->placement_, OHOS::Ace::Placement::RIGHT_TOP);
    ASSERT_TRUE(menuLayoutProperty->GetMenuPlacement().has_value());
    EXPECT_EQ(menuLayoutProperty->GetMenuPlacement().value(), OHOS::Ace::Placement::RIGHT_TOP);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5000
 * @tc.desc: Test Measure with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5000, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device type is PHONE and orientation is LANDSCAPE, create menu node, preview node and
     * menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    LayoutWrapperNode layoutWrapper(menuNode, menuGeometryNode, menuLayoutProperty);
    layoutWrapper.GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(FULL_SCREEN_WIDTH), CalcLength(FULL_SCREEN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();

    /**
     * @tc.steps: step2. call Measure function
     * @tc.expected: the placement of the menu defaults to RIGHT_TOP
     */
    menuAlgorithm->Measure(&layoutWrapper);
    EXPECT_EQ(menuAlgorithm->placement_, OHOS::Ace::Placement::RIGHT_TOP);
    ASSERT_TRUE(menuLayoutProperty->GetMenuPlacement().has_value());
    EXPECT_EQ(menuLayoutProperty->GetMenuPlacement().value(), OHOS::Ace::Placement::RIGHT_TOP);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5100
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetImagePreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the width of window can't accommodate preview and menu, placement is BOTTOM_LEFT,
     * layout preview and menu
     * @tc.expected: scale the preview to [FULL_SCREEN_WIDTH, SCALE_PREVIEW_HEIGHT]
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM_LEFT);
    menuAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));

    previewGeometryNode->SetFrameSize(SizeF(GREATER_WINDOW_PREVIEW_WIDTH, OFFSET_FORTH));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(0.0f, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(FULL_SCREEN_WIDTH, SCALE_PREVIEW_HEIGHT));
    auto expectMenuOffset =
        OffsetF(0.0f, SCALE_PREVIEW_HEIGHT + TARGET_SECURITY.ConvertToPx() + PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5200
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5200, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetImagePreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the width of window can't accommodate preview and menu, placement is TOP_LEFT,
     * layout preview and menu
     * @tc.expected: scale the preview to [FULL_SCREEN_WIDTH, SCALE_PREVIEW_HEIGHT]
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::TOP_LEFT);
    menuAlgorithm->placement_ = Placement::TOP_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    previewGeometryNode->SetFrameSize(SizeF(GREATER_WINDOW_PREVIEW_WIDTH, OFFSET_FORTH));

    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectMenuOffset = OffsetF(0.0f, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto expectPreviewOffset =
        OffsetF(0.0f, TARGET_SIZE_HEIGHT + TARGET_SECURITY.ConvertToPx() + PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(FULL_SCREEN_WIDTH, SCALE_PREVIEW_HEIGHT));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5300
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5300, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the height of window can't accommodate preview and menu, and the menu height is less than half
     * the height of the preview contentt , placement is BOTTOM_LEFT, layout preview and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH, SCALE_PREVIEW_HEIGHT_THIRD]
     */
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, GREATER_WINDOW_PREVIEW_HEIGHT));
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM_LEFT);
    menuAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset =
        OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH, SCALE_PREVIEW_HEIGHT_THIRD));
    auto expectMenuOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2,
        FULL_SCREEN_HEIGHT - PORTRAIT_BOTTOM_SECURITY.ConvertToPx() - TARGET_SIZE_HEIGHT);
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));

    /**
     * @tc.steps: step3. the height of window can't accommodate preview and menu, and the menu height is less than half
     * the height of the preview contentt , placement is TOP_LEFT, layout preview and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH, SCALE_PREVIEW_HEIGHT_THIRD], menu is on top
     */
    layoutProperty->UpdateMenuPlacement(Placement::TOP_LEFT);
    menuAlgorithm->placement_ = Placement::TOP_LEFT;
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2,
        PORTRAIT_TOP_SECURITY.ConvertToPx() + TARGET_SIZE_HEIGHT + TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH, SCALE_PREVIEW_HEIGHT_THIRD));
    expectMenuOffset =
        OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5400
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5400, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper(SizeF(TARGET_SIZE_WIDTH, GREATER_HALF_PREVIEW_MENUITEM_HEIGHT));
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the height of window can't accommodate preview and menu, and the menu height is less than half
     * height of the preview content, but the item total size is great than it , placement is BOTTOM_LEFT,
     * layout preview and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH_THIRD, SCALE_PREVIEW_HEIGHT_SECOND]
     */
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM_LEFT);
    menuAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));

    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, GREATER_WINDOW_PREVIEW_HEIGHT_SECOND));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(
        OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_THIRD) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH_THIRD, SCALE_PREVIEW_HEIGHT_SECOND));
    auto expectMenuOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_THIRD) / 2,
        PORTRAIT_TOP_SECURITY.ConvertToPx() + SCALE_PREVIEW_HEIGHT_SECOND + TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, SCALE_MENU_HEIGHT_THIRD));

    /**
     * @tc.steps: step3. the height of window can't accommodate preview and menu, and the menu height is less than half
     * height of the preview content, but the item total size is great than it , placement is TOP_LEFT, layout preview
     * and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH_THIRD, SCALE_PREVIEW_HEIGHT_SECOND]
     */
    layoutProperty->UpdateMenuPlacement(Placement::TOP_LEFT);
    menuAlgorithm->placement_ = Placement::TOP_LEFT;

    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, GREATER_WINDOW_PREVIEW_HEIGHT_SECOND));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_THIRD) / 2,
        PORTRAIT_TOP_SECURITY.ConvertToPx() + SCALE_MENU_HEIGHT_THIRD + TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH_THIRD, SCALE_PREVIEW_HEIGHT_SECOND));
    expectMenuOffset = OffsetF(
        OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_THIRD) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5500
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5500, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, OFFSET_FORTH));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, OFFSET_THIRD);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the window can accommodate preview, placement is TOP_LEFT, but menu is in top security
     * layout preview and menu
     * @tc.expected: preview offset is fixed [OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2,
     * PORTRAIT_TOP_SECURITY.ConvertToPx() + OFFSET_FORTH + TARGET_SECURITY.ConvertToPx()],
     * menu offset is [OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx()]
     */
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    menuAlgorithm->placement_ = Placement::TOP_LEFT;
    layoutProperty->UpdateMenuPlacement(Placement::TOP_LEFT);
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2,
        PORTRAIT_TOP_SECURITY.ConvertToPx() + OFFSET_FORTH + TARGET_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(PREVIEW_WIDTH, TARGET_SIZE_HEIGHT));
    auto expectMenuOffset =
        OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, OFFSET_FORTH));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5600
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5600, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, OFFSET_FORTH));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    previewGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, FULL_SCREEN_HEIGHT - OFFSET_THIRD);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the window can accommodate preview, placement is BOTTOM_LEFT, but menu is in top security
     * layout preview and menu
     * @tc.expected: preview offset is fixed [OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2, FULL_SCREEN_HEIGHT
     * - PORTRAIT_BOTTOM_SECURITY.ConvertToPx() - OFFSET_FORTH - TARGET_SECURITY.ConvertToPx() - TARGET_SIZE_HEIGHT],
     * menu offset is [OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2, FULL_SCREEN_HEIGHT -
     * PORTRAIT_BOTTOM_SECURITY.ConvertToPx() - OFFSET_FORTH]
     */
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    menuAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM_LEFT);
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2,
        FULL_SCREEN_HEIGHT - PORTRAIT_BOTTOM_SECURITY.ConvertToPx() - OFFSET_FORTH - TARGET_SECURITY.ConvertToPx() -
            TARGET_SIZE_HEIGHT);
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(PREVIEW_WIDTH, TARGET_SIZE_HEIGHT));
    auto expectMenuOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - PREVIEW_WIDTH) / 2,
        FULL_SCREEN_HEIGHT - PORTRAIT_BOTTOM_SECURITY.ConvertToPx() - OFFSET_FORTH);
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, OFFSET_FORTH));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5700
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5700, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetImagePreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, GREATER_WINDOW_MENU_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the width and height of window can't accommodate preview and menu, placement is BOTTOM_LEFT,
     * layout preview and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH_FORTH, SCALE_PREVIEW_HEIGHT_FORTH]
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::BOTTOM_LEFT);
    menuAlgorithm->placement_ = Placement::BOTTOM_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));

    previewGeometryNode->SetFrameSize(SizeF(GREATER_WINDOW_PREVIEW_WIDTH_SECOND, PREVIEW_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(
        OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_FORTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH_FORTH, SCALE_PREVIEW_HEIGHT_FORTH));
    auto expectMenuOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_FORTH) / 2,
        SCALE_PREVIEW_HEIGHT_FORTH + TARGET_SECURITY.ConvertToPx() + PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, SCALE_MENU_HEIGHT));
}

/**
 * @tc.name: MenuLayoutAlgorithmTestNg5800
 * @tc.desc: Test Layout with preview content
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutAlgorithmTestNg5800, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menu node, preview node and menuLayoutAlgorithm, then set the initial properties
     * @tc.expected: menu node, preview node and menuLayoutAlgorithm are not null
     */
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto menuWrapperNode = GetImagePreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    menuGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, GREATER_WINDOW_MENU_HEIGHT));
    auto previewNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(1));
    auto previewGeometryNode = previewNode->GetGeometryNode();
    ASSERT_NE(previewGeometryNode, nullptr);
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    menuAlgorithm->wrapperSize_ = SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
    menuAlgorithm->targetSize_ = SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT);
    menuAlgorithm->targetOffset_ = OffsetF(OFFSET_FORTH, 0.0f);
    menuAlgorithm->targetSecurity_ = TARGET_SECURITY.ConvertToPx();
    menuAlgorithm->previewScale_ = 1.0f;
    auto pipelineContext = menuAlgorithm->GetCurrentPipelineContext();
    ASSERT_NE(pipelineContext, nullptr);

    /**
     * @tc.steps: step2. the width and height of window can't accommodate preview and menu, placement is TOP_LEFT,
     * layout preview and menu
     * @tc.expected: scale the preview to [SCALE_PREVIEW_WIDTH_FORTH, SCALE_PREVIEW_HEIGHT_FORTH], the size of menu is
     * [TARGET_SIZE_WIDTH, SCALE_MENU_HEIGHT]
     */
    auto layoutProperty = AceType::DynamicCast<MenuLayoutProperty>(menuNode->GetLayoutProperty());
    layoutProperty->UpdateMenuPlacement(Placement::TOP_LEFT);
    menuAlgorithm->placement_ = Placement::TOP_LEFT;
    pipelineContext->SetDisplayWindowRectInfo(Rect(0.0f, 0.0f, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));

    previewGeometryNode->SetFrameSize(SizeF(GREATER_WINDOW_PREVIEW_WIDTH_SECOND, PREVIEW_HEIGHT));
    menuAlgorithm->Layout(AceType::RawPtr(menuNode));
    auto expectPreviewOffset = OffsetF(OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_FORTH) / 2,
        PORTRAIT_TOP_SECURITY.ConvertToPx() + TARGET_SECURITY.ConvertToPx() + SCALE_MENU_HEIGHT);
    EXPECT_EQ(previewGeometryNode->GetFrameOffset(), expectPreviewOffset);
    EXPECT_EQ(previewGeometryNode->GetFrameSize(), SizeF(SCALE_PREVIEW_WIDTH_FORTH, SCALE_PREVIEW_HEIGHT_FORTH));
    auto expectMenuOffset = OffsetF(
        OFFSET_FORTH + (TARGET_SIZE_WIDTH - SCALE_PREVIEW_WIDTH_FORTH) / 2, PORTRAIT_TOP_SECURITY.ConvertToPx());
    EXPECT_EQ(menuGeometryNode->GetFrameOffset(), expectMenuOffset);
    EXPECT_EQ(menuGeometryNode->GetFrameSize(), SizeF(TARGET_SIZE_WIDTH, SCALE_MENU_HEIGHT));
}

/**
 * @tc.name: MenuItemGroupLayoutAlgorithmTestNg001
 * @tc.desc: Test MenuItemGroup measure algorithm.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemGroupLayoutAlgorithmTestNg001, TestSize.Level1)
{
    // create menu item group
    auto menuItemGroupPattern = AceType::MakeRefPtr<MenuItemGroupPattern>();
    auto menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    menuItemGroup->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->OnModifyDone();
    auto algorithm = AceType::MakeRefPtr<MenuItemGroupLayoutAlgorithm>(-1, -1, 0);
    ASSERT_TRUE(algorithm);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto* layoutWrapper = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    auto props = layoutWrapper->GetLayoutProperty();
    props->UpdateLayoutConstraint(parentLayoutConstraint);
    props->UpdateContentConstraint();
    // create menu item
    for (int32_t i = 0; i < 3; ++i) {
        auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
        auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, itemPattern);
        auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
        auto childWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);
        layoutWrapper->AppendChild(childWrapper);
    }
    // set selectTheme to themeManager before using themeManager to get selectTheme
    // test measure
    algorithm->Measure(layoutWrapper);
    // @tc.expected: menu content width = item width, height = sum(item height)
    auto expectedSize = SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT * 3);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), expectedSize);

    // test layout
    algorithm->Layout(layoutWrapper);
    // @tc.expected: menu item offset y = item height * (index - 1)
    OffsetF offset;
    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        EXPECT_EQ(child->GetGeometryNode()->GetMarginFrameOffset(), offset);
        offset.AddY(MENU_SIZE_HEIGHT / 3);
    }
    parentLayoutConstraint.selfIdealSize.SetWidth(10);
    props->UpdateLayoutConstraint(parentLayoutConstraint);
    props->UpdateContentConstraint();
    algorithm->headerIndex_ = 0;
    algorithm->footerIndex_ = 0;
    algorithm->Measure(layoutWrapper);
    algorithm->Layout(layoutWrapper);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize().Height(), MENU_ITEM_SIZE_HEIGHT * 5);
    algorithm->needHeaderPadding_ = true;
    algorithm->LayoutHeader(layoutWrapper);
    algorithm->NeedHeaderPadding(menuItemGroup);
    algorithm->UpdateHeaderAndFooterMargin(layoutWrapper);
}

/**
 * @tc.name: MenuItemGroupLayoutAlgorithmTestNg002
 * @tc.desc: Test MenuItemGroup measure UpdateHeaderAndFooterMargin.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemGroupLayoutAlgorithmTestNg002, TestSize.Level1)
{
    // create menu item group
    auto menuItemGroupPattern = AceType::MakeRefPtr<MenuItemGroupPattern>();
    menuItemGroupPattern->hasSelectIcon_ = true;
    auto menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    menuItemGroup->MountToParent(wrapperNode);
    auto wrapperPattern = wrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);
    wrapperPattern->OnModifyDone();
    auto algorithm = AceType::MakeRefPtr<MenuItemGroupLayoutAlgorithm>(-1, -1, 0);
    ASSERT_TRUE(algorithm);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutProp = AceType::MakeRefPtr<LayoutProperty>();
    auto* layoutWrapper = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);
    algorithm->UpdateHeaderAndFooterMargin(layoutWrapper);
    menuItemGroupPattern->hasStartIcon_ = true;
    menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    layoutWrapper = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);
    for (int32_t i = 0; i < 3; ++i) {
        auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
        auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, itemPattern);
        auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
        auto childWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);
        layoutWrapper->AppendChild(childWrapper);
    }
    algorithm->UpdateHeaderAndFooterMargin(layoutWrapper);
    algorithm->headerIndex_ = 0;
    algorithm->UpdateHeaderAndFooterMargin(layoutWrapper);
    algorithm->footerIndex_ = 0;
    algorithm->UpdateHeaderAndFooterMargin(layoutWrapper);
    menuItemGroupPattern->hasSelectIcon_ = false;
    menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    layoutWrapper = new LayoutWrapperNode(menuItemGroup, geometryNode, layoutProp);
    for (int32_t i = 0; i < 3; ++i) {
        auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
        auto menuItem = AceType::MakeRefPtr<FrameNode>("", -1, itemPattern);
        auto itemGeoNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeoNode->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
        auto childWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(menuItem, itemGeoNode, layoutProp);
        layoutWrapper->AppendChild(childWrapper);
    }
    algorithm->UpdateHeaderAndFooterMargin(layoutWrapper);
    algorithm->Measure(layoutWrapper);
    auto expectedSize = SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT * 3);
}

/**
 * @tc.name: MenuItemGroupPaintMethod001
 * @tc.desc: Test MenuItemGroup GetOverlayDrawFunction.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemGroupPaintMethod001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare paint method object.
     */
    RefPtr<MenuItemGroupPaintProperty> paintProp = AceType::MakeRefPtr<MenuItemGroupPaintProperty>();
    RefPtr<MenuItemGroupPaintMethod> paintMethod = AceType::MakeRefPtr<MenuItemGroupPaintMethod>();
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).Times(AtLeast(1));
    /**
     * @tc.steps: step2. update paint property and excute GetOverlayDrawFunction.
     * @tc.expected:  return value are as expected.
     */
    WeakPtr<RenderContext> renderContext;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper* paintWrapper = new PaintWrapper(renderContext, geometryNode, paintProp);
    auto result = paintMethod->GetOverlayDrawFunction(paintWrapper);
    EXPECT_NE(result, nullptr);
    result(canvas);
    delete paintWrapper;
    paintWrapper = nullptr;
    paintProp->UpdateNeedHeaderPadding(true);
    paintWrapper = new PaintWrapper(renderContext, geometryNode, paintProp);
    result = paintMethod->GetOverlayDrawFunction(paintWrapper);
    EXPECT_NE(result, nullptr);
    result(canvas);
    delete paintWrapper;
    paintWrapper = nullptr;
    paintProp->UpdateNeedFooterPadding(true);
    paintWrapper = new PaintWrapper(renderContext, geometryNode, paintProp);
    result = paintMethod->GetOverlayDrawFunction(paintWrapper);
    EXPECT_NE(result, nullptr);
    result(canvas);
    delete paintWrapper;
    paintWrapper = nullptr;
}

/**
 * @tc.name: MenuItemGroupPattern001
 * @tc.desc: Test MenuItemGroup pattern.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuItemGroupPattern001, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    MneuModelInstance.SetFontSize(Dimension(25.0));
    MneuModelInstance.SetFontColor(Color::RED);
    MneuModelInstance.SetFontWeight(FontWeight::BOLD);

    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto layoutProperty = menuPattern->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);

    MenuItemProperties itemOption;
    itemOption.content = "content";
    itemOption.labelInfo = "label";
    MneuItemModelInstance.Create(itemOption);
    auto itemNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(itemNode, nullptr);
    auto itemPattern = itemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    itemPattern->OnModifyDone();
    itemNode->MountToParent(menuNode);
    itemNode->OnMountToParentDone();

    // call UpdateMenuItemChildren
    menuPattern->OnModifyDone();

    auto contentNode = itemPattern->GetContentNode();
    ASSERT_NE(contentNode, nullptr);
    auto textProperty = contentNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);

    auto menuItemGroupPattern = AceType::MakeRefPtr<MenuItemGroupPattern>();
    auto menuItemGroup = FrameNode::CreateFrameNode(V2::MENU_ITEM_GROUP_ETS_TAG, -1, menuItemGroupPattern);
    menuItemGroup->MountToParent(menuNode);
    menuItemGroupPattern->hasSelectIcon_ = true;
    menuItemGroupPattern->headerContent_ = contentNode;
    menuItemGroupPattern->OnMountToParentDone();
    menuItemGroupPattern->footerContent_ = contentNode;
    menuItemGroupPattern->OnMountToParentDone();
    RefPtr<NG::UINode> headerNode;
    headerNode = NG::ViewStackProcessor::GetInstance()->Finish();
    menuItemGroupPattern->AddHeader(headerNode);
    RefPtr<NG::UINode> footerNode;
    footerNode = NG::ViewStackProcessor::GetInstance()->Finish();
    menuItemGroupPattern->AddFooter(footerNode);
    EXPECT_EQ(menuItemGroupPattern->headerIndex_, 0);
    EXPECT_EQ(menuItemGroupPattern->footerIndex_, 1);
    menuItemGroupPattern->headerIndex_ = 0;
    menuItemGroupPattern->footerIndex_ = 0;
    menuItemGroupPattern->AddHeader(headerNode);
    menuItemGroupPattern->AddFooter(footerNode);
    EXPECT_FALSE(menuItemGroupPattern->GetMenu() == nullptr);
}

/**
 * @tc.name: MenuLayoutPropertyTestNg001
 * @tc.desc: Verify GetPositionOffset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg001, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetPositionOffset().has_value());
    property.UpdatePositionOffset(OffsetF(25.0f, 30.0f));
    ASSERT_TRUE(property.GetPositionOffset().has_value());
    EXPECT_EQ(property.GetPositionOffset().value(), OffsetF(25.0f, 30.0f));
}

/**
 * @tc.name: MenuLayoutPropertyTestNg002
 * @tc.desc: Verify GetTitle.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg002, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetTitle().has_value());
    property.UpdateTitle("title");
    ASSERT_TRUE(property.GetTitle().has_value());
    EXPECT_EQ(property.GetTitle().value(), "title");
}

/**
 * @tc.name: MenuLayoutPropertyTestNg003
 * @tc.desc: Verify GetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg003, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetFontSize().has_value());
    property.UpdateFontSize(Dimension(25.0f));
    ASSERT_TRUE(property.GetFontSize().has_value());
    EXPECT_EQ(property.GetFontSize().value(), Dimension(25.0f));
}

/**
 * @tc.name: MenuLayoutPropertyTestNg004
 * @tc.desc: Verify GetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg004, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetFontColor().has_value());
    property.UpdateFontColor(Color::RED);
    ASSERT_TRUE(property.GetFontColor().has_value());
    EXPECT_EQ(property.GetFontColor().value(), Color::RED);
}

/**
 * @tc.name: MenuLayoutPropertyTestNg005
 * @tc.desc: Verify GetFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg005, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetFontWeight().has_value());
    property.UpdateFontWeight(FontWeight::BOLD);
    ASSERT_TRUE(property.GetFontWeight().has_value());
    EXPECT_EQ(property.GetFontWeight().value(), FontWeight::BOLD);
}

/**
 * @tc.name: MenuLayoutPropertyTestNg006
 * @tc.desc: Verify GetMenuOffset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg006, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetMenuOffset().has_value());
    property.UpdateMenuOffset(OffsetF(25.0f, 30.0f));
    ASSERT_TRUE(property.GetMenuOffset().has_value());
    EXPECT_EQ(property.GetMenuOffset().value(), OffsetF(25.0f, 30.0f));
}

/**
 * @tc.name: MenuLayoutPropertyTestNg007
 * @tc.desc: Verify Reset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg007, TestSize.Level1)
{
    MenuLayoutProperty property;
    property.UpdateMenuOffset(OffsetF(25.0f, 30.0f));
    property.UpdatePositionOffset(OffsetF(25.0f, 30.0f));
    property.UpdateTitle("title");
    property.UpdateFontSize(Dimension(25.0f));
    property.UpdateFontColor(Color::RED);
    property.UpdateFontWeight(FontWeight::BOLD);
    property.UpdateAlignType(MenuAlignType::START);
    property.UpdateOffset(DimensionOffset(Dimension(0, DimensionUnit::VP), Dimension(0, DimensionUnit::VP)));
    EXPECT_TRUE(property.GetMenuOffset().has_value());
    EXPECT_TRUE(property.GetPositionOffset().has_value());
    EXPECT_TRUE(property.GetTitle().has_value());
    EXPECT_TRUE(property.GetFontSize().has_value());
    EXPECT_TRUE(property.GetFontColor().has_value());
    EXPECT_TRUE(property.GetFontWeight().has_value());
    EXPECT_TRUE(property.GetAlignType().has_value());
    EXPECT_TRUE(property.GetOffset().has_value());
    property.Reset();
    EXPECT_FALSE(property.GetMenuOffset().has_value());
    EXPECT_FALSE(property.GetPositionOffset().has_value());
    EXPECT_FALSE(property.GetTitle().has_value());
    EXPECT_FALSE(property.GetFontSize().has_value());
    EXPECT_FALSE(property.GetFontColor().has_value());
    EXPECT_FALSE(property.GetFontWeight().has_value());
    EXPECT_FALSE(property.GetAlignType().has_value());
    EXPECT_FALSE(property.GetOffset().has_value());
}

/**
 * @tc.name: MenuLayoutPropertyTestNg008
 * @tc.desc: Verify Clone.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg008, TestSize.Level1)
{
    MenuLayoutProperty property;
    property.UpdateMenuOffset(OffsetF(25.0f, 30.0f));
    property.UpdatePositionOffset(OffsetF(25.0f, 30.0f));
    property.UpdateTitle("title");
    property.UpdateFontSize(Dimension(25.0f));
    property.UpdateFontColor(Color::RED);
    property.UpdateFontWeight(FontWeight::BOLD);
    property.UpdateAlignType(MenuAlignType::START);
    property.UpdateOffset(DimensionOffset(Dimension(10.0, DimensionUnit::VP), Dimension(10.0, DimensionUnit::VP)));

    auto cloneProperty = AceType::DynamicCast<MenuLayoutProperty>(property.Clone());
    ASSERT_NE(cloneProperty, nullptr);
    EXPECT_EQ(property.GetMenuOffset().value(), cloneProperty->GetMenuOffset().value());
    EXPECT_EQ(property.GetPositionOffset().value(), cloneProperty->GetPositionOffset().value());
    EXPECT_EQ(property.GetTitle().value(), cloneProperty->GetTitle().value());
    EXPECT_EQ(property.GetFontSize().value(), cloneProperty->GetFontSize().value());
    EXPECT_EQ(property.GetFontColor().value(), cloneProperty->GetFontColor().value());
    EXPECT_EQ(property.GetFontWeight().value(), cloneProperty->GetFontWeight().value());
    EXPECT_EQ(property.GetAlignType().value(), cloneProperty->GetAlignType().value());
    EXPECT_EQ(property.GetOffset().value(), cloneProperty->GetOffset().value());
}

/**
 * @tc.name: MenuLayoutPropertyTestNg009
 * @tc.desc: Verify ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg009, TestSize.Level1)
{
    MenuLayoutProperty property;
    property.UpdateMenuOffset(OffsetF(25.0f, 30.0f));
    property.UpdatePositionOffset(OffsetF(25.0f, 30.0f));
    property.UpdateTitle("title");
    property.UpdateFontSize(Dimension(25.0f));
    property.UpdateFontColor(Color::RED);
    property.UpdateFontWeight(FontWeight::BOLD);

    auto json = JsonUtil::Create(true);
    property.ToJsonValue(json);
    auto fontJsonObject = json->GetObject("font");
    EXPECT_EQ(json->GetString("title"), "title");
    EXPECT_EQ(json->GetString("offset"), OffsetF(25.0f, 30.0f).ToString());
    EXPECT_EQ(json->GetString("fontSize"), Dimension(25.0f).ToString());
    EXPECT_EQ(json->GetString("fontColor"), Color::RED.ColorToString());
    EXPECT_EQ(fontJsonObject->GetString("weight"), V2::ConvertWrapFontWeightToStirng(FontWeight::BOLD));
}

/**
 * @tc.name: MenuLayoutPropertyTestNg010
 * @tc.desc: Verify ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg010, TestSize.Level1)
{
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "fakeIcon", nullptr);
    optionParams.emplace_back("MenuItem2", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);

    auto json = JsonUtil::Create(true);
    property->ToJsonValue(json);
    auto itemArray = json->GetValue("bindMenu");
    ASSERT_EQ(itemArray->GetArraySize(), 2u);
    auto item1 = itemArray->GetArrayItem(0);
    EXPECT_EQ(item1->GetString("value"), "MenuItem1");
    EXPECT_EQ(item1->GetString("icon"), "fakeIcon");
    auto item2 = itemArray->GetArrayItem(1);
    EXPECT_EQ(item2->GetString("value"), "MenuItem2");
    EXPECT_EQ(item2->GetString("icon"), "");
}

/**
 * @tc.name: MenuLayoutPropertyTestNg011
 * @tc.desc: Verify SelectMenuAlignOption AlignType.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg011, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetAlignType().has_value());
    /**
     * @tc.cases: case1. verify the alignType property.
     */
    property.UpdateAlignType(MenuAlignType::CENTER);
    ASSERT_TRUE(property.GetAlignType().has_value());
    EXPECT_EQ(property.GetAlignType().value(), MenuAlignType::CENTER);
}

/**
 * @tc.name: MenuLayoutPropertyTestNg012
 * @tc.desc: Verify SelectMenuAlignOption Offset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuLayoutPropertyTestNg012, TestSize.Level1)
{
    MenuLayoutProperty property;
    EXPECT_FALSE(property.GetOffset().has_value());
    DimensionOffset offset(Dimension(MENU_OFFSET_X, DimensionUnit::VP), Dimension(MENU_OFFSET_Y, DimensionUnit::VP));
    /**
     * @tc.cases: case1. verify the offset property.
     */
    property.UpdateOffset(offset);
    ASSERT_TRUE(property.GetOffset().has_value());
    EXPECT_EQ(property.GetOffset().value(), offset);
}

/**
 * @tc.name: MenuViewTestNgCreate001
 * @tc.desc: Verify Create.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgCreate001, TestSize.Level1)
{
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "fakeIcon", nullptr);
    optionParams.emplace_back("MenuItem2", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    ASSERT_EQ(menuPattern->GetOptions().size(), 2);

    auto firstOption = menuPattern->GetOptions()[0];
    ASSERT_NE(firstOption, nullptr);
    EXPECT_EQ(firstOption->GetTag(), V2::OPTION_ETS_TAG);
    auto firstPattern = firstOption->GetPattern<OptionPattern>();
    ASSERT_NE(firstPattern, nullptr);
    EXPECT_EQ(firstPattern->GetText(), "MenuItem1");
    EXPECT_EQ(firstPattern->GetIcon(), "fakeIcon");
    auto secondOption = menuPattern->GetOptions()[1];
    ASSERT_NE(secondOption, nullptr);
    EXPECT_EQ(secondOption->GetTag(), V2::OPTION_ETS_TAG);
    auto secondPattern = secondOption->GetPattern<OptionPattern>();
    ASSERT_NE(secondPattern, nullptr);
    EXPECT_EQ(secondPattern->GetText(), "MenuItem2");
    EXPECT_EQ(secondPattern->GetIcon(), "");
}

/**
 * @tc.name: MenuViewTestNgCreate002
 * @tc.desc: Verify Create.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgCreate002, TestSize.Level1)
{
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "fakeIcon", nullptr);
    optionParams.emplace_back("MenuItem2", "", nullptr);
    MenuParam menuParam;
    menuParam.title = "Title";
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    menuPattern->OnModifyDone();
    auto column = menuPattern->GetMenuColumn();
    ASSERT_NE(column, nullptr);
    auto children = column->GetChildren();
    ASSERT_EQ(children.size(), 3);
    auto titleChild = AceType::DynamicCast<FrameNode>(column->GetChildAtIndex(0));
    ASSERT_NE(titleChild, nullptr);
    EXPECT_EQ(titleChild->GetTag(), V2::TEXT_ETS_TAG);
    auto textProperty = titleChild->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProperty, nullptr);
    EXPECT_TRUE(textProperty->GetContent().has_value());
    EXPECT_EQ(textProperty->GetContent().value(), "Title");
}

/**
 * @tc.name: MenuViewTestNgCreate003
 * @tc.desc: Verify Create.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgCreate003, TestSize.Level1)
{
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    MenuParam menuParam;
    menuParam.positionOffset = { 10.0f, 10.0f };
    auto menuWrapperNode = MenuView::Create(textNode, TARGET_ID, "", menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    EXPECT_TRUE(property->GetPositionOffset().has_value());
    EXPECT_EQ(property->GetPositionOffset().value(), OffsetF(10.0f, 10.0f));
}

/**
 * @tc.name: MenuViewTestNgCreate004
 * @tc.desc: Verify Create SubMenu.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgCreate004, TestSize.Level1)
{
    /**
     * @tc.steps: step1: create custom node and menu node
     * @tc.expected: menuNode not null
     */
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    MenuParam menuParam;
    menuParam.positionOffset = { 10.0f, 10.0f };
    menuParam.type = MenuType::SUB_MENU;
    auto menuNode = MenuView::Create(textNode, TARGET_ID, "", menuParam);
    ASSERT_NE(menuNode, nullptr);
    /**
     * @tc.steps: step2: get menuNode layoutProperty
     * @tc.expected: layoutProperty as expected
     */
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    EXPECT_TRUE(property->GetPositionOffset().has_value());
    EXPECT_EQ(property->GetPositionOffset().value(), OffsetF(10.0f, 10.0f));
}

/**
 * @tc.name: MenuViewTestNgSetFontSize001
 * @tc.desc: Verify SetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontSize001, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ViewStackProcessor::GetInstance()->Push(menuNode);
    MneuModelInstance.SetFontSize(Dimension(50.0));
    ASSERT_TRUE(property->GetFontSize().has_value());
    EXPECT_EQ(property->GetFontSize().value(), Dimension(50.0));
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: MenuViewTestNgSetFontSize002
 * @tc.desc: Verify SetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontSize002, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ViewStackProcessor::GetInstance()->Push(menuNode);
    MneuModelInstance.SetFontSize(Dimension());
    EXPECT_FALSE(property->GetFontSize().has_value());
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: MenuViewTestNgSetFontSize003
 * @tc.desc: Verify SetFontSize.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontSize003, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    MneuModelInstance.SetFontSize(Dimension(50.0));
    ASSERT_FALSE(property->GetFontSize().has_value());
}

/**
 * @tc.name: MenuViewTestNgSetFontColor001
 * @tc.desc: Verify SetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontColor001, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ViewStackProcessor::GetInstance()->Push(menuNode);
    MneuModelInstance.SetFontColor(Color::RED);
    ASSERT_TRUE(property->GetFontColor().has_value());
    EXPECT_EQ(property->GetFontColor().value(), Color::RED);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: MenuViewTestNgSetFontColor002
 * @tc.desc: Verify SetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontColor002, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    MneuModelInstance.SetFontColor(Color::RED);
    ASSERT_FALSE(property->GetFontColor().has_value());
}

/**
 * @tc.name: MenuViewTestNgSetFontColor003
 * @tc.desc: Verify SetFontColor.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontColor003, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ViewStackProcessor::GetInstance()->Push(menuNode);
    MneuModelInstance.SetFontColor(Color::RED);
    ASSERT_TRUE(property->GetFontColor().has_value());
    EXPECT_EQ(property->GetFontColor().value(), Color::RED);
    MneuModelInstance.SetFontColor(std::nullopt);
    ASSERT_FALSE(property->GetFontColor().has_value());
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: MenuViewTestNgSetFontWeight001
 * @tc.desc: Verify SetFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontWeight001, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    ViewStackProcessor::GetInstance()->Push(menuNode);
    MneuModelInstance.SetFontWeight(FontWeight::BOLDER);
    ASSERT_TRUE(property->GetFontWeight().has_value());
    EXPECT_EQ(property->GetFontWeight().value(), FontWeight::BOLDER);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: MenuViewTestNgSetFontWeight002
 * @tc.desc: Verify SetFontWeight.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetFontWeight002, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    MneuModelInstance.SetFontWeight(FontWeight::BOLDER);
    ASSERT_FALSE(property->GetFontWeight().has_value());
}

/**
 * @tc.name: MenuViewTestNgSetMenuPlacement001
 * @tc.desc: Verify SetMenuPlacement.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetMenuPlacement001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuWrapperNode with menuItems and set MenuPlacement to Placement::TOP
     * @tc.expected: step1. Get menuPlacement is Placement::TOP
     */
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem", "", nullptr);
    MenuParam menuParam;
    menuParam.placement = OHOS::Ace::Placement::TOP;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    EXPECT_TRUE(property->GetMenuPlacement().has_value());
    EXPECT_EQ(property->GetMenuPlacement().value(), OHOS::Ace::Placement::TOP);
}

/**
 * @tc.name: MenuViewTestNgSetMenuPlacement002
 * @tc.desc: Verify SetMenuPlacement.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuViewTestNgSetMenuPlacement002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create menuWrapperNode with custom node from a builder and set MenuPlacement to
     * Placement::BOTTOM
     * @tc.expected: step1. Get menuPlacement is Placement::BOTTOM
     */
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    MenuParam menuParam;
    menuParam.placement = OHOS::Ace::Placement::BOTTOM;
    menuParam.type = TYPE;
    auto menuWrapperNode = MenuView::Create(textNode, TARGET_ID, "", menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    EXPECT_TRUE(property->GetMenuPlacement().has_value());
    EXPECT_EQ(property->GetMenuPlacement().value(), OHOS::Ace::Placement::BOTTOM);
}

/**
 * @tc.name: MenuPaintMethodTestNg001
 * @tc.desc: Verify UpdateArrowPath.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintMethodTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare paint method object.
     */
    RefPtr<MenuPaintProperty> paintProp = AceType::MakeRefPtr<MenuPaintProperty>();
    RefPtr<MenuPaintMethod> paintMethod = AceType::MakeRefPtr<MenuPaintMethod>();
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawPath(_)).Times(AtLeast(1));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    /**
     * @tc.steps: step2. update paint property and excute GetOverlayDrawFunction.
     * @tc.expected:  return value are as expected.
     */
    paintProp->UpdateEnableArrow(true);
    paintProp->UpdateArrowPosition(OffsetF(10.0f, 10.0f));
    Placement placements[] = { Placement::TOP, Placement::BOTTOM, Placement::RIGHT, Placement::LEFT };
    for (Placement placementValue : placements) {
        paintProp->UpdateArrowPlacement(placementValue);
        PaintWrapper* paintWrapper = GetPaintWrapper(paintProp);
        auto result = paintMethod->GetOverlayDrawFunction(paintWrapper);
        EXPECT_NE(result, nullptr);
        result(canvas);
        delete paintWrapper;
        paintWrapper = nullptr;
    }
    /**
     * @tc.steps: step3. update enableArrow to false.
     * @tc.expected:  return value are as expected.
     */
    paintProp->UpdateEnableArrow(false);
    PaintWrapper* paintWrapper = GetPaintWrapper(paintProp);
    auto result = paintMethod->GetOverlayDrawFunction(paintWrapper);
    EXPECT_NE(result, nullptr);
    result(canvas);
    delete paintWrapper;
    paintWrapper = nullptr;
}

/**
 * @tc.name: MenuPaintMethodTestNg003
 * @tc.desc: Verify GetOverlayDrawFunction.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintMethodTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare paint method object.
     */
    RefPtr<MenuPaintProperty> paintProp = AceType::MakeRefPtr<MenuPaintProperty>();
    RefPtr<MenuPaintMethod> paintMethod = AceType::MakeRefPtr<MenuPaintMethod>();
    PaintWrapper* paintWrapper = GetPaintWrapper(paintProp);
    /**
     * @tc.steps: step2. excute functions.
     * @tc.expected:  return value are as expected.
     */
    auto arrowX = 0.0;
    auto arrowY = 0.0;
    RSPath path;
    Placement placements[] = { Placement::TOP, Placement::TOP_RIGHT, Placement::BOTTOM_RIGHT, Placement::RIGHT_TOP,
        Placement::LEFT_TOP, Placement::NONE };
    for (Placement placementValue : placements) {
        paintMethod->UpdateArrowPath(placementValue, arrowX, arrowY, path);
    }
    auto result = paintMethod->GetOverlayDrawFunction(paintWrapper);
    EXPECT_NE(result, nullptr);
    delete paintWrapper;
    paintWrapper = nullptr;
}

/**
 * @tc.name: OnColorConfigurationUpdate001
 * @tc.desc: Test on color configuration update.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, OnColorConfigurationUpdate001, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MenuItemModelNG MneuItemModelInstance;
    MneuModelInstance.Create();
    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto menuTheme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(menuTheme);
    menuTheme->backgroundColor_ = Color::BLACK;
    menuPattern->OnColorConfigurationUpdate();
    auto renderContext = menuNode->GetRenderContext();
    EXPECT_EQ(renderContext->GetBackgroundColor(), Color::BLACK);
}

/**
 * @tc.name: MenuPaintPropertyTestNg001
 * @tc.desc: Verify GetEnableArrow.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty.
     * @tc.expected: enableArrow has not value.
     */
    MenuPaintProperty property;
    EXPECT_FALSE(property.GetEnableArrow().has_value());
    /**
     * @tc.steps: step2. update enableArrow
     * @tc.expected: enableArrow value are as expected.
     */
    property.UpdateEnableArrow(true);
    ASSERT_TRUE(property.GetEnableArrow().has_value());
    EXPECT_TRUE(property.GetEnableArrow().value());
}

/**
 * @tc.name: MenuPaintPropertyTestNg002
 * @tc.desc: Verify GetArrowOffset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty.
     * @tc.expected: arrowOffset has not value.
     */
    MenuPaintProperty property;
    EXPECT_FALSE(property.GetArrowOffset().has_value());
    /**
     * @tc.steps: step2. update arrowOffset
     * @tc.expected: arrowOffset value are as expected.
     */
    property.UpdateArrowOffset(Dimension(10.0f));
    ASSERT_TRUE(property.GetArrowOffset().has_value());
    EXPECT_EQ(property.GetArrowOffset().value(), Dimension(10.0f));
}

/**
 * @tc.name: MenuPaintPropertyTestNg003
 * @tc.desc: Verify GetArrowPosition.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty.
     * @tc.expected: arrowPosition has not value.
     */
    MenuPaintProperty property;
    EXPECT_FALSE(property.GetArrowPosition().has_value());
    /**
     * @tc.steps: step2. update arrowPosition
     * @tc.expected: arrowPosition value are as expected.
     */
    property.UpdateArrowPosition(OffsetF(10.0f, 10.0f));
    ASSERT_TRUE(property.GetArrowPosition().has_value());
    EXPECT_EQ(property.GetArrowPosition().value(), OffsetF(10.0f, 10.0f));
}

/**
 * @tc.name: MenuPaintPropertyTestNg004
 * @tc.desc: Verify GetArrowPlacement.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty.
     * @tc.expected: arrowPlacement has not value.
     */
    MenuPaintProperty property;
    EXPECT_FALSE(property.GetArrowPlacement().has_value());
    /**
     * @tc.steps: step2. update arrowPlacement
     * @tc.expected: arrowPlacement value are as expected.
     */
    property.UpdateArrowPlacement(Placement::TOP);
    ASSERT_TRUE(property.GetArrowPlacement().has_value());
    EXPECT_EQ(property.GetArrowPlacement().value(), Placement::TOP);
}

/**
 * @tc.name: MenuPaintPropertyTestNg005
 * @tc.desc: Verify Reset.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty, update property
     * @tc.expected: property value are as expected.
     */
    MenuPaintProperty property;
    property.UpdateEnableArrow(true);
    property.UpdateArrowOffset(Dimension(10.0f));
    property.UpdateArrowPosition(OffsetF(10.0f, 10.0f));
    property.UpdateArrowPlacement(Placement::TOP);
    EXPECT_TRUE(property.GetEnableArrow().has_value());
    EXPECT_TRUE(property.GetArrowOffset().has_value());
    EXPECT_TRUE(property.GetArrowPosition().has_value());
    EXPECT_TRUE(property.GetArrowPlacement().has_value());
    /**
     * @tc.steps: step2. reset property
     * @tc.expected: property value are as expected.
     */
    property.Reset();
    EXPECT_FALSE(property.GetEnableArrow().has_value());
    EXPECT_FALSE(property.GetArrowOffset().has_value());
    EXPECT_FALSE(property.GetArrowPosition().has_value());
    EXPECT_FALSE(property.GetArrowPlacement().has_value());
}

/**
 * @tc.name: MenuPaintPropertyTestNg006
 * @tc.desc: Verify Clone.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty, update property
     */
    MenuPaintProperty property;
    property.UpdateEnableArrow(true);
    property.UpdateArrowOffset(Dimension(10.0f));
    property.UpdateArrowPosition(OffsetF(10.0f, 10.0f));
    property.UpdateArrowPlacement(Placement::TOP);
    /**
     * @tc.steps: step2. clone property
     * @tc.expected: property value are as expected.
     */
    auto cloneProperty = AceType::DynamicCast<MenuPaintProperty>(property.Clone());
    ASSERT_NE(cloneProperty, nullptr);
    EXPECT_EQ(property.GetEnableArrow().value(), cloneProperty->GetEnableArrow().value());
    EXPECT_EQ(property.GetArrowOffset().value(), cloneProperty->GetArrowOffset().value());
    EXPECT_EQ(property.GetArrowPosition().value(), cloneProperty->GetArrowPosition().value());
    EXPECT_EQ(property.GetArrowPlacement().value(), cloneProperty->GetArrowPlacement().value());
}

/**
 * @tc.name: MenuPaintPropertyTestNg007
 * @tc.desc: Verify ToJsonValue.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty, update property
     */
    MenuPaintProperty property;
    property.UpdateEnableArrow(true);
    property.UpdateArrowOffset(Dimension(10.0f));
    property.UpdateArrowPosition(OffsetF(10.0f, 10.0f));
    property.UpdateArrowPlacement(Placement::TOP);
    /**
     * @tc.steps: step2. property to json
     * @tc.expected: property jsonValue are as expected.
     */
    auto json = JsonUtil::Create(true);
    property.ToJsonValue(json);
    EXPECT_EQ(json->GetString("enableArrow"), V2::ConvertBoolToString(true).c_str());
    EXPECT_EQ(json->GetString("arrowOffset"), Dimension(10.0f).ToString().c_str());
    EXPECT_EQ(json->GetString("arrowPosition"), OffsetF(10.0f, 10.0f).ToString().c_str());
    EXPECT_EQ(json->GetString("arrowPlacement"), property.ConvertPlacementToString(Placement::TOP).c_str());
}

/**
 * @tc.name: MenuPaintPropertyTestNg008
 * @tc.desc: Verify ToJsonValue default value.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPaintPropertyTestNg008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Build a object MenuPaintProperty
     */
    MenuPaintProperty property;
    /**
     * @tc.steps: step2. property to json
     * @tc.expected: property jsonValue are as expected.
     */
    auto json = JsonUtil::Create(true);
    property.ToJsonValue(json);
    EXPECT_EQ(json->GetString("enableArrow"), V2::ConvertBoolToString(false).c_str());
    EXPECT_EQ(json->GetString("arrowOffset"), Dimension(0.0, DimensionUnit::VP).ToString().c_str());
    EXPECT_EQ(json->GetString("arrowPosition"), OffsetF(0.0f, 0.0f).ToString().c_str());
    EXPECT_EQ(json->GetString("arrowPlacement"), property.ConvertPlacementToString(Placement::NONE).c_str());
}

/**
 * @tc.name: MenuPreviewPatternTestNg0100
 * @tc.desc: Test MenuPreviewPattern functions.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPreviewPatternTestNg0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create preview node, then set the initial properties
     * @tc.expected: preview node, preview pattern are not null
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    RefPtr<MenuTheme> menuTheme = AceType::MakeRefPtr<MenuTheme>();
    menuTheme->previewBorderRadius_ = CONTAINER_OUTER_RADIUS;
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(menuTheme));
    auto previewNode = FrameNode::CreateFrameNode(V2::MENU_PREVIEW_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuPreviewPattern>());
    ASSERT_NE(previewNode, nullptr);
    auto menuWrapperNode = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(1));
    ASSERT_NE(menuWrapperNode, nullptr);
    previewNode->MountToParent(menuWrapperNode);
    auto previewPattern = previewNode->GetPattern<MenuPreviewPattern>();
    ASSERT_NE(previewPattern, nullptr);
    const RefPtr<LayoutWrapperNode> layoutWrapper;
    previewPattern->isFirstShow_ = true;

    /**
     * @tc.steps: step2. call OnModifyDone and OnDirtyLayoutWrapperSwap
     * @tc.expected: borderRadius is CONTAINER_OUTER_RADIUS, isFirstShow_ is false and panEvent is not empty
     */
    previewPattern->OnModifyDone();
    previewPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, configDirtySwap);
    auto renderContext = previewNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(CONTAINER_OUTER_RADIUS);
    EXPECT_EQ(renderContext->GetBorderRadius().value(), borderRadius);
    EXPECT_FALSE(previewPattern->isFirstShow_);
    auto gestureHub = previewNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    auto panEventActuator = gestureHub->panEventActuator_;
    ASSERT_NE(panEventActuator, nullptr);
    EXPECT_FALSE(panEventActuator->panEvents_.empty());
    auto menuWrapperPattern = menuWrapperNode->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(menuWrapperPattern, nullptr);
    menuWrapperPattern->isHided_ = false;
    GestureEvent info;
    info.offsetY_ = 1.0;

    /**
     * @tc.steps: step3. call pan task
     * @tc.expected: menuWrapperPattern's isHided_ is true
     */
    auto endTask = panEventActuator->panEvents_.front()->GetActionEndEventFunc();
    endTask(info);
    EXPECT_TRUE(menuWrapperPattern->isHided_);

    /**
     * @tc.steps: step4. call SetFirstShow
     * @tc.expected: isFirstShow_ is true.
     */
    previewPattern->SetFirstShow();
    EXPECT_TRUE(previewPattern->isFirstShow_);
}

/**
 * @tc.name: MenuPreviewLayoutAlgorithmTestNg0100
 * @tc.desc: Test MenuPreviewLayoutAlgorithm Measure and Layout function.
 * @tc.type: FUNC
 */
HWTEST_F(MenuTestNg, MenuPreviewLayoutAlgorithmTestNg0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create preview node and menuPreviewLayoutAlgorithm, then set the initial properties
     * @tc.expected: preview node, menuPreviewLayoutAlgorithm are not null
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    RefPtr<MenuTheme> menuTheme = AceType::MakeRefPtr<MenuTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(menuTheme));
    auto previewNode = FrameNode::CreateFrameNode(V2::MENU_PREVIEW_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuPreviewPattern>());
    auto previewPattern = previewNode->GetPattern<MenuPreviewPattern>();
    ASSERT_NE(previewPattern, nullptr);
    RefPtr<MenuPreviewLayoutAlgorithm> menuPreviewlayoutAlgorithm =
        AceType::DynamicCast<MenuPreviewLayoutAlgorithm>(previewPattern->CreateLayoutAlgorithm());
    ASSERT_NE(menuPreviewlayoutAlgorithm, nullptr);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    auto layoutProp = AceType::MakeRefPtr<LayoutProperty>();
    ASSERT_NE(layoutProp, nullptr);
    auto layoutWrapper = LayoutWrapperNode(previewNode, geometryNode, layoutProp);
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = FULL_SCREEN_SIZE;
    parentLayoutConstraint.percentReference = FULL_SCREEN_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT));
    layoutWrapper.GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper.GetLayoutProperty()->UpdateContentConstraint();

    /**
     * @tc.steps: step2. create menuPreviewLayoutAlgorithm Measure and Layout functions
     * @tc.expected: padding property is PREVIEW_INNER_SECURITY
     */
    menuPreviewlayoutAlgorithm->Measure(&layoutWrapper);
    menuPreviewlayoutAlgorithm->Layout(&layoutWrapper);
    PaddingProperty padding;
    ASSERT_NE(layoutProp, nullptr);
    EXPECT_EQ(layoutProp->GetPaddingProperty()->top, CalcLength(PREVIEW_INNER_SECURITY));
    EXPECT_EQ(layoutProp->GetPaddingProperty()->left, CalcLength(PREVIEW_INNER_SECURITY));
    EXPECT_EQ(layoutProp->GetPaddingProperty()->bottom, CalcLength(PREVIEW_INNER_SECURITY));
    EXPECT_EQ(layoutProp->GetPaddingProperty()->right, CalcLength(PREVIEW_INNER_SECURITY));
}
} // namespace OHOS::Ace::NG