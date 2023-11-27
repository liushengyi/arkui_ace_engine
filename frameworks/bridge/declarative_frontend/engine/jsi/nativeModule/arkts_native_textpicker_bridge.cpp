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
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_api.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_textpicker_bridge.h"

namespace OHOS::Ace::NG {
const std::string FORMAT_FONT = "%s|%s|%s";
constexpr int NUM_0 = 0;
constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;
constexpr int NUM_3 = 3;
constexpr int NUM_4 = 4;
constexpr int NUM_5 = 5;

bool TextpickerBridge::ParseDimensionNG(const EcmaVM* vm, const Local<JSValueRef>& jsValue, CalcDimension& result,
    DimensionUnit defaultUnit, bool isSupportPercent = true)
{
    if (jsValue->IsNumber()) {
        result = CalcDimension(jsValue->ToNumber(vm)->Value(), defaultUnit);
        return true;
    }
    if (jsValue->IsString()) {
        auto value = jsValue->ToString(vm)->ToString();
        if (value.back() == '%' && !isSupportPercent) {
            return false;
        }
        return StringUtils::StringToCalcDimensionNG(jsValue->ToString(vm)->ToString(), result, false, defaultUnit);
    }
    // resouce ignore by design
    return false;
}
bool TextpickerBridge::ParseDimensionVpNG(
    const EcmaVM* vm, const Local<JSValueRef>& jsValue, CalcDimension& result, bool isSupportPercent = true)
{
    return ParseDimensionNG(vm, jsValue, result, DimensionUnit::VP, isSupportPercent);
}

bool TextpickerBridge::ParseJsIntegerArray(Local<JSValueRef> values, EcmaVM* vm, std::vector<uint32_t>& result)
{
    Local<panda::ArrayRef> valueArray = static_cast<Local<panda::ArrayRef>>(values);
    for (size_t i = 0; i < valueArray->Length(vm); i++) {
        Local<JSValueRef> value = valueArray->GetValueAt(vm, values, i);
        if (value->IsNumber()) {
            result.emplace_back(value->Uint32Value(vm));
        } else {
            return false;
        }
    }
    return true;
}

std::string TextpickerBridge::GetStringFromJS(const EcmaVM* vm, const Local<JSValueRef>& value)
{
    if (!value->IsNull() && value->IsString()) {
        return value->ToString(vm)->ToString();
    } else {
        return std::string("-1");
    }
}

ArkUINativeModuleValue TextpickerBridge::SetBackgroundColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    uint32_t color = secondArg->Uint32Value(vm);
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerBackgroundColor(nativeNode, color);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue TextpickerBridge::ResetBackgroundColor(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().ResetTextpickerBackgroundColor(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue TextpickerBridge::SetCanLoop(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    bool canLoop = secondArg->ToBoolean(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerCanLoop(nativeNode, canLoop);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::SetSelectedIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> secondArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    uint32_t selectedValue = 0;

    if (secondArg->IsArray(vm)) {
        std::vector<uint32_t> selectedValues;
        if (!TextpickerBridge::ParseJsIntegerArray(secondArg, vm, selectedValues)) {
            selectedValues.clear();
            GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerSelectedIndex(
                nativeNode, selectedValues.data(), -1);
            return panda::JSValueRef::Undefined(vm);
        }
        if (selectedValues.size() > 0) {
            GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerSelectedIndex(
                nativeNode, selectedValues.data(), selectedValues.size());
            return panda::JSValueRef::Undefined(vm);
        } else {
            selectedValues.emplace_back(0);
        }
        GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerSelected(nativeNode, selectedValues[0]);
    } else {
        if (secondArg->IsNumber()) {
            selectedValue = secondArg->Uint32Value(vm);
        }
        GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerSelected(nativeNode, selectedValue);
    }
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue TextpickerBridge::SetTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> fontSizeArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fontWeightArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fontFamilyArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> fontStyleArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    uint32_t color = colorArg->Uint32Value(vm);
    std::string fontSize = TextpickerBridge::GetStringFromJS(vm, fontSizeArg);
    std::string weight = TextpickerBridge::GetStringFromJS(vm, fontWeightArg);
    std::string fontFamily = TextpickerBridge::GetStringFromJS(vm, fontFamilyArg);
    int32_t styleVal = 0;
    if (!fontStyleArg->IsNull()) {
        styleVal = fontStyleArg->Int32Value(vm);
    }
    std::string fontInfo = StringUtils::FormatString(
        FORMAT_FONT.c_str(), fontSize.c_str(), weight.c_str(), fontFamily.c_str());

    GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerTextStyle(
        nativeNode, color, fontInfo.c_str(), styleVal);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::SetSelectedTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> fontSizeArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fontWeightArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fontFamilyArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> fontStyleArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    uint32_t color = colorArg->Uint32Value(vm);
    std::string fontSize = TextpickerBridge::GetStringFromJS(vm, fontSizeArg);
    std::string weight = TextpickerBridge::GetStringFromJS(vm, fontWeightArg);
    std::string fontFamily = TextpickerBridge::GetStringFromJS(vm, fontFamilyArg);
    int32_t styleVal = 0;
    if (!fontStyleArg->IsNull()) {
        styleVal = fontStyleArg->Int32Value(vm);
    }
    std::string fontInfo = StringUtils::FormatString(
        FORMAT_FONT.c_str(), fontSize.c_str(), weight.c_str(), fontFamily.c_str());

    GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerSelectedTextStyle(
        nativeNode, color, fontInfo.c_str(), styleVal);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::SetDisappearTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> colorArg = runtimeCallInfo->GetCallArgRef(NUM_1);
    Local<JSValueRef> fontSizeArg = runtimeCallInfo->GetCallArgRef(NUM_2);
    Local<JSValueRef> fontWeightArg = runtimeCallInfo->GetCallArgRef(NUM_3);
    Local<JSValueRef> fontFamilyArg = runtimeCallInfo->GetCallArgRef(NUM_4);
    Local<JSValueRef> fontStyleArg = runtimeCallInfo->GetCallArgRef(NUM_5);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    
    uint32_t color = colorArg->Uint32Value(vm);
    std::string fontSize = TextpickerBridge::GetStringFromJS(vm, fontSizeArg);
    std::string weight = TextpickerBridge::GetStringFromJS(vm, fontWeightArg);
    std::string fontFamily = TextpickerBridge::GetStringFromJS(vm, fontFamilyArg);
    int32_t styleVal = 0;
    if (!fontStyleArg->IsNull()) {
        styleVal = fontStyleArg->Int32Value(vm);
    }
    std::string fontInfo = StringUtils::FormatString(
        FORMAT_FONT.c_str(), fontSize.c_str(), weight.c_str(), fontFamily.c_str());

    GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerDisappearTextStyle(
        nativeNode, color, fontInfo.c_str(), styleVal);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::SetDefaultPickerItemHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    Local<JSValueRef> jsValue = runtimeCallInfo->GetCallArgRef(NUM_1);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();

    CalcDimension height;
    if (jsValue->IsUndefined() || !ParseDimensionVpNG(vm, jsValue, height)) {
        GetArkUIInternalNodeAPI()->GetCommonModifier().ResetWidth(nativeNode);
    } else {
        if (LessNotEqual(height.Value(), 0.0)) {
            height.SetValue(0.0);
        }
        GetArkUIInternalNodeAPI()->GetTextpickerModifier().SetTextpickerDefaultPickerItemHeight(
            nativeNode, height.Value(), static_cast<int>(height.Unit()));
    }
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::ResetCanLoop(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().ResetTextpickerCanLoop(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::ResetSelectedIndex(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().ResetTextpickerSelected(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::ResetTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().ResetTextpickerTextStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::ResetSelectedTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().ResetTextpickerSelectedTextStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::ResetDisappearTextStyle(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().ResetTextpickerDisappearTextStyle(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
ArkUINativeModuleValue TextpickerBridge::ResetDefaultPickerItemHeight(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::NativePointerRef::New(vm, nullptr));
    Local<JSValueRef> firstArg = runtimeCallInfo->GetCallArgRef(NUM_0);
    void* nativeNode = firstArg->ToNativePointer(vm)->Value();
    GetArkUIInternalNodeAPI()->GetTextpickerModifier().ResetTextpickerDefaultPickerItemHeight(nativeNode);
    return panda::JSValueRef::Undefined(vm);
}
}