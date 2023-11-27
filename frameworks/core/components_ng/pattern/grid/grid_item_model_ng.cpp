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

#include "core/components_ng/pattern/grid/grid_item_model_ng.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void GridItemModelNG::Create(GridItemStyle gridItemStyle)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_SCOPED_TRACE("Create[%s][self:%d]", V2::GRID_ITEM_ETS_TAG, nodeId);
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::GRID_ITEM_ETS_TAG, nodeId,
        [itemStyle = gridItemStyle]() { return AceType::MakeRefPtr<GridItemPattern>(nullptr, itemStyle); });
    stack->Push(frameNode);
}

void GridItemModelNG::Create(std::function<void(int32_t)>&& deepRenderFunc, bool isLazy, GridItemStyle gridItemStyle)
{
    if (!isLazy) {
        Create(gridItemStyle);
        return;
    }

    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto deepRender = [nodeId, deepRenderFunc = std::move(deepRenderFunc)]() -> RefPtr<UINode> {
        CHECK_NULL_RETURN(deepRenderFunc, nullptr);
        ScopedViewStackProcessor scopedViewStackProcessor;
        deepRenderFunc(nodeId);
        return ViewStackProcessor::GetInstance()->Finish();
    };
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::GRID_ITEM_ETS_TAG, nodeId,
        [shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(std::move(deepRender)), itemStyle = gridItemStyle]() {
            return AceType::MakeRefPtr<GridItemPattern>(shallowBuilder, itemStyle);
        });
    stack->Push(frameNode);
}

void GridItemModelNG::SetRowStart(int32_t value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(GridItemLayoutProperty, RowStart, value);
}

void GridItemModelNG::SetRowEnd(int32_t value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(GridItemLayoutProperty, RowEnd, value);
}

void GridItemModelNG::SetColumnStart(int32_t value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(GridItemLayoutProperty, ColumnStart, value);
}

void GridItemModelNG::SetColumnEnd(int32_t value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(GridItemLayoutProperty, ColumnEnd, value);
}

void GridItemModelNG::SetForceRebuild(bool value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<GridItemPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetForceRebuild(value);
}

void GridItemModelNG::SetSelectable(bool value)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<GridItemPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectable(value);
}

void GridItemModelNG::SetSelected(bool selected)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<GridItemPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelected(selected);
    auto eventHub = frameNode->GetEventHub<GridItemEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCurrentUIState(UI_STATE_SELECTED, selected);
}

void GridItemModelNG::SetSelectChangeEvent(std::function<void(bool)>&& changeEvent)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<GridItemEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetSelectChangeEvent(std::move(changeEvent));
}

void GridItemModelNG::SetOnSelect(SelectFunc&& onSelect)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<GridItemEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnSelect(std::move(onSelect));
}

} // namespace OHOS::Ace::NG
