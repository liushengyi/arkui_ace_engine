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
#include "core/common/recorder/exposure_processor.h"

#include "base/utils/time_util.h"
#include "core/common/recorder/event_recorder.h"
#include "core/common/recorder/node_data_cache.h"

namespace OHOS::Ace::Recorder {
ExposureProcessor::ExposureProcessor(const std::string& inspectorId)
{
    if (!inspectorId.empty()) {
        NodeDataCache::Get().GetExposureCfg(inspectorId, cfg_);
        if (IsNeedRecord()) {
            TAG_LOGD(AceLogTag::ACE_UIEVENT, "ExposureProcessor id %{public}s", cfg_.id.c_str());
        }
    }
}

bool ExposureProcessor::IsNeedRecord() const
{
    return !cfg_.id.empty();
}

double ExposureProcessor::GetRatio() const
{
    return cfg_.ratio;
}

void ExposureProcessor::OnVisibleChange(bool isVisible)
{
    auto current = GetCurrentTimestamp();
    if (isVisible) {
        startTime_ = current;
        pageUrl_ = EventRecorder::Get().GetPageUrl();
        navDstName_ = EventRecorder::Get().GetNavDstName();
    } else if (startTime_ > 0) {
        auto duration = current - startTime_;
        if (duration >= cfg_.duration) {
            EventParamsBuilder builder;
            builder.SetId(cfg_.id)
                .SetPageUrl(pageUrl_)
                .SetNavDst(navDstName_)
                .SetExtra(KEY_DURATION, std::to_string(duration));
            EventRecorder::Get().OnExposure(std::move(builder));
        }
    }
}
} // namespace OHOS::Ace::Recorder
