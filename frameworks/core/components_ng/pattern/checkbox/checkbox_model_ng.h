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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_MODEL_NG_H

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/checkbox/checkbox_model.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT CheckBoxModelNG : public OHOS::Ace::CheckBoxModel {
public:
    void Create(const std::optional<std::string>& name, const std::optional<std::string>& groupName,
        const std::string& tagName) override;
    void SetSelect(bool isSelected) override;
    void SetSelectedColor(const Color& color) override;
    void SetUnSelectedColor(const Color& color) override;
    void SetCheckMarkColor(const Color& color) override;
    void SetCheckMarkSize(const Dimension& size) override;
    void SetCheckMarkWidth(const Dimension& width) override;
    void SetOnChange(ChangeEvent&& onChange) override;
    void SetWidth(const Dimension& width) override;
    void SetHeight(const Dimension& height) override;
    void SetPadding(const NG::PaddingPropertyF& args, const NG::PaddingProperty& newArgs, bool flag) override;
    void SetChangeEvent(ChangeEvent&& changeEvent) override;
    void SetResponseRegion(const std::vector<DimensionRect>& responseRegion) override;
    void SetCheckboxStyle(CheckBoxStyle checkboxStyle) override;
    
    static void SetSelect(FrameNode* frameNode, bool isSelected);
    static void SetSelectedColor(FrameNode* frameNode, const Color& color);
    static void SetUnSelectedColor(FrameNode* frameNode, const Color& color);
    static void SetWidth(FrameNode* frameNode, const Dimension& width);
    static void SetHeight(FrameNode* frameNode, const Dimension& height);
    static void SetCheckMarkColor(FrameNode* frameNode, const Color& color);
    static void SetCheckMarkSize(FrameNode* frameNode, const Dimension& size);
    static void SetCheckMarkWidth(FrameNode* frameNode, const Dimension& width);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_MODEL_NG_H
