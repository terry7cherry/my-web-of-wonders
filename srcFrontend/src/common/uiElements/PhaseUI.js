import Grid from './Grid.js';
import Ticker from "./Ticker.js";
import * as PIXI from "pixi.js";

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
     * @param {number} [layout.gridWidth] - Width of the reel grid.
     * @param {number} [layout.gridHeight] - Height of the reel grid.
     * @param {number} [layout.gridX] - X-position of the reel grid.
     * @param {number} [layout.gridY] - Y-position of the reel grid.
     * @param {number} [layout.reelWidth] - Width of each reel.
     * @param {number} [layout.symbolHeight] - Height of each symbol.
     * @param {number} [layout.width] - Width of the phase UI (including left/right panels).
     * @param {number} [layout.height] - Height of the phase UI (including top/bottom panels).
     * @param {number} [layout.topPanelHeight] - Height of the top UI panel.
     * @param {number} [layout.bottomPanelHeight] - Height of the bottom UI panel.
     * @param {number} [layout.leftPanelWidth] - Width of the left UI panel.
     * @param {number} [layout.rightPanelWidth] - Width of the right UI panel.
     * @param {number} [layout.x] - X position of phase UI.
     * @param {number} [layout.y] - X position of phase UI.
     * @param {number} [colours.topPanelColour] - Colour of the top panel.
     * @param {number} [colours.bottomPanelColour] - Colour of the bottom panel.
     * @param {number} [colours.leftPanelColour] - Colour of the left panel.
     * @param {number} [colours.rightPanelColour] - Colour of the right panel.
     * @param {number} [colours.winLineColour=0xf80000] - Colour for win line animations.
     * @param {number} [colours.backgroundColour=0x1099bb] - PIXI app background colour for MGame etc.
     * @param {number} [grid.numReels] - Override numReels in maths.
     * @param {number} [grid.numRows] - Override numRows in maths.
     * @param {Array<Array<number>>} [grid.displayReelSet] - Reel set for spins.
     * @param {Function} [grid.symbolColourMapper] - Where no symbol images (imageMap) is provided. Function that takes symbolCode and returns colour in hex format (#ffffff).
     * @param {boolean} [grid.simultaneousWinLineAnimations=false] - Whether to animate win lines simultaneously.
     * @param {number} [grid.suspenseSymbolCode] - Symbol code for suspense symbol.
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
        this.setColours(colours);


    }

    build(maths, outerContainer, bottomTickerContainer, soundPlayer, tickerTextColour, imageMap) {
        this.maths = maths;
        this.createContainer(outerContainer);
        this.createGrid(maths, soundPlayer, imageMap);
        this.createMarginPanels();
        this.createBalanceTicker(bottomTickerContainer, tickerTextColour);
        if (this.hasSpins) this.createSpinsLeftTicker(bottomTickerContainer, tickerTextColour);
        this.hide();
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


    setColours(
        {
            topPanelColour,
            bottomPanelColour,
            leftPanelColour,
            rightPanelColour,
            backgroundColour = 0x1099bb,
            winLineColour = 0xf80000,
        }
    ) {
        this.backgroundColour = backgroundColour;
        this.topPanelColour = topPanelColour;
        this.bottomPanelColour = bottomPanelColour;
        this.leftPanelColour = leftPanelColour;
        this.rightPanelColour = rightPanelColour;
        this.winLineColour = winLineColour;
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
        this.balance = new Ticker({
            parent: bottomTickerContainer,
            width: 200,
            height: 20,
            x: this.layout.width - 200,
            pauseScale: 0,
            textFormat: {fontFamily: "Arial", fontSize: 100, fill: tickerTextColour ?? 0x000000},
        });


        this.balance.update = async (value) => {
            this.balance.value = value;
            await this.balance.updateText(this.name + " Winnings: £" + (this.balance.value / 100).toFixed(2));
        }
    }

    createSpinsLeftTicker(bottomTickerContainer, tickerTextColour) {
        this.spinsLeft = new Ticker({
            parent: bottomTickerContainer,
            width: 200,
            height: 20,
            x: (this.layout.width - 200) / 2,
            pauseScale: 3,
            textFormat: {fontFamily: "Arial", fontSize: 100, fill: tickerTextColour ?? 0x000000},
        });

        this.spinsLeft.update = async (spins) => {
            this.spinsLeft.value = spins;
            await this.spinsLeft.updateText(`${this.spinsLeft.value} spins left in ${this.name}`);
        }
    }

    destroy() {
        this.container.destroy({children: true});
    }

    show(app) {
        this.balance.update(0);
        this.container.visible = true;
        this.balance.setVisible(true);
        this.spinsLeft?.setVisible(true);
        app.renderer.background.color = this.backgroundColour;
    }

    hide() {
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
            suspenseSymbolCode: this.gridOptions.suspenseSymbolCode ?? maths.suspenseSymbolCode,
            simultaneousWinLineAnimations: this.gridOptions.simultaneousWinLineAnimations,
            winLineColour: this.winLineColour,
        });
    }

    createMarginPanels() {
        if (this.layout.topPanelHeight)
            this.topPanel = this.createPanel(
                this.layout.gridX,
                this.layout.gridY - this.layout.topPanelHeight,
                this.grid.width,
                this.layout.topPanelHeight,
                this.topPanelColour);

        if (this.layout.bottomPanelHeight)
            this.bottomPanel = this.createPanel(
                this.layout.gridX,
                this.layout.gridY + this.layout.gridHeight,
                this.layout.gridWidth,
                this.layout.bottomPanelHeight,
                this.bottomPanelColour,
            )

        if (this.layout.leftPanelWidth)
            this.leftPanel = this.createPanel(
                0,
                this.layout.gridY,
                this.layout.leftPanelWidth,
                this.layout.gridHeight,
                this.leftPanelColour
            )

        if (this.layout.rightPanelWidth)
            this.rightPanel = this.createPanel(
                this.layout.gridWidth + this.layout.gridX,
                this.layout.gridY,
                this.layout.rightPanelWidth,
                this.layout.gridHeight,
                this.rightPanelColour
            )
    }

    createPanel(x, y, width, height, colour) {
        const panelContainer = new PIXI.Container();
        panelContainer.x = x;
        panelContainer.y = y;
        panelContainer.shape = {
            width, height
        }
        this.container.addChild(panelContainer);
        if (colour) {
            panelContainer.addChild(this.createPanelBackground(width, height, colour))
        }
        return panelContainer;
    }

    createPanelBackground(width, height, colour) {
        const bg = new PIXI.Sprite(PIXI.Texture.WHITE);
        bg.width = width;
        bg.height = height;
        bg.tint = colour;
        return bg;
    }
}
