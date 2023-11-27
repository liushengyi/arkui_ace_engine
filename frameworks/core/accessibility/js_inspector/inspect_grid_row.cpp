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

#include "inspect_grid_row.h"

#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::Framework {
InspectGridRow::InspectGridRow(NodeId nodeId, const std::string& nodeName) : InspectNode(nodeId, nodeName) {
}

void InspectGridRow::PackAttrAndStyle()
{
    // add for the attrs
    attrs_.insert(std::make_pair("disabled", "false"));
    attrs_.insert(std::make_pair("focusable", "false"));

    // add for the styles
    styles_.insert(std::make_pair("flex-wrap", "nowrap"));
    styles_.insert(std::make_pair("justify-content", "flex-start"));
    styles_.insert(std::make_pair("align-items", "flex-start"));
    styles_.insert(std::make_pair("align-content", "flex-start"));
    styles_.erase("padding-left");
    styles_.erase("padding-top");
    styles_.erase("padding-right");
    styles_.erase("padding-bottom");
    styles_.erase("padding-start");
    styles_.erase("padding-end");
    styles_.erase("margin-left");
    styles_.erase("margin-top");
    styles_.erase("margin-right");
    styles_.erase("margin-bottom");
    styles_.erase("margin-start");
    styles_.erase("margin-end");
    styles_.erase("flex-grow");
    styles_.erase("flex-shrink");
    styles_.erase("position");
}
} // namespace OHOS::Ace::Framework
