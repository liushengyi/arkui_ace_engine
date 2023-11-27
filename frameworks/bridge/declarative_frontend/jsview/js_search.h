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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SEARCH_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SEARCH_H

#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "frameworks/core/components/text_field/text_field_controller.h"
#include "frameworks/core/components_ng/pattern/search/search_model_ng.h"

namespace OHOS::Ace::Framework {

class JSSearch : public JSViewAbstract, public JSInteractableView {
public:
    static void JSBind(BindingTarget globalObj);
    static void Create(const JSCallbackInfo& info);

    static void SetEnableKeyboardOnFocus(const JSCallbackInfo& info);
    static void SetSearchButton(const JSCallbackInfo& info);
    static void SetSearchIcon(const JSCallbackInfo& info);
    static void SetCancelButton(const JSCallbackInfo& info);
    static void SetTextColor(const JSCallbackInfo& info);
    static void SetCaret(const JSCallbackInfo& info);
    static void SetPlaceholderColor(const JSCallbackInfo& info);
    static void SetPlaceholderFont(const JSCallbackInfo& info);
    static void SetTextFont(const JSCallbackInfo& info);
    static void SetTextAlign(int32_t value);
    static void JsBorder(const JSCallbackInfo& info);
    static void JsBorderWidth(const JSCallbackInfo& info);
    static void JsBorderColor(const JSCallbackInfo& info);
    static void JsBorderStyle(const JSCallbackInfo& info);
    static void JsBorderRadius(const JSCallbackInfo& info);
    static void OnSubmit(const JSCallbackInfo& info);
    static void OnChange(const JSCallbackInfo& info);
    static void SetOnTextSelectionChange(const JSCallbackInfo& info);
    static void SetOnScroll(const JSCallbackInfo& info);
    static void SetHeight(const JSCallbackInfo& info);
    static void SetOnCopy(const JSCallbackInfo& info);
    static void SetOnCut(const JSCallbackInfo& info);
    static void SetOnPaste(const JSCallbackInfo& info);
    static void SetCopyOption(const JSCallbackInfo& info);
    static void SetIconStyle(const JSCallbackInfo& info);
    static void JsMenuOptionsExtension(const JSCallbackInfo& info);
    static void SetSelectionMenuHidden(const JSCallbackInfo& info);
    static void SetCustomKeyboard(const JSCallbackInfo& info);
    static void SetMaxLength(const JSCallbackInfo& info);
    static void SetType(const JSCallbackInfo& info);
};

class JSSearchController final : public Referenced {
public:
    JSSearchController() = default;
    ~JSSearchController() override = default;

    static void JSBind(BindingTarget globalObj);
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSSearchController* scroller);
    void CaretPosition(int32_t caretPosition);
    void StopEditing();
    void SetController(const RefPtr<TextFieldControllerBase>& controller)
    {
        controller_ = controller;
    }

    void GetTextContentRect(const JSCallbackInfo& info);
    void GetTextContentLinesNum(const JSCallbackInfo& info);
    void GetCaretOffset(const JSCallbackInfo& info);

private:
    JSRef<JSObject> CreateRectangle(const Rect& info);

    WeakPtr<TextFieldControllerBase> controller_;
    ACE_DISALLOW_COPY_AND_MOVE(JSSearchController);
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SEARCH_H
