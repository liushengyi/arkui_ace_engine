const KeyCode = requireNapi("multimodalInput.keyCode").KeyCode;
const measure = requireNapi('measure');
const mediaquery = requireNapi('mediaquery');
const resourceManager = requireNapi('resourceManager');

export var ChipSize;
!function(e){
    e.NORMAL = "NORMAL";
    e.SMALL = "SMALL"
}(ChipSize || (ChipSize = {}));
var BreakPointsType;
!function(e){
    e.SM = "SM";
    e.MD = "MD";
    e.LG = "LG"
}(BreakPointsType || (BreakPointsType = {}));

export const defaultTheme = {
    prefixIcon: {
        size: { width: 16, height: 16 },
        fillColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_secondary"],
            bundleName: "",
            moduleName: ""
        }
    },
    label: {
        fontSize: {
            id: -1,
            type: 10002,
            params: ["sys.float.ohos_id_text_size_button3"],
            bundleName: "",
            moduleName: ""
        },
        fontColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_text_primary"],
            bundleName: "",
            moduleName: ""
        },
        fontFamily: "HarmonyOS Sans",
        normalMargin: { left: 6, right: 6, top: 0, bottom: 0 },
        smallMargin: { left: 4, right: 4, top: 0, bottom: 0 }
    },
    suffixIcon: {
        size: { width: 16, height: 16 },
        fillColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_primary"],
            bundleName: "",
            moduleName: ""
        },
        defaultDeleteIcon: {
            id: -1,
            type: 2e4,
            params: ["sys.media.ohos_ic_public_cancel", 16, 16],
            bundleName: "",
            moduleName: ""
        },
        focusable: !1
    },
    chipNode: {
        normalHeight: 36,
        smallHeight: 28,
        enabled: !0,
        backgroundColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_button_normal"],
            bundleName: "",
            moduleName: ""
        },
        focusOutlineColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_focused_outline"],
            bundleName: "",
            moduleName: ""
        },
        normalBorderRadius: {
            id: -1,
            type: 10002,
            params: ["sys.float.ohos_id_corner_radius_button"],
            bundleName: "",
            moduleName: ""
        },
        smallBorderRadius: {
            id: -1,
            type: 10002,
            params: ["sys.float.ohos_id_corner_radius_small_button"],
            bundleName: "",
            moduleName: ""
        },
        borderWidth: 2,
        normalPadding: { left: 16, right: 16, top: 0, bottom: 0 },
        smallPadding: { left: 12, right: 12, top: 0, bottom: 0 },
        hoverBlendColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_hover"],
            bundleName: "",
            moduleName: ""
        },
        pressedBlendColor: {
            id: -1,
            type: 10001,
            params: ["sys.color.ohos_id_color_click_effect"],
            bundleName: "",
            moduleName: ""
        },
        opacity: { normal: 1, hover: .95, pressed: .9, disabled: .4 },
        breakPointConstraintWidth: {
            breakPointMinWidth: 128,
            breakPointSmMaxWidth: 156,
            breakPointMdMaxWidth: 280,
            breakPointLgMaxWidth: 400
        }
    }
};
const noop = () => {
};

export function Chip(e, i = null) {
    (i || this).observeComponentCreation2(((t, o) => {
        if (o) {
            let o = () => ({
                chipSize: e.size,
                prefixIcon: e.prefixIcon,
                label: e.label,
                suffixIcon: e.suffixIcon,
                allowClose: e.allowClose,
                chipEnabled: e.enabled,
                chipNodeBackgroundColor: e.backgroundColor,
                chipNodeRadius: e.borderRadius,
                onClose: e.onClose
            });
            ViewPU.create(new ChipComponent(i || this, {
                chipSize: e.size,
                prefixIcon: e.prefixIcon,
                label: e.label,
                suffixIcon: e.suffixIcon,
                allowClose: e.allowClose,
                chipEnabled: e.enabled,
                chipNodeBackgroundColor: e.backgroundColor,
                chipNodeRadius: e.borderRadius,
                onClose: e.onClose
            }, void 0, t, o))
        } else (i || this).updateStateVarsOfChildByElmtId(t, {
            chipSize: e.size,
            prefixIcon: e.prefixIcon,
            label: e.label,
            suffixIcon: e.suffixIcon,
            allowClose: e.allowClose,
            chipEnabled: e.enabled,
            chipNodeBackgroundColor: e.backgroundColor,
            chipNodeRadius: e.borderRadius
        })
    }), null)
}

export class ChipComponent extends ViewPU {
    constructor(e, i, t, o = -1, h = void 0) {
        super(e, t, o);
        "function" == typeof h && (this.paramsGenerator_ = h);
        this.theme = defaultTheme;
        this.__chipSize = new SynchedPropertyObjectOneWayPU(i.chipSize, this, "chipSize");
        this.__allowClose = new SynchedPropertySimpleOneWayPU(i.allowClose, this, "allowClose");
        this.__prefixIcon = new SynchedPropertyObjectOneWayPU(i.prefixIcon, this, "prefixIcon");
        this.__label = new SynchedPropertyObjectOneWayPU(i.label, this, "label");
        this.__suffixIcon = new SynchedPropertyObjectOneWayPU(i.suffixIcon, this, "suffixIcon");
        this.__chipNodeBackgroundColor = new SynchedPropertyObjectOneWayPU(i.chipNodeBackgroundColor, this, "chipNodeBackgroundColor");
        this.__chipNodeRadius = new SynchedPropertyObjectOneWayPU(i.chipNodeRadius, this, "chipNodeRadius");
        this.__chipEnabled = new SynchedPropertySimpleOneWayPU(i.chipEnabled, this, "chipEnabled");
        this.__isHover = new ObservedPropertySimplePU(!1, this, "isHover");
        this.__isPressed = new ObservedPropertySimplePU(!1, this, "isPressed");
        this.__chipScale = new ObservedPropertyObjectPU({ x: 1, y: 1 }, this, "chipScale");
        this.__chipOpacity = new ObservedPropertySimplePU(1, this, "chipOpacity");
        this.__chipBlendColor = new ObservedPropertyObjectPU(Color.Transparent, this, "chipBlendColor");
        this.__deleteChip = new ObservedPropertySimplePU(!1, this, "deleteChip");
        this.__chipNodeOnFocus = new ObservedPropertySimplePU(!1, this, "chipNodeOnFocus");
        this.__useDefaultSuffixIcon = new ObservedPropertySimplePU(!1, this, "useDefaultSuffixIcon");
        this.chipNodeSize = {};
        this.onClose = noop;
        this.__suffixIconOnFocus = new ObservedPropertySimplePU(!1, this, "suffixIconOnFocus");
        this.__chipBreakPoints = new ObservedPropertySimplePU(BreakPointsType.SM, this, "chipBreakPoints");
        this.smListener = mediaquery.matchMediaSync("0vp<width<600vp");
        this.mdListener = mediaquery.matchMediaSync("600vp<=width<840vp");
        this.lgListener = mediaquery.matchMediaSync("840vp<=width");
        this.setInitiallyProvidedValue(i)
    }

    setInitiallyProvidedValue(e) {
        void 0 !== e.theme && (this.theme = e.theme);
        void 0 === e.chipSize && this.__chipSize.set(ChipSize.NORMAL);
        void 0 === e.allowClose && this.__allowClose.set(!0);
        void 0 === e.prefixIcon && this.__prefixIcon.set({ src: "" });
        void 0 === e.label && this.__label.set({ text: "" });
        void 0 === e.suffixIcon && this.__suffixIcon.set({ src: "" });
        void 0 === e.chipNodeBackgroundColor && this.__chipNodeBackgroundColor.set(this.theme.chipNode.backgroundColor);
        void 0 === e.chipNodeRadius && this.__chipNodeRadius.set(void 0);
        void 0 === e.chipEnabled && this.__chipEnabled.set(!0);
        void 0 !== e.isHover && (this.isHover = e.isHover);
        void 0 !== e.isPressed && (this.isPressed = e.isPressed);
        void 0 !== e.chipScale && (this.chipScale = e.chipScale);
        void 0 !== e.chipOpacity && (this.chipOpacity = e.chipOpacity);
        void 0 !== e.chipBlendColor && (this.chipBlendColor = e.chipBlendColor);
        void 0 !== e.deleteChip && (this.deleteChip = e.deleteChip);
        void 0 !== e.chipNodeOnFocus && (this.chipNodeOnFocus = e.chipNodeOnFocus);
        void 0 !== e.useDefaultSuffixIcon && (this.useDefaultSuffixIcon = e.useDefaultSuffixIcon);
        void 0 !== e.chipNodeSize && (this.chipNodeSize = e.chipNodeSize);
        void 0 !== e.onClose && (this.onClose = e.onClose);
        void 0 !== e.suffixIconOnFocus && (this.suffixIconOnFocus = e.suffixIconOnFocus);
        void 0 !== e.chipBreakPoints && (this.chipBreakPoints = e.chipBreakPoints);
        void 0 !== e.smListener && (this.smListener = e.smListener);
        void 0 !== e.mdListener && (this.mdListener = e.mdListener);
        void 0 !== e.lgListener && (this.lgListener = e.lgListener)
    }

    updateStateVars(e) {
        this.__chipSize.reset(e.chipSize);
        this.__allowClose.reset(e.allowClose);
        this.__prefixIcon.reset(e.prefixIcon);
        this.__label.reset(e.label);
        this.__suffixIcon.reset(e.suffixIcon);
        this.__chipNodeBackgroundColor.reset(e.chipNodeBackgroundColor);
        this.__chipNodeRadius.reset(e.chipNodeRadius);
        this.__chipEnabled.reset(e.chipEnabled)
    }

    purgeVariableDependenciesOnElmtId(e) {
        this.__chipSize.purgeDependencyOnElmtId(e);
        this.__allowClose.purgeDependencyOnElmtId(e);
        this.__prefixIcon.purgeDependencyOnElmtId(e);
        this.__label.purgeDependencyOnElmtId(e);
        this.__suffixIcon.purgeDependencyOnElmtId(e);
        this.__chipNodeBackgroundColor.purgeDependencyOnElmtId(e);
        this.__chipNodeRadius.purgeDependencyOnElmtId(e);
        this.__chipEnabled.purgeDependencyOnElmtId(e);
        this.__isHover.purgeDependencyOnElmtId(e);
        this.__isPressed.purgeDependencyOnElmtId(e);
        this.__chipScale.purgeDependencyOnElmtId(e);
        this.__chipOpacity.purgeDependencyOnElmtId(e);
        this.__chipBlendColor.purgeDependencyOnElmtId(e);
        this.__deleteChip.purgeDependencyOnElmtId(e);
        this.__chipNodeOnFocus.purgeDependencyOnElmtId(e);
        this.__useDefaultSuffixIcon.purgeDependencyOnElmtId(e);
        this.__suffixIconOnFocus.purgeDependencyOnElmtId(e);
        this.__chipBreakPoints.purgeDependencyOnElmtId(e)
    }

    aboutToBeDeleted() {
        this.__chipSize.aboutToBeDeleted();
        this.__allowClose.aboutToBeDeleted();
        this.__prefixIcon.aboutToBeDeleted();
        this.__label.aboutToBeDeleted();
        this.__suffixIcon.aboutToBeDeleted();
        this.__chipNodeBackgroundColor.aboutToBeDeleted();
        this.__chipNodeRadius.aboutToBeDeleted();
        this.__chipEnabled.aboutToBeDeleted();
        this.__isHover.aboutToBeDeleted();
        this.__isPressed.aboutToBeDeleted();
        this.__chipScale.aboutToBeDeleted();
        this.__chipOpacity.aboutToBeDeleted();
        this.__chipBlendColor.aboutToBeDeleted();
        this.__deleteChip.aboutToBeDeleted();
        this.__chipNodeOnFocus.aboutToBeDeleted();
        this.__useDefaultSuffixIcon.aboutToBeDeleted();
        this.__suffixIconOnFocus.aboutToBeDeleted();
        this.__chipBreakPoints.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal()
    }

    get chipSize() {
        return this.__chipSize.get()
    }

    set chipSize(e) {
        this.__chipSize.set(e)
    }

    get allowClose() {
        return this.__allowClose.get()
    }

    set allowClose(e) {
        this.__allowClose.set(e)
    }

    get prefixIcon() {
        return this.__prefixIcon.get()
    }

    set prefixIcon(e) {
        this.__prefixIcon.set(e)
    }

    get label() {
        return this.__label.get()
    }

    set label(e) {
        this.__label.set(e)
    }

    get suffixIcon() {
        return this.__suffixIcon.get()
    }

    set suffixIcon(e) {
        this.__suffixIcon.set(e)
    }

    get chipNodeBackgroundColor() {
        return this.__chipNodeBackgroundColor.get()
    }

    set chipNodeBackgroundColor(e) {
        this.__chipNodeBackgroundColor.set(e)
    }

    get chipNodeRadius() {
        return this.__chipNodeRadius.get()
    }

    set chipNodeRadius(e) {
        this.__chipNodeRadius.set(e)
    }

    get chipEnabled() {
        return this.__chipEnabled.get()
    }

    set chipEnabled(e) {
        this.__chipEnabled.set(e)
    }

    get isHover() {
        return this.__isHover.get()
    }

    set isHover(e) {
        this.__isHover.set(e)
    }

    get isPressed() {
        return this.__isPressed.get()
    }

    set isPressed(e) {
        this.__isPressed.set(e)
    }

    get chipScale() {
        return this.__chipScale.get()
    }

    set chipScale(e) {
        this.__chipScale.set(e)
    }

    get chipOpacity() {
        return this.__chipOpacity.get()
    }

    set chipOpacity(e) {
        this.__chipOpacity.set(e)
    }

    get chipBlendColor() {
        return this.__chipBlendColor.get()
    }

    set chipBlendColor(e) {
        this.__chipBlendColor.set(e)
    }

    get deleteChip() {
        return this.__deleteChip.get()
    }

    set deleteChip(e) {
        this.__deleteChip.set(e)
    }

    get chipNodeOnFocus() {
        return this.__chipNodeOnFocus.get()
    }

    set chipNodeOnFocus(e) {
        this.__chipNodeOnFocus.set(e)
    }

    get useDefaultSuffixIcon() {
        return this.__useDefaultSuffixIcon.get()
    }

    set useDefaultSuffixIcon(e) {
        this.__useDefaultSuffixIcon.set(e)
    }

    get suffixIconOnFocus() {
        return this.__suffixIconOnFocus.get()
    }

    set suffixIconOnFocus(e) {
        this.__suffixIconOnFocus.set(e)
    }

    get chipBreakPoints() {
        return this.__chipBreakPoints.get()
    }

    set chipBreakPoints(e) {
        this.__chipBreakPoints.set(e)
    }

    isChipSizeEnum() {
        return "string" == typeof this.chipSize
    }

    getLabelFontSize() {
        var e;
        try {
            return void 0 !== (null === (e = this.label) || void 0 === e ? void 0 : e.fontSize) && this.label.fontSize >= 0 ? this.label.fontSize : resourceManager.getSystemResourceManager().getNumberByName(this.theme.label.fontSize.params[0].split(".")[2])
        } catch (e) {
            return 0
        }
    }

    getLabelFontColor() {
        var e, i;
        return null !== (i = null === (e = this.label) || void 0 === e ? void 0 : e.fontColor) && void 0 !== i ? i : this.theme.label.fontColor
    }

    getLabelFontFamily() {
        var e, i;
        return null !== (i = null === (e = this.label) || void 0 === e ? void 0 : e.fontFamily) && void 0 !== i ? i : this.theme.label.fontFamily
    }

    toVp(e) {
        var i, t;
        if (void 0 === e) return 0;
        switch (typeof e) {
            case "number":
                return e;
            case "object":
                try {
                    return resourceManager.getSystemResourceManager().getNumber(e.id)
                } catch (e) {
                    return 0
                }
            case "string":
                let o = new RegExp("(-?\\d+(?:\\.\\d+)?)_?(fp|vp|px|lpx|%)?$", "i");
                let h = e.match(o);
                if (!h) return 0;
                let s = Number(null !== (i = null == h ? void 0 : h[1]) && void 0 !== i ? i : 0);
                switch ((null !== (t = null == h ? void 0 : h[2]) && void 0 !== t ? t : "vp").toLowerCase()) {
                    case "px":
                        s = px2vp(s);
                        break;
                    case "fp":
                        s = px2vp(fp2px(s));
                        break;
                    case "lpx":
                        s = px2vp(lpx2px(s));
                        break;
                    case "%":
                        s = Number.NEGATIVE_INFINITY
                }
                return s;
            default:
                return 0
        }
    }

    getLabelMargin() {
        var e, i, t, o, h, s, n, l;
        let r = { left: 0, right: 0 };
        void 0 !== (null === (i = null === (e = this.label) || void 0 === e ? void 0 : e.labelMargin) || void 0 === i ? void 0 : i.left) && this.toVp(this.label.labelMargin.left) >= 0 ? r.left = null === (o = null === (t = this.label) || void 0 === t ? void 0 : t.labelMargin) || void 0 === o ? void 0 : o.left : this.isChipSizeEnum() && this.chipSize == ChipSize.SMALL ? r.left = this.theme.label.smallMargin.left : r.left = this.theme.label.normalMargin.left;
        void 0 !== (null === (s = null === (h = this.label) || void 0 === h ? void 0 : h.labelMargin) || void 0 === s ? void 0 : s.right) && this.toVp(this.label.labelMargin.right) >= 0 ? r.right = null === (l = null === (n = this.label) || void 0 === n ? void 0 : n.labelMargin) || void 0 === l ? void 0 : l.right : this.isChipSizeEnum() && this.chipSize == ChipSize.SMALL ? r.right = this.theme.label.smallMargin.right : r.right = this.theme.label.normalMargin.right;
        return r
    }

    getSuffixIconSize() {
        var e, i, t, o, h, s, n, l, r, d, a, c;
        let p = { width: 0, height: 0 };
        void 0 !== (null === (i = null === (e = this.suffixIcon) || void 0 === e ? void 0 : e.size) || void 0 === i ? void 0 : i.width) && this.toVp(null === (o = null === (t = this.suffixIcon) || void 0 === t ? void 0 : t.size) || void 0 === o ? void 0 : o.width) >= 0 ? p.width = null === (s = null === (h = this.suffixIcon) || void 0 === h ? void 0 : h.size) || void 0 === s ? void 0 : s.width : this.getSuffixIconSrc() ? p.width = this.theme.suffixIcon.size.width : p.width = 0;
        void 0 !== (null === (l = null === (n = this.suffixIcon) || void 0 === n ? void 0 : n.size) || void 0 === l ? void 0 : l.height) && this.toVp(null === (d = null === (r = this.suffixIcon) || void 0 === r ? void 0 : r.size) || void 0 === d ? void 0 : d.height) >= 0 ? p.height = null === (c = null === (a = this.suffixIcon) || void 0 === a ? void 0 : a.size) || void 0 === c ? void 0 : c.height : this.getSuffixIconSrc() ? p.height = this.theme.suffixIcon.size.height : p.height = 0;
        return p
    }

    getPrefixIconSize() {
        var e, i, t, o, h, s, n, l, r, d, a, c, p, u;
        let f = { width: 0, height: 0 };
        void 0 !== (null === (i = null === (e = this.prefixIcon) || void 0 === e ? void 0 : e.size) || void 0 === i ? void 0 : i.width) && this.toVp(null === (o = null === (t = this.prefixIcon) || void 0 === t ? void 0 : t.size) || void 0 === o ? void 0 : o.width) >= 0 ? f.width = null === (s = null === (h = this.prefixIcon) || void 0 === h ? void 0 : h.size) || void 0 === s ? void 0 : s.width : (null === (n = this.prefixIcon) || void 0 === n ? void 0 : n.src) ? f.width = this.theme.prefixIcon.size.width : f.width = 0;
        void 0 !== (null === (r = null === (l = this.prefixIcon) || void 0 === l ? void 0 : l.size) || void 0 === r ? void 0 : r.height) && this.toVp(null === (a = null === (d = this.prefixIcon) || void 0 === d ? void 0 : d.size) || void 0 === a ? void 0 : a.height) >= 0 ? f.height = null === (p = null === (c = this.prefixIcon) || void 0 === c ? void 0 : c.size) || void 0 === p ? void 0 : p.height : (null === (u = this.prefixIcon) || void 0 === u ? void 0 : u.src) ? f.height = this.theme.prefixIcon.size.height : f.height = 0;
        return f
    }

    getPrefixIconFilledColor() {
        var e, i;
        return null !== (i = null === (e = this.prefixIcon) || void 0 === e ? void 0 : e.fillColor) && void 0 !== i ? i : this.theme.prefixIcon.fillColor
    }

    getSuffixIconFilledColor() {
        var e, i;
        return null !== (i = null === (e = this.suffixIcon) || void 0 === e ? void 0 : e.fillColor) && void 0 !== i ? i : this.theme.suffixIcon.fillColor
    }

    getSuffixIconFocusable() {
        var e;
        return this.useDefaultSuffixIcon && this.allowClose || void 0 !== (null === (e = this.suffixIcon) || void 0 === e ? void 0 : e.action)
    }

    getChipNodePadding() {
        return this.isChipSizeEnum() && this.chipSize === ChipSize.SMALL ? this.theme.chipNode.smallPadding : this.theme.chipNode.normalPadding
    }

    getChipNodeRadius() {
        return void 0 !== this.chipNodeRadius && this.toVp(this.chipNodeRadius) >= 0 ? this.chipNodeRadius : this.isChipSizeEnum() && this.chipSize === ChipSize.SMALL ? this.theme.chipNode.smallBorderRadius : this.theme.chipNode.normalBorderRadius
    }

    getChipNodeBackGroundColor() {
        var e;
        return null !== (e = this.chipNodeBackgroundColor) && void 0 !== e ? e : this.theme.chipNode.backgroundColor
    }

    getChipNodeHeight() {
        var e, i, t;
        if (this.isChipSizeEnum()) return this.chipSize === ChipSize.SMALL ? this.theme.chipNode.smallHeight : this.theme.chipNode.normalHeight;
        this.chipNodeSize = this.chipSize;
        return void 0 !== (null === (e = this.chipNodeSize) || void 0 === e ? void 0 : e.height) && this.toVp(null === (i = this.chipNodeSize) || void 0 === i ? void 0 : i.height) >= 0 ? this.toVp(null === (t = this.chipNodeSize) || void 0 === t ? void 0 : t.height) : this.theme.chipNode.normalHeight
    }

    getLabelWidth() {
        var e, i, t, o;
        return px2vp(measure.measureText({
            textContent: null !== (i = null === (e = this.label) || void 0 === e ? void 0 : e.text) && void 0 !== i ? i : "",
            fontSize: this.getLabelFontSize(),
            fontFamily: null !== (o = null === (t = this.label) || void 0 === t ? void 0 : t.fontFamily) && void 0 !== o ? o : this.theme.label.fontFamily,
            maxLines: 1,
            overflow: TextOverflow.Ellipsis,
            textAlign: TextAlign.Center
        }))
    }

    getCalculateChipNodeWidth() {
        let e = 0;
        e += this.getChipNodePadding().left;
        e += this.toVp(this.getPrefixIconSize().width);
        e += this.toVp(this.getLabelMargin().left);
        e += this.getLabelWidth();
        e += this.toVp(this.getLabelMargin().right);
        e += this.toVp(this.getSuffixIconSize().width);
        e += this.getChipNodePadding().right;
        return e
    }

    getChipEnable() {
        return this.chipEnabled || void 0 === this.chipEnabled
    }

    getChipNodeOpacity() {
        return this.getChipEnable() ? this.chipOpacity : this.theme.chipNode.opacity.disabled
    }

    handleTouch(e) {
        if (this.getChipEnable()) {
            this.isPressed = e.type === TouchType.Down;
            this.isHover ? e.type === TouchType.Down ? Context.animateTo({ curve: Curve.Sharp, duration: 100 }, (() => {
                this.chipBlendColor = this.theme.chipNode.pressedBlendColor;
                this.chipOpacity = this.theme.chipNode.opacity.pressed
            })) : e.type === TouchType.Up && Context.animateTo({ curve: Curve.Sharp, duration: 100 }, (() => {
                this.chipBlendColor = this.theme.chipNode.hoverBlendColor;
                this.chipOpacity = this.theme.chipNode.opacity.hover
            })) : e.type === TouchType.Down ? Context.animateTo({ curve: Curve.Friction, duration: 350 }, (() => {
                this.chipBlendColor = this.theme.chipNode.pressedBlendColor;
                this.chipOpacity = this.theme.chipNode.opacity.pressed
            })) : e.type === TouchType.Up && Context.animateTo({ curve: Curve.Friction, duration: 350 }, (() => {
                this.chipBlendColor = Color.Transparent;
                this.chipOpacity = this.theme.chipNode.opacity.normal
            }))
        }
    }

    hoverAnimate(e) {
        if (this.getChipEnable()) {
            this.isHover = e;
            Context.animateTo({ curve: Curve.Friction, duration: 250 }, (() => {
                if (e) {
                    this.chipBlendColor = this.theme.chipNode.hoverBlendColor;
                    this.chipOpacity = this.theme.chipNode.opacity.hover
                } else {
                    this.chipBlendColor = Color.Transparent;
                    this.chipOpacity = this.theme.chipNode.opacity.normal
                }
            }))
        }
    }

    deleteChipNodeAnimate() {
        Context.animateTo({ duration: 150, curve: Curve.Sharp }, (() => {
            this.chipOpacity = 0
        }));
        Context.animateTo({ duration: 150, curve: Curve.FastOutLinearIn, onFinish: () => {
            this.deleteChip = !0
        } }, (() => {
            this.chipScale = { x: .85, y: .85 }
        }))
    }

    getSuffixIconSrc() {
        var e, i, t;
        this.useDefaultSuffixIcon = !(null === (e = this.suffixIcon) || void 0 === e ? void 0 : e.src) && this.allowClose;
        return this.useDefaultSuffixIcon ? this.theme.suffixIcon.defaultDeleteIcon : null !== (t = null === (i = this.suffixIcon) || void 0 === i ? void 0 : i.src) && void 0 !== t ? t : void 0
    }

    getChipNodeWidth() {
        var e;
        if (!this.isChipSizeEnum()) {
            this.chipNodeSize = this.chipSize;
            if (void 0 !== (null === (e = this.chipNodeSize) || void 0 === e ? void 0 : e.width) && this.toVp(this.chipNodeSize.width) >= 0) return this.toVp(this.chipNodeSize.width)
        }
        let i = this.getChipConstraintWidth();
        return Math.min(Math.max(this.getCalculateChipNodeWidth(), i.minWidth), i.maxWidth)
    }

    getFocusOverlaySize() {
        return { width: this.getChipNodeWidth() + 8, height: this.getChipNodeHeight() + 8 }
    }

    getChipConstraintWidth() {
        var e, i;
        if (!this.isChipSizeEnum()) {
            this.chipNodeSize = this.chipSize;
            if (void 0 !== (null === (e = this.chipNodeSize) || void 0 === e ? void 0 : e.width) && this.toVp(null === (i = this.chipNodeSize) || void 0 === i ? void 0 : i.width) >= 0) return {
                minWidth: 0,
                maxWidth: this.chipNodeSize.width
            }
        }
        let t = this.getCalculateChipNodeWidth();
        switch (this.chipBreakPoints) {
            case BreakPointsType.SM:
                return {
                    minWidth: 0,
                    maxWidth: Math.min(t, this.theme.chipNode.breakPointConstraintWidth.breakPointSmMaxWidth)
                };
            case BreakPointsType.MD:
                return {
                    minWidth: Math.max(t, this.theme.chipNode.breakPointConstraintWidth.breakPointMinWidth),
                    maxWidth: Math.min(t, this.theme.chipNode.breakPointConstraintWidth.breakPointMdMaxWidth)
                };
            case BreakPointsType.LG:
                return {
                    minWidth: Math.max(t, this.theme.chipNode.breakPointConstraintWidth.breakPointMinWidth),
                    maxWidth: Math.min(t, this.theme.chipNode.breakPointConstraintWidth.breakPointLgMaxWidth)
                };
            default:
                return { minWidth: 0, maxWidth: t }
        }
    }

    focusOverlay(e = null) {
        this.observeComponentCreation2(((e, i) => {
            Stack.create();
            Stack.size({ width: 1, height: 1 });
            Stack.align(Alignment.Center)
        }), Stack);
        this.observeComponentCreation2(((e, i) => {
            If.create();
            this.chipNodeOnFocus && !this.suffixIconOnFocus ? this.ifElseBranchUpdateFunction(0, (() => {
                this.observeComponentCreation2(((e, i) => {
                    Stack.create();
                    Stack.borderRadius(this.getChipNodeRadius());
                    Stack.size(this.getFocusOverlaySize());
                    Stack.borderColor(this.theme.chipNode.focusOutlineColor);
                    Stack.borderWidth(this.theme.chipNode.borderWidth)
                }), Stack);
                Stack.pop()
            })) : this.ifElseBranchUpdateFunction(1, (() => {
            }))
        }), If);
        If.pop();
        Stack.pop()
    }

    aboutToAppear() {
        this.smListener.on("change", (e => {
            e.matches && (this.chipBreakPoints = BreakPointsType.SM)
        }));
        this.mdListener.on("change", (e => {
            e.matches && (this.chipBreakPoints = BreakPointsType.MD)
        }));
        this.lgListener.on("change", (e => {
            e.matches && (this.chipBreakPoints = BreakPointsType.LG)
        }))
    }

    aboutToDisappear() {
        this.smListener.off("change");
        this.mdListener.off("change");
        this.lgListener.off("change")
    }

    chipBuilder(e = null) {
        this.observeComponentCreation2(((e, i) => {
            Row.create();
            Row.justifyContent(FlexAlign.Center);
            Row.clip(!1);
            Row.padding(this.getChipNodePadding());
            Row.height(this.getChipNodeHeight());
            Row.width(this.getChipNodeWidth());
            Row.constraintSize(this.getChipConstraintWidth());
            Row.backgroundColor(this.getChipNodeBackGroundColor());
            Row.borderRadius(this.getChipNodeRadius());
            Row.enabled(this.getChipEnable());
            Row.scale(ObservedObject.GetRawObject(this.chipScale));
            Row.focusable(!0);
            Row.colorBlend(ObservedObject.GetRawObject(this.chipBlendColor));
            Row.opacity(this.getChipNodeOpacity());
            Row.overlay({ builder: this.focusOverlay.bind(this) }, { align: Alignment.Center });
            Row.onFocus((() => {
                this.chipNodeOnFocus = !0
            }));
            Row.onBlur((() => {
                this.chipNodeOnFocus = !1
            }));
            Row.onTouch((e => {
                this.handleTouch(e)
            }));
            Row.onHover((e => {
                this.hoverAnimate(e)
            }));
            Row.onKeyEvent((e => {
                e.type !== KeyType.Down || e.keyCode !== KeyCode.KEYCODE_FORWARD_DEL || this.suffixIconOnFocus || this.deleteChipNodeAnimate()
            }))
        }), Row);
        this.observeComponentCreation2(((e, i) => {
            var t;
            If.create();
            "" !== (null === (t = this.prefixIcon) || void 0 === t ? void 0 : t.src) ? this.ifElseBranchUpdateFunction(0, (() => {
                this.observeComponentCreation2(((e, i) => {
                    var t;
                    Image.create(null === (t = this.prefixIcon) || void 0 === t ? void 0 : t.src);
                    Image.opacity(this.getChipNodeOpacity());
                    Image.size(this.getPrefixIconSize());
                    Image.fillColor(this.getPrefixIconFilledColor());
                    Image.enabled(this.getChipEnable());
                    Image.objectFit(ImageFit.Cover);
                    Image.focusable(!1);
                    Image.flexShrink(0)
                }), Image)
            })) : this.ifElseBranchUpdateFunction(1, (() => {
            }))
        }), If);
        If.pop();
        this.observeComponentCreation2(((e, i) => {
            var t, o;
            Text.create(null !== (o = null === (t = this.label) || void 0 === t ? void 0 : t.text) && void 0 !== o ? o : "");
            Text.opacity(this.getChipNodeOpacity());
            Text.fontSize(this.getLabelFontSize());
            Text.fontColor(this.getLabelFontColor());
            Text.fontFamily(this.getLabelFontFamily());
            Text.margin(this.getLabelMargin());
            Text.enabled(this.getChipEnable());
            Text.maxLines(1);
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.flexShrink(1);
            Text.focusable(!0);
            Text.textAlign(TextAlign.Center)
        }), Text);
        Text.pop();
        this.observeComponentCreation2(((e, i) => {
            Image.create(this.getSuffixIconSrc());
            Image.opacity(this.getChipNodeOpacity());
            Image.size(this.getSuffixIconSize());
            Image.fillColor(this.getSuffixIconFilledColor());
            Image.enabled(this.getChipEnable());
            Image.focusable(this.getSuffixIconFocusable());
            Image.objectFit(ImageFit.Cover);
            Image.flexShrink(0);
            ViewStackProcessor.visualState("focused");
            Image.borderColor(this.theme.chipNode.focusOutlineColor);
            Image.borderWidth(this.getSuffixIconFocusable() ? this.theme.chipNode.borderWidth : 0);
            ViewStackProcessor.visualState("normal");
            Image.borderColor(Color.Transparent);
            Image.borderWidth(0);
            ViewStackProcessor.visualState();
            Image.onFocus((() => {
                this.suffixIconOnFocus = !0
            }));
            Image.onBlur((() => {
                this.suffixIconOnFocus = !1
            }));
            Image.onClick((() => {
                var e;
                if (this.getChipEnable()) if (null === (e = this.suffixIcon) || void 0 === e ? void 0 : e.action) this.suffixIcon.action(); else if (this.allowClose && this.useDefaultSuffixIcon) {
                    this.onClose();
                    this.deleteChipNodeAnimate()
                } else ;
            }))
        }), Image);
        Row.pop()
    }

    initialRender() {
        this.observeComponentCreation2(((e, i) => {
            If.create();
            !this.deleteChip && this.toVp(this.getChipNodeHeight()) > 0 && this.toVp(this.getChipNodeWidth()) > 0 ? this.ifElseBranchUpdateFunction(0, (() => {
                this.chipBuilder.bind(this)()
            })) : this.ifElseBranchUpdateFunction(1, (() => {
            }))
        }), If);
        If.pop()
    }

    rerender() {
        this.updateDirtyElements()
    }
}
export default {
    Chip, ChipSize
}
