import InfoPanel from "./InfoPanel.js"
import * as PIXI from "pixi.js"

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

        const margin = 50;
        const spacing = 5;

        const gridWidth = Math.floor((this.container.width - 2 * margin) / 2);
        const gridHeight = Math.floor((this.container.height - 2 * margin) * 2 / Object.keys(payTable).length);

        let x;
        let y;
        this.grid = [];
        let symbolNum = 0
        for (const [symbol, {func, awards}] of Object.entries(payTable)) {
            if (symbolNum % 2 === 0) x = margin;
            else x = margin + gridWidth;
            y = Math.floor(symbolNum / 2) * gridHeight + margin;
            this.grid[symbolNum] = this.createInfoGrid(x, y, gridWidth - 2 * spacing, gridHeight - 2 * spacing, this.style, symbol, func, awards, getSymbol);
            this.container.addChild(this.grid[symbolNum]);
            symbolNum++;
        }
    }

    createInfoGrid(x, y, width, height, style, symbol, func, awards, getSymbol) {
        const entries = 1 + awards.filter(award => award > 0).length;
        const textHeight = Math.floor(height / entries);

        style.fontSize = Math.min(textHeight, Math.floor(height / 4));

        const infoGrid = new PIXI.Container();
        infoGrid.x = x;
        infoGrid.y = y;

        const sprite = getSymbol(symbol);

        // Make the symbol area a square of size `height` (as your layout assumes)
        const symbolBoxSize = height;

        // Reset scale before measuring (prevents “scale a scaled thing” blow-ups)
        if (sprite?.scale?.set) sprite.scale.set(1, 1);

        const bounds = sprite.getLocalBounds();
        const baseW = bounds.width || 1;
        const baseH = bounds.height || 1;

        const scale = Math.min(symbolBoxSize / baseW, symbolBoxSize / baseH);
        sprite.scale.set(scale, scale);

        // Center inside the symbol square
        sprite.x = (symbolBoxSize - sprite.width) / 2;
        sprite.y = (symbolBoxSize - sprite.height) / 2;

        const headText = new PIXI.Text();
        headText.style = style;
        headText.text = "Type: " + func;
        headText.x = symbolBoxSize + 10;

        infoGrid.addChild(sprite);
        infoGrid.addChild(headText);

        let count = 0;
        for (let index = awards.length - 1; index >= 0; index--) {
            if (awards[index] > 0) {
                const subText = new PIXI.Text();
                subText.x = symbolBoxSize + 10;
                subText.y = textHeight * count + textHeight;
                subText.style = style;
                subText.text = (index + 1) + " symbols: " + "£" + (this.stakeMultiplier * awards[index] / 100).toFixed(2);
                infoGrid.addChild(subText);
                count++;
            }
        }

        return infoGrid;
    }
}
