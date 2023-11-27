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

#include "base/log/log_wrapper.h"

#include <cstring>
#include <map>
#include <unordered_map>

#include "hilog/log.h"

#ifdef ACE_INSTANCE_LOG
#include "core/common/container.h"
#endif

extern "C" {
int HiLogPrintArgs(LogType type, LogLevel level, unsigned int domain, const char* tag, const char* fmt, va_list ap);
}

namespace OHOS::Ace {

namespace {

const ::LogLevel LOG_LEVELS[] = {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
};

const std::map<AceLogTag, const char*> DOMAIN_CONTENTS_MAP = {
    { AceLogTag::DEFAULT, "Ace" },
    { AceLogTag::ACE_ALPHABET_INDEXER, "AceAlphabetIndexer" },
    { AceLogTag::ACE_COUNTER, "AceCounter" },
    { AceLogTag::ACE_SUB_WINDOW, "AceSubWindow" },
    { AceLogTag::ACE_FORM, "AceForm" },
    { AceLogTag::ACE_DRAG, "AceDrag" },
    { AceLogTag::ACE_VIDEO, "AceVideo" },
    { AceLogTag::ACE_COMPONENT_SNAPSHOT, "AceComponentSnapshot" },
    { AceLogTag::ACE_CANVAS, "AceCanvas" },
    { AceLogTag::ACE_INPUTTRACKING, "AceInputTracking" },
    { AceLogTag::ACE_REFRESH, "AceRefresh" },
    { AceLogTag::ACE_SCROLL, "AceScroll" },
    { AceLogTag::ACE_SCROLLABLE, "AceScrollable" },
    { AceLogTag::ACE_INNER_SCROLL_BAR, "AceInnerScrollBar" },
    { AceLogTag::ACE_OUTER_SCROLL_BAR, "AceOuterScrollBar" },
    { AceLogTag::ACE_FONT, "AceFont" },
    { AceLogTag::ACE_OVERLAY, "AceOverlay" },
    { AceLogTag::ACE_PROMPT_ACTION_TOAST, "AcePromptActionToast" },
    { AceLogTag::ACE_PROMPT_ACTION_MENU, "AcePromptActionMenu" },
    { AceLogTag::ACE_DIALOG_ALERT, "AceDialogAlert" },
    { AceLogTag::ACE_DIALOG_TEXTPICKER, "AceDialogTextPicker" },
    { AceLogTag::ACE_DIALOG_TIMEPICKER, "AceDialogTimePicker" },
    { AceLogTag::ACE_DIALOG_DATEPICKER, "AceDialogDatePicker" },
    { AceLogTag::ACE_DIALOG, "AceDialog" },
    { AceLogTag::ACE_PANEL, "AcePanel" },
    { AceLogTag::ACE_MENU, "AceMenu" },
    { AceLogTag::ACE_TEXTINPUT, "AceTextInput" },
    { AceLogTag::ACE_TEXT, "AceText" },
    { AceLogTag::ACE_TEXTAREA, "AceTextArea" },
    { AceLogTag::ACE_TEXT_FIELD, "AceTextField" },
    { AceLogTag::ACE_TEXT_CLOCK, "AceTextClock" },
    { AceLogTag::ACE_TEXT_TIMER, "AceTextTimer" },
    { AceLogTag::ACE_SWIPER, "AceSwiper" },
    { AceLogTag::ACE_TABS, "AceTabs" },
    { AceLogTag::ACE_DIVIDER, "AceDivider" },
    { AceLogTag::ACE_BLANK, "AceBlank" },
    { AceLogTag::ACE_GRIDROW, "AceGridRow" },
    { AceLogTag::ACE_RICH_TEXT, "AceRichText" },
    { AceLogTag::ACE_WEB, "AceWeb" },
    { AceLogTag::ACE_FOCUS, "AceFocus" },
    { AceLogTag::ACE_GESTURE, "AceGesture" },
    { AceLogTag::ACE_IMAGE, "AceImage" },
    { AceLogTag::ACE_RATING, "AceRating" },
    { AceLogTag::ACE_LIST, "AceList" },
    { AceLogTag::ACE_NAVIGATION, "AceNavigation" },
    { AceLogTag::ACE_WATERFLOW, "AceWaterFlow" },
    { AceLogTag::ACE_LOADINGPROGRESS, "AceLoadingProgress" },
    { AceLogTag::ACE_PATTERNLOCK, "AcePatternLock" },
    { AceLogTag::ACE_PROGRESS, "AceProgress" },
    { AceLogTag::ACE_QRCODE, "AceQRCode" },
    { AceLogTag::ACE_ACCESSIBILITY, "AceAccessibility" },
    { AceLogTag::ACE_ROUTER, "AceRouter" },
    { AceLogTag::ACE_THEME, "AceTheme" },
    { AceLogTag::ACE_BORDER, "AceBorder" },
    { AceLogTag::ACE_BORDER_IMAGE, "AceBorderImage" },
    { AceLogTag::ACE_LINEAR_SPLIT, "AceLinearSplit" },
    { AceLogTag::ACE_GRID, "AceGrid" },
    { AceLogTag::ACE_PLUGINCOMPONENT, "AcePluginComponent" },
    { AceLogTag::ACE_UIEXTENSIONCOMPONENT, "AceUiExtensionComponent" },
    { AceLogTag::ACE_IF, "AceIf" },
    { AceLogTag::ACE_FOREACH, "AceForEach" },
    { AceLogTag::ACE_LAZYFOREACH, "AceLazyForEach" },
    { AceLogTag::ACE_GAUGE, "AceGauge" },
    { AceLogTag::ACE_HYPERLINK, "AceHyperLink" },
    { AceLogTag::ACE_ANIMATION, "AceAnimation" },
    { AceLogTag::ACE_DATE_PICKER, "AceDatePicker" },
    { AceLogTag::ACE_TEXT_PICKER, "AceTextPicker" },
};

const char* APP_DOMAIN_CONTENT = "JSApp";

constexpr uint32_t LOG_DOMAINS[] = {
    0xD003900,
    0xC0D0,
};

constexpr LogType LOG_TYPES[] = {
    LOG_CORE,
    LOG_APP,
};

}

// initial static member object
LogLevel LogWrapper::level_ = LogLevel::DEBUG;

char LogWrapper::GetSeparatorCharacter()
{
    return '/';
}

void LogWrapper::PrintLog(LogDomain domain, LogLevel level, AceLogTag tag, const char* fmt, va_list args)
{
    uint32_t hilogDomain = LOG_DOMAINS[static_cast<uint32_t>(domain)] + static_cast<uint32_t>(tag);
    const char* domainContent = domain == LogDomain::FRAMEWORK ? DOMAIN_CONTENTS_MAP.at(tag) : APP_DOMAIN_CONTENT;
#ifdef ACE_PRIVATE_LOG
    std::string newFmt(fmt);
    ReplaceFormatString("{private}", "{public}", newFmt);
    HiLogPrintArgs(LOG_TYPES[static_cast<uint32_t>(domain)], LOG_LEVELS[static_cast<uint32_t>(level)],
        hilogDomain, domainContent, newFmt.c_str(), args);
#else
    HiLogPrintArgs(LOG_TYPES[static_cast<uint32_t>(domain)], LOG_LEVELS[static_cast<uint32_t>(level)],
        hilogDomain, domainContent, fmt, args);
#endif
}

int32_t LogWrapper::GetId()
{
#ifdef ACE_INSTANCE_LOG
    return Container::CurrentId();
#else
    return 0;
#endif
}

} // namespace OHOS::Ace
