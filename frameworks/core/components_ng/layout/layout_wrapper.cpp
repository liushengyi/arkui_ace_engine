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

#include "core/components_ng/layout/layout_wrapper.h"

#include <algorithm>

#include "base/log/ace_checker.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/layout/layout_wrapper_builder.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

bool LayoutWrapper::SkipMeasureContent() const
{
    return skipMeasureContent_ == true;
}

void LayoutWrapper::ApplySafeArea(const SafeAreaInsets& insets, LayoutConstraintF& constraint)
{
    constraint.MinusPadding(
        insets.left_.Length(), insets.right_.Length(), insets.top_.Length(), insets.bottom_.Length());
}

void LayoutWrapper::OffsetNodeToSafeArea()
{
    auto&& insets = GetLayoutProperty()->GetSafeAreaInsets();
    CHECK_NULL_VOID(insets);
    auto geometryNode = GetGeometryNode();
    auto offset = geometryNode->GetMarginFrameOffset();
    if (offset.GetX() < insets->left_.end) {
        offset.SetX(insets->left_.end);
    }
    if (offset.GetY() < insets->top_.end) {
        offset.SetY(insets->top_.end);
    }

    auto right = offset.GetX() + geometryNode->GetMarginFrameSize().Width();
    auto rightBound = insets->right_.IsValid() ? insets->right_.start : PipelineContext::GetCurrentRootWidth();
    if (right > rightBound) {
        offset.SetX(rightBound - geometryNode->GetMarginFrameSize().Width());
    }
    auto bottomBound = insets->bottom_.IsValid() ? insets->bottom_.start : PipelineContext::GetCurrentRootHeight();
    auto bottom = offset.GetY() + geometryNode->GetMarginFrameSize().Height();
    if (bottom > bottomBound) {
        offset.SetY(bottomBound - geometryNode->GetMarginFrameSize().Height());
    }
    geometryNode->SetMarginFrameOffset(offset);
}

void LayoutWrapper::RestoreGeoState()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetSafeAreaManager();
    auto&& restoreNodes = manager->GetGeoRestoreNodes();
    if (restoreNodes.find(hostNode_) != restoreNodes.end()) {
        GetGeometryNode()->Restore();
        manager->RemoveRestoreNode(hostNode_);
    }
}

void LayoutWrapper::AvoidKeyboard()
{
    // apply keyboard avoidance on Page
    if (GetHostTag() == V2::PAGE_ETS_TAG) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto manager = pipeline->GetSafeAreaManager();
        GetGeometryNode()->SetFrameOffset(
            GetGeometryNode()->GetFrameOffset() + OffsetF(0, manager->GetKeyboardOffset()));
    }
}

void LayoutWrapper::SaveGeoState()
{
    auto&& expandOpts = GetLayoutProperty()->GetSafeAreaExpandOpts();
    if ((expandOpts && expandOpts->Expansive()) || GetHostTag() == V2::PAGE_ETS_TAG) {
        // save geometry state before SafeArea expansion / keyboard avoidance.
        GetGeometryNode()->Save();
        // record nodes whose geometry states need to be restored, to speed up RestoreGeoState
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto manager = pipeline->GetSafeAreaManager();
        manager->AddGeoRestoreNode(GetHostNode());
    }
}

void LayoutWrapper::ExpandSafeArea()
{
    auto&& opts = GetLayoutProperty()->GetSafeAreaExpandOpts();
    CHECK_NULL_VOID(opts && opts->Expansive());

    // children of Scrollable nodes don't support expandSafeArea
    auto host = GetHostNode();
    CHECK_NULL_VOID(host);
    auto parent = host->GetAncestorNodeOfFrame();
    if (parent && parent->GetPattern<ScrollablePattern>()) {
        return;
    }

    if ((opts->edges & SAFE_AREA_EDGE_BOTTOM) && (opts->type & SAFE_AREA_TYPE_KEYBOARD)) {
        ExpandIntoKeyboard();
    }

    // get frame in global offset
    auto parentGlobalOffset = host->GetParentGlobalOffsetDuringLayout();
    auto geometryNode = GetGeometryNode();
    auto frame = geometryNode->GetFrameRect() + parentGlobalOffset;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto safeArea = pipeline->GetSafeAreaManager()->GetCombinedSafeArea(*opts);
    if ((opts->edges & SAFE_AREA_EDGE_START) && safeArea.left_.IsOverlapped(frame.Left())) {
        frame.SetWidth(frame.Width() + frame.Left() - safeArea.left_.start);
        frame.SetLeft(safeArea.left_.start);
    }
    if ((opts->edges & SAFE_AREA_EDGE_TOP) && safeArea.top_.IsOverlapped(frame.Top())) {
        frame.SetHeight(frame.Height() + frame.Top() - safeArea.top_.start);
        frame.SetTop(safeArea.top_.start);
    }

    if ((opts->edges & SAFE_AREA_EDGE_END) && safeArea.right_.IsOverlapped(frame.Right())) {
        frame.SetWidth(frame.Width() + (safeArea.right_.end - frame.Right()));
    }
    if ((opts->edges & SAFE_AREA_EDGE_BOTTOM) && safeArea.bottom_.IsOverlapped(frame.Bottom())) {
        frame.SetHeight(frame.Height() + (safeArea.bottom_.end - frame.Bottom()));
    }

    // reset if User has fixed size
    auto layoutProperty = GetLayoutProperty();
    if (layoutProperty->HasFixedWidth()) {
        frame.SetWidth(geometryNode->GetFrameRect().Width());
    }
    if (layoutProperty->HasFixedHeight()) {
        frame.SetHeight(geometryNode->GetFrameRect().Height());
    }
    if (layoutProperty->HasAspectRatio()) {
        frame.SetHeight(frame.Width() / layoutProperty->GetAspectRatio());
    }
    // restore to local offset
    frame -= parentGlobalOffset;
    auto diff = geometryNode->GetFrameOffset() - frame.GetOffset();
    if (!diff.NonOffset()) {
        // children's position should remain the same.
        AdjustChildren(diff);
    }
    geometryNode->SetFrameOffset(frame.GetOffset());
    geometryNode->SetFrameSize(frame.GetSize());
}

void LayoutWrapper::AdjustChildren(const OffsetF& offset)
{
    for (const auto& childUI : GetHostNode()->GetChildren()) {
        auto child = DynamicCast<FrameNode>(childUI);
        if (!child) {
            continue;
        }
        auto childGeo = child->GetGeometryNode();
        childGeo->SetFrameOffset(childGeo->GetFrameOffset() + offset);
        child->ForceSyncGeometryNode();
    }
}

void LayoutWrapper::ExpandIntoKeyboard()
{
    // if parent already expanded into keyboard, offset shouldn't be applied again
    auto parent = GetHostNode()->GetAncestorNodeOfFrame();
    while (parent) {
        auto&& opts = parent->GetLayoutProperty()->GetSafeAreaExpandOpts();
        if (opts && (opts->edges & SAFE_AREA_EDGE_BOTTOM) && opts->type & SAFE_AREA_TYPE_KEYBOARD) {
            return;
        }
        parent = parent->GetAncestorNodeOfFrame();
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto geometryNode = GetGeometryNode();
    geometryNode->SetFrameOffset(
        geometryNode->GetFrameOffset() - OffsetF(0, pipeline->GetSafeAreaManager()->GetKeyboardOffset()));
}

void LayoutWrapper::ApplyConstraint(LayoutConstraintF constraint)
{
    GetGeometryNode()->SetParentLayoutConstraint(constraint);

    auto layoutProperty = GetLayoutProperty();
    const auto& magicItemProperty = layoutProperty->GetMagicItemProperty();
    if (magicItemProperty && magicItemProperty->HasAspectRatio()) {
        std::optional<CalcSize> idealSize = std::nullopt;
        if (layoutProperty->GetCalcLayoutConstraint()) {
            idealSize = layoutProperty->GetCalcLayoutConstraint()->selfIdealSize;
        }
        auto greaterThanApiTen = Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN);
        constraint.ApplyAspectRatio(magicItemProperty->GetAspectRatioValue(), idealSize, greaterThanApiTen);
    }

    auto&& insets = layoutProperty->GetSafeAreaInsets();
    if (insets) {
        ApplySafeArea(*insets, constraint);
    }

    layoutProperty->UpdateLayoutConstraint(constraint);
}

void LayoutWrapper::CreateRootConstraint()
{
    LayoutConstraintF layoutConstraint;
    layoutConstraint.percentReference.SetWidth(PipelineContext::GetCurrentRootWidth());
    auto layoutProperty = GetLayoutProperty();
    const auto& magicItemProperty = layoutProperty->GetMagicItemProperty();
    auto hasAspectRatio = magicItemProperty && magicItemProperty->HasAspectRatio();
    if (hasAspectRatio) {
        auto aspectRatio = magicItemProperty->GetAspectRatioValue();
        if (Positive(aspectRatio)) {
            auto height = PipelineContext::GetCurrentRootHeight() / aspectRatio;
            layoutConstraint.percentReference.SetHeight(height);
        }
    } else {
        layoutConstraint.percentReference.SetHeight(PipelineContext::GetCurrentRootHeight());
    }
    layoutProperty->UpdateLayoutConstraint(layoutConstraint);
}

RefPtr<FrameNode> LayoutWrapper::GetHostNode() const
{
    return hostNode_.Upgrade();
}

void LayoutWrapper::AddNodeFlexLayouts()
{
    if (!AceChecker::IsPerformanceCheckEnabled()) {
        return;
    }
    auto host = GetHostNode();
    CHECK_NULL_VOID(host);
    auto frameNodeParent = host->GetAncestorNodeOfFrame();
    CHECK_NULL_VOID(frameNodeParent);
    if (frameNodeParent->GetTag() == V2::FLEX_ETS_TAG) {
        auto parent = host->GetParent();
        CHECK_NULL_VOID(parent);
        if (parent->GetTag() == V2::JS_VIEW_ETS_TAG) {
            parent->AddFlexLayouts();
        } else if (host->GetTag() == V2::COMMON_VIEW_ETS_TAG) {
            auto children = host->GetChildren();
            if (!children.empty()) {
                auto begin = children.begin();
                (*begin)->AddFlexLayouts();
            }
        } else {
            host->AddFlexLayouts();
        }
    }
}

void LayoutWrapper::AddNodeLayoutTime(int64_t time)
{
    if (!AceChecker::IsPerformanceCheckEnabled()) {
        return;
    }
    auto host = GetHostNode();
    CHECK_NULL_VOID(host);
    host->SetLayoutTime(time);
}

} // namespace OHOS::Ace::NG
