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

#include "core/components_v2/inspector/select_composed_element.h"

#include "core/components/select/select_element.h"
#include "core/components_v2/inspector/utils.h"

#include "base/log/dump_log.h"

namespace OHOS::Ace::V2 {

const std::unordered_map<std::string, std::function<std::string(const SelectComposedElement&)>> CREATE_JSON_MAP {
    { "options", [](const SelectComposedElement& inspector) { return inspector.GetOptions(); } },
    { "selected", [](const SelectComposedElement& inspector) { return inspector.GetSelected(); } },
    { "value", [](const SelectComposedElement& inspector) { return inspector.GetSelectValue(); } },
    { "font", [](const SelectComposedElement& inspector) { return inspector.GetFont(); } },
    { "fontColor", [](const SelectComposedElement& inspector) { return inspector.GetFontColor(); } },
    { "selectedOptionBgColor", [](const SelectComposedElement& inspector) { return inspector.GetSelectBgColor(); } },
    { "selectedOptionFont", [](const SelectComposedElement& inspector) { return inspector.GetSelectFont(); } },
    { "selectedOptionFontColor", [](const SelectComposedElement& inspector)
        { return inspector.GetSelectFontColor(); } },
    { "optionBgColor", [](const SelectComposedElement& inspector) { return inspector.GetOptionBgColor(); } },
    { "optionFont", [](const SelectComposedElement& inspector) { return inspector.GetOptionFont(); } },
    { "optionFontColor", [](const SelectComposedElement& inspector) { return inspector.GetOptionFontColor(); } }
};

void SelectComposedElement::Dump(void)
{
    InspectorComposedElement::Dump();
    DumpLog::GetInstance().AddDesc(std::string("select_composed_element"));
    DumpLog::GetInstance().AddDesc(std::string("options: ").append(GetOptions()));
    DumpLog::GetInstance().AddDesc(std::string("selected: ").append(GetSelected()));
    DumpLog::GetInstance().AddDesc(std::string("value: ").append(GetSelectValue()));
    DumpLog::GetInstance().AddDesc(std::string("font: ").append(GetFont()));
    DumpLog::GetInstance().AddDesc(std::string("fontColor: ").append(GetFontColor()));
    DumpLog::GetInstance().AddDesc(std::string("selectedOptionBgColor: ").append(GetSelectBgColor()));
    DumpLog::GetInstance().AddDesc(std::string("selectedOptionFont: ").append(GetSelectFont()));
    DumpLog::GetInstance().AddDesc(std::string("selectedOptionFontColor: ").append(GetSelectFontColor()));
    DumpLog::GetInstance().AddDesc(std::string("optionBgColor: ").append(GetOptionBgColor()));
    DumpLog::GetInstance().AddDesc(std::string("optionFont: ").append(GetOptionFont()));
    DumpLog::GetInstance().AddDesc(std::string("optionFontColor: ").append(GetOptionFontColor()));
}

std::unique_ptr<OHOS::Ace::JsonValue> SelectComposedElement::ToJsonObject() const
{
    auto resultJson = InspectorComposedElement::ToJsonObject();
    for (const auto& value : CREATE_JSON_MAP) {
        resultJson->Put(value.first.c_str(), value.second(*this).c_str());
    }
    return resultJson;
}

std::string SelectComposedElement::GetOptions() const
{
    auto render = GetRenderSelect();
    auto jsonValue = JsonUtil::Create(true);
    if (!render) {
        return "";
    }
    auto selectComponent = render->GetSelectComponent();
    if (!selectComponent) {
        return "";
    }
    auto popup = selectComponent->GetSelectPopupComponent();
    if (popup) {
        auto vectorValue = popup->GetSelectOptions();
        auto jsonOptions = JsonUtil::CreateArray(true);
        for (size_t i = 0; i < vectorValue.size(); i++) {
            if (vectorValue[i] && vectorValue[i]->GetIcon()) {
                auto temp = JsonUtil::Create(true);
                temp->Put("value", vectorValue[i]->GetValue().c_str());
                temp->Put("icon", vectorValue[i]->GetIcon()->GetSrc().c_str());
                auto index = std::to_string(i);
                jsonOptions->Put(index.c_str(), temp);
            }
        }
        jsonValue->Put("options", jsonOptions);
        return jsonValue->ToString();
    }
    return "";
}

std::string SelectComposedElement::GetSelected() const
{
    auto render = GetRenderSelect();
    if (render) {
        if (render->GetSelectComponent()) {
            return std::to_string(render->GetSelectComponent()->GetSelected());
        }
    }
    return "";
}

std::string SelectComposedElement::GetSelectValue() const
{
    auto render = GetRenderSelect();
    if (render) {
        return render->GetSelectComponent()->GetTipText()->GetData();
    }
    return "";
}

std::string SelectComposedElement::GetFont() const
{
    auto render = GetRenderSelect();
    if (render) {
        return GetTextStyleInJson(render->GetSelectComponent()->GetSelectStyle());
    }
    return "";
}

std::string SelectComposedElement::GetFontColor() const
{
    auto render = GetRenderSelect();
    if (render) {
        return ConvertColorToString(render->GetSelectComponent()->GetSelectStyle().GetTextColor());
    }
    return "";
}

std::string SelectComposedElement::GetSelectBgColor() const
{
    auto render = GetRenderSelect();
    if (render) {
        auto popup = render->GetSelectComponent()->GetPopup();
        if (!popup) {
            LOGE("popup is null");
            return "";
        }
        auto option = popup->GetSelectOptions();
        for (auto& optionItem : option) {
            if (optionItem) {
                return ConvertColorToString(optionItem->GetSelectedBackgroundColor());
            }
        }
    }
    return "";
}

std::string SelectComposedElement::GetSelectFont() const
{
    auto render = GetRenderSelect();
    if (render) {
        auto popup = render->GetSelectComponent()->GetPopup();
        if (!popup) {
            LOGE("popup is null");
            return "";
        }
        auto option = popup->GetSelectOptions();
        for (auto& optionItem : option) {
            if (optionItem) {
                return GetTextStyleInJson(optionItem->GetSelectedTextStyle());
            }
        }
    }
    return "";
}

std::string SelectComposedElement::GetSelectFontColor() const
{
    auto render = GetRenderSelect();
    if (render) {
        auto popup = render->GetSelectComponent()->GetPopup();
        if (!popup) {
            LOGE("popup is null");
            return "";
        }
        auto option = popup->GetSelectOptions();
        for (auto& optionItem : option) {
            if (optionItem) {
                return ConvertColorToString(optionItem->GetSelectedTextStyle().GetTextColor());
            }
        }
    }
    return "";
}

std::string SelectComposedElement::GetOptionBgColor() const
{
    auto render = GetRenderSelect();
    if (render) {
        auto popup = render->GetSelectComponent()->GetPopup();
        if (!popup) {
            LOGE("popup is null");
            return "";
        }
        auto option = popup->GetSelectOptions();
        for (auto& optionItem : option) {
            if (optionItem) {
                return ConvertColorToString(optionItem->GetBackgroundColor());
            }
        }
    }
    return "";
}

std::string SelectComposedElement::GetOptionFont() const
{
    auto render = GetRenderSelect();
    if (render) {
        auto popup = render->GetSelectComponent()->GetPopup();
        if (!popup) {
            LOGE("popup is null");
            return "";
        }
        auto option = popup->GetSelectOptions();
        for (auto& optionItem : option) {
            if (optionItem) {
                return GetTextStyleInJson(optionItem->GetTextStyle());
            }
        }
    }
    return "";
}

std::string SelectComposedElement::GetOptionFontColor() const
{
    auto render = GetRenderSelect();
    if (render) {
        return ConvertColorToString(render->GetSelectComponent()->GetSelectStyle().GetTextColor());
    }
    return "";
}

std::string SelectComposedElement::GetWidth() const
{
    auto render = GetRenderSelect();
    if (!render) {
        return "";
    }
    auto selectComponent = render->GetSelectComponent();
    if (!selectComponent) {
        return "";
    }
    return selectComponent->GetWidth().ToString();
}

std::string SelectComposedElement::GetHeight() const
{
    auto render = GetRenderSelect();
    if (!render) {
        return "";
    }
    auto selectComponent = render->GetSelectComponent();
    if (!selectComponent) {
        return "";
    }
    return selectComponent->GetHeight().ToString();
}

OHOS::Ace::RefPtr<OHOS::Ace::RenderSelect> SelectComposedElement::GetRenderSelect() const
{
    auto node = GetInspectorNode(SelectElement::TypeId());
    if (node) {
        return AceType::DynamicCast<RenderSelect>(node);
    }
    return nullptr;
}

} // namespace OHOS::Ace::V2