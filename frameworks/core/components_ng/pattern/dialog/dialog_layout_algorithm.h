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
#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_DIALOG_DIALOG_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_DIALOG_DIALOG_LAYOUT_ALGORITHM_H

#include <string>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/size_t.h"
#include "core/components/common/layout/grid_layout_info.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/common/properties/border.h"
#include "core/components/common/properties/edge.h"
#include "core/components/common/properties/placement.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/pattern/dialog/dialog_layout_property.h"

namespace OHOS::Ace::NG {
enum class TouchingBoundaryType {
    NotTouchBoundary = 0,
    TouchBottomBoundary,
    TouchRightBoundary,
};
// DialogLayoutAlgorithm uses for Dialog Node.
class ACE_EXPORT DialogLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(DialogLayoutAlgorithm, LayoutAlgorithm);

public:
    DialogLayoutAlgorithm() = default;
    ~DialogLayoutAlgorithm() override = default;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    RectF GetTouchRegion() const
    {
        return touchRegion_;
    }

private:
    LayoutConstraintF CreateDialogChildConstraint(LayoutWrapper* layoutWrapper, float height, float width);
    void Distribute(float& scrollHeight, float& listHeight, float restHeight);
    void AnalysisHeightOfChild(LayoutWrapper* layoutWrapper);
    void AnalysisLayoutOfContent(LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& scroll);

    void ComputeInnerLayoutParam(LayoutConstraintF& innerLayout);
    double GetMaxWidthBasedOnGridType(const RefPtr<GridColumnInfo>& info, GridSizeType type, DeviceType deviceType);

    OffsetF ComputeChildPosition(
        const SizeF& childSize, const RefPtr<DialogLayoutProperty>& prop, const SizeF& slefSize);
    bool SetAlignmentSwitch(const SizeF& maxSize, const SizeF& childSize, OffsetF& topLeftPoint);
    bool IsDialogTouchingBoundary(OffsetF topLeftPoint, SizeF childSize, SizeF selfSize);
    void MultipleDialog(const RefPtr<DialogLayoutProperty>& dialogProp, const SizeF& childSize, const SizeF& selfSize);
    void ProcessMaskRect(std::optional<DimensionRect> maskRect, const RefPtr<FrameNode>& dialog);
	
    void UpdateTouchRegion();

    double GetPaddingBottom() const;

    OffsetF AdjustChildPosition(
        OffsetF& topLeftPoint, const OffsetF& dialogOffset, const SizeF& childSize, bool needAvoidKeyboard) const;

    RectF touchRegion_;
    OffsetF topLeftPoint_;
    bool customSize_ = false;

    int32_t gridCount_ = -1;
    int32_t subWindowId_ = -1;
    DimensionOffset dialogOffset_;
    DialogAlignment alignment_ = DialogAlignment::DEFAULT;
    TouchingBoundaryType touchingBoundaryFlag_ = TouchingBoundaryType::NotTouchBoundary;

    ACE_DISALLOW_COPY_AND_MOVE(DialogLayoutAlgorithm);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_DIALOG_DIALOG_LAYOUT_ALGORITHM_H
