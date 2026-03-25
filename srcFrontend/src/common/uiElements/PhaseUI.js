import Grid from './Grid.js';
import Ticker from "./Ticker.js";
import * as PIXI from "pixi.js";
import toCurrency from "../utils/toCurrency.js";
import Panel from "./Panel.js";

export default class PhaseUI {
    /**
     * Constructs a new Game instance with the specified configuration.
     *
     * @constructor
     * @param {string} name - Name for balance display purposes.
     * @param {boolean} hasSpins - Does phase have a spin counter?
     * @param {Object} [layout]
     * @param {Object} colours - Optional colours for the main UI elements.
     * @param {Object} grid - Grid options.
     * @param {number} [layout.topPanelHeight] - Height of the top phase UI panel.
     * @param {number} [layout.bottomPanelHeight] - Height of the bottom phase UI panel.
     * @param {number} [layout.leftPanelWidth] - Width of the left phase UI panel.
     * @param {number} [layout.rightPanelWidth] - Width of the right phase UI panel.
     * @param {number} [layout.gridWidth] - Width of the reel grid.
     * @param {number} [layout.gridHeight] - Height of the reel grid.
     * @param {number} [layout.gridX] - X-position of the reel grid.
     * @param {number} [layout.gridY] - Y-position of the reel grid.
     * @param {number} [layout.reelWidth] - Width of each reel.
     * @param {number} [layout.symbolHeight] - Height of each symbol.
     * @param {number} [layout.width] - Width of the phase UI (including left/right panels).
     * @param {number} [layout.height] - Height of the phase UI (including top/bottom panels).
     * @param {number} [layout.x] - X position of phase UI.
     * @param {number} [layout.y] - X position of phase UI.
     * @param {number} [colours.topPanelColour] - Colour of the top panel.
     * @param {number} [colours.bottomPanelColour] - Colour of the bottom panel.
     * @param {number} [colours.leftPanelColour] - Colour of the left panel.
     * @param {number} [colours.rightPanelColour] - Colour of the right panel.
     * @param {number} [colours.winLineColour=0xf80000] - Colour for win line animations.
     * @param {number} [colours.waysWinColour=0xf80000] - Colour for ways win animations.
     * @param {number} [colours.backgroundColour=0x1099bb] - PIXI app background colour for MGame etc.
     * @param {number} [grid.numReels] - Override numReels in maths.
     * @param {number} [grid.numRows] - Override numRows in maths.
     * @param {boolean} [grid.isHoldAndSpin] - Set this grid to hold and spin mode.
     * @param {Array<string>} [grid.holdAndSpinBlankSymbols] - Array of symbol names that should not lock on spin. (First element will be used as starting symbol for grid.)
     * @param {Array<{weight: number, outcome: string}>} [grid.weightedDisplayReelSet] - For hold and spin (mostly). A symbol weight table to use for spinning animations.
     * @param {Array<Array<number>>} [grid.displayReelSet] - Reel set for spinning animations.
     * @param {Function} [grid.symbolColourMapper] - Where no symbol images (imageMap) is provided. Function that takes symbolCode and returns colour in hex format (#ffffff).
     * @param {boolean} [grid.simultaneousWinLineAnimations=false] - Whether to animate win lines simultaneously.
     * @param {number} [grid.suspenseSymbol] - Symbol code for suspense symbol.
     */
    constructor(
        name,
        hasSpins,
        layout,
        colours,
        grid,
    ) {
        this.name = name;
        this.hasSpins = hasSpins;
        this.gridOptions = grid;
        this.layout = layout;
        this.colours = colours;
    }

    build(maths, outerContainer, bottomTickerContainer, soundPlayer, tickerTextColour, imageMap, state) {
        this.maths = maths;
        this.createContainer(outerContainer);
        this.state = state;
        this.createGrid(maths, soundPlayer, imageMap);
        this.createMarginPanels();
        this.createBalanceTicker(bottomTickerContainer, tickerTextColour);
        if (this.hasSpins) this.createSpinsLeftTicker(bottomTickerContainer, tickerTextColour);
        this.close();
    }

    setDimensions(maths) {
        this.numReels = this.gridOptions.numReels ?? maths.numReels;
        this.numRows = this.gridOptions.numRows ?? maths.numRows;
        this.layout.leftPanelWidth = this.layout.leftPanelWidth ?? 0;
        this.layout.rightPanelWidth = this.layout.rightPanelWidth ?? 0;
        this.layout.topPanelHeight = this.layout.topPanelHeight ?? 0;
        this.layout.bottomPanelHeight = this.layout.bottomPanelHeight ?? 0;
        if (!this.layout.gridWidth && !this.layout.reelWidth) this.layout.reelWidth = 160;
        if (!this.layout.gridWidth) this.layout.gridWidth = this.layout.reelWidth * this.numReels;
        if (!this.layout.width) this.layout.width = this.layout.gridWidth + this.layout.leftPanelWidth + this.layout.rightPanelWidth;
        if (!this.layout.reelWidth) this.layout.reelWidth = this.layout.gridWidth / this.numReels;
        if (!this.layout.gridHeight && !this.layout.symbolHeight) this.layout.symbolHeight = 160;
        if (!this.layout.gridHeight) this.layout.gridHeight = this.layout.symbolHeight * this.numRows;
        if (!this.layout.symbolHeight) this.layout.symbolHeight = this.layout.gridHeight / this.numRows;
        if (!this.layout.height) this.layout.height = this.layout.gridHeight + this.layout.topPanelHeight + this.layout.bottomPanelHeight;
        this.layout.gridX = this.layout.gridX ?? this.layout.leftPanelWidth;
        this.layout.gridY = this.layout.gridY ?? this.layout.topPanelHeight;
    }


    createContainer(outerContainer) {
        if (typeof this.layout.x === "undefined") this.layout.x = (outerContainer.maxWidth - this.layout.width) / 2;
        if (typeof this.layout.y === "undefined") this.layout.y = (outerContainer.maxHeight - this.layout.height) / 2;
        this.container = new PIXI.Container()
        this.container.x = this.layout.x;
        this.container.y = this.layout.y;
        outerContainer.addChild(this.container);
    }

    createBalanceTicker(bottomTickerContainer, tickerTextColour) {
        const width = Math.min(200, bottomTickerContainer.maxWidth * 0.3);
        this.balance = new Ticker({
            parent: bottomTickerContainer,
            width: width,
            height: 20,
            x: bottomTickerContainer.maxWidth - width,
            pauseScale: 0,
            textFormat: {fontFamily: "Arial", fontSize: 100, fill: tickerTextColour ?? 0x000000},
        });


        this.balance.formatValue = (value) => this.name + " Winnings: " + toCurrency(value);
    }

    createSpinsLeftTicker(bottomTickerContainer, tickerTextColour) {
        const width = Math.min(200, bottomTickerContainer.maxWidth * 0.3);
        this.spinsLeft = new Ticker({
            parent: bottomTickerContainer,
            width: width,
            height: 20,
            x: Math.min(bottomTickerContainer.maxWidth - this.balance.width - width - bottomTickerContainer.maxWidth * 0.05,
                (bottomTickerContainer.maxWidth - width) / 2),
            pauseScale: 3,
            textFormat: {fontFamily: "Arial", fontSize: 100, fill: tickerTextColour ?? 0x000000},
        });

        this.spinsLeft.formatValue = (spins) => `${spins} spins left in ${this.name}`;
    }

    destroy() {
        this.container.destroy({children: true});
    }

    show(app) {
        this.ended = false;
        this.balance.updateInstant(0);
        this.container.visible = true;
        this.balance.setVisible(true);
        this.spinsLeft?.setVisible(true);
        app.renderer.background.color = this.colours.backgroundColour ?? 0x1099bb;
    }

    close() {
        this.container.visible = false;
        this.balance.setVisible(false);
        this.spinsLeft?.setVisible(false);
    }

    createGrid(maths, soundPlayer, imageMap) {
        this.gridContainer = new PIXI.Container();
        this.gridContainer.x = this.layout.gridX;
        this.gridContainer.y = this.layout.gridY;
        this.container.addChild(this.gridContainer);

        this.grid = new Grid({
            parent: this.gridContainer,
            width: this.layout.gridWidth,
            height: this.layout.gridHeight,
            numReels: this.numReels,
            numRows: this.numRows,
            displayReelSet: this.gridOptions.displayReelSet,
            symbolColourMapper: this.gridOptions.symbolColourMapper,
            sound: soundPlayer,
            symbolMap: maths.symbolNames,
            imageMap: imageMap,
            suspenseSymbol: this.gridOptions.suspenseSymbol,
            simultaneousWinLineAnimations: this.gridOptions.simultaneousWinLineAnimations,
            isHoldAndSpin: this.gridOptions.isHoldAndSpin,
            holdAndSpinBlankSymbols: this.gridOptions.holdAndSpinBlankSymbols,
            weightedDisplayReelSet: this.gridOptions.weightedDisplayReelSet,
            winLineColour: this.colours.winLineColour ?? 0xf80000,
            waysWinColour: this.colours.waysWinColour ?? 0xf80000,
            state: this.state,
        });
    }

    createMarginPanels() {
        if (this.layout.topPanelHeight)
            this.topPanel = new Panel({
                x: this.layout.gridX,
                y: this.layout.gridY - this.layout.topPanelHeight,
                width: this.grid.width,
                height: this.layout.topPanelHeight,
                colour: this.colours.topPanelColour,
                parent: this.container
            });

        if (this.layout.bottomPanelHeight)
            this.bottomPanel = new Panel({
                x: this.layout.gridX,
                y: this.layout.gridY + this.layout.gridHeight,
                width: this.layout.gridWidth,
                height: this.layout.bottomPanelHeight,
                colour: this.colours.bottomPanelColour,
                parent: this.container
            })

        if (this.layout.leftPanelWidth)
            this.leftPanel = new Panel({
                x: 0,
                y: this.layout.gridY,
                width: this.layout.leftPanelWidth,
                height: this.layout.gridHeight,
                colour: this.colours.leftPanelColour,
                parent: this.container
            })

        if (this.layout.rightPanelWidth)
            this.rightPanel = new Panel({
                x: this.layout.gridWidth + this.layout.gridX,
                y: this.layout.gridY,
                width: this.layout.rightPanelWidth,
                height: this.layout.gridHeight,
                colour: this.colours.rightPanelColour,
                parent: this.container
            })
    }

    async checkBalanceAgainstServer(serverWinnings, ticker) {
        if (serverWinnings === this.balance.value) return;
        await ticker.updateText("Free spins total win mismatch.")
        await ticker.updateText(`Server: ${toCurrency(serverWinnings)}. Frontend: ${toCurrency(this.balance.value)}`);
        throw new Error("Free spins total win mismatch.");
    }
}
