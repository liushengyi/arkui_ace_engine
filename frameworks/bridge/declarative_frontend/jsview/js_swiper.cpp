/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/jsview/js_swiper.h"

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "base/log/ace_scoring_log.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/engine/functions/js_click_function.h"
#include "bridge/declarative_frontend/engine/functions/js_swiper_function.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "bridge/declarative_frontend/jsview/models/swiper_model_impl.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "bridge/js_frontend/engine/jsi/js_value.h"
#include "core/animation/curve.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/scroll_bar.h"
#include "core/components/swiper/swiper_component.h"
#include "core/components/swiper/swiper_indicator_theme.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"
#include "core/components_ng/pattern/swiper/swiper_model_ng.h"

namespace OHOS::Ace {
namespace {
constexpr float ARROW_SIZE_COEFFICIENT = 0.75f;
} // namespace
std::unique_ptr<SwiperModel> SwiperModel::instance_ = nullptr;
std::mutex SwiperModel::mutex_;

SwiperModel* SwiperModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::SwiperModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::SwiperModelNG());
            } else {
                instance_.reset(new Framework::SwiperModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {
namespace {

const std::vector<EdgeEffect> EDGE_EFFECT = { EdgeEffect::SPRING, EdgeEffect::FADE, EdgeEffect::NONE };
const std::vector<SwiperDisplayMode> DISPLAY_MODE = { SwiperDisplayMode::STRETCH, SwiperDisplayMode::AUTO_LINEAR };
const std::vector<SwiperIndicatorType> INDICATOR_TYPE = { SwiperIndicatorType::DOT, SwiperIndicatorType::DIGIT };
const static int32_t DEFAULT_INTERVAL = 3000;
const static int32_t DEFAULT_DURATION = 400;
const static int32_t DEFAULT_DISPLAY_COUNT = 1;
const static int32_t DEFAULT_CACHED_COUNT = 1;

JSRef<JSVal> SwiperChangeEventToJSValue(const SwiperChangeEvent& eventInfo)
{
    return JSRef<JSVal>::Make(ToJSValue(eventInfo.GetIndex()));
}

} // namespace

void JSSwiper::Create(const JSCallbackInfo& info)
{
    auto controller = SwiperModel::GetInstance()->Create();

    if (info.Length() > 0 && info[0]->IsObject()) {
        auto* jsController = JSRef<JSObject>::Cast(info[0])->Unwrap<JSSwiperController>();
        if (jsController) {
            jsController->SetController(controller);
        }
    }
}

void JSSwiper::JsRemoteMessage(const JSCallbackInfo& info)
{
    RemoteCallback remoteCallback;
    JSInteractableView::JsRemoteMessage(info, remoteCallback);

    SwiperModel::GetInstance()->SetRemoteMessageEventId(std::move(remoteCallback));
}

void JSSwiper::JSBind(BindingTarget globalObj)
{
    JSClass<JSSwiper>::Declare("Swiper");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSSwiper>::StaticMethod("create", &JSSwiper::Create, opt);
    JSClass<JSSwiper>::StaticMethod("autoPlay", &JSSwiper::SetAutoPlay, opt);
    JSClass<JSSwiper>::StaticMethod("duration", &JSSwiper::SetDuration, opt);
    JSClass<JSSwiper>::StaticMethod("index", &JSSwiper::SetIndex, opt);
    JSClass<JSSwiper>::StaticMethod("interval", &JSSwiper::SetInterval, opt);
    JSClass<JSSwiper>::StaticMethod("loop", &JSSwiper::SetLoop, opt);
    JSClass<JSSwiper>::StaticMethod("vertical", &JSSwiper::SetVertical, opt);
    JSClass<JSSwiper>::StaticMethod("indicator", &JSSwiper::SetIndicator, opt);
    JSClass<JSSwiper>::StaticMethod("displayMode", &JSSwiper::SetDisplayMode);
    JSClass<JSSwiper>::StaticMethod("effectMode", &JSSwiper::SetEffectMode);
    JSClass<JSSwiper>::StaticMethod("displayCount", &JSSwiper::SetDisplayCount);
    JSClass<JSSwiper>::StaticMethod("itemSpace", &JSSwiper::SetItemSpace);
    JSClass<JSSwiper>::StaticMethod("prevMargin", &JSSwiper::SetPreviousMargin);
    JSClass<JSSwiper>::StaticMethod("nextMargin", &JSSwiper::SetNextMargin);
    JSClass<JSSwiper>::StaticMethod("cachedCount", &JSSwiper::SetCachedCount);
    JSClass<JSSwiper>::StaticMethod("curve", &JSSwiper::SetCurve);
    JSClass<JSSwiper>::StaticMethod("onChange", &JSSwiper::SetOnChange);
    JSClass<JSSwiper>::StaticMethod("onAnimationStart", &JSSwiper::SetOnAnimationStart);
    JSClass<JSSwiper>::StaticMethod("onAnimationEnd", &JSSwiper::SetOnAnimationEnd);
    JSClass<JSSwiper>::StaticMethod("onGestureSwipe", &JSSwiper::SetOnGestureSwipe);
    JSClass<JSSwiper>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSSwiper>::StaticMethod("onHover", &JSInteractableView::JsOnHover);
    JSClass<JSSwiper>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSSwiper>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSSwiper>::StaticMethod("remoteMessage", &JSSwiper::JsRemoteMessage);
    JSClass<JSSwiper>::StaticMethod("onClick", &JSSwiper::SetOnClick);
    JSClass<JSSwiper>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSSwiper>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSSwiper>::StaticMethod("indicatorStyle", &JSSwiper::SetIndicatorStyle);
    JSClass<JSSwiper>::StaticMethod("enabled", &JSSwiper::SetEnabled);
    JSClass<JSSwiper>::StaticMethod("disableSwipe", &JSSwiper::SetDisableSwipe);
    JSClass<JSSwiper>::StaticMethod("height", &JSSwiper::SetHeight);
    JSClass<JSSwiper>::StaticMethod("width", &JSSwiper::SetWidth);
    JSClass<JSSwiper>::StaticMethod("size", &JSSwiper::SetSize);
    JSClass<JSSwiper>::StaticMethod("displayArrow", &JSSwiper::SetDisplayArrow);
    JSClass<JSSwiper>::StaticMethod("nestedScroll", &JSSwiper::SetNestedScroll);
    JSClass<JSSwiper>::InheritAndBind<JSContainerBase>(globalObj);
}

void JSSwiper::SetAutoPlay(bool autoPlay)
{
    SwiperModel::GetInstance()->SetAutoPlay(autoPlay);
}

void JSSwiper::SetEnabled(const JSCallbackInfo& info)
{
    JSViewAbstract::JsEnabled(info);
    if (info.Length() < 1) {
        return;
    }

    if (!info[0]->IsBoolean()) {
        return;
    }

    SwiperModel::GetInstance()->SetEnabled(info[0]->ToBoolean());
}

void JSSwiper::SetDisableSwipe(bool disableSwipe)
{
    SwiperModel::GetInstance()->SetDisableSwipe(disableSwipe);
}

void JSSwiper::SetEffectMode(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    if (!info[0]->IsNumber()) {
        return;
    }

    auto edgeEffect = info[0]->ToNumber<int32_t>();
    if (edgeEffect < 0 || edgeEffect >= static_cast<int32_t>(EDGE_EFFECT.size())) {
        return;
    }

    SwiperModel::GetInstance()->SetEdgeEffect(EDGE_EFFECT[edgeEffect]);
}

void JSSwiper::SetDisplayCount(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        if (info[0]->IsString() && info[0]->ToString() == "auto") {
            SwiperModel::GetInstance()->SetDisplayMode(SwiperDisplayMode::AUTO_LINEAR);
            SwiperModel::GetInstance()->ResetDisplayCount();
        } else if (info[0]->IsNumber() && info[0]->ToNumber<int32_t>() > 0) {
            SwiperModel::GetInstance()->SetDisplayCount(info[0]->ToNumber<int32_t>());
        } else if (info[0]->IsObject()) {
            JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(info[0]);
            auto minSizeParam = jsObj->GetProperty("minSize");
            if (minSizeParam->IsNull()) {
                return;
            }
            CalcDimension minSizeValue;
            if (!ParseJsDimensionVp(minSizeParam, minSizeValue)) {
                SwiperModel::GetInstance()->SetMinSize(0.0_vp);
                return;
            }
            SwiperModel::GetInstance()->SetMinSize(minSizeValue);
        } else {
            SwiperModel::GetInstance()->SetDisplayCount(DEFAULT_DISPLAY_COUNT);
        }

        return;
    }

    if (info[0]->IsString() && info[0]->ToString() == "auto") {
        SwiperModel::GetInstance()->SetDisplayMode(SwiperDisplayMode::AUTO_LINEAR);
        SwiperModel::GetInstance()->ResetDisplayCount();
    } else if (info[0]->IsNumber()) {
        SwiperModel::GetInstance()->SetDisplayCount(info[0]->ToNumber<int32_t>());
    }
}

void JSSwiper::SetDuration(const JSCallbackInfo& info)
{
    int32_t duration = DEFAULT_DURATION;

    if (info.Length() < 1) { // user do not set any value
        return;
    }

    // undefined value turn to default 400
    if (!info[0]->IsUndefined() && info[0]->IsNumber()) {
        duration = info[0]->ToNumber<int32_t>();
        if (duration < 0) {
            duration = DEFAULT_DURATION;
        }
    }

    SwiperModel::GetInstance()->SetDuration(duration);
}

void ParseSwiperIndexObject(const JSCallbackInfo& args, const JSRef<JSVal>& changeEventVal)
{
    CHECK_NULL_VOID(changeEventVal->IsFunction());

    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(changeEventVal));
    auto onIndex = [execCtx = args.GetExecutionContext(), func = std::move(jsFunc)](const BaseEventInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("Swiper.onChangeEvent");
        const auto* swiperInfo = TypeInfoHelper::DynamicCast<SwiperChangeEvent>(info);
        if (!swiperInfo) {
            return;
        }
        auto newJSVal = JSRef<JSVal>::Make(ToJSValue(swiperInfo->GetIndex()));
        func->ExecuteJS(1, &newJSVal);
    };
    SwiperModel::GetInstance()->SetOnChangeEvent(std::move(onIndex));
}

void JSSwiper::SetIndex(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || info.Length() > 2) {
        return;
    }

    int32_t index = 0;
    if (info.Length() > 0 && info[0]->IsNumber()) {
        index = info[0]->ToNumber<int32_t>();
    }

    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        index = index < 0 ? 0 : index;
    }

    if (index < 0) {
        return;
    }
    SwiperModel::GetInstance()->SetIndex(index);

    if (info.Length() > 1 && info[1]->IsFunction()) {
        ParseSwiperIndexObject(info, info[1]);
    }
}

void JSSwiper::SetInterval(const JSCallbackInfo& info)
{
    int32_t interval = DEFAULT_INTERVAL;

    if (info.Length() < 1) { // user do not set any value
        return;
    }

    // undefined value turn to default 3000
    if (!info[0]->IsUndefined() && info[0]->IsNumber()) {
        interval = info[0]->ToNumber<int32_t>();
        if (interval < 0) {
            interval = DEFAULT_INTERVAL;
        }
    }

    SwiperModel::GetInstance()->SetAutoPlayInterval(interval);
}

void JSSwiper::SetLoop(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_TEN)) {
        SwiperModel::GetInstance()->SetLoop(info[0]->ToBoolean());
        return;
    }

    if (info[0]->IsBoolean()) {
        SwiperModel::GetInstance()->SetLoop(info[0]->ToBoolean());
    } else {
        SwiperModel::GetInstance()->SetLoop(true);
    }
}

void JSSwiper::SetVertical(bool isVertical)
{
    SwiperModel::GetInstance()->SetDirection(isVertical ? Axis::VERTICAL : Axis::HORIZONTAL);
}

void JSSwiper::GetFontContent(const JSRef<JSVal>& font, bool isSelected, SwiperDigitalParameters& digitalParameters)
{
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(font);
    JSRef<JSVal> size = obj->GetProperty("size");
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_VOID(swiperIndicatorTheme);
    // set font size, unit FP
    CalcDimension fontSize;
    if (!size->IsUndefined() && !size->IsNull() && ParseJsDimensionFp(size, fontSize)) {
        if (LessOrEqual(fontSize.Value(), 0.0) || LessOrEqual(size->ToNumber<double>(), 0.0) ||
            fontSize.Unit() == DimensionUnit::PERCENT) {
            fontSize = swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetFontSize();
        }
    } else {
        fontSize = swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetFontSize();
    }
    if (isSelected) {
        digitalParameters.selectedFontSize = fontSize;
    } else {
        digitalParameters.fontSize = fontSize;
    }
    JSRef<JSVal> weight = obj->GetProperty("weight");
    if (!weight->IsNull()) {
        std::string weightValue;
        if (weight->IsNumber()) {
            weightValue = std::to_string(weight->ToNumber<int32_t>());
        } else {
            ParseJsString(weight, weightValue);
        }
        if (isSelected) {
            digitalParameters.selectedFontWeight = ConvertStrToFontWeight(weightValue);
        } else {
            digitalParameters.fontWeight = ConvertStrToFontWeight(weightValue);
        }
    } else {
        if (isSelected) {
            digitalParameters.selectedFontWeight = swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetFontWeight();
        } else {
            digitalParameters.fontWeight = swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetFontWeight();
        }
    }
}

void JSSwiper::SetIsIndicatorCustomSize(const Dimension& dimPosition, bool parseOk)
{
    if (parseOk && dimPosition > 0.0_vp) {
        SwiperModel::GetInstance()->SetIsIndicatorCustomSize(true);
    } else {
        SwiperModel::GetInstance()->SetIsIndicatorCustomSize(false);
    }
}

SwiperParameters JSSwiper::GetDotIndicatorInfo(const JSRef<JSObject>& obj)
{
    JSRef<JSVal> leftValue = obj->GetProperty("leftValue");
    JSRef<JSVal> topValue = obj->GetProperty("topValue");
    JSRef<JSVal> rightValue = obj->GetProperty("rightValue");
    JSRef<JSVal> bottomValue = obj->GetProperty("bottomValue");
    JSRef<JSVal> itemWidthValue = obj->GetProperty("itemWidthValue");
    JSRef<JSVal> itemHeightValue = obj->GetProperty("itemHeightValue");
    JSRef<JSVal> selectedItemWidthValue = obj->GetProperty("selectedItemWidthValue");
    JSRef<JSVal> selectedItemHeightValue = obj->GetProperty("selectedItemHeightValue");
    JSRef<JSVal> maskValue = obj->GetProperty("maskValue");
    JSRef<JSVal> colorValue = obj->GetProperty("colorValue");
    JSRef<JSVal> selectedColorValue = obj->GetProperty("selectedColorValue");
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, SwiperParameters());
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_RETURN(swiperIndicatorTheme, SwiperParameters());
    bool parseOk = false;
    SwiperParameters swiperParameters;
    CalcDimension dimPosition;
    parseOk = ParseJsDimensionVp(leftValue, dimPosition);
    if (parseOk) {
        if (dimPosition.ConvertToPx() < 0.0f) {
            dimPosition = 0.0_vp;
        }
    } else {
        dimPosition = 0.0_vp;
    }
    swiperParameters.dimLeft = dimPosition;
    parseOk = ParseJsDimensionVp(topValue, dimPosition);
    swiperParameters.dimTop = parseOk ? dimPosition : 0.0_vp;
    parseOk = ParseJsDimensionVp(rightValue, dimPosition);
    swiperParameters.dimRight = parseOk ? dimPosition : 0.0_vp;
    parseOk = ParseJsDimensionVp(bottomValue, dimPosition);
    swiperParameters.dimBottom = parseOk ? dimPosition : 0.0_vp;
    bool parseItemWOk =
        ParseJsDimensionVp(itemWidthValue, dimPosition) && (dimPosition.Unit() != DimensionUnit::PERCENT);
    auto defaultSize = swiperIndicatorTheme->GetSize();
    swiperParameters.itemWidth = parseItemWOk && dimPosition > 0.0_vp ? dimPosition : defaultSize;
    bool parseItemHOk =
        ParseJsDimensionVp(itemHeightValue, dimPosition) && (dimPosition.Unit() != DimensionUnit::PERCENT);
    swiperParameters.itemHeight = parseItemHOk && dimPosition > 0.0_vp ? dimPosition : defaultSize;
    bool parseSeleItemWOk =
        ParseJsDimensionVp(selectedItemWidthValue, dimPosition) && (dimPosition.Unit() != DimensionUnit::PERCENT);
    swiperParameters.selectedItemWidth = parseSeleItemWOk && dimPosition > 0.0_vp ? dimPosition : defaultSize;
    bool parseSeleItemHOk =
        ParseJsDimensionVp(selectedItemHeightValue, dimPosition) && (dimPosition.Unit() != DimensionUnit::PERCENT);
    swiperParameters.selectedItemHeight = parseSeleItemHOk && dimPosition > 0.0_vp ? dimPosition : defaultSize;
    if (parseSeleItemWOk == false && parseSeleItemHOk == false && parseItemWOk == false && parseItemHOk == false) {
        SwiperModel::GetInstance()->SetIsIndicatorCustomSize(false);
    } else {
        SwiperModel::GetInstance()->SetIsIndicatorCustomSize(true);
    }
    if (maskValue->IsBoolean()) {
        auto mask = maskValue->ToBoolean();
        swiperParameters.maskValue = mask;
    }
    Color colorVal;
    parseOk = ParseJsColor(colorValue, colorVal);
    swiperParameters.colorVal = parseOk ? colorVal : swiperIndicatorTheme->GetColor();
    parseOk = ParseJsColor(selectedColorValue, colorVal);
    swiperParameters.selectedColorVal = parseOk ? colorVal : swiperIndicatorTheme->GetSelectedColor();
    return swiperParameters;
}

SwiperDigitalParameters JSSwiper::GetDigitIndicatorInfo(const JSRef<JSObject>& obj)
{
    JSRef<JSVal> dotLeftValue = obj->GetProperty("leftValue");
    JSRef<JSVal> dotTopValue = obj->GetProperty("topValue");
    JSRef<JSVal> dotRightValue = obj->GetProperty("rightValue");
    JSRef<JSVal> dotBottomValue = obj->GetProperty("bottomValue");
    JSRef<JSVal> fontColorValue = obj->GetProperty("fontColorValue");
    JSRef<JSVal> selectedFontColorValue = obj->GetProperty("selectedFontColorValue");
    JSRef<JSVal> digitFontValue = obj->GetProperty("digitFontValue");
    JSRef<JSVal> selectedDigitFontValue = obj->GetProperty("selectedDigitFontValue");
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, SwiperDigitalParameters());
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_RETURN(swiperIndicatorTheme, SwiperDigitalParameters());
    bool parseOk = false;
    SwiperDigitalParameters digitalParameters;
    CalcDimension dimPosition;
    parseOk = ParseJsDimensionVp(dotLeftValue, dimPosition);
    if (parseOk) {
        if (dimPosition.ConvertToPx() < 0.0f) {
            dimPosition = 0.0_vp;
        }
    } else {
        dimPosition = 0.0_vp;
    }
    digitalParameters.dimLeft = dimPosition;
    parseOk = ParseJsDimensionVp(dotTopValue, dimPosition);
    digitalParameters.dimTop = parseOk ? dimPosition : 0.0_vp;
    parseOk = ParseJsDimensionVp(dotRightValue, dimPosition);
    digitalParameters.dimRight = parseOk ? dimPosition : 0.0_vp;
    parseOk = ParseJsDimensionVp(dotBottomValue, dimPosition);
    digitalParameters.dimBottom = parseOk ? dimPosition : 0.0_vp;
    Color fontColor;
    parseOk = JSViewAbstract::ParseJsColor(fontColorValue, fontColor);
    digitalParameters.fontColor =
        parseOk ? fontColor : swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetTextColor();
    parseOk = JSViewAbstract::ParseJsColor(selectedFontColorValue, fontColor);
    digitalParameters.selectedFontColor =
        parseOk ? fontColor : swiperIndicatorTheme->GetDigitalIndicatorTextStyle().GetTextColor();
    if (!digitFontValue->IsNull() && digitFontValue->IsObject()) {
        GetFontContent(digitFontValue, false, digitalParameters);
    }
    if (!selectedDigitFontValue->IsNull() && selectedDigitFontValue->IsObject()) {
        GetFontContent(selectedDigitFontValue, true, digitalParameters);
    }
    return digitalParameters;
}

bool JSSwiper::GetArrowInfo(const JSRef<JSObject>& obj, SwiperArrowParameters& swiperArrowParameters)
{
    auto isShowBackgroundValue = obj->GetProperty("showBackground");
    auto isSidebarMiddleValue = obj->GetProperty("isSidebarMiddle");
    auto backgroundSizeValue = obj->GetProperty("backgroundSize");
    auto backgroundColorValue = obj->GetProperty("backgroundColor");
    auto arrowSizeValue = obj->GetProperty("arrowSize");
    auto arrowColorValue = obj->GetProperty("arrowColor");
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
    CHECK_NULL_RETURN(swiperIndicatorTheme, false);
    swiperArrowParameters.isShowBackground = isShowBackgroundValue->IsBoolean()
                                                 ? isShowBackgroundValue->ToBoolean()
                                                 : swiperIndicatorTheme->GetIsShowArrowBackground();
    swiperArrowParameters.isSidebarMiddle = isSidebarMiddleValue->IsBoolean()
                                                ? isSidebarMiddleValue->ToBoolean()
                                                : swiperIndicatorTheme->GetIsSidebarMiddle();
    bool parseOk = false;
    CalcDimension dimension;
    Color color;
    if (swiperArrowParameters.isSidebarMiddle.value()) {
        parseOk = ParseJsDimensionVp(backgroundSizeValue, dimension);
        swiperArrowParameters.backgroundSize =
            parseOk && GreatNotEqual(dimension.ConvertToVp(), 0.0) && !(dimension.Unit() == DimensionUnit::PERCENT)
                ? dimension
                : swiperIndicatorTheme->GetBigArrowBackgroundSize();
        parseOk = ParseJsColor(backgroundColorValue, color);
        swiperArrowParameters.backgroundColor = parseOk ? color : swiperIndicatorTheme->GetBigArrowBackgroundColor();
        if (swiperArrowParameters.isShowBackground.value()) {
            swiperArrowParameters.arrowSize = swiperArrowParameters.backgroundSize.value() * ARROW_SIZE_COEFFICIENT;
        } else {
            parseOk = ParseJsDimensionVpNG(arrowSizeValue, dimension);
            swiperArrowParameters.arrowSize =
                parseOk && GreatNotEqual(dimension.ConvertToVp(), 0.0) && !(dimension.Unit() == DimensionUnit::PERCENT)
                    ? dimension
                    : swiperIndicatorTheme->GetBigArrowSize();
            swiperArrowParameters.backgroundSize = swiperArrowParameters.arrowSize;
        }
        parseOk = ParseJsColor(arrowColorValue, color);
        swiperArrowParameters.arrowColor = parseOk ? color : swiperIndicatorTheme->GetBigArrowColor();
    } else {
        parseOk = ParseJsDimensionVp(backgroundSizeValue, dimension);
        swiperArrowParameters.backgroundSize =
            parseOk && GreatNotEqual(dimension.ConvertToVp(), 0.0) && !(dimension.Unit() == DimensionUnit::PERCENT)
                ? dimension
                : swiperIndicatorTheme->GetSmallArrowBackgroundSize();
        parseOk = ParseJsColor(backgroundColorValue, color);
        swiperArrowParameters.backgroundColor = parseOk ? color : swiperIndicatorTheme->GetSmallArrowBackgroundColor();
        if (swiperArrowParameters.isShowBackground.value()) {
            swiperArrowParameters.arrowSize = swiperArrowParameters.backgroundSize.value() * ARROW_SIZE_COEFFICIENT;
        } else {
            parseOk = ParseJsDimensionVpNG(arrowSizeValue, dimension);
            swiperArrowParameters.arrowSize =
                parseOk && GreatNotEqual(dimension.ConvertToVp(), 0.0) && !(dimension.Unit() == DimensionUnit::PERCENT)
                    ? dimension
                    : swiperIndicatorTheme->GetSmallArrowSize();
            swiperArrowParameters.backgroundSize = swiperArrowParameters.arrowSize;
        }
        parseOk = ParseJsColor(arrowColorValue, color);
        swiperArrowParameters.arrowColor = parseOk ? color : swiperIndicatorTheme->GetSmallArrowColor();
    }
    return true;
}

void JSSwiper::SetDisplayArrow(const JSCallbackInfo& info)
{
    if (info[0]->IsEmpty() || info[0]->IsUndefined()) {
        SwiperModel::GetInstance()->SetDisplayArrow(false);
        return;
    }
    if (info.Length() > 0 && info[0]->IsObject()) {
        auto obj = JSRef<JSObject>::Cast(info[0]);
        SwiperArrowParameters swiperArrowParameters;
        if (!GetArrowInfo(obj, swiperArrowParameters)) {
            SwiperModel::GetInstance()->SetDisplayArrow(false);
            return;
        }
        SwiperModel::GetInstance()->SetArrowStyle(swiperArrowParameters);
        SwiperModel::GetInstance()->SetDisplayArrow(true);
    } else if (info[0]->IsBoolean()) {
        if (info[0]->ToBoolean()) {
            auto pipelineContext = PipelineBase::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
            CHECK_NULL_VOID(swiperIndicatorTheme);
            SwiperArrowParameters swiperArrowParameters;
            swiperArrowParameters.isShowBackground = swiperIndicatorTheme->GetIsShowArrowBackground();
            swiperArrowParameters.isSidebarMiddle = swiperIndicatorTheme->GetIsSidebarMiddle();
            swiperArrowParameters.backgroundSize = swiperIndicatorTheme->GetSmallArrowBackgroundSize();
            swiperArrowParameters.backgroundColor = swiperIndicatorTheme->GetSmallArrowBackgroundColor();
            swiperArrowParameters.arrowSize = swiperIndicatorTheme->GetSmallArrowSize();
            swiperArrowParameters.arrowColor = swiperIndicatorTheme->GetSmallArrowColor();
            SwiperModel::GetInstance()->SetArrowStyle(swiperArrowParameters);
            SwiperModel::GetInstance()->SetDisplayArrow(true);
        } else {
            SwiperModel::GetInstance()->SetDisplayArrow(false);
            return;
        }
    } else {
        SwiperModel::GetInstance()->SetDisplayArrow(false);
        return;
    }
    if (info.Length() > 1 && info[1]->IsBoolean()) {
        SwiperModel::GetInstance()->SetHoverShow(info[1]->ToBoolean());
    } else {
        SwiperModel::GetInstance()->SetHoverShow(false);
    }
}
void JSSwiper::SetIndicator(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    if (info[0]->IsUndefined()) {
        SwiperModel::GetInstance()->SetShowIndicator(true);
        return;
    }
    auto obj = JSRef<JSObject>::Cast(info[0]);
    if (info[0]->IsObject()) {
        SwiperModel::GetInstance()->SetIndicatorIsBoolean(false);

        JSRef<JSVal> typeParam = obj->GetProperty("type");
        if (typeParam->IsString()) {
            auto type = typeParam->ToString();
            if (type == "DigitIndicator") {
                SwiperDigitalParameters digitalParameters = GetDigitIndicatorInfo(obj);
                SwiperModel::GetInstance()->SetDigitIndicatorStyle(digitalParameters);
                SwiperModel::GetInstance()->SetIndicatorType(SwiperIndicatorType::DIGIT);
            } else {
                SwiperParameters swiperParameters = GetDotIndicatorInfo(obj);
                SwiperModel::GetInstance()->SetDotIndicatorStyle(swiperParameters);
                SwiperModel::GetInstance()->SetIndicatorType(SwiperIndicatorType::DOT);
            }
        } else {
            SwiperParameters swiperParameters = GetDotIndicatorInfo(obj);
            SwiperModel::GetInstance()->SetDotIndicatorStyle(swiperParameters);
            SwiperModel::GetInstance()->SetIndicatorType(SwiperIndicatorType::DOT);
        }
    } else {
        SwiperParameters swiperParameters = GetDotIndicatorInfo(obj);
        SwiperModel::GetInstance()->SetDotIndicatorStyle(swiperParameters);
        SwiperModel::GetInstance()->SetIndicatorType(SwiperIndicatorType::DOT);
    }
    if (info[0]->IsBoolean()) {
        bool showIndicator = false;
        ParseJsBool(obj, showIndicator);
        SwiperModel::GetInstance()->SetShowIndicator(showIndicator);
    } else {
        SwiperModel::GetInstance()->SetShowIndicator(true);
    }
}

void JSSwiper::SetIndicatorStyle(const JSCallbackInfo& info)
{
    SwiperParameters swiperParameters;
    if (info[0]->IsObject()) {
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
        JSRef<JSVal> leftValue = obj->GetProperty("left");
        JSRef<JSVal> topValue = obj->GetProperty("top");
        JSRef<JSVal> rightValue = obj->GetProperty("right");
        JSRef<JSVal> bottomValue = obj->GetProperty("bottom");
        JSRef<JSVal> sizeValue = obj->GetProperty("size");
        JSRef<JSVal> maskValue = obj->GetProperty("mask");
        JSRef<JSVal> colorValue = obj->GetProperty("color");
        JSRef<JSVal> selectedColorValue = obj->GetProperty("selectedColor");
        auto pipelineContext = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto swiperIndicatorTheme = pipelineContext->GetTheme<SwiperIndicatorTheme>();
        CHECK_NULL_VOID(swiperIndicatorTheme);
        CalcDimension dimPosition;
        bool parseOk = ParseJsDimensionVp(leftValue, dimPosition);
        swiperParameters.dimLeft = parseOk ? dimPosition : 0.0_vp;
        parseOk = ParseJsDimensionVp(topValue, dimPosition);
        swiperParameters.dimTop = parseOk ? dimPosition : 0.0_vp;
        parseOk = ParseJsDimensionVp(rightValue, dimPosition);
        swiperParameters.dimRight = parseOk ? dimPosition : 0.0_vp;
        parseOk = ParseJsDimensionVp(bottomValue, dimPosition);
        swiperParameters.dimBottom = parseOk ? dimPosition : 0.0_vp;
        parseOk = ParseJsDimensionVp(sizeValue, dimPosition) && (dimPosition.Unit() != DimensionUnit::PERCENT);
        SetIsIndicatorCustomSize(dimPosition, parseOk);
        swiperParameters.itemWidth = parseOk && dimPosition > 0.0_vp ? dimPosition : swiperIndicatorTheme->GetSize();
        swiperParameters.itemHeight = parseOk && dimPosition > 0.0_vp ? dimPosition : swiperIndicatorTheme->GetSize();
        swiperParameters.selectedItemWidth =
            parseOk && dimPosition > 0.0_vp ? dimPosition : swiperIndicatorTheme->GetSize();
        swiperParameters.selectedItemHeight =
            parseOk && dimPosition > 0.0_vp ? dimPosition : swiperIndicatorTheme->GetSize();
        if (maskValue->IsBoolean()) {
            auto mask = maskValue->ToBoolean();
            swiperParameters.maskValue = mask;
        }
        Color colorVal;
        parseOk = ParseJsColor(colorValue, colorVal);
        swiperParameters.colorVal = parseOk ? colorVal : swiperIndicatorTheme->GetColor();
        parseOk = ParseJsColor(selectedColorValue, colorVal);
        swiperParameters.selectedColorVal = parseOk ? colorVal : swiperIndicatorTheme->GetSelectedColor();
    }
    SwiperModel::GetInstance()->SetDotIndicatorStyle(swiperParameters);
    info.ReturnSelf();
}

void JSSwiper::SetItemSpace(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value) || LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    SwiperModel::GetInstance()->SetItemSpace(value);
}

void JSSwiper::SetPreviousMargin(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value) || info[0]->IsNull() || info[0]->IsUndefined() ||
        LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    SwiperModel::GetInstance()->SetPreviousMargin(value);
}

void JSSwiper::SetNextMargin(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value) || info[0]->IsNull() || info[0]->IsUndefined() ||
        LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }
    SwiperModel::GetInstance()->SetNextMargin(value);
}

void JSSwiper::SetDisplayMode(int32_t index)
{
    if (index < 0 || index >= static_cast<int32_t>(DISPLAY_MODE.size())) {
        return;
    }

    SwiperModel::GetInstance()->SetDisplayMode(DISPLAY_MODE[index]);
}

void JSSwiper::SetCachedCount(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    int32_t cachedCount = DEFAULT_CACHED_COUNT;
    if (!info[0]->IsUndefined() && info[0]->IsNumber()) {
        cachedCount = info[0]->ToNumber<int32_t>();
        if (cachedCount < 0) {
            cachedCount = DEFAULT_CACHED_COUNT;
        }
    }
    SwiperModel::GetInstance()->SetCachedCount(cachedCount);
}

void JSSwiper::SetCurve(const JSCallbackInfo& info)
{
    RefPtr<Curve> curve = Curves::LINEAR;
    if (info[0]->IsString()) {
        curve = CreateCurve(info[0]->ToString());
    } else if (info[0]->IsObject()) {
        auto object = JSRef<JSObject>::Cast(info[0]);
        std::function<float(float)> customCallBack = nullptr;
        JSRef<JSVal> onCallBack = object->GetProperty("__curveCustomFunc");
        if (onCallBack->IsFunction()) {
            RefPtr<JsFunction> jsFuncCallBack =
                AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onCallBack));
            customCallBack = [func = std::move(jsFuncCallBack), id = Container::CurrentId()](float time) -> float {
                ContainerScope scope(id);
                JSRef<JSVal> params[1];
                params[0] = JSRef<JSVal>::Make(ToJSValue(time));
                auto result = func->ExecuteJS(1, params);
                auto resultValue = result->IsNumber() ? result->ToNumber<float>() : 1.0f;
                return resultValue;
            };
        }
        auto jsCurveString = object->GetProperty("__curveString");
        if (jsCurveString->IsString()) {
            auto aniTimFunc = jsCurveString->ToString();
            if (aniTimFunc == DOM_ANIMATION_TIMING_FUNCTION_CUSTOM && customCallBack) {
                curve = CreateCurve(customCallBack);
            } else if (aniTimFunc != DOM_ANIMATION_TIMING_FUNCTION_CUSTOM) {
                curve = CreateCurve(aniTimFunc);
            }
        }
    }
    SwiperModel::GetInstance()->SetCurve(curve);
}

void JSSwiper::SetOnChange(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto changeHandler = AceType::MakeRefPtr<JsEventFunction<SwiperChangeEvent, 1>>(
        JSRef<JSFunc>::Cast(info[0]), SwiperChangeEventToJSValue);
    auto onChange = [executionContext = info.GetExecutionContext(), func = std::move(changeHandler)](
                        const BaseEventInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(executionContext);
        const auto* swiperInfo = TypeInfoHelper::DynamicCast<SwiperChangeEvent>(info);
        if (!swiperInfo) {
            TAG_LOGW(AceLogTag::ACE_SWIPER, "Swiper onChange callback execute failed.");
            return;
        }
        ACE_SCORING_EVENT("Swiper.OnChange");
        func->Execute(*swiperInfo);
    };

    SwiperModel::GetInstance()->SetOnChange(std::move(onChange));
}

void JSSwiper::SetOnAnimationStart(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        auto animationStartHandler = AceType::MakeRefPtr<JsSwiperFunction>(JSRef<JSFunc>::Cast(info[0]));
        auto onAnimationStart = [executionContext = info.GetExecutionContext(),
                                    func = std::move(animationStartHandler)](
                                    int32_t index, int32_t targetIndex, const AnimationCallbackInfo& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(executionContext);
            ACE_SCORING_EVENT("Swiper.onAnimationStart");
            func->Execute(index, targetIndex, info);
        };

        SwiperModel::GetInstance()->SetOnAnimationStart(std::move(onAnimationStart));
        return;
    }

    auto animationStartHandler = AceType::MakeRefPtr<JsEventFunction<SwiperChangeEvent, 1>>(
        JSRef<JSFunc>::Cast(info[0]), SwiperChangeEventToJSValue);
    auto onAnimationStart = [executionContext = info.GetExecutionContext(), func = std::move(animationStartHandler)](
                                const BaseEventInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(executionContext);
        const auto* swiperInfo = TypeInfoHelper::DynamicCast<SwiperChangeEvent>(info);
        if (!swiperInfo) {
            TAG_LOGW(AceLogTag::ACE_SWIPER, "Swiper onAnimationStart callback execute failed.");
            return;
        }
        ACE_SCORING_EVENT("Swiper.onAnimationStart");
        func->Execute(*swiperInfo);
    };

    SwiperModel::GetInstance()->SetOnAnimationStart(std::move(onAnimationStart));
}

void JSSwiper::SetOnAnimationEnd(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        auto animationEndHandler = AceType::MakeRefPtr<JsSwiperFunction>(JSRef<JSFunc>::Cast(info[0]));
        auto onAnimationEnd = [executionContext = info.GetExecutionContext(), func = std::move(animationEndHandler)](
                                  int32_t index, const AnimationCallbackInfo& info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(executionContext);
            ACE_SCORING_EVENT("Swiper.onAnimationEnd");
            func->Execute(index, info);
        };

        SwiperModel::GetInstance()->SetOnAnimationEnd(std::move(onAnimationEnd));
        return;
    }

    auto animationEndHandler = AceType::MakeRefPtr<JsEventFunction<SwiperChangeEvent, 1>>(
        JSRef<JSFunc>::Cast(info[0]), SwiperChangeEventToJSValue);
    auto onAnimationEnd = [executionContext = info.GetExecutionContext(), func = std::move(animationEndHandler)](
                              const BaseEventInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(executionContext);
        const auto* swiperInfo = TypeInfoHelper::DynamicCast<SwiperChangeEvent>(info);
        if (!swiperInfo) {
            TAG_LOGW(AceLogTag::ACE_SWIPER, "Swiper onAnimationEnd callback execute failed.");
            return;
        }
        ACE_SCORING_EVENT("Swiper.onAnimationEnd");
        func->Execute(*swiperInfo);
    };

    SwiperModel::GetInstance()->SetOnAnimationEnd(std::move(onAnimationEnd));
}

void JSSwiper::SetOnGestureSwipe(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto gestureSwipeHandler = AceType::MakeRefPtr<JsSwiperFunction>(JSRef<JSFunc>::Cast(info[0]));
    auto onGestureSwipe = [executionContext = info.GetExecutionContext(), func = std::move(gestureSwipeHandler)](
                              int32_t index, const AnimationCallbackInfo& info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(executionContext);
        ACE_SCORING_EVENT("Swiper.onGestureSwipe");
        func->Execute(index, info);
    };

    SwiperModel::GetInstance()->SetOnGestureSwipe(std::move(onGestureSwipe));
}

void JSSwiper::SetOnClick(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSInteractableView::JsOnClick(info);
        return;
    }

    if (!info[0]->IsFunction()) {
        return;
    }

    RefPtr<JsClickFunction> jsOnClickFunc = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(info[0]));
    auto onClick = [execCtx = info.GetExecutionContext(), func = std::move(jsOnClickFunc)](
                       const BaseEventInfo* info, const RefPtr<V2::InspectorFunctionImpl>& impl) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        const auto* clickInfo = TypeInfoHelper::DynamicCast<ClickInfo>(info);
        auto newInfo = *clickInfo;
        if (impl) {
            impl->UpdateEventInfo(newInfo);
        }
        ACE_SCORING_EVENT("onClick");
        func->Execute(newInfo);
    };

    SwiperModel::GetInstance()->SetOnClick(onClick);
}

void JSSwiper::SetWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    SetWidth(info[0]);
}

void JSSwiper::SetWidth(const JSRef<JSVal>& jsValue)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsWidth(jsValue);
        return;
    }

    JSViewAbstract::JsWidth(jsValue);
    SwiperModel::GetInstance()->SetMainSwiperSizeWidth();
}

void JSSwiper::SetHeight(const JSRef<JSVal>& jsValue)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsHeight(jsValue);
        return;
    }

    JSViewAbstract::JsHeight(jsValue);
    SwiperModel::GetInstance()->SetMainSwiperSizeHeight();
}

void JSSwiper::SetHeight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    SetHeight(info[0]);
}

void JSSwiper::SetSize(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }

    if (!info[0]->IsObject()) {
        return;
    }

    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
    SetWidth(sizeObj->GetProperty("width"));
    SetHeight(sizeObj->GetProperty("height"));
}

void JSSwiperController::JSBind(BindingTarget globalObj)
{
    JSClass<JSSwiperController>::Declare("SwiperController");
    JSClass<JSSwiperController>::CustomMethod("swipeTo", &JSSwiperController::SwipeTo);
    JSClass<JSSwiperController>::CustomMethod("showNext", &JSSwiperController::ShowNext);
    JSClass<JSSwiperController>::CustomMethod("showPrevious", &JSSwiperController::ShowPrevious);
    JSClass<JSSwiperController>::CustomMethod("finishAnimation", &JSSwiperController::FinishAnimation);
    JSClass<JSSwiperController>::Bind(globalObj, JSSwiperController::Constructor, JSSwiperController::Destructor);
}

void JSSwiperController::Constructor(const JSCallbackInfo& args)
{
    auto scroller = Referenced::MakeRefPtr<JSSwiperController>();
    scroller->IncRefCount();
    args.SetReturnValue(Referenced::RawPtr(scroller));
}

void JSSwiperController::Destructor(JSSwiperController* scroller)
{
    if (scroller != nullptr) {
        scroller->DecRefCount();
    }
}

void JSSwiperController::FinishAnimation(const JSCallbackInfo& args)
{
    if (!controller_) {
        return;
    }

    if (args.Length() > 0 && args[0]->IsFunction()) {
        RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(args[0]));
        auto onFinish = [execCtx = args.GetExecutionContext(), func = std::move(jsFunc)]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("Swiper.finishAnimation");
            TAG_LOGD(AceLogTag::ACE_SWIPER, "SwiperController finishAnimation callback execute.");
            func->Execute();
        };

        controller_->SetFinishCallback(onFinish);
        controller_->FinishAnimation();
        return;
    }

    controller_->FinishAnimation();
}

void JSSwiper::SetNestedScroll(const JSCallbackInfo& args)
{
    // default value
    NestedScrollOptions nestedOpt = {
        .forward = NestedScrollMode::SELF_ONLY,
        .backward = NestedScrollMode::SELF_ONLY,
    };
    if (args.Length() < 1 || !args[0]->IsNumber()) {
        SwiperModel::GetInstance()->SetNestedScroll(nestedOpt);
        return;
    }
    int32_t value = -1;
    JSViewAbstract::ParseJsInt32(args[0], value);
    auto mode = static_cast<NestedScrollMode>(value);
    if (mode < NestedScrollMode::SELF_ONLY || mode > NestedScrollMode::SELF_FIRST) {
        SwiperModel::GetInstance()->SetNestedScroll(nestedOpt);
        return;
    }
    nestedOpt.forward = mode;
    nestedOpt.backward = mode;
    SwiperModel::GetInstance()->SetNestedScroll(nestedOpt);
    args.ReturnSelf();
}
} // namespace OHOS::Ace::Framework
