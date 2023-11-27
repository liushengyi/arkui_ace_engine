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

#include "core/components_ng/pattern/web/richtext_model_ng.h"

#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/components_ng/pattern/web/web_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void RichTextModelNG::Create(const std::string& webData)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_SCOPED_TRACE("Create[%s][self:%d]", V2::WEB_ETS_TAG, nodeId);
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);

    auto webPattern = frameNode->GetPattern<WebPattern>();
    CHECK_NULL_VOID(webPattern);
    webPattern->SetWebData(webData);
    auto pipeline = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddWindowStateChangedCallback(nodeId);
    pipeline->AddWindowSizeChangeCallback(nodeId);
}

void RichTextModelNG::SetOnPageStart(std::function<void(const BaseEventInfo*)>&& onPageStarted)
{
    auto func = onPageStarted;
    auto onPageStartedEvent = [func](const std::shared_ptr<BaseEventInfo>& info) { func(info.get()); };
    auto webEventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<WebEventHub>();
    CHECK_NULL_VOID(webEventHub);
    webEventHub->SetOnPageStartedEvent(std::move(onPageStartedEvent));
}

void RichTextModelNG::SetOnPageFinish(std::function<void(const BaseEventInfo*)>&& onPageFinish)
{
    auto func = onPageFinish;
    auto onPageFinishEvent = [func](const std::shared_ptr<BaseEventInfo>& info) { func(info.get()); };
    auto webEventHub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<WebEventHub>();
    CHECK_NULL_VOID(webEventHub);
    webEventHub->SetOnPageFinishedEvent(std::move(onPageFinishEvent));
}
} // namespace OHOS::Ace::NG
