import * as PIXI from "pixi.js";

const SCROLLBAR_WIDTH = 12;
const SCROLLBAR_TRACK_COLOUR = 0x333333;
const SCROLLBAR_THUMB_COLOUR = 0x888888;

export default class ScrollableContainer extends PIXI.Container {
    /**
     * A container that clips its content to a visible area and provides
     * a scroll bar + mouse-wheel scrolling. Uses native PIXI only.
     *
     * @param {object} opts
     * @param {number} opts.x
     * @param {number} opts.y
     * @param {number} opts.width   - Visible width (including scrollbar)
     * @param {number} opts.height  - Visible height
     * @param {PIXI.Container} opts.parent
     */
    constructor({x, y, width, height, parent}) {
        super();
        this.x = x;
        this.y = y;
        this._visibleWidth = width;
        this._visibleHeight = height;

        // --- Clipping mask ---
        this._maskGraphic = new PIXI.Graphics();
        this._maskGraphic.beginFill(0xffffff);
        this._maskGraphic.drawRect(0, 0, width, height);
        this._maskGraphic.endFill();
        this.addChild(this._maskGraphic);

        // --- Content container (what gets scrolled) ---
        this.content = new PIXI.Container();
        this.content.mask = this._maskGraphic;
        this.addChild(this.content);

        // --- Scrollbar track ---
        const trackX = width - SCROLLBAR_WIDTH;
        this._track = new PIXI.Graphics();
        this._track.beginFill(SCROLLBAR_TRACK_COLOUR);
        this._track.drawRect(trackX, 0, SCROLLBAR_WIDTH, height);
        this._track.endFill();
        this.addChild(this._track);

        // --- Scrollbar thumb ---
        this._thumb = new PIXI.Graphics();
        this.addChild(this._thumb);

        // --- State ---
        this._scrollY = 0;
        this._dragging = false;
        this._dragStartY = 0;
        this._dragStartScrollY = 0;

        this._setupThumbDrag();
        this._setupWheelScroll();

        if (parent) parent.addChild(this);
    }

    /**
     * The total scrollable content height. Call this after adding all children
     * to `this.content`, or whenever the content changes.
     */
    get contentHeight() {
        return this._contentHeight ?? this._visibleHeight;
    }

    /**
     * Recalculates thumb size & position based on the current content height.
     * Must be called after the content has been fully populated / updated.
     */
    refresh() {
        this._contentHeight = this.content.height;
        this._updateThumb();
        this._applyScroll();
    }

    // ── Private ────────────────────────────────────────────────────────────────

    _maxScroll() {
        return Math.max(0, this._contentHeight - this._visibleHeight);
    }

    _thumbHeight() {
        if (this._maxScroll() === 0) return this._visibleHeight;
        return Math.max(
            30,
            (this._visibleHeight / this._contentHeight) * this._visibleHeight
        );
    }

    _thumbY() {
        if (this._maxScroll() === 0) return 0;
        const trackRange = this._visibleHeight - this._thumbHeight();
        return (this._scrollY / this._maxScroll()) * trackRange;
    }

    _updateThumb() {
        const trackX = this._visibleWidth - SCROLLBAR_WIDTH;
        const th = this._thumbHeight();
        const ty = this._thumbY();
        this._thumb.clear();
        this._thumb.beginFill(SCROLLBAR_THUMB_COLOUR);
        this._thumb.drawRoundedRect(trackX + 1, ty, SCROLLBAR_WIDTH - 2, th, 4);
        this._thumb.endFill();

        // Hide scrollbar when not needed
        this._track.visible = this._thumb.visible = this._maxScroll() > 0;
    }

    _applyScroll() {
        this.content.y = -this._scrollY;
        this._updateThumb();
    }

    _clampScroll(value) {
        return Math.max(0, Math.min(value, this._maxScroll()));
    }

    _scrollTo(y) {
        this._scrollY = this._clampScroll(y);
        this._applyScroll();
    }

    _setupThumbDrag() {
        this._thumb.interactive = true;
        this._thumb.cursor = "pointer";

        this._thumb.on("pointerdown", (e) => {
            this._dragging = true;
            this._dragStartY = e.global.y;
            this._dragStartScrollY = this._scrollY;
            e.stopPropagation();
        });

        // Use the stage-level events so dragging outside the thumb still works.
        // We attach to `this` (the ScrollableContainer itself) so we don't need
        // a reference to the app.
        this.on("pointermove", (e) => {
            if (!this._dragging) return;
            const dy = e.global.y - this._dragStartY;
            const trackRange = this._visibleHeight - this._thumbHeight();
            if (trackRange === 0) return;
            const scrollDelta = (dy / trackRange) * this._maxScroll();
            this._scrollTo(this._dragStartScrollY + scrollDelta);
        });

        this.on("pointerup", () => {
            this._dragging = false;
        });
        this.on("pointerupoutside", () => {
            this._dragging = false;
        });

        // Make the whole container interactive so pointermove is received
        this.interactive = true;
    }

    _setupWheelScroll() {
        // PIXI doesn't expose a wheel event natively, so we listen on the canvas.
        // We lazily grab the canvas reference the first time we are added to a stage.
        this.on("added", () => {
            const canvas = this._getCanvas();
            if (!canvas) return;
            this._wheelHandler = (e) => {
                // Don't scroll if this container or any ancestor is hidden
                if (!this.worldVisible) return;

                // Only scroll when the pointer is over our visible area
                const bounds = canvas.getBoundingClientRect();
                const localX = e.clientX - bounds.left - this.getGlobalPosition().x;
                const localY = e.clientY - bounds.top - this.getGlobalPosition().y;
                if (
                    localX < 0 || localX > this._visibleWidth ||
                    localY < 0 || localY > this._visibleHeight
                ) return;
                e.preventDefault();
                this._scrollTo(this._scrollY + e.deltaY * 0.5);
            };
            canvas.addEventListener("wheel", this._wheelHandler, {passive: false});
        });

        this.on("removed", () => {
            const canvas = this._getCanvas();
            if (canvas && this._wheelHandler) {
                canvas.removeEventListener("wheel", this._wheelHandler);
            }
        });
    }

    _getCanvas() {
        // Walk up through PIXI parents to find the Application's view (canvas).
        let obj = this;
        while (obj.parent) obj = obj.parent;
        // `obj` is now the PIXI Stage; its renderer's view is the canvas.
        return obj?._application?.view ?? document.querySelector("canvas");
    }
}