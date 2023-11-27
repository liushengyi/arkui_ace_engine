/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_text_area_modifier.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/pipeline/base/element_register.h"
namespace OHOS::Ace::NG {
constexpr uint32_t DEFAULT_MAX_VIEW_LINE = 3;
void SetTextAreaMaxLines(NodeHandle node, uint32_t maxLine)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxViewLines(frameNode, maxLine);
}

void ResetTextAreaMaxLines(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxViewLines(frameNode, DEFAULT_MAX_VIEW_LINE);
}

ArkUITextAreaModifierAPI GetTextAreaModifier()
{
    static const ArkUITextAreaModifierAPI modifier = { SetTextAreaMaxLines, ResetTextAreaMaxLines };
    return modifier;
}
} // namespace OHOS::Ace::NG