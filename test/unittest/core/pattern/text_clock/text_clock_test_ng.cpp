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

#include <optional>
#include <string>
#include <sys/time.h>

#include "gtest/gtest.h"
#define private public
#define protected public
#include "test/mock/core/pipeline/mock_pipeline_base.h"

#include "base/utils/time_util.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/text_clock/text_clock_layout_property.h"
#include "core/components_ng/pattern/text_clock/text_clock_model_ng.h"
#include "core/components_ng/pattern/text_clock/text_clock_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t HOURS_WEST = -8;
inline const std::string CLOCK_FORMAT = "aa h:m:s";
inline const std::string UTC_1 = "1000000000000";
inline const std::string UTC_2 = "2000000000000";
inline const std::string FORMAT_DATA = "08:00:00";
inline const std::string FORM_FORMAT = "hm";
inline const std::vector<std::string> FONT_FAMILY_VALUE = { "cursive" };
const std::string EMPTY_TEXT = "";
const std::string TEXTCLOCK_CONTENT = "08:00:00";
const Dimension FONT_SIZE_VALUE = Dimension(20.1, DimensionUnit::PX);
const Color TEXT_COLOR_VALUE = Color::FromRGB(255, 100, 100);
const Ace::FontStyle ITALIC_FONT_STYLE_VALUE = Ace::FontStyle::ITALIC;
const Ace::FontWeight FONT_WEIGHT_VALUE = Ace::FontWeight::W100;
} // namespace

struct TestProperty {
    std::optional<std::string> format = std::nullopt;
    std::optional<int32_t> hoursWest = std::nullopt;
    std::optional<Dimension> fontSize = std::nullopt;
    std::optional<Color> textColor = std::nullopt;
    std::optional<Ace::FontStyle> italicFontStyle = std::nullopt;
    std::optional<Ace::FontWeight> fontWeight = std::nullopt;
    std::optional<std::vector<std::string>> fontFamily = std::nullopt;
    std::optional<std::vector<Shadow>> textShadow = std::nullopt;
    std::optional<FONT_FEATURES_MAP> fontFeature = std::nullopt;
};

class TextClockTestNG : public testing::Test {
protected:
    static RefPtr<FrameNode> CreateTextClockParagraph(const TestProperty& testProperty);
};

RefPtr<FrameNode> TextClockTestNG::CreateTextClockParagraph(const TestProperty& testProperty)
{
    TextClockModelNG textClockModel;
    textClockModel.Create();
    if (testProperty.format.has_value()) {
        textClockModel.SetFormat(testProperty.format.value());
    }
    if (testProperty.hoursWest.has_value()) {
        textClockModel.SetHoursWest(testProperty.hoursWest.value());
    }
    if (testProperty.fontSize.has_value()) {
        textClockModel.SetFontSize(testProperty.fontSize.value());
    }
    if (testProperty.textColor.has_value()) {
        textClockModel.SetTextColor(testProperty.textColor.value());
    }
    if (testProperty.italicFontStyle.has_value()) {
        textClockModel.SetItalicFontStyle(testProperty.italicFontStyle.value());
    }
    if (testProperty.fontWeight.has_value()) {
        textClockModel.SetFontWeight(testProperty.fontWeight.value());
    }
    if (testProperty.fontFamily.has_value()) {
        textClockModel.SetFontFamily(testProperty.fontFamily.value());
    }
    if (testProperty.textShadow.has_value()) {
        textClockModel.SetTextShadow(testProperty.textShadow.value());
    }
    if (testProperty.fontFeature.has_value()) {
        textClockModel.SetFontFeature(testProperty.fontFeature.value());
    }
    return ViewStackProcessor::GetInstance()->GetMainFrameNode();
}

/**
 * @tc.name: TextClockTest001
 * @tc.desc: Test all the properties of textClock.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of textclock.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional(CLOCK_FORMAT);
    testProperty.hoursWest = std::make_optional(HOURS_WEST);

    /**
     * @tc.steps: step2. create frameNode to get layout properties.
     * @tc.expected: related function is called.
     */
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextClockLayoutProperty> textClockLayoutProperty =
        AceType::DynamicCast<TextClockLayoutProperty>(layoutProperty);
    ASSERT_NE(textClockLayoutProperty, nullptr);

    /**
     * @tc.steps: step3. get the properties of all settings.
     * @tc.expected: check whether the properties is correct.
     */
    EXPECT_EQ(textClockLayoutProperty->GetFormat(), CLOCK_FORMAT);
    EXPECT_EQ(textClockLayoutProperty->GetHoursWest(), HOURS_WEST);

    Shadow shadow;
    shadow.SetBlurRadius(10);
    shadow.SetOffsetX(10);
    shadow.SetOffsetY(10);
    shadow.SetColor(Color(Color::RED));
    shadow.SetShadowType(ShadowType::COLOR);
    std::vector<Shadow> setShadows;
    setShadows.emplace_back(shadow);
    textClockLayoutProperty->UpdateTextShadow(setShadows);
    EXPECT_EQ(textClockLayoutProperty->GetTextShadow(), setShadows);

    Shadow errShadow;
    errShadow.SetBlurRadius(-0.1f);
    errShadow.SetOffsetX(10);
    errShadow.SetOffsetY(10);
    errShadow.SetColor(Color(Color::RED));
    errShadow.SetShadowType(ShadowType::COLOR);
    std::vector<Shadow> errSetShadows;
    errSetShadows.emplace_back(errShadow);
    textClockLayoutProperty->UpdateTextShadow(errSetShadows);
    EXPECT_EQ(textClockLayoutProperty->GetTextShadow(), errSetShadows);
    EXPECT_EQ(errShadow.GetBlurRadius(), 0);

    FONT_FEATURES_MAP fontFeatures;
    fontFeatures.try_emplace("ss01", 1);
    textClockLayoutProperty->UpdateFontFeature(fontFeatures);
    EXPECT_EQ(textClockLayoutProperty->GetFontFeature(), fontFeatures);

    FONT_FEATURES_MAP errFontFeatures;
    errFontFeatures.try_emplace("ss02", 1);
    textClockLayoutProperty->UpdateFontFeature(fontFeatures);
    EXPECT_EQ(textClockLayoutProperty->GetFontFeature(), fontFeatures);

    textClockLayoutProperty->UpdateFontFamily(FONT_FAMILY_VALUE);
    auto json = JsonUtil::Create(true);
    textClockLayoutProperty->ToJsonValue(json);
    EXPECT_EQ(textClockLayoutProperty->GetFontFamily(), FONT_FAMILY_VALUE);
}

/**
 * @tc.name: TextClockTest002
 * @tc.desc: Verify whether the layout property, event and controller functions are created.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock and get frameNode.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional(FORMAT_DATA);
    auto frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);
    auto textNode = AceType::DynamicCast<FrameNode>(frameNode->GetLastChild());
    ASSERT_NE(textNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and create layout property.
     * @tc.expected: related function is called.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextClockLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    EXPECT_NE(textLayoutProperty, nullptr);

    /**
     * @tc.steps: step3. call OnModifyDone function when default properties.
     * @tc.expected: check whether the content is correct.
     */
    pattern->InitTextClockController();
    pattern->OnModifyDone();
    EXPECT_EQ(textLayoutProperty->GetContent(), FORMAT_DATA);

    /**
     * @tc.steps: step4. get controller and create layout property and event.
     * @tc.expected: related function is called.
     */
    auto controller = pattern->GetTextClockController();
    ASSERT_NE(controller, nullptr);
    controller->Start();
    controller->Stop();
    EXPECT_EQ(textLayoutProperty->GetContent(), FORMAT_DATA);
    auto clockLayoutProperty = pattern->CreateLayoutProperty();
    ASSERT_NE(clockLayoutProperty, nullptr);
    auto event = pattern->CreateEventHub();
    ASSERT_NE(event, nullptr);

    /**
     * @tc.steps: step5. garbage branch coverage.
     * @tc.expected: related function is called.
     */
    pattern->isStart_ = false;
    pattern->UpdateTimeText();
    pattern->textClockController_ = nullptr;
    pattern->InitUpdateTimeTextCallBack();
    EXPECT_EQ(pattern->textClockController_, nullptr);
    EXPECT_EQ(textLayoutProperty->GetContent(), FORMAT_DATA);
}

/**
 * @tc.name: TextClockTest003
 * @tc.desc: Test event function of textclock.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock and set event.
     */
    TextClockModelNG textClockModel;
    textClockModel.Create();
    std::string utc = UTC_1;
    auto onChange = [&utc](const std::string& isUtc) { utc = isUtc; };
    textClockModel.SetOnDateChange(onChange);

    /**
     * @tc.steps: step2. get textclock frameNode and event.
     * @tc.expected: function is called.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextClockEventHub> eventHub = frameNode->GetEventHub<NG::TextClockEventHub>();
    ASSERT_NE(eventHub, nullptr);

    /**
     * @tc.steps: step3. call the event entry function.
     * @tc.expected: check whether the value is correct.
     */
    eventHub->FireChangeEvent(UTC_2);
    EXPECT_EQ(utc, UTC_2);
}

/**
 * @tc.name: TextClockTest004
 * @tc.desc: Test GetHourWest function of textclock.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize the format property of textclock.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional(CLOCK_FORMAT);

    /**
     * @tc.steps: step2. create frameNode to get layout properties.
     * @tc.expected: related function is called.
     */
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextClockLayoutProperty> textClockLayoutProperty =
        AceType::DynamicCast<TextClockLayoutProperty>(layoutProperty);
    ASSERT_NE(textClockLayoutProperty, nullptr);
    /**
     * @tc.steps: step3. get pattern.
     * @tc.expected: related function is called and return right value.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->GetHoursWest(), INT_MAX);
    textClockLayoutProperty->UpdateHoursWest(HOURS_WEST);
    EXPECT_EQ(pattern->GetHoursWest(), HOURS_WEST);
}

/**
 * @tc.name: TextClockTest005
 * @tc.desc: Test the fuction of parse format.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of textclock.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional("M月d日yy年y E EEEE HH:mm:ss.SSS aa");
    testProperty.hoursWest = std::make_optional(HOURS_WEST);
    /**
     * @tc.steps: step2. create frameNode to get layout properties.
     * @tc.expected: related function is called.
     */
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextClockLayoutProperty> textClockLayoutProperty =
        AceType::DynamicCast<TextClockLayoutProperty>(layoutProperty);
    ASSERT_NE(textClockLayoutProperty, nullptr);
    /**
     * @tc.steps: step3. get pattern.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step4. call the format and datetime split, and datetime splice function.
     * @tc.expected: check whether the value is correct.
     */
    bool is24H = false;
    pattern->ParseInputFormat(is24H);
    std::vector<std::string> curDateTime = { "1900", "0", "1", "0", "0", "0", "0", "", "2" };
    std::string dateTimeValue = "2023/07/08, 下午8:35:07.007";
    curDateTime = pattern->ParseDateTimeValue(dateTimeValue);
    dateTimeValue = "7/8/2023, 8:35:07.67 am";
    curDateTime = pattern->ParseDateTimeValue(dateTimeValue);
    dateTimeValue = "07/08/2023, 20:35:07.007";
    curDateTime = pattern->ParseDateTimeValue(dateTimeValue);
    pattern->SpliceDateTime(curDateTime);
    pattern->CheckDateTimeElement(curDateTime, 'y', 0, true);
    pattern->CheckDateTimeElement(curDateTime, 'M', 1, true);
    pattern->CheckDateTimeElement(curDateTime, 'd', 2, true);
    pattern->CheckDateTimeElement(curDateTime, 'm', 4, true);
    pattern->CheckDateTimeElement(curDateTime, 'E', 13, true);
    pattern->CheckDateTimeElement(curDateTime, 'E', 8, true);
    EXPECT_EQ(is24H, true);
}

/**
 * @tc.name: TextClockTest006
 * @tc.desc: Test the fuction of parse format.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize all properties of textclock.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional("yyyy-M-M-d-d EEEE hh:mm:ss.SS.SSS aa");
    testProperty.hoursWest = std::make_optional(HOURS_WEST);

    /**
     * @tc.steps: step2. create frameNode to get layout properties.
     * @tc.expected: related function is called.
     */
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<LayoutProperty> layoutProperty = frameNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    RefPtr<TextClockLayoutProperty> textClockLayoutProperty =
        AceType::DynamicCast<TextClockLayoutProperty>(layoutProperty);
    ASSERT_NE(textClockLayoutProperty, nullptr);

    /**
     * @tc.steps: step3. get pattern.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step4. call the format split function.
     * @tc.expected: check whether the value is correct.
     */
    bool is24H = false;
    pattern->GetWeek(true, 3);
    pattern->GetWeek(false, 5);
    pattern->GetDigitNumber("12345abcde-=_+");
    pattern->ParseInputFormat(is24H);
    EXPECT_EQ(is24H, false);
}

/**
 * @tc.name: TextClockAccessibilityPropertyIsScrollable001
 * @tc.desc: Test IsScrollable of textClockAccessibilityProperty.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockAccessibilityPropertyIsScrollable001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::TEXTCLOCK_ETS_TAG,
        ViewStackProcessor::GetInstance()->ClaimNodeId(), []() { return AceType::MakeRefPtr<TextClockPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    auto textLayoutProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
    EXPECT_NE(textLayoutProperty, nullptr);
    auto textClockAccessibilityProperty = frameNode->GetAccessibilityProperty<TextClockAccessibilityProperty>();
    ASSERT_NE(textClockAccessibilityProperty, nullptr);
    textClockAccessibilityProperty->SetHost(AceType::WeakClaim(AceType::RawPtr(frameNode)));

    EXPECT_EQ(textClockAccessibilityProperty->GetText(), EMPTY_TEXT);

    textLayoutProperty->UpdateContent(TEXTCLOCK_CONTENT);
    EXPECT_EQ(textClockAccessibilityProperty->GetText(), TEXTCLOCK_CONTENT);
}

/**
 * @tc.name: TextClockTest007
 * @tc.desc: Test UpdateTextLayoutProperty of TextClockPattern.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock frameNode.
     */
    TestProperty testProperty;
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and layoutProperty.
     * @tc.expected: UpdateTextLayoutProperty function is called.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    auto host = pattern->GetHost();
    ASSERT_NE(host, nullptr);
    auto textClockProperty = frameNode->GetLayoutProperty<TextClockLayoutProperty>();
    ASSERT_NE(textClockProperty, nullptr);
    auto textLayoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    pattern->UpdateTextLayoutProperty(textClockProperty, textLayoutProperty);

    /**
     * @tc.steps: step3. get the properties of all settings.
     * @tc.expected: step3. check whether the properties is correct.
     */
    EXPECT_FALSE(textLayoutProperty->HasFontSize());
    EXPECT_FALSE(textLayoutProperty->HasTextColor());
    EXPECT_FALSE(textLayoutProperty->HasItalicFontStyle());
    EXPECT_FALSE(textLayoutProperty->HasFontWeight());
    EXPECT_FALSE(textLayoutProperty->HasFontFamily());

    auto textNode = AceType::DynamicCast<FrameNode>(host->GetLastChild());
    ASSERT_NE(textNode, nullptr);
    textNode->tag_ = V2::TEXTCLOCK_ETS_TAG;
    EXPECT_EQ(pattern->GetTextNode(), nullptr);
}

/**
 * @tc.name: TextClockTest008
 * @tc.desc: Test UpdateTextLayoutProperty of TextClockPattern.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock frameNode.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional(CLOCK_FORMAT);
    testProperty.hoursWest = std::make_optional(HOURS_WEST);
    testProperty.fontSize = std::make_optional(FONT_SIZE_VALUE);
    testProperty.fontWeight = std::make_optional(FONT_WEIGHT_VALUE);
    testProperty.textColor = std::make_optional(TEXT_COLOR_VALUE);
    testProperty.italicFontStyle = std::make_optional(ITALIC_FONT_STYLE_VALUE);
    testProperty.fontFamily = std::make_optional(FONT_FAMILY_VALUE);
    Shadow shadow;
    shadow.SetBlurRadius(10);
    shadow.SetOffsetX(20);
    shadow.SetOffsetY(20);
    shadow.SetColor(Color(Color::BLACK));
    shadow.SetShadowType(ShadowType::COLOR);
    std::vector<Shadow> setShadows;
    setShadows.emplace_back(shadow);
    testProperty.textShadow = std::make_optional(setShadows);
    FONT_FEATURES_MAP fontFeatures;
    fontFeatures.try_emplace("ss02", 1);
    testProperty.fontFeature = std::make_optional(fontFeatures);

    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and layoutProperty.
     * @tc.expected: UpdateTextLayoutProperty function is called.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    auto host = pattern->GetHost();
    ASSERT_NE(host, nullptr);
    auto textClockProperty = frameNode->GetLayoutProperty<TextClockLayoutProperty>();
    ASSERT_NE(textClockProperty, nullptr);
    auto textLayoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textLayoutProperty, nullptr);

    pattern->UpdateTextLayoutProperty(textClockProperty, textLayoutProperty);

    /**
     * @tc.steps: step3. get the properties of all settings.
     * @tc.expected: step3. check whether the properties is correct.
     */
    EXPECT_EQ(textLayoutProperty->GetFontSize(), FONT_SIZE_VALUE);
    EXPECT_EQ(textLayoutProperty->GetTextColor(), TEXT_COLOR_VALUE);
    EXPECT_EQ(textLayoutProperty->GetItalicFontStyle(), ITALIC_FONT_STYLE_VALUE);
    EXPECT_EQ(textLayoutProperty->GetFontWeight(), FONT_WEIGHT_VALUE);
    EXPECT_EQ(textLayoutProperty->GetFontFamily(), FONT_FAMILY_VALUE);
    EXPECT_EQ(textLayoutProperty->GetTextShadow(), setShadows);
    EXPECT_EQ(textLayoutProperty->GetFontFeature(), fontFeatures);
}

/**
 * @tc.name: TextClockTest009
 * @tc.desc: Test UpdateTextLayoutProperty of TextClockPattern.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock frameNode.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional(CLOCK_FORMAT);
    testProperty.hoursWest = std::make_optional(HOURS_WEST);
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern.
     * @tc.expected: step2.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step3. ParseDateTimeValue function is called..
     * @tc.expected: step3. check whether the properties is correct.
     */
    std::vector<std::string> strVec = { "1900", "0", "1", "0", "0", "0", "0", "", "0", "", "", "", "", "" };
    std::string strDateTimeValue = "1970.01.01";
    std::vector<std::string> str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec);

    strDateTimeValue = "1970/01.01";
    str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec);

    strDateTimeValue = "，1970/01/01";
    str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec);

    std::vector<std::string> strVec2 = { "1970", "01", "01", "0", "0", "0", "0", "", "0", "70", "1", "1", "0", "" };
    strDateTimeValue = "1970/01/01,";
    str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec2);

    strDateTimeValue = "1970/01/01 ";
    str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec2);

    strDateTimeValue = "1970/01/01, 01:01";
    str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec2);

    strDateTimeValue = "1970/01/01, 01:01.001:01";
    str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec2);

    std::vector<std::string> strVec3 = { "1970", "01", "01", "01", "01", "01", "001", "", "0", "70", "1", "1", "00",
        "" };
    strDateTimeValue = "1970/01/01, 01:01:01.001";
    str = pattern->ParseDateTimeValue(strDateTimeValue);
    EXPECT_EQ(str, strVec3);
}

/**
 * @tc.name: TextClockTest010
 * @tc.desc: Test time format in card.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock frameNode.
     */
    TestProperty testProperty;
    testProperty.hoursWest = std::make_optional(HOURS_WEST);
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. get pattern and textClockLayoutProperty.
     * @tc.expected: step3.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textClockLayoutProperty = frameNode->GetLayoutProperty<TextClockLayoutProperty>();
    ASSERT_NE(textClockLayoutProperty, nullptr);

    /**
     * @tc.steps: step3. set is form render.
     */
    MockPipelineBase::SetUp();
    auto pipeline = MockPipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->SetIsFormRender(true);
    EXPECT_FALSE(pattern->isForm_);

    /**
     * @tc.steps: step4. isForm_ is change to true.
     * @tc.expected: check default time format is correct
     */
    pattern->isStart_ = false;
    pattern->InitUpdateTimeTextCallBack();
    EXPECT_TRUE(pattern->isForm_);
    // textClockLayoutProperty.format is std::nullopt
    auto format = pattern->GetFormat();
    EXPECT_EQ(format, FORM_FORMAT);

    /**
     * @tc.steps: step5. check time format in card.
     * @tc.expected: get crrect time format,format split function is crrect.
     */
    textClockLayoutProperty->UpdateFormat(CLOCK_FORMAT);
    format = pattern->GetFormat();
    EXPECT_EQ(format, FORM_FORMAT);
    textClockLayoutProperty->UpdateFormat("mm:SS");
    format = pattern->GetFormat();
    EXPECT_EQ(format, FORM_FORMAT);
    bool is24H = false;
    pattern->GetWeek(true, 3);
    pattern->GetWeek(false, 5);
    pattern->ParseInputFormat(is24H);
    EXPECT_EQ(is24H, false);

    /**
     * @tc.steps: step6. isForm_ is change to false.
     * @tc.expected: check default time format is correct
     */
    pattern->isForm_ = false;
    pipeline->SetIsFormRender(false);
    pattern->InitUpdateTimeTextCallBack();
    EXPECT_FALSE(pattern->isForm_);
    EXPECT_EQ(textClockLayoutProperty->GetFormat(), "mm:SS");

    pipeline = nullptr;
    MockPipelineBase::TearDown();
}

/**
 * @tc.name: TextClockTest011
 * @tc.desc: Test event function when TextClock visible be changed.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock and set event.
     */
    TextClockModelNG textClockModel;
    textClockModel.Create();
    std::string utc = UTC_1;
    auto onChange = [&utc](const std::string& isUtc) { utc = UTC_2; };
    textClockModel.SetOnDateChange(onChange);

    /**
     * @tc.steps: step2. get textclock frameNode and event.
     * @tc.expected: function is called.
     */
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(frameNode, nullptr);
    RefPtr<TextClockEventHub> eventHub = frameNode->GetEventHub<NG::TextClockEventHub>();
    ASSERT_NE(eventHub, nullptr);

    /**
     * @tc.steps: step3. get pattern.
     * @tc.expected: step4.
     */
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isStart_ = false;
    MockPipelineBase::SetUp();
    pattern->InitUpdateTimeTextCallBack();
    pattern->RegistVisibleAreaChangeCallback();

    /**
     * @tc.steps: step4. VisibleArea be changed, call the event entry function.
     * @tc.expected: check whether the value is correct.
     */
    pattern->isStart_ = true;
    pattern->OnVisibleAreaChange(false);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_1);
    pattern->OnVisibleAreaChange(true);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_2);

    /**
     * @tc.steps: step5. visible attribute be changed, call the event entry function.
     * @tc.expected: check whether the value is correct.
     */
    utc = UTC_1;
    pattern->OnVisibleChange(false);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_1);
    pattern->OnVisibleChange(true);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_1);
    pattern->prevTime_ = "";
    pattern->OnVisibleChange(true);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_2);

    /**
     * @tc.steps: step6. form visible be changed, call the event entry function.
     * @tc.expected: check whether the value is correct.
     */
    utc = UTC_1;
    pattern->OnFormVisibleChange(false);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_1);
    pattern->OnFormVisibleChange(true);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_1);
    pattern->prevTime_ = "";
    pattern->OnFormVisibleChange(true);
    pattern->UpdateTimeText();
    EXPECT_EQ(utc, UTC_2);
    MockPipelineBase::TearDown();
}

/**
 * @tc.name: TextClockLayoutAlgorithm001
 * @tc.desc: Test TextClockLayoutAlgorithm of TextClock.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockLayoutAlgorithm001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock frameNode.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional(CLOCK_FORMAT);
    testProperty.hoursWest = std::make_optional(HOURS_WEST);
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextClockLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetFormat(), CLOCK_FORMAT);
    EXPECT_EQ(layoutProperty->GetHoursWest(), HOURS_WEST);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, layoutProperty);
    ASSERT_NE(layoutWrapper, nullptr);

    /**
     * @tc.steps: step2. create childFrameNode.
     * @tc.expected: step2. check whether the properties is correct.
     */
    auto textNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXT_ETS_TAG, textNodeId, []() { return AceType::MakeRefPtr<TextPattern>(); });
    ASSERT_NE(textNode, nullptr);
    RefPtr<GeometryNode> textGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto textLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(textNode, textGeometryNode, textNode->GetLayoutProperty());
    ASSERT_NE(textLayoutWrapper, nullptr);
    textLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(textNode->GetPattern()->CreateLayoutAlgorithm()));
    textNode->MountToParent(frameNode);
    layoutWrapper->AppendChild(textLayoutWrapper);
    EXPECT_EQ(layoutWrapper->currentChildCount_, 1);

    LayoutConstraintF contentConstraint;
    contentConstraint.maxSize = SizeF(720.f, 1136.f);
    contentConstraint.percentReference = SizeF(720.f, 1136.f);
    contentConstraint.parentIdealSize.SetSize(SizeF(720.f, 1136.f));
    layoutProperty->UpdateLayoutConstraint(contentConstraint);
    layoutProperty->UpdateContentConstraint();
    auto layoutAlgorithm = AceType::MakeRefPtr<TextClockLayoutAlgorithm>();
    layoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(geometryNode->GetFrameSize(), SizeF());

    contentConstraint.selfIdealSize.SetSize(SizeF(200.f, 200.f));
    layoutProperty->UpdateLayoutConstraint(contentConstraint);
    layoutProperty->UpdateContentConstraint();
    layoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(geometryNode->GetFrameSize(), SizeF(200.f, 200.f));
}

/**
 * @tc.name: TextClockTest012
 * @tc.desc: Test TextClockPattern of TextClock.
 * @tc.type: FUNC
 */
HWTEST_F(TextClockTestNG, TextClockTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create textclock frameNode.
     */
    TestProperty testProperty;
    testProperty.format = std::make_optional(CLOCK_FORMAT);
    testProperty.hoursWest = std::make_optional(HOURS_WEST);
    RefPtr<FrameNode> frameNode = CreateTextClockParagraph(testProperty);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextClockPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<TextClockLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetFormat(), CLOCK_FORMAT);
    EXPECT_EQ(layoutProperty->GetHoursWest(), HOURS_WEST);

    /**
     * @tc.steps: step2. get the properties of all settings.
     * @tc.expected: step2. check whether the properties is correct.
     */
    pattern->isForm_ = true;
    pattern->OnVisibleAreaChange(true);
    EXPECT_TRUE(pattern->isInVisibleArea_);
    EXPECT_TRUE(pattern->isForm_);

    pattern->OnVisibleChange(false);
    pattern->OnVisibleAreaChange(false);
    pattern->OnFormVisibleChange(false);
    EXPECT_FALSE(pattern->isSetVisible_);
    EXPECT_FALSE(pattern->isInVisibleArea_);
    EXPECT_FALSE(pattern->isFormVisible_);

    pattern->OnVisibleChange(true);
    pattern->OnVisibleAreaChange(true);
    pattern->OnFormVisibleChange(true);
    EXPECT_TRUE(pattern->isSetVisible_);
    EXPECT_TRUE(pattern->isInVisibleArea_);
    EXPECT_TRUE(pattern->isFormVisible_);
}
} // namespace OHOS::Ace::NG