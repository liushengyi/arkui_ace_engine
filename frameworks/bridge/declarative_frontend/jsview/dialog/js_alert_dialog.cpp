/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "frameworks/bridge/declarative_frontend/jsview/dialog/js_alert_dialog.h"

#include <sstream>
#include <string>
#include <vector>

#include "base/log/ace_scoring_log.h"
#include "bridge/declarative_frontend/jsview/models/alert_dialog_model_impl.h"
#include "core/common/container.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/dialog/alert_dialog_model_ng.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_function.h"

namespace OHOS::Ace {
std::unique_ptr<AlertDialogModel> AlertDialogModel::instance_ = nullptr;
std::mutex AlertDialogModel::mutex_;
AlertDialogModel* AlertDialogModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::AlertDialogModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::AlertDialogModelNG());
            } else {
                instance_.reset(new Framework::AlertDialogModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {
namespace {
const std::vector<DialogAlignment> DIALOG_ALIGNMENT = { DialogAlignment::TOP, DialogAlignment::CENTER,
    DialogAlignment::BOTTOM, DialogAlignment::DEFAULT, DialogAlignment::TOP_START, DialogAlignment::TOP_END,
    DialogAlignment::CENTER_START, DialogAlignment::CENTER_END, DialogAlignment::BOTTOM_START,
    DialogAlignment::BOTTOM_END };
const std::vector<DialogButtonDirection> DIALOG_BUTTONS_DIRECTION = { DialogButtonDirection::AUTO,
    DialogButtonDirection::HORIZONTAL, DialogButtonDirection::VERTICAL };
} // namespace

void SetParseStyle(ButtonInfo& buttonInfo, const int32_t styleValue)
{
    if (styleValue >= static_cast<int32_t>(DialogButtonStyle::DEFAULT) &&
        styleValue <= static_cast<int32_t>(DialogButtonStyle::HIGHTLIGHT)) {
        buttonInfo.dlgButtonStyle = static_cast<DialogButtonStyle>(styleValue);
    }
}

void ParseButtonObj(
    const JSCallbackInfo& args, DialogProperties& properties, JSRef<JSVal> jsVal, const std::string& property)
{
    if (!jsVal->IsObject()) {
        return;
    }
    auto objInner = JSRef<JSObject>::Cast(jsVal);
    auto value = objInner->GetProperty("value");
    std::string buttonValue;
    ButtonInfo buttonInfo;
    if (JSAlertDialog::ParseJsString(value, buttonValue)) {
        buttonInfo.text = buttonValue;
    }

    // Parse enabled
    auto enabledValue = objInner->GetProperty("enabled");
    if (enabledValue->IsBoolean()) {
        buttonInfo.enabled = enabledValue->ToBoolean();
    }

    // Parse defaultFocus
    auto defaultFocusValue = objInner->GetProperty("defaultFocus");
    if (defaultFocusValue->IsBoolean()) {
        buttonInfo.defaultFocus = defaultFocusValue->ToBoolean();
    }

    // Parse style
    auto style = objInner->GetProperty("style");
    if (style->IsNumber()) {
        auto styleValue = style->ToNumber<int32_t>();
        SetParseStyle(buttonInfo, styleValue);
    }

    auto fontColorValue = objInner->GetProperty("fontColor");
    Color textColor;
    if (JSAlertDialog::ParseJsColor(fontColorValue, textColor)) {
        buttonInfo.textColor = textColor.ColorToString();
    }

    auto backgroundColorValue = objInner->GetProperty("backgroundColor");
    Color backgroundColor;
    if (JSAlertDialog::ParseJsColor(backgroundColorValue, backgroundColor)) {
        buttonInfo.isBgColorSetted = true;
        buttonInfo.bgColor = backgroundColor;
    }

    auto actionValue = objInner->GetProperty("action");
    if (actionValue->IsFunction()) {
        WeakPtr<NG::FrameNode> frameNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
        auto actionFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(actionValue));
        auto eventFunc = [execCtx = args.GetExecutionContext(), func = std::move(actionFunc), property,
                            node = frameNode]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("AlertDialog.[" + property + "].onAction");
            auto pipelineContext = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->UpdateCurrentActiveNode(node);
            func->Execute();
        };
        AlertDialogModel::GetInstance()->SetParseButtonObj(eventFunc, buttonInfo, properties, property);
    }

    if (buttonInfo.IsValid()) {
        properties.buttons.emplace_back(buttonInfo);
    }
}

void ParseButtonArray(
    const JSCallbackInfo& args, DialogProperties& properties, JSRef<JSObject> obj, const std::string& property)
{
    auto jsVal = obj->GetProperty(property.c_str());
    if (!jsVal->IsArray()) {
        return;
    }
    JSRef<JSArray> array = JSRef<JSArray>::Cast(jsVal);
    size_t length = array->Length();
    if (length <= 0) {
        return;
    }
    for (size_t i = 0; i < length; i++) {
        JSRef<JSVal> buttonItem = array->GetValueAt(i);
        if (!buttonItem->IsObject()) {
            break;
        }
        ParseButtonObj(args, properties, buttonItem, property + std::to_string(i));
    }
}

void JSAlertDialog::Show(const JSCallbackInfo& args)
{
    auto scopedDelegate = EngineHelper::GetCurrentDelegate();
    if (!scopedDelegate) {
        // this case usually means there is no foreground container, need to figure out the reason.
        LOGE("scopedDelegate is null, please check");
        return;
    }

    DialogProperties properties { .type = DialogType::ALERT_DIALOG };
    if (args[0]->IsObject()) {
        auto obj = JSRef<JSObject>::Cast(args[0]);

        // Parse title.
        auto titleValue = obj->GetProperty("title");
        std::string title;
        if (ParseJsString(titleValue, title)) {
            properties.title = title;
        }

        // Parse subtitle.
        auto subtitleValue = obj->GetProperty("subtitle");
        std::string subtitle;
        if (ParseJsString(subtitleValue, subtitle)) {
            properties.subtitle = subtitle;
        }

        // Parses message.
        auto messageValue = obj->GetProperty("message");
        std::string message;
        if (ParseJsString(messageValue, message)) {
            properties.content = message;
        }

        // Parses gridCount.
        auto gridCountValue = obj->GetProperty("gridCount");
        if (gridCountValue->IsNumber()) {
            properties.gridCount = gridCountValue->ToNumber<int32_t>();
        }

        // Parse auto autoCancel.
        auto autoCancelValue = obj->GetProperty("autoCancel");
        if (autoCancelValue->IsBoolean()) {
            properties.autoCancel = autoCancelValue->ToBoolean();
        }

        // Parse cancel.
        auto cancelValue = obj->GetProperty("cancel");
        if (cancelValue->IsFunction()) {
            WeakPtr<NG::FrameNode> frameNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
            auto cancelFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(cancelValue));
            auto eventFunc = [execCtx = args.GetExecutionContext(), func = std::move(cancelFunc), node = frameNode]() {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                ACE_SCORING_EVENT("AlertDialog.property.cancel");
                auto pipelineContext = PipelineContext::GetCurrentContext();
                CHECK_NULL_VOID(pipelineContext);
                pipelineContext->UpdateCurrentActiveNode(node);
                func->Execute();
            };
            AlertDialogModel::GetInstance()->SetOnCancel(eventFunc, properties);
        }

        if (obj->GetProperty("confirm")->IsObject()) {
            // Parse confirm.
            auto objInner = obj->GetProperty("confirm");
            ParseButtonObj(args, properties, objInner, "confirm");
        } else if (obj->GetProperty("buttons")->IsArray()) {
            // Parse buttons array.
            ParseButtonArray(args, properties, obj, "buttons");
        } else {
            // Parse primaryButton and secondaryButton.
            auto objInner = obj->GetProperty("primaryButton");
            ParseButtonObj(args, properties, objInner, "primaryButton");
            objInner = obj->GetProperty("secondaryButton");
            ParseButtonObj(args, properties, objInner, "secondaryButton");
        }

        // Parse buttons direction.
        auto directionValue = obj->GetProperty("buttonDirection");
        if (directionValue->IsNumber()) {
            auto buttonDirection = directionValue->ToNumber<int32_t>();
            if (buttonDirection >= 0 && buttonDirection <= static_cast<int32_t>(DIALOG_BUTTONS_DIRECTION.size())) {
                properties.buttonDirection = DIALOG_BUTTONS_DIRECTION[buttonDirection];
            }
        }

        // Parse alignment
        auto alignmentValue = obj->GetProperty("alignment");
        if (alignmentValue->IsNumber()) {
            auto alignment = alignmentValue->ToNumber<int32_t>();
            if (alignment >= 0 && alignment <= static_cast<int32_t>(DIALOG_ALIGNMENT.size())) {
                properties.alignment = DIALOG_ALIGNMENT[alignment];
            }
        }

        // Parse offset
        auto offsetValue = obj->GetProperty("offset");
        if (offsetValue->IsObject()) {
            auto offsetObj = JSRef<JSObject>::Cast(offsetValue);
            CalcDimension dx;
            auto dxValue = offsetObj->GetProperty("dx");
            ParseJsDimensionVp(dxValue, dx);
            CalcDimension dy;
            auto dyValue = offsetObj->GetProperty("dy");
            ParseJsDimensionVp(dyValue, dy);
            properties.offset = DimensionOffset(dx, dy);
        }

        // Parse maskRect.
        auto maskRectValue = obj->GetProperty("maskRect");
        DimensionRect maskRect;
        if (JSViewAbstract::ParseJsDimensionRect(maskRectValue, maskRect)) {
            properties.maskRect = maskRect;
        }

        // Parse showInSubWindowValue.
        auto showInSubWindowValue = obj->GetProperty("showInSubWindow");
        if (showInSubWindowValue->IsBoolean()) {
            LOGI("Parse showInSubWindowValue");
            properties.isShowInSubWindow = showInSubWindowValue->ToBoolean();
        }

        // Parse isModal.
        auto isModalValue = obj->GetProperty("isModal");
        if (isModalValue->IsBoolean()) {
            LOGI("Parse isModalValue");
            properties.isModal = isModalValue->ToBoolean();
        }
        AlertDialogModel::GetInstance()->SetShowDialog(properties);
    }
}

void JSAlertDialog::JSBind(BindingTarget globalObj)
{
    JSClass<JSAlertDialog>::Declare("AlertDialog");
    JSClass<JSAlertDialog>::StaticMethod("show", &JSAlertDialog::Show);

    JSClass<JSAlertDialog>::InheritAndBind<JSViewAbstract>(globalObj);
}

} // namespace OHOS::Ace::Framework
