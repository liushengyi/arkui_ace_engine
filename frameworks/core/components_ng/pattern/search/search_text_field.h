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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SEARCH_SEARCH_TEXT_FIELD_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SEARCH_SEARCH_TEXT_FIELD_PATTERN_H

#include "core/components_ng/pattern/text_field/text_field_pattern.h"

namespace OHOS::Ace::NG {
class SearchTextFieldPattern final : public TextFieldPattern {
    DECLARE_ACE_TYPE(SearchTextFieldPattern, TextFieldPattern);

public:
    SearchTextFieldPattern() = default;
    ~SearchTextFieldPattern() override = default;

    RefPtr<FocusHub> GetFocusHub() const override;
    bool IsSearchParentNode() const;
    void PerformAction(TextInputAction action, bool forceCloseKeyboard = true) override;
    TextInputAction GetDefaultTextInputAction() override;
#ifdef ENABLE_DRAG_FRAMEWORK
    void InitDragEvent() override;
#endif
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SEARCH_SEARCH_TEXT_FIELD_PATTERN_H