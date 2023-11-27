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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_SUBWINDOW_SUBWINDOW_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_SUBWINDOW_SUBWINDOW_MANAGER_H

#include <mutex>
#include <set>
#include <unordered_map>

#include "base/memory/referenced.h"
#include "base/subwindow/subwindow.h"
#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"

namespace OHOS::Ace {

using SubwindowMap = std::unordered_map<int32_t, RefPtr<Subwindow>>;

class ACE_FORCE_EXPORT SubwindowManager final : public NonCopyable {
public:
    // Get the instance
    static std::shared_ptr<SubwindowManager> GetInstance();

    void AddContainerId(uint32_t windowId, int32_t containerId);
    void RemoveContainerId(uint32_t windowId);
    int32_t GetContainerId(uint32_t windowId);

    void AddParentContainerId(int32_t containerId, int32_t parentContainerId);
    void RemoveParentContainerId(int32_t containerId);
    int32_t GetParentContainerId(int32_t containerId);

    void AddSubwindow(int32_t instanceId, RefPtr<Subwindow>);
    void RemoveSubwindow(int32_t instanceId);

    // Get the subwindow of instance, return the window or nullptr.
    const RefPtr<Subwindow> GetSubwindow(int32_t instanceId);

    void HideCurrentSubwindow();

    void SetCurrentSubwindowName(const std::string& currentSubwindow);
    std::string GetCurrentSubWindowName();

    void SetCurrentSubwindow(const RefPtr<Subwindow>& subwindow);

    const RefPtr<Subwindow>& GetCurrentWindow();
    Rect GetParentWindowRect();

    void ShowMenu(const RefPtr<Component>& newComponent);
    void ShowMenuNG(
        const RefPtr<NG::FrameNode>& menuNode, int32_t targetId, const NG::OffsetF& offset, bool isAboveApps = false);
    void HideMenuNG(const RefPtr<NG::FrameNode>& menu, int32_t targetId);
    void HideMenuNG(bool showPreviewAnimation = true, bool startDrag = false);
    void ShowPopup(const RefPtr<Component>& newComponent, bool disableTouchEvent = true);
    void ShowPopupNG(int32_t targetId, const NG::PopupInfo& popupInfo);
    void HidePopupNG(int32_t targetId, int32_t instanceId = -1);
    bool CancelPopup(const std::string& id);
    void CloseMenu();
    void ClearMenu();
    void ClearMenuNG(int32_t instanceId = -1, bool inWindow = true, bool showAnimation = false);
    void ClearPopupInSubwindow(int32_t instanceId = -1);
    RefPtr<NG::FrameNode> ShowDialogNG(const DialogProperties& dialogProps, std::function<void()>&& buildFunc);
    void HideSubWindowNG();
    void HideDialogSubWindow(int32_t instanceId);
    void SetDialogHotAreas(const std::vector<Rect>& rects, int32_t overlayId, int32_t instanceId);
    void SetHotAreas(const std::vector<Rect>& rects, int32_t overlayId = -1, int32_t instanceId = -1);
    int32_t GetDialogSubWindowId()
    {
        return dialogSubWindowId_;
    }
    void SetDialogSubWindowId(int32_t dialogSubWindowId)
    {
        dialogSubWindowId_ = dialogSubWindowId;
    }
    void AddDialogSubwindow(int32_t instanceId, const RefPtr<Subwindow>& subwindow);
    // Get the dialog subwindow of instance, return the window or nullptr.
    int32_t GetDialogSubwindowInstanceId(int32_t SubwindowId);
    const RefPtr<Subwindow> GetDialogSubwindow(int32_t instanceId);
    void SetCurrentDialogSubwindow(const RefPtr<Subwindow>& subwindow);
    const RefPtr<Subwindow>& GetCurrentDialogWindow();
    void DeleteHotAreas(int32_t subwindowid, int32_t overlayid);

    void ClearToastInSubwindow();
    void ShowToast(
        const std::string& message, int32_t duration, const std::string& bottom, const NG::ToastShowMode& showMode);
    void ShowDialog(const std::string& title, const std::string& message, const std::vector<ButtonInfo>& buttons,
        bool autoCancel, std::function<void(int32_t, int32_t)>&& napiCallback,
        const std::set<std::string>& dialogCallbacks);
    void ShowDialog(const PromptDialogAttr& dialogAttr, const std::vector<ButtonInfo>& buttons,
        std::function<void(int32_t, int32_t)>&& napiCallback, const std::set<std::string>& dialogCallbacks);
    void ShowActionMenu(const std::string& title, const std::vector<ButtonInfo>& button,
        std::function<void(int32_t, int32_t)>&& callback);
    void CloseDialog(int32_t instanceId);
    void RequestFocusSubwindow(int32_t instanceId);
    
    bool GetShown();

private:
    RefPtr<Subwindow> GetOrCreateSubWindow();

    static std::mutex instanceMutex_;
    static std::shared_ptr<SubwindowManager> instance_;

    std::mutex mutex_;
    std::unordered_map<uint32_t, int32_t> containerMap_;

    std::mutex parentMutex_;
    std::unordered_map<int32_t, int32_t> parentContainerMap_;

    // Used to save the relationship between container and subwindow, it is 1:1
    std::mutex subwindowMutex_;
    SubwindowMap subwindowMap_;
    int32_t dialogSubWindowId_;
    std::mutex currentSubwindowMutex_;
    std::string currentSubwindowName_;

    RefPtr<Subwindow> currentSubwindow_;

    // Used to save the relationship between container and dialog subwindow, it is 1:1
    std::mutex dialogSubwindowMutex_;
    SubwindowMap dialogSubwindowMap_;
    std::mutex currentDialogSubwindowMutex_;
    RefPtr<Subwindow> currentDialogSubwindow_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_SUBWINDOW_SUBWINDOW_MANAGER_H
