/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "base/log/ace_trace.h"

namespace OHOS::Ace {

AceScopedTrace::AceScopedTrace(const char* format, ...)
{
    traceEnabled_ = false;
}

AceScopedTrace::~AceScopedTrace()
{
}

AceAsyncScopedTrace::AceAsyncScopedTrace(const char* format, ...)
{
    asyncTraceEnabled_ = false;
    taskId_ = 0;
}

AceAsyncScopedTrace::~AceAsyncScopedTrace()
{
}

bool AceTraceEnabled()
{
    return false;
}
void AceTraceEnd()
{}
void AceAsyncTraceEnd(int32_t taskId, const char* name)
{}
}