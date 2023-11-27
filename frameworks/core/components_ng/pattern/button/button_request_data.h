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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BUTTON_BUTTON_REQUEST_DATA_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BUTTON_BUTTON_REQUEST_DATA_H

#include <optional>

#include "base/geometry/dimension.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"

namespace OHOS::Ace {
struct CreateWithPara {
    std::optional<bool> labelSetInfoFirst;
    std::optional<bool> labelSetInfoSecond;
    std::optional<bool> stateEffectFirst;
    std::optional<bool> stateEffectSecond;
    std::optional<bool> parseSuccess;
    std::optional<ButtonType> typeFirst;
    std::optional<ButtonType> typeSecond;
    std::optional<std::string> label;
};

struct ButtonParameters {
    std::optional<Ace::TextOverflow> textOverflow;
    std::optional<uint32_t> maxLines;
    std::optional<Dimension> minFontSize;
    std::optional<Dimension> maxFontSize;
    std::optional<Ace::TextHeightAdaptivePolicy> heightAdaptivePolicy;
    std::optional<Dimension> fontSize;
    std::optional<Ace::FontWeight> fontWeight;
    std::optional<std::vector<std::string>> fontFamily;
    std::optional<Ace::FontStyle> fontStyle;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BUTTON_BUTTON_REQUEST_DATA_H
