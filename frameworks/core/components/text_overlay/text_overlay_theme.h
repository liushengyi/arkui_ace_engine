/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_OVERLAY_TEXT_OVERLAY_THEME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_OVERLAY_TEXT_OVERLAY_THEME_H

#include "base/geometry/dimension.h"
#include "core/components/common/properties/border.h"
#include "core/components/common/properties/border_edge.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/edge.h"
#include "core/components/common/properties/radius.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_constants_defines.h"

namespace OHOS::Ace {

/**
 * DialogTheme defines color and styles of PopupComponent. PopupTheme should be built
 * using DialogTheme::Builder.
 */
class TextOverlayTheme : public virtual Theme {
    DECLARE_ACE_TYPE(TextOverlayTheme, Theme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<TextOverlayTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<TextOverlayTheme> theme = AceType::Claim(new TextOverlayTheme());
            if (!themeConstants) {
                return theme;
            }
            // init theme from global data
            theme->menuBorder_ = Border(BorderEdge(themeConstants->GetColor(THEME_TEXT_OVERLAY_MENU_BORDER_COLOR),
                themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_BORDER_WIDTH),
                BorderStyle(themeConstants->GetInt(THEME_TEXT_OVERLAY_MENU_BORDER_STYLE))));
            theme->menuBackgroundColor_ = themeConstants->GetColor(THEME_TEXT_OVERLAY_MENU_BACKGROUND_COLOR);
            theme->handleColor_ = themeConstants->GetColor(THEME_TEXT_OVERLAY_HANDLE_COLOR);
            theme->handleColorInner_ = themeConstants->GetColor(THEME_TEXT_OVERLAY_HANDLE_COLOR_INNER);
            theme->buttonClickedColor_ = themeConstants->GetColor(THEME_TEXT_OVERLAY_BUTTON_CLICKED_COLOR);
            theme->buttonHoverColor_ = themeConstants->GetColor(THEME_TEXT_OVERLAY_BUTTON_HOVER_COLOR);
            theme->iconColor_ = themeConstants->GetColor(THEME_TEXT_OVERLAY_MENU_ICON_COLOR);
            theme->menuIconColor_ = themeConstants->GetColor(THEME_TEXT_OVERLAY_MENU_ICON_COLOR);
            theme->menuPadding_ = Edge(themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_PADDING_LEFT),
                themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_PADDING_TOP),
                themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_PADDING_RIGHT),
                themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_PADDING_BOTTOM));
            theme->handleDiameter_ = themeConstants->GetDimension(THEME_TEXT_OVERLAY_HANDLE_DIAMETER);
            theme->handleDiameterInner_ = themeConstants->GetDimension(THEME_TEXT_OVERLAY_HANDLE_DIAMETER_INNER);
            theme->menuSpacingWithText_ = themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_SPACING_WITH_TEXT);
            theme->menuButtonWidth_ = themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_BUTTON_WIDTH);
            theme->menuButtonHeight_ = themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_BUTTON_HEIGHT);
            theme->menuButtonTextStyle_.SetFontSize(
                themeConstants->GetDimension(THEME_TEXT_OVERLAY_BUTTON_TEXT_FONTSIZE));
            theme->menuButtonTextStyle_.SetFontWeight(
                FontWeight(themeConstants->GetInt(THEME_TEXT_OVERLAY_BUTTON_TEXT_FONTWEIGHT)));
            theme->menuButtonTextStyle_.SetTextColor(themeConstants->GetColor(THEME_TEXT_OVERLAY_BUTTON_TEXT_COLOR));
            theme->menuButtonPadding_ =
                Edge(themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_BUTTON_PADDING_LEFT).Value(),
                    themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_BUTTON_PADDING_TOP).Value(),
                    themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_BUTTON_PADDING_RIGHT).Value(),
                    themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_BUTTON_PADDING_BOTTOM).Value(),
                    themeConstants->GetDimension(THEME_TEXT_OVERLAY_MENU_PADDING_LEFT).Unit());
            theme->backResourceId_ = themeConstants->GetResourceId(THEME_NAVIGATION_BAR_RESOURCE_ID_BACK);
            theme->moreResourceId_ = themeConstants->GetResourceId(THEME_NAVIGATION_BAR_RESOURCE_ID_MORE);
            ParsePattern(themeConstants->GetThemeStyle(), theme);
            return theme;
        }

    private:
        void ParsePattern(const RefPtr<ThemeStyle>& themeStyle, const RefPtr<TextOverlayTheme>& theme) const
        {
            if (!themeStyle || !theme) {
                return;
            }
            auto pattern = themeStyle->GetAttr<RefPtr<ThemeStyle>>("text_overlay_pattern", nullptr);
            if (pattern) {
                const double defaultTertiaryColorAlpha = 0.4;

                theme->iconColor_ = pattern->GetAttr<Color>("icon_color", Color());
                theme->menuIconColor_ = pattern->GetAttr<Color>("memu_icon_color", Color());
                theme->handleColor_ = pattern->GetAttr<Color>("handle_outer_color", Color());
                theme->handleColorInner_ = pattern->GetAttr<Color>("handle_inner_color", Color());
                theme->menuBackgroundColor_ = pattern->GetAttr<Color>("menu_bg_color", Color());
                theme->buttonHoverColor_ = pattern->GetAttr<Color>("button_bg_color_hovered", Color());
                theme->buttonClickedColor_ = pattern->GetAttr<Color>("button_bg_color_clicked", Color());
                theme->moreOrBackIconColor_ = pattern->GetAttr<Color>("more_or_back_icon_color", Color());
                theme->menuButtonTextStyle_.SetTextColor(pattern->GetAttr<Color>(PATTERN_TEXT_COLOR, Color()));
                theme->menuButtonTextStyle_.SetFontSize(pattern->GetAttr<Dimension>(PATTERN_TEXT_SIZE, 0.0_fp));
                theme->handleDiameter_ = pattern->GetAttr<Dimension>("handle_outer_diameter", 0.0_vp);
                theme->handleDiameterInner_ = pattern->GetAttr<Dimension>("handle_inner_diameter", 0.0_vp);
                theme->moreButtonHeight_ = pattern->GetAttr<Dimension>("more_button_height", 24.0_vp);
                theme->selectOverlayMaxWidth_ = pattern->GetAttr<Dimension>("select_overlay_max_width", 280.0_vp);
                theme->alphaDisabled_ =
                    pattern->GetAttr<double>(PATTERN_BG_COLOR_DISABLED_ALPHA, defaultTertiaryColorAlpha);
                theme->cameraInput_ = pattern->GetAttr<std::string>("camera_input", "Camera input");
            } else {
                LOGW("find pattern of textoverlay fail");
            }
        }
    };

    ~TextOverlayTheme() override = default;

    const Border& GetMenuBorder() const
    {
        return menuBorder_;
    }

    const Color& GetMenuBackgroundColor() const
    {
        return menuBackgroundColor_;
    }

    const Color& GetHandleColor() const
    {
        return handleColor_;
    }

    const Color& GetHandleColorInner() const
    {
        return handleColorInner_;
    }

    const Color& GetButtonClickedColor() const
    {
        return buttonClickedColor_;
    }

    const Color& GetButtonHoverColor() const
    {
        return buttonHoverColor_;
    }

    const Color& GetIconColor() const
    {
        return iconColor_;
    }

    const Color& GetMenuIconColor() const
    {
        return menuIconColor_;
    }

    const Color& GetMoreOrBackIconColor() const
    {
        return moreOrBackIconColor_;
    }

    const Edge& GetMenuPadding() const
    {
        return menuPadding_;
    }

    const Edge& GetMenuButtonPadding() const
    {
        return menuButtonPadding_;
    }

    const Dimension& GetHandleDiameter() const
    {
        return handleDiameter_;
    }

    const Dimension& GetHandleDiameterInner() const
    {
        return handleDiameterInner_;
    }

    const Dimension& GetMenuSpacingWithText() const
    {
        return menuSpacingWithText_;
    }

    const Dimension& GetMenuButtonWidth() const
    {
        return menuButtonWidth_;
    }

    const Dimension& GetMenuButtonHeight() const
    {
        return menuButtonHeight_;
    }

    const Dimension& GetMoreButtonHeight() const
    {
        return moreButtonHeight_;
    }

    const Dimension& GetSelectOverlayMaxWidth() const
    {
        return selectOverlayMaxWidth_;
    }

    const TextStyle& GetMenuButtonTextStyle() const
    {
        return menuButtonTextStyle_;
    }

    Dimension GetMenuToolbarHeight() const
    {
        return 40.0_vp;
    }

    float GetDefaultMenuPositionX()
    {
        return 10.0f;
    }

    Dimension GetHandleLineWidth() const
    {
        return 1.5_vp;
    }

    Dimension GetHandleHotZoneRadius() const
    {
        return handleDiameter_;
    }

    InternalResource::ResourceId GetBackResourceId() const
    {
        return backResourceId_;
    }

    InternalResource::ResourceId GetMoreResourceId() const
    {
        return moreResourceId_;
    }

    double GetAlphaDisabled() const
    {
        return alphaDisabled_;
    }

    const std::string& GetCameraInput() const
    {
        return cameraInput_;
    }

protected:
    TextOverlayTheme() = default;

private:
    Border menuBorder_;
    Color iconColor_;
    Color menuIconColor_;
    Color menuBackgroundColor_;
    Color handleColor_;
    Color handleColorInner_;
    Color buttonClickedColor_;
    Color buttonHoverColor_;
    Color moreOrBackIconColor_;
    Edge menuPadding_;
    Edge menuButtonPadding_;
    Dimension handleDiameter_;
    Dimension handleDiameterInner_;
    Dimension menuSpacingWithText_;
    Dimension menuButtonWidth_;
    Dimension menuButtonHeight_;
    Dimension moreButtonHeight_;
    Dimension selectOverlayMaxWidth_;
    TextStyle menuButtonTextStyle_;
    double alphaDisabled_ = 0.0;
    std::string cameraInput_;

    InternalResource::ResourceId backResourceId_ = InternalResource::ResourceId::NO_ID;
    InternalResource::ResourceId moreResourceId_ = InternalResource::ResourceId::NO_ID;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_OVERLAY_TEXT_OVERLAY_THEME_H
