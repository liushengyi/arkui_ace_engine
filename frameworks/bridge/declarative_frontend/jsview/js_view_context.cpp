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

#include "bridge/declarative_frontend/jsview/js_view_context.h"

#include <functional>
#include <memory>
#include <sstream>

#include "base/log/ace_trace.h"
#include "base/log/jank_frame_report.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/engine_helper.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/jsview/models/view_context_model_impl.h"
#include "core/common/ace_engine.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/view_context/view_context_model_ng.h"

#ifdef USE_ARK_ENGINE
#include "bridge/declarative_frontend/engine/jsi/jsi_declarative_engine.h"
#endif

namespace OHOS::Ace {

std::unique_ptr<ViewContextModel> ViewContextModel::instance_ = nullptr;
std::mutex ViewContextModel::mutex_;

ViewContextModel* ViewContextModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::ViewContextModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::ViewContextModelNG());
            } else {
                instance_.reset(new Framework::ViewContextModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
namespace {

constexpr uint32_t DEFAULT_DURATION = 1000; // ms
constexpr int64_t MICROSEC_TO_MILLISEC = 1000;

void AnimateToForStageMode(const RefPtr<PipelineBase>& pipelineContext, AnimationOption& option,
    JSRef<JSFunc> jsAnimateToFunc, std::function<void()>& onFinishEvent)
{
    auto triggerId = Container::CurrentId();
    AceEngine::Get().NotifyContainers([triggerId, option](const RefPtr<Container>& container) {
        auto context = container->GetPipelineContext();
        if (!context) {
            // pa container do not have pipeline context.
            return;
        }
        if (!container->GetSettings().usingSharedRuntime) {
            return;
        }
        if (!container->IsFRSCardContainer() && !container->WindowIsShow()) {
            return;
        }
        ContainerScope scope(container->GetInstanceId());
        context->FlushBuild();
        if (context->GetInstanceId() == triggerId) {
            return;
        }
        context->PrepareOpenImplicitAnimation();
    });
    pipelineContext->OpenImplicitAnimation(option, option.GetCurve(), onFinishEvent);
    pipelineContext->SetSyncAnimationOption(option);
    // Execute the function.
    jsAnimateToFunc->Call(jsAnimateToFunc);
    AceEngine::Get().NotifyContainers([triggerId](const RefPtr<Container>& container) {
        auto context = container->GetPipelineContext();
        if (!context) {
            // pa container do not have pipeline context.
            return;
        }
        if (!container->GetSettings().usingSharedRuntime) {
            return;
        }
        if (!container->IsFRSCardContainer() && !container->WindowIsShow()) {
            return;
        }
        ContainerScope scope(container->GetInstanceId());
        context->FlushBuild();
        if (context->GetInstanceId() == triggerId) {
            return;
        }
        context->PrepareCloseImplicitAnimation();
    });
    pipelineContext->SetSyncAnimationOption(AnimationOption());
    pipelineContext->CloseImplicitAnimation();
}

void AnimateToForFaMode(const RefPtr<PipelineBase>& pipelineContext, AnimationOption& option,
    const JSCallbackInfo& info, std::function<void()>& onFinishEvent)
{
    pipelineContext->FlushBuild();
    pipelineContext->OpenImplicitAnimation(option, option.GetCurve(), onFinishEvent);
    pipelineContext->SetSyncAnimationOption(option);
    JSRef<JSFunc> jsAnimateToFunc = JSRef<JSFunc>::Cast(info[1]);
    jsAnimateToFunc->Call(info[1]);
    pipelineContext->FlushBuild();
    pipelineContext->SetSyncAnimationOption(AnimationOption());
    pipelineContext->CloseImplicitAnimation();
}

int64_t GetFormAnimationTimeInterval(const RefPtr<PipelineBase>& pipelineContext)
{
    CHECK_NULL_RETURN(pipelineContext, 0);
    return (GetMicroTickCount() - pipelineContext->GetFormAnimationStartTime()) / MICROSEC_TO_MILLISEC;
}

bool CheckIfSetFormAnimationDuration(const RefPtr<PipelineBase>& pipelineContext, const AnimationOption& option)
{
    CHECK_NULL_RETURN(pipelineContext, false);
    return pipelineContext->IsFormAnimationFinishCallback() && pipelineContext->IsFormRender() &&
        option.GetDuration() > (DEFAULT_DURATION - GetFormAnimationTimeInterval(pipelineContext));
}

} // namespace

const AnimationOption JSViewContext::CreateAnimation(
    const JSRef<JSObject>& animationArgs, const std::function<float(float)>& jsFunc, bool isForm)
{
    AnimationOption option = AnimationOption();
    // If the attribute does not exist, the default value is used.
    auto duration = animationArgs->GetPropertyValue<int32_t>("duration", DEFAULT_DURATION);
    auto delay = animationArgs->GetPropertyValue<int32_t>("delay", 0);
    auto iterations = animationArgs->GetPropertyValue<int32_t>("iterations", 1);
    auto tempo = animationArgs->GetPropertyValue<double>("tempo", 1.0);
    if (SystemProperties::GetRosenBackendEnabled() && NearZero(tempo)) {
        // set duration to 0 to disable animation.
        duration = 0;
    }
    auto direction = StringToAnimationDirection(animationArgs->GetPropertyValue<std::string>("playMode", "normal"));
    auto finishCallbackType = static_cast<FinishCallbackType>(
        animationArgs->GetPropertyValue<int32_t>("finishCallbackType", 0));
    RefPtr<Curve> curve;
    auto curveArgs = animationArgs->GetProperty("curve");
    if (curveArgs->IsString()) {
        curve = CreateCurve(animationArgs->GetPropertyValue<std::string>("curve",
            DOM_ANIMATION_TIMING_FUNCTION_EASE_IN_OUT));
    } else if (curveArgs->IsObject()) {
        JSRef<JSVal> curveString = JSRef<JSObject>::Cast(curveArgs)->GetProperty("__curveString");
        if (!curveString->IsString()) {
            // Default AnimationOption which is invalid.
            return option;
        }
        auto aniTimFunc = curveString->ToString();

        std::string customFuncName(DOM_ANIMATION_TIMING_FUNCTION_CUSTOM);
        if (aniTimFunc == customFuncName) {
            curve = CreateCurve(jsFunc);
        } else {
            curve = CreateCurve(aniTimFunc);
        }
    } else {
        curve = Curves::EASE_IN_OUT;
    }

    // limit animation for ArkTS Form
    if (isForm) {
        if (duration > static_cast<int32_t>(DEFAULT_DURATION)) {
            duration = static_cast<int32_t>(DEFAULT_DURATION);
        }
        if (delay != 0) {
            delay = 0;
        }
        if (SystemProperties::IsFormAnimationLimited() && iterations != 1) {
            iterations = 1;
        }
        if (!NearEqual(tempo, 1.0)) {
            tempo = 1.0;
        }
    }

    option.SetDuration(duration);
    option.SetDelay(delay);
    option.SetIteration(iterations);
    option.SetTempo(tempo);
    option.SetAnimationDirection(direction);
    option.SetCurve(curve);
    option.SetFinishCallbackType(finishCallbackType);
    return option;
}

std::function<float(float)> ParseCallBackFunction(const JSRef<JSObject>& obj)
{
    std::function<float(float)> customCallBack = nullptr;
    JSRef<JSVal> curveVal = obj->GetProperty("curve");
    if (curveVal->IsObject()) {
        JSRef<JSObject> curveobj = JSRef<JSObject>::Cast(curveVal);
        JSRef<JSVal> onCallBack = curveobj->GetProperty("__curveCustomFunc");
        if (onCallBack->IsFunction()) {
            WeakPtr<NG::FrameNode> frameNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
            RefPtr<JsFunction> jsFuncCallBack =
                AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onCallBack));
            customCallBack = [func = std::move(jsFuncCallBack), id = Container::CurrentId(), node = frameNode]
                (float time) -> float {
                auto pipelineContext = PipelineContext::GetCurrentContext();
                CHECK_NULL_RETURN(pipelineContext, 1.0f);
                pipelineContext->UpdateCurrentActiveNode(node);
                ContainerScope scope(id);
                JSRef<JSVal> params[1];
                params[0] = JSRef<JSVal>::Make(ToJSValue(time));
                auto result = func->ExecuteJS(1, params);
                return result->IsNumber() ? result->ToNumber<float>() : 1.0f;
            };
        }
    }
    return customCallBack;
}

void JSViewContext::JSAnimation(const JSCallbackInfo& info)
{
    ACE_FUNCTION_TRACE();
    auto scopedDelegate = EngineHelper::GetCurrentDelegate();
    if (!scopedDelegate) {
        // this case usually means there is no foreground container, need to figure out the reason.
        return;
    }
    if (ViewStackModel::GetInstance()->CheckTopNodeFirstBuilding()) {
        // the node sets attribute value for the first time. No animation is generated.
        return;
    }
    AnimationOption option = AnimationOption();
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipelineContextBase = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContextBase);
    if (pipelineContextBase->IsFormAnimationFinishCallback() && pipelineContextBase->IsFormRender() &&
        GetFormAnimationTimeInterval(pipelineContextBase) > DEFAULT_DURATION) {
        TAG_LOGW(
            AceLogTag::ACE_FORM, "[Form animation] Form finish callback triggered animation cannot exceed 1000ms.");
        return;
    }
    if (info[0]->IsNull() || !info[0]->IsObject()) {
        ViewContextModel::GetInstance()->closeAnimation(option, true);
        return;
    }
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> onFinish = obj->GetProperty("onFinish");
    std::function<void()> onFinishEvent;
    if (onFinish->IsFunction()) {
        WeakPtr<NG::FrameNode> frameNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
        RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onFinish));
        onFinishEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc),
                            id = Container::CurrentId(), node = frameNode]() {
            ContainerScope scope(id);
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            auto pipelineContext = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->UpdateCurrentActiveNode(node);
            func->Execute();
        };
    }

    option = CreateAnimation(obj, ParseCallBackFunction(obj), pipelineContextBase->IsFormRender());
    if (pipelineContextBase->IsFormAnimationFinishCallback() && pipelineContextBase->IsFormRender() &&
        option.GetDuration() > (DEFAULT_DURATION - GetFormAnimationTimeInterval(pipelineContextBase))) {
        option.SetDuration(DEFAULT_DURATION - GetFormAnimationTimeInterval(pipelineContextBase));
        TAG_LOGW(AceLogTag::ACE_FORM, "[Form animation]  Form animation SetDuration: %{public}lld ms",
            static_cast<long long>(DEFAULT_DURATION - GetFormAnimationTimeInterval(pipelineContextBase)));
    }

    option.SetOnFinishEvent(onFinishEvent);
    if (SystemProperties::GetRosenBackendEnabled()) {
        option.SetAllowRunningAsynchronously(true);
    }
    ViewContextModel::GetInstance()->openAnimation(option);
    JankFrameReport::ReportJSAnimation();
}

void JSViewContext::JSAnimateTo(const JSCallbackInfo& info)
{
    ACE_FUNCTION_TRACE();
    auto scopedDelegate = EngineHelper::GetCurrentDelegate();
    if (!scopedDelegate) {
        // this case usually means there is no foreground container, need to figure out the reason.
        return;
    }
    if (info.Length() < 2) {
        return;
    }
    if (!info[0]->IsObject()) {
        return;
    }
    // 2nd argument should be a closure passed to the animateTo function.
    if (!info[1]->IsFunction()) {
        return;
    }

    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    if (pipelineContext->IsFormAnimationFinishCallback() && pipelineContext->IsFormRender() &&
        GetFormAnimationTimeInterval(pipelineContext) > DEFAULT_DURATION) {
        TAG_LOGW(
            AceLogTag::ACE_FORM, "[Form animation] Form finish callback triggered animation cannot exceed 1000ms.");
        return;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> onFinish = obj->GetProperty("onFinish");
    std::function<void()> onFinishEvent;
    auto traceStreamPtr = std::make_shared<std::stringstream>();
    if (onFinish->IsFunction()) {
        WeakPtr<NG::FrameNode> frameNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
        RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onFinish));
        onFinishEvent = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc),
                            id = Container::CurrentId(), traceStreamPtr, node = frameNode]() {
            ContainerScope scope(id);
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            auto pipelineContext = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipelineContext);
            pipelineContext->UpdateCurrentActiveNode(node);
            func->Execute();
            AceAsyncTraceEnd(0, traceStreamPtr->str().c_str(), true);
        };
    } else {
        onFinishEvent = [traceStreamPtr]() {
            AceAsyncTraceEnd(0, traceStreamPtr->str().c_str(), true);
        };
    }

    AnimationOption option =
        CreateAnimation(obj, ParseCallBackFunction(obj), pipelineContext->IsFormRender());
    *traceStreamPtr << "AnimateTo, Options"
                    << " duration:" << option.GetDuration()
                    << ",iteration:" << option.GetIteration()
                    << ",delay:" << option.GetDelay()
                    << ",tempo:" << option.GetTempo()
                    << ",direction:" << (uint32_t) option.GetAnimationDirection()
                    << ",curve:" << (option.GetCurve() ? option.GetCurve()->ToString().c_str() : "");
    AceAsyncTraceBegin(0, traceStreamPtr->str().c_str(), true);
    if (CheckIfSetFormAnimationDuration(pipelineContext, option)) {
        option.SetDuration(DEFAULT_DURATION - GetFormAnimationTimeInterval(pipelineContext));
        TAG_LOGW(AceLogTag::ACE_FORM, "[Form animation]  Form animation SetDuration: %{public}lld ms",
            static_cast<long long>(DEFAULT_DURATION - GetFormAnimationTimeInterval(pipelineContext)));
    }
    if (SystemProperties::GetRosenBackendEnabled()) {
        bool usingSharedRuntime = container->GetSettings().usingSharedRuntime;
        if (usingSharedRuntime) {
            if (pipelineContext->IsLayouting()) {
                TAG_LOGW(AceLogTag::ACE_ANIMATION,
                    "pipeline is layouting, post animateTo, duration:%{public}d, curve:%{public}s",
                    option.GetDuration(), option.GetCurve() ? option.GetCurve()->ToString().c_str() : "");
                pipelineContext->GetTaskExecutor()->PostTask(
                    [id = Container::CurrentId(), option, func = JSRef<JSFunc>::Cast(info[1]),
                        onFinishEvent]() mutable {
                        ContainerScope scope(id);
                        auto container = Container::Current();
                        CHECK_NULL_VOID(container);
                        auto pipelineContext = container->GetPipelineContext();
                        CHECK_NULL_VOID(pipelineContext);
                        AnimateToForStageMode(pipelineContext, option, func, onFinishEvent);
                    },
                    TaskExecutor::TaskType::UI);
                return;
            }
            AnimateToForStageMode(pipelineContext, option, JSRef<JSFunc>::Cast(info[1]), onFinishEvent);
        } else {
            AnimateToForFaMode(pipelineContext, option, info, onFinishEvent);
        }
    } else {
        pipelineContext->FlushBuild();
        pipelineContext->SaveExplicitAnimationOption(option);
        // Execute the function.
        JSRef<JSFunc> jsAnimateToFunc = JSRef<JSFunc>::Cast(info[1]);
        jsAnimateToFunc->Call(info[1]);
        pipelineContext->FlushBuild();
        pipelineContext->CreateExplicitAnimator(onFinishEvent);
        pipelineContext->ClearExplicitAnimationOption();
    }
}

void JSViewContext::JSBind(BindingTarget globalObj)
{
    JSClass<JSViewContext>::Declare("Context");
    JSClass<JSViewContext>::StaticMethod("animation", JSAnimation);
    JSClass<JSViewContext>::StaticMethod("animateTo", JSAnimateTo);
    JSClass<JSViewContext>::Bind<>(globalObj);
}

} // namespace OHOS::Ace::Framework
