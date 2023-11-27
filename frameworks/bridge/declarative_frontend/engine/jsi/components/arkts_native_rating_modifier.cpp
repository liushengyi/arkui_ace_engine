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
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_rating_modifier.h"
#include "core/components_ng/pattern/rating/rating_model_ng.h"
#include "core/pipeline/base/element_register.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components/common/layout/constants.h"

namespace OHOS::Ace::NG {
constexpr int32_t STARS_DEFAULT = 5;
constexpr double STEPS_DEFAULT = 0.5;
void SetStars(NodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RatingModelNG::SetStars(frameNode, value);
}
void SetRatingStepSize(NodeHandle node, double value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RatingModelNG::SetStepSize(frameNode, value);
}
void SetStarStyle(NodeHandle node,
    const char* backgroundUri, const char* foregroundUri, const char* secondaryUri)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RatingModelNG::SetBackgroundSrc(frameNode, backgroundUri, false);
    RatingModelNG::SetForegroundSrc(frameNode, foregroundUri, false);
 
    if (secondaryUri != nullptr && secondaryUri[0] != '\0') {
        RatingModelNG::SetSecondarySrc(frameNode, secondaryUri, false);
    } else if (backgroundUri != nullptr && backgroundUri[0] != '\0') {
        RatingModelNG::SetSecondarySrc(frameNode, backgroundUri, false);
    } else {
        RatingModelNG::SetSecondarySrc(frameNode, "", true);
    }
}

void ResetStars(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RatingModelNG::SetStars(frameNode, STARS_DEFAULT);
}
void ResetRatingStepSize(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RatingModelNG::SetStepSize(frameNode, STEPS_DEFAULT);
}
void ResetStarStyle(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RatingModelNG::SetBackgroundSrc(frameNode, "", true);
    RatingModelNG::SetForegroundSrc(frameNode, "", true);
    RatingModelNG::SetSecondarySrc(frameNode, "", true);
}
ArkUIRatingModifierAPI GetRatingModifier()
{
    static const ArkUIRatingModifierAPI modifier = {SetStars, SetRatingStepSize, SetStarStyle,
        ResetStars, ResetRatingStepSize, ResetStarStyle};
    return modifier;
}
}