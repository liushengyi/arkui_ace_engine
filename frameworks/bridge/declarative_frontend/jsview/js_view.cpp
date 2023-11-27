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

#include "bridge/declarative_frontend/jsview/js_view.h"

#include "base/log/ace_checker.h"
#include "base/log/ace_performance_check.h"
#include "base/log/ace_trace.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/engine_helper.h"
#include "bridge/declarative_frontend/engine/js_converter.h"
#include "bridge/declarative_frontend/engine/js_execution_scope_defines.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/jsview/js_view_stack_processor.h"
#include "bridge/declarative_frontend/jsview/models/view_full_update_model_impl.h"
#include "bridge/declarative_frontend/jsview/models/view_partial_update_model_impl.h"
#include "bridge/declarative_frontend/ng/declarative_frontend_ng.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components_ng/base/observer_handler.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/view_full_update_model.h"
#include "core/components_ng/base/view_full_update_model_ng.h"
#include "core/components_ng/base/view_partial_update_model.h"
#include "core/components_ng/base/view_partial_update_model_ng.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace {

std::unique_ptr<ViewFullUpdateModel> ViewFullUpdateModel::instance_ = nullptr;
std::mutex ViewFullUpdateModel::mutex_;
std::unique_ptr<ViewPartialUpdateModel> ViewPartialUpdateModel::instance_ = nullptr;
std::mutex ViewPartialUpdateModel::mutex_;

ViewFullUpdateModel* ViewFullUpdateModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::ViewFullUpdateModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::ViewFullUpdateModelNG());
            } else {
                instance_.reset(new Framework::ViewFullUpdateModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

ViewPartialUpdateModel* ViewPartialUpdateModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::ViewPartialUpdateModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::ViewPartialUpdateModelNG());
            } else {
                instance_.reset(new Framework::ViewPartialUpdateModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {

void JSView::JSBind(BindingTarget object)
{
    JSViewPartialUpdate::JSBind(object);
    JSViewFullUpdate::JSBind(object);
}

void JSView::RenderJSExecution()
{
    JAVASCRIPT_EXECUTION_SCOPE_STATIC;
    if (!jsViewFunction_) {
        LOGE("JSView: InternalRender jsViewFunction_ error");
        return;
    }
    {
        ACE_SCORING_EVENT("Component.AboutToRender");
        jsViewFunction_->ExecuteAboutToRender();
    }
    if (!jsViewFunction_) {
        LOGE("JSView: After ExecuteAboutToRender jsViewFunction_ error");
        return;
    }
    {
        ACE_SCORING_EVENT("Component.Build");
        ViewStackModel::GetInstance()->PushKey(viewId_);
        jsViewFunction_->ExecuteRender();
        ViewStackModel::GetInstance()->PopKey();
    }
    if (!jsViewFunction_) {
        LOGE("JSView: After ExecuteRender jsViewFunction_ error");
        return;
    }
    {
        ACE_SCORING_EVENT("Component.OnRenderDone");
        jsViewFunction_->ExecuteOnRenderDone();
        if (notifyRenderDone_) {
            notifyRenderDone_();
        }
    }
}

void JSView::SyncInstanceId()
{
    restoreInstanceId_ = Container::CurrentId();
    ContainerScope::UpdateCurrent(instanceId_);
}

void JSView::RestoreInstanceId()
{
    ContainerScope::UpdateCurrent(restoreInstanceId_);
}

void JSView::GetInstanceId(const JSCallbackInfo& info)
{
    info.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(instanceId_)));
}

void JSView::JsSetCardId(int64_t cardId)
{
    cardId_ = cardId;
}

void JSView::JsGetCardId(const JSCallbackInfo& info)
{
    info.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(cardId_)));
}

JSViewFullUpdate::JSViewFullUpdate(const std::string& viewId, JSRef<JSObject> jsObject, JSRef<JSFunc> jsRenderFunction)
{
    viewId_ = viewId;
    jsViewFunction_ = AceType::MakeRefPtr<ViewFunctions>(jsObject, jsRenderFunction);
    jsViewObject_ = jsObject;
    LOGD("JSViewFullUpdate constructor");
}

JSViewFullUpdate::~JSViewFullUpdate()
{
    LOGD("JSViewFullUpdate destructor");
    jsViewFunction_.Reset();
};

RefPtr<AceType> JSViewFullUpdate::CreateViewNode(bool isTitleNode)
{
    auto appearFunc = [weak = AceType::WeakClaim(this)] {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        ACE_SCORING_EVENT("Component[" + jsView->viewId_ + "].Appear");
        if (jsView->viewNode_.Invalid() && jsView->jsViewFunction_) {
            jsView->jsViewFunction_->ExecuteAppear();
        }
    };

    auto renderFunction = [weak = AceType::WeakClaim(this)]() -> RefPtr<AceType> {
        auto jsView = weak.Upgrade();
        CHECK_NULL_RETURN(jsView, nullptr);
        ContainerScope scope(jsView->GetInstanceId());
        return jsView->InternalRender();
    };

    auto pageTransitionFunction = [weak = AceType::WeakClaim(this)]() {
        auto jsView = weak.Upgrade();
        if (!jsView || !jsView->jsViewFunction_) {
            return;
        }
        {
            ContainerScope scope(jsView->GetInstanceId());
            ACE_SCORING_EVENT("Component[" + jsView->viewId_ + "].Transition");
            jsView->jsViewFunction_->ExecuteTransition();
        }
    };

    auto updateViewNodeFunction = [weak = AceType::WeakClaim(this)](const RefPtr<AceType>& node) {
        auto jsView = weak.Upgrade();
        if (jsView) {
            jsView->viewNode_ = node;
        }
    };

    auto removeFunction = [weak = AceType::WeakClaim(this)]() -> void {
        auto jsView = weak.Upgrade();
        if (jsView && jsView->jsViewFunction_) {
            ContainerScope scope(jsView->GetInstanceId());
            jsView->jsViewFunction_->ExecuteDisappear();
        }
    };

    NodeInfo info = { .viewId = viewId_,
        .appearFunc = std::move(appearFunc),
        .renderFunc = std::move(renderFunction),
        .removeFunc = std::move(removeFunction),
        .updateNodeFunc = std::move(updateViewNodeFunction),
        .isStatic = IsStatic() };

    if (jsViewFunction_ && jsViewFunction_->HasPageTransition()) {
        info.pageTransitionFunc = std::move(pageTransitionFunction);
    }

    return ViewFullUpdateModel::GetInstance()->CreateNode(std::move(info));
}

RefPtr<AceType> JSViewFullUpdate::InternalRender()
{
    JAVASCRIPT_EXECUTION_SCOPE_STATIC;
    needsUpdate_ = false;
    RenderJSExecution();
    CleanUpAbandonedChild();
    jsViewFunction_->Destroy();
    return ViewStackModel::GetInstance()->Finish();
}

/**
 * marks the JSView's composed component as needing update / rerender
 */
void JSViewFullUpdate::MarkNeedUpdate()
{
    ACE_SCOPED_TRACE("JSView::MarkNeedUpdate");
    needsUpdate_ = ViewFullUpdateModel::GetInstance()->MarkNeedUpdate(viewNode_);
}

void JSViewFullUpdate::Destroy(JSView* parentCustomView)
{
    LOGD("JSViewFullUpdate::Destroy start");
    DestroyChild(parentCustomView);
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].Disappear");
        jsViewFunction_->ExecuteDisappear();
    }
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].AboutToBeDeleted");
        jsViewFunction_->ExecuteAboutToBeDeleted();
    }
    jsViewObject_.Reset();
    LOGD("JSViewFullUpdate::Destroy end");
}

void JSViewFullUpdate::Create(const JSCallbackInfo& info)
{
    LOGD("Creating new View for full update");
    ACE_DCHECK(!Container::IsCurrentUsePartialUpdate());

    if (info[0]->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
        auto* view = object->Unwrap<JSViewFullUpdate>();
        if (view == nullptr) {
            LOGE("JSView is null");
            return;
        }
        ViewStackModel::GetInstance()->Push(view->CreateViewNode(), true);
    } else {
        LOGE("JSView Object is expected.");
    }
}

void JSViewFullUpdate::JSBind(BindingTarget object)
{
    LOGD("JSViewFullUpdate::Bind");
    JSClass<JSViewFullUpdate>::Declare("NativeViewFullUpdate");
    JSClass<JSViewFullUpdate>::StaticMethod("create", &JSViewFullUpdate::Create);
    JSClass<JSViewFullUpdate>::Method("markNeedUpdate", &JSViewFullUpdate::MarkNeedUpdate);
    JSClass<JSViewFullUpdate>::Method("syncInstanceId", &JSViewFullUpdate::SyncInstanceId);
    JSClass<JSViewFullUpdate>::Method("restoreInstanceId", &JSViewFullUpdate::RestoreInstanceId);
    JSClass<JSViewFullUpdate>::CustomMethod("getInstanceId", &JSViewFullUpdate::GetInstanceId);
    JSClass<JSViewFullUpdate>::Method("needsUpdate", &JSViewFullUpdate::NeedsUpdate);
    JSClass<JSViewFullUpdate>::Method("markStatic", &JSViewFullUpdate::MarkStatic);
    JSClass<JSViewFullUpdate>::Method("setCardId", &JSViewFullUpdate::JsSetCardId);
    JSClass<JSViewFullUpdate>::CustomMethod("getCardId", &JSViewFullUpdate::JsGetCardId);
    JSClass<JSViewFullUpdate>::CustomMethod("findChildById", &JSViewFullUpdate::FindChildById);
    JSClass<JSViewFullUpdate>::CustomMethod("findChildByIdForPreview", &JSViewFullUpdate::FindChildByIdForPreview);
    JSClass<JSViewFullUpdate>::InheritAndBind<JSViewAbstract>(object, ConstructorCallback, DestructorCallback);
}

void JSViewFullUpdate::FindChildById(const JSCallbackInfo& info)
{
    LOGD("JSView::FindChildById");
    if (info[0]->IsNumber() || info[0]->IsString()) {
        std::string viewId = info[0]->ToString();
        info.SetReturnValue(GetChildById(viewId));
    } else {
        LOGE("JSView FindChildById with invalid arguments.");
        JSException::Throw("%s", "JSView FindChildById with invalid arguments.");
    }
}

void JSViewFullUpdate::FindChildByIdForPreview(const JSCallbackInfo& info)
{
    if (!info[0]->IsNumber()) {
        LOGE("info[0] is not a number");
        return;
    }
    std::string viewId = std::to_string(info[0]->ToNumber<int32_t>());
    if (viewId_ == viewId) {
        info.SetReturnValue(jsViewObject_);
        return;
    }
    JSRef<JSObject> targetView = JSRef<JSObject>::New();
    for (auto&& child : customViewChildren_) {
        if (GetChildByViewId(viewId, child.second, targetView)) {
            break;
        }
    }
    auto view = targetView->Unwrap<JSViewFullUpdate>();
    if (view) {
        LOGD("find targetView success");
        info.SetReturnValue(targetView);
    }
    return;
}

bool JSViewFullUpdate::GetChildByViewId(
    const std::string& viewId, JSRef<JSObject>& childView, JSRef<JSObject>& targetView)
{
    auto* view = childView->Unwrap<JSViewFullUpdate>();
    if (view && view->viewId_ == viewId) {
        targetView = childView;
        return true;
    }
    for (auto&& child : view->customViewChildren_) {
        if (GetChildByViewId(viewId, child.second, targetView)) {
            return true;
        }
    }
    return false;
}

void JSViewFullUpdate::ConstructorCallback(const JSCallbackInfo& info)
{
    JSRef<JSObject> thisObj = info.This();
    JSRef<JSVal> renderFunc = thisObj->GetProperty("render");
    if (!renderFunc->IsFunction()) {
        LOGE("View derived classes must provide render(){...} function");
        JSException::Throw("%s", "View derived classes must provide render(){...} function");
        return;
    }

    int argc = info.Length();
    if (argc > 1 && (info[0]->IsNumber() || info[0]->IsString())) {
        std::string viewId = info[0]->ToString();
        auto instance = AceType::MakeRefPtr<JSViewFullUpdate>(viewId, info.This(), JSRef<JSFunc>::Cast(renderFunc));
        auto context = info.GetExecutionContext();
        instance->SetContext(context);
        instance->IncRefCount();
        info.SetReturnValue(AceType::RawPtr(instance));
        if (!info[1]->IsUndefined() && info[1]->IsObject()) {
            JSRef<JSObject> parentObj = JSRef<JSObject>::Cast(info[1]);
            auto* parentView = parentObj->Unwrap<JSViewFullUpdate>();
            if (parentView != nullptr) {
                auto id = parentView->AddChildById(viewId, info.This());
                instance->id_ = id;
            }
        }
        LOGD("JSView ConstructorCallback: %{public}s", instance->id_.c_str());
    } else {
        LOGE("JSView creation with invalid arguments.");
        JSException::Throw("%s", "JSView creation with invalid arguments.");
    }
}

void JSViewFullUpdate::DestructorCallback(JSViewFullUpdate* view)
{
    if (view == nullptr) {
        LOGE("JSViewFullUpdate::DestructorCallback failed: the view is nullptr");
        return;
    }
    LOGD("JSViewFullUpdate(DestructorCallback) start: %{public}s", view->id_.c_str());
    view->DecRefCount();
    LOGD("JSViewFullUpdate(DestructorCallback) end");
}

void JSViewFullUpdate::DestroyChild(JSView* parentCustomView)
{
    LOGD("JSViewFullUpdate::DestroyChild start");
    for (auto&& child : customViewChildren_) {
        auto* view = child.second->Unwrap<JSView>();
        if (view != nullptr) {
            view->Destroy(this);
        }
        child.second.Reset();
    }
    customViewChildren_.clear();
    for (auto&& lazyChild : customViewChildrenWithLazy_) {
        auto* view = lazyChild.second->Unwrap<JSView>();
        if (view != nullptr) {
            view->Destroy(this);
        }
        lazyChild.second.Reset();
    }
    customViewChildrenWithLazy_.clear();
    LOGD("JSViewFullUpdate::DestroyChild end");
}

void JSViewFullUpdate::CleanUpAbandonedChild()
{
    auto startIter = customViewChildren_.begin();
    auto endIter = customViewChildren_.end();
    std::vector<std::string> removedViewIds;
    while (startIter != endIter) {
        auto found = lastAccessedViewIds_.find(startIter->first);
        if (found == lastAccessedViewIds_.end()) {
            LOGD(" found abandoned view with id %{public}s", startIter->first.c_str());
            removedViewIds.emplace_back(startIter->first);
            auto* view = startIter->second->Unwrap<JSView>();
            if (view != nullptr) {
                view->Destroy(this);
            }
            startIter->second.Reset();
        }
        ++startIter;
    }

    for (auto& viewId : removedViewIds) {
        customViewChildren_.erase(viewId);
    }

    lastAccessedViewIds_.clear();
}

JSRef<JSObject> JSViewFullUpdate::GetChildById(const std::string& viewId)
{
    std::string id = ViewStackModel::GetInstance()->ProcessViewId(viewId);
    auto found = customViewChildren_.find(id);
    if (found != customViewChildren_.end()) {
        ChildAccessedById(id);
        return found->second;
    }
    auto lazyItem = customViewChildrenWithLazy_.find(id);
    if (lazyItem != customViewChildrenWithLazy_.end()) {
        return lazyItem->second;
    }
    return {};
}

std::string JSViewFullUpdate::AddChildById(const std::string& viewId, const JSRef<JSObject>& obj)
{
    std::string id = ViewStackModel::GetInstance()->ProcessViewId(viewId);
    JSView* jsView = nullptr;
    if (isLazyForEachProcessed_) {
        auto result = customViewChildrenWithLazy_.try_emplace(id, obj);
        if (!result.second) {
            jsView = result.first->second->Unwrap<JSView>();
            result.first->second = obj;
        } else {
            lazyItemGroups_[lazyItemGroupId_].emplace_back(id);
        }
    } else {
        auto result = customViewChildren_.try_emplace(id, obj);
        if (!result.second) {
            jsView = result.first->second->Unwrap<JSView>();
            result.first->second = obj;
        }
        ChildAccessedById(id);
    }
    if (jsView != nullptr) {
        jsView->Destroy(this);
    }
    return id;
}

void JSViewFullUpdate::RemoveChildGroupById(const std::string& viewId)
{
    // js runtime may be released
    CHECK_JAVASCRIPT_SCOPE_AND_RETURN;
    JAVASCRIPT_EXECUTION_SCOPE_STATIC;
    LOGD("JSViewFullUpdate::RemoveChildGroupById in lazy for each case: %{public}s", viewId.c_str());
    auto iter = lazyItemGroups_.find(viewId);
    if (iter == lazyItemGroups_.end()) {
        LOGI("can not find this group to delete: %{public}s", viewId.c_str());
        return;
    }
    std::vector<std::string> removedViewIds;
    for (auto&& item : iter->second) {
        auto removeView = customViewChildrenWithLazy_.find(item);
        if (removeView != customViewChildrenWithLazy_.end()) {
            if (!removeView->second.IsEmpty()) {
                auto* view = removeView->second->Unwrap<JSView>();
                if (view != nullptr) {
                    view->Destroy(this);
                }
                removeView->second.Reset();
            }
            removedViewIds.emplace_back(item);
        }
    }

    for (auto&& removeId : removedViewIds) {
        customViewChildrenWithLazy_.erase(removeId);
    }
    lazyItemGroups_.erase(iter);
}

void JSViewFullUpdate::ChildAccessedById(const std::string& viewId)
{
    lastAccessedViewIds_.emplace(viewId);
}

// =================================================================

std::map<std::string, JSRef<JSObject>> JSViewStackProcessor::viewMap_;

JSViewPartialUpdate::JSViewPartialUpdate(JSRef<JSObject> jsViewObject)
{
    jsViewFunction_ = AceType::MakeRefPtr<ViewFunctions>(jsViewObject);
    LOGD("JSViewPartialUpdate constructor");
    // keep the reference to the JS View object to prevent GC
    jsViewObject_ = jsViewObject;
}

JSViewPartialUpdate::~JSViewPartialUpdate()
{
    LOGD("JSViewPartialUpdate destructor");
    jsViewFunction_.Reset();
};

RefPtr<AceType> JSViewPartialUpdate::CreateViewNode(bool isTitleNode)
{
    auto updateViewIdFunc = [weak = AceType::WeakClaim(this)](const std::string viewId) {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        jsView->viewId_ = viewId;
    };

    auto appearFunc = [weak = AceType::WeakClaim(this)]() {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        ACE_SCORING_EVENT("Component[" + jsView->viewId_ + "].Appear");
        if (jsView->jsViewFunction_) {
            jsView->jsViewFunction_->ExecuteAppear();
        }
    };

    auto renderFunction = [weak = AceType::WeakClaim(this)]() -> RefPtr<AceType> {
        auto jsView = weak.Upgrade();
        CHECK_NULL_RETURN(jsView, nullptr);
        ContainerScope scope(jsView->GetInstanceId());
        if (!jsView->isFirstRender_) {
            LOGW("the js view has already called initial render");
            return nullptr;
        }
        jsView->isFirstRender_ = false;
        return jsView->InitialRender();
    };

    auto updateFunction = [weak = AceType::WeakClaim(this)]() -> void {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        if (!jsView->needsUpdate_) {
            LOGW("the js view does not need to update");
            return;
        }
        jsView->needsUpdate_ = false;
        LOGD("Rerender function start for ComposedElement elmtId %{public}s - start...", jsView->viewId_.c_str());
        {
            ACE_SCOPED_TRACE("JSView: ExecuteRerender");
            jsView->jsViewFunction_->ExecuteRerender();
        }
        for (const UpdateTask& updateTask : jsView->pendingUpdateTasks_) {
            ViewPartialUpdateModel::GetInstance()->FlushUpdateTask(updateTask);
        }
        jsView->pendingUpdateTasks_.clear();
    };

    auto reloadFunction = [weak = AceType::WeakClaim(this)](bool deep) {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        CHECK_NULL_VOID(jsView->jsViewFunction_);
        ContainerScope scope(jsView->GetInstanceId());
        jsView->jsViewFunction_->ExecuteReload(deep);
    };

    // @Component level complete reload, can detect added/deleted frame nodes
    auto completeReloadFunc = [weak = AceType::WeakClaim(this)]() -> RefPtr<AceType> {
        auto jsView = weak.Upgrade();
        CHECK_NULL_RETURN(jsView, nullptr);
        ContainerScope scope(jsView->GetInstanceId());
        return jsView->InitialRender();
    };

    auto pageTransitionFunction = [weak = AceType::WeakClaim(this)]() {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        CHECK_NULL_VOID(jsView->jsViewFunction_);
        ContainerScope scope(jsView->GetInstanceId());
        {
            ACE_SCORING_EVENT("Component[" + jsView->viewId_ + "].Transition");
            jsView->jsViewFunction_->ExecuteTransition();
        }
    };

    auto removeFunction = [weak = AceType::WeakClaim(this)]() -> void {
        LOGD("call remove view function");
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        jsView->Destroy(nullptr);
        jsView->viewNode_.Reset();
    };

    auto updateViewNodeFunction = [weak = AceType::WeakClaim(this)](const RefPtr<AceType>& node) {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        jsView->viewNode_ = node;
    };

    auto nodeUpdateFunc = [weak = AceType::WeakClaim(this)](int32_t nodeId) {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        CHECK_NULL_VOID(jsView->jsViewFunction_);
        ContainerScope scope(jsView->GetInstanceId());
        jsView->jsViewFunction_->ExecuteForceNodeRerender(nodeId);
    };

    auto recycleCustomNode = [weak = AceType::WeakClaim(this)](const RefPtr<NG::CustomNodeBase>& recycleNode) -> void {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        recycleNode->ResetRecycle();
        auto name = jsView->GetRecycleCustomNodeName();
        if (name.empty()) {
            return;
        }
        AceType::DynamicCast<NG::UINode>(recycleNode)->SetActive(false);
        jsView->SetRecycleCustomNode(recycleNode);
        if (!recycleNode->HasRecycleRenderFunc()) {
            jsView->jsViewFunction_->ExecuteAboutToRecycle();
        }
        jsView->jsViewFunction_->ExecuteRecycle(jsView->GetRecycleCustomNodeName());
    };

    auto setActiveFunc = [weak = AceType::WeakClaim(this)](bool active) -> void {
        if (!AceApplicationInfo::GetInstance().IsDelayedUpdateOnInactive()) {
            return;
        }
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        jsView->jsViewFunction_->ExecuteSetActive(active);
    };

    auto onDumpInfoFunc = [weak = AceType::WeakClaim(this)](const std::vector<std::string>& params) -> void {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        jsView->jsViewFunction_->ExecuteOnDumpInfo(params);
    };

    NodeInfoPU info = { .appearFunc = std::move(appearFunc),
        .renderFunc = std::move(renderFunction),
        .updateFunc = std::move(updateFunction),
        .removeFunc = std::move(removeFunction),
        .updateNodeFunc = std::move(updateViewNodeFunction),
        .pageTransitionFunc = std::move(pageTransitionFunction),
        .reloadFunc = std::move(reloadFunction),
        .completeReloadFunc = std::move(completeReloadFunc),
        .nodeUpdateFunc = std::move(nodeUpdateFunc),
        .recycleCustomNodeFunc = recycleCustomNode,
        .setActiveFunc = std::move(setActiveFunc),
        .onDumpInfoFunc = std::move(onDumpInfoFunc),
        .hasMeasureOrLayout = jsViewFunction_->HasMeasure() || jsViewFunction_->HasLayout() ||
                              jsViewFunction_->HasMeasureSize() || jsViewFunction_->HasPlaceChildren(),
        .isStatic = IsStatic(),
        .jsViewName = GetJSViewName() };

    auto measureFunc = [weak = AceType::WeakClaim(this)](NG::LayoutWrapper* layoutWrapper) -> void {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        jsView->jsViewFunction_->ExecuteMeasure(layoutWrapper);
    };
    if (jsViewFunction_->HasMeasure()) {
        info.measureFunc = std::move(measureFunc);
    }

    auto layoutFunc = [weak = AceType::WeakClaim(this)](NG::LayoutWrapper* layoutWrapper) -> void {
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        ContainerScope scope(jsView->GetInstanceId());
        jsView->jsViewFunction_->ExecuteLayout(layoutWrapper);
    };
    if (jsViewFunction_->HasLayout()) {
        info.layoutFunc = std::move(layoutFunc);
    }

    if (jsViewFunction_->HasMeasureSize()) {
        auto measureSizeFunc = [weak = AceType::WeakClaim(this)](NG::LayoutWrapper* layoutWrapper) -> void {
            auto jsView = weak.Upgrade();
            CHECK_NULL_VOID(jsView);
            ContainerScope scope(jsView->GetInstanceId());
            jsView->jsViewFunction_->ExecuteMeasureSize(layoutWrapper);
        };
        info.measureSizeFunc = std::move(measureSizeFunc);
    }

    if (jsViewFunction_->HasPlaceChildren()) {
        auto placeChildren = [weak = AceType::WeakClaim(this)](NG::LayoutWrapper* layoutWrapper) -> void {
            auto jsView = weak.Upgrade();
            CHECK_NULL_VOID(jsView);
            ContainerScope scope(jsView->GetInstanceId());
            jsView->jsViewFunction_->ExecutePlaceChildren(layoutWrapper);
        };
        info.placeChildrenFunc = std::move(placeChildren);
    }

    JSRef<JSObject> jsViewExtraInfo = jsViewObject_->GetProperty("extraInfo_");
    if (!jsViewExtraInfo->IsUndefined()) {
        JSRef<JSVal> jsPage = jsViewExtraInfo->GetProperty("page");
        JSRef<JSVal> jsLine = jsViewExtraInfo->GetProperty("line");
        info.extraInfo = {.page = jsPage->ToString(), .line = jsLine->ToNumber<int32_t>()};
    }
    
    if (isTitleNode) {
        info.isCustomTitle = true;
    }

    auto node = ViewPartialUpdateModel::GetInstance()->CreateNode(std::move(info));
#ifdef PREVIEW
    auto uiNode = AceType::DynamicCast<NG::UINode>(node);
    if (uiNode) {
        Framework::JSViewStackProcessor::SetViewMap(std::to_string(uiNode->GetId()), jsViewObject_);
    }
#endif

    if (AceChecker::IsPerformanceCheckEnabled()) {
        auto uiNode = AceType::DynamicCast<NG::UINode>(node);
        if (uiNode) {
            auto codeInfo = EngineHelper::GetPositionOnJsCode();
            uiNode->SetRow(codeInfo.first);
            uiNode->SetCol(codeInfo.second);
        }
    }
    return node;
}

RefPtr<AceType> JSViewPartialUpdate::InitialRender()
{
    needsUpdate_ = false;
    RenderJSExecution();
    return ViewStackModel::GetInstance()->Finish();
}

// parentCustomView in not used by PartialUpdate
void JSViewPartialUpdate::Destroy(JSView* parentCustomView)
{
    if (jsViewFunction_ == nullptr) {
        // already called Destroy before
        return;
    }

    LOGD("JSViewPartialUpdate::Destroy start");
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].Disappear");
        jsViewFunction_->ExecuteDisappear();
    }
    {
        ACE_SCORING_EVENT("Component[" + viewId_ + "].AboutToBeDeleted");
        jsViewFunction_->ExecuteAboutToBeDeleted();
    }
    pendingUpdateTasks_.clear();
    jsViewFunction_->Destroy();
    jsViewFunction_.Reset();

    // release reference to JS view object, and allow GC, calls DestructorCallback
    jsViewObject_.Reset();
    LOGD("JSViewPartialUpdate::Destroy end");
}

void JSViewPartialUpdate::MarkNeedUpdate()
{
    needsUpdate_ = ViewPartialUpdateModel::GetInstance()->MarkNeedUpdate(viewNode_);
}

/**
 * in JS View.create(new View(...));
 * used for FullRender case, not for re-render case
 */
void JSViewPartialUpdate::Create(const JSCallbackInfo& info)
{
    LOGD("Creating new JSViewPartialUpdate for partial update");
    ACE_DCHECK(Container::IsCurrentUsePartialUpdate());

    if (info[0]->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
        auto* view = object->Unwrap<JSView>();
        if (view == nullptr) {
            LOGE("View is null");
            return;
        }
        ViewStackModel::GetInstance()->Push(view->CreateViewNode(), true);
    } else {
        LOGE("View Object is expected.");
    }
}

enum {
    PARAM_VIEW_OBJ = 0,
    PARAM_IS_RECYCLE,
    PARAM_NODE_NAME,
    PARAM_RECYCLE_UPDATE_FUNC,

    PARAM_SIZE,
};

bool ParseRecycleParams(const JSCallbackInfo& info, JSRef<JSVal> (&params)[PARAM_SIZE])
{
    if (info.Length() != PARAM_SIZE) {
        return false;
    }
    if (!info[PARAM_VIEW_OBJ]->IsObject()) {
        return false;
    }
    if (!info[PARAM_IS_RECYCLE]->IsBoolean()) {
        return false;
    }
    if (!info[PARAM_RECYCLE_UPDATE_FUNC]->IsFunction()) {
        return false;
    }

    for (int32_t idx = PARAM_VIEW_OBJ; idx < PARAM_SIZE; ++idx) {
        params[idx] = info[idx];
    }
    return true;
}

/**
 * in JS ViewPU.createRecycle(...)
 * create a recyclable custom node
 */
void JSViewPartialUpdate::CreateRecycle(const JSCallbackInfo& info)
{
    ACE_DCHECK(Container::IsCurrentUsePartialUpdate());

    JSRef<JSVal> params[PARAM_SIZE];
    if (!ParseRecycleParams(info, params)) {
        LOGE("Invalid parameters");
        return;
    }

    auto viewObj = JSRef<JSObject>::Cast(params[PARAM_VIEW_OBJ]);
    auto* view = viewObj->Unwrap<JSViewPartialUpdate>();
    if (!view) {
        LOGE("Invalid JSView");
        return;
    }
    if (info[PARAM_NODE_NAME]->IsUndefined()) {
        view->SetRecycleCustomNodeName("");
        ViewStackModel::GetInstance()->Push(view->CreateViewNode(), true);
        return;
    }
    auto recycle = params[PARAM_IS_RECYCLE]->ToBoolean();
    auto nodeName = params[PARAM_NODE_NAME]->ToString();
    auto jsRecycleUpdateFunc =
        AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(params[PARAM_RECYCLE_UPDATE_FUNC]));
    auto recycleUpdateFunc = [weak = AceType::WeakClaim(view), execCtx = info.GetExecutionContext(),
                                 func = std::move(jsRecycleUpdateFunc)]() -> void {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        auto jsView = weak.Upgrade();
        CHECK_NULL_VOID(jsView);
        jsView->SetIsRecycleRerender(true);
        func->ExecuteJS();
        jsView->SetIsRecycleRerender(false);
    };

    // update view and node property
    view->SetRecycleCustomNodeName(nodeName);

    // get or create recycle node
    if (recycle) {
        auto node = view->GetCachedRecycleNode();
        node->SetRecycleRenderFunc(std::move(recycleUpdateFunc));
        auto newElmtId = ViewStackModel::GetInstance()->GetElmtIdToAccountFor();
        auto uiNode = AceType::DynamicCast<NG::UINode>(node);
        ElementRegister::GetInstance()->UpdateRecycleElmtId(uiNode->GetId(), newElmtId);
        uiNode->UpdateRecycleElmtId(newElmtId);
        ViewStackModel::GetInstance()->Push(node, true);
    } else {
        ViewStackModel::GetInstance()->Push(view->CreateViewNode(), true);
    }
}

void JSViewPartialUpdate::OnDumpInfo(const std::vector<std::string>& params)
{
    CHECK_NULL_VOID(jsViewFunction_);
    jsViewFunction_->ExecuteOnDumpInfo(params);
}

void JSViewPartialUpdate::JSGetNavDestinationInfo(const JSCallbackInfo& info)
{
    auto result = NG::UIObserverHandler::GetInstance().GetNavigationState(GetViewNode());
    if (result) {
        JSRef<JSObject> obj = JSRef<JSObject>::New();
        obj->SetProperty<std::string>("navigationId", result->navigationId);
        obj->SetProperty<std::string>("name", result->name);
        obj->SetProperty<int32_t>("state", static_cast<int32_t>(result->state));
        info.SetReturnValue(obj);
    }
}

void JSViewPartialUpdate::JSGetUIContext(const JSCallbackInfo& info)
{
    ContainerScope scope(GetInstanceId());
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto frontend = container->GetFrontend();
    CHECK_NULL_VOID(frontend);
    auto context = frontend->GetContextValue();
    auto jsVal = JsConverter::ConvertNapiValueToJsVal(context);
    info.SetReturnValue(jsVal);
}

void JSViewPartialUpdate::JSBind(BindingTarget object)
{
    LOGD("JSViewPartialUpdate::Bind");
    JSClass<JSViewPartialUpdate>::Declare("NativeViewPartialUpdate");
    MethodOptions opt = MethodOptions::NONE;

    JSClass<JSViewPartialUpdate>::StaticMethod("create", &JSViewPartialUpdate::Create, opt);
    JSClass<JSViewPartialUpdate>::StaticMethod("createRecycle", &JSViewPartialUpdate::CreateRecycle, opt);
    JSClass<JSViewPartialUpdate>::Method("markNeedUpdate", &JSViewPartialUpdate::MarkNeedUpdate);
    JSClass<JSViewPartialUpdate>::Method("syncInstanceId", &JSViewPartialUpdate::SyncInstanceId);
    JSClass<JSViewPartialUpdate>::Method("restoreInstanceId", &JSViewPartialUpdate::RestoreInstanceId);
    JSClass<JSViewPartialUpdate>::CustomMethod("getInstanceId", &JSViewPartialUpdate::GetInstanceId);
    JSClass<JSViewPartialUpdate>::Method("markStatic", &JSViewPartialUpdate::MarkStatic);
    JSClass<JSViewPartialUpdate>::Method("finishUpdateFunc", &JSViewPartialUpdate::JsFinishUpdateFunc);
    JSClass<JSViewPartialUpdate>::Method("setCardId", &JSViewPartialUpdate::JsSetCardId);
    JSClass<JSViewPartialUpdate>::CustomMethod("getCardId", &JSViewPartialUpdate::JsGetCardId);
    JSClass<JSViewPartialUpdate>::Method("elmtIdExists", &JSViewPartialUpdate::JsElementIdExists);
    JSClass<JSViewPartialUpdate>::CustomMethod("isLazyItemRender", &JSViewPartialUpdate::JSGetProxiedItemRenderState);
    JSClass<JSViewPartialUpdate>::CustomMethod("isFirstRender", &JSViewPartialUpdate::IsFirstRender);
    JSClass<JSViewPartialUpdate>::CustomMethod(
        "findChildByIdForPreview", &JSViewPartialUpdate::FindChildByIdForPreview);
    JSClass<JSViewPartialUpdate>::CustomMethod(
        "resetRecycleCustomNode", &JSViewPartialUpdate::JSResetRecycleCustomNode);
    JSClass<JSViewPartialUpdate>::Method("invalidateLayout", &JSViewPartialUpdate::JsInvalidateLayout);
    JSClass<JSViewPartialUpdate>::CustomMethod(
        "queryNavDestinationInfo", &JSViewPartialUpdate::JSGetNavDestinationInfo);
    JSClass<JSViewPartialUpdate>::CustomMethod("getUIContext", &JSViewPartialUpdate::JSGetUIContext);
    JSClass<JSViewPartialUpdate>::InheritAndBind<JSViewAbstract>(object, ConstructorCallback, DestructorCallback);
}

void JSViewPartialUpdate::ConstructorCallback(const JSCallbackInfo& info)
{
    LOGD("creating C++ and JS View Objects ...");
    JSRef<JSObject> thisObj = info.This();

    // Get js view name by this.constructor.name
    JSRef<JSObject> constructor = thisObj->GetProperty("constructor");
    JSRef<JSVal> jsViewName = constructor->GetProperty("name");
    auto viewName = jsViewName->ToString();
    auto* instance = new JSViewPartialUpdate(thisObj);

    auto context = info.GetExecutionContext();
    instance->SetContext(context);
    instance->SetJSViewName(viewName);

    //  The JS object owns the C++ object:
    // make sure the C++ is not destroyed when RefPtr thisObj goes out of scope
    // JSView::DestructorCallback has view->DecRefCount()
    instance->IncRefCount();

    info.SetReturnValue(instance);
}

void JSViewPartialUpdate::DestructorCallback(JSViewPartialUpdate* view)
{
    if (view == nullptr) {
        LOGE("JSViewPartialUpdate::DestructorCallback failed: the view is nullptr");
        return;
    }
    LOGD("JSViewPartialUpdate(DestructorCallback) start");
    view->DecRefCount();
    LOGD("JSViewPartialUpdate(DestructorCallback) end");
}

// ===========================================================
// partial update own functions start below
// ===========================================================

void JSViewPartialUpdate::JsFinishUpdateFunc(int32_t elmtId)
{
    ViewPartialUpdateModel::GetInstance()->FinishUpdate(
        viewNode_, elmtId, [weak = AceType::WeakClaim(this)](const UpdateTask& task) {
            auto jsView = weak.Upgrade();
            if (jsView) {
                jsView->pendingUpdateTasks_.push_back(task);
            }
        });
}

bool JSViewPartialUpdate::JsElementIdExists(int32_t elmtId)
{
    return ElementRegister::GetInstance()->Exists(elmtId);
}

void JSViewPartialUpdate::JSGetProxiedItemRenderState(const JSCallbackInfo& info)
{
    if (info.Length() != 1) {
        LOGE("JSView::JSGetProxiedItemRenderState. elmtId parameter expected");
        info.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(false)));
        return;
    }
    const auto elmtId = info[0]->ToNumber<int32_t>();

    if (elmtId == ElementRegister::UndefinedElementId) {
        LOGE("JSView::JSGetProxiedItemRenderState. elmtId must not be undefined");
        info.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(false)));
        return;
    }

    // TODO: Check this return value
    auto result = false;

    // set boolean return value to JS
    info.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
}

void JSViewPartialUpdate::IsFirstRender(const JSCallbackInfo& info)
{
    info.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(isFirstRender_)));
}

void JSViewPartialUpdate::FindChildByIdForPreview(const JSCallbackInfo& info)
{
    LOGD("JSViewPartialUpdate::FindChildByIdForPreview");
    if (!info[0]->IsNumber()) {
        LOGE("info[0] is not a number");
        return;
    }
    std::string viewId = std::to_string(info[0]->ToNumber<int32_t>());
    JSRef<JSObject> targetView = Framework::JSViewStackProcessor::GetViewById(viewId);
    info.SetReturnValue(targetView);
    return;
}

void JSViewPartialUpdate::JsInvalidateLayout()
{
    ACE_FUNCTION_TRACE();
    ViewPartialUpdateModel::GetInstance()->InvalidateLayout(viewNode_);
}
} // namespace OHOS::Ace::Framework
