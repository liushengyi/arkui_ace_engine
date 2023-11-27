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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_TEXTAREA_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_TEXTAREA_H

#include "base/memory/referenced.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_function.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "frameworks/core/components/text_field/text_field_component.h"
#include "frameworks/core/components/text_field/text_field_controller.h"
#include "frameworks/core/components/text_field/textfield_theme.h"

namespace OHOS::Ace::Framework {

class JSTextArea : public JSViewAbstract {
public:
    static void JSBind(BindingTarget globalObj);
    static void Create(const JSCallbackInfo& info);
};

class JSTextAreaController final : public Referenced {
public:
    JSTextAreaController() = default;
    ~JSTextAreaController() override = default;

    static void JSBind(BindingTarget globalObj);
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSTextAreaController* scroller);
    void CaretPosition(int32_t caretPosition);
    void SetTextSelection(int32_t selectionStart, int32_t selectionEnd);
    void StopEditing();
    void SetController(const RefPtr<TextFieldControllerBase>& controller)
    {
        controllerWeak_ = controller;
    }

    void GetTextContentRect(const JSCallbackInfo& info);
    void GetTextContentLinesNum(const JSCallbackInfo& info);
    void GetCaretOffset(const JSCallbackInfo& info);
private:
    JSRef<JSObject> CreateRectangle(const Rect& info);

    WeakPtr<TextFieldControllerBase> controllerWeak_;
    ACE_DISALLOW_COPY_AND_MOVE(JSTextAreaController);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_TEXTAREA_H
