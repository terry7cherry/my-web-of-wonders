import InfoPanel from "./InfoPanel.js"
import * as PIXI from "pixi.js"
import toCurrency from "../utils/toCurrency.js";
import ScrollableContainer from "./ScrollableContainer.js";

const SCROLLBAR_WIDTH = 12;
const TOP_OFFSET = 60;   // space for the close button
const MARGIN = 10;
const SPACING = 5;
const ROW_HEIGHT = 120;  // fixed height per symbol row

export default class PayTable extends InfoPanel {
    constructor({state, app, x, y, width, height}) {
        super({state, app, x, y, width, height});

        this.style = {
            fontFamily: "Arial",
            fontSize: 20,
            fill: 0xffffff,
            align: "center",
        };
    }

    payTableSetup(payTable, stakeMultiplier, getSymbol) {
        if (!payTable) return;

        this.reset();
        this.payTable = payTable;
        this.stakeMultiplier = stakeMultiplier;

        const scrollHeight = this.height - TOP_OFFSET - MARGIN;
        this._scrollArea = new ScrollableContainer({
            x: 0,
            y: TOP_OFFSET,
            width: this.width,
            height: scrollHeight,
            parent: this.container,
        });

        const gridWidth = Math.floor((this.width - SCROLLBAR_WIDTH - 2 * MARGIN) / 2);

        this.grid = [];
        let symbolNum = 0;
        for (const [symbol, {func, awards}] of Object.entries(payTable)) {
            const col = symbolNum % 2;
            const row = Math.floor(symbolNum / 2);
            const x = MARGIN + col * (gridWidth + SPACING);
            const y = MARGIN + row * ROW_HEIGHT;

            this.grid[symbolNum] = this.createInfoGrid(
                x, y,
                gridWidth - SPACING,
                80,
                this.style, symbol, func, awards, getSymbol
            );
            this._scrollArea.content.addChild(this.grid[symbolNum]);
            symbolNum++;
        }

        this._scrollArea.refresh();
    }

    createInfoGrid(x, y, width, height, style, symbol, func, awards, getSymbol) {
        const entries = 1 + awards.filter(award => award > 0).length;
        style.fontSize = 15;

        const infoGrid = new PIXI.Container();
        infoGrid.x = x;
        infoGrid.y = y;

        const sprite = getSymbol(symbol);

        // Make the symbol area a square of size `height` (as your layout assumes)
        const symbolBoxSize = Math.max(height, entries * style.fontSize + 5);

        // Reset scale before measuring (prevents “scale a scaled thing” blow-ups)
        if (sprite?.scale?.set) sprite.scale.set(1, 1);

        const bounds = sprite.getLocalBounds();
        const baseW = bounds.width || 1;
        const baseH = bounds.height || 1;

        const scale = Math.min(height / baseW, height / baseH);
        sprite.scale.set(scale, scale);

        // Center inside the symbol square
        sprite.x = (symbolBoxSize - sprite.width) / 2;
        sprite.y = (symbolBoxSize - sprite.height) / 2;

        const headText = new PIXI.Text();
        headText.style = style;
        headText.text = "Type: " + func;
        headText.x = height + 10;

        infoGrid.addChild(sprite);
        infoGrid.addChild(headText);

        let count = 0;
        for (let index = awards.length - 1; index >= 0; index--) {
            if (awards[index] > 0) {
                const subText = new PIXI.Text();
                subText.x = height + 10;
                subText.y = style.fontSize * count + style.fontSize;
                subText.style = style;
                subText.text = (index + 1) + " symbols: " + toCurrency(this.stakeMultiplier * awards[index]);
                infoGrid.addChild(subText);
                count++;
            }
        }

        return infoGrid;
    }

}
