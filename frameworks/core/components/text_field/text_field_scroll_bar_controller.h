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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FIELD_TEXT_FIELD_SCROLLBAR_CONTROLLER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FIELD_TEXT_FIELD_SCROLLBAR_CONTROLLER_H

#include "core/components/scroll/scroll_bar_controller.h"

namespace OHOS::Ace {
class TextFieldScrollBarController : public ScrollBarController {
    DECLARE_ACE_TYPE(TextFieldScrollBarController, ScrollBarController);

public:
    TextFieldScrollBarController() = default;
    ~TextFieldScrollBarController() override = default;

    void MarkScrollRender() override;
    bool CheckScroll() override;

protected:
    bool UpdateScrollPosition(double offset, int32_t source) override;
    void ProcessScrollMotion(double position) override;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FIELD_TEXT_FIELD_SCROLLBAR_CONTROLLER_H