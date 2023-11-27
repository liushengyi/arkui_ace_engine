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
#include "core/components_ng/pattern/toast/toast_pattern.h"

#include "base/subwindow/subwindow_manager.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/container.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t API_VERSION_9 = 9;

float GetTextHeight(const RefPtr<FrameNode>& textNode)
{
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, 0.0f);
    auto layoutConstraint = textLayoutProperty->GetLayoutConstraint();

    auto textLayoutWrapper = textNode->CreateLayoutWrapper();
    CHECK_NULL_RETURN(textLayoutWrapper, 0.0f);
    textLayoutWrapper->Measure(layoutConstraint);
    auto textGeometry = textLayoutWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(textGeometry, 0.0f);
    auto textSize = textGeometry->GetMarginFrameSize();
    return textSize.Height();
}

// get main window's pipeline
RefPtr<PipelineContext> GetMainPipelineContext()
{
    auto containerId = Container::CurrentId();
    RefPtr<PipelineContext> context;
    if (containerId >= MIN_SUBCONTAINER_ID) {
        auto parentContainerId = SubwindowManager::GetInstance()->GetParentContainerId(containerId);
        auto parentContainer = AceEngine::Get().GetContainer(parentContainerId);
        CHECK_NULL_RETURN(parentContainer, nullptr);
        context = AceType::DynamicCast<PipelineContext>(parentContainer->GetPipelineContext());
    } else {
        context = PipelineContext::GetCurrentContext();
    }
    return context;
}
} // namespace

bool ToastPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& changeConfig)
{
    CHECK_NULL_RETURN(dirty, false);

    auto toastBottom = GetBottomValue(dirty);
    // todo get parent width and height
    auto context = IsDefaultToast() ? PipelineContext::GetCurrentContext() : GetMainPipelineContext();
    CHECK_NULL_RETURN(context, false);
    auto rootHeight = context->GetRootHeight();
    auto rootWidth = context->GetRootWidth();
    auto text = dirty->GetOrCreateChildByIndex(0);
    CHECK_NULL_RETURN(text, false);
    auto toastNode = dirty->GetHostNode();
    auto toastContext = toastNode->GetRenderContext();
    CHECK_NULL_RETURN(toastContext, false);

    OffsetT<Dimension> offset { Dimension((rootWidth - text->GetGeometryNode()->GetMarginFrameSize().Width()) / 2.0f),
        {} };
    if (context->GetMinPlatformVersion() > API_VERSION_9) {
        auto layoutProperty = dirty->GetLayoutProperty();
        CHECK_NULL_RETURN(layoutProperty, false);
        const auto& safeArea = layoutProperty->GetSafeAreaInsets();
        offset.SetY(Dimension { rootHeight - toastBottom - dirty->GetGeometryNode()->GetMarginFrameSize().Height() -
                                (safeArea ? safeArea->bottom_.Length() : 0.0f) });
    } else {
        offset.SetY(Dimension { rootHeight - toastBottom });
    }
    // show in the float subwindow
    if (!IsDefaultToast()) {
        OffsetT<Dimension> displayWindowOffset = { Dimension(context->GetDisplayWindowRectInfo().GetOffset().GetX()),
            Dimension(context->GetDisplayWindowRectInfo().GetOffset().GetY()) };
        offset += displayWindowOffset;
    }
    TAG_LOGD(AceLogTag::ACE_OVERLAY,
        "Toast bottom value: [%{public}f],offsetX[% { public } s] offsetY[% { public } s] ", toastBottom,
        offset.GetX().ToString().c_str(), offset.GetY().ToString().c_str());
    toastContext->UpdateOffset(offset);
    return true;
}

double ToastPattern::GetBottomValue(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    // Obtain the height relative to the main window
    auto pipeline = IsDefaultToast() ? PipelineContext::GetCurrentContext() : GetMainPipelineContext();
    CHECK_NULL_RETURN(pipeline, 0.0);
    auto rootHeight = Dimension(pipeline->GetRootHeight());
    auto toastTheme = pipeline->GetTheme<ToastTheme>();
    CHECK_NULL_RETURN(toastTheme, 0.0);

    auto toastProp = DynamicCast<ToastLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(toastProp, 0.0);
    auto toastBottom = toastProp->GetBottomValue(toastTheme->GetBottom());
    if (toastBottom.Unit() == DimensionUnit::PERCENT) {
        toastBottom = rootHeight * toastBottom.Value();
    }
    return GreatOrEqual(toastBottom.ConvertToPx(), 0.0) ? toastBottom.ConvertToPx()
                                                        : toastTheme->GetBottom().ConvertToPx();
}

void ToastPattern::BeforeCreateLayoutWrapper()
{
    PopupBasePattern::BeforeCreateLayoutWrapper();

    auto toastNode = GetHost();
    CHECK_NULL_VOID(toastNode);
    UpdateToastSize(toastNode);

    auto textNode = DynamicCast<FrameNode>(toastNode->GetFirstChild());
    CHECK_NULL_VOID(textNode);
    UpdateTextSizeConstraint(textNode);

    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto toastTheme = context->GetTheme<ToastTheme>();
    CHECK_NULL_VOID(toastTheme);
    auto textHeight = GetTextHeight(textNode);
    if (textHeight > toastTheme->GetMinHeight().ConvertToPx()) {
        textNode->GetLayoutProperty<TextLayoutProperty>()->UpdateTextAlign(TextAlign::START);
    }
}

void ToastPattern::UpdateToastSize(const RefPtr<FrameNode>& toast)
{
    CHECK_NULL_VOID(toast);
    auto toastProperty = toast->GetLayoutProperty<ToastLayoutProperty>();
    CHECK_NULL_VOID(toastProperty);
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto rootWidth = Dimension(context->GetRootWidth());
    toastProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(rootWidth), std::nullopt));
}

void ToastPattern::UpdateTextSizeConstraint(const RefPtr<FrameNode>& text)
{
    CHECK_NULL_VOID(text);
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto gridColumnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::TOAST);
    auto parent = gridColumnInfo->GetParent();
    if (parent) {
        parent->BuildColumnWidth(context->GetRootWidth());
    }
    auto maxWidth = Dimension(gridColumnInfo->GetMaxWidth());
    auto textLayoutProperty = text->GetLayoutProperty();
    CHECK_NULL_VOID(textLayoutProperty);
    textLayoutProperty->UpdateCalcMaxSize(CalcSize(NG::CalcLength(maxWidth), std::nullopt));

    auto toastTheme = context->GetTheme<ToastTheme>();
    CHECK_NULL_VOID(toastTheme);
    auto minWidth = Dimension(toastTheme->GetMinWidth().ConvertToPx());
    auto minHeight = Dimension(toastTheme->GetMinHeight().ConvertToPx());
    textLayoutProperty->UpdateCalcMinSize(CalcSize(NG::CalcLength(minWidth), NG::CalcLength(minHeight)));
}

void ToastPattern::OnColorConfigurationUpdate()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto textContext = host->GetRenderContext();
    CHECK_NULL_VOID(textContext);
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto toastTheme = pipelineContext->GetTheme<ToastTheme>();
    CHECK_NULL_VOID(toastTheme);
    auto textColor = toastTheme->GetTextStyle().GetTextColor();
    auto textLayoutProperty = textNode_->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    textLayoutProperty->UpdateTextColor(textColor);
    host->SetNeedCallChildrenUpdate(false);
}
} // namespace OHOS::Ace::NG
