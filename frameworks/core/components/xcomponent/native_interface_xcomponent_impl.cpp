/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "frameworks/core/components/xcomponent/native_interface_xcomponent_impl.h"

#include "securec.h"

int32_t OH_NativeXComponent::GetXComponentId(char* id, uint64_t* size)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    auto xcomponentId = xcomponentImpl_->GetXComponentId();
    uint64_t idSize = static_cast<uint64_t>(xcomponentId.size());
    if (idSize > (uint64_t)(OH_XCOMPONENT_ID_LEN_MAX)) {
        LOGE("Length of XComponent id should be no more than OH_XCOMPONENT_ID_LEN_MAX");
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    if (strncpy_s(id, (*size), xcomponentId.c_str(), idSize) == 0) {
        (*size) = idSize;
        return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
    } else {
        return OH_NATIVEXCOMPONENT_RESULT_FAILED;
    }
}

int32_t OH_NativeXComponent::GetNativeWindow(void** window)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    auto surface = const_cast<void*>(xcomponentImpl_->GetSurface());
    if (surface) {
        (*window) = surface;
        return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
    } else {
        return OH_NATIVEXCOMPONENT_RESULT_FAILED;
    }
}

int32_t OH_NativeXComponent::GetXComponentSize(const void* window, uint64_t* width, uint64_t* height)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    auto surfaceWindow = const_cast<void*>(xcomponentImpl_->GetSurface());
    if (window != surfaceWindow) {
        return OH_NATIVEXCOMPONENT_RESULT_FAILED;
    }
    (*width) = static_cast<uint64_t>(xcomponentImpl_->GetXComponentWidth());
    (*height) = static_cast<uint64_t>(xcomponentImpl_->GetXComponentHeight());

    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetXComponentOffset(const void* window, double* x, double* y)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    auto surfaceWindow = const_cast<void*>(xcomponentImpl_->GetSurface());
    if (window != surfaceWindow) {
        return OH_NATIVEXCOMPONENT_RESULT_FAILED;
    }
    (*x) = static_cast<double>(xcomponentImpl_->GetXComponentOffsetX());
    (*y) = static_cast<double>(xcomponentImpl_->GetXComponentOffsetY());

    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetHistoryPoints(const void* window,
    int32_t* size, OH_NativeXComponent_HistoricalPoint** historicalPoints)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    auto surfaceWindow = const_cast<void*>(xcomponentImpl_->GetSurface());
    if (window != surfaceWindow) {
        return OH_NATIVEXCOMPONENT_RESULT_FAILED;
    }
    xcomponentImpl_->GetHistoryPoints(size, historicalPoints);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetTouchEvent(const void* window, OH_NativeXComponent_TouchEvent* touchEvent)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    auto surfaceWindow = const_cast<void*>(xcomponentImpl_->GetSurface());
    if (window != surfaceWindow) {
        return OH_NATIVEXCOMPONENT_RESULT_FAILED;
    }
    (*touchEvent) = xcomponentImpl_->GetTouchEvent();
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetToolType(size_t pointIndex, OH_NativeXComponent_TouchPointToolType* toolType)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    (*toolType) = xcomponentImpl_->GetToolType(pointIndex);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetTiltX(size_t pointIndex, float* tiltX)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    (*tiltX) = xcomponentImpl_->GetTiltX(pointIndex);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetTiltY(size_t pointIndex, float* tiltY)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    (*tiltY) = xcomponentImpl_->GetTiltX(pointIndex);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetMouseEvent(const void* window, OH_NativeXComponent_MouseEvent* mouseEvent)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    auto surfaceWindow = const_cast<void*>(xcomponentImpl_->GetSurface());
    if (window != surfaceWindow) {
        return OH_NATIVEXCOMPONENT_RESULT_FAILED;
    }
    (*mouseEvent) = xcomponentImpl_->GetMouseEvent();
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::RegisterCallback(OH_NativeXComponent_Callback* callback)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    xcomponentImpl_->SetCallback(callback);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::RegisterMouseEventCallback(OH_NativeXComponent_MouseEvent_Callback* callback)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    xcomponentImpl_->SetMouseEventCallback(callback);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::RegisterFocusEventCallback(void (*callback)(OH_NativeXComponent* component, void* window))
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    xcomponentImpl_->SetFocusEventCallback(callback);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::RegisterKeyEventCallback(void (*callback)(OH_NativeXComponent* component, void* window))
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    xcomponentImpl_->SetKeyEventCallback(callback);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::RegisterBlurEventCallback(void (*callback)(OH_NativeXComponent* component, void* window))
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    xcomponentImpl_->SetBlurEventCallback(callback);
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}

int32_t OH_NativeXComponent::GetKeyEvent(OH_NativeXComponent_KeyEvent** keyEvent)
{
    if (xcomponentImpl_ == nullptr) {
        return OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER;
    }
    (*keyEvent) = xcomponentImpl_->GetKeyEvent();
    return OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
}
