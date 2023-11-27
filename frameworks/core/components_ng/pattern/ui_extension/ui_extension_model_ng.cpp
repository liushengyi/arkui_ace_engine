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

#include "core/components_ng/pattern/ui_extension/ui_extension_model_ng.h"

#include "want.h"

#include "interfaces/inner_api/ace/modal_ui_extension_config.h"

#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
RefPtr<FrameNode> UIExtensionModelNG::Create(const std::string& bundleName, const std::string& abilityName,
    const std::map<std::string, std::string>& params, std::function<void(int32_t)>&& onRelease,
    std::function<void(int32_t, const std::string&, const std::string&)>&& onError)
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    ACE_SCOPED_TRACE("Create[%s][self:%d]", V2::UI_EXTENSION_COMPONENT_ETS_TAG, nodeId);
    auto wantWrap = WantWrap::CreateWantWrap(bundleName, abilityName);
    wantWrap->SetWantParam(params);
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::UI_EXTENSION_COMPONENT_ETS_TAG, nodeId,
        []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    CHECK_NULL_RETURN(pattern, frameNode);
    pattern->UpdateWant(wantWrap);
    pattern->SetOnReleaseCallback(std::move(onRelease));
    pattern->SetOnErrorCallback(std::move(onError));
    return frameNode;
}

RefPtr<FrameNode> UIExtensionModelNG::Create(const AAFwk::Want& want, const ModalUIExtensionCallbacks& callbacks)
{
    auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    CHECK_NULL_RETURN(pattern, frameNode);
    pattern->SetModalFlag(true);
    pattern->UpdateWant(want);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, frameNode);
    pipeline->AddWindowStateChangedCallback(nodeId);
    pattern->SetOnReleaseCallback(std::move(callbacks.onRelease));
    pattern->SetOnErrorCallback(std::move(callbacks.onError));
    pattern->SetOnResultCallback(std::move(callbacks.onResult));
    pattern->SetOnReceiveCallback(std::move(callbacks.onReceive));
    pattern->SetModalOnRemoteReadyCallback(std::move(callbacks.onRemoteReady));
    pattern->SetModalOnDestroy(std::move(callbacks.onDestroy));
    return frameNode;
}

void UIExtensionModelNG::Create(const RefPtr<OHOS::Ace::WantWrap>& wantWrap, bool transferringCaller)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::UI_EXTENSION_COMPONENT_ETS_TAG, nodeId,
        []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetTransferringCaller(transferringCaller);
    pattern->UpdateWant(wantWrap);
    stack->Push(frameNode);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddWindowStateChangedCallback(nodeId);
    auto dragDropManager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(dragDropManager);
    dragDropManager->AddDragFrameNode(nodeId, AceType::WeakClaim(AceType::RawPtr(frameNode)));
}

void UIExtensionModelNG::SetOnRemoteReady(std::function<void(const RefPtr<UIExtensionProxy>&)>&& onRemoteReady)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOnRemoteReadyCallback(std::move(onRemoteReady));
}

void UIExtensionModelNG::SetOnRelease(std::function<void(int32_t)>&& onRelease)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOnReleaseCallback(std::move(onRelease));
}

void UIExtensionModelNG::SetOnResult(std::function<void(int32_t, const AAFwk::Want&)>&& onResult)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    pattern->SetOnResultCallback(std::move(onResult));
}

void UIExtensionModelNG::SetOnReceive(std::function<void(const AAFwk::WantParams&)>&& onReceive)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    pattern->SetOnReceiveCallback(std::move(onReceive));
}

void UIExtensionModelNG::SetOnError(
    std::function<void(int32_t code, const std::string& name, const std::string& message)>&& onError)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<UIExtensionPattern>();
    pattern->SetOnErrorCallback(std::move(onError));
}
} // namespace OHOS::Ace::NG
