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
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_checkboxgroup_modifier.h"

#include "core/pipeline/base/element_register.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/checkboxgroup/checkboxgroup_model_ng.h"
#include "frameworks/core/components/checkable/checkable_theme.h"

namespace OHOS::Ace::NG {
const DimensionUnit DEFAULT_UNIT = DimensionUnit::VP;
void SetGroupSelectedColor(NodeHandle node, uint32_t color)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CheckBoxGroupModelNG::SetSelectedColor(frameNode, Color(color));
}

void ResetGroupSelectedColor(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto checkBoxTheme = themeManager->GetTheme<CheckboxTheme>();
    CheckBoxGroupModelNG::SetSelectedColor(frameNode, checkBoxTheme->GetActiveColor());
}

void SetGroupUnSelectedColor(NodeHandle node, uint32_t color)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CheckBoxGroupModelNG::SetUnSelectedColor(frameNode, Color(color));
}

void ResetGroupUnSelectedColor(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto checkBoxTheme = themeManager->GetTheme<CheckboxTheme>();
    CheckBoxGroupModelNG::SetUnSelectedColor(frameNode, checkBoxTheme->GetInactiveColor());
}

void SetGroupSelectAll(NodeHandle node, bool isSelected)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CheckBoxGroupModelNG::SetSelectAll(frameNode, isSelected);
}

void ResetGroupSelectAll(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CheckBoxGroupModelNG::SetSelectAll(frameNode, false);
}

void SetCheckboxGroupWidth(NodeHandle node, double value, int unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    Dimension width = Dimension(value, static_cast<OHOS::Ace::DimensionUnit>(unit));
    CheckBoxGroupModelNG::SetWidth(frameNode, width);
}

void ResetCheckboxGroupWidth(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto checkBoxTheme = themeManager->GetTheme<CheckboxTheme>();
    CHECK_NULL_VOID(checkBoxTheme);
    auto defaultWidth = checkBoxTheme->GetDefaultWidth();
    auto horizontalPadding = checkBoxTheme->GetHotZoneHorizontalPadding();
    auto width = defaultWidth - horizontalPadding * 2;
    CheckBoxGroupModelNG::SetWidth(frameNode, width);
}

void SetCheckboxGroupHeight(NodeHandle node, double value, int unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    Dimension height = Dimension(value, static_cast<OHOS::Ace::DimensionUnit>(unit));
    CheckBoxGroupModelNG::SetHeight(frameNode, height);
}

void ResetCheckboxGroupHeight(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto checkBoxTheme = themeManager->GetTheme<CheckboxTheme>();
    CHECK_NULL_VOID(checkBoxTheme);
    auto defaultHeight = checkBoxTheme->GetDefaultHeight();
    auto verticalPadding = checkBoxTheme->GetHotZoneVerticalPadding();
    auto height = defaultHeight - verticalPadding * 2;
    CheckBoxGroupModelNG::SetHeight(frameNode, height);
}

void SetGroupMark(NodeHandle node, uint32_t color, double sizeValue, double widthValue)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CheckBoxGroupModelNG::SetCheckMarkColor(frameNode, Color(color));

    Dimension size = Dimension(sizeValue, DEFAULT_UNIT);
    CheckBoxGroupModelNG::SetCheckMarkSize(frameNode, size);

    Dimension width = Dimension(widthValue, DEFAULT_UNIT);
    CheckBoxGroupModelNG::SetCheckMarkWidth(frameNode, width);
}

void ResetGroupMark(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto context = frameNode->GetContext();
    CHECK_NULL_VOID(context);
    auto themeManager = context->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto checkBoxTheme = themeManager->GetTheme<CheckboxTheme>();

    CheckBoxGroupModelNG::SetCheckMarkColor(frameNode, checkBoxTheme->GetPointColor());

    Dimension size = Dimension(0.0, DEFAULT_UNIT);
    CheckBoxGroupModelNG::SetCheckMarkSize(frameNode, size);

    Dimension width = Dimension(2.0, DEFAULT_UNIT);
    CheckBoxGroupModelNG::SetCheckMarkWidth(frameNode, width);
}

ArkUICheckboxGroupModifierAPI GetCheckboxGroupModifier()
{
    static const ArkUICheckboxGroupModifierAPI modifier = { SetGroupSelectedColor, ResetGroupSelectedColor,
        SetGroupUnSelectedColor, ResetGroupUnSelectedColor, SetGroupSelectAll, ResetGroupSelectAll,
        SetCheckboxGroupWidth, ResetCheckboxGroupWidth, SetCheckboxGroupHeight,
        ResetCheckboxGroupHeight, SetGroupMark, ResetGroupMark };
    return modifier;
}
}
