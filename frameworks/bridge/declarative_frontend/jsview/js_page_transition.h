/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_PAGE_TRANSITION_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_PAGE_TRANSITION_H

#include "frameworks/base/json/json_util.h"
#include "frameworks/base/memory/ace_type.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "frameworks/core/components/page_transition/page_transition_info.h"

namespace OHOS::Ace::Framework {

class JSPageTransition : public AceType {
    DECLARE_ACE_TYPE(JSPageTransition, AceType);

public:
    JSPageTransition() = default;
    ~JSPageTransition() override = default;

    static void JSBind(BindingTarget globalObj);

    static void Create(const JSCallbackInfo& info);
    static void Pop();

    static void Slide(const JSCallbackInfo& info);
    static void Translate(const JSCallbackInfo& info);
    static void Scale(const JSCallbackInfo& info);
    static void Opacity(const JSCallbackInfo& info);

    static void JsHandlerOnEnter(const JSCallbackInfo& info);
    static void JsHandlerOnExit(const JSCallbackInfo& info);

protected:
    static PageTransitionOption ParseTransitionOption(const JSRef<JSVal>& transitionArgs);
};

class JSPageTransitionEnter final : public JSPageTransition {
    DECLARE_ACE_TYPE(JSPageTransitionEnter, JSPageTransition);

public:
    static void Create(const JSCallbackInfo& info);
    // for partial update
    static void Pop() {};
};

class JSPageTransitionExit final : public JSPageTransition {
    DECLARE_ACE_TYPE(JSPageTransitionExit, JSPageTransition);

public:
    static void Create(const JSCallbackInfo& info);
    static void Pop() {};
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_PAGE_TRANSITION_H
