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

#include "core/components_ng/pattern/root/root_layout_algorithm.h"

#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void RootLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    auto&& children = layoutWrapper->GetAllChildrenWithBuild();
    if (children.size() == 1) {
        // Stage node only
        (*children.begin())->Measure(layoutConstraint);
        PerformMeasureSelf(layoutWrapper);
        return;
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    childInsets_ = pipeline->GetSafeArea();
    auto manager = pipeline->GetSafeAreaManager();
    childInsets_.bottom_ = childInsets_.bottom_.Combine(manager->GetKeyboardInset());
    auto safeAreaConstraint = layoutConstraint;
    LayoutWrapper::ApplySafeArea(childInsets_, safeAreaConstraint);
    for (auto&& child : children) {
        // Stage displays app background color and needs to be full screen.
        if (child->GetHostTag() == V2::STAGE_ETS_TAG || child->GetHostTag() == V2::CONTAINER_MODAL_ETS_TAG) {
            child->Measure(layoutConstraint);
        } else {
            child->Measure(safeAreaConstraint);
        }
    }
    PerformMeasureSelf(layoutWrapper);
}

void RootLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    PerformLayout(layoutWrapper);
    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        if (child->GetHostTag() != V2::STAGE_ETS_TAG) {
            child->GetGeometryNode()->SetFrameOffset(OffsetF(childInsets_.left_.Length(), childInsets_.top_.Length()));
        }
        child->Layout();
    }
}
} // namespace OHOS::Ace::NG
