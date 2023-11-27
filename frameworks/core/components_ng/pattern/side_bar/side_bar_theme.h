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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SIDE_BAR_SIDE_BAR_THEME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SIDE_BAR_SIDE_BAR_THEME_H

#include "core/components/common/properties/color.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_constants.h"

namespace OHOS::Ace::NG {
/**
 * SideBarTheme defines color and styles of SideBarPattern. SideBarTheme should be built
 * using SideBarTheme::Builder.
 */
class SideBarTheme : public virtual Theme {
    DECLARE_ACE_TYPE(SideBarTheme, Theme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<SideBarTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<SideBarTheme> theme = AceType::Claim(new SideBarTheme());
            if (!themeConstants) {
                return theme;
            }

            ParsePattern(themeConstants->GetThemeStyle(), theme);
            return theme;
        }

    private:
        void ParsePattern(const RefPtr<ThemeStyle>& themeStyle, const RefPtr<SideBarTheme>& theme) const
        {
            if (!themeStyle) {
                LOGE("themeStyle is null");
                return;
            }

            auto sideBarPattern = themeStyle->GetAttr<RefPtr<ThemeStyle>>(THEME_PATTERN_SIDE_BAR, nullptr);
            if (!sideBarPattern) {
                LOGE("sideBarPattern is null");
                return;
            }

            theme->controlImageColor_ = sideBarPattern->GetAttr<Color>("control_image_color", Color::BLACK);
            theme->sideBarBackgroundColor_ = sideBarPattern->GetAttr<Color>("side_bar_background_color", Color::WHITE);
            theme->controlButtonRadius_ = sideBarPattern->GetAttr<Dimension>("control_button_radius", 0.0_vp);
            auto dividerShadowEnable = sideBarPattern->GetAttr<std::string>("divider_shadow_enable", "0");
            theme->dividerShadowEnable_ = StringUtils::StringToInt(dividerShadowEnable);

            auto sideBarUnfocusEffectEnable
                = sideBarPattern->GetAttr<std::string>("section_unfocus_effect_enable", "0");
            theme->sideBarUnfocusEffectEnable_ = StringUtils::StringToInt(sideBarUnfocusEffectEnable);
            theme->sideBarUnfocusColor_ = sideBarPattern->GetAttr<Color>("section_unfocus_color", Color::TRANSPARENT);
        }
    };

    ~SideBarTheme() override = default;

    const Color& GetControlImageColor() const
    {
        return controlImageColor_;
    }

    const Color& GetSideBarBackgroundColor() const
    {
        return sideBarBackgroundColor_;
    }

    const Dimension& GetControlButtonRadius() const
    {
        return controlButtonRadius_;
    }

    uint32_t GetDividerShadowEnable() const
    {
        return dividerShadowEnable_;
    }

    const uint32_t& GetSideBarUnfocusEffectEnable() const
    {
        return sideBarUnfocusEffectEnable_;
    }

    const Color& GetSideBarUnfocusColor() const
    {
        return sideBarUnfocusColor_;
    }

protected:
    SideBarTheme() = default;

private:
    Color controlImageColor_ = Color::BLACK;
    Color sideBarBackgroundColor_ = Color::WHITE;
    Dimension controlButtonRadius_;
    uint32_t dividerShadowEnable_ = 0;
    uint32_t sideBarUnfocusEffectEnable_ = 0;
    Color sideBarUnfocusColor_ = Color::TRANSPARENT;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SIDE_BAR_SIDE_BAR_THEME_H
