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

#include "js_ui_observer.h"
#include "ui_observer.h"
#include "ui_observer_listener.h"

#include <map>
#include <string>

#include "interfaces/napi/kits/utils/napi_utils.h"
#include "js_native_api.h"
#include "js_native_api_types.h"

#include "core/components_ng/base/observer_handler.h"

namespace OHOS::Ace::Napi {
namespace {
#define GET_PARAMS(env, info, max) \
    size_t argc = max;             \
    napi_value argv[max] = { 0 };  \
    napi_value thisVar = nullptr;  \
    void* data;                    \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

bool PargeStringFromNapi(napi_env env, napi_value val, std::string& str)
{
    if (!val || !MatchValueType(env, val, napi_string)) {
        return false;
    }
    size_t len = 0;
    napi_get_value_string_utf8(env, val, nullptr, 0, &len);
    std::unique_ptr<char[]> result = std::make_unique<char[]>(len + 1);
    napi_get_value_string_utf8(env, val, result.get(), len + 1, &len);
    str = result.get();
    return true;
}

bool ParseNavigationId(napi_env env, napi_value obj, std::string& navigationStr)
{
    napi_value navigationId = nullptr;
    napi_get_named_property(env, obj, "navigationId", &navigationId);
    if (!MatchValueType(env, navigationId, napi_string)) {
        return false;
    }
    return PargeStringFromNapi(env, navigationId, navigationStr);
}
} // namespace

ObserverProcess::ObserverProcess()
{
    registerProcess_ = {
        { NAVDESTINATION_UPDATE, &ObserverProcess::ProcessNavigationRegister },
    };
    unregisterProcess_ = {
        { NAVDESTINATION_UPDATE, &ObserverProcess::ProcessNavigationUnRegister },
    };
}

ObserverProcess& ObserverProcess::GetInstance()
{
    static ObserverProcess instance;
    return instance;
}

napi_value ObserverProcess::ProcessRegister(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);
    NAPI_ASSERT(env, (argc >= 2 && thisVar != nullptr), "Invalid arguments");
    std::string type;
    napi_value result = nullptr;
    if (!PargeStringFromNapi(env, argv[0], type)) {
        return result;
    }
    return (this->*registerProcess_[type])(env, info);
}

napi_value ObserverProcess::ProcessUnRegister(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);
    NAPI_ASSERT(env, (argc >= 1 && thisVar != nullptr), "Invalid arguments");
    std::string type;
    if (!PargeStringFromNapi(env, argv[0], type)) {
        napi_value result = nullptr;
        return result;
    }
    return (this->*unregisterProcess_[type])(env, info);
}

napi_value ObserverProcess::ProcessNavigationRegister(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);

    if (argc == 2 && MatchValueType(env, argv[1], napi_function)) {
        auto listener = std::make_shared<UIObserverListener>(env, argv[1]);
        UIObserver::RegisterNavigationCallback(listener);
    }

    if (argc == 3 && MatchValueType(env, argv[1], napi_object) && MatchValueType(env, argv[2], napi_function)) {
        std::string id;
        if (ParseNavigationId(env, argv[1], id)) {
            auto listener = std::make_shared<UIObserverListener>(env, argv[2]);
            UIObserver::RegisterNavigationCallback(id, listener);
        }
    }

    napi_value result = nullptr;
    return result;
}

napi_value ObserverProcess::ProcessNavigationUnRegister(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);

    if (argc == 1) {
        UIObserver::UnRegisterNavigationCallback(nullptr);
    }

    if (argc == 2 && MatchValueType(env, argv[1], napi_function)) {
        UIObserver::UnRegisterNavigationCallback(argv[1]);
    }

    if (argc == 2 && MatchValueType(env, argv[1], napi_object)) {
        std::string id;
        if (ParseNavigationId(env, argv[1], id)) {
            UIObserver::UnRegisterNavigationCallback(id, nullptr);
        }
    }

    if (argc == 3 && MatchValueType(env, argv[1], napi_object) && MatchValueType(env, argv[2], napi_function)) {
        std::string id;
        if (ParseNavigationId(env, argv[1], id)) {
            UIObserver::UnRegisterNavigationCallback(id, argv[2]);
        }
    }

    napi_value result = nullptr;
    return result;
}

napi_value ObserverOn(napi_env env, napi_callback_info info)
{
    return ObserverProcess::GetInstance().ProcessRegister(env, info);
}

napi_value ObserverOff(napi_env env, napi_callback_info info)
{
    return ObserverProcess::GetInstance().ProcessUnRegister(env, info);
}

static napi_value UIObserverExport(napi_env env, napi_value exports)
{
    NG::UIObserverHandler::GetInstance().SetHandleNavigationChangeFunc(&UIObserver::HandleNavigationStateChange);
    napi_property_descriptor uiObserverDesc[] = {
        DECLARE_NAPI_FUNCTION("on", ObserverOn),
        DECLARE_NAPI_FUNCTION("off", ObserverOff),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(uiObserverDesc) / sizeof(uiObserverDesc[0]), uiObserverDesc));
    return exports;
}

static napi_module uiObserverModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = UIObserverExport,
    .nm_modname = "arkui.observer",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void ObserverRegister()
{
    napi_module_register(&uiObserverModule);
}
} // namespace OHOS::Ace::Napi
