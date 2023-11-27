/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/list/list_position_controller.h"

#include "core/components_ng/pattern/list/list_pattern.h"

namespace OHOS::Ace::NG {

Rect ListPositionController::GetItemRectInGroup(int32_t index, int32_t indexInGroup) const
{
    auto listPattern = AceType::DynamicCast<ListPattern>(scroll_.Upgrade());
    CHECK_NULL_RETURN(listPattern, Rect());
    return listPattern->GetItemRectInGroup(index, indexInGroup);
}

void ListPositionController::JumpToItemInGroup(int32_t index, int32_t indexInGroup, bool smooth,
    ScrollAlign align, int32_t source)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    auto listPattern = AceType::DynamicCast<ListPattern>(pattern);
    CHECK_NULL_VOID(listPattern);
    if (align == ScrollAlign::NONE) {
        align = ScrollAlign::START;
    }
    listPattern->ScrollToItemInGroup(index, indexInGroup, smooth, align);
}

void ListPositionController::CloseAllSwipeActions(OnFinishFunc&& onFinishCallback)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    auto listPattern = AceType::DynamicCast<ListPattern>(pattern);
    CHECK_NULL_VOID(listPattern);
    return listPattern->CloseAllSwipeActions(std::move(onFinishCallback));
}

} // namespace OHOS::Ace::NG
