/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "base/log/ace_checker.h"
#include "base/log/ace_performance_check.h"
#include "base/utils/system_properties.h"
#include "bridge/common/utils/engine_helper.h"

namespace OHOS::Ace {
namespace {
constexpr int32_t ORIENTATION_PORTRAIT = 0;
constexpr int32_t ORIENTATION_LANDSCAPE = 1;

void Swap(int32_t& deviceWidth, int32_t& deviceHeight)
{
    int32_t temp = deviceWidth;
    deviceWidth = deviceHeight;
    deviceHeight = temp;
}
} // namespace

DeviceType SystemProperties::deviceType_ = DeviceType::PHONE;
DeviceOrientation SystemProperties::orientation_ { DeviceOrientation::PORTRAIT };
bool SystemProperties::isHookModeEnabled_ = false;
bool SystemProperties::rosenBackendEnabled_ = true;
bool SystemProperties::windowAnimationEnabled_ = true;
double SystemProperties::resolution_ = 0.0;
constexpr float defaultAnimationScale = 1.0f;
bool SystemProperties::extSurfaceEnabled_ = false;
uint32_t SystemProperties::dumpFrameCount_ = 0;
bool SystemProperties::debugEnabled_ = false;
ColorMode SystemProperties::colorMode_ { ColorMode::LIGHT };
int32_t SystemProperties::deviceWidth_ = 720;
int32_t SystemProperties::deviceHeight_ = 1280;

int32_t AceChecker::pageNodes_ = 0;
int32_t AceChecker::pageDepth_ = 0;
int32_t AceChecker::nodeChildren_ = 0;
int32_t AceChecker::functionTimeout_ = 0;
int32_t AceChecker::vsyncTimeout_ = 0;
int32_t AceChecker::nodeTimeout_ = 0;
int32_t AceChecker::foreachItems_ = 0;
int32_t AceChecker::flexLayouts_ = 0;

// =================================================================================
// resolve compile error temporarily and wait
// for unittest cases to be integrated and modified
ScopedDelegate::ScopedDelegate(const RefPtr<Framework::FrontendDelegate>& delegate, int32_t id)
    : delegate_(delegate), scope_(new ContainerScope(id))
{}

ScopedDelegate::~ScopedDelegate()
{
    delete scope_;
    scope_ = nullptr;
}

std::pair<int32_t, int32_t> EngineHelper::GetPositionOnJsCode()
{
    return { 0, 0 };
}

ScopedDelegate EngineHelper::GetCurrentDelegate()
{
    return { nullptr, 0 };
}

void AceScopedPerformanceCheck::RecordPerformanceCheckData(const PerformanceCheckNodeMap& nodeMap, int64_t vsyncTimeout)
{}

AceScopedPerformanceCheck::AceScopedPerformanceCheck(const std::string& /* name */) {}

AceScopedPerformanceCheck::~AceScopedPerformanceCheck() {}

bool AceChecker::IsPerformanceCheckEnabled()
{
    return false;
}

void AceChecker::NotifyCaution(const std::string& tag) {}

void AceChecker::InitPerformanceParameters() {}
// =================================================================================

float SystemProperties::GetFontWeightScale()
{
    // Default value of font weight scale is 1.0.
    return 1.0f;
}

DeviceType SystemProperties::GetDeviceType()
{
    return deviceType_;
}

bool SystemProperties::GetDebugEnabled()
{
    return false;
}

float SystemProperties::GetAnimationScale()
{
    return defaultAnimationScale;
}

bool SystemProperties::GetIsUseMemoryMonitor()
{
    return false;
}

void SystemProperties::SetDeviceOrientation(int32_t orientation)
{
    if (orientation == ORIENTATION_PORTRAIT && orientation_ != DeviceOrientation::PORTRAIT) {
        Swap(deviceWidth_, deviceHeight_);
        orientation_ = DeviceOrientation::PORTRAIT;
    } else if (orientation == ORIENTATION_LANDSCAPE && orientation_ != DeviceOrientation::LANDSCAPE) {
        Swap(deviceWidth_, deviceHeight_);
        orientation_ = DeviceOrientation::LANDSCAPE;
    }
}

} // namespace OHOS::Ace
