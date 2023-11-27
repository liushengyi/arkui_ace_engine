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
#include "core/components_ng/pattern/common_view/common_view_model_ng.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/common_view/common_view_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
void CommonViewModelNG::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    ACE_SCOPED_TRACE("Create[%s][self:%d]", V2::COMMON_VIEW_ETS_TAG, nodeId);
    auto blankNode = FrameNode::GetOrCreateFrameNode(
        V2::COMMON_VIEW_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<CommonViewPattern>(); });
    blankNode->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);
    stack->Push(blankNode);
}
} // namespace OHOS::Ace::NG
