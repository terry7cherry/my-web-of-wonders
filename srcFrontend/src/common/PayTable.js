import InfoPanel from "./InfoPanel.js"
import * as PIXI from "pixi.js"

export default class PayTable extends InfoPanel {
    constructor({game, app, x, y, width, height, getSymbol}) {
        super({game, app, x, y, width, height});

        this.style = {
           fontFamily: "Arial",
           fontSize: 20,
           fill: 0xffffff,
           align: "center",
         };

        this.getSymbol = getSymbol;
        this.payTableSetup(game.basePayTable, game.stakeMultiplier);
    }

    payTableSetup(payTable, stakeMultiplier) {
        if (!payTable) return;

        this.reset();
        this.payTable = payTable;
        this.stakeMultiplier = stakeMultiplier;

        var margin = 50;
        var spacing = 5;

        var gridWidth = Math.floor((this.container.width - 2 * margin)/ 2);
        var gridHeight = Math.floor((this.container.height - 2 * margin) * 2/ payTable.length);

        var x;
        var y;
        this.grid = [];
        for (let symbol = 0; symbol < payTable.length; symbol++) {
            if (symbol % 2 == 0) x = margin;
                else x = margin + gridWidth;
            y = Math.floor(symbol/2) * gridHeight + margin;
            this.grid[symbol] = this.createInfoGrid(x, y, gridWidth- 2*spacing, gridHeight - 2*spacing, this.style, this.payTable[symbol]);
            this.container.addChild(this.grid[symbol]);
        }
    }

    createInfoGrid(x, y, width, height, style, symbolInfo) {
        var entries = 1 + this.countNonZeroReturns(symbolInfo);
        var textHeight = Math.floor(height / entries);

        var newStyle = style;
        newStyle.fontSize = Math.min(textHeight, Math.floor(height / 4));

        const infoGrid = new PIXI.Container();
        infoGrid.x = x;
        infoGrid.y = y;

        const sprite = this.getSymbol(symbolInfo.id).symbol;
        const scale = Math.min(height / sprite.height, height / sprite.width);
        sprite.scale.set(scale, scale);
        sprite.x = (height - sprite.width) / 2;
        sprite.y = (height - sprite.height) / 2;

        const headText = new PIXI.Text();
        headText.style = style;
        headText.text = "Type: " + symbolInfo.function;
        headText.x = height + 10;

        infoGrid.addChild(sprite);
        infoGrid.addChild(headText);

        var count = 0;
        for (let index = symbolInfo.wins.length - 1; index >= 0; index--) {
            if (symbolInfo.wins[index] > 0) {
                const subText = new PIXI.Text();
                subText.x = height + 10;
                subText.y = textHeight * count + textHeight;
                subText.style = style;
                subText.text= (index + 1) + " symbols: " + "£" + (this.stakeMultiplier * symbolInfo.wins[index] / 100).toFixed(2);
                infoGrid.addChild(subText);
                count++
            }
        }

        return infoGrid;
    }

    countNonZeroReturns(symbolInfo) {
        var result = 0;
        var array = symbolInfo.wins;
        for (let count = 0; count < array.length; count++) {
            if (array[count] > 0) result++;
        }
        return result;
    }
}
