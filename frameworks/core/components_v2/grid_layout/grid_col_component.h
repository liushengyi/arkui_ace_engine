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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_GRID_LAYOUT_GRIDCOL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_GRID_LAYOUT_GRIDCOL_H

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_v2/grid_layout/grid_col_element.h"
#include "core/components_v2/grid_layout/grid_container_util_class.h"
#include "core/components_v2/grid_layout/render_grid_col.h"
#include "core/pipeline/base/sole_child_component.h"

namespace OHOS::Ace::V2 {

class ACE_EXPORT GridColComponent : public SoleChildComponent {
    DECLARE_ACE_TYPE(GridColComponent, SoleChildComponent);

public:
    void SetSpan(const RefPtr<GridContainerSize>& span)
    {
        span_ = span;
    }

    void SetOffset(const RefPtr<GridContainerSize>& offset)
    {
        offset_ = offset;
    }

    void SetOrder(const RefPtr<GridContainerSize>& order)
    {
        order_ = order;
    }

    RefPtr<GridContainerSize> GetSpan() const
    {
        return span_;
    }

    int32_t GetSpan(GridSizeType sizeType) const
    {
        LOGD("GetSpan %{public}d", span_->sm);
        switch (sizeType) {
            case GridSizeType::XS:
                return span_->xs;
                break;
            case GridSizeType::SM:
                return span_->sm;
                break;
            case GridSizeType::MD:
                return span_->md;
                break;
            case GridSizeType::LG:
                return span_->lg;
                break;
            case GridSizeType::XL:
                return span_->xl;
                break;
            case GridSizeType::XXL:
                return span_->xxl;
                break;
            default:
                return span_->xs;
        }
    }

    RefPtr<GridContainerSize> GetOffset() const
    {
        return offset_;
    }

    int32_t GetOffset(GridSizeType sizeType) const
    {
        switch (sizeType) {
            case GridSizeType::XS:
                return offset_->xs;
                break;
            case GridSizeType::SM:
                return offset_->sm;
                break;
            case GridSizeType::MD:
                return offset_->md;
                break;
            case GridSizeType::LG:
                return offset_->lg;
                break;
            case GridSizeType::XL:
                return offset_->xl;
                break;
            case GridSizeType::XXL:
                return offset_->xxl;
                break;
            default:
                return offset_->xs;
        }
    }

    RefPtr<GridContainerSize> GetOrder() const
    {
        return order_;
    }

    int32_t GetOrder(GridSizeType sizeType) const
    {
        switch (sizeType) {
            case GridSizeType::XS:
                return order_->xs;
                break;
            case GridSizeType::SM:
                return order_->sm;
                break;
            case GridSizeType::MD:
                return order_->md;
                break;
            case GridSizeType::LG:
                return order_->lg;
                break;
            case GridSizeType::XL:
                return order_->xl;
                break;
            case GridSizeType::XXL:
                return order_->xxl;
                break;
            default:
                return order_->xs;
        }
    }

    RefPtr<Element> CreateElement() override;

    RefPtr<RenderNode> CreateRenderNode() override;

private:
    RefPtr<GridContainerSize> span_ = AceType::MakeRefPtr<GridContainerSize>(1);
    RefPtr<GridContainerSize> offset_ = AceType::MakeRefPtr<GridContainerSize>(0);
    RefPtr<GridContainerSize> order_ = AceType::MakeRefPtr<GridContainerSize>(0);
};

} // namespace OHOS::Ace::V2
#endif