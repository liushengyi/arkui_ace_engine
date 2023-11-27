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

#include "core/components_ng/pattern/list/list_lanes_layout_algorithm.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void ListLanesLayoutAlgorithm::UpdateListItemConstraint(
    Axis axis, const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint)
{
    contentConstraint.parentIdealSize = selfIdealSize;
    contentConstraint.maxSize.SetMainSize(Infinity<float>(), axis);
    groupLayoutConstraint_ = contentConstraint;
    auto crossSizeOptional = selfIdealSize.CrossSize(axis);
    if (crossSizeOptional.has_value()) {
        float crossSize = crossSizeOptional.value();
        groupLayoutConstraint_.maxSize.SetCrossSize(crossSize, axis);
        if (lanes_ > 1) {
            float laneGutter = GetLaneGutter();
            crossSize = (crossSize + laneGutter) / lanes_ - laneGutter;
            crossSize = crossSize <= 0 ? 1 : crossSize;
        }
        if (maxLaneLength_.has_value() && maxLaneLength_.value() < crossSize) {
            crossSize = maxLaneLength_.value();
        }
        contentConstraint.percentReference.SetCrossSize(crossSize, axis);
        contentConstraint.parentIdealSize.SetCrossSize(crossSize, axis);
        contentConstraint.maxSize.SetCrossSize(crossSize, axis);
        if (minLaneLength_.has_value()) {
            contentConstraint.minSize.SetCrossSize(minLaneLength_.value(), axis);
        }
    }
}

float ListLanesLayoutAlgorithm::MeasureAndGetChildHeight(LayoutWrapper* layoutWrapper, int32_t childIndex)
{
    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(childIndex);
    CHECK_NULL_RETURN(wrapper, 0.0f);
    bool isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    if (isGroup) {
        auto listLayoutProperty =
            AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
        // true: layout forward, true: layout all group items.
        SetListItemGroupParam(wrapper, 0.0f, true, listLayoutProperty, true);
        wrapper->Measure(groupLayoutConstraint_);
    } else {
        wrapper->Measure(childLayoutConstraint_);
    }
    float mainLen = GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_);
    return mainLen;
}

int32_t ListLanesLayoutAlgorithm::LayoutALineForward(LayoutWrapper* layoutWrapper,
    int32_t& currentIndex, float startPos, float& endPos)
{
    float mainLen = 0.0f;
    bool isGroup = false;
    int32_t cnt = 0;
    int32_t lanes = lanes_ > 1 ? lanes_ : 1;
    for (int32_t i = 0; i < lanes && currentIndex + 1 <= GetMaxListItemIndex(); i++) {
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex + 1);
        if (!wrapper) {
            break;
        }
        isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        if (isGroup && cnt > 0) {
            wrapper->SetActive(false);
            isGroup = false;
            break;
        }
        cnt++;
        ++currentIndex;
        if (isGroup) {
            if (wrapper->GetHostNode()) {
                ACE_SCOPED_TRACE("[MeasureListForwardItemGroup:%d][self:%d][parent:%d]", currentIndex,
                    wrapper->GetHostNode()->GetId(), wrapper->GetHostNode()->GetParent() ?
                        wrapper->GetHostNode()->GetParent()->GetId() : 0);
            }
            auto listLayoutProperty = AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
            SetListItemGroupParam(wrapper, startPos, true, listLayoutProperty, GroupNeedAllLayout());
            wrapper->Measure(groupLayoutConstraint_);
        } else {
            if (wrapper->GetHostNode()) {
                ACE_SCOPED_TRACE("[MeasureListForwardItem:%d][self:%d][parent:%d]", currentIndex,
                    wrapper->GetHostNode()->GetId(), wrapper->GetHostNode()->GetParent() ?
                        wrapper->GetHostNode()->GetParent()->GetId() : 0);
            }
            wrapper->Measure(childLayoutConstraint_);
        }
        mainLen = std::max(mainLen, GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_));
        if (isGroup) {
            break;
        }
    }
    if (cnt > 0) {
        endPos = startPos + mainLen;
        for (int32_t i = 0; i < cnt; i++) {
            SetItemInfo(currentIndex - i, { startPos, endPos, isGroup });
        }
    }
    return cnt;
}

int32_t ListLanesLayoutAlgorithm::LayoutALineBackward(LayoutWrapper* layoutWrapper,
    int32_t& currentIndex, float endPos, float& startPos)
{
    float mainLen = 0.0f;
    bool isGroup = false;
    int32_t cnt = 0;
    int32_t lanes = lanes_ > 1 ? lanes_ : 1;
    for (int32_t i = 0; i < lanes && currentIndex - 1 >= 0; i++) {
        if (currentIndex > GetMaxListItemIndex() + 1) {
            --currentIndex;
            continue;
        }
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(currentIndex - 1);
        if (!wrapper) {
            break;
        }
        isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        if (isGroup && cnt > 0) {
            wrapper->SetActive(false);
            isGroup = false;
            break;
        }
        --currentIndex;

        cnt++;
        if (isGroup) {
            if (wrapper->GetHostNode()) {
                ACE_SCOPED_TRACE("[MeasureListBackwardItemGroup:%d][self:%d][parent:%d]", currentIndex,
                    wrapper->GetHostNode()->GetId(), wrapper->GetHostNode()->GetParent() ?
                        wrapper->GetHostNode()->GetParent()->GetId() : 0);
            }
            auto listLayoutProperty = AceType::DynamicCast<ListLayoutProperty>(layoutWrapper->GetLayoutProperty());
            SetListItemGroupParam(wrapper, endPos, false, listLayoutProperty, GroupNeedAllLayout());
            wrapper->Measure(groupLayoutConstraint_);
        } else {
            if (wrapper->GetHostNode()) {
                ACE_SCOPED_TRACE("[MeasureListBackwardItem:%d][self:%d][parent:%d]", currentIndex,
                    wrapper->GetHostNode()->GetId(), wrapper->GetHostNode()->GetParent() ?
                        wrapper->GetHostNode()->GetParent()->GetId() : 0);
            }
            wrapper->Measure(childLayoutConstraint_);
        }
        mainLen = std::max(mainLen, GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_));
        if (isGroup || (currentIndex - FindLanesStartIndex(layoutWrapper, currentIndex)) % lanes == 0) {
            break;
        }
    }
    if (cnt > 0) {
        startPos = endPos - mainLen;
        for (int32_t i = 0; i < cnt; i++) {
            SetItemInfo(currentIndex + i, { startPos, endPos, isGroup });
        }
    }
    return cnt;
}

int32_t ListLanesLayoutAlgorithm::CalculateLanesParam(std::optional<float>& minLaneLength,
    std::optional<float>& maxLaneLength, int32_t lanes, std::optional<float> crossSizeOptional, float laneGutter)
{
    if (lanes < 1) {
        return 1;
    }
    // Case 1: lane length constrain is not set
    //      1.1: use [lanes] set by user if [lanes] is set
    //      1.2: set [lanes] to 1 if [lanes] is not set
    if (!crossSizeOptional.has_value() || GreaterOrEqualToInfinity(crossSizeOptional.value()) ||
        !minLaneLength.has_value() || !maxLaneLength.has_value()) {
        maxLaneLength.reset();
        minLaneLength.reset();
        return lanes;
    }
    // Case 2: lane length constrain is set --> need to calculate [lanes_] according to contraint.
    // We agreed on such rules (assuming we have a vertical list here):
    // rule 1: [minLaneLength_] has a higher priority than [maxLaneLength_] when decide [lanes_], for e.g.,
    //         if [minLaneLength_] is 40, [maxLaneLength_] is 60, list's width is 120,
    //         the [lanes_] is 3 rather than 2.
    // rule 2: after [lanes_] is determined by rule 1, the width of lane will be as large as it can be, for
    // e.g.,
    //         if [minLaneLength_] is 40, [maxLaneLength_] is 60, list's width is 132, the [lanes_] is 3
    //         according to rule 1, then the width of lane will be 132 / 3 = 44 rather than 40,
    //         its [minLaneLength_].
    auto crossSize = crossSizeOptional.value();
    ModifyLaneLength(minLaneLength, maxLaneLength, crossSize);

    // if minLaneLength is 40, maxLaneLength is 60
    // when list's width is 120, lanes_ = 3
    // when list's width is 80, lanes_ = 2
    // when list's width is 70, lanes_ = 1
    float maxLanes = (crossSize + laneGutter) / (minLaneLength.value() + laneGutter);
    float minLanes = (crossSize + laneGutter) / (maxLaneLength.value() + laneGutter);
    // let's considerate scenarios when maxCrossSize > 0
    // now it's guaranteed that [minLaneLength_] <= [maxLaneLength_], i.e., maxLanes >= minLanes > 0
    // there are 3 scenarios:
    // 1. 1 > maxLanes >= minLanes > 0
    // 2. maxLanes >= 1 >= minLanes > 0
    // 3. maxLanes >= minLanes > 1
    // 1. 1 > maxLanes >= minLanes > 0 ---> maxCrossSize < minLaneLength_ =< maxLaneLength
    if (GreatNotEqual(1, maxLanes) && GreatOrEqual(maxLanes, minLanes)) {
        lanes = 1;
        minLaneLength = crossSize;
        maxLaneLength = crossSize;
        return lanes;
    }
    // 2. maxLanes >= 1 >= minLanes > 0 ---> minLaneLength_ = maxCrossSize < maxLaneLength
    if (GreatOrEqual(maxLanes, 1) && LessOrEqual(minLanes, 1)) {
        lanes = std::floor(maxLanes);
        maxLaneLength = crossSize;
        return lanes;
    }
    // 3. maxLanes >= minLanes > 1 ---> minLaneLength_ <= maxLaneLength < maxCrossSize
    if (GreatOrEqual(maxLanes, minLanes) && GreatNotEqual(minLanes, 1)) {
        lanes = std::floor(maxLanes);
        return lanes;
    }
    lanes = 1;
    TAG_LOGD(AceLogTag::ACE_LIST, "unexpected situation, set lanes to 1, maxLanes: %{public}f, "
        "minLanes: %{public}f, minLaneLength_: %{public}f, maxLaneLength_: %{public}f",
        maxLanes, minLanes, minLaneLength.value(), maxLaneLength.value());
    return lanes;
}

void ListLanesLayoutAlgorithm::CalculateLanes(const RefPtr<ListLayoutProperty>& layoutProperty,
    const LayoutConstraintF& layoutConstraint, std::optional<float> crossSizeOptional, Axis axis)
{
    auto contentConstraint = layoutProperty->GetContentLayoutConstraint().value();
    auto mainPercentRefer = GetMainAxisSize(contentConstraint.percentReference, axis);
    int32_t lanes = layoutProperty->GetLanes().value_or(1);
    lanes = lanes > 1 ? lanes : 1;
    if (layoutProperty->GetLaneMinLength().has_value()) {
        minLaneLength_ =
            ConvertToPx(layoutProperty->GetLaneMinLength().value(), layoutConstraint.scaleProperty, mainPercentRefer);
    }
    if (layoutProperty->GetLaneMaxLength().has_value()) {
        maxLaneLength_ =
            ConvertToPx(layoutProperty->GetLaneMaxLength().value(), layoutConstraint.scaleProperty, mainPercentRefer);
    }
    float laneGutter = 0.0f;
    if (layoutProperty->GetLaneGutter().has_value()) {
        laneGutter = ConvertToPx(
            layoutProperty->GetLaneGutter().value(), layoutConstraint.scaleProperty, crossSizeOptional.value_or(0.0))
                .value();
        SetLaneGutter(laneGutter);
    }
    lanes_ = CalculateLanesParam(minLaneLength_, maxLaneLength_, lanes, crossSizeOptional, laneGutter);
}

void ListLanesLayoutAlgorithm::ModifyLaneLength(
    std::optional<float>& minLaneLength, std::optional<float>& maxLaneLength, float crossSize)
{
    if (GreatNotEqual(minLaneLength.value(), maxLaneLength.value())) {
        TAG_LOGD(AceLogTag::ACE_LIST, "minLaneLength: %{public}f is greater than maxLaneLength: %{public}f, "
            "assign minLaneLength to maxLaneLength", minLaneLength.value(), maxLaneLength.value());
        maxLaneLength = minLaneLength;
    }
}

float ListLanesLayoutAlgorithm::CalculateLaneCrossOffset(float crossSize, float childCrossSize)
{
    if (lanes_ <= 0) {
        return 0.0f;
    }
    return ListLayoutAlgorithm::CalculateLaneCrossOffset((crossSize + GetLaneGutter()) / lanes_,
        childCrossSize / lanes_);
}

int32_t ListLanesLayoutAlgorithm::GetLazyForEachIndex(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_RETURN(host, -1);
    auto lazyForEach = AceType::DynamicCast<LazyForEachNode>(host->GetParent());
    CHECK_NULL_RETURN(lazyForEach, -1);
    return lazyForEach->GetIndexByUINode(host);
}

int32_t ListLanesLayoutAlgorithm::FindLanesStartIndex(LayoutWrapper* layoutWrapper, int32_t startIndex, int32_t index)
{
    auto wrapper = layoutWrapper->GetOrCreateChildByIndex(index, false);
    CHECK_NULL_RETURN(wrapper, index);
    if (wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG) {
        return index;
    }
    auto lazyIndex = GetLazyForEachIndex(wrapper->GetHostNode());
    if (lazyIndex > 0) {
        index -= lazyIndex;
    }
    for (int32_t idx = index; idx > startIndex; idx--) {
        auto wrapper = layoutWrapper->GetOrCreateChildByIndex(idx - 1, false);
        CHECK_NULL_RETURN(wrapper, idx);
        if (wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG) {
            return idx;
        }
    }
    if (startIndex == 0) {
        return 0;
    }
    return -1;
}

int32_t ListLanesLayoutAlgorithm::FindLanesStartIndex(LayoutWrapper* layoutWrapper, int32_t index)
{
    if (lanes_ == 1) {
        return 0;
    }
    auto it = lanesItemRange_.upper_bound(index);
    if (it == lanesItemRange_.begin()) {
        int32_t startIdx = FindLanesStartIndex(layoutWrapper, 0, index);
        lanesItemRange_[startIdx] = index;
        return startIdx;
    }
    it--;
    if (it->second >= index) {
        return it->first;
    }
    int32_t startIdx = FindLanesStartIndex(layoutWrapper, it->second, index);
    if (startIdx >= 0) {
        lanesItemRange_[startIdx] = index;
        return startIdx;
    }
    it->second = index;
    return it->first;
}

int32_t ListLanesLayoutAlgorithm::GetLanesFloor(LayoutWrapper* layoutWrapper, int32_t index)
{
    if (lanes_ > 1) {
        int32_t startIndex = FindLanesStartIndex(layoutWrapper, index);
        return index - (index - startIndex) % lanes_;
    }
    return index;
}

int32_t ListLanesLayoutAlgorithm::GetLanesCeil(LayoutWrapper* layoutWrapper, int32_t index)
{
    if (lanes_ > 1) {
        int32_t startIndex = GetLanesFloor(layoutWrapper, index);
        while (startIndex == GetLanesFloor(layoutWrapper, index + 1)) {
            index++;
        }
    }
    return index;
}

std::list<int32_t> ListLanesLayoutAlgorithm::LayoutCachedALineForward(LayoutWrapper* layoutWrapper,
    int32_t& index, float& startPos, float crossSize)
{
    std::list<int32_t> predictBuildList;
    ListLayoutAlgorithm::PositionMap posMap;
    float mainLen = 0.0f;
    bool isGroup = false;
    int32_t cnt = 0;
    int32_t lanes = lanes_ > 1 ? lanes_ : 1;
    for (int32_t i = 0; i < lanes && index + i <= GetMaxListItemIndex(); i++) {
        auto wrapper = layoutWrapper->GetChildByIndex(index + i);
        if (!wrapper) {
            predictBuildList.emplace_back(index + i);
            continue;
        }
        isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        if (isGroup && cnt > 0) {
            isGroup = false;
            break;
        }
        cnt++;
        mainLen = std::max(mainLen, GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_));
        if (isGroup) {
            break;
        }
    }
    if (cnt > 0) {
        auto endPos = startPos + mainLen;
        for (int32_t i = 0; i < cnt; i++) {
            posMap[index + i] = { startPos, endPos, isGroup };
        }
        startPos = endPos + GetSpaceWidth();
        auto startIndex = index;
        for (const auto& pos: posMap) {
            auto wrapper = layoutWrapper->GetChildByIndex(pos.first);
            if (!wrapper) {
                break;
            }
            LayoutItem(wrapper, pos.first, pos.second, startIndex, crossSize);
            SyncGeometry(wrapper);
            wrapper->SetActive(false);
        }
    }
    index += cnt + static_cast<int32_t>(predictBuildList.size());
    return predictBuildList;
}

std::list<int32_t> ListLanesLayoutAlgorithm::LayoutCachedALineBackward(LayoutWrapper* layoutWrapper,
    int32_t& index, float& endPos, float crossSize)
{
    std::list<int32_t> predictBuildList;
    ListLayoutAlgorithm::PositionMap posMap;
    float mainLen = 0.0f;
    bool isGroup = false;
    int32_t cnt = 0;
    int32_t lanes = lanes_ > 1 ? lanes_ : 1;
    for (int32_t i = 0; i < lanes && index >= 0; i++) {
        auto idx = index - i;
        auto wrapper = layoutWrapper->GetChildByIndex(idx);
        if (!wrapper) {
            predictBuildList.emplace_back(idx);
            continue;
        }
        isGroup = wrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
        if (isGroup && cnt > 0) {
            isGroup = false;
            break;
        }

        cnt++;
        mainLen = std::max(mainLen, GetMainAxisSize(wrapper->GetGeometryNode()->GetMarginFrameSize(), axis_));
        if (isGroup || (idx - FindLanesStartIndex(layoutWrapper, idx)) % lanes == 0) {
            break;
        }
    }
    if (cnt > 0) {
        auto startPos = endPos - mainLen;
        for (int32_t i = 0; i < cnt; i++) {
            posMap[index - i] = { startPos, endPos, isGroup };
        }
        endPos = startPos - GetSpaceWidth();
        auto startIndex = index - cnt + 1;
        for (const auto& pos: posMap) {
            auto wrapper = layoutWrapper->GetChildByIndex(pos.first);
            if (!wrapper) {
                break;
            }
            LayoutItem(wrapper, pos.first, pos.second, startIndex, crossSize);
            SyncGeometry(wrapper);
            wrapper->SetActive(false);
        }
    }
    index -= cnt + static_cast<int32_t>(predictBuildList.size());
    return predictBuildList;
}

std::list<int32_t> ListLanesLayoutAlgorithm::LayoutCachedItem(LayoutWrapper* layoutWrapper, int32_t cacheCount)
{
    std::list<int32_t> predictBuildList;
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    float crossSize = GetCrossAxisSize(size, axis_);

    auto itemPosition = GetItemPosition();
    auto currIndex = itemPosition.rbegin()->first + 1;
    auto currPos = itemPosition.rbegin()->second.endPos + GetSpaceWidth();
    for (int32_t i = 0; i < cacheCount && currIndex <= GetMaxListItemIndex(); i++) {
        auto tmpList = LayoutCachedALineForward(layoutWrapper, currIndex, currPos, crossSize);
        predictBuildList.merge(tmpList);
    }
    currIndex = itemPosition.begin()->first - 1;
    currPos = itemPosition.begin()->second.startPos - GetSpaceWidth();
    for (int32_t i = 0; i < cacheCount && currIndex >= 0; i++) {
        auto tmpList = LayoutCachedALineBackward(layoutWrapper, currIndex, currPos, crossSize);
        predictBuildList.merge(tmpList);
    }
    return predictBuildList;
}
} // namespace OHOS::Ace::NG
