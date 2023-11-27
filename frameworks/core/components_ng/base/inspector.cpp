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

#include "core/components_ng/base/inspector.h"

#include <unordered_set>

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
const char INSPECTOR_TYPE[] = "$type";
const char INSPECTOR_ID[] = "$ID";
const char INSPECTOR_RECT[] = "$rect";
const char INSPECTOR_ATTRS[] = "$attrs";
const char INSPECTOR_ROOT[] = "root";
const char INSPECTOR_WIDTH[] = "width";
const char INSPECTOR_HEIGHT[] = "height";
const char INSPECTOR_RESOLUTION[] = "$resolution";
const char INSPECTOR_CHILDREN[] = "$children";
const char INSPECTOR_DEBUGLINE[] = "$debugLine";
#ifdef PREVIEW
const char INSPECTOR_VIEW_ID[] = "$viewID";
#endif

const uint32_t LONG_PRESS_DELAY = 1000;
RectF deviceRect;

RefPtr<UINode> GetInspectorByKey(const RefPtr<FrameNode>& root, const std::string& key)
{
    std::queue<RefPtr<UINode>> elements;
    elements.push(root);
    RefPtr<UINode> inspectorElement;
    while (!elements.empty()) {
        auto current = elements.front();
        elements.pop();
        if (key == current->GetInspectorId().value_or("")) {
            return current;
        }

        const auto& children = current->GetChildren();
        for (const auto& child : children) {
            elements.push(child);
        }
    }
    return nullptr;
}

void DumpElementTree(
    int32_t depth, const RefPtr<UINode>& element, std::map<int32_t, std::list<RefPtr<UINode>>>& depthElementMap)
{
    if (element->GetChildren().empty()) {
        return;
    }
    const auto& children = element->GetChildren();
    depthElementMap[depth].insert(depthElementMap[depth].end(), children.begin(), children.end());
    for (const auto& depthElement : children) {
        DumpElementTree(depth + 1, depthElement, depthElementMap);
    }
}

TouchEvent GetUpPoint(const TouchEvent& downPoint)
{
    return {
        .x = downPoint.x, .y = downPoint.y, .type = TouchType::UP, .time = std::chrono::high_resolution_clock::now()
    };
}
#ifdef PREVIEW
void GetFrameNodeChildren(const RefPtr<NG::UINode>& uiNode, std::vector<RefPtr<NG::UINode>>& children, int32_t pageId)
{
    // Set ViewId for the fast preview.
    auto parent = uiNode->GetParent();
    if (parent && parent->GetTag() == "JsView") {
        uiNode->SetViewId(std::to_string(parent->GetId()));
    } else {
        uiNode->SetViewId(parent->GetViewId());
    }
    if (uiNode->GetTag() == "stage") {
    } else if (uiNode->GetTag() == "page") {
        if (uiNode->GetPageId() != pageId) {
            return;
        }
    } else {
        if (!uiNode->GetDebugLine().empty()) {
            children.emplace_back(uiNode);
            return;
        }
    }

    for (const auto& frameChild : uiNode->GetChildren()) {
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

void GetSpanInspector(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId)
{
    // span rect follows parent text size
    auto spanParentNode = parent->GetParent();
    CHECK_NULL_VOID(spanParentNode);
    auto node = AceType::DynamicCast<FrameNode>(spanParentNode);
    CHECK_NULL_VOID(node);
    auto jsonNode = JsonUtil::Create(true);
    auto jsonObject = JsonUtil::Create(true);
    parent->ToJsonValue(jsonObject);
    jsonNode->Put(INSPECTOR_ATTRS, jsonObject);
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    RectF rect = node->GetTransformRectRelativeToWindow();
    rect = rect.Constrain(deviceRect);
    if (rect.IsEmpty()) {
        rect.SetRect(0, 0, 0, 0);
    }
    auto strRec = std::to_string(rect.Left())
                      .append(",")
                      .append(std::to_string(rect.Top()))
                      .append(",")
                      .append(std::to_string(rect.Width()))
                      .append(",")
                      .append(std::to_string(rect.Height()));
    jsonNode->Put(INSPECTOR_RECT, strRec.c_str());
    jsonNode->Put(INSPECTOR_DEBUGLINE, parent->GetDebugLine().c_str());
    jsonNode->Put(INSPECTOR_VIEW_ID, parent->GetViewId().c_str());
    jsonNodeArray->Put(jsonNode);
}

void GetInspectorChildren(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId, bool isActive)
{
    // Span is a special case in Inspector since span inherits from UINode
    if (AceType::InstanceOf<SpanNode>(parent)) {
        GetSpanInspector(parent, jsonNodeArray, pageId);
        return;
    }
    auto jsonNode = JsonUtil::Create(true);
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    auto node = AceType::DynamicCast<FrameNode>(parent);
    if (node) {
        RectF rect;
        isActive = isActive && node->IsActive();
        if (isActive) {
            rect = node->GetTransformRectRelativeToWindow();
        }
        rect = rect.Constrain(deviceRect);
        if (rect.IsEmpty()) {
            rect.SetRect(0, 0, 0, 0);
        }
        auto strRec = std::to_string(rect.Left())
                          .append(",")
                          .append(std::to_string(rect.Top()))
                          .append(",")
                          .append(std::to_string(rect.Width()))
                          .append(",")
                          .append(std::to_string(rect.Height()));
        jsonNode->Put(INSPECTOR_RECT, strRec.c_str());
        jsonNode->Put(INSPECTOR_DEBUGLINE, node->GetDebugLine().c_str());
        jsonNode->Put(INSPECTOR_VIEW_ID, node->GetViewId().c_str());
        auto jsonObject = JsonUtil::Create(true);
        parent->ToJsonValue(jsonObject);
        jsonNode->Put(INSPECTOR_ATTRS, jsonObject);
    }

    std::vector<RefPtr<NG::UINode>> children;
    for (const auto& item : parent->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    auto jsonChildrenArray = JsonUtil::CreateArray(true);
    for (auto uiNode : children) {
        GetInspectorChildren(uiNode, jsonChildrenArray, pageId, isActive);
    }
    if (jsonChildrenArray->GetArraySize()) {
        jsonNode->Put(INSPECTOR_CHILDREN, jsonChildrenArray);
    }
    jsonNodeArray->Put(jsonNode);
}

#else
void GetFrameNodeChildren(const RefPtr<NG::UINode>& uiNode, std::vector<RefPtr<NG::UINode>>& children, int32_t pageId)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode) || AceType::InstanceOf<SpanNode>(uiNode)) {
        if (uiNode->GetTag() == "stage") {
        } else if (uiNode->GetTag() == "page") {
            if (uiNode->GetPageId() != pageId) {
                return;
            }
        } else {
            auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
            auto spanNode = AceType::DynamicCast<NG::SpanNode>(uiNode);
            if ((frameNode && !frameNode->IsInternal()) || spanNode) {
                children.emplace_back(uiNode);
                return;
            }
        }
    }

    for (const auto& frameChild : uiNode->GetChildren()) {
        GetFrameNodeChildren(frameChild, children, pageId);
    }
}

void GetSpanInspector(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId)
{
    // span rect follows parent text size
    auto spanParentNode = parent->GetParent();
    CHECK_NULL_VOID(spanParentNode);
    auto node = AceType::DynamicCast<FrameNode>(spanParentNode);
    CHECK_NULL_VOID(node);
    auto jsonNode = JsonUtil::Create(true);
    auto jsonObject = JsonUtil::Create(true);
    parent->ToJsonValue(jsonObject);
    jsonNode->Put(INSPECTOR_ATTRS, jsonObject);
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    jsonNode->Put(INSPECTOR_DEBUGLINE, parent->GetDebugLine().c_str());
    RectF rect = node->GetTransformRectRelativeToWindow();
    jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
    jsonNodeArray->Put(jsonNode);
}

void GetInspectorChildren(
    const RefPtr<NG::UINode>& parent, std::unique_ptr<OHOS::Ace::JsonValue>& jsonNodeArray, int pageId, bool isActive)
{
    // Span is a special case in Inspector since span inherits from UINode
    if (AceType::InstanceOf<SpanNode>(parent)) {
        GetSpanInspector(parent, jsonNodeArray, pageId);
        return;
    }
    auto jsonNode = JsonUtil::Create(true);
    jsonNode->Put(INSPECTOR_TYPE, parent->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, parent->GetId());
    auto node = AceType::DynamicCast<FrameNode>(parent);
    auto ctx = node->GetRenderContext();

    RectF rect;
    isActive = isActive && node->IsActive();
    if (isActive) {
        rect = node->GetTransformRectRelativeToWindow();
    }

    jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
    jsonNode->Put(INSPECTOR_DEBUGLINE, node->GetDebugLine().c_str());
    auto jsonObject = JsonUtil::Create(true);
    parent->ToJsonValue(jsonObject);
    jsonNode->Put(INSPECTOR_ATTRS, jsonObject);
    std::vector<RefPtr<NG::UINode>> children;
    for (const auto& item : parent->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    auto jsonChildrenArray = JsonUtil::CreateArray(true);
    for (auto uiNode : children) {
        GetInspectorChildren(uiNode, jsonChildrenArray, pageId, isActive);
    }
    if (jsonChildrenArray->GetArraySize()) {
        jsonNode->Put(INSPECTOR_CHILDREN, jsonChildrenArray);
    }
    jsonNodeArray->Put(jsonNode);
}
#endif

RefPtr<NG::UINode> GetOverlayNode(const RefPtr<NG::UINode>& pageNode)
{
    CHECK_NULL_RETURN(pageNode, nullptr);
    auto stageNode = pageNode->GetParent();
    CHECK_NULL_RETURN(stageNode, nullptr);
    auto stageParent = stageNode->GetParent();
    CHECK_NULL_RETURN(stageParent, nullptr);
    auto overlayNode = stageParent->GetChildren().back();
    if (overlayNode->GetTag() == "stage") {
        return nullptr;
    }
    return overlayNode;
}

void GetContextInfo(const RefPtr<PipelineContext>& context, std::unique_ptr<JsonValue>& jsonRoot)
{
    auto scale = context->GetViewScale();
    auto rootHeight = context->GetRootHeight();
    auto rootWidth = context->GetRootWidth();
    deviceRect.SetRect(0, 0, rootWidth * scale, rootHeight * scale);
    jsonRoot->Put(INSPECTOR_WIDTH, std::to_string(rootWidth * scale).c_str());
    jsonRoot->Put(INSPECTOR_HEIGHT, std::to_string(rootHeight * scale).c_str());
    jsonRoot->Put(INSPECTOR_RESOLUTION, std::to_string(SystemProperties::GetResolution()).c_str());
}

std::string GetInspectorInfo(std::vector<RefPtr<NG::UINode>> children, int32_t pageId,
    std::unique_ptr<JsonValue> jsonRoot, bool isLayoutInspector)
{
    auto jsonNodeArray = JsonUtil::CreateArray(true);
    for (auto& uiNode : children) {
        GetInspectorChildren(uiNode, jsonNodeArray, pageId, true);
    }
    if (jsonNodeArray->GetArraySize()) {
        jsonRoot->Put(INSPECTOR_CHILDREN, jsonNodeArray);
    }

    if (isLayoutInspector) {
        auto jsonTree = JsonUtil::Create(true);
        jsonTree->Put("type", "root");
        jsonTree->Put("content", jsonRoot);
        return jsonTree->ToString();
    }

    return jsonRoot->ToString();
}
} // namespace

std::set<RefPtr<FrameNode>> Inspector::offscreenNodes;

RefPtr<FrameNode> Inspector::GetFrameNodeByKey(const std::string& key)
{
    if (!offscreenNodes.empty()) {
        for (auto node : offscreenNodes) {
            auto frameNode = AceType::DynamicCast<FrameNode>(GetInspectorByKey(node, key));
            if (frameNode) {
                return frameNode;
            }
        }
    }
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, nullptr);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_RETURN(rootNode, nullptr);

    return AceType::DynamicCast<FrameNode>(GetInspectorByKey(rootNode, key));
}

std::string Inspector::GetInspectorNodeByKey(const std::string& key)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, "");
    auto rootNode = context->GetRootElement();
    CHECK_NULL_RETURN(rootNode, "");

    auto inspectorElement = GetInspectorByKey(rootNode, key);
    CHECK_NULL_RETURN(inspectorElement, "");

    auto jsonNode = JsonUtil::Create(true);
    jsonNode->Put(INSPECTOR_TYPE, inspectorElement->GetTag().c_str());
    jsonNode->Put(INSPECTOR_ID, inspectorElement->GetId());
    auto frameNode = AceType::DynamicCast<FrameNode>(inspectorElement);
    if (frameNode) {
        auto rect = frameNode->GetTransformRectRelativeToWindow();
        jsonNode->Put(INSPECTOR_RECT, rect.ToBounds().c_str());
    }
    auto jsonAttrs = JsonUtil::Create(true);
    std::string debugLine = inspectorElement->GetDebugLine();
    jsonNode->Put(INSPECTOR_DEBUGLINE, debugLine.c_str());
    inspectorElement->ToJsonValue(jsonAttrs);
    jsonNode->Put(INSPECTOR_ATTRS, jsonAttrs);
    return jsonNode->ToString();
}

void Inspector::GetRectangleById(const std::string& key, Rectangle& rectangle)
{
    auto frameNode = Inspector::GetFrameNodeByKey(key);
    CHECK_NULL_VOID(frameNode);
    rectangle.size = frameNode->GetGeometryNode()->GetFrameSize();
    auto context = frameNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    rectangle.localOffset = context->GetPaintRectWithTransform().GetOffset();
    rectangle.windowOffset = frameNode->GetTransformRelativeOffset();
    auto pipeline = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    rectangle.screenRect = pipeline->GetCurrentWindowRect();
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    Matrix4 defMatrix4 = Matrix4::CreateIdentity();
    Matrix4 matrix4 = renderContext->GetTransformMatrixValue(defMatrix4);
    rectangle.matrix4 = matrix4;
    auto rect = renderContext->GetPaintRectWithoutTransform();
    const double halfDimension = 50.0;
    auto center = renderContext->GetTransformCenter().value_or(DimensionOffset(
        Dimension(halfDimension, DimensionUnit::PERCENT), Dimension(halfDimension, DimensionUnit::PERCENT)));
    double centerX = 0.0;
    double centerY = 0.0;
    if (center.GetX().Unit() == DimensionUnit::PERCENT || center.GetY().Unit() == DimensionUnit::PERCENT) {
        if (rect.IsValid()) {
            centerX = Dimension(center.GetX().ConvertToPxWithSize(rect.Width()), DimensionUnit::PX).ConvertToVp();
            centerY = Dimension(center.GetY().ConvertToPxWithSize(rect.Height()), DimensionUnit::PX).ConvertToVp();
        }
    } else {
        centerX = center.GetX().ConvertToVp();
        centerY = center.GetY().ConvertToVp();
    }
    VectorF defScale = VectorF(1.0, 1.0);
    VectorF scale = renderContext->GetTransformScaleValue(defScale);
    rectangle.scale.x = scale.x;
    rectangle.scale.y = scale.y;
    rectangle.scale.z = 1.0;
    rectangle.scale.centerX = centerX;
    rectangle.scale.centerY = centerY;
    Vector5F defRotate = Vector5F(0.0, 0.0, 0.0, 0.0, 0.0);
    Vector5F rotate = renderContext->GetTransformRotateValue(defRotate);
    rectangle.rotate.x = rotate.x;
    rectangle.rotate.y = rotate.y;
    rectangle.rotate.z = rotate.z;
    rectangle.rotate.angle = rotate.w;
    rectangle.rotate.centerX = centerX;
    rectangle.rotate.centerY = centerY;
    TranslateOptions defTranslate = TranslateOptions(0.0, 0.0, 0.0);
    TranslateOptions translate = renderContext->GetTransformTranslateValue(defTranslate);
    if ((translate.x.Unit() == DimensionUnit::PERCENT) && rect.IsValid()) {
        rectangle.translate.x =
            Dimension(translate.x.ConvertToPxWithSize(rect.Width()), DimensionUnit::PX).ConvertToVp();
    } else {
        rectangle.translate.x = translate.x.ConvertToVp();
    }
    if ((translate.y.Unit() == DimensionUnit::PERCENT) && rect.IsValid()) {
        rectangle.translate.y =
            Dimension(translate.y.ConvertToPxWithSize(rect.Height()), DimensionUnit::PX).ConvertToVp();
    } else {
        rectangle.translate.y = translate.y.ConvertToVp();
    }
    rectangle.translate.z = translate.z.ConvertToVp();
}
std::string Inspector::GetInspector(bool isLayoutInspector)
{
    auto jsonRoot = JsonUtil::Create(true);
    jsonRoot->Put(INSPECTOR_TYPE, INSPECTOR_ROOT);

    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, jsonRoot->ToString());
    GetContextInfo(context, jsonRoot);
    auto pageRootNode = context->GetStageManager()->GetLastPage();
    CHECK_NULL_RETURN(pageRootNode, jsonRoot->ToString());
    auto pageId = context->GetStageManager()->GetLastPage()->GetPageId();
    std::vector<RefPtr<NG::UINode>> children;
    for (const auto& item : pageRootNode->GetChildren()) {
        GetFrameNodeChildren(item, children, pageId);
    }
    auto overlayNode = GetOverlayNode(pageRootNode);
    if (overlayNode) {
        GetFrameNodeChildren(overlayNode, children, pageId);
    }

    return GetInspectorInfo(children, pageId, std::move(jsonRoot), isLayoutInspector);
}

std::string Inspector::GetSubWindowInspector(bool isLayoutInspector)
{
    auto jsonRoot = JsonUtil::Create(true);
    jsonRoot->Put(INSPECTOR_TYPE, INSPECTOR_ROOT);

    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, jsonRoot->ToString());
    GetContextInfo(context, jsonRoot);
    auto overlayNode = context->GetOverlayManager()->GetRootNode().Upgrade();
    CHECK_NULL_RETURN(overlayNode, jsonRoot->ToString());
    auto pageId = 0;
    std::vector<RefPtr<NG::UINode>> children;
    GetFrameNodeChildren(overlayNode, children, pageId);

    return GetInspectorInfo(children, 0, std::move(jsonRoot), isLayoutInspector);
}

bool Inspector::SendEventByKey(const std::string& key, int action, const std::string& params)
{
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto rootNode = context->GetRootElement();
    CHECK_NULL_RETURN(rootNode, false);

    auto inspectorElement = AceType::DynamicCast<FrameNode>(GetInspectorByKey(rootNode, key));
    CHECK_NULL_RETURN(inspectorElement, false);

    auto size = inspectorElement->GetGeometryNode()->GetFrameSize();
    auto offset = inspectorElement->GetTransformRelativeOffset();
    Rect rect { offset.GetX(), offset.GetY(), size.Width(), size.Height() };
    context->GetTaskExecutor()->PostTask(
        [weak = AceType::WeakClaim(AceType::RawPtr(context)), rect, action, params]() {
            auto context = weak.Upgrade();
            if (!context) {
                return;
            }

            TouchEvent point { .x = (rect.Left() + rect.Width() / 2),
                .y = (rect.Top() + rect.Height() / 2),
                .type = TouchType::DOWN,
                .time = std::chrono::high_resolution_clock::now() };
            context->OnTouchEvent(point.UpdatePointers());

            switch (action) {
                case static_cast<int>(AceAction::ACTION_CLICK): {
                    context->OnTouchEvent(GetUpPoint(point).UpdatePointers());
                    break;
                }
                case static_cast<int>(AceAction::ACTION_LONG_CLICK): {
                    CancelableCallback<void()> inspectorTimer;
                    auto&& callback = [weak, point]() {
                        auto refPtr = weak.Upgrade();
                        if (refPtr) {
                            refPtr->OnTouchEvent(GetUpPoint(point).UpdatePointers());
                        }
                    };
                    inspectorTimer.Reset(callback);
                    auto taskExecutor =
                        SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
                    taskExecutor.PostDelayedTask(inspectorTimer, LONG_PRESS_DELAY);
                    break;
                }
                default:
                    break;
            }
        },
        TaskExecutor::TaskType::UI);

    return true;
}

void Inspector::HideAllMenus()
{
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->HideAllMenus();
}

void Inspector::AddOffscreenNode(RefPtr<FrameNode> node)
{
    CHECK_NULL_VOID(node);
    offscreenNodes.insert(node);
}

void Inspector::RemoveOffscreenNode(RefPtr<FrameNode> node)
{
    CHECK_NULL_VOID(node);
    offscreenNodes.erase(node);
}

} // namespace OHOS::Ace::NG
