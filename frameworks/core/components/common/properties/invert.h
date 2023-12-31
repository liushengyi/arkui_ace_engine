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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_PROPERTIES_INVERT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_PROPERTIES_INVERT_H

#include <variant>

namespace OHOS::Ace {
struct InvertOption final {
    float low_ = 0.0f;
    float high_ = 0.0f;
    float threshold_ = 0.0f;
    float thresholdRange_ = 0.0f;

    bool operator==(const InvertOption& opt) const
    {
        return NearEqual(high_, opt.high_) && NearEqual(low_, opt.high_) && NearEqual(threshold_, opt.threshold_) &&
               NearEqual(thresholdRange_, opt.thresholdRange_);
    }
};
using InvertVariant = std::variant<float, InvertOption>;
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_PROPERTIES_INVERT_H
