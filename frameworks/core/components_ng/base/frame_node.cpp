/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/base/frame_node.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/point_t.h"
#include "base/log/ace_trace.h"
#include "base/log/dump_log.h"
#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/thread/cancelable_callback.h"
#include "base/thread/task_executor.h"
#include "base/utils/system_properties.h"
#include "base/utils/time_util.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_scene_status.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/target_component.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paint_wrapper.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/touch_event.h"
#include "core/gestures/gesture_info.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace {
constexpr double VISIBLE_RATIO_MIN = 0.0;
constexpr double VISIBLE_RATIO_MAX = 1.0;
constexpr int32_t SUBSTR_LENGTH = 3;
const char DIMENSION_UNIT_VP[] = "vp";
} // namespace
namespace OHOS::Ace::NG {

class FramePorxy {
public:
    struct FrameChildNode {
        RefPtr<UINode> node;
        uint32_t startIndex = 0;
        uint32_t count = 0;
    };

    struct RecursionGuard {
        FramePorxy* proxy_;
        bool inUse_;
        explicit RecursionGuard(FramePorxy* proxy) : proxy_(proxy), inUse_(proxy->inUse_)
        {
            proxy_->inUse_ = true;
        }
        ~RecursionGuard()
        {
            proxy_->inUse_ = inUse_;
            if (!proxy_->inUse_ && proxy_->delayReset_) {
                proxy_->ResetChildren(proxy_->needResetChild_);
            }
        }
    };

    RecursionGuard GetGuard()
    {
        return RecursionGuard{this};
    }

    explicit FramePorxy(FrameNode* frameNode) : hostNode_(frameNode) {}

    void Build()
    {
        if (hostNode_ == nullptr || !children_.empty()) {
            return;
        }
        totalCount_ = 0;
        auto children = hostNode_->GetChildren();
        int32_t startIndex = 0;
        int32_t count = 0;
        for (const auto& child : children) {
            count = child->FrameCount();
            children_.push_back({ child, startIndex, count });
            startIndex += count;
            totalCount_ += count;
        }
        cursor_ = children_.begin();
    }

    static void AddFrameNode(const RefPtr<UINode>& UiNode, std::list<RefPtr<LayoutWrapper>>& allFrameNodeChildren,
        std::map<uint32_t, RefPtr<LayoutWrapper>>& partFrameNodeChildren, uint32_t& count)
    {
        auto frameNode = AceType::DynamicCast<FrameNode>(UiNode);
        if (frameNode) {
            allFrameNodeChildren.emplace_back(frameNode);
            partFrameNodeChildren[count++] = frameNode;
            return;
        }
        auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(UiNode);
        if (lazyForEachNode) {
            lazyForEachNode->BuildAllChildren();
        } else {
            auto customNode = AceType::DynamicCast<CustomNode>(UiNode);
            if (customNode) {
                customNode->Render();
            }
        }
        for (const auto& child : UiNode->GetChildren()) {
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            if (frameNode) {
                allFrameNodeChildren.emplace_back(frameNode);
                partFrameNodeChildren[count++] = frameNode;
                continue;
            }
            AddFrameNode(child, allFrameNodeChildren, partFrameNodeChildren, count);
        }
    }

    const std::list<RefPtr<LayoutWrapper>>& GetAllFrameChildren()
    {
        if (!allFrameNodeChildren_.empty()) {
            return allFrameNodeChildren_;
        }
        Build();
        {
            uint32_t count = 0;
            auto guard = GetGuard();
            for (const auto& child : children_) {
                AddFrameNode(child.node, allFrameNodeChildren_, partFrameNodeChildren_, count);
            }
        }
        return allFrameNodeChildren_;
    }

    RefPtr<LayoutWrapper> FindFrameNodeByIndex(uint32_t index, bool needBuild)
    {
        while (cursor_ != children_.end()) {
            if (cursor_->startIndex > index) {
                cursor_--;
                continue;
            }

            if (cursor_->startIndex + cursor_->count > index) {
                auto frameNode = AceType::DynamicCast<FrameNode>(
                    cursor_->node->GetFrameChildByIndex(index - cursor_->startIndex, needBuild));
                return frameNode;
            }
            cursor_++;
            if (cursor_ == children_.end()) {
                cursor_ = children_.begin();
                return nullptr;
            }
        }
        return nullptr;
    }

    RefPtr<LayoutWrapper> GetFrameNodeByIndex(uint32_t index, bool needBuild)
    {
        auto itor = partFrameNodeChildren_.find(index);
        if (itor == partFrameNodeChildren_.end()) {
            Build();
            auto child = FindFrameNodeByIndex(index, needBuild);
            if (child) {
                partFrameNodeChildren_[index] = child;
                return child;
            }
            return nullptr;
        }
        return itor->second;
    }

    void ResetChildren(bool needResetChild = false)
    {
        if (inUse_) {
            delayReset_ = true;
            needResetChild_ = needResetChild;
            return;
        }
        delayReset_ = false;
        allFrameNodeChildren_.clear();
        partFrameNodeChildren_.clear();
        totalCount_ = 0;
        if (needResetChild) {
            children_.clear();
            cursor_ = children_.begin();
        }
    }

    void RemoveChildInRenderTree(uint32_t index)
    {
        auto itor = partFrameNodeChildren_.find(index);
        if (itor == partFrameNodeChildren_.end()) {
            return;
        }
        itor->second->SetActive(false);
        partFrameNodeChildren_.erase(itor);
        while (cursor_ != children_.end()) {
            if (cursor_->startIndex > index) {
                cursor_--;
                continue;
            }
            if (cursor_->startIndex + cursor_->count > index) {
                cursor_->node->DoRemoveChildInRenderTree(index - cursor_->startIndex);
                return;
            }
            cursor_++;
            if (cursor_ == children_.end()) {
                cursor_ = children_.begin();
                return;
            }
        }
    }

    void RemoveAllChildInRenderTree()
    {
        SetAllChildrenInActive();
        ResetChildren();
        Build();
        auto guard = GetGuard();
        for (const auto& child : children_) {
            child.node->DoRemoveChildInRenderTree(0, true);
        }
    }

    uint32_t GetTotalCount()
    {
        return totalCount_;
    }

    void SetAllChildrenInActive()
    {
        auto guard = GetGuard();
        for (const auto& child : partFrameNodeChildren_) {
            child.second->SetActive(false);
        }
    }

    std::string Dump()
    {
        if (totalCount_ == 0) {
            return "totalCount is 0";
        }
        std::string info = "FrameChildNode:[";
        auto guard = GetGuard();
        for (const auto& child : children_) {
            info += std::to_string(child.node->GetId());
            info += "-";
            info += std::to_string(child.startIndex);
            info += "-";
            info += std::to_string(child.count);
            info += ",";
        }
        info += "] partFrameNodeChildren:[";
        for (const auto& child : partFrameNodeChildren_) {
            info += std::to_string(child.second->GetHostNode()->GetId());
            info += ",";
        }
        info += "] TotalCount:";
        info += std::to_string(totalCount_);
        return info;
    }

    void SetCacheCount(int32_t cacheCount, const std::optional<LayoutConstraintF>& itemConstraint)
    {
        auto guard = GetGuard();
        for (const auto& child : children_) {
            child.node->OnSetCacheCount(cacheCount, itemConstraint);
        }
    }

private:
    std::list<FrameChildNode> children_;
    std::list<FrameChildNode>::iterator cursor_ = children_.begin();
    std::list<RefPtr<LayoutWrapper>> allFrameNodeChildren_;
    std::map<uint32_t, RefPtr<LayoutWrapper>> partFrameNodeChildren_;
    uint32_t totalCount_ = 0;
    FrameNode* hostNode_ { nullptr };
    bool inUse_ = false;
    bool delayReset_ = false;
    bool needResetChild_ = false;
}; // namespace OHOS::Ace::NG

FrameNode::FrameNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot)
    : UINode(tag, nodeId, isRoot), LayoutWrapper(WeakClaim(this)), pattern_(pattern),
      frameProxy_(std::make_unique<FramePorxy>(this))
{
    renderContext_->InitContext(IsRootNode(), pattern_->GetContextParam());
    paintProperty_ = pattern->CreatePaintProperty();
    layoutProperty_ = pattern->CreateLayoutProperty();
    eventHub_ = pattern->CreateEventHub();
    accessibilityProperty_ = pattern->CreateAccessibilityProperty();
    // first create make layout property dirty.
    layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    layoutProperty_->SetHost(WeakClaim(this));
}

FrameNode::~FrameNode()
{
    for (const auto& destroyCallback : destroyCallbacks_) {
        destroyCallback();
    }

    pattern_->DetachFromFrameNode(this);
    if (IsOnMainTree()) {
        OnDetachFromMainTree(false);
    }
    TriggerVisibleAreaChangeCallback(true);
    visibleAreaUserCallbacks_.clear();
    visibleAreaInnerCallbacks_.clear();
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline) {
        pipeline->RemoveOnAreaChangeNode(GetId());
        pipeline->RemoveVisibleAreaChangeNode(GetId());
        pipeline->ChangeMouseStyle(GetId(), MouseFormat::DEFAULT);
        pipeline->FreeMouseStyleHoldNode(GetId());
        pipeline->RemoveStoredNode(GetRestoreId());
        auto dragManager = pipeline->GetDragDropManager();
        if (dragManager) {
            dragManager->RemoveDragFrameNode(GetId());
#ifdef ENABLE_DRAG_FRAMEWORK
            dragManager->UnRegisterDragStatusListener(GetId());
#endif // ENABLE_DRAG_FRAMEWORK
        }
        auto frameRateManager = pipeline->GetFrameRateManager();
        if (frameRateManager) {
            frameRateManager->RemoveNodeRate(GetId());
        }
    }
}

RefPtr<FrameNode> FrameNode::CreateFrameNodeWithTree(
    const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
{
    auto newChild = CreateFrameNode(tag, nodeId, pattern, true);
    newChild->SetDepth(1);
    return newChild;
}

RefPtr<FrameNode> FrameNode::GetOrCreateFrameNode(
    const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
{
    auto frameNode = GetFrameNode(tag, nodeId);
    if (frameNode) {
        return frameNode;
    }
    auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
    return CreateFrameNode(tag, nodeId, pattern);
}

RefPtr<FrameNode> FrameNode::GetFrameNode(const std::string& tag, int32_t nodeId)
{
    auto frameNode = ElementRegister::GetInstance()->GetSpecificItemById<FrameNode>(nodeId);
    CHECK_NULL_RETURN(frameNode, nullptr);
    if (frameNode->GetTag() != tag) {
        ElementRegister::GetInstance()->RemoveItemSilently(nodeId);
        auto parent = frameNode->GetParent();
        if (parent) {
            parent->RemoveChild(frameNode);
        }
        return nullptr;
    }
    return frameNode;
}

RefPtr<FrameNode> FrameNode::CreateFrameNode(
    const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot)
{
    auto frameNode = MakeRefPtr<FrameNode>(tag, nodeId, pattern, isRoot);
    frameNode->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(frameNode);
    return frameNode;
}

void FrameNode::ProcessOffscreenNode(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    node->ProcessOffscreenTask();
    node->MarkModifyDone();
    node->UpdateLayoutPropertyFlag();
    node->SetActive();
    node->isLayoutDirtyMarked_ = true;
    node->CreateLayoutTask();

    auto paintProperty = node->GetPaintProperty<PaintProperty>();
    auto wrapper = node->CreatePaintWrapper();
    if (wrapper != nullptr) {
        wrapper->FlushRender();
    }
    paintProperty->CleanDirty();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->FlushMessages();
    node->SetActive(false);
}

void FrameNode::InitializePatternAndContext()
{
    eventHub_->AttachHost(WeakClaim(this));
    pattern_->AttachToFrameNode(WeakClaim(this));
    accessibilityProperty_->SetHost(WeakClaim(this));
    renderContext_->SetRequestFrame([weak = WeakClaim(this)] {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        if (frameNode->IsOnMainTree()) {
            auto context = frameNode->GetContext();
            CHECK_NULL_VOID(context);
            context->RequestFrame();
            return;
        }
        frameNode->hasPendingRequest_ = true;
    });
    renderContext_->SetHostNode(WeakClaim(this));
    // Initialize FocusHub
    if (pattern_->GetFocusPattern().GetFocusType() != FocusType::DISABLE) {
        GetOrCreateFocusHub();
    }
}

void FrameNode::DumpCommonInfo()
{
    auto transInfo = renderContext_->GetTrans();
    if (!geometryNode_->GetFrameRect().ToString().compare(renderContext_->GetPaintRectWithTransform().ToString())) {
        DumpLog::GetInstance().AddDesc(std::string("FrameRect: ").append(geometryNode_->GetFrameRect().ToString()));
    }
    // transInfo is a one-dimensional expansion of the affine transformation matrix
    if (!transInfo.empty()) {
        if (!(NearEqual(transInfo[0], 1) && NearEqual(transInfo[0], 1))) {
            DumpLog::GetInstance().AddDesc(std::string("scale: ").append(
                std::to_string(transInfo[0]).append(",").append(std::to_string(transInfo[1]))));
        }
        if (!(NearZero(transInfo[6]) && NearZero(transInfo[7]))) {
            DumpLog::GetInstance().AddDesc(
                std::string("translate: ")
                    .append(std::to_string(transInfo[6]).append(",").append(std::to_string(transInfo[7]))));
        }
        if (!(NearZero(transInfo[8]))) {
            DumpLog::GetInstance().AddDesc(std::string("degree: ").append(std::to_string(transInfo[8])));
        }
    }
    if (renderContext_->GetBackgroundColor()->ColorToString().compare("#00000000") != 0) {
        DumpLog::GetInstance().AddDesc(
            std::string("BackgroundColor: ").append(renderContext_->GetBackgroundColor()->ColorToString()));
    }
    if (geometryNode_->GetParentLayoutConstraint().has_value())
        DumpLog::GetInstance().AddDesc(std::string("ParentLayoutConstraint: ")
                                           .append(geometryNode_->GetParentLayoutConstraint().value().ToString()));
    if (!(NearZero(GetOffsetRelativeToWindow().GetY()) && NearZero(GetOffsetRelativeToWindow().GetX()))) {
        DumpLog::GetInstance().AddDesc(std::string("top: ")
                                           .append(std::to_string(GetOffsetRelativeToWindow().GetY()))
                                           .append(" left: ")
                                           .append(std::to_string(GetOffsetRelativeToWindow().GetX())));
    }
    if (static_cast<int32_t>(IsActive()) != 1) {
        DumpLog::GetInstance().AddDesc(
            std::string("Active: ").append(std::to_string(static_cast<int32_t>(IsActive()))));
    }

    if (static_cast<int32_t>(layoutProperty_->GetVisibility().value_or(VisibleType::VISIBLE)) != 0) {
        DumpLog::GetInstance().AddDesc(std::string("Visible: ")
                                           .append(std::to_string(static_cast<int32_t>(
                                               layoutProperty_->GetVisibility().value_or(VisibleType::VISIBLE)))));
    }
    if (layoutProperty_->GetPaddingProperty()) {
        DumpLog::GetInstance().AddDesc(
            std::string("Padding: ").append(layoutProperty_->GetPaddingProperty()->ToString().c_str()));
    }
    if (layoutProperty_->GetBorderWidthProperty()) {
        DumpLog::GetInstance().AddDesc(
            std::string("Border: ").append(layoutProperty_->GetBorderWidthProperty()->ToString().c_str()));
    }
    if (layoutProperty_->GetMarginProperty()) {
        DumpLog::GetInstance().AddDesc(
            std::string("Margin: ").append(layoutProperty_->GetMarginProperty()->ToString().c_str()));
    }
    if (layoutProperty_->GetCalcLayoutConstraint()) {
        DumpLog::GetInstance().AddDesc(std::string("User defined constraint: ")
                                           .append(layoutProperty_->GetCalcLayoutConstraint()->ToString().c_str()));
    }
    if (!propInspectorId_->empty()) {
        DumpLog::GetInstance().AddDesc(std::string("compid: ").append(propInspectorId_.value_or("")));
    }
    if (layoutProperty_->GetPaddingProperty() || layoutProperty_->GetBorderWidthProperty() ||
        layoutProperty_->GetMarginProperty() || layoutProperty_->GetCalcLayoutConstraint()) {
        DumpLog::GetInstance().AddDesc(
            std::string("ContentConstraint: ")
                .append(layoutProperty_->GetContentLayoutConstraint().has_value() ?
                            layoutProperty_->GetContentLayoutConstraint().value().ToString() : "NA"));
    }
    DumpOverlayInfo();
    if (frameProxy_->Dump().compare("totalCount is 0") != 0) {
        DumpLog::GetInstance().AddDesc(std::string("FrameProxy: ").append(frameProxy_->Dump().c_str()));
    }
}

void FrameNode::DumpOverlayInfo()
{
    if (!layoutProperty_->IsOverlayNode()) {
        return;
    }
    DumpLog::GetInstance().AddDesc(std::string("IsOverlayNode: ").append(std::string("true")));
    Dimension offsetX, offsetY;
    layoutProperty_->GetOverlayOffset(offsetX, offsetY);
    DumpLog::GetInstance().AddDesc(
        std::string("OverlayOffset: ").append(offsetX.ToString()).append(std::string(", ")).append(offsetY.ToString()));
}

void FrameNode::DumpInfo()
{
    DumpCommonInfo();
    if (pattern_) {
        pattern_->DumpInfo();
    }
    if (renderContext_) {
        renderContext_->DumpInfo();
    }
}

void FrameNode::DumpAdvanceInfo()
{
    DumpCommonInfo();
    if (pattern_) {
        pattern_->DumpInfo();
        pattern_->DumpAdvanceInfo();
    }
    if (renderContext_) {
        renderContext_->DumpInfo();
    }
}

void FrameNode::DumpViewDataPageNode(RefPtr<ViewDataWrap> viewDataWrap)
{
    if (pattern_) {
        pattern_->DumpViewDataPageNode(viewDataWrap);
    }
}

bool FrameNode::CheckAutoSave()
{
    if (pattern_) {
        return pattern_->CheckAutoSave();
    }
    return false;
}

void FrameNode::FocusToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    bool enabled = true;
    bool focusable = false;
    bool focused = false;
    bool defaultFocus = false;
    bool groupDefaultFocus = false;
    bool focusOnTouch = false;
    int32_t tabIndex = 0;
    auto focusHub = GetFocusHub();
    if (focusHub) {
        enabled = focusHub->IsEnabled();
        focusable = focusHub->IsFocusable();
        focused = focusHub->IsCurrentFocus();
        defaultFocus = focusHub->IsDefaultFocus();
        groupDefaultFocus = focusHub->IsDefaultGroupFocus();
        focusOnTouch = focusHub->IsFocusOnTouch().value_or(false);
        tabIndex = focusHub->GetTabIndex();
    }
    json->Put("enabled", enabled);
    json->Put("focusable", focusable);
    json->Put("focused", focused);
    json->Put("defaultFocus", defaultFocus);
    json->Put("groupDefaultFocus", groupDefaultFocus);
    json->Put("focusOnTouch", focusOnTouch);
    json->Put("tabIndex", tabIndex);
}

void FrameNode::MouseToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    std::string hoverEffect = "HoverEffect.Auto";
    auto inputEventHub = GetOrCreateInputEventHub();
    if (inputEventHub) {
        hoverEffect = inputEventHub->GetHoverEffectStr();
    }
    json->Put("hoverEffect", hoverEffect.c_str());
}

void FrameNode::TouchToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    bool touchable = true;
    std::string hitTestMode = "HitTestMode.Default";
    auto gestureEventHub = GetOrCreateGestureEventHub();
    std::vector<DimensionRect> responseRegion;
    std::vector<DimensionRect> mouseResponseRegion;
    if (gestureEventHub) {
        touchable = gestureEventHub->GetTouchable();
        hitTestMode = gestureEventHub->GetHitTestModeStr();
        responseRegion = gestureEventHub->GetResponseRegion();
        mouseResponseRegion = gestureEventHub->GetMouseResponseRegion();
    }
    json->Put("touchable", touchable);
    json->Put("hitTestBehavior", hitTestMode.c_str());
    auto jsArr = JsonUtil::CreateArray(true);
    for (int32_t i = 0; i < static_cast<int32_t>(responseRegion.size()); ++i) {
        auto iStr = std::to_string(i);
        jsArr->Put(iStr.c_str(), responseRegion[i].ToJsonString().c_str());
    }
    json->Put("responseRegion", jsArr);
    for (int32_t i = 0; i < static_cast<int32_t>(mouseResponseRegion.size()); ++i) {
        auto iStr = std::to_string(i);
        jsArr->Put(iStr.c_str(), mouseResponseRegion[i].ToJsonString().c_str());
    }
    json->Put("mouseResponseRegion", jsArr);
}

void FrameNode::GeometryNodeToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    bool hasIdealWidth = false;
    bool hasIdealHeight = false;
    if (layoutProperty_ && layoutProperty_->GetCalcLayoutConstraint()) {
        auto selfIdealSize = layoutProperty_->GetCalcLayoutConstraint()->selfIdealSize;
        hasIdealWidth = selfIdealSize.has_value() && selfIdealSize.value().Width().has_value();
        hasIdealHeight = selfIdealSize.has_value() && selfIdealSize.value().Height().has_value();
    }

    auto jsonSize = json->GetValue("size");
    if (!hasIdealWidth) {
        auto idealWidthVpStr = std::to_string(Dimension(geometryNode_->GetFrameSize().Width()).ConvertToVp());
        auto widthStr =
            (idealWidthVpStr.substr(0, idealWidthVpStr.find(".") + SUBSTR_LENGTH) + DIMENSION_UNIT_VP);
        json->Put("width", widthStr.c_str());
        if (jsonSize) {
            jsonSize->Put("width", widthStr.c_str());
        }
    }

    if (!hasIdealHeight) {
        auto idealHeightVpStr = std::to_string(Dimension(geometryNode_->GetFrameSize().Height()).ConvertToVp());
        auto heightStr =
            (idealHeightVpStr.substr(0, idealHeightVpStr.find(".") + SUBSTR_LENGTH) + DIMENSION_UNIT_VP);
        json->Put("height", heightStr.c_str());
        if (jsonSize) {
            jsonSize->Put("height", heightStr.c_str());
        }
    }
}

void FrameNode::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    if (renderContext_) {
        renderContext_->ToJsonValue(json);
    }
    // scrollable in AccessibilityProperty
    ACE_PROPERTY_TO_JSON_VALUE(accessibilityProperty_, AccessibilityProperty);
    ACE_PROPERTY_TO_JSON_VALUE(layoutProperty_, LayoutProperty);
    ACE_PROPERTY_TO_JSON_VALUE(paintProperty_, PaintProperty);
    ACE_PROPERTY_TO_JSON_VALUE(pattern_, Pattern);
    if (eventHub_) {
        eventHub_->ToJsonValue(json);
    }
    FocusToJsonValue(json);
    MouseToJsonValue(json);
    TouchToJsonValue(json);
    if (Container::LessThanAPIVersion(PlatformVersion::VERSION_ELEVEN)) {
#if defined(PREVIEW)
        GeometryNodeToJsonValue(json);
#endif
    } else {
        GeometryNodeToJsonValue(json);
    }
    json->Put("id", propInspectorId_.value_or("").c_str());
}

void FrameNode::FromJson(const std::unique_ptr<JsonValue>& json)
{
    if (renderContext_) {
        renderContext_->FromJson(json);
    }
    accessibilityProperty_->FromJson(json);
    layoutProperty_->FromJson(json);
    paintProperty_->FromJson(json);
    pattern_->FromJson(json);
    if (eventHub_) {
        eventHub_->FromJson(json);
    }
}

void FrameNode::OnAttachToMainTree(bool recursive)
{
    eventHub_->FireOnAppear();
    renderContext_->OnNodeAppear(recursive);
    pattern_->OnAttachToMainTree();
    if (IsResponseRegion() || HasPositionProp()) {
        auto parent = GetParent();
        while (parent) {
            auto frameNode = AceType::DynamicCast<FrameNode>(parent);
            if (frameNode) {
                frameNode->MarkResponseRegion(true);
            }
            parent = parent->GetParent();
        }
    }
    // node may have been measured before AttachToMainTree
    if (geometryNode_->GetParentLayoutConstraint().has_value() && !UseOffscreenProcess()) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE_SELF);
    }

    UINode::OnAttachToMainTree(recursive);

    if (!hasPendingRequest_) {
        return;
    }
    auto context = GetContext();
    CHECK_NULL_VOID(context);
    context->RequestFrame();
    hasPendingRequest_ = false;
}

void FrameNode::OnConfigurationUpdate(const OnConfigurationChange& configurationChange)
{
    if (configurationChange.languageUpdate) {
        pattern_->OnLanguageConfigurationUpdate();
        MarkModifyDone();
        MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
    if (configurationChange.colorModeUpdate) {
        pattern_->OnColorConfigurationUpdate();
        MarkModifyDone();
        MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }
    if (configurationChange.directionUpdate) {
        pattern_->OnDirectionConfigurationUpdate();
        MarkModifyDone();
        MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }
    if (configurationChange.dpiUpdate) {
        pattern_->OnDpiConfigurationUpdate();
        MarkModifyDone();
        MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }
}

void FrameNode::OnVisibleChange(bool isVisible)
{
    pattern_->OnVisibleChange(isVisible);
    UpdateChildrenVisible(isVisible);
    TriggerVisibleAreaChangeCallback(true);
}

void FrameNode::OnDetachFromMainTree(bool recursive)
{
    if (auto focusHub = GetFocusHub()) {
        focusHub->RemoveSelf();
    }
    eventHub_->FireOnDisappear();
    renderContext_->OnNodeDisappear(recursive);
}

void FrameNode::SwapDirtyLayoutWrapperOnMainThread(const RefPtr<LayoutWrapper>& dirty)
{
    CHECK_NULL_VOID(dirty);

    // update new layout constrain.
    layoutProperty_->UpdateLayoutConstraint(dirty->GetLayoutProperty());

    // active change flag judge.
    SetActive(dirty->IsActive());
    if (!isActive_) {
        return;
    }

    // update layout size.
    bool frameSizeChange = geometryNode_->GetFrameSize() != dirty->GetGeometryNode()->GetFrameSize();
    bool frameOffsetChange = geometryNode_->GetFrameOffset() != dirty->GetGeometryNode()->GetFrameOffset();
    bool contentSizeChange = geometryNode_->GetContentSize() != dirty->GetGeometryNode()->GetContentSize();
    bool contentOffsetChange = geometryNode_->GetContentOffset() != dirty->GetGeometryNode()->GetContentOffset();

    SetGeometryNode(dirty->GetGeometryNode());

    const auto& geometryTransition = layoutProperty_->GetGeometryTransition();
    if (geometryTransition != nullptr && geometryTransition->IsRunning(WeakClaim(this))) {
        geometryTransition->DidLayout(dirty);
        if (geometryTransition->IsNodeOutAndActive(WeakClaim(this))) {
            isLayoutDirtyMarked_ = true;
        }
    } else if (frameSizeChange || frameOffsetChange || HasPositionProp() ||
               (pattern_->GetContextParam().has_value() && contentSizeChange)) {
        renderContext_->SyncGeometryProperties(RawPtr(dirty->GetGeometryNode()));
    }

    // clean layout flag.
    layoutProperty_->CleanDirty();
    DirtySwapConfig config { frameSizeChange, frameOffsetChange, contentSizeChange, contentOffsetChange };
    // check if need to paint content.
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_VOID(layoutAlgorithmWrapper);
    config.skipMeasure = layoutAlgorithmWrapper->SkipMeasure() || dirty->SkipMeasureContent();
    config.skipLayout = layoutAlgorithmWrapper->SkipLayout();
    if ((config.skipMeasure == false) && (config.skipLayout == false) && GetInspectorId().has_value()) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        pipeline->OnLayoutCompleted(GetInspectorId()->c_str());
    }
    auto needRerender = pattern_->OnDirtyLayoutWrapperSwap(dirty, config);
    // TODO: temp use and need to delete.
    needRerender = needRerender || pattern_->OnDirtyLayoutWrapperSwap(dirty, config.skipMeasure, config.skipLayout);
    if (needRerender || CheckNeedRender(paintProperty_->GetPropertyChangeFlag())) {
        MarkDirtyNode(true, true, PROPERTY_UPDATE_RENDER);
    }

    // update border.
    if (layoutProperty_->GetBorderWidthProperty()) {
        if (!renderContext_->HasBorderColor()) {
            BorderColorProperty borderColorProperty;
            borderColorProperty.SetColor(Color::BLACK);
            renderContext_->UpdateBorderColor(borderColorProperty);
        }
        if (!renderContext_->HasBorderStyle()) {
            BorderStyleProperty borderStyleProperty;
            borderStyleProperty.SetBorderStyle(BorderStyle::SOLID);
            renderContext_->UpdateBorderStyle(borderStyleProperty);
        }
        if (layoutProperty_->GetLayoutConstraint().has_value()) {
            renderContext_->UpdateBorderWidthF(ConvertToBorderWidthPropertyF(layoutProperty_->GetBorderWidthProperty(),
                ScaleProperty::CreateScaleProperty(),
                layoutProperty_->GetLayoutConstraint()->percentReference.Width()));
        } else {
            renderContext_->UpdateBorderWidthF(ConvertToBorderWidthPropertyF(layoutProperty_->GetBorderWidthProperty(),
                ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth()));
        }
    }

    // update background
    if (builderFunc_) {
        auto builderNode = builderFunc_();
        auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        builderNode->MountToParent(columnNode);
        SetBackgroundLayoutConstraint(columnNode);
        renderContext_->CreateBackgroundPixelMap(columnNode);
        builderFunc_ = nullptr;
        backgroundNode_ = columnNode;
    }

    // update focus state
    auto focusHub = GetFocusHub();
    if (focusHub && focusHub->IsCurrentFocus()) {
        focusHub->ClearFocusState(false);
        focusHub->PaintFocusState(false);
    }

    // rebuild child render node.
    RebuildRenderContextTree();
}

void FrameNode::SetBackgroundLayoutConstraint(const RefPtr<FrameNode>& customNode)
{
    CHECK_NULL_VOID(customNode);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.scaleProperty = ScaleProperty::CreateScaleProperty();
    layoutConstraint.percentReference.SetWidth(geometryNode_->GetFrameSize().Width());
    layoutConstraint.percentReference.SetHeight(geometryNode_->GetFrameSize().Height());
    layoutConstraint.maxSize.SetWidth(geometryNode_->GetFrameSize().Width());
    layoutConstraint.maxSize.SetHeight(geometryNode_->GetFrameSize().Height());
    customNode->GetGeometryNode()->SetParentLayoutConstraint(layoutConstraint);
}

void FrameNode::AdjustGridOffset()
{
    if (!isActive_) {
        return;
    }
    if (layoutProperty_->UpdateGridOffset(Claim(this))) {
        renderContext_->UpdateOffset(OffsetT<Dimension>());
        renderContext_->UpdateAnchor(OffsetT<Dimension>());
        renderContext_->SyncGeometryProperties(RawPtr(GetGeometryNode()));
    }
}

void FrameNode::ClearUserOnAreaChange()
{
    if (eventHub_) {
        eventHub_->ClearUserOnAreaChanged();
    }
}

void FrameNode::SetOnAreaChangeCallback(OnAreaChangedFunc&& callback)
{
    if (!lastFrameRect_) {
        lastFrameRect_ = std::make_unique<RectF>();
    }
    if (!lastParentOffsetToWindow_) {
        lastParentOffsetToWindow_ = std::make_unique<OffsetF>();
    }
    eventHub_->SetOnAreaChanged(std::move(callback));
}

void FrameNode::TriggerOnAreaChangeCallback()
{
    if (eventHub_->HasOnAreaChanged() && lastFrameRect_ && lastParentOffsetToWindow_) {
        auto currFrameRect = geometryNode_->GetFrameRect();
        auto currParentOffsetToWindow = GetOffsetRelativeToWindow() - currFrameRect.GetOffset();
        if (currFrameRect != *lastFrameRect_ || currParentOffsetToWindow != *lastParentOffsetToWindow_) {
            eventHub_->FireOnAreaChanged(
                *lastFrameRect_, *lastParentOffsetToWindow_, currFrameRect, currParentOffsetToWindow);
            *lastFrameRect_ = currFrameRect;
            *lastParentOffsetToWindow_ = currParentOffsetToWindow;
        }
    }
    pattern_->OnAreaChangedInner();
}

void FrameNode::TriggerVisibleAreaChangeCallback(bool forceDisappear)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    bool isFrameDisappear = forceDisappear || !context->GetOnShow() || !IsOnMainTree();
    if (!isFrameDisappear) {
        bool curFrameIsActive = isActive_;
        bool curIsVisible = IsVisible();
        auto parent = GetParent();
        while (parent) {
            auto parentFrame = AceType::DynamicCast<FrameNode>(parent);
            if (!parentFrame) {
                parent = parent->GetParent();
                continue;
            }
            if (!parentFrame->isActive_) {
                curFrameIsActive = false;
                break;
            }
            if (!parentFrame->IsVisible()) {
                curIsVisible = false;
                break;
            }
            parent = parent->GetParent();
        }
        isFrameDisappear = !curIsVisible || !curFrameIsActive;
    }

    if (isFrameDisappear) {
        if (!NearEqual(lastVisibleRatio_, VISIBLE_RATIO_MIN)) {
            ProcessAllVisibleCallback(visibleAreaUserCallbacks_, VISIBLE_RATIO_MIN);
            ProcessAllVisibleCallback(visibleAreaInnerCallbacks_, VISIBLE_RATIO_MIN);
            lastVisibleRatio_ = VISIBLE_RATIO_MIN;
        }
        return;
    }

    auto frameRect = GetTransformRectRelativeToWindow();
    auto visibleRect = frameRect;
    RectF parentRect;
    auto parentUi = GetParent();
    if (!parentUi) {
        visibleRect.SetWidth(0.0f);
        visibleRect.SetHeight(0.0f);
    }
    while (parentUi) {
        auto parentFrame = AceType::DynamicCast<FrameNode>(parentUi);
        if (!parentFrame) {
            parentUi = parentUi->GetParent();
            continue;
        }
        parentRect = parentFrame->GetTransformRectRelativeToWindow();
        visibleRect = visibleRect.Constrain(parentRect);
        parentUi = parentUi->GetParent();
    }

    double currentVisibleRatio =
        std::clamp(CalculateCurrentVisibleRatio(visibleRect, frameRect), VISIBLE_RATIO_MIN, VISIBLE_RATIO_MAX);
    if (!NearEqual(currentVisibleRatio, lastVisibleRatio_)) {
        ProcessAllVisibleCallback(visibleAreaUserCallbacks_, currentVisibleRatio);
        ProcessAllVisibleCallback(visibleAreaInnerCallbacks_, currentVisibleRatio);
        lastVisibleRatio_ = currentVisibleRatio;
    }
}

double FrameNode::CalculateCurrentVisibleRatio(const RectF& visibleRect, const RectF& renderRect)
{
    if (!visibleRect.IsValid() || !renderRect.IsValid()) {
        return 0.0;
    }
    return visibleRect.Width() * visibleRect.Height() / (renderRect.Width() * renderRect.Height());
}

void FrameNode::ProcessAllVisibleCallback(
    std::unordered_map<double, VisibleCallbackInfo>& visibleAreaCallbacks, double currentVisibleRatio)
{
    bool isHandled = false;
    for (auto& nodeCallbackInfo : visibleAreaCallbacks) {
        auto callbackRatio = nodeCallbackInfo.first;
        auto callbackIsVisible = nodeCallbackInfo.second.isCurrentVisible;
        if (GreatNotEqual(currentVisibleRatio, callbackRatio) && !callbackIsVisible) {
            OnVisibleAreaChangeCallback(nodeCallbackInfo.second, true, currentVisibleRatio, isHandled);
            isHandled = true;
            continue;
        }

        if (LessNotEqual(currentVisibleRatio, callbackRatio) && callbackIsVisible) {
            OnVisibleAreaChangeCallback(nodeCallbackInfo.second, false, currentVisibleRatio, isHandled);
            isHandled = true;
            continue;
        }

        if (NearEqual(currentVisibleRatio, callbackRatio) && NearEqual(callbackRatio, VISIBLE_RATIO_MIN)) {
            if (callbackIsVisible) {
                OnVisibleAreaChangeCallback(nodeCallbackInfo.second, false, VISIBLE_RATIO_MIN, isHandled);
            } else {
                OnVisibleAreaChangeCallback(nodeCallbackInfo.second, true, VISIBLE_RATIO_MIN, isHandled);
            }
            isHandled = true;
        } else if (NearEqual(currentVisibleRatio, callbackRatio) && NearEqual(callbackRatio, VISIBLE_RATIO_MAX)) {
            if (!callbackIsVisible) {
                OnVisibleAreaChangeCallback(nodeCallbackInfo.second, true, VISIBLE_RATIO_MAX, isHandled);
            } else {
                OnVisibleAreaChangeCallback(nodeCallbackInfo.second, false, VISIBLE_RATIO_MAX, isHandled);
            }
            isHandled = true;
        }
    }
}

void FrameNode::OnVisibleAreaChangeCallback(
    VisibleCallbackInfo& callbackInfo, bool visibleType, double currentVisibleRatio, bool isHandled)
{
    callbackInfo.isCurrentVisible = visibleType;
    if (callbackInfo.callback && !isHandled) {
        callbackInfo.callback(visibleType, currentVisibleRatio);
    }
}

void FrameNode::SetActive(bool active)
{
    bool activeChanged = false;
    if (active && !isActive_) {
        pattern_->OnActive();
        isActive_ = true;
        activeChanged = true;
    }
    if (!active && isActive_) {
        pattern_->OnInActive();
        isActive_ = false;
        activeChanged = true;
    }
    if (activeChanged) {
        auto parent = GetAncestorNodeOfFrame();
        if (parent) {
            parent->MarkNeedSyncRenderTree();
        }
    }
    if (GetTag() == V2::TAB_CONTENT_ITEM_ETS_TAG) {
        SetJSViewActive(active);
    }
}

void FrameNode::SetGeometryNode(const RefPtr<GeometryNode>& node)
{
    geometryNode_ = node;
}

void FrameNode::CreateLayoutTask(bool forceUseMainThread)
{
    if (!isLayoutDirtyMarked_) {
        return;
    }
    SetRootMeasureNode(true);
    UpdateLayoutPropertyFlag();
    SetSkipSyncGeometryNode(false);
    {
        ACE_SCOPED_TRACE("Layout[%s][self:%d][parent:%d]", GetTag().c_str(), GetId(),
            GetParent() ? GetParent()->GetId() : 0);
        Measure(GetLayoutConstraint());
        Layout();
    }
    SetRootMeasureNode(false);
}

std::optional<UITask> FrameNode::CreateRenderTask(bool forceUseMainThread)
{
    if (!isRenderDirtyMarked_) {
        return std::nullopt;
    }
    auto wrapper = CreatePaintWrapper();
    CHECK_NULL_RETURN(wrapper, std::nullopt);
    auto task = [weak = WeakClaim(this), wrapper, paintProperty = paintProperty_]() {
        ACE_SCOPED_TRACE("FrameNode::RenderTask");
        auto self = weak.Upgrade();
        wrapper->FlushRender();
        paintProperty->CleanDirty();

        if (self->GetInspectorId()) {
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            pipeline->SetNeedRenderNode(self);
        }
    };
    if (forceUseMainThread || wrapper->CheckShouldRunOnMain()) {
        return UITask(std::move(task), MAIN_TASK);
    }
    return UITask(std::move(task), wrapper->CanRunOnWhichThread());
}

LayoutConstraintF FrameNode::GetLayoutConstraint() const
{
    if (geometryNode_->GetParentLayoutConstraint().has_value()) {
        return geometryNode_->GetParentLayoutConstraint().value();
    }
    LayoutConstraintF layoutConstraint;
    layoutConstraint.scaleProperty = ScaleProperty::CreateScaleProperty();
    auto rootWidth = PipelineContext::GetCurrentRootWidth();
    auto rootHeight = PipelineContext::GetCurrentRootHeight();
    layoutConstraint.percentReference.SetWidth(rootWidth);
    layoutConstraint.percentReference.SetHeight(rootHeight);
    layoutConstraint.maxSize.SetWidth(rootWidth);
    layoutConstraint.maxSize.SetHeight(rootHeight);
    return layoutConstraint;
}

OffsetF FrameNode::GetParentGlobalOffset() const
{
    auto parent = GetAncestorNodeOfFrame();
    if (!parent) {
        return { 0.0f, 0.0f };
    }
    return parent->geometryNode_->GetParentGlobalOffset();
}

void FrameNode::UpdateLayoutPropertyFlag()
{
    auto selfFlag = layoutProperty_->GetPropertyChangeFlag();
    if (!CheckUpdateByChildRequest(selfFlag)) {
        return;
    }
    if (CheckForceParentMeasureFlag(selfFlag)) {
        return;
    }
    auto flag = PROPERTY_UPDATE_NORMAL;
    const auto& children = GetChildren();
    for (const auto& child : children) {
        child->UpdateLayoutPropertyFlag();
        child->AdjustParentLayoutFlag(flag);
        if (CheckForceParentMeasureFlag(selfFlag)) {
            break;
        }
    }
    if (CheckForceParentMeasureFlag(flag)) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
}

void FrameNode::ForceUpdateLayoutPropertyFlag(PropertyChangeFlag propertyChangeFlag)
{
    layoutProperty_->UpdatePropertyChangeFlag(propertyChangeFlag);
}

void FrameNode::AdjustParentLayoutFlag(PropertyChangeFlag& flag)
{
    flag = flag | layoutProperty_->GetPropertyChangeFlag();
}

RefPtr<LayoutWrapperNode> FrameNode::CreateLayoutWrapper(bool forceMeasure, bool forceLayout)
{
    return UpdateLayoutWrapper(nullptr, forceMeasure, forceLayout);
}

RefPtr<LayoutWrapperNode> FrameNode::UpdateLayoutWrapper(
    RefPtr<LayoutWrapperNode> layoutWrapper, bool forceMeasure, bool forceLayout)
{
    CHECK_NULL_RETURN(layoutProperty_, nullptr);
    CHECK_NULL_RETURN(pattern_, nullptr);
    if (layoutProperty_->GetVisibility().value_or(VisibleType::VISIBLE) == VisibleType::GONE) {
        if (!layoutWrapper) {
            layoutWrapper =
                MakeRefPtr<LayoutWrapperNode>(WeakClaim(this), MakeRefPtr<GeometryNode>(), layoutProperty_->Clone());
        } else {
            layoutWrapper->Update(WeakClaim(this), MakeRefPtr<GeometryNode>(), layoutProperty_->Clone());
        }
        layoutWrapper->SetLayoutAlgorithm(MakeRefPtr<LayoutAlgorithmWrapper>(nullptr, true, true));
        isLayoutDirtyMarked_ = false;
        return layoutWrapper;
    }

    pattern_->BeforeCreateLayoutWrapper();
    if (forceMeasure) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
    if (forceLayout) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_LAYOUT);
    }
    auto flag = layoutProperty_->GetPropertyChangeFlag();
    // It is necessary to copy the layoutProperty property to prevent the layoutProperty property from being
    // modified during the layout process, resulting in the problem of judging whether the front-end setting value
    // changes the next time js is executed.
    if (!layoutWrapper) {
        layoutWrapper =
            MakeRefPtr<LayoutWrapperNode>(WeakClaim(this), geometryNode_->Clone(), layoutProperty_->Clone());
    } else {
        layoutWrapper->Update(WeakClaim(this), geometryNode_->Clone(), layoutProperty_->Clone());
    }
    LOGD("%{public}s create layout wrapper: flag = %{public}x, forceMeasure = %{public}d, forceLayout = %{public}d",
        GetTag().c_str(), flag, forceMeasure, forceLayout);
    do {
        if (CheckNeedMeasure(flag) || forceMeasure) {
            layoutWrapper->SetLayoutAlgorithm(MakeRefPtr<LayoutAlgorithmWrapper>(pattern_->CreateLayoutAlgorithm()));
            bool forceChildMeasure = CheckMeasureFlag(flag) || CheckMeasureSelfAndChildFlag(flag) || forceMeasure;
            UpdateChildrenLayoutWrapper(layoutWrapper, forceChildMeasure, false);
            break;
        }
        if (CheckNeedLayout(flag) || forceLayout) {
            layoutWrapper->SetLayoutAlgorithm(
                MakeRefPtr<LayoutAlgorithmWrapper>(pattern_->CreateLayoutAlgorithm(), true, false));
            UpdateChildrenLayoutWrapper(layoutWrapper, false, false);
            break;
        }
        layoutWrapper->SetLayoutAlgorithm(MakeRefPtr<LayoutAlgorithmWrapper>(nullptr, true, true));
    } while (false);
    // check position flag.
    layoutWrapper->SetOutOfLayout(renderContext_->HasPosition());
    layoutWrapper->SetActive(isActive_);
    layoutWrapper->SetIsOverlayNode(layoutProperty_->IsOverlayNode());
    isLayoutDirtyMarked_ = false;
    return layoutWrapper;
}

void FrameNode::UpdateChildrenLayoutWrapper(const RefPtr<LayoutWrapperNode>& self, bool forceMeasure, bool forceLayout)
{
    const auto& children = GetChildren();
    for (const auto& child : children) {
        child->AdjustLayoutWrapperTree(self, forceMeasure, forceLayout);
    }
}

void FrameNode::AdjustLayoutWrapperTree(const RefPtr<LayoutWrapperNode>& parent, bool forceMeasure, bool forceLayout)
{
    ACE_DCHECK(parent);
    CHECK_NULL_VOID(layoutProperty_);
    const auto& geometryTransition = layoutProperty_->GetGeometryTransition();
    if (geometryTransition != nullptr && geometryTransition->IsNodeOutAndActive(WeakClaim(this))) {
        return;
    }
    auto layoutWrapper = CreateLayoutWrapper(forceMeasure, forceLayout);
    parent->AppendChild(layoutWrapper, layoutProperty_->IsOverlayNode());
}

RefPtr<PaintWrapper> FrameNode::CreatePaintWrapper()
{
    pattern_->BeforeCreatePaintWrapper();
    isRenderDirtyMarked_ = false;
    auto paintMethod = pattern_->CreateNodePaintMethod();
    // It is necessary to copy the layoutProperty property to prevent the paintProperty_ property from being
    // modified during the paint process, resulting in the problem of judging whether the front-end setting value
    // changes the next time js is executed.
    if (paintMethod) {
        auto paintWrapper = MakeRefPtr<PaintWrapper>(renderContext_, geometryNode_->Clone(), paintProperty_->Clone());
        paintWrapper->SetNodePaintMethod(paintMethod);
        return paintWrapper;
    }
    if (renderContext_->GetAccessibilityFocus().value_or(false)) {
        auto paintWrapper = MakeRefPtr<PaintWrapper>(renderContext_, geometryNode_->Clone(), paintProperty_->Clone());
        paintWrapper->SetNodePaintMethod(MakeRefPtr<NodePaintMethod>());
        return paintWrapper;
    }
    return nullptr;
}

void FrameNode::PostTask(std::function<void()>&& task, TaskExecutor::TaskType taskType)
{
    auto context = GetContext();
    CHECK_NULL_VOID(context);
    context->PostAsyncEvent(std::move(task), taskType);
}

void FrameNode::UpdateLayoutConstraint(const MeasureProperty& calcLayoutConstraint)
{
    layoutProperty_->UpdateCalcLayoutProperty(calcLayoutConstraint);
}

void FrameNode::RebuildRenderContextTree()
{
    if (!needSyncRenderTree_) {
        return;
    }
    frameChildren_.clear();
    std::list<RefPtr<FrameNode>> children;
    // generate full children list, including disappear children.
    GenerateOneDepthVisibleFrameWithTransition(children);
    if (overlayNode_) {
        children.push_back(overlayNode_);
    }
    for (const auto& child : children) {
        frameChildren_.emplace(child);
    }
    renderContext_->RebuildFrame(this, children);
    pattern_->OnRebuildFrame();
    needSyncRenderTree_ = false;
}

void FrameNode::MarkModifyDone()
{
    pattern_->OnModifyDone();
    // restore info will overwrite the first setted attribute
    if (!isRestoreInfoUsed_) {
        isRestoreInfoUsed_ = true;
        auto pipeline = PipelineContext::GetCurrentContext();
        int32_t restoreId = GetRestoreId();
        if (pipeline && restoreId >= 0) {
            // store distribute node
            pipeline->StoreNode(restoreId, AceType::WeakClaim(this));
            // restore distribute node info
            std::string restoreInfo;
            if (pipeline->GetRestoreInfo(restoreId, restoreInfo)) {
                pattern_->OnRestoreInfo(restoreInfo);
            }
        }
    }
    eventHub_->MarkModifyDone();
    if (IsResponseRegion() || HasPositionProp()) {
        auto parent = GetParent();
        while (parent) {
            auto frameNode = AceType::DynamicCast<FrameNode>(parent);
            if (frameNode) {
                frameNode->MarkResponseRegion(true);
            }
            parent = parent->GetParent();
        }
    }
    renderContext_->OnModifyDone();
}

void FrameNode::OnMountToParentDone()
{
    pattern_->OnMountToParentDone();
}

void FrameNode::FlushUpdateAndMarkDirty()
{
    MarkDirtyNode();
}

void FrameNode::MarkDirtyNode(PropertyChangeFlag extraFlag)
{
    MarkDirtyNode(IsMeasureBoundary(), IsRenderBoundary(), extraFlag);
}

RefPtr<FrameNode> FrameNode::GetAncestorNodeOfFrame() const
{
    auto parent = GetParent();
    while (parent) {
        if (InstanceOf<FrameNode>(parent)) {
            return DynamicCast<FrameNode>(parent);
        }
        parent = parent->GetParent();
    }
    return nullptr;
}

RefPtr<FrameNode> FrameNode::GetPageNode()
{
    if (GetTag() == "page") {
        return Claim(this);
    }
    auto parent = GetParent();
    while (parent && parent->GetTag() != "page") {
        parent = parent->GetParent();
    }
    return AceType::DynamicCast<FrameNode>(parent);
}

void FrameNode::NotifyFillRequestSuccess(RefPtr<PageNodeInfoWrap> nodeWrap, AceAutoFillType autoFillType)
{
    if (pattern_) {
        pattern_->NotifyFillRequestSuccess(nodeWrap, autoFillType);
    }
}

void FrameNode::NotifyFillRequestFailed(int32_t errCode)
{
    if (pattern_) {
        pattern_->NotifyFillRequestFailed(errCode);
    }
}

void FrameNode::MarkNeedRenderOnly()
{
    MarkNeedRender(IsRenderBoundary());
}

void FrameNode::MarkNeedRender(bool isRenderBoundary)
{
    auto context = GetContext();
    CHECK_NULL_VOID(context);
    // If it has dirtyLayoutBox, need to mark dirty after layout done.
    paintProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_RENDER);
    if (isRenderDirtyMarked_ || isLayoutDirtyMarked_) {
        return;
    }
    isRenderDirtyMarked_ = true;
    if (isRenderBoundary) {
        context->AddDirtyRenderNode(Claim(this));
        return;
    }
    auto parent = GetAncestorNodeOfFrame();
    if (parent) {
        parent->MarkDirtyNode(PROPERTY_UPDATE_RENDER_BY_CHILD_REQUEST);
    }
}

void FrameNode::MarkDirtyNode(bool isMeasureBoundary, bool isRenderBoundary, PropertyChangeFlag extraFlag)
{
    if (CheckNeedRender(extraFlag)) {
        paintProperty_->UpdatePropertyChangeFlag(extraFlag);
    }
    layoutProperty_->UpdatePropertyChangeFlag(extraFlag);
    paintProperty_->UpdatePropertyChangeFlag(extraFlag);
    auto layoutFlag = layoutProperty_->GetPropertyChangeFlag();
    auto paintFlag = paintProperty_->GetPropertyChangeFlag();
    if (CheckNoChanged(layoutFlag | paintFlag)) {
        return;
    }
    auto context = GetContext();
    CHECK_NULL_VOID(context);

    if (CheckNeedRequestMeasureAndLayout(layoutFlag)) {
        if (!isMeasureBoundary && IsNeedRequestParentMeasure()) {
            auto parent = GetAncestorNodeOfFrame();
            if (parent) {
                parent->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
                return;
            }
        }
        if (isLayoutDirtyMarked_) {
            return;
        }
        isLayoutDirtyMarked_ = true;
        context->AddDirtyLayoutNode(Claim(this));
        return;
    }
    layoutProperty_->CleanDirty();
    MarkNeedRender(isRenderBoundary);
}

bool FrameNode::IsNeedRequestParentMeasure() const
{
    auto layoutFlag = layoutProperty_->GetPropertyChangeFlag();
    if (layoutFlag == PROPERTY_UPDATE_BY_CHILD_REQUEST) {
        const auto& calcLayoutConstraint = layoutProperty_->GetCalcLayoutConstraint();
        if (calcLayoutConstraint && calcLayoutConstraint->selfIdealSize &&
            calcLayoutConstraint->selfIdealSize->IsValid()) {
            return false;
        }
    }
    return CheckNeedRequestParentMeasure(layoutFlag);
}

void FrameNode::OnGenerateOneDepthVisibleFrame(std::list<RefPtr<FrameNode>>& visibleList)
{
    if (isActive_ && IsVisible()) {
        visibleList.emplace_back(Claim(this));
    }
}

void FrameNode::OnGenerateOneDepthAllFrame(std::list<RefPtr<FrameNode>>& allList)
{
    allList.emplace_back(Claim(this));
}

void FrameNode::OnGenerateOneDepthVisibleFrameWithTransition(std::list<RefPtr<FrameNode>>& visibleList)
{
    auto context = GetRenderContext();
    CHECK_NULL_VOID(context);
    // skip if 1.not active or 2.not visible and has no transition out animation.
    if (!isActive_ || (!IsVisible() && !context->HasTransitionOutAnimation())) {
        return;
    }
    visibleList.emplace_back(Claim(this));
}

bool FrameNode::IsMeasureBoundary()
{
    return isMeasureBoundary_ || pattern_->IsMeasureBoundary();
}

bool FrameNode::IsRenderBoundary()
{
    return pattern_->IsRenderBoundary();
}

const RefPtr<Pattern>& FrameNode::GetPattern() const
{
    return pattern_;
}

bool FrameNode::IsAtomicNode() const
{
    return pattern_->IsAtomicNode();
}

HitTestMode FrameNode::GetHitTestMode() const
{
    auto gestureHub = eventHub_->GetGestureEventHub();
    return gestureHub ? gestureHub->GetHitTestMode() : HitTestMode::HTMDEFAULT;
}

void FrameNode::SetHitTestMode(HitTestMode mode)
{
    auto gestureHub = eventHub_->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetHitTestMode(mode);
}

bool FrameNode::GetTouchable() const
{
    auto gestureHub = eventHub_->GetGestureEventHub();
    return gestureHub ? gestureHub->GetTouchable() : true;
}

bool FrameNode::GetMonopolizeEvents() const
{
    auto gestureHub = eventHub_->GetGestureEventHub();
    return gestureHub ? gestureHub->GetMonopolizeEvents() : false;
}

bool FrameNode::IsResponseRegion() const
{
    auto renderContext = GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    auto clip = renderContext->GetClipEdge().value_or(false);
    if (clip) {
        return false;
    }
    auto gestureHub = eventHub_->GetGestureEventHub();
    return gestureHub ? gestureHub->IsResponseRegion() : false;
}

void FrameNode::MarkResponseRegion(bool isResponseRegion)
{
    auto gestureHub = eventHub_->GetOrCreateGestureEventHub();
    if (gestureHub) {
        gestureHub->MarkResponseRegion(isResponseRegion);
    }
}

RectF FrameNode::GetPaintRectWithTransform() const
{
    return renderContext_->GetPaintRectWithTransform();
}

VectorF FrameNode::GetTransformScale() const
{
    return renderContext_->GetTransformScaleValue({ 1.0f, 1.0f });
}

bool FrameNode::IsOutOfTouchTestRegion(const PointF& parentRevertPoint, int32_t sourceType)
{
    bool isInChildRegion = false;
    auto paintRect = renderContext_->GetPaintRectWithoutTransform();
    auto responseRegionList = GetResponseRegionList(paintRect, sourceType);
    auto renderContext = GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);

    auto revertPoint = parentRevertPoint;
    renderContext->GetPointWithRevert(revertPoint);
    auto subRevertPoint = revertPoint - paintRect.GetOffset();
    auto clip = renderContext->GetClipEdge().value_or(false);
    if (!InResponseRegionList(revertPoint, responseRegionList) || !GetTouchable()) {
        if (clip) {
            if (SystemProperties::GetDebugEnabled()) {
                LOGI("TouchTest: frameNode use clip, point is out of region in %{public}s", GetTag().c_str());
            }
            return true;
        }
        for (auto iter = frameChildren_.rbegin(); iter != frameChildren_.rend(); ++iter) {
            const auto& child = iter->Upgrade();
            if (child && !child->IsOutOfTouchTestRegion(subRevertPoint, sourceType)) {
                if (SystemProperties::GetDebugEnabled()) {
                    LOGI("TouchTest: point is out of region in %{public}s, but is in child region", GetTag().c_str());
                }
                isInChildRegion = true;
                break;
            }
        }
        if (!isInChildRegion) {
            if (SystemProperties::GetDebugEnabled()) {
                LOGI("TouchTest: point is out of region in %{public}s", GetTag().c_str());
            }
            return true;
        }
    }
    return false;
}

HitTestResult FrameNode::TouchTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    const PointF& parentRevertPoint, const TouchRestrict& touchRestrict, TouchTestResult& result, int32_t touchId)
{
    auto targetComponent = MakeRefPtr<TargetComponent>();
    targetComponent->SetNode(WeakClaim(this));
    auto gestureHub = eventHub_->GetGestureEventHub();
    if (gestureHub) {
        auto callback = gestureHub->GetOnGestureJudgeBeginCallback();
        if (callback) {
            targetComponent->SetOnGestureJudgeBegin(std::move(callback));
        }
    }

    if (!isActive_ || !eventHub_->IsEnabled() || bypass_) {
        if (SystemProperties::GetDebugEnabled()) {
            LOGI("%{public}s is inActive, need't do touch test", GetTag().c_str());
        }
        return HitTestResult::OUT_OF_REGION;
    }
    auto& translateIds = NGGestureRecognizer::GetGlobalTransIds();
    auto& translateCfg = NGGestureRecognizer::GetGlobalTransCfg();
    auto paintRect = renderContext_->GetPaintRectWithTransform();
    auto origRect = renderContext_->GetPaintRectWithoutTransform();
    auto localMat = renderContext_->GetLocalTransformMatrix();
    auto param = renderContext_->GetTrans();
    localMat_ = localMat;
    if (param.empty()) {
        translateCfg[GetId()] = { .id = GetId(), .localMat = localMat };
    } else {
        translateCfg[GetId()] = { param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7],
            param[8], GetId(), localMat };
    }

    if (GetInspectorId()->find("SCBScreen-Temp") != std::string::npos &&
        static_cast<int>(translateCfg[GetId()].degree) != 0) {
        translateCfg[GetId()].degree = 0.0;
        translateCfg[GetId()].localMat = Matrix4();
    }
    int32_t parentId = -1;
    auto parent = GetAncestorNodeOfFrame();
    if (parent) {
        AncestorNodeInfo ancestorNodeInfo { parent->GetId() };
        translateIds[GetId()] = ancestorNodeInfo;
        parentId = parent->GetId();
    }

    auto responseRegionList = GetResponseRegionList(origRect, static_cast<int32_t>(touchRestrict.sourceType));
    if (SystemProperties::GetDebugEnabled()) {
        LOGI("TouchTest: point is %{public}s in %{public}s, depth: %{public}d", parentRevertPoint.ToString().c_str(),
            GetTag().c_str(), GetDepth());
        for (const auto& rect : responseRegionList) {
            LOGI("TouchTest: responseRegionList is %{public}s, point is %{public}s", rect.ToString().c_str(),
                parentRevertPoint.ToString().c_str());
        }
    }
    {
        ACE_DEBUG_SCOPED_TRACE("FrameNode::IsOutOfTouchTestRegion");
        bool isOutOfRegion = IsOutOfTouchTestRegion(parentRevertPoint, static_cast<int32_t>(touchRestrict.sourceType));
        AddFrameNodeSnapshot(!isOutOfRegion, parentId);
        if (isOutOfRegion) {
            return HitTestResult::OUT_OF_REGION;
        }
    }

    HitTestResult testResult = HitTestResult::OUT_OF_REGION;
    bool preventBubbling = false;
    // Child nodes are repackaged into gesture groups (parallel gesture groups, exclusive gesture groups, etc.)
    // based on the gesture attributes set by the current parent node (high and low priority, parallel gestures,
    // etc.), the newComingTargets is the template object to collect child nodes gesture and used by gestureHub to
    // pack gesture group.
    TouchTestResult newComingTargets;
    auto tmp = parentLocalPoint - paintRect.GetOffset();
    auto preLocation = tmp;
    renderContext_->GetPointWithTransform(tmp);
    const auto localPoint = tmp;
    auto localTransformOffset = preLocation - localPoint;

    auto revertPoint = parentRevertPoint;
    renderContext_->GetPointWithRevert(revertPoint);
    auto subRevertPoint = revertPoint - origRect.GetOffset();
    bool consumed = false;
    for (auto iter = frameChildren_.rbegin(); iter != frameChildren_.rend(); ++iter) {
        if (GetHitTestMode() == HitTestMode::HTMBLOCK) {
            break;
        }

        const auto& child = iter->Upgrade();
        if (!child) {
            continue;
        }
        auto childHitResult =
            child->TouchTest(globalPoint, localPoint, subRevertPoint, touchRestrict, newComingTargets, touchId);
        if (childHitResult == HitTestResult::STOP_BUBBLING) {
            preventBubbling = true;
            consumed = true;
            if ((child->GetHitTestMode() == HitTestMode::HTMBLOCK) ||
                (child->GetHitTestMode() == HitTestMode::HTMDEFAULT) ||
                (child->GetHitTestMode() == HitTestMode::HTMTRANSPARENT_SELF) ||
                ((child->GetHitTestMode() != HitTestMode::HTMTRANSPARENT) && IsExclusiveEventForChild())) {
                break;
            }
        }

        // In normal process, the node block the brother node.
        if (childHitResult == HitTestResult::BUBBLING &&
            ((child->GetHitTestMode() == HitTestMode::HTMDEFAULT) ||
                (child->GetHitTestMode() == HitTestMode::HTMTRANSPARENT_SELF) ||
                ((child->GetHitTestMode() != HitTestMode::HTMTRANSPARENT) && IsExclusiveEventForChild()))) {
            consumed = true;
            break;
        }
    }

    // first update HitTestResult by children status.
    if (consumed) {
        testResult = preventBubbling ? HitTestResult::STOP_BUBBLING : HitTestResult::BUBBLING;
        consumed = false;
    } else if (GetHitTestMode() == HitTestMode::HTMBLOCK) {
        testResult = HitTestResult::STOP_BUBBLING;
    }

    if (!preventBubbling && (GetHitTestMode() != HitTestMode::HTMNONE) &&
        (InResponseRegionList(revertPoint, responseRegionList))) {
        pattern_->OnTouchTestHit(touchRestrict.hitTestType);
        consumed = true;
        if (touchRestrict.hitTestType == SourceType::TOUCH) {
            auto gestureHub = eventHub_->GetGestureEventHub();
            if (gestureHub) {
                TouchTestResult finalResult;
                const auto coordinateOffset = globalPoint - localPoint - localTransformOffset;
                preventBubbling = gestureHub->ProcessTouchTestHit(coordinateOffset, touchRestrict, newComingTargets,
                    finalResult, touchId, localPoint, targetComponent);
                newComingTargets.swap(finalResult);
            }
        } else if (touchRestrict.hitTestType == SourceType::MOUSE) {
            auto mouseHub = eventHub_->GetInputEventHub();
            if (mouseHub) {
                const auto coordinateOffset = globalPoint - localPoint;
                preventBubbling = mouseHub->ProcessMouseTestHit(coordinateOffset, newComingTargets);
            }
        }
    }

    result.splice(result.end(), std::move(newComingTargets));
    if (touchRestrict.hitTestType == SourceType::TOUCH) {
        // combine into exclusive recognizer group.
        auto gestureHub = eventHub_->GetGestureEventHub();
        if (gestureHub) {
            gestureHub->CombineIntoExclusiveRecognizer(globalPoint, localPoint, result, touchId);
        }
    }

    // consumed by children and return result.
    if (!consumed) {
        return testResult;
    }

    if (testResult == HitTestResult::OUT_OF_REGION) {
        // consume only by self.
        if (preventBubbling) {
            return HitTestResult::STOP_BUBBLING;
        }
        return (GetHitTestMode() == HitTestMode::HTMTRANSPARENT_SELF) ? HitTestResult::SELF_TRANSPARENT
                                                                      : HitTestResult::BUBBLING;
    }
    // consume by self and children.
    return testResult;
}

std::vector<RectF> FrameNode::GetResponseRegionList(const RectF& rect, int32_t sourceType)
{
    std::vector<RectF> responseRegionList;
    auto gestureHub = eventHub_->GetGestureEventHub();
    if (!gestureHub) {
        responseRegionList.emplace_back(rect);
        return responseRegionList;
    }
    auto scaleProperty = ScaleProperty::CreateScaleProperty();
    bool isMouseEvent = (static_cast<SourceType>(sourceType) == SourceType::MOUSE);
    if (isMouseEvent) {
        if (gestureHub->GetResponseRegion().empty() && (gestureHub->GetMouseResponseRegion().empty())) {
            responseRegionList.emplace_back(rect);
            return responseRegionList;
        }
    } else {
        if (gestureHub->GetResponseRegion().empty()) {
            responseRegionList.emplace_back(rect);
            return responseRegionList;
        }
    }

    if (isMouseEvent && (!gestureHub->GetMouseResponseRegion().empty())) {
        for (const auto& region : gestureHub->GetMouseResponseRegion()) {
            auto x = ConvertToPx(region.GetOffset().GetX(), scaleProperty, rect.Width());
            auto y = ConvertToPx(region.GetOffset().GetY(), scaleProperty, rect.Height());
            auto width = ConvertToPx(region.GetWidth(), scaleProperty, rect.Width());
            auto height = ConvertToPx(region.GetHeight(), scaleProperty, rect.Height());
            RectF mouseRegion(rect.GetOffset().GetX() + x.value(), rect.GetOffset().GetY() + y.value(), width.value(),
                height.value());
            responseRegionList.emplace_back(mouseRegion);
        }
        return responseRegionList;
    }
    for (const auto& region : gestureHub->GetResponseRegion()) {
        auto x = ConvertToPx(region.GetOffset().GetX(), scaleProperty, rect.Width());
        auto y = ConvertToPx(region.GetOffset().GetY(), scaleProperty, rect.Height());
        auto width = ConvertToPx(region.GetWidth(), scaleProperty, rect.Width());
        auto height = ConvertToPx(region.GetHeight(), scaleProperty, rect.Height());
        RectF responseRegion(
            rect.GetOffset().GetX() + x.value(), rect.GetOffset().GetY() + y.value(), width.value(), height.value());
        responseRegionList.emplace_back(responseRegion);
    }
    return responseRegionList;
}

bool FrameNode::InResponseRegionList(const PointF& parentLocalPoint, const std::vector<RectF>& responseRegionList) const
{
    for (const auto& rect : responseRegionList) {
        if (rect.IsInRegion(parentLocalPoint)) {
            return true;
        }
    }
    return false;
}

HitTestResult FrameNode::MouseTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    MouseTestResult& onMouseResult, MouseTestResult& onHoverResult, RefPtr<FrameNode>& hoverNode)
{
    // unuseable function. do nothing.
    return HitTestResult::BUBBLING;
}

HitTestResult FrameNode::AxisTest(
    const PointF& globalPoint, const PointF& parentLocalPoint, AxisTestResult& onAxisResult)
{
    const auto& rect = renderContext_->GetPaintRectWithTransform();
    LOGD("AxisTest: type is %{public}s, the region is %{public}lf, %{public}lf, %{public}lf, %{public}lf",
        GetTag().c_str(), rect.Left(), rect.Top(), rect.Width(), rect.Height());
    // TODO: disableTouchEvent || disabled_ need handle

    // TODO: Region need change to RectList
    if (!rect.IsInRegion(parentLocalPoint)) {
        return HitTestResult::OUT_OF_REGION;
    }

    bool preventBubbling = false;

    const auto localPoint = parentLocalPoint - rect.GetOffset();
    const auto& children = GetChildren();
    for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
        auto& child = *iter;
        auto childHitResult = child->AxisTest(globalPoint, localPoint, onAxisResult);
        if (childHitResult == HitTestResult::STOP_BUBBLING) {
            preventBubbling = true;
        }
        // In normal process, the node block the brother node.
        if (childHitResult == HitTestResult::BUBBLING) {
            // TODO: add hit test mode judge.
            break;
        }
    }

    AxisTestResult axisResult;
    bool isPrevent = false;
    auto inputHub = eventHub_->GetInputEventHub();
    if (inputHub) {
        const auto coordinateOffset = globalPoint - localPoint;
        isPrevent = inputHub->ProcessAxisTestHit(coordinateOffset, axisResult);
    }

    if (!preventBubbling) {
        preventBubbling = isPrevent;
        onAxisResult.splice(onAxisResult.end(), std::move(axisResult));
    }
    if (preventBubbling) {
        return HitTestResult::STOP_BUBBLING;
    }
    return HitTestResult::BUBBLING;
}

void FrameNode::AnimateHoverEffect(bool isHovered) const
{
    auto renderContext = GetRenderContext();
    if (!renderContext) {
        return;
    }
    HoverEffectType animationType = HoverEffectType::UNKNOWN;
    auto inputEventHub = eventHub_->GetInputEventHub();
    if (inputEventHub) {
        animationType = inputEventHub->GetHoverEffect();
        if (animationType == HoverEffectType::UNKNOWN || animationType == HoverEffectType::AUTO) {
            animationType = inputEventHub->GetHoverEffectAuto();
        }
    }
    if (animationType == HoverEffectType::SCALE) {
        renderContext->AnimateHoverEffectScale(isHovered);
    } else if (animationType == HoverEffectType::BOARD) {
        renderContext->AnimateHoverEffectBoard(isHovered);
    }
}

RefPtr<FocusHub> FrameNode::GetOrCreateFocusHub() const
{
    if (!pattern_) {
        return eventHub_->GetOrCreateFocusHub();
    }
    auto focusPattern = pattern_->GetFocusPattern();
    return eventHub_->GetOrCreateFocusHub(focusPattern.GetFocusType(), focusPattern.GetFocusable(),
        focusPattern.GetStyleType(), focusPattern.GetFocusPaintParams());
}

void FrameNode::OnWindowShow()
{
    pattern_->OnWindowShow();
}

void FrameNode::OnWindowHide()
{
    pattern_->OnWindowHide();
}

void FrameNode::OnWindowFocused()
{
    pattern_->OnWindowFocused();
}

void FrameNode::OnWindowUnfocused()
{
    pattern_->OnWindowUnfocused();
}

std::pair<float, float> FrameNode::ContextPositionConvertToPX(
    const RefPtr<RenderContext>& context, const SizeF& percentReference) const
{
    std::pair<float, float> position;
    CHECK_NULL_RETURN(context, position);
    auto scaleProperty = ScaleProperty::CreateScaleProperty();
    position.first =
        ConvertToPx(context->GetPositionProperty()->GetPosition()->GetX(), scaleProperty, percentReference.Width())
            .value_or(0.0);
    position.second =
        ConvertToPx(context->GetPositionProperty()->GetPosition()->GetY(), scaleProperty, percentReference.Height())
            .value_or(0.0);
    return position;
}

void FrameNode::OnPixelRoundFinish(const SizeF& pixelGridRoundSize)
{
    CHECK_NULL_VOID(pattern_);
    pattern_->OnPixelRoundFinish(pixelGridRoundSize);
}

void FrameNode::OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type)
{
    pattern_->OnWindowSizeChanged(width, height, type);
}

/* @deprecated  This func will be deleted, please use GetTransformRelativeOffset() instead. */
OffsetF FrameNode::GetOffsetRelativeToWindow() const
{
    auto offset = geometryNode_->GetFrameOffset();
    auto parent = GetAncestorNodeOfFrame();
    if (renderContext_ && renderContext_->GetPositionProperty()) {
        if (renderContext_->GetPositionProperty()->HasPosition()) {
            auto renderPosition =
                ContextPositionConvertToPX(renderContext_, layoutProperty_->GetLayoutConstraint()->percentReference);
            offset.SetX(static_cast<float>(renderPosition.first));
            offset.SetY(static_cast<float>(renderPosition.second));
        }
    }
    while (parent) {
        auto parentRenderContext = parent->GetRenderContext();
        if (parentRenderContext && parentRenderContext->GetPositionProperty()) {
            if (parentRenderContext->GetPositionProperty()->HasPosition()) {
                auto parentLayoutProperty = parent->GetLayoutProperty();
                CHECK_NULL_RETURN(parentLayoutProperty, offset);
                auto parentRenderContextPosition = ContextPositionConvertToPX(
                    parentRenderContext, parentLayoutProperty->GetLayoutConstraint()->percentReference);
                offset.AddX(static_cast<float>(parentRenderContextPosition.first));
                offset.AddY(static_cast<float>(parentRenderContextPosition.second));
                parent = parent->GetAncestorNodeOfFrame();
                continue;
            }
        }

        offset += parent->geometryNode_->GetFrameOffset();
        parent = parent->GetAncestorNodeOfFrame();
    }

    return offset;
}

RectF FrameNode::GetTransformRectRelativeToWindow() const
{
    auto context = GetRenderContext();
    CHECK_NULL_RETURN(context, RectF());
    RectF rect = context->GetPaintRectWithTransform();
    auto offset = rect.GetOffset();
    auto parent = GetAncestorNodeOfFrame();
    while (parent) {
        auto parentRenderContext = parent->GetRenderContext();
        CHECK_NULL_RETURN(parentRenderContext, rect);
        auto parentScale = parentRenderContext->GetTransformScale();
        if (parentScale) {
            auto oldSize = rect.GetSize();
            auto newSize = SizeF(oldSize.Width() * parentScale.value().x, oldSize.Height() * parentScale.value().y);
            rect.SetSize(newSize);

            offset = OffsetF(offset.GetX() * parentScale.value().x, offset.GetY() * parentScale.value().y);
        }

        offset += parentRenderContext->GetPaintRectWithTransform().GetOffset();

        parent = parent->GetAncestorNodeOfFrame();
    }
    rect.SetOffset(offset);
    return rect;
}

OffsetF FrameNode::GetTransformRelativeOffset() const
{
    auto context = GetRenderContext();
    CHECK_NULL_RETURN(context, OffsetF());
    auto offset = context->GetPaintRectWithTransform().GetOffset();
    auto parent = GetAncestorNodeOfFrame();

    while (parent) {
        auto parentRenderContext = parent->GetRenderContext();
        offset += parentRenderContext->GetPaintRectWithTransform().GetOffset();
        parent = parent->GetAncestorNodeOfFrame();
    }

    return offset;
}

OffsetF FrameNode::GetPaintRectOffset(bool excludeSelf) const
{
    auto context = GetRenderContext();
    CHECK_NULL_RETURN(context, OffsetF());
    OffsetF offset = excludeSelf ? OffsetF() : context->GetPaintRectWithTransform().GetOffset();
    auto parent = GetAncestorNodeOfFrame();
    while (parent) {
        auto renderContext = parent->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, OffsetF());
        offset += renderContext->GetPaintRectWithTransform().GetOffset();
        parent = parent->GetAncestorNodeOfFrame();
    }
    return offset;
}

OffsetF FrameNode::GetParentGlobalOffsetDuringLayout() const
{
    OffsetF offset {};
    auto parent = GetAncestorNodeOfFrame();
    while (parent) {
        offset += parent->geometryNode_->GetFrameOffset();
        parent = parent->GetAncestorNodeOfFrame();
    }
    return offset;
}

OffsetF FrameNode::GetPaintRectGlobalOffsetWithTranslate(bool excludeSelf) const
{
    auto context = GetRenderContext();
    CHECK_NULL_RETURN(context, OffsetF());
    OffsetF offset = excludeSelf ? OffsetF() : context->GetPaintRectWithTranslate().GetOffset();
    auto parent = GetAncestorNodeOfFrame();
    while (parent) {
        auto renderContext = parent->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, OffsetF());
        auto rect = renderContext->GetPaintRectWithTranslate();
        CHECK_NULL_RETURN(rect.IsValid(), offset + parent->GetPaintRectOffset());
        offset += rect.GetOffset();
        parent = parent->GetAncestorNodeOfFrame();
    }
    return offset;
}

OffsetF FrameNode::GetPaintRectOffsetToPage() const
{
    auto context = GetRenderContext();
    CHECK_NULL_RETURN(context, OffsetF());
    OffsetF offset = context->GetPaintRectWithTransform().GetOffset();
    auto parent = GetAncestorNodeOfFrame();
    while (parent && parent->GetTag() != V2::PAGE_ETS_TAG) {
        auto renderContext = parent->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, OffsetF());
        offset += renderContext->GetPaintRectWithTransform().GetOffset();
        parent = parent->GetAncestorNodeOfFrame();
    }
    return (parent && parent->GetTag() == V2::PAGE_ETS_TAG) ? offset : OffsetF();
}

std::optional<RectF> FrameNode::GetViewPort() const
{
    if (viewPort_.has_value()) {
        return viewPort_;
    }
    auto parent = GetAncestorNodeOfFrame();
    while (parent && parent->GetTag() != V2::PAGE_ETS_TAG) {
        auto parentViewPort = parent->GetSelfViewPort();
        if (parentViewPort.has_value()) {
            return parentViewPort;
        }
        parent = parent->GetAncestorNodeOfFrame();
    }
    return std::nullopt;
}

void FrameNode::OnNotifyMemoryLevel(int32_t level)
{
    pattern_->OnNotifyMemoryLevel(level);
}

int32_t FrameNode::GetAllDepthChildrenCount()
{
    int32_t result = 0;
    std::list<RefPtr<FrameNode>> children;
    children.emplace_back(Claim(this));
    while (!children.empty()) {
        auto& node = children.front();
        if (!node->IsInternal()) {
            result++;
            node->GenerateOneDepthVisibleFrame(children);
        }
        children.pop_front();
    }
    return result;
}

void FrameNode::OnAccessibilityEvent(
    AccessibilityEventType eventType, WindowsContentChangeTypes windowsContentChangeType) const
{
    if (AceApplicationInfo::GetInstance().IsAccessibilityEnabled()) {
        AccessibilityEvent event;
        event.type = eventType;
        event.windowContentChangeTypes = windowsContentChangeType;
        event.nodeId = GetAccessibilityId();
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        pipeline->SendEventToAccessibility(event);
    }
}

void FrameNode::OnAccessibilityEvent(
    AccessibilityEventType eventType, std::string beforeText, std::string latestContent) const
{
    if (AceApplicationInfo::GetInstance().IsAccessibilityEnabled()) {
        AccessibilityEvent event;
        event.type = eventType;
        event.nodeId = GetAccessibilityId();
        event.beforeText = beforeText;
        event.latestContent = latestContent;
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        pipeline->SendEventToAccessibility(event);
    }
}

bool FrameNode::MarkRemoving()
{
    bool pendingRemove = false;
    if (!layoutProperty_ || !geometryNode_) {
        return pendingRemove;
    }

    isRemoving_ = true;

    const auto& geometryTransition = layoutProperty_->GetGeometryTransition();
    if (geometryTransition != nullptr) {
        geometryTransition->Build(WeakClaim(this), false);
        pendingRemove = true;
    }

    const auto& children = GetChildren();
    for (const auto& child : children) {
        pendingRemove = child->MarkRemoving() || pendingRemove;
    }
    return pendingRemove;
}

void FrameNode::AddHotZoneRect(const DimensionRect& hotZoneRect) const
{
    auto gestureHub = GetOrCreateGestureEventHub();
    gestureHub->AddResponseRect(hotZoneRect);
}

void FrameNode::RemoveLastHotZoneRect() const
{
    auto gestureHub = GetOrCreateGestureEventHub();
    gestureHub->RemoveLastResponseRect();
}

bool FrameNode::OnRemoveFromParent(bool allowTransition)
{
    // kick out transition animation if needed, wont re-entry if already detached.
    DetachFromMainTree(!allowTransition);
    auto context = GetRenderContext();
    CHECK_NULL_RETURN(context, false);
    if (!allowTransition || RemoveImmediately()) {
        // directly remove, reset parent and depth
        ResetParent();
        return true;
    }
    // delayed remove, will move self into disappearing children
    return false;
}

RefPtr<FrameNode> FrameNode::FindChildByPosition(float x, float y)
{
    std::map<int32_t, RefPtr<FrameNode>> hitFrameNodes;
    std::list<RefPtr<FrameNode>> children;
    GenerateOneDepthAllFrame(children);
    for (const auto& child : children) {
        auto geometryNode = child->GetGeometryNode();
        if (!geometryNode) {
            continue;
        }

        auto globalFrameRect = geometryNode->GetFrameRect();
        globalFrameRect.SetOffset(child->GetOffsetRelativeToWindow());

        if (globalFrameRect.IsInRegion(PointF(x, y))) {
            hitFrameNodes.insert(std::make_pair(child->GetDepth(), child));
        }
    }

    if (hitFrameNodes.empty()) {
        return nullptr;
    }

    return hitFrameNodes.rbegin()->second;
}

RefPtr<NodeAnimatablePropertyBase> FrameNode::GetAnimatablePropertyFloat(const std::string& propertyName) const
{
    auto iter = nodeAnimatablePropertyMap_.find(propertyName);
    if (iter == nodeAnimatablePropertyMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

void FrameNode::CreateAnimatablePropertyFloat(
    const std::string& propertyName, float value, const std::function<void(float)>& onCallbackEvent)
{
    auto context = GetRenderContext();
    CHECK_NULL_VOID(context);
    auto iter = nodeAnimatablePropertyMap_.find(propertyName);
    if (iter != nodeAnimatablePropertyMap_.end()) {
        return;
    }
    auto property = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(value, std::move(onCallbackEvent));
    context->AttachNodeAnimatableProperty(property);
    nodeAnimatablePropertyMap_.emplace(propertyName, property);
}

void FrameNode::DeleteAnimatablePropertyFloat(const std::string& propertyName)
{
    auto context = GetRenderContext();
    CHECK_NULL_VOID(context);
    RefPtr<NodeAnimatablePropertyBase> propertyRef = GetAnimatablePropertyFloat(propertyName);
    if (propertyRef) {
        context->DetachNodeAnimatableProperty(propertyRef);
        nodeAnimatablePropertyMap_.erase(propertyName);
    }
}

void FrameNode::UpdateAnimatablePropertyFloat(const std::string& propertyName, float value)
{
    auto iter = nodeAnimatablePropertyMap_.find(propertyName);
    if (iter == nodeAnimatablePropertyMap_.end()) {
        return;
    }
    auto property = AceType::DynamicCast<NodeAnimatablePropertyFloat>(iter->second);
    CHECK_NULL_VOID(property);
    property->Set(value);
}

void FrameNode::CreateAnimatableArithmeticProperty(const std::string& propertyName,
    RefPtr<CustomAnimatableArithmetic>& value,
    std::function<void(const RefPtr<CustomAnimatableArithmetic>&)>& onCallbackEvent)
{
    auto context = GetRenderContext();
    CHECK_NULL_VOID(context);
    auto iter = nodeAnimatablePropertyMap_.find(propertyName);
    if (iter != nodeAnimatablePropertyMap_.end()) {
        return;
    }
    auto property = AceType::MakeRefPtr<NodeAnimatableArithmeticProperty>(value, std::move(onCallbackEvent));
    context->AttachNodeAnimatableProperty(property);
    nodeAnimatablePropertyMap_.emplace(propertyName, property);
}

void FrameNode::UpdateAnimatableArithmeticProperty(
    const std::string& propertyName, RefPtr<CustomAnimatableArithmetic>& value)
{
    auto iter = nodeAnimatablePropertyMap_.find(propertyName);
    if (iter == nodeAnimatablePropertyMap_.end()) {
        return;
    }
    auto property = AceType::DynamicCast<NodeAnimatableArithmeticProperty>(iter->second);
    CHECK_NULL_VOID(property);
    property->Set(value);
}

std::string FrameNode::ProvideRestoreInfo()
{
    return pattern_->ProvideRestoreInfo();
}

bool FrameNode::RemoveImmediately() const
{
    auto context = GetRenderContext();
    CHECK_NULL_RETURN(context, true);
    // has transition out animation, need to wait for animation end
    return !context->HasTransitionOutAnimation();
}

std::vector<RefPtr<FrameNode>> FrameNode::GetNodesById(const std::unordered_set<int32_t>& set)
{
    std::vector<RefPtr<FrameNode>> nodes;
    for (auto nodeId : set) {
        auto uiNode = ElementRegister::GetInstance()->GetUINodeById(nodeId);
        if (!uiNode) {
            continue;
        }
        auto frameNode = DynamicCast<FrameNode>(uiNode);
        if (frameNode) {
            nodes.emplace_back(frameNode);
        }
    }
    return nodes;
}

int32_t FrameNode::GetNodeExpectedRate()
{
    if (sceneRateMap_.empty()) {
        return 0;
    }
    auto iter = std::max_element(
        sceneRateMap_.begin(), sceneRateMap_.end(), [](auto a, auto b) { return a.second < b.second; });
    return iter->second;
}

void FrameNode::AddFRCSceneInfo(const std::string& scene, float speed, SceneStatus status)
{
    if (SystemProperties::GetDebugEnabled()) {
        const std::string sceneStatusStrs[] = {"START", "RUNNING", "END"};
        LOGI("%{public}s  AddFRCSceneInfo scene:%{public}s   speed:%{public}f  status:%{public}s", GetTag().c_str(),
            scene.c_str(), speed, sceneStatusStrs[static_cast<int32_t>(status)].c_str());
    }

    auto renderContext = GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto pipelineContext = GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto frameRateManager = pipelineContext->GetFrameRateManager();
    CHECK_NULL_VOID(frameRateManager);

    auto expectedRate = renderContext->CalcExpectedFrameRate(scene, speed);
    auto nodeId = GetId();
    auto iter = sceneRateMap_.find(scene);
    switch (status) {
        case SceneStatus::START: {
            if (iter == sceneRateMap_.end()) {
                if (sceneRateMap_.empty()) {
                    frameRateManager->AddNodeRate(nodeId);
                }
                sceneRateMap_.emplace(scene, expectedRate);
                frameRateManager->UpdateNodeRate(nodeId, GetNodeExpectedRate());
            }
            return;
        }
        case SceneStatus::RUNNING: {
            if (iter != sceneRateMap_.end() && iter->second != expectedRate) {
                iter->second = expectedRate;
                auto nodeExpectedRate = GetNodeExpectedRate();
                frameRateManager->UpdateNodeRate(nodeId, nodeExpectedRate);
            }
            return;
        }
        case SceneStatus::END: {
            if (iter != sceneRateMap_.end()) {
                sceneRateMap_.erase(iter);
                if (sceneRateMap_.empty()) {
                    frameRateManager->RemoveNodeRate(nodeId);
                } else {
                    auto nodeExpectedRate = GetNodeExpectedRate();
                    frameRateManager->UpdateNodeRate(nodeId, nodeExpectedRate);
                }
            }
            return;
        }
        default:
            return;
    }
}

void FrameNode::CheckSecurityComponentStatus(std::vector<RectF>& rect)
{
    auto paintRect = GetTransformRectRelativeToWindow();
    if (IsSecurityComponent()) {
        bypass_ = CheckRectIntersect(paintRect, rect);
    }
    for (auto iter = frameChildren_.rbegin(); iter != frameChildren_.rend(); ++iter) {
        const auto& child = iter->Upgrade();
        if (child) {
            child->CheckSecurityComponentStatus(rect);
        }
    }
    rect.push_back(paintRect);
}

bool FrameNode::CheckRectIntersect(const RectF& dest, std::vector<RectF>& origin)
{
    for (auto originRect : origin) {
        if (originRect.IsInnerIntersectWith(dest)) {
            return true;
        }
    }
    return false;
}

bool FrameNode::HaveSecurityComponent()
{
    if (IsSecurityComponent()) {
        return true;
    }
    for (auto iter = frameChildren_.rbegin(); iter != frameChildren_.rend(); ++iter) {
        const auto& child = iter->Upgrade();
        if (child && child->HaveSecurityComponent()) {
            return true;
        }
    }
    return false;
}

bool FrameNode::IsSecurityComponent()
{
    return GetTag() == V2::LOCATION_BUTTON_ETS_TAG || GetTag() == V2::PASTE_BUTTON_ETS_TAG ||
           GetTag() == V2::SAVE_BUTTON_ETS_TAG;
}

void FrameNode::GetPercentSensitive()
{
    auto res = layoutProperty_->GetPercentSensitive();
    if (res.first) {
        if (layoutAlgorithm_) {
            layoutAlgorithm_->SetPercentWidth(true);
        }
    }
    if (res.second) {
        if (layoutAlgorithm_) {
            layoutAlgorithm_->SetPercentHeight(true);
        }
    }
}

void FrameNode::UpdatePercentSensitive()
{
    bool percentHeight = layoutAlgorithm_ ? layoutAlgorithm_->GetPercentHeight() : true;
    bool percentWidth = layoutAlgorithm_ ? layoutAlgorithm_->GetPercentWidth() : true;
    auto res = layoutProperty_->UpdatePercentSensitive(percentHeight, percentWidth);
    if (res.first) {
        auto parent = GetAncestorNodeOfFrame();
        if (parent && parent->layoutAlgorithm_) {
            parent->layoutAlgorithm_->SetPercentWidth(true);
        }
    }
    if (res.second) {
        auto parent = GetAncestorNodeOfFrame();
        if (parent && parent->layoutAlgorithm_) {
            parent->layoutAlgorithm_->SetPercentHeight(true);
        }
    }
}

// This will call child and self measure process.
void FrameNode::Measure(const std::optional<LayoutConstraintF>& parentConstraint)
{
    ACE_SCOPED_TRACE("Measure[%s][self:%d][parent:%d]", GetTag().c_str(),
        GetId(), GetParent() ? GetParent()->GetId() : 0);
    isLayoutComplete_ = false;
    if (!oldGeometryNode_) {
        oldGeometryNode_ = geometryNode_->Clone();
    }
    RestoreGeoState();
    pattern_->BeforeCreateLayoutWrapper();
    GetLayoutAlgorithm(true);

    if (layoutProperty_->GetVisibility().value_or(VisibleType::VISIBLE) == VisibleType::GONE) {
        layoutAlgorithm_->SetSkipMeasure();
        layoutAlgorithm_->SetSkipLayout();
        geometryNode_->SetFrameSize(SizeF());
        isLayoutDirtyMarked_ = false;
        return;
    }
    if (!isActive_) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }

    if (layoutAlgorithm_->SkipMeasure()) {
        LOGD("%{public}s, depth: %{public}d: the layoutAlgorithm skip measure", GetTag().c_str(), GetDepth());
        isLayoutDirtyMarked_ = false;
        return;
    }

    const auto& geometryTransition = layoutProperty_->GetGeometryTransition();
    if (geometryTransition != nullptr && geometryTransition->IsRunning(WeakClaim(this))) {
        geometryTransition->WillLayout(Claim(this));
    }
    auto preConstraint = layoutProperty_->GetLayoutConstraint();
    auto contentConstraint = layoutProperty_->GetContentLayoutConstraint();
    layoutProperty_->BuildGridProperty(Claim(this));

    if (parentConstraint) {
        ApplyConstraint(*parentConstraint);
    } else {
        CreateRootConstraint();
    }

    layoutProperty_->UpdateContentConstraint();
    geometryNode_->UpdateMargin(layoutProperty_->CreateMargin());
    geometryNode_->UpdatePaddingWithBorder(layoutProperty_->CreatePaddingAndBorder());

    isConstraintNotChanged_ = layoutProperty_->ConstraintEqual(preConstraint, contentConstraint);

    LOGD("Measure: %{public}s, depth: %{public}d, Constraint: %{public}s", GetTag().c_str(), GetDepth(),
        layoutProperty_->GetLayoutConstraint()->ToString().c_str());

    isLayoutDirtyMarked_ = false;

    if (isConstraintNotChanged_) {
        if (!CheckNeedForceMeasureAndLayout()) {
            ACE_SCOPED_TRACE("SkipMeasure");
            LOGD("%{public}s (depth: %{public}d) skip measure content", GetTag().c_str(), GetDepth());
            layoutAlgorithm_->SetSkipMeasure();
            return;
        }
    }

    auto size = layoutAlgorithm_->MeasureContent(layoutProperty_->CreateContentConstraint(), this);
    if (size.has_value()) {
        geometryNode_->SetContentSize(size.value());
    }
    GetPercentSensitive();
    layoutAlgorithm_->Measure(this);
    if (overlayNode_) {
        overlayNode_->Measure(layoutProperty_->CreateChildConstraint());
    }
    UpdatePercentSensitive();
    // check aspect radio.
    if (pattern_ && pattern_->IsNeedAdjustByAspectRatio()) {
        const auto& magicItemProperty = layoutProperty_->GetMagicItemProperty();
        auto aspectRatio = magicItemProperty->GetAspectRatioValue();
        // Adjust by aspect ratio, firstly pick height based on width. It means that when width, height and
        // aspectRatio are all set, the height is not used.
        auto width = geometryNode_->GetFrameSize().Width();
        LOGD("aspect ratio affects, origin width: %{public}f, height: %{public}f", width,
            geometryNode_->GetFrameSize().Height());
        auto height = width / aspectRatio;
        LOGD("aspect ratio affects, new width: %{public}f, height: %{public}f", width, height);
        geometryNode_->SetFrameSize(SizeF({ width, height }));
    }

    LOGD("on Measure Done: type: %{public}s, depth: %{public}d, Size: %{public}s", GetTag().c_str(), GetDepth(),
        geometryNode_->GetFrameSize().ToString().c_str());

    layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_LAYOUT);
}

// Called to perform layout children.
void FrameNode::Layout()
{
    ACE_SCOPED_TRACE("Layout[%s][self:%d][parent:%d]", GetTag().c_str(),
        GetId(), GetParent() ? GetParent()->GetId() : 0);
    int64_t time = GetSysTimestamp();
    OffsetNodeToSafeArea();
    const auto& geometryTransition = layoutProperty_->GetGeometryTransition();
    if (geometryTransition != nullptr) {
        if (!IsRootMeasureNode() && geometryTransition->IsNodeInAndActive(Claim(this))) {
            SetSkipSyncGeometryNode();
        }
    }
    if (CheckNeedLayout(layoutProperty_->GetPropertyChangeFlag())) {
        if (!layoutProperty_->GetLayoutConstraint()) {
            const auto& parentLayoutConstraint = geometryNode_->GetParentLayoutConstraint();
            if (parentLayoutConstraint) {
                layoutProperty_->UpdateLayoutConstraint(parentLayoutConstraint.value());
            } else {
                LayoutConstraintF layoutConstraint;
                layoutConstraint.percentReference.SetWidth(PipelineContext::GetCurrentRootWidth());
                layoutConstraint.percentReference.SetHeight(PipelineContext::GetCurrentRootHeight());
                layoutProperty_->UpdateLayoutConstraint(layoutConstraint);
            }
            layoutProperty_->UpdateContentConstraint();
        }
        GetLayoutAlgorithm()->Layout(this);
        if (overlayNode_) {
            LayoutOverlay();
        }
        time = GetSysTimestamp() - time;
        AddNodeFlexLayouts();
        AddNodeLayoutTime(time);
    } else {
        GetLayoutAlgorithm()->SetSkipLayout();
    }

    SaveGeoState();
    AvoidKeyboard();
    ExpandSafeArea();

    LOGD("On Layout Done: type: %{public}s, depth: %{public}d, Offset: %{public}s", GetTag().c_str(), GetDepth(),
        geometryNode_->GetFrameOffset().ToString().c_str());
    SyncGeometryNode();

    UpdateParentAbsoluteOffset();
}

void FrameNode::SyncGeometryNode()
{
    const auto& geometryTransition = layoutProperty_->GetGeometryTransition();
    bool hasTransition = geometryTransition != nullptr && geometryTransition->IsRunning(WeakClaim(this));

    if (!isActive_ && !hasTransition) {
        layoutAlgorithm_.Reset();
        return;
    }
    if (SkipSyncGeometryNode() && (!geometryTransition || !geometryTransition->IsNodeInAndActive(Claim(this)))) {
        layoutAlgorithm_.Reset();
        return;
    }

    // update layout size.
    bool frameSizeChange = true;
    bool frameOffsetChange = true;
    bool contentSizeChange = true;
    bool contentOffsetChange = true;
    if (oldGeometryNode_) {
        frameSizeChange = geometryNode_->GetFrameSize() != oldGeometryNode_->GetFrameSize();
        frameOffsetChange = geometryNode_->GetFrameOffset() != oldGeometryNode_->GetFrameOffset();
        contentSizeChange = geometryNode_->GetContentSize() != oldGeometryNode_->GetContentSize();
        contentOffsetChange = geometryNode_->GetContentOffset() != oldGeometryNode_->GetContentOffset();
        oldGeometryNode_.Reset();
    }

    // update border.
    if (layoutProperty_->GetBorderWidthProperty()) {
        if (!renderContext_->HasBorderColor()) {
            BorderColorProperty borderColorProperty;
            borderColorProperty.SetColor(Color::BLACK);
            renderContext_->UpdateBorderColor(borderColorProperty);
        }
        if (!renderContext_->HasBorderStyle()) {
            BorderStyleProperty borderStyleProperty;
            borderStyleProperty.SetBorderStyle(BorderStyle::SOLID);
            renderContext_->UpdateBorderStyle(borderStyleProperty);
        }
        if (layoutProperty_->GetLayoutConstraint().has_value()) {
            renderContext_->UpdateBorderWidthF(ConvertToBorderWidthPropertyF(layoutProperty_->GetBorderWidthProperty(),
                ScaleProperty::CreateScaleProperty(),
                layoutProperty_->GetLayoutConstraint()->percentReference.Width()));
        } else {
            renderContext_->UpdateBorderWidthF(ConvertToBorderWidthPropertyF(layoutProperty_->GetBorderWidthProperty(),
                ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth()));
        }
    }

    // clean layout flag.
    layoutProperty_->CleanDirty();

    if (hasTransition) {
        geometryTransition->DidLayout(Claim(this));
        if (geometryTransition->IsNodeOutAndActive(WeakClaim(this))) {
            isLayoutDirtyMarked_ = true;
        }
    } else if (frameSizeChange || frameOffsetChange || HasPositionProp() ||
               (pattern_->GetContextParam().has_value() && contentSizeChange)) {
        isLayoutComplete_ = true;
        renderContext_->SyncGeometryProperties(RawPtr(geometryNode_), true);
    }

    DirtySwapConfig config { frameSizeChange, frameOffsetChange, contentSizeChange, contentOffsetChange };
    // check if need to paint content.
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(layoutAlgorithm_);
    CHECK_NULL_VOID(layoutAlgorithmWrapper);
    config.skipMeasure = layoutAlgorithmWrapper->SkipMeasure();
    config.skipLayout = layoutAlgorithmWrapper->SkipLayout();
    if (!config.skipMeasure && !config.skipLayout && GetInspectorId()) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        pipeline->OnLayoutCompleted(GetInspectorId()->c_str());
    }
    auto needRerender = pattern_->OnDirtyLayoutWrapperSwap(Claim(this), config);
    // TODO: temp use and need to delete.
    needRerender =
        needRerender || pattern_->OnDirtyLayoutWrapperSwap(Claim(this), config.skipMeasure, config.skipLayout);
    if (needRerender || CheckNeedRender(paintProperty_->GetPropertyChangeFlag())) {
        MarkDirtyNode(true, true, PROPERTY_UPDATE_RENDER);
    }

    // update background
    if (builderFunc_) {
        auto builderNode = builderFunc_();
        auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        builderNode->MountToParent(columnNode);
        SetBackgroundLayoutConstraint(columnNode);
        renderContext_->CreateBackgroundPixelMap(columnNode);
        builderFunc_ = nullptr;
        backgroundNode_ = columnNode;
    }

    // update focus state
    auto focusHub = GetFocusHub();
    if (focusHub && focusHub->IsCurrentFocus()) {
        focusHub->ClearFocusState(false);
        focusHub->PaintFocusState(false);
    }

    // rebuild child render node.
    RebuildRenderContextTree();

    /* Adjust components' position which have been set grid properties */
    AdjustGridOffset();

    layoutAlgorithm_.Reset();
}

void FrameNode::UpdateParentAbsoluteOffset()
{
    const auto& children = GetChildren();
    for (const auto& child : children) {
        auto frameNode = AceType::DynamicCast<FrameNode>(child);
        CHECK_NULL_VOID(frameNode);
        auto childNode = frameNode->GetGeometryNode();
        CHECK_NULL_VOID(childNode);
        childNode->SetParentAbsoluteOffset(geometryNode_->GetParentAbsoluteOffset() + GetOffsetRelativeToWindow());
    }
}

RefPtr<LayoutWrapper> FrameNode::GetOrCreateChildByIndex(uint32_t index, bool addToRenderTree)
{
    auto child = frameProxy_->GetFrameNodeByIndex(index, true);
    if (child) {
        child->SetSkipSyncGeometryNode(SkipSyncGeometryNode());
        if (addToRenderTree) {
            child->SetActive(true);
        }
    }
    return child;
}

RefPtr<LayoutWrapper> FrameNode::GetChildByIndex(uint32_t index)
{
    return frameProxy_->GetFrameNodeByIndex(index, false);
}

const std::list<RefPtr<LayoutWrapper>>& FrameNode::GetAllChildrenWithBuild(bool addToRenderTree)
{
    const auto& children = frameProxy_->GetAllFrameChildren();
    {
        auto guard = frameProxy_->GetGuard();
        for (const auto& child : children) {
            if (addToRenderTree) {
                child->SetActive(true);
            }
            child->SetSkipSyncGeometryNode(SkipSyncGeometryNode());
        }
    }

    return children;
}

void FrameNode::RemoveAllChildInRenderTree()
{
    frameProxy_->RemoveAllChildInRenderTree();
}

void FrameNode::RemoveChildInRenderTree(uint32_t index)
{
    frameProxy_->RemoveChildInRenderTree(index);
}

bool FrameNode::SkipMeasureContent() const
{
    return layoutAlgorithm_->SkipMeasure();
}

bool FrameNode::CheckNeedForceMeasureAndLayout()
{
    PropertyChangeFlag flag = layoutProperty_->GetPropertyChangeFlag();
    return CheckNeedMeasure(flag) || CheckNeedLayout(flag);
}

float FrameNode::GetBaselineDistance() const
{
    const auto& children = frameProxy_->GetAllFrameChildren();
    if (children.empty()) {
        return geometryNode_->GetBaselineDistance();
    }
    float distance = 0.0;
    {
        auto guard = frameProxy_->GetGuard();
        for (const auto& child : children) {
            float childBaseline = child->GetBaselineDistance();
            distance = NearZero(distance) ? childBaseline : std::min(distance, childBaseline);
        }
    }
    return distance;
}

void FrameNode::MarkNeedSyncRenderTree(bool needRebuild)
{
    if (needRebuild) {
        frameProxy_->ResetChildren(true);
    }
    needSyncRenderTree_ = true;
}

RefPtr<UINode> FrameNode::GetFrameChildByIndex(uint32_t index, bool needBuild)
{
    if (index != 0) {
        return nullptr;
    }
    return Claim(this);
}

const RefPtr<LayoutAlgorithmWrapper>& FrameNode::GetLayoutAlgorithm(bool needReset)
{
    if ((!layoutAlgorithm_ || (needReset && layoutAlgorithm_->IsExpire())) && pattern_) {
        layoutAlgorithm_ = MakeRefPtr<LayoutAlgorithmWrapper>(pattern_->CreateLayoutAlgorithm());
    }
    if (needReset) {
        layoutAlgorithm_->SetNeedMeasure();
    }
    return layoutAlgorithm_;
}

void FrameNode::SetCacheCount(int32_t cacheCount, const std::optional<LayoutConstraintF>& itemConstraint)
{
    frameProxy_->SetCacheCount(cacheCount, itemConstraint);
}

void FrameNode::LayoutOverlay()
{
    auto size = geometryNode_->GetFrameSize();
    auto align = Alignment::TOP_LEFT;
    Dimension offsetX, offsetY;
    auto childLayoutProperty = overlayNode_->GetLayoutProperty();
    childLayoutProperty->GetOverlayOffset(offsetX, offsetY);
    auto offset = OffsetF(offsetX.ConvertToPx(), offsetY.ConvertToPx());
    if (childLayoutProperty->GetPositionProperty()) {
        align = childLayoutProperty->GetPositionProperty()->GetAlignment().value_or(align);
    }

    auto childSize = overlayNode_->GetGeometryNode()->GetMarginFrameSize();
    auto translate = Alignment::GetAlignPosition(size, childSize, align) + offset;
    overlayNode_->GetGeometryNode()->SetMarginFrameOffset(translate);
    overlayNode_->Layout();
}

void FrameNode::DoRemoveChildInRenderTree(uint32_t index, bool isAll)
{
    isActive_ = false;
}

void FrameNode::OnInspectorIdUpdate(const std::string& /*unused*/)
{
    auto parent = GetAncestorNodeOfFrame();
    CHECK_NULL_VOID(parent);
    if (parent->GetTag() == V2::RELATIVE_CONTAINER_ETS_TAG) {
        parent->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    }
}

void FrameNode::AddFrameNodeSnapshot(bool isHit, int32_t parentId)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto eventMgr = context->GetEventManager();
    CHECK_NULL_VOID(eventMgr);

    FrameNodeSnapshot info = {
        .nodeId = GetId(),
        .parentNodeId = parentId,
        .tag = GetTag(),
        .comId = propInspectorId_.value_or(""),
        .monopolizeEvents = GetMonopolizeEvents(),
        .isHit = isHit,
        .hitTestMode = static_cast<int32_t>(GetHitTestMode())
    };
    eventMgr->GetEventTreeRecord().AddFrameNodeSnapshot(std::move(info));
}

int32_t FrameNode::GetUiExtensionId()
{
    if (pattern_) {
        return pattern_->GetUiExtensionId();
    }
    return -1;
}

int32_t FrameNode::WrapExtensionAbilityId(int32_t extensionOffset, int32_t abilityId)
{
    if (pattern_) {
        return pattern_->WrapExtensionAbilityId(extensionOffset, abilityId);
    }
    return -1;
}

void FrameNode::SearchExtensionElementInfoByAccessibilityIdNG(int32_t elementId, int32_t mode,
    int32_t offset, std::list<Accessibility::AccessibilityElementInfo>& output)
{
    if (pattern_) {
        pattern_->SearchExtensionElementInfoByAccessibilityId(elementId, mode, offset, output);
    }
}

void FrameNode::SearchElementInfosByTextNG(int32_t elementId, const std::string& text,
    int32_t offset, std::list<Accessibility::AccessibilityElementInfo>& output)
{
    if (pattern_) {
        pattern_->SearchElementInfosByText(elementId, text, offset, output);
    }
}

void FrameNode::FindFocusedExtensionElementInfoNG(int32_t elementId, int32_t focusType,
    int32_t offset, Accessibility::AccessibilityElementInfo& output)
{
    if (pattern_) {
        pattern_->FindFocusedElementInfo(elementId, focusType, offset, output);
    }
}

void FrameNode::FocusMoveSearchNG(int32_t elementId, int32_t direction,
    int32_t offset, Accessibility::AccessibilityElementInfo& output)
{
    if (pattern_) {
        pattern_->FocusMoveSearch(elementId, direction, offset, output);
    }
}

bool FrameNode::TransferExecuteAction(int32_t elementId, const std::map<std::string, std::string>& actionArguments,
    int32_t action, int32_t offset)
{
    bool isExecuted = false;
    if (pattern_) {
        isExecuted = pattern_->TransferExecuteAction(elementId, actionArguments, action, offset);
    }
    return isExecuted;
}

} // namespace OHOS::Ace::NG
