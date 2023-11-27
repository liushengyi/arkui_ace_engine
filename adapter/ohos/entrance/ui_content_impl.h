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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_ACE_UI_CONTENT_IMPL_H
#define FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_ACE_UI_CONTENT_IMPL_H

#include <list>

#include "ability_info.h"
#include "interfaces/inner_api/ace/ui_content.h"
#include "interfaces/inner_api/ace/viewport_config.h"
#include "key_event.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "wm/window.h"

#include "adapter/ohos/entrance/distributed_ui_manager.h"
#include "base/view_data/view_data_wrap.h"
#include "core/common/flutter/flutter_asset_manager.h"

namespace OHOS::Accessibility {
class AccessibilityElementInfo;
}

namespace OHOS::Ace {
class ACE_FORCE_EXPORT UIContentImpl : public UIContent {
public:
    UIContentImpl(OHOS::AbilityRuntime::Context* context, void* runtime);
    UIContentImpl(OHOS::AppExecFwk::Ability* ability);
    UIContentImpl(OHOS::AbilityRuntime::Context* context, void* runtime, bool isCard);
    ~UIContentImpl()
    {
        DestroyUIDirector();
        DestroyCallback();
    }

    // UI content lifeCycles
    void Initialize(OHOS::Rosen::Window* window, const std::string& url, napi_value storage) override;
    void Initialize(OHOS::Rosen::Window* window,
        const std::shared_ptr<std::vector<uint8_t>>& content, napi_value storage) override;
    void InitializeByName(OHOS::Rosen::Window* window, const std::string& name, napi_value storage) override;
    void Initialize(
        OHOS::Rosen::Window* window, const std::string& url, napi_value storage, uint32_t focusWindowId) override;
    void Foreground() override;
    void Background() override;
    void Focus() override;
    void UnFocus() override;
    void Destroy() override;
    void OnNewWant(const OHOS::AAFwk::Want& want) override;

    // distribute
    void Restore(OHOS::Rosen::Window* window, const std::string& contentInfo, napi_value storage) override;
    std::string GetContentInfo() const override;
    void DestroyUIDirector() override;

    // UI content event process
    bool ProcessBackPressed() override;
    bool ProcessPointerEvent(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent) override;
    bool ProcessKeyEvent(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent) override;
    bool ProcessAxisEvent(const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent) override;
    bool ProcessVsyncEvent(uint64_t timeStampNanos) override;
    void UpdateConfiguration(const std::shared_ptr<OHOS::AppExecFwk::Configuration>& config) override;
    void UpdateViewportConfig(const ViewportConfig& config, OHOS::Rosen::WindowSizeChangeReason reason,
        const std::shared_ptr<OHOS::Rosen::RSTransaction>& rsTransaction = nullptr) override;
    void UpdateWindowMode(OHOS::Rosen::WindowMode mode, bool hasDeco = true) override;
    void HideWindowTitleButton(bool hideSplit, bool hideMaximize, bool hideMinimize) override;
    void SetIgnoreViewSafeArea(bool ignoreViewSafeArea) override;
    void UpdateMaximizeMode(OHOS::Rosen::MaximizeMode mode) override;
    void ProcessFormVisibleChange(bool isVisible) override;
    void UpdateTitleInTargetPos(bool isShow, int32_t height) override;

    // Window color
    uint32_t GetBackgroundColor() override;
    void SetBackgroundColor(uint32_t color) override;

    bool NeedSoftKeyboard() override;

    void SetOnWindowFocused(const std::function<void()>& callback) override;

    void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override;

    // Set UIContent callback for custom window animation
    void SetNextFrameLayoutCallback(std::function<void()>&& callback) override;

    // Set UIContent callback after layout finish
    void SetFrameLayoutFinishCallback(std::function<void()>&& callback) override;

    // Receive memory level notification
    void NotifyMemoryLevel(int32_t level) override;

    void SetAppWindowTitle(const std::string& title) override;
    void SetAppWindowIcon(const std::shared_ptr<Media::PixelMap>& pixelMap) override;

    // ArkTS Form
    std::shared_ptr<Rosen::RSSurfaceNode> GetFormRootNode() override;
    void UpdateFormData(const std::string& data) override;
    void UpdateFormSharedImage(const std::map<std::string, sptr<OHOS::AppExecFwk::FormAshmem>>& imageDataMap) override;
    void ReloadForm(const std::string& url) override;

    void SetFormWidth(float width) override
    {
        formWidth_ = width;
    }
    void SetFormHeight(float height) override
    {
        formHeight_ = height;
    }
    float GetFormWidth() override
    {
        return formWidth_;
    }
    float GetFormHeight() override
    {
        return formHeight_;
    }

    void SetActionEventHandler(std::function<void(const std::string& action)>&& actionCallback) override;
    void SetErrorEventHandler(std::function<void(const std::string&, const std::string&)>&& errorCallback) override;
    void SetFormLinkInfoUpdateHandler(std::function<void(const std::vector<std::string>&)>&& callback) override;

    void OnFormSurfaceChange(float width, float height) override;

    void SetFormBackgroundColor(const std::string& color) override;

    SerializeableObjectArray DumpUITree() override
    {
        CHECK_NULL_RETURN(uiManager_, SerializeableObjectArray());
        return uiManager_->DumpUITree();
    }
    void SubscribeUpdate(const std::function<void(int32_t, SerializeableObjectArray&)>& onUpdate) override
    {
        CHECK_NULL_VOID(uiManager_);
        return uiManager_->SubscribeUpdate(onUpdate);
    }
    void UnSubscribeUpdate() override
    {
        CHECK_NULL_VOID(uiManager_);
        uiManager_->UnSubscribeUpdate();
    }
    void ProcessSerializeableInputEvent(const SerializeableObjectArray& array) override
    {
        CHECK_NULL_VOID(uiManager_);
        uiManager_->ProcessSerializeableInputEvent(array);
    }
    void RestoreUITree(const SerializeableObjectArray& array) override
    {
        CHECK_NULL_VOID(uiManager_);
        uiManager_->RestoreUITree(array);
    }
    void UpdateUITree(const SerializeableObjectArray& array) override
    {
        CHECK_NULL_VOID(uiManager_);
        uiManager_->UpdateUITree(array);
    }
    void SubscribeInputEventProcess(const std::function<void(SerializeableObjectArray&)>& onEvent) override
    {
        CHECK_NULL_VOID(uiManager_);
        uiManager_->SubscribeInputEventProcess(onEvent);
    }
    void UnSubscribeInputEventProcess() override
    {
        CHECK_NULL_VOID(uiManager_);
        uiManager_->UnSubscribeInputEventProcess();
    }
    void GetResourcePaths(std::vector<std::string>& resourcesPaths, std::string& assetRootPath,
        std::vector<std::string>& assetBasePaths, std::string& resFolderName) override;
    void SetResourcePaths(const std::vector<std::string>& resourcesPaths, const std::string& assetRootPath,
        const std::vector<std::string>& assetBasePaths) override;

    napi_value GetUINapiContext() override;
    void SetIsFocusActive(bool isFocusActive) override;

    void UpdateResource() override;

    int32_t CreateModalUIExtension(const AAFwk::Want& want,
        const ModalUIExtensionCallbacks& callbacks, const ModalUIExtensionConfig& config) override;
    void CloseModalUIExtension(int32_t sessionId) override;

    void SetParentToken(sptr<IRemoteObject> token) override;
    sptr<IRemoteObject> GetParentToken() override;
    bool DumpViewData(AbilityBase::ViewData& viewData) override;
    bool CheckNeedAutoSave() override;
    bool DumpViewData(const RefPtr<NG::FrameNode>& node, RefPtr<ViewDataWrap> viewDataWrap);

    void SearchElementInfoByAccessibilityId(
        int32_t elementId, int32_t mode,
        int32_t baseParent, std::list<Accessibility::AccessibilityElementInfo>& output) override;

    void SearchElementInfosByText(
        int32_t elementId, const std::string& text, int32_t baseParent,
        std::list<Accessibility::AccessibilityElementInfo>& output) override;

    void FindFocusedElementInfo(
        int32_t elementId, int32_t focusType,
        int32_t baseParent, Accessibility::AccessibilityElementInfo& output) override;

    void FocusMoveSearch(
        int32_t elementId, int32_t direction,
        int32_t baseParent, Accessibility::AccessibilityElementInfo& output) override;

    bool NotifyExecuteAction(int32_t elementId, const std::map<std::string, std::string>& actionArguments,
        int32_t action, int32_t offset) override;
    
private:
    void InitializeInner(
        OHOS::Rosen::Window* window, const std::string& contentInfo, napi_value storage, bool isNamedRouter);
    void CommonInitialize(OHOS::Rosen::Window* window, const std::string& contentInfo, napi_value storage);
    void CommonInitializeForm(OHOS::Rosen::Window* window, const std::string& contentInfo, napi_value storage);
    void InitializeSubWindow(OHOS::Rosen::Window* window, bool isDialog = false);
    void DestroyCallback() const;
    void SetConfiguration(const std::shared_ptr<OHOS::AppExecFwk::Configuration>& config);

    void InitializeSafeArea(const RefPtr<Platform::AceContainer>& container);

    std::weak_ptr<OHOS::AbilityRuntime::Context> context_;
    void* runtime_ = nullptr;
    OHOS::Rosen::Window* window_ = nullptr;
    std::string startUrl_;
    int32_t instanceId_ = -1;
    OHOS::sptr<OHOS::Rosen::IWindowDragListener> dragWindowListener_ = nullptr;
    OHOS::sptr<OHOS::Rosen::IOccupiedAreaChangeListener> occupiedAreaChangeListener_ = nullptr;
    OHOS::sptr<OHOS::Rosen::IAvoidAreaChangedListener> avoidAreaChangedListener_ = nullptr;

    // ITouchOutsideListener is used for touching out of hot areas of window.
    OHOS::sptr<OHOS::Rosen::ITouchOutsideListener> touchOutsideListener_ = nullptr;

    // ArkTS Form
    bool isFormRender_ = false;
    bool isFormRenderInit_ = false;
    std::string bundleName_;
    std::string moduleName_;
    std::string hapPath_;
    bool isBundle_ = false;
    float formWidth_ = 0.0;
    float formHeight_ = 0.0;
    std::string formData_;
    std::map<std::string, sptr<OHOS::AppExecFwk::FormAshmem>> formImageDataMap_;
    std::unique_ptr<DistributedUIManager> uiManager_;

    sptr<IRemoteObject> parentToken_ = nullptr;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_OHOS_ENTRANCE_ACE_UI_CONTENT_IMPL_H
