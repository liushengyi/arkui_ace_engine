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

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_base.h"
#include "test/mock/core/render/mock_paragraph.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/offset.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/string_utils.h"
#include "base/utils/type_definition.h"
#include "core/common/ime/constant.h"
#include "core/common/ime/text_editing_value.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/ime/text_selection.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/event/key_event.h"
#include "core/event/touch_event.h"
#include "core/gestures/gesture_info.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr double ICON_SIZE = 24;
constexpr double ICON_HOT_ZONE_SIZE = 40;
constexpr double FONT_SIZE = 16;
constexpr int32_t DEFAULT_NODE_ID = 1;
constexpr int32_t MAX_BACKWARD_NUMBER = 30;
constexpr int32_t MAX_FORWARD_NUMBER = 30;
constexpr uint32_t DEFAULT_MAX_LINES = 1;
constexpr uint32_t DEFAULT_MAX_LENGTH = 30;
constexpr int32_t MIN_PLATFORM_VERSION = 10;
const std::string DEFAULT_TEXT = "abcdefghijklmnopqrstuvwxyz";
const std::string DEFAULT_PLACE_HOLDER = "please input text here";
const Color DEFAULT_PLACE_HODER_COLOR = Color::RED;
const Color DEFAULT_SELECTED_BACKFROUND_COLOR = Color::BLUE;
const Color DEFAULT_CARET_COLOR = Color::BLACK;
const Color DEFAULT_TEXT_COLOR = Color::BLACK;
const Dimension DEFAULT_FONT_SIZE = Dimension(16, DimensionUnit::VP);
const FontWeight DEFAULT_FONT_WEIGHT = FontWeight::W500;
const std::string DEFAULT_INPUT_FILTER = "[a-z]";
const InputStyle DEFAULT_INPUT_STYLE = InputStyle::INLINE;
const CopyOptions DEFAULT_COPY_OPTIONS = CopyOptions::InApp;
const TextAlign DEFAULT_TEXT_ALIGN = TextAlign::LEFT;
const CaretStyle DEFAULT_CARET_STYLE = { Dimension(3, DimensionUnit::VP) };
const OHOS::Ace::DisplayMode DEFAULT_DISPLAY_MODE = OHOS::Ace::DisplayMode::AUTO;
const TextInputAction DEFAULT_ENTER_KEY_TYPE = TextInputAction::BEGIN;
template<typename CheckItem, typename Expected>
struct TestItem {
    CheckItem item;
    Expected expected;
    std::string error;
    TestItem(CheckItem checkItem, Expected expectedValue, std::string message = "")
        : item(checkItem), expected(expectedValue), error(std::move(message))
    {}
    TestItem() = default;
};
struct ExpectParagaphParams {
    float height = 50.0f;
    float longestLine = 460.0f;
    float maxWidth = 460.0f;
    size_t lineCount = 1;
    bool firstCalc = true;
    bool secondCalc = true;
};
constexpr float CONTEXT_WIDTH_VALUE = 300.0f;
constexpr float CONTEXT_HEIGHT_VALUE = 150.0f;
} // namespace

class TextInputBase : public testing::Test, public TestNG {
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void TearDown() override;

    void CreateTextField(const std::string& text = "", const std::string& placeHolder = "",
        const std::function<void(TextFieldModelNG&)>& callback = nullptr);
    void RunMeasureAndLayout();
    void GetFocus();
    static void ExpectCallParagraphMethods(ExpectParagaphParams params);

    RefPtr<FrameNode> frameNode_;
    RefPtr<TextFieldPattern> pattern_;
    RefPtr<TextFieldEventHub> eventHub_;
    RefPtr<TextFieldLayoutProperty> layoutProperty_;
    RefPtr<TextFieldAccessibilityProperty> accessibilityProperty_;
};

void TextInputBase::SetUpTestSuite()
{
    MockContainer::SetUp();
    MockPipelineBase::SetUp();
    MockPipelineBase::GetCurrent()->SetRootSize(DEVICE_WIDTH, DEVICE_HEIGHT);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    auto textFieldTheme = AceType::MakeRefPtr<TextFieldTheme>();
    textFieldTheme->iconSize_ = Dimension(ICON_SIZE, DimensionUnit::VP);
    textFieldTheme->iconHotZoneSize_ = Dimension(ICON_HOT_ZONE_SIZE, DimensionUnit::VP);
    textFieldTheme->fontSize_ = Dimension(FONT_SIZE, DimensionUnit::FP);
    textFieldTheme->fontWeight_ = FontWeight::W400;
    textFieldTheme->textColor_ = Color::FromString("#ff182431");
    EXPECT_CALL(*themeManager, GetTheme(_))
        .WillRepeatedly([textFieldTheme = textFieldTheme](ThemeType type) -> RefPtr<Theme> {
            if (type == ScrollBarTheme::TypeId()) {
                return AceType::MakeRefPtr<ScrollBarTheme>();
            }
            return textFieldTheme;
        });
    MockPipelineBase::GetCurrent()->SetMinPlatformVersion(MIN_PLATFORM_VERSION);
    MockPipelineBase::GetCurrent()->SetTextFieldManager(AceType::MakeRefPtr<TextFieldManagerNG>());
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
}

void TextInputBase::TearDownTestSuite()
{
    MockContainer::TearDown();
    MockPipelineBase::TearDown();
    MockParagraph::TearDown();
}

void TextInputBase::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    eventHub_ = nullptr;
    layoutProperty_ = nullptr;
    accessibilityProperty_ = nullptr;
}

void TextInputBase::ExpectCallParagraphMethods(ExpectParagaphParams params)
{
    auto paragraph = MockParagraph::GetOrCreateMockParagraph();
    EXPECT_CALL(*paragraph, PushStyle(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, AddText(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, PopStyle()).Times(AnyNumber());
    EXPECT_CALL(*paragraph, Build()).Times(AnyNumber());
    EXPECT_CALL(*paragraph, Layout(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph, GetHeight()).WillRepeatedly(Return(params.height));
    EXPECT_CALL(*paragraph, GetLongestLine()).WillRepeatedly(Return(params.longestLine));
    EXPECT_CALL(*paragraph, GetMaxWidth()).WillRepeatedly(Return(params.maxWidth));
    EXPECT_CALL(*paragraph, GetLineCount()).WillRepeatedly(Return(params.lineCount));
}

void TextInputBase::CreateTextField(
    const std::string& text, const std::string& placeHolder, const std::function<void(TextFieldModelNG&)>& callback)
{
    auto* stack = ViewStackProcessor::GetInstance();
    stack->StartGetAccessRecordingFor(DEFAULT_NODE_ID);
    TextFieldModelNG textFieldModelNG;
    textFieldModelNG.CreateTextInput(placeHolder, text);
    if (callback) {
        callback(textFieldModelNG);
    }
    stack->StopGetAccessRecording();
    frameNode_ = AceType::DynamicCast<FrameNode>(stack->Finish());
    pattern_ = frameNode_->GetPattern<TextFieldPattern>();
    eventHub_ = frameNode_->GetEventHub<TextFieldEventHub>();
    layoutProperty_ = frameNode_->GetLayoutProperty<TextFieldLayoutProperty>();
    accessibilityProperty_ = frameNode_->GetAccessibilityProperty<TextFieldAccessibilityProperty>();
    RunMeasureAndLayout();
}

void TextInputBase::RunMeasureAndLayout()
{
    ExpectCallParagraphMethods(ExpectParagaphParams());
    frameNode_->SetActive();
    frameNode_->SetRootMeasureNode(true);
    frameNode_->UpdateLayoutPropertyFlag();
    frameNode_->SetSkipSyncGeometryNode(false);
    frameNode_->Measure(frameNode_->GetLayoutConstraint());
    frameNode_->Layout();
    frameNode_->SetRootMeasureNode(false);
}

void TextInputBase::GetFocus()
{
    auto focushHub = pattern_->GetFocusHub();
    focushHub->currentFocus_ = true;
    pattern_->HandleFocusEvent();
    RunMeasureAndLayout();
}

class TextInputCursorTest : public TextInputBase {};
class TextFieldControllerTest : public TextInputBase {};
class TextFieldKeyEventHandlerTest : public TextInputBase {};
class TextFiledAttrsTest : public TextInputBase {};
class TextFieldUXTest : public TextInputBase {};

/**
 * @tc.name: LayoutProperty001
 * @tc.desc: Test attrs on TextInput
 * @tc.type: FUNC
 */
HWTEST_F(TextFiledAttrsTest, LayoutProperty001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default attrs
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetWidthAuto(true);
        model.SetType(TextInputType::TEXT);
        model.SetPlaceholderColor(DEFAULT_PLACE_HODER_COLOR);
        model.SetTextColor(DEFAULT_TEXT_COLOR);
        model.SetEnterKeyType(DEFAULT_ENTER_KEY_TYPE);
        model.SetTextAlign(DEFAULT_TEXT_ALIGN);
        model.SetCaretColor(DEFAULT_CARET_COLOR);
        model.SetCaretStyle(DEFAULT_CARET_STYLE);
        model.SetSelectedBackgroundColor(DEFAULT_SELECTED_BACKFROUND_COLOR);
        model.SetMaxLength(DEFAULT_MAX_LENGTH);
        model.SetMaxLines(DEFAULT_MAX_LINES);
        model.SetFontSize(DEFAULT_FONT_SIZE);
        model.SetFontWeight(DEFAULT_FONT_WEIGHT);
        model.SetTextColor(DEFAULT_TEXT_COLOR);
        model.SetInputFilter(DEFAULT_INPUT_FILTER, nullptr);
        model.SetCopyOption(DEFAULT_COPY_OPTIONS);
        model.SetBarState(DEFAULT_DISPLAY_MODE);
        model.SetInputStyle(DEFAULT_INPUT_STYLE);
        model.SetShowUnderline(true);
        model.SetSelectAllValue(true);
    });

    /**
     * @tc.expected: Check if all set properties are displayed in the corresponding JSON
     */
    auto json = JsonUtil::Create(true);
    pattern_->ToJsonValue(json);
    EXPECT_EQ(json->GetString("text"), DEFAULT_TEXT.c_str());
    EXPECT_EQ(json->GetString("type"), "InputType.Normal");
    EXPECT_EQ(json->GetString("caretColor"), "#FF000000");
    EXPECT_EQ(json->GetString("placeholderColor"), "#FFFF0000");
    EXPECT_EQ(json->GetString("textAlign"), "TextAlign.Left");
    EXPECT_EQ(json->GetString("enterKeyType"), "EnterKeyType.Done");
    EXPECT_EQ(json->GetString("maxLength"), "30");
    EXPECT_EQ(json->GetString("inputFilter"), "[a-z]");
    EXPECT_EQ(json->GetString("copyOption"), "CopyOptions.InApp");
    EXPECT_EQ(json->GetString("style"), "TextInputStyle.Inline");
    EXPECT_EQ(json->GetString("maxLines"), "3");
    EXPECT_EQ(json->GetString("barState"), "BarState.AUTO");
    json = JsonUtil::Create(true);
    layoutProperty_->ToJsonValue(json);
    EXPECT_EQ(json->GetString("caretPosition"), "");
    EXPECT_TRUE(json->GetBool("showUnderline"));
    EXPECT_TRUE(json->GetBool("selectAll"));
}

/**
 * @tc.name: CaretPosition001
 * @tc.desc: Test caret position on TextFieldModelNG::CreateNode.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition001, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size()));

    /**
     * @tc.steps: Changed new text and remeasure and layout
     */
    pattern_->InsertValue("new");
    RunMeasureAndLayout();

    /**
     * @tc.expected: Current caret position is end of text
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size() + 3));
}

/**
 * @tc.name: CaretPosition002
 * @tc.desc: Test caret position on SetType.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition002, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder and set input type
     */
    std::string text = "openharmony@huawei.com+*0123456789";
    std::vector<TestItem<TextInputType, int32_t>> testItems;
    testItems.emplace_back(TextInputType::TEXT, text.length(), "TextInputType::TEXT");
    testItems.emplace_back(TextInputType::NUMBER, 10, "TextInputType::NUMBER");
    testItems.emplace_back(TextInputType::PHONE, 12, "TextInputType::PHONE");
    testItems.emplace_back(TextInputType::EMAIL_ADDRESS, text.length() - 2, "TextInputType::EMAIL_ADDRESS");
    testItems.emplace_back(TextInputType::VISIBLE_PASSWORD, text.length(), "TextInputType::VISIBLE_PASSWORD");
    testItems.emplace_back(TextInputType::NUMBER_PASSWORD, 10, "TextInputType::NUMBER_PASSWORD");
    testItems.emplace_back(TextInputType::SCREEN_LOCK_PASSWORD, text.length(), "TextInputType::SCREEN_LOCK_PASSWORD");

    /**
     * @tc.expected: Check if the text filter rules for the input box are compliant
     */
    for (const auto& testItem : testItems) {
        CreateTextField(text, "", [testItem](TextFieldModelNG& model) { model.SetType(testItem.item); });
        auto errorMessage = "InputType is " + testItem.error + ", text is " + pattern_->GetTextValue();
        EXPECT_EQ(pattern_->GetCaretIndex(), testItem.expected) << errorMessage;
        TearDown();
    }
}

/**
 * @tc.name: CaretPosition003
 * @tc.desc: Test caret position on SetCaretPosition
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition003, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text
     * @tc.expected: Cursor movement position matches the actual position
     */
    CreateTextField(DEFAULT_TEXT);
    auto controller = pattern_->GetTextFieldController();
    controller->CaretPosition(static_cast<int>(DEFAULT_TEXT.size() - 2));
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size() - 2));
}

/**
 * @tc.name: CaretPosition004
 * @tc.desc: Test caret position on SetMaxLength
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition004, TestSize.Level1)
{
    /**
     * @tc.steps: Create Text filed node with default text and placeholder
     * @tc.expected: Cursor movement position matches the actual position
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG& model) { model.SetMaxLength(DEFAULT_TEXT.size() - 2); });
    auto controller = pattern_->GetTextFieldController();
    controller->CaretPosition(static_cast<int>(DEFAULT_TEXT.size() - 2));
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.size() - 2));
}

/**
 * @tc.name: CaretPosition005
 * @tc.desc: Test caret position on SetInputFilter.
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition005, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text and filter patterns
     */
    std::string text = "abcdefghABCDEFG0123456789";
    std::vector<TestItem<std::string, int32_t>> testItems;
    testItems.emplace_back("", StringUtils::ToWstring(text).length());
    testItems.emplace_back("[0-9]", 10);
    testItems.emplace_back("[A-Z]", 7);
    testItems.emplace_back("[a-z]", 8);

    /**
     * @tc.expected: Check if the text filter patterns for the input box are compliant
     */
    for (const auto& testItem : testItems) {
        CreateTextField(
            text, "", [testItem](TextFieldModelNG& model) { model.SetInputFilter(testItem.item, nullptr); });
        auto errorMessage = "InputType is " + testItem.error + ", text is " + pattern_->GetTextValue();
        EXPECT_EQ(pattern_->GetCaretIndex(), testItem.expected) << errorMessage;
        TearDown();
    }
}

/**
 * @tc.name: CaretPosition005
 * @tc.desc: Test input string at the cursor position
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition006, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input and get select controller, update caret position and insert value
     */
    CreateTextField(DEFAULT_TEXT);

    auto controller = pattern_->GetTextSelectController();
    controller->UpdateCaretIndex(2);
    pattern_->InsertValue("new");
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue(), "abnewcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ(controller->GetCaretIndex(), 5);
}

/**
 * @tc.name: CaretPosition006
 * @tc.desc: Test stop edting input mode
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CaretPosition007, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: The cursor is neither blinking nor visible when unfocused
     */
    EXPECT_FALSE(pattern_->GetCursorVisible());

    /**
     * @tc.steps: Manually trigger focus and perform measure and layout again
     * @tc.expected: Check if the cursor is twinking
     */
    GetFocus();
    EXPECT_TRUE(pattern_->GetCursorVisible());

    /**
     * @tc.steps: Get text filed controller and stop editing
     */
    auto controller = pattern_->GetTextFieldController();
    controller->StopEditing();
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the cursor stop twinking
     */
    EXPECT_FALSE(pattern_->GetCursorVisible());
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition001
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node and call text changed listener update edting value
     */
    CreateTextField(DEFAULT_TEXT);

    GetFocus();
    TextEditingValue value;
    TextSelection selection;
    value.text = "new text";
    selection.baseOffset = value.text.length();
    value.selection = selection;
    pattern_->UpdateEditingValue(std::make_shared<TextEditingValue>(value));
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("new text"), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(value.text.length()));
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition002
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition002, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node and call delete backward
     */
    CreateTextField(DEFAULT_TEXT);

    GetFocus();
    pattern_->DeleteBackward(5);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstu"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.length() - 5));

    /**
     * @tc.steps: Move the cursor and then delete text
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(5);
    pattern_->DeleteBackward(5);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("fghijklmnopqrstu"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), 0);

    /**
     * @tc.steps: Trigger a backspace key press that exceeds the length of the text
     */
    pattern_->DeleteBackward(MAX_BACKWARD_NUMBER);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("fghijklmnopqrstu"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), 0);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition003
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition003, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT);

    GetFocus();
    pattern_->DeleteForward(5);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare(DEFAULT_TEXT), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), static_cast<int>(DEFAULT_TEXT.length()));

    /**
     * @tc.steps: Move the cursor and then delete text forward.
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(5);
    pattern_->DeleteForward(MAX_FORWARD_NUMBER);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("abcde"), 0) << "Text is " + pattern_->GetTextValue();
    EXPECT_EQ(pattern_->GetCaretIndex(), 5) << "Caret position is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition004
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition004, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values when 'IsSelected() = false'
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    pattern_->InsertValue("abc");
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare(DEFAULT_TEXT + "abc"), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length() + 3);

    /**
     * @tc.steps: Move the cursor and then insert text forward.
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(0);
    pattern_->InsertValue("abcde");
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new text and cursor position are correct
     */
    EXPECT_EQ(pattern_->GetTextValue().compare("abcde" + DEFAULT_TEXT + "abc"), 0);
    EXPECT_EQ(pattern_->GetCaretIndex(), 5);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition005
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition005, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    int32_t start = 5;
    int32_t end = 10;
    pattern_->HandleSetSelection(start, end);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new handle positions are correct
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition006
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition006, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    std::vector<std::int32_t> action = {
        ACTION_SELECT_ALL,
        ACTION_CUT,
        ACTION_COPY,
        ACTION_PASTE,
    };
    pattern_->HandleExtendAction(action[0]);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new handle positions are correct
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 0);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 26);

    /**
     * @tc.steps: Move the handles and then cut text snippet.
     */
    int32_t start = 5;
    int32_t end = 10;
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleExtendAction(action[1]);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new handle positions are correct
     * Cut data hasn't simulated
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 5);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 5);
    EXPECT_EQ(pattern_->GetTextSelectController()->GetCaretIndex(), 5);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition007
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition007, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then cut text snippet.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::string expectStr = "fghij";
    std::vector<std::int32_t> action = {
        ACTION_SELECT_ALL,
        ACTION_CUT,
        ACTION_COPY,
        ACTION_PASTE,
    };
    auto callback = [expectStr](const std::string& str) { EXPECT_EQ(expectStr, str); };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER, [&](TextFieldModel& model) { model.SetOnCut(callback); });
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleExtendAction(action[1]);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if the new handle positions are correct
     *               Verify the cut data
     */
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, start);
    EXPECT_EQ(pattern_->GetTextSelectController()->GetCaretIndex(), start);
    EXPECT_EQ(pattern_->contentController_->GetTextValue().compare("abcdeklmnopqrstuvwxyz"), 0);
}

/**
 * @tc.name: OnTextChangedListenerCaretPosition008
 * @tc.desc: Test the soft keyboard interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnTextChangedListenerCaretPosition008, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then cut text snippet.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::string expectStr = "fghij";
    std::vector<std::int32_t> action = {
        ACTION_SELECT_ALL,
        ACTION_CUT,
        ACTION_COPY,
        ACTION_PASTE,
    };
    auto onCopy = [expectStr](const std::string& str) { EXPECT_EQ(expectStr, str); };
    auto onPaste = [expectStr](const std::string& str) { EXPECT_EQ(expectStr, str); };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER, [&](TextFieldModel& model) -> void {
        model.SetOnCopy(onCopy);
        model.SetOnPaste(onPaste);
    });

    /**
     * @tc.steps: Move the handles and then cut text snippet.
     *            Verify the copy and paste data.
     */
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleExtendAction(action[2]);
    pattern_->HandleExtendAction(action[3]);
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijfghijklmnopqrstuvwxyz"), 0)
        << "Text is " + pattern_->GetTextValue();
}

/**
 * @tc.name: OnHandleMove001
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::vector<std::int32_t> select = { 2014, 2015, 2012, 2013 };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection left.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleSelect(select[0], 0);
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end - 1);

    /**
     * @tc.steps: Move the handles and selection right.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleSelect(select[1], 0);
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end + 1);
}

/**
 * @tc.name: OnHandleMove002
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove002, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    int32_t start = 5;
    int32_t end = 10;
    std::vector<std::int32_t> select = { 2014, 2015, 2012, 2013 };
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection up.
     *            Verify the selection data.
     */
    EXPECT_FALSE(pattern_->IsTextArea());
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleSelect(select[2], 0);
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end);

    /**
     * @tc.steps: Move the handles and selection down.
     *            Verify the selection data.
     */
    EXPECT_FALSE(pattern_->IsTextArea());
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleSelect(select[3], 0);
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end);
}

/**
 * @tc.name: OnHandleMove003
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove003, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection left word.
     *            Verify the selection data.
     */
    auto textFiledController = pattern_->GetTextFieldController();
    textFiledController->CaretPosition(5);
    pattern_->HandleSelectionLeftWord();
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 5);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 0);

    /**
     * @tc.steps: Move the handles and selection right word.
     *            Verify the selection data.
     */
    pattern_->HandleSelectionRightWord();
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, 5);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 21);
}

/**
 * @tc.name: OnHandleMove004
 * @tc.desc: Test the clip board interface
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, OnHandleMove004, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Initialize text input and Move the handles and then do handle selection.
     */
    int32_t start = 5;
    int32_t end = 10;
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);

    /**
     * @tc.steps: Move the handles and selection line begin.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleSelectionLineBegin();
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, end);

    /**
     * @tc.steps: Move the handles and selection line end.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleSelectionLineEnd();
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 26);

    /**
     * @tc.steps: Move the handles and selection home.
     *            Verify the selection data.
     */
    pattern_->HandleSelectionHome();
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 0);

    /**
     * @tc.steps: Move the handles and selection end.
     *            Verify the selection data.
     */
    pattern_->HandleSetSelection(start, end);
    pattern_->HandleSelectionEnd();
    RunMeasureAndLayout();
    EXPECT_EQ(pattern_->selectController_->GetFirstHandleInfo().index, start);
    EXPECT_EQ(pattern_->selectController_->GetSecondHandleInfo().index, 26);
}

/**
 * @tc.name: CursonMoveLeftTest001
 * @tc.desc: Test the curson move left
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursonMoveLeftTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    GetFocus();
    auto ret = pattern_->CursorMoveLeft();
    RunMeasureAndLayout();

    /**
     * @tc.expected: In a situation where no text is selected, the movement is successfull
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length() - 1);

    /**
     * @tc.steps: In a situation where text is selected, the movement is successful
     */
    pattern_->HandleSetSelection(3, 5);
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveLeft();

    /**
     * @tc.expected: The cursor moves to the position after the selected text is deleted
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 4)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursonMoveLeftWordTest001
 * @tc.desc: Test the curson move left word
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursonMoveLeftWordTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text input node and default text
     */
    CreateTextField(DEFAULT_TEXT, DEFAULT_PLACE_HOLDER);
    GetFocus();

    auto ret = pattern_->CursorMoveLeftWord();

    /**
     * @tc.expected: In a situation where no text is selected, the movement is successfull
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: In a situation where text is selected, the movement is successful
     */
    pattern_->HandleSetSelection(3, 5);
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveLeftWord();

    /**
     * @tc.expected: The cursor moves to the position after the selected text is deleted
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveLineBeginTest001
 * @tc.desc: Test the cursor move line begin
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveLineBeginTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize empty text and CursorMoveLineBegin
     */
    CreateTextField();
    GetFocus();
    auto ret = pattern_->CursorMoveLineBegin();
    EXPECT_TRUE(ret);

    /**
     * @tc.steps: step2. Insert text and move line begin
     */
    pattern_->InsertValue("hello world");
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveLineBegin();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: In a situation where text is all selected, the movement is successful
     */
    pattern_->HandleSetSelection(0, 11);
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveLineBegin();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveToParagraphBeginTest001
 * @tc.desc: Test the cursor move paragraph begin
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveToParagraphBeginTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize empty text and cursor move to paragraph begin
     */
    CreateTextField();
    GetFocus();
    auto ret = pattern_->CursorMoveToParagraphBegin();
    EXPECT_TRUE(ret);

    /**
     * @tc.steps: step2. Insert text
     */
    pattern_->InsertValue("hello world");
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveToParagraphBegin();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveHomeTest001
 * @tc.desc: Test the cursor move home
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveHomeTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize empty text and curson move home
     */
    CreateTextField();
    GetFocus();
    auto ret = pattern_->CursorMoveHome();
    EXPECT_TRUE(ret);

    /**
     * @tc.steps: step2. Insert text
     */
    pattern_->InsertValue("hello world");
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveHome();

    /**
     * @tc.expected: Cursor move to the line head
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 0)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveRightTest001
 * @tc.desc: Test the cursor move right
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveRightTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize default text and curson move right
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveRight();

    /**
     * @tc.expected: Unable to move
     */
    EXPECT_FALSE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 26)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step2. Move cursor back to the line head and move right
     */
    ret = pattern_->CursorMoveLineBegin();
    ret = pattern_->CursorMoveRight();

    /**
     * @tc.expected: In a situation where no text is selected, the movement is successfull
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 1)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step3. Select the text within coordinates 3 to 5 and move cursor right
     */
    pattern_->HandleSetSelection(3, 5);
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveRight();

    /**
     * @tc.expected: Select from 3 to 5, move the cursor to 6.
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), 6)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveRightWordTest001
 * @tc.desc: Test the cursor move right word
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveRightWordTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize default text and curson move right
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveRightWord();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: steps2. Move to the left 2 times first.
     */
    ret = pattern_->CursorMoveLeft();
    ret = pattern_->CursorMoveLeft();
    RunMeasureAndLayout();

    /**
     * @tc.expected:  the current text length - 2
     */
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length() - 2)
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: steps3. Continue moving to the right word.
     */
    ret = pattern_->CursorMoveRightWord();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step4. Move to the beginning of the line and select all text.
     */
    ret = pattern_->CursorMoveLineBegin();
    pattern_->HandleSetSelection(0, DEFAULT_TEXT.length());
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveRightWord();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveLineEndTest001
 * @tc.desc: Test the cursor move line end
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveLineEndTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize  text and move to the end of the line
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveLineEnd();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());

    /**
     * @tc.steps: step2. Move to the beginning of the line and select all text
     */
    pattern_->HandleSetSelection(0, DEFAULT_TEXT.length());
    RunMeasureAndLayout();
    ret = pattern_->CursorMoveLineEnd();

    /**
     * @tc.expected: Moving to the right character when there is initial text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetSelectMode(), SelectionMode::NONE);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveToParagraphEndTest001
 * @tc.desc: Test the cursor move to pragraph to the end
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveToParagraphEndTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text and move to the pargraph of the line
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveToParagraphEnd();
    EXPECT_TRUE(ret);

    /**
     * @tc.expected: Moving to the paragraph end and check if cursor is on pargraph end
     */
    ret = pattern_->CursorMoveLeft();
    ret = pattern_->CursorMoveToParagraphEnd();
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length())
        << "Text is " + pattern_->GetTextValue() + ", CaretIndex is " + std::to_string(pattern_->GetCaretIndex());
}

/**
 * @tc.name: CursorMoveEndTest001
 * @tc.desc: Test the cursor move end
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveEndTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text and move to the pargraph of the line
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveToParagraphEnd();
    EXPECT_TRUE(ret);

    /**
     * @tc.expected: Move left once first, and then move to the end
     *               Check if the cursor is at the end of the text.
     */
    ret = pattern_->CursorMoveLeft();
    ret = pattern_->CursorMoveEnd();
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetCaretIndex(), DEFAULT_TEXT.length());
}

/**
 * @tc.name: CursorMoveUpTest001
 * @tc.desc: Test the cursor move up
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, CursorMoveUpTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Move up and down in a single-line text
     * @tc.expected: In single-line text, there is no up and down movement
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();
    auto ret = pattern_->CursorMoveUp();
    EXPECT_FALSE(ret);
    ret = pattern_->CursorMoveDown();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetLeftTextOfCursor001
 * @tc.desc: Test get text of left cursor
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, GetLeftTextOfCursor001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Create default text and retrieve the left 5 characters before the cursor
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetLeftTextOfCursor(5)), "vwxyz");

    /**
     * @tc.steps: step2. Select the text from position 3 to 5
     */
    pattern_->HandleSetSelection(3, 5);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetLeftTextOfCursor(5)), "abc");
}

/**
 * @tc.name: GetRightTextOfCursor001
 * @tc.desc: Test get text of left cursor
 * @tc.type: FUNC
 */
HWTEST_F(TextInputCursorTest, GetRightTextOfCursor001, TestSize.Level1)
{
    /**
     * @tc.steps: steps1. Create default text and retrieve the left 5 characters before the cursor
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetRightTextOfCursor(5)), "");

    /**
     * @tc.steps: step2. Select the text from position 3 to 5
     */
    pattern_->HandleSetSelection(3, 5);
    RunMeasureAndLayout();

    /**
     * @tc.expected: Check if it equals "vwxyz"
     */
    EXPECT_EQ(StringUtils::Str16ToStr8(pattern_->GetRightTextOfCursor(5)), "fghij");
}

/**
 * @tc.name: ContentController001
 * @tc.desc: Test ContentController in different input type
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldControllerTest, ContentController001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize insert text and expected values
     */
    std::vector<std::string> insertValues = {
        "openharmony123_ *+%$",
        "openharmony123456*+&@huawei.com",
        "openharmony#15612932075*.com",
        "open_harmony@@huawei.com*+$helloworld",
        "open_harmony123 password*+#",
        "openharmony123456*+&@huawei.com",
        "o达瓦大屋顶pen_harmony456顶顶顶 password*+#得到",
    };
    std::vector<TestItem<TextInputType, std::string>> testItems;
    testItems.emplace_back(TextInputType::TEXT, "openharmony123_ *+%$", "TextInputType::TEXT");
    testItems.emplace_back(TextInputType::NUMBER, "123456", "TextInputType::NUMBER");
    testItems.emplace_back(TextInputType::PHONE, "#15612932075*", "TextInputType::PHONE");
    testItems.emplace_back(
        TextInputType::EMAIL_ADDRESS, "open_harmony@huawei.comhelloworld", "TextInputType::EMAIL_ADDRESS");
    testItems.emplace_back(
        TextInputType::VISIBLE_PASSWORD, "open_harmony123 password*+#", "TextInputType::VISIBLE_PASSWORD");
    testItems.emplace_back(TextInputType::NUMBER_PASSWORD, "123456", "TextInputType::NUMBER_PASSWORD");
    testItems.emplace_back(
        TextInputType::SCREEN_LOCK_PASSWORD, "open_harmony456 password*+#", "TextInputType::SCREEN_LOCK_PASSWORD");

    /**
     * @tc.expected: Check if text filtering meets expectations
     */
    int index = 0;
    for (const auto& testItem : testItems) {
        CreateTextField("", "", [testItem](TextFieldModelNG& model) { model.SetType(testItem.item); });

        pattern_->contentController_->InsertValue(0, insertValues[index]);
        index++;
        auto errorMessage = "InputType is " + testItem.error + ", text is " + pattern_->GetTextValue();
        EXPECT_EQ(pattern_->GetTextValue().compare(testItem.expected), 0) << errorMessage;
    }
}

/**
 * @tc.name: ContentController002
 * @tc.desc: Test ContentController in different input filter
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldControllerTest, ContentController002, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text and filter patterns
     */
    std::string text = "CabcdefgABhCDEFG0123a456A789";
    std::vector<TestItem<std::string, std::string>> testItems;
    testItems.emplace_back("", "CabcdefgABhCDEFG0123a456A789", "None");
    testItems.emplace_back("[0-9]", "0123456789", "Input filter [0-9]");
    testItems.emplace_back("[A-Z]", "CABCDEFGA", "Input filter [A-Z]");
    testItems.emplace_back("[a-z]", "abcdefgha", "Input filter [a-z]");

    /**
     * @tc.expected: Check if the text filter patterns for the input box are compliant
     */
    for (const auto& testItem : testItems) {
        CreateTextField("", "", [testItem](TextFieldModelNG& model) { model.SetInputFilter(testItem.item, nullptr); });

        pattern_->contentController_->InsertValue(0, text);
        auto errorMessage = testItem.error + ", text is " + pattern_->GetTextValue();
        EXPECT_EQ(pattern_->GetTextValue().compare(testItem.expected), 0) << errorMessage;
    }
}

/**
 * @tc.name: ContentController003
 * @tc.desc: Test ContentController in different input filter
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldControllerTest, ContentController003, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text filed node
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Check if text is selected based on corresponding left and right coordinates
     */
    auto selectedValue = pattern_->contentController_->GetSelectedValue(1, 4);
    EXPECT_EQ(selectedValue.compare("bcd"), 0) << "Text is " + selectedValue;

    /**
     * @tc.expected: Check if text is selected based on preceding coordinates
     */
    auto beforeSelectedValue = pattern_->contentController_->GetValueBeforeIndex(3);
    EXPECT_EQ(beforeSelectedValue.compare("abc"), 0) << "Text is " + beforeSelectedValue;

    /**
     * @tc.expected: Check if text is selected based on trailing coordinates
     */
    auto afterSelectedValue = pattern_->contentController_->GetValueAfterIndex(3);
    EXPECT_EQ(afterSelectedValue.compare("defghijklmnopqrstuvwxyz"), 0) << "Text is " + afterSelectedValue;
}

/**
 * @tc.name: TextFiledControllerTest001
 * @tc.desc: Test TextFieldController GetTextContentLinesNum
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldControllerTest, TextFiledControllerTest001, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text filed node
     */
    CreateTextField();

    /**
     * @tc.expected: Check if the number of lines meets the expectation
     */
    auto controller = pattern_->GetTextFieldController();
    auto line = controller->GetTextContentLinesNum();
    EXPECT_EQ(line, 0);
}

/**
 * @tc.name: ControllerTest002
 * @tc.desc: Test TextFieldController GetTextContentLinesNum
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldControllerTest, TextFiledControllerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: Initialize text filed node
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.expected: Check if the number of lines meets the expectation
     */
    auto controller = pattern_->GetTextFieldController();
    auto line = controller->GetTextContentLinesNum();
    EXPECT_EQ(line, 1);
}

/**
 * @tc.name: KeyEventHandler001
 * @tc.desc: Test KeyEventHandler HandleShiftPressedEvent
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldKeyEventHandlerTest, KeyEventHandler001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize insert text and key event handler
     */
    CreateTextField(DEFAULT_TEXT);

    auto keyEventHandler = AceType::MakeRefPtr<KeyEventHandler>();
    keyEventHandler->UpdateWeakPattern(pattern_);

    /**
     * @tc.steps: step2. Initialize key event and press ` into text
     */
    KeyEvent event;
    std::vector<KeyCode> presscodes = {};
    event.code = KeyCode::KEY_GRAVE;
    presscodes.emplace_back(event.code);
    event.pressedCodes = presscodes;
    auto ret = keyEventHandler->HandleShiftPressedEvent(event);
    RunMeasureAndLayout();

    /**
     * @tc.expected: call func HandleShiftPressedEvent result is true and add ` to text
     */
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstuvwxyz`"), 0)
        << "Text is " + pattern_->GetTextValue();

    /**
     * @tc.steps: step3. press A to text
     * @tc.expected: call func HandleShiftPressedEvent result is true and add ` to text
     */
    event.code = KeyCode::KEY_A;
    ret = keyEventHandler->HandleShiftPressedEvent(event);
    RunMeasureAndLayout();
    EXPECT_FALSE(ret);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstuvwxyz`"), 0)
        << "Text is " + pattern_->GetTextValue();

    /**
     * @tc.steps: step4. press left shift + right shift + - to text
     * @tc.expected: call func HandleShiftPressedEvent result is true and add 3 to text
     */
    event.pressedCodes.clear();
    event.pressedCodes.emplace_back(KeyCode::KEY_SHIFT_LEFT);
    event.pressedCodes.emplace_back(KeyCode::KEY_SHIFT_RIGHT);
    event.code = KeyCode::KEY_MINUS;
    ret = keyEventHandler->HandleShiftPressedEvent(event);
    RunMeasureAndLayout();
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstuvwxyz`_"), 0)
        << "Text is " + pattern_->GetTextValue();

    /**
     * @tc.steps: step5. press left shift + right shift + D to text
     * @tc.expected: call func HandleShiftPressedEvent result is true and add D to text
     */
    event.code = KeyCode::KEY_D;
    ret = keyEventHandler->HandleShiftPressedEvent(event);
    RunMeasureAndLayout();
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstuvwxyz`_D"), 0)
        << "Text is " + pattern_->GetTextValue();

    /**
     * @tc.steps: step6. press left shift + right shift + 5 to text
     * @tc.expected: call func HandleShiftPressedEvent result is true and add % to text
     */
    event.code = KeyCode::KEY_5;
    ret = keyEventHandler->HandleShiftPressedEvent(event);
    RunMeasureAndLayout();
    EXPECT_TRUE(ret);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstuvwxyz`_D%"), 0)
        << "Text is " + pattern_->GetTextValue();

    /**
     * @tc.steps: step7. press left shift + right shift + blank to text
     * @tc.expected: text is not changed
     */
    event.code = KeyCode::KEY_SPACE;
    ret = keyEventHandler->HandleShiftPressedEvent(event);
    RunMeasureAndLayout();
    EXPECT_FALSE(ret);
    EXPECT_EQ(pattern_->GetTextValue().compare("abcdefghijklmnopqrstuvwxyz`_D%"), 0)
        << "Text is " + pattern_->GetTextValue();
}

/**
 * @tc.name: KeyEventHandler002
 * @tc.desc: Test KeyEventHandler HandleDirectionalKey
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldKeyEventHandlerTest, KeyEventHandler002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input and key event handler
     */
    CreateTextField(DEFAULT_TEXT);

    auto keyEventHandler = AceType::MakeRefPtr<KeyEventHandler>();
    keyEventHandler->UpdateWeakPattern(pattern_);

    /**
     * @tc.steps: step2. Initialize KeyEvent and call HandleDirectionalKey
     * @tc.expected: return as expected
     */
    KeyEvent event;
    std::vector<KeyCode> eventCodes = {
        KeyCode::KEY_DPAD_UP,
        KeyCode::KEY_DPAD_DOWN,
        KeyCode::KEY_DPAD_LEFT,
        KeyCode::KEY_DPAD_RIGHT,
        KeyCode::KEY_MOVE_HOME,
        KeyCode::KEY_MOVE_END,
    };
    std::vector<KeyCode> presscodes = {};
    event.pressedCodes = presscodes;
    for (auto eventCode : eventCodes) {
        event.pressedCodes.emplace_back(KeyCode::KEY_SHIFT_LEFT);
        event.pressedCodes.emplace_back(eventCode);
        auto ret = keyEventHandler->HandleDirectionalKey(event);
        EXPECT_TRUE(ret);
    }
    event.pressedCodes.clear();
    for (auto eventCode : eventCodes) {
        event.pressedCodes.emplace_back(KeyCode::KEY_CTRL_LEFT);
        event.pressedCodes.emplace_back(KeyCode::KEY_SHIFT_LEFT);
        event.pressedCodes.emplace_back(eventCode);
        auto ret = keyEventHandler->HandleDirectionalKey(event);
        EXPECT_TRUE(ret);
    }
    event.pressedCodes.clear();
    event.pressedCodes.emplace_back(KeyCode::KEY_BACK);
    auto ret = keyEventHandler->HandleDirectionalKey(event);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: KeyEventHandler003
 * @tc.desc: Test KeyEventHandler IsCtrlShiftWith
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldKeyEventHandlerTest, KeyEventHandler003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input and key event handler
     */
    CreateTextField(DEFAULT_TEXT);

    auto keyEventHandler = AceType::MakeRefPtr<KeyEventHandler>();
    keyEventHandler->UpdateWeakPattern(pattern_);

    /**
     * @tc.steps: step2. Initialize KeyEvent and call HandleDirectionalMoveKey
     * @tc.expected: return as expected
     */
    KeyEvent event;
    std::vector<KeyCode> eventCodes = {
        KeyCode::KEY_DPAD_LEFT,
        KeyCode::KEY_DPAD_UP,
        KeyCode::KEY_MOVE_HOME,
        KeyCode::KEY_DPAD_RIGHT,
        KeyCode::KEY_DPAD_DOWN,
        KeyCode::KEY_MOVE_END,
    };
    std::vector<KeyCode> presscodes = {};
    event.pressedCodes = presscodes;
    for (auto eventCode : eventCodes) {
        event.pressedCodes.emplace_back(KeyCode::KEY_CTRL_LEFT);
        event.pressedCodes.emplace_back(eventCode);
        auto ret = keyEventHandler->HandleDirectionalMoveKey(event);
        EXPECT_TRUE(ret) << "KeyCode: " + std::to_string(static_cast<int>(eventCode));
    }
    event.pressedCodes.clear();
    std::array<bool, 6> results = { true, false, true, true, false, true };
    int index = 0;
    for (auto eventCode : eventCodes) {
        event.pressedCodes.emplace_back(eventCode);
        event.code = eventCode;
        auto ret = keyEventHandler->HandleDirectionalMoveKey(event);
        EXPECT_EQ(results[index], ret) << "KeyCode: " + std::to_string(static_cast<int>(eventCode));
        index++;
    }
    event.code = KeyCode::KEY_DPAD_CENTER;
    event.pressedCodes.clear();
    event.pressedCodes.emplace_back(event.code);
    auto ret = keyEventHandler->HandleDirectionalMoveKey(event);
    EXPECT_FALSE(ret) << "KeyCode: " + std::to_string(static_cast<int>(event.code));
}

/**
 * @tc.name: UpdateCaretByTouchMove001
 * @tc.desc: Test UpdateCaretByTouchMove
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateCaretByTouchMove001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize textInput and focusHub
     */
    CreateTextField();
    auto focusHub = frameNode_->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    /**
     * @tc.steps: step2. create location info, touch type DOWN
     */
    TouchLocationInfo touchLocationInfo1(0);
    touchLocationInfo1.touchType_ = TouchType::DOWN;
    touchLocationInfo1.localLocation_ = Offset(0.0f, 0.0f);

    /**
     * @tc.steps: step3. create touch info, touch type DOWN
     */
    TouchEventInfo touchInfo1("");
    touchInfo1.AddTouchLocationInfo(std::move(touchLocationInfo1));

    /**
     * @tc.steps: step4. test touch down
     */
    pattern_->HandleTouchEvent(touchInfo1);
    EXPECT_TRUE(pattern_->isTouchCaret_);

    /**
     * @tc.steps: step5. create location info, touch type MOVE
     */
    TouchLocationInfo touchLocationInfo2(0);
    touchLocationInfo2.touchType_ = TouchType::MOVE;
    touchLocationInfo2.localLocation_ = Offset(0.0f, 0.0f);

    /**
     * @tc.steps: step6. create touch info, touch type MOVE
     */
    TouchEventInfo touchInfo2("");
    touchInfo2.AddTouchLocationInfo(std::move(touchLocationInfo2));

    /**
     * @tc.steps: step7. test touch move
     */
    pattern_->HandleTouchEvent(touchInfo2);
    EXPECT_EQ(pattern_->selectController_->GetCaretIndex(), 0);

    /**
     * @tc.steps: step8. create location, touch type info UP
     */
    TouchLocationInfo touchLocationInfo3(0);
    touchLocationInfo3.touchType_ = TouchType::UP;
    touchLocationInfo3.localLocation_ = Offset(0.0f, 0.0f);

    /**
     * @tc.steps: step9. create touch info, touch type UP
     */
    TouchEventInfo touchInfo3("");
    touchInfo3.AddTouchLocationInfo(std::move(touchLocationInfo3));

    /**
     * @tc.steps: step10. test touch up
     */
    pattern_->HandleTouchEvent(touchInfo3);
    EXPECT_FALSE(pattern_->isTouchCaret_);
}

/**
 * @tc.name: CleanNode001
 * @tc.desc: Test UpdateClearNode
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, CleanNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Get clear node response area
     */
    auto cleanNodeResponseArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern_->cleanNodeResponseArea_);
    ASSERT_NE(cleanNodeResponseArea, nullptr);

    /**
     * @tc.steps: step3. Get clean node from clear node response area
     */
    auto stackNode = cleanNodeResponseArea->cleanNode_;
    ASSERT_NE(stackNode, nullptr);

    /**
     * @tc.steps: step4. Get image node from clean node
     */
    auto imageUiNode = stackNode->GetFirstChild();
    ASSERT_NE(imageUiNode, nullptr);
    auto imageFrameNode = AceType::DynamicCast<FrameNode>(imageUiNode);
    ASSERT_NE(imageFrameNode, nullptr);

    /**
     * @tc.steps: step5. Get image node layout property
     */
    auto imageLayoutProperty = imageFrameNode->GetLayoutProperty<ImageLayoutProperty>();
    ASSERT_NE(imageLayoutProperty, nullptr);

    /**
     * @tc.steps: step6. create text inco size
     */
    auto iconSize = Dimension(ICON_SIZE, DimensionUnit::PX);

    /**
     * @tc.steps: step7. test Update clear node true
     */
    cleanNodeResponseArea->UpdateCleanNode(true);
    EXPECT_EQ(imageLayoutProperty->calcLayoutConstraint_->selfIdealSize,
        CalcSize(CalcLength(iconSize), CalcLength(iconSize)));

    /**
     * @tc.steps: step8. test Update clear node false
     */
    cleanNodeResponseArea->UpdateCleanNode(false);
    EXPECT_EQ(imageLayoutProperty->calcLayoutConstraint_->selfIdealSize, CalcSize(CalcLength(0.0), CalcLength(0.0)));
}

/**
 * @tc.name: CleanNode002
 * @tc.desc: Test OnCleanNodeClicked
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, CleanNode002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Get clean node response area
     */
    auto cleanNodeResponseArea = AceType::DynamicCast<CleanNodeResponseArea>(pattern_->cleanNodeResponseArea_);
    ASSERT_NE(cleanNodeResponseArea, nullptr);

    /**
     * @tc.steps: step3. test clean node clicked
     */
    cleanNodeResponseArea->OnCleanNodeClicked();
    EXPECT_EQ(pattern_->GetTextValue(), "");
}

/**
 * @tc.name: RepeatClickCaret
 * @tc.desc: Test RepeatClickCaret
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, RepeatClickCaret, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Initialize click offset
     */
    Offset clickOffset(0.0f, 0.0f);
    int32_t lastIndex = 0;

    /**
     * @tc.steps: step3. Text input request focus
     */
    auto focusHub = pattern_->GetFocusHub();
    ASSERT_NE(focusHub, nullptr);
    focusHub->currentFocus_ = true;

    /**
     * @tc.steps: step3. test repeat click caret
     */
    EXPECT_TRUE(pattern_->RepeatClickCaret(clickOffset, lastIndex));
}

/**
 * @tc.name: UpdateFocusForward001
 * @tc.desc: Test UpdateFocusForward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusForward001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input.
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Text input request focus.
     */
    GetFocus();

    /**
     * @tc.steps: step3. Test update focus forward when focus index = UNIT.
     */
    pattern_->focusIndex_ = FocuseIndex::UNIT;
    EXPECT_FALSE(pattern_->UpdateFocusForward());
}

/**
 * @tc.name: UpdateFocusForward002
 * @tc.desc: Test UpdateFocusForward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusForward002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input.
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Text input request focus.
     */
    GetFocus();

    /**
     * @tc.steps: step3. show cancel image.
     */
    pattern_->cleanNodeStyle_ = CleanNodeStyle::CONSTANT;
    RunMeasureAndLayout();

    /**
     * @tc.steps: step4. Test update focus forward when focus index = CANCEL.
     */
    pattern_->focusIndex_ = FocuseIndex::CANCEL;
    EXPECT_FALSE(pattern_->UpdateFocusForward());
}

/**
 * @tc.name: UpdateFocusForward003
 * @tc.desc: Test UpdateFocusForward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusForward003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize show password icon text input.
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetType(TextInputType::VISIBLE_PASSWORD);
        model.SetShowPasswordIcon(true);
    });

    /**
     * @tc.steps: step2. Text input request focus.
     */
    GetFocus();

    /**
     * @tc.steps: step3. show cancel image.
     */
    pattern_->cleanNodeStyle_ = CleanNodeStyle::CONSTANT;
    RunMeasureAndLayout();

    /**
     * @tc.steps: step4. Test update focus forward, focus index = CANCEL.
     */
    pattern_->focusIndex_ = FocuseIndex::CANCEL;
    EXPECT_TRUE(pattern_->UpdateFocusForward());
}

/**
 * @tc.name: UpdateFocusForward004
 * @tc.desc: Test UpdateFocusForward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusForward004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize show password icon text input.
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetType(TextInputType::VISIBLE_PASSWORD);
        model.SetShowPasswordIcon(true);
    });

    /**
     * @tc.steps: step2. Text input request focus.
     */
    GetFocus();

    /**
     * @tc.steps: step3. Test update focus forward when focus index = TEXT.
     */
    pattern_->focusIndex_ = FocuseIndex::TEXT;
    EXPECT_TRUE(pattern_->UpdateFocusForward());
}

/**
 * @tc.name: UpdateFocusBackward001
 * @tc.desc: Test UpdateFocusBackward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusBackward001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input.
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Text input request focus.
     */
    GetFocus();

    /**
     * @tc.steps: step3. Test update focus backward when focus index = TEXT.
     */
    pattern_->focusIndex_ = FocuseIndex::TEXT;
    EXPECT_FALSE(pattern_->UpdateFocusBackward());
}

/**
 * @tc.name: UpdateFocusBackward002
 * @tc.desc: Test UpdateFocusBackward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusBackward002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input.
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Text input request focus.
     */
    GetFocus();

    /**
     * @tc.steps: step3. show cancel image.
     */
    pattern_->cleanNodeStyle_ = CleanNodeStyle::CONSTANT;
    RunMeasureAndLayout();

    /**
     * @tc.steps: step4. Test update focus backward when focus index = CANCEL.
     */
    pattern_->focusIndex_ = FocuseIndex::CANCEL;
    EXPECT_TRUE(pattern_->UpdateFocusBackward());
}

/**
 * @tc.name: UpdateFocusBackward003
 * @tc.desc: Test UpdateFocusBackward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusBackward003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize show password icon text input.
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetType(TextInputType::VISIBLE_PASSWORD);
        model.SetShowPasswordIcon(true);
    });

    /**
     * @tc.steps: step2. Text input request focus
     */
    GetFocus();

    /**
     * @tc.steps: step3. Test update focus backward when focus index = UNIT.
     */
    pattern_->focusIndex_ = FocuseIndex::UNIT;
    EXPECT_TRUE(pattern_->UpdateFocusBackward());
}

/**
 * @tc.name: UpdateFocusBackward004
 * @tc.desc: Test UpdateFocusBackward
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, UpdateFocusBackward004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize show password icon text input.
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetType(TextInputType::VISIBLE_PASSWORD);
        model.SetShowPasswordIcon(true);
    });

    /**
     * @tc.steps: step2. Text input request focus.
     */
    GetFocus();

    /**
     * @tc.steps: step3. show cancel image.
     */
    pattern_->cleanNodeStyle_ = CleanNodeStyle::CONSTANT;
    RunMeasureAndLayout();

    /**
     * @tc.steps: step4. Test update focus backward when focus index = UNIT.
     */
    pattern_->focusIndex_ = FocuseIndex::UNIT;
    EXPECT_TRUE(pattern_->UpdateFocusBackward());
}

/**
 * @tc.name: onDraw001
 * @tc.desc: Verify the onDraw Magnifier.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, onDraw001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input and get focus
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.steps: step2. Move handle
     */
    OffsetF localOffset(1.0f, 1.0f);
    pattern_->SetLocalOffset(localOffset);
    RectF handleRect;
    pattern_->OnHandleMove(handleRect, false);

    /**
     * @tc.steps: step3. Craete TextFieldOverlayModifier
     */
    EdgeEffect edgeEffect;
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>(edgeEffect);
    auto textFieldOverlayModifier = AceType::MakeRefPtr<TextFieldOverlayModifier>(pattern_, scrollEdgeEffect);

    /**
     * @tc.steps: step4. Create DrawingContext
     */
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachPen(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    DrawingContext context { rsCanvas, CONTEXT_WIDTH_VALUE, CONTEXT_HEIGHT_VALUE };

    /**
     * @tc.steps: step5. Do onDraw(context)
     */
    textFieldOverlayModifier->onDraw(context);

    /**
     * @tc.steps: step6. Test magnifier open or close
     * @tc.expected: magnifier is open
     */
    auto ret = pattern_->GetShowMagnifier();
    EXPECT_TRUE(ret);

    /**
     * @tc.steps: step7. When handle move done
     */
    pattern_->OnHandleMoveDone(handleRect, true);

    /**
     * @tc.steps: step8. Test magnifier open or close
     * @tc.expected: magnifier is close
     */
    ret = pattern_->GetShowMagnifier();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ShowMenu001
 * @tc.desc: Test close menu after ShowMenu()
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, ShowMenu001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input and get focus
     */
    CreateTextField(DEFAULT_TEXT);
    GetFocus();

    /**
     * @tc.steps: step2. Create selectOverlayProxy
     */
    pattern_->ProcessOverlay(true, true, true);

    /**
     * @tc.steps: step3. Do ShowMenu()
     */
    pattern_->ShowMenu();

    /**
     * @tc.steps: step4. Press esc
     */
    KeyEvent event;
    event.code = KeyCode::KEY_ESCAPE;
    pattern_->OnKeyEvent(event);

    /**
     * @tc.steps: step5. Test menu open or close
     * @tc.expected: text menu is close
     */
    auto ret = pattern_->GetSelectOverlayProxy()->IsMenuShow();
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step6. Show menu when select all value
     */
    pattern_->HandleOnSelectAll(true);
    pattern_->ShowMenu();

    /**
     * @tc.steps: step7. Select all value again
     */
    pattern_->HandleOnSelectAll(true);

    /**
     * @tc.steps: step8. Test menu open or close
     * @tc.expected: text menu is close
     */
    ret = pattern_->GetSelectOverlayProxy()->IsMenuShow();
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step9. Get keyEventHandler
     */
    auto keyEventHandler = AceType::MakeRefPtr<KeyEventHandler>();
    keyEventHandler->UpdateWeakPattern(pattern_);
    ASSERT_NE(keyEventHandler, nullptr);

    /**
     * @tc.steps: step10. Press shift + F10 to open menu
     */
    event.code = KeyCode::KEY_F10;
    keyEventHandler->HandleShiftPressedEvent(event);

    /**
     * @tc.steps: step11. Inset value
     */
    pattern_->InsertValue("abc");

    /**
     * @tc.steps: step12. Test menu open or close
     * @tc.expected: text menu is close
     */
    ret = pattern_->GetSelectOverlayProxy()->IsMenuShow();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: SelectAll001
 * @tc.desc: Test .SelectAll(true)
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, SelectAll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Set .SelectAll(true)
     */
    layoutProperty_->UpdateSelectAllValue(true);

    /**
     * @tc.steps: step3. Get focus by single click
     * @tc.expected: Select all value without handles
     */
    GestureEvent info;
    pattern_->HandleSingleClickEvent(info);
    EXPECT_EQ(pattern_->GetTextSelectController()->GetFirstHandleOffset().GetX(),
        pattern_->GetTextSelectController()->GetSecondHandleOffset().GetX());
}

/**
 * @tc.name: TabGetFocus001
 * @tc.desc: Test select all value when press tab and get focus
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, TabGetFocus001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Get foucs
     */
    GetFocus();

    /**
     * @tc.steps: step3. Get foucs by press tab
     * @tc.expected: Select all value without handles
     */
    KeyEvent event;
    event.code = KeyCode::KEY_TAB;
    pattern_->OnKeyEvent(event);
    EXPECT_EQ(pattern_->GetTextSelectController()->GetFirstHandleOffset().GetX(),
        pattern_->GetTextSelectController()->GetSecondHandleOffset().GetX());
}

/**
 * @tc.name: NeedSoftKeyboard001
 * @tc.desc: Test NeedSoftKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldUXTest, NeedSoftKeyboard001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize text input
     */
    CreateTextField(DEFAULT_TEXT);

    /**
     * @tc.steps: step2. Test whether text field need soft keyboard.
     */
    ASSERT_NE(pattern_, nullptr);
    EXPECT_TRUE(pattern_->NeedSoftKeyboard());
}
} // namespace OHOS::Ace::NG
