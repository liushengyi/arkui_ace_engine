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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_MEASURE_LAYOUT_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_MEASURE_LAYOUT_H

#include <utility>

#include "base/memory/ace_type.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_param.h"
#include "frameworks/bridge/declarative_frontend/engine/js_ref_ptr.h"

namespace OHOS::Ace::Framework {

class JSMeasureLayoutParam : public NG::MeasureLayoutParam {
    DECLARE_ACE_TYPE(JSMeasureLayoutParam, NG::MeasureLayoutParam);
public:
    JSMeasureLayoutParam(NG::LayoutWrapper* layoutWrapper);
    JSRef<JSObject> GetConstraint();
    JSRef<JSArray> GetChildArray()
    {
        return childArray_;
    }
    void Update(NG::LayoutWrapper* layoutWrapper) override;
    static RefPtr<JSMeasureLayoutParam> GetInstance(NG::LayoutWrapper* layoutWrapper);
private:
    void GenChildArray(int32_t start, int32_t end);
    JSRef<JSArray> childArray_;
};

class JSMeasureLayoutParamNG : public NG::MeasureLayoutParam {
    DECLARE_ACE_TYPE(JSMeasureLayoutParamNG, NG::MeasureLayoutParam);
public:
    JSMeasureLayoutParamNG(NG::LayoutWrapper* layoutWrapper);
    JSRef<JSObject> GetConstraint();
    JSRef<JSArray> GetChildArray()
    {
        return childArray_;
    }
    JSRef<JSObject> GetSelfLayoutInfo();
    void UpdateSize(int32_t index, const NG::SizeF& size) override;
    void Update(NG::LayoutWrapper* layoutWrapper) override;
    static RefPtr<JSMeasureLayoutParamNG> GetInstance(NG::LayoutWrapper* layoutWrapper);
private:
    void GenChildArray(int32_t start, int32_t end);
    JSRef<JSArray> childArray_;
};

class ACE_EXPORT ViewMeasureLayout : AceType {
    DECLARE_ACE_TYPE(ViewMeasureLayout, AceType);

public:
    ViewMeasureLayout() = default;
    ~ViewMeasureLayout() override
    {
        LOGD("Destroy: ViewMeasureLayout");
    }

#ifdef USE_ARK_ENGINE
    static panda::Local<panda::JSValueRef> JSMeasure(panda::JsiRuntimeCallInfo* info);
    static panda::Local<panda::JSValueRef> JSLayout(panda::JsiRuntimeCallInfo* info);
    static panda::Local<panda::JSValueRef> JSPlaceChildren(panda::JsiRuntimeCallInfo* info);
#endif
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_MEASURE_LAYOUT_H