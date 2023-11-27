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

#define private public
#define protected public
#include "base/memory/ace_type.h"
#include "core/components/theme/icon_theme.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_manager.h"

namespace OHOS::Ace {

RefPtr<IconTheme> IconTheme::Builder::Build(const RefPtr<ThemeConstants>&  /* themeConstants */) const
{
    return nullptr;
}

void IconTheme::Builder::ParsePattern(const RefPtr<ThemeStyle>& themeStyle, const RefPtr<IconTheme>& theme) const {}

RefPtr<Theme> ThemeManager::GetTheme(ThemeType /* type */)
{
    return AceType::MakeRefPtr<IconTheme>();
}

std::string IconTheme::GetIconPath(const InternalResource::ResourceId& resourceId) const
{
    if (resourceId == InternalResource::ResourceId::NO_ID) {
        return std::string();
    }
    return "resource:///ohos_test_image.svg";
}

} // namespace OHOS::Ace
