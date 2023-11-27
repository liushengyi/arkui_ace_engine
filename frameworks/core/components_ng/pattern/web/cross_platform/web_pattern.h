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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_PATTERN_H

#include <optional>
#include <string>
#include <utility>

#include "base/thread/cancelable_callback.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "base/geometry/axis.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/web/web_property.h"
#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/manager/select_overlay/select_overlay_manager.h"
#include "core/components_ng/manager/select_overlay/select_overlay_proxy.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/web/web_accessibility_property.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/components_ng/pattern/web/web_layout_algorithm.h"
#include "core/components_ng/pattern/web/web_paint_property.h"
#include "core/components_ng/pattern/web/web_pattern_property.h"
#include "core/components_ng/pattern/web/web_paint_method.h"
#include "core/components_ng/pattern/web/web_delegate_interface.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/manager/select_overlay/selection_host.h"
#include "core/components_ng/render/render_surface.h"
#include "core/components_ng/pattern/scrollable/nestable_scroll_container.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"

#include "core/components_ng/pattern/web/web_delegate_interface.h"

namespace OHOS::Ace::NG {
namespace {
struct MouseClickInfo {
    double x = -1;
    double y = -1;
    TimeStamp start;
};

struct TouchInfo {
    double x = -1;
    double y = -1;
    int32_t id = -1;
};

struct TouchHandleState {
    int32_t id = -1;
    int32_t x = -1;
    int32_t y = -1;
    int32_t edge_height = 0;
};

enum WebOverlayType { INSERT_OVERLAY, SELECTION_OVERLAY, INVALID_OVERLAY };
} // namespace

class WebPattern : public Pattern, public SelectionHost {
    DECLARE_ACE_TYPE(WebPattern, Pattern, SelectionHost);

public:
    using SetWebIdCallback = std::function<void(int32_t)>;
    using SetHapPathCallback = std::function<void(const std::string&)>;
    using JsProxyCallback = std::function<void()>;
    using OnControllerAttachedCallback = std::function<void()>;
    WebPattern();
    WebPattern(std::string webSrc, const RefPtr<WebController>& webController, WebType type = WebType::SURFACE);
    WebPattern(std::string webSrc, const SetWebIdCallback& setWebIdCallback, WebType type = WebType::SURFACE);

    ~WebPattern() override;

    enum class VkState {
        VK_NONE,
        VK_SHOW,
        VK_HIDE
    };

    std::optional<RenderContext::ContextParam> GetContextParam() const override
    {
        if (type_ == WebType::TEXTURE) {
            return RenderContext::ContextParam { RenderContext::ContextType::CANVAS };
        } else {
        return RenderContext::ContextParam { RenderContext::ContextType::SURFACE, "RosenWeb" };
        }
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    bool IsAtomicNode() const override
    {
        return true;
    }

    void UpdateScrollOffset(SizeF frameSize) override;

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<WebEventHub>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<WebAccessibilityProperty>();
    }

    void OnModifyDone() override;

    void SetWebSrc(const std::string& webSrc)
    {
        if (webSrc_ != webSrc_) {
            OnWebSrcUpdate();
            webSrc_ = webSrc;
        }
        if (webPaintProperty_) {
            webPaintProperty_->SetWebPaintData(webSrc);
        }
    }

    const std::optional<std::string>& GetWebSrc() const
    {
        return webSrc_;
    }

    void SetPopup(bool popup)
    {
        isPopup_ = popup;
    }

    void SetParentNWebId(int32_t parentNWebId)
    {
        parentNWebId_ = parentNWebId;
    }

    void SetWebData(const std::string& webData)
    {
        if (webData_ != webData) {
            webData_ = webData;
            OnWebDataUpdate();
        }
        if (webPaintProperty_) {
            webPaintProperty_->SetWebPaintData(webData);
        }
    }

    const std::optional<std::string>& GetWebData() const
    {
        return webData_;
    }

    void SetCustomScheme(const std::string& scheme)
    {
        customScheme_ = scheme;
    }

    const std::optional<std::string>& GetCustomScheme() const
    {
        return customScheme_;
    }

    void SetWebController(const RefPtr<WebController>& webController)
    {
        webController_ = webController;
    }

    RefPtr<WebController> GetWebController() const
    {
        return webController_;
    }

    void SetSetWebIdCallback(SetWebIdCallback&& SetIdCallback)
    {
        setWebIdCallback_ = std::move(SetIdCallback);
    }

    SetWebIdCallback GetSetWebIdCallback() const
    {
        return setWebIdCallback_;
    }

    void SetWebType(WebType type)
    {
        type_ = type;
    }

    WebType GetWebType()
    {
        return type_;
    }

    void SetOnControllerAttachedCallback(OnControllerAttachedCallback&& callback)
    {
        onControllerAttachedCallback_ = std::move(callback);
    }

    OnControllerAttachedCallback GetOnControllerAttachedCallback()
    {
        return onControllerAttachedCallback_;
    }

    void SetSetHapPathCallback(SetHapPathCallback&& callback)
    {
        setHapPathCallback_ = std::move(callback);
    }

    SetHapPathCallback GetSetHapPathCallback() const
    {
        return setHapPathCallback_;
    }

    void SetJsProxyCallback(JsProxyCallback&& jsProxyCallback)
    {
        jsProxyCallback_ = std::move(jsProxyCallback);
    }

    void CallJsProxyCallback()
    {
        if (jsProxyCallback_) {
            jsProxyCallback_();
        }
    }

    RefPtr<WebEventHub> GetWebEventHub()
    {
        return GetEventHub<WebEventHub>();
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true };
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        if (!webPaintProperty_) {
            webPaintProperty_ = MakeRefPtr<WebPaintProperty>();
            if (!webPaintProperty_) {
            }
        }
        return webPaintProperty_;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<WebLayoutAlgorithm>();
    }

    bool BetweenSelectedPosition(const Offset& globalOffset) override
    {
        return false;
    }

    int32_t GetDragRecordSize() override
    {
        return 1;
    }

    void SetNestedScroll(const NestedScrollOptions& nestedOpt);
    /**
     *  End of NestableScrollContainer implementations
     */

    ACE_DEFINE_PROPERTY_GROUP(WebProperty, WebPatternProperty);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, JsEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MediaPlayGestureAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OnLineImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DomStorageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MixedMode, MixedModeContent);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ZoomAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, GeolocationAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, UserAgent, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, CacheMode, WebCacheMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverviewModeAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileFromUrlAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DatabaseAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, TextZoomRatio, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebDebuggingAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BackgroundColor, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, InitialScale, float);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, PinchSmoothModeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MultiWindowAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AllowWindowOpenMethod, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebCursiveFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebFantasyFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebFixedFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebSansSerifFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebSerifFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebStandardFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultFixedFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MinFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MinLogicalFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BlockNetwork, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DarkMode, WebDarkMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ForceDarkAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AudioResumeInterval, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AudioExclusive, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, HorizontalScrollBarAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, VerticalScrollBarAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ScrollBarColor, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverScrollMode, int32_t);
    
    void RequestFullScreen();
    void ExitFullScreen();
    bool IsFullScreen() const
    {
        return isFullScreen_;
    }
    void UpdateLocale();
    void SetDrawRect(int32_t x, int32_t y, int32_t width, int32_t height);
    void OnCompleteSwapWithNewSize();
    void OnResizeNotWork();
    bool OnBackPressed() const;
    void SetFullScreenExitHandler(const std::shared_ptr<FullScreenEnterEvent>& fullScreenExitHandler);
    void UpdateJavaScriptOnDocumentStart();
    void JavaScriptOnDocumentStart(const ScriptItems& scriptItems);
#ifdef ENABLE_DRAG_FRAMEWORK
    bool NotifyStartDragTask();
    bool IsImageDrag();
    DragRet GetDragAcceptableStatus();
    Offset GetDragOffset() const;
#endif
    void SetLayoutMode(WebLayoutMode mode)
    {
        layoutMode_ = mode;
    }
    WebLayoutMode GetLayoutMode() const
    {
        return layoutMode_;
    }
    void OnRootLayerChanged(int width, int height);
    int GetRootLayerWidth() const
    {
        return rootLayerWidth_;
    }
    int GetRootLayerHeight() const
    {
        return rootLayerHeight_;
    }

private:
    void RegistVirtualKeyBoardListener();
    bool ProcessVirtualKeyBoard(int32_t width, int32_t height, double keyboard);
    void UpdateWebLayoutSize(int32_t width, int32_t height);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnWindowShow() override;
    void OnWindowHide() override;
    void OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) override;
    void OnInActive() override;
    void OnActive() override;
    void OnVisibleChange(bool isVisible) override;
    void OnAreaChangedInner() override;

    void OnWebSrcUpdate();
    void OnWebDataUpdate();
    void OnJsEnabledUpdate(bool value);
    void OnMediaPlayGestureAccessUpdate(bool value);
    void OnFileAccessEnabledUpdate(bool value);
    void OnOnLineImageAccessEnabledUpdate(bool value);
    void OnDomStorageAccessEnabledUpdate(bool value);
    void OnImageAccessEnabledUpdate(bool value);
    void OnMixedModeUpdate(MixedModeContent value);
    void OnZoomAccessEnabledUpdate(bool value);
    void OnGeolocationAccessEnabledUpdate(bool value);
    void OnUserAgentUpdate(const std::string& value);
    void OnCacheModeUpdate(WebCacheMode value);
    void OnOverviewModeAccessEnabledUpdate(bool value);
    void OnFileFromUrlAccessEnabledUpdate(bool value);
    void OnDatabaseAccessEnabledUpdate(bool value);
    void OnTextZoomRatioUpdate(int32_t value);
    void OnWebDebuggingAccessEnabledUpdate(bool value);
    void OnPinchSmoothModeEnabledUpdate(bool value);
    void OnBackgroundColorUpdate(int32_t value);
    void OnInitialScaleUpdate(float value);
    void OnMultiWindowAccessEnabledUpdate(bool value);
    void OnAllowWindowOpenMethodUpdate(bool value);
    void OnWebCursiveFontUpdate(const std::string& value);
    void OnWebFantasyFontUpdate(const std::string& value);
    void OnWebFixedFontUpdate(const std::string& value);
    void OnWebSerifFontUpdate(const std::string& value);
    void OnWebSansSerifFontUpdate(const std::string& value);
    void OnWebStandardFontUpdate(const std::string& value);
    void OnDefaultFixedFontSizeUpdate(int32_t value);
    void OnDefaultFontSizeUpdate(int32_t value);
    void OnMinFontSizeUpdate(int32_t value);
    void OnMinLogicalFontSizeUpdate(int32_t value);
    void OnBlockNetworkUpdate(bool value);
    void OnDarkModeUpdate(WebDarkMode mode);
    void OnForceDarkAccessUpdate(bool access);
    void OnAudioResumeIntervalUpdate(int32_t resumeInterval);
    void OnAudioExclusiveUpdate(bool audioExclusive);
    void OnHorizontalScrollBarAccessEnabledUpdate(bool value);
    void OnVerticalScrollBarAccessEnabledUpdate(bool value);
    void OnScrollBarColorUpdate(const std::string& value);
    void OnOverScrollModeUpdate(const int32_t value);

    void InitEvent();
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitMouseEvent(const RefPtr<InputEventHub>& inputHub);
    void InitHoverEvent(const RefPtr<InputEventHub>& inputHub);
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleMouseEvent(MouseInfo& info);
    void WebOnMouseEvent(const MouseInfo& info);
    bool HandleDoubleClickEvent(const MouseInfo& info);
    void SendDoubleClickEvent(const MouseClickInfo& info);
    void InitFocusEvent(const RefPtr<FocusHub>& focusHub);
    void HandleFocusEvent();
    void HandleBlurEvent(const BlurReason& blurReason);
    bool HandleKeyEvent(const KeyEvent& keyEvent);
    bool WebOnKeyEvent(const KeyEvent& keyEvent);
    void WebRequestFocus();
    void ResetDragAction();
    RefPtr<ScrollPattern> SearchParent();
    void InitScrollUpdateListener();
    void CalculateHorizontalDrawRect(const SizeF frameSize);
    void CalculateVerticalDrawRect(const SizeF frameSize);
#ifdef ENABLE_DRAG_FRAMEWORK
    void InitCommonDragDropEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitWebEventHubDragDropStart(const RefPtr<WebEventHub>& eventHub);
    void InitWebEventHubDragDropEnd(const RefPtr<WebEventHub>& eventHub);
    void HandleDragMove(const GestureEvent& event);
    void InitDragEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleDragStart(int32_t x, int32_t y);
    void HandleDragEnd(int32_t x, int32_t y);
    void HandleDragCancel();
    void ClearDragData();
    bool GenerateDragDropInfo(NG::DragDropInfo& dragDropInfo);
    NG::DragDropInfo HandleOnDragStart(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDragEnter(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDropMove(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDragDrop(const RefPtr<OHOS::Ace::DragEvent>& info);
    void HandleOnDragLeave(int32_t x, int32_t y);
    void HandleOnDragEnd(int32_t x, int32_t y);
#endif
    int onDragMoveCnt = 0;
    std::chrono::time_point<std::chrono::system_clock> firstMoveInTime;
    std::chrono::time_point<std::chrono::system_clock> preMoveInTime;
    std::chrono::time_point<std::chrono::system_clock> curMoveInTime;
    CancelableCallback<void()> timer_;
    int32_t duration_ = 100; // 100: 100ms
    void DoRepeat();
    void StartRepeatTimer();

    void HandleTouchDown(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchUp(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchMove(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchCancel(const TouchEventInfo& info);

    std::optional<OffsetF> GetCoordinatePoint();

    struct TouchInfo {
        float x = -1.0f;
        float y = -1.0f;
        int32_t id = -1;
    };
    static bool ParseTouchInfo(const TouchEventInfo& info, std::list<TouchInfo>& touchInfos);
    void InitEnhanceSurfaceFlag();
    void UpdateBackgroundColorRightNow(int32_t color);
    void UpdateContentOffset(const RefPtr<LayoutWrapper>& dirty);
    void PostTaskToUI(const std::function<void()>&& task) const;

    std::optional<std::string> webSrc_;
    std::optional<std::string> webData_;
    std::optional<std::string> customScheme_;
    RefPtr<WebController> webController_;
    SetWebIdCallback setWebIdCallback_ = nullptr;
    WebType type_;
    SetHapPathCallback setHapPathCallback_ = nullptr;
    JsProxyCallback jsProxyCallback_ = nullptr;
    OnControllerAttachedCallback onControllerAttachedCallback_ = nullptr;
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<InputEvent> hoverEvent_;
    RefPtr<PanEvent> panEvent_ = nullptr;
    RefPtr<WebPaintProperty> webPaintProperty_ = nullptr;
    RefPtr<DragEvent> dragEvent_;
    bool isUrlLoaded_ = false;
    std::queue<MouseClickInfo> doubleClickQueue_;
    bool isFullScreen_ = false;
    std::shared_ptr<FullScreenEnterEvent> fullScreenExitHandler_ = nullptr;
    bool needOnFocus_ = false;
    Size drawSize_;
    Size drawSizeCache_;
    bool needUpdateWeb_ = true;
    bool isFocus_ = false;
    VkState isVirtualKeyBoardShow_ { VkState::VK_NONE };
    bool isDragging_ = false;
    bool isW3cDragEvent_ = false;
    bool isWindowShow_ = true;
    bool isActive_ = true;
    bool isEnhanceSurface_ = false;
    bool isAllowWindowOpenMethod_ = false;
    OffsetF webOffset_;
    bool isPopup_ = false;
    int32_t parentNWebId_ = -1;
    bool isInWindowDrag_ = false;
    bool isWaiting_ = false;
    bool isDisableDrag_ = false;
    bool isMouseEvent_ = false;
    bool isVisible_ = true;
    bool isVisibleActiveEnable_ = true;
    bool isMemoryLevelEnable_ = true;
    bool isParentHasScroll_ = false;
    OffsetF relativeOffsetOfScroll_;
    RefPtr<WebDelegateInterface> delegate_ = nullptr;

    bool selectPopupMenuShowing_ = false;
    WebLayoutMode layoutMode_ = WebLayoutMode::NONE;
    int32_t rootLayerWidth_ = 0;
    int32_t rootLayerHeight_ = 0;
    ACE_DISALLOW_COPY_AND_MOVE(WebPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_PATTERN_H
