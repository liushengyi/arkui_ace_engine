/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

export const defaultTheme = {
    icon: {
        size: { width: 32, height: 32 },
        margin: { top: 12, bottom: 12, left: 12, right: 12 },
        fillColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_fa_icon_secondary"],
            bundleName: "",
            moduleName: ""
        },
        borderRadius: {
            id: -1,
            type: 10002,
            params: ["sys.float.ohos_id_corner_radius_default_s"],
            bundleName: "",
            moduleName: ""
        }
    },
    title: {
        margin: { bottom: 2 },
        minFontSize: 12,
        fontWeight: FontWeight.Normal,
        fontSize: {
            id: -1,
            type: 10002,
            params: ["sys.float.ohos_id_text_size_sub_title2"],
            bundleName: "",
            moduleName: ""
        },
        fontColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_fa_text_primary"],
            bundleName: "",
            moduleName: ""
        }
    },
    button: {
        margin: { top: 16, bottom: 16, left: 16, right: 16 },
        fontSize: {
            id: -1,
            type: 10002,
            params: ["sys.float.ohos_id_text_size_button2"],
            bundleName: "",
            moduleName: ""
        },
        fontColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_fa_text_activated"],
            bundleName: "",
            moduleName: ""
        },
        textMargin: { top: 8, bottom: 8, left: 8, right: 8 },
        minFontSize: 9,
        hoverColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_hover"],
            bundleName: "",
            moduleName: ""
        },
        backgroundColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_background_transparent"],
            bundleName: "",
            moduleName: ""
        }
    },
    message: {
        fontSize: {
            id: -1,
            type: 10002,
            params: ["sys.float.ohos_id_text_size_body2"],
            bundleName: "",
            moduleName: ""
        },
        fontColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_fa_text_secondary"],
            bundleName: "",
            moduleName: ""
        }
    },
    windows: { padding: { top: 12, bottom: 12, left: 12, right: 12 } },
    closeButton: {
        size: { width: 20, height: 20 },
        margin: { top: 12, bottom: 12, left: 12, right: 12 },
        image: {
            id: -1,
            type: 2e4,
            params: ["sys.media.ohos_ic_public_cancel"],
            bundleName: "",
            moduleName: ""
        },
        fillColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_fa_icon_secondary"],
            bundleName: "",
            moduleName: ""
        },
        hoverColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_hover"],
            bundleName: "",
            moduleName: ""
        },
        backgroundColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_background_transparent"],
            bundleName: "",
            moduleName: ""
        }
    }
};
const noop = () => {
};

export function Popup(t, e = null) {
    (e || this).observeComponentCreation2(((o, i) => {
        if (i) {
            let i = () => ({
                icon: t.icon,
                title: t.title,
                message: t.message,
                showClose: t.showClose,
                onClose: t.onClose,
                buttons: t.buttons
            });
            ViewPU.create(new PopupComponent(e || this, {
                icon: t.icon,
                title: t.title,
                message: t.message,
                showClose: t.showClose,
                onClose: t.onClose,
                buttons: t.buttons
            }, void 0, o, i))
        } else (e || this).updateStateVarsOfChildByElmtId(o, {
            icon: t.icon,
            title: t.title,
            message: t.message,
            showClose: t.showClose,
            buttons: t.buttons
        })
    }), null)
}

export class PopupComponent extends ViewPU {
    constructor(t, e, o, i = -1, n = void 0) {
        super(t, o, i);
        "function" == typeof n && (this.paramsGenerator_ = n);
        this.theme = defaultTheme;
        this.__icon = new SynchedPropertyObjectOneWayPU(e.icon, this, "icon");
        this.__title = new SynchedPropertyObjectOneWayPU(e.title, this, "title");
        this.__message = new SynchedPropertyObjectOneWayPU(e.message, this, "message");
        this.__showClose = new SynchedPropertyObjectOneWayPU(e.showClose, this, "showClose");
        this.__buttons = new SynchedPropertyObjectOneWayPU(e.buttons, this, "buttons");
        this.onClose = noop;
        this.__closeButtonBackgroundColor = new ObservedPropertyObjectPU({
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_background_transparent"],
            bundleName: "",
            moduleName: ""
        }, this, "closeButtonBackgroundColor");
        this.__firstButtonBackgroundColor = new ObservedPropertyObjectPU({
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_background_transparent"],
            bundleName: "",
            moduleName: ""
        }, this, "firstButtonBackgroundColor");
        this.__secondButtonBackgroundColor = new ObservedPropertyObjectPU({
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_background_transparent"],
            bundleName: "",
            moduleName: ""
        }, this, "secondButtonBackgroundColor");
        this.__titleHeight = new ObservedPropertySimplePU(0, this, "titleHeight");
        this.__buttonHeight = new ObservedPropertySimplePU(0, this, "buttonHeight");
        this.setInitiallyProvidedValue(e)
    }

    setInitiallyProvidedValue(t) {
        void 0 !== t.theme && (this.theme = t.theme);
        void 0 === t.icon && this.__icon.set({ image: "" });
        void 0 === t.title && this.__title.set({ text: "" });
        void 0 === t.message && this.__message.set({ text: "" });
        void 0 === t.showClose && this.__showClose.set(!0);
        void 0 === t.buttons && this.__buttons.set([{ text: "" }, { text: "" }]);
        void 0 !== t.onClose && (this.onClose = t.onClose);
        void 0 !== t.closeButtonBackgroundColor && (this.closeButtonBackgroundColor = t.closeButtonBackgroundColor);
        void 0 !== t.firstButtonBackgroundColor && (this.firstButtonBackgroundColor = t.firstButtonBackgroundColor);
        void 0 !== t.secondButtonBackgroundColor && (this.secondButtonBackgroundColor = t.secondButtonBackgroundColor);
        void 0 !== t.titleHeight && (this.titleHeight = t.titleHeight);
        void 0 !== t.buttonHeight && (this.buttonHeight = t.buttonHeight)
    }

    updateStateVars(t) {
        this.__icon.reset(t.icon);
        this.__title.reset(t.title);
        this.__message.reset(t.message);
        this.__showClose.reset(t.showClose);
        this.__buttons.reset(t.buttons)
    }

    purgeVariableDependenciesOnElmtId(t) {
        this.__icon.purgeDependencyOnElmtId(t);
        this.__title.purgeDependencyOnElmtId(t);
        this.__message.purgeDependencyOnElmtId(t);
        this.__showClose.purgeDependencyOnElmtId(t);
        this.__buttons.purgeDependencyOnElmtId(t);
        this.__closeButtonBackgroundColor.purgeDependencyOnElmtId(t);
        this.__firstButtonBackgroundColor.purgeDependencyOnElmtId(t);
        this.__secondButtonBackgroundColor.purgeDependencyOnElmtId(t);
        this.__titleHeight.purgeDependencyOnElmtId(t);
        this.__buttonHeight.purgeDependencyOnElmtId(t)
    }

    aboutToBeDeleted() {
        this.__icon.aboutToBeDeleted();
        this.__title.aboutToBeDeleted();
        this.__message.aboutToBeDeleted();
        this.__showClose.aboutToBeDeleted();
        this.__buttons.aboutToBeDeleted();
        this.__closeButtonBackgroundColor.aboutToBeDeleted();
        this.__firstButtonBackgroundColor.aboutToBeDeleted();
        this.__secondButtonBackgroundColor.aboutToBeDeleted();
        this.__titleHeight.aboutToBeDeleted();
        this.__buttonHeight.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal()
    }

    get icon() {
        return this.__icon.get()
    }

    set icon(t) {
        this.__icon.set(t)
    }

    get title() {
        return this.__title.get()
    }

    set title(t) {
        this.__title.set(t)
    }

    get message() {
        return this.__message.get()
    }

    set message(t) {
        this.__message.set(t)
    }

    get showClose() {
        return this.__showClose.get()
    }

    set showClose(t) {
        this.__showClose.set(t)
    }

    get buttons() {
        return this.__buttons.get()
    }

    set buttons(t) {
        this.__buttons.set(t)
    }

    get closeButtonBackgroundColor() {
        return this.__closeButtonBackgroundColor.get()
    }

    set closeButtonBackgroundColor(t) {
        this.__closeButtonBackgroundColor.set(t)
    }

    get firstButtonBackgroundColor() {
        return this.__firstButtonBackgroundColor.get()
    }

    set firstButtonBackgroundColor(t) {
        this.__firstButtonBackgroundColor.set(t)
    }

    get secondButtonBackgroundColor() {
        return this.__secondButtonBackgroundColor.get()
    }

    set secondButtonBackgroundColor(t) {
        this.__secondButtonBackgroundColor.set(t)
    }

    get titleHeight() {
        return this.__titleHeight.get()
    }

    set titleHeight(t) {
        this.__titleHeight.set(t)
    }

    get buttonHeight() {
        return this.__buttonHeight.get()
    }

    set buttonHeight(t) {
        this.__buttonHeight.set(t)
    }

    getScrollMaxHeight() {
        let t = 480;
        t -= this.titleHeight;
        t -= this.buttonHeight;
        t -= this.theme.windows.padding.top;
        t -= this.theme.button.textMargin.bottom / 2;
        t -= this.theme.title.margin.bottom;
        t -= this.theme.windows.padding.bottom - this.theme.button.textMargin.bottom / 2;
        return t
    }

    getIconWidth() {
        var t, e;
        return null !== (e = null === (t = this.icon) || void 0 === t ? void 0 : t.width) && void 0 !== e ? e : this.theme.icon.size.width
    }

    getIconHeight() {
        var t, e;
        return null !== (e = null === (t = this.icon) || void 0 === t ? void 0 : t.height) && void 0 !== e ? e : this.theme.icon.size.height
    }

    getIconFillColor() {
        var t, e;
        return null !== (e = null === (t = this.icon) || void 0 === t ? void 0 : t.fillColor) && void 0 !== e ? e : this.theme.icon.fillColor
    }

    getIconBorderRadius() {
        var t, e;
        return null !== (e = null === (t = this.icon) || void 0 === t ? void 0 : t.borderRadius) && void 0 !== e ? e : this.theme.icon.borderRadius
    }

    getIconMargin() {
        return {
            left: this.theme.button.margin.left / 2,
            right: this.theme.icon.margin.right - this.theme.button.margin.right / 2
        }
    }

    getIconImage() {
        var t;
        return null === (t = this.icon) || void 0 === t ? void 0 : t.image
    }

    getTitleText() {
        var t;
        return null === (t = this.title) || void 0 === t ? void 0 : t.text
    }

    getTitlePadding() {
        return { left: this.theme.button.margin.left / 2, right: this.theme.closeButton.margin.right }
    }

    getTitleMargin() {
        return this.theme.title.margin
    }

    getTitleMinFontSize() {
        return this.theme.title.minFontSize
    }

    getTitleFontWeight() {
        var t, e;
        return null !== (e = null === (t = this.title) || void 0 === t ? void 0 : t.fontWeight) && void 0 !== e ? e : this.theme.title.fontWeight
    }

    getTitleFontSize() {
        var t, e;
        return null !== (e = null === (t = this.title) || void 0 === t ? void 0 : t.fontSize) && void 0 !== e ? e : this.theme.title.fontSize
    }

    getTitleFontColor() {
        var t, e;
        return null !== (e = null === (t = this.title) || void 0 === t ? void 0 : t.fontColor) && void 0 !== e ? e : this.theme.title.fontColor
    }

    getCloseButtonWidth() {
        return this.theme.closeButton.size.width
    }

    getCloseButtonHeight() {
        return this.theme.closeButton.size.height
    }

    getCloseButtonImage() {
        return this.theme.closeButton.image
    }

    getCloseButtonFillColor() {
        return this.theme.closeButton.fillColor
    }

    getCloseButtonHoverColor() {
        return this.theme.closeButton.hoverColor
    }

    getCloseButtonBackgroundColor() {
        return this.theme.closeButton.backgroundColor
    }

    getMessageText() {
        return this.message.text
    }

    getMessageFontSize() {
        var t;
        return null !== (t = this.message.fontSize) && void 0 !== t ? t : this.theme.message.fontSize
    }

    getMessageFontColor() {
        var t;
        return null !== (t = this.message.fontColor) && void 0 !== t ? t : this.theme.message.fontColor
    }

    getMessagePadding() {
        let t;
        t = this.title ? { left: this.theme.button.margin.left / 2 } : {
            left: this.theme.button.margin.left / 2,
            right: this.theme.closeButton.margin.right
        };
        return t
    }

    getButtonMargin() {
        return {
            top: this.theme.button.textMargin.top / 2,
            bottom: this.theme.button.textMargin.bottom / 2,
            left: this.theme.button.margin.left / 2,
            right: this.theme.button.margin.right / 2
        }
    }

    getButtonTextMargin() {
        return { top: this.theme.button.textMargin.bottom / 2 }
    }

    getButtonHoverColor() {
        return this.theme.button.hoverColor
    }

    getButtonBackgroundColor() {
        return this.theme.button.backgroundColor
    }

    getFirstButtonText() {
        var t, e;
        return null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[0]) || void 0 === e ? void 0 : e.text
    }

    getSecondButtonText() {
        var t, e;
        return null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[1]) || void 0 === e ? void 0 : e.text
    }

    getFirstButtonFontSize() {
        var t, e, o;
        return null !== (o = null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[0]) || void 0 === e ? void 0 : e.fontSize) && void 0 !== o ? o : this.theme.button.fontSize
    }

    getSecondButtonFontSize() {
        var t, e, o;
        return null !== (o = null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[1]) || void 0 === e ? void 0 : e.fontSize) && void 0 !== o ? o : this.theme.button.fontSize
    }

    getFirstButtonFontColor() {
        var t, e, o;
        return null !== (o = null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[0]) || void 0 === e ? void 0 : e.fontColor) && void 0 !== o ? o : this.theme.button.fontColor
    }

    getSecondButtonFontColor() {
        var t, e, o;
        return null !== (o = null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[1]) || void 0 === e ? void 0 : e.fontColor) && void 0 !== o ? o : this.theme.button.fontColor
    }

    getButtonMinFontSize() {
        return this.theme.button.minFontSize
    }

    getWindowsPadding() {
        return {
            top: this.theme.windows.padding.top,
            bottom: this.theme.windows.padding.bottom - this.theme.button.textMargin.bottom / 2,
            left: this.theme.windows.padding.left - this.theme.button.margin.right / 2,
            right: this.theme.windows.padding.right
        }
    }

    initialRender() {
        this.observeComponentCreation2(((t, e) => {
            Row.create();
            Row.padding(this.getWindowsPadding());
            Row.clip(!0);
            Row.focusable(!0);
            Row.alignItems(VerticalAlign.Top);
            Row.backgroundColor(Color.Transparent);
            Row.constraintSize({ maxHeight: "100%" })
        }), Row);
        this.observeComponentCreation2(((t, e) => {
            If.create();
            this.icon ? this.ifElseBranchUpdateFunction(0, (() => {
                this.observeComponentCreation2(((t, e) => {
                    Image.create(this.getIconImage());
                    Image.width(this.getIconWidth());
                    Image.height(this.getIconHeight());
                    Image.margin(this.getIconMargin());
                    Image.fillColor(this.getIconFillColor());
                    Image.borderRadius(this.getIconBorderRadius())
                }), Image)
            })) : this.ifElseBranchUpdateFunction(1, (() => {
            }))
        }), If);
        If.pop();
        this.observeComponentCreation2(((t, e) => {
            If.create();
            this.title ? this.ifElseBranchUpdateFunction(0, (() => {
                this.observeComponentCreation2(((t, e) => {
                    Column.create();
                    Column.layoutWeight(1)
                }), Column);
                this.observeComponentCreation2(((t, e) => {
                    Flex.create({ alignItems: ItemAlign.Start });
                    Flex.width("100%");
                    Flex.margin(this.getTitleMargin());
                    Flex.onAreaChange(((t, e) => {
                        this.titleHeight = e.height
                    }))
                }), Flex);
                this.observeComponentCreation2(((t, e) => {
                    Text.create(this.getTitleText());
                    Text.flexGrow(1);
                    Text.maxLines(2);
                    Text.align(Alignment.Start);
                    Text.padding(this.getTitlePadding());
                    Text.minFontSize(this.getTitleMinFontSize());
                    Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                    Text.fontWeight(this.getTitleFontWeight());
                    Text.fontSize(this.getTitleFontSize());
                    Text.fontColor(this.getTitleFontColor())
                }), Text);
                Text.pop();
                this.observeComponentCreation2(((t, e) => {
                    If.create();
                    this.showClose ? this.ifElseBranchUpdateFunction(0, (() => {
                        this.observeComponentCreation2(((t, e) => {
                            Button.createWithChild();
                            Button.width(this.getCloseButtonWidth());
                            Button.height(this.getCloseButtonHeight());
                            Button.backgroundColor(ObservedObject.GetRawObject(this.closeButtonBackgroundColor));
                            Button.onHover((t => {
                                this.closeButtonBackgroundColor = t ? this.getCloseButtonHoverColor() : this.getCloseButtonBackgroundColor()
                            }));
                            Button.onClick((() => {
                                this.onClose && this.onClose()
                            }))
                        }), Button);
                        this.observeComponentCreation2(((t, e) => {
                            Image.create(this.getCloseButtonImage());
                            Image.focusable(!0);
                            Image.fillColor(this.getCloseButtonFillColor())
                        }), Image);
                        Button.pop()
                    })) : this.ifElseBranchUpdateFunction(1, (() => {
                    }))
                }), If);
                If.pop();
                Flex.pop();
                this.observeComponentCreation2(((t, e) => {
                    Scroll.create();
                    Scroll.width("100%");
                    Scroll.align(Alignment.TopStart);
                    Scroll.padding(this.getMessagePadding());
                    Scroll.scrollBar(BarState.Auto);
                    Scroll.scrollable(ScrollDirection.Vertical);
                    Scroll.constraintSize({ maxHeight: this.getScrollMaxHeight() })
                }), Scroll);
                this.observeComponentCreation2(((t, e) => {
                    Text.create(this.getMessageText());
                    Text.fontSize(this.getMessageFontSize());
                    Text.fontColor(this.getMessageFontColor())
                }), Text);
                Text.pop();
                Scroll.pop();
                this.observeComponentCreation2(((t, e) => {
                    Flex.create({ wrap: FlexWrap.Wrap });
                    Flex.margin(this.getButtonTextMargin());
                    Flex.flexGrow(1);
                    Flex.onAreaChange(((t, e) => {
                        this.buttonHeight = e.height
                    }))
                }), Flex);
                this.observeComponentCreation2(((t, e) => {
                    var o;
                    If.create();
                    (null === (o = this.buttons) || void 0 === o ? void 0 : o[0]) ? this.ifElseBranchUpdateFunction(0, (() => {
                        this.observeComponentCreation2(((t, e) => {
                            Button.createWithChild();
                            Button.margin(this.getButtonMargin());
                            Button.backgroundColor(ObservedObject.GetRawObject(this.firstButtonBackgroundColor));
                            Button.onHover((t => {
                                this.firstButtonBackgroundColor = t ? this.getButtonHoverColor() : this.getButtonBackgroundColor()
                            }));
                            Button.onClick((() => {
                                var t, e, o, i;
                                (null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[0]) || void 0 === e ? void 0 : e.action) && (null === (i = null === (o = this.buttons) || void 0 === o ? void 0 : o[0]) || void 0 === i || i.action())
                            }))
                        }), Button);
                        this.observeComponentCreation2(((t, e) => {
                            Text.create(this.getFirstButtonText());
                            Text.maxLines(2);
                            Text.focusable(!0);
                            Text.fontSize(this.getFirstButtonFontSize());
                            Text.fontColor(this.getFirstButtonFontColor());
                            Text.minFontSize(this.getButtonMinFontSize());
                            Text.textOverflow({ overflow: TextOverflow.Ellipsis })
                        }), Text);
                        Text.pop();
                        Button.pop()
                    })) : this.ifElseBranchUpdateFunction(1, (() => {
                    }))
                }), If);
                If.pop();
                this.observeComponentCreation2(((t, e) => {
                    var o;
                    If.create();
                    (null === (o = this.buttons) || void 0 === o ? void 0 : o[1]) ? this.ifElseBranchUpdateFunction(0, (() => {
                        this.observeComponentCreation2(((t, e) => {
                            Button.createWithChild();
                            Button.margin(this.getButtonMargin());
                            Button.backgroundColor(ObservedObject.GetRawObject(this.secondButtonBackgroundColor));
                            Button.onHover((t => {
                                this.secondButtonBackgroundColor = t ? this.getButtonHoverColor() : this.getButtonBackgroundColor()
                            }));
                            Button.onClick((() => {
                                var t, e, o, i;
                                (null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[1]) || void 0 === e ? void 0 : e.action) && (null === (i = null === (o = this.buttons) || void 0 === o ? void 0 : o[1]) || void 0 === i || i.action())
                            }))
                        }), Button);
                        this.observeComponentCreation2(((t, e) => {
                            Text.create(this.getSecondButtonText());
                            Text.maxLines(2);
                            Text.focusable(!0);
                            Text.fontSize(this.getSecondButtonFontSize());
                            Text.fontColor(this.getSecondButtonFontColor());
                            Text.minFontSize(this.getButtonMinFontSize());
                            Text.textOverflow({ overflow: TextOverflow.Ellipsis })
                        }), Text);
                        Text.pop();
                        Button.pop()
                    })) : this.ifElseBranchUpdateFunction(1, (() => {
                    }))
                }), If);
                If.pop();
                Flex.pop();
                Column.pop()
            })) : this.ifElseBranchUpdateFunction(1, (() => {
                this.observeComponentCreation2(((t, e) => {
                    Column.create();
                    Column.layoutWeight(1)
                }), Column);
                this.observeComponentCreation2(((t, e) => {
                    Row.create();
                    Row.alignItems(VerticalAlign.Top);
                    Row.margin(this.getTitleMargin())
                }), Row);
                this.observeComponentCreation2(((t, e) => {
                    Scroll.create();
                    Scroll.layoutWeight(1);
                    Scroll.align(Alignment.TopStart);
                    Scroll.padding(this.getMessagePadding());
                    Scroll.scrollBar(BarState.Auto);
                    Scroll.scrollable(ScrollDirection.Vertical);
                    Scroll.constraintSize({ maxHeight: this.getScrollMaxHeight() })
                }), Scroll);
                this.observeComponentCreation2(((t, e) => {
                    Text.create(this.getMessageText());
                    Text.fontSize(this.getMessageFontSize());
                    Text.fontColor(this.getMessageFontColor())
                }), Text);
                Text.pop();
                Scroll.pop();
                this.observeComponentCreation2(((t, e) => {
                    If.create();
                    this.showClose ? this.ifElseBranchUpdateFunction(0, (() => {
                        this.observeComponentCreation2(((t, e) => {
                            Button.createWithChild();
                            Button.width(this.getCloseButtonWidth());
                            Button.height(this.getCloseButtonHeight());
                            Button.backgroundColor(ObservedObject.GetRawObject(this.closeButtonBackgroundColor));
                            Button.onHover((t => {
                                this.closeButtonBackgroundColor = t ? this.getCloseButtonHoverColor() : this.getCloseButtonBackgroundColor()
                            }));
                            Button.onClick((() => {
                                this.onClose && this.onClose()
                            }))
                        }), Button);
                        this.observeComponentCreation2(((t, e) => {
                            Image.create(this.getCloseButtonImage());
                            Image.focusable(!0);
                            Image.fillColor(this.getCloseButtonFillColor())
                        }), Image);
                        Button.pop()
                    })) : this.ifElseBranchUpdateFunction(1, (() => {
                    }))
                }), If);
                If.pop();
                Row.pop();
                this.observeComponentCreation2(((t, e) => {
                    Flex.create({ wrap: FlexWrap.Wrap });
                    Flex.margin(this.getButtonTextMargin());
                    Flex.flexGrow(1);
                    Flex.onAreaChange(((t, e) => {
                        this.buttonHeight = e.height
                    }))
                }), Flex);
                this.observeComponentCreation2(((t, e) => {
                    var o;
                    If.create();
                    (null === (o = this.buttons) || void 0 === o ? void 0 : o[0]) ? this.ifElseBranchUpdateFunction(0, (() => {
                        this.observeComponentCreation2(((t, e) => {
                            Button.createWithChild();
                            Button.margin(this.getButtonMargin());
                            Button.backgroundColor(ObservedObject.GetRawObject(this.firstButtonBackgroundColor));
                            Button.onHover((t => {
                                this.firstButtonBackgroundColor = t ? this.getButtonHoverColor() : this.getButtonBackgroundColor()
                            }));
                            Button.onClick((() => {
                                var t, e, o, i;
                                (null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[0]) || void 0 === e ? void 0 : e.action) && (null === (i = null === (o = this.buttons) || void 0 === o ? void 0 : o[0]) || void 0 === i || i.action())
                            }))
                        }), Button);
                        this.observeComponentCreation2(((t, e) => {
                            Text.create(this.getFirstButtonText());
                            Text.maxLines(2);
                            Text.focusable(!0);
                            Text.fontSize(this.getFirstButtonFontSize());
                            Text.fontColor(this.getFirstButtonFontColor());
                            Text.minFontSize(this.getButtonMinFontSize());
                            Text.textOverflow({ overflow: TextOverflow.Ellipsis })
                        }), Text);
                        Text.pop();
                        Button.pop()
                    })) : this.ifElseBranchUpdateFunction(1, (() => {
                    }))
                }), If);
                If.pop();
                this.observeComponentCreation2(((t, e) => {
                    var o;
                    If.create();
                    (null === (o = this.buttons) || void 0 === o ? void 0 : o[1]) ? this.ifElseBranchUpdateFunction(0, (() => {
                        this.observeComponentCreation2(((t, e) => {
                            Button.createWithChild();
                            Button.margin(this.getButtonMargin());
                            Button.backgroundColor(ObservedObject.GetRawObject(this.secondButtonBackgroundColor));
                            Button.onHover((t => {
                                this.secondButtonBackgroundColor = t ? this.getButtonHoverColor() : this.getButtonBackgroundColor()
                            }));
                            Button.onClick((() => {
                                var t, e, o, i;
                                (null === (e = null === (t = this.buttons) || void 0 === t ? void 0 : t[1]) || void 0 === e ? void 0 : e.action) && (null === (i = null === (o = this.buttons) || void 0 === o ? void 0 : o[1]) || void 0 === i || i.action())
                            }))
                        }), Button);
                        this.observeComponentCreation2(((t, e) => {
                            Text.create(this.getSecondButtonText());
                            Text.maxLines(2);
                            Text.focusable(!0);
                            Text.fontSize(this.getSecondButtonFontSize());
                            Text.fontColor(this.getSecondButtonFontColor());
                            Text.minFontSize(this.getButtonMinFontSize());
                            Text.textOverflow({ overflow: TextOverflow.Ellipsis })
                        }), Text);
                        Text.pop();
                        Button.pop()
                    })) : this.ifElseBranchUpdateFunction(1, (() => {
                    }))
                }), If);
                If.pop();
                Flex.pop();
                Column.pop()
            }))
        }), If);
        If.pop();
        Row.pop()
    }

    rerender() {
        this.updateDirtyElements()
    }
}
export default { Popup };