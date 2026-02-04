import Ticker from "../uiElements/Ticker.js";
import PayTable from "./PayTable.js";
import BetSelector from "./BetSelector.js"
import Forces from "./Forces.js";
import Button from "../uiElements/Button.js";
import NiceText from "../uiElements/NiceText.js";
import * as PIXI from "pixi.js";

export default class StandardGameUI {
    /**
     * Constructs a new Game instance with the specified configuration.
     *
     * @constructor
     * @param {Object} state - State object containing running bool.
     * @param {Object} maths - RTP- and betType-specific maths info sent by server.
     // param that is an Object where each value is a PhaseUI instance.
     * @param {Object<string, PhaseUI>} phases - Object with named phase UIs.
     * @param {Object} soundPlayer - PIXI sound player.
     * @param {ShellUI} shell - ShellUI instance.
     * @param {Object<string, PIXI.ImageBitmapResource>} imageMap - Map of images by symbol name.
     * @param {Object} layout - Optional layout parameters providing the size and position of the main UI elements.
     * @param {Object} colours - Optional colours for the main UI elements.
     * @param {number} [layout.titleWidth] - Width of the game title.
     * @param {number} [layout.titleHeight] - Height of the game title.
     * @param {number} [layout.titleX] - X position of the game title.
     * @param {number} [layout.titleY] - Y position of the game title.
     * @param {number} [colours.titleColour=0xb02525] - Colour of the title text.
     * @param {number} [colours.tickerTextColour] - Colour of text in tickers.
     */
    constructor(
        phases,
        layout,
        colours,
    ) {
        this.phases = phases;
        this.layout = layout;
        this.colours = colours;
    }

    init(state, maths, soundPlayer, shell, imageMap) {
        this.state = state;
        this.maths = maths;
        this.setDimensions({
            ...this.layout
        });
        this.setColours(this.colours);
        this.createPixiApp();
        this.createTitleText(maths.gameTitle);
        this.createSpinButtons();
        this.createTicker(shell);
        this.createBottomTickers();
        this.createPhaseContainer();
        this.buildPhaseUIs(soundPlayer, imageMap);
        this.createPayTable();
        this.createForces();
        this.createConfigPanel();
        this.setPhase("base");
    }

    setDimensions({
                      titleWidth,
                      titleHeight,
                      titleX,
                      titleY,
                  }) {
        Object.values(this.phases).forEach(phase => phase.setDimensions(this.maths));
        this.maxPhaseHeight = Math.max(...Object.values(this.phases).map(phase => phase.layout.height));
        this.width = Math.max(...Object.values(this.phases).map(phase => phase.layout.width));
        this.titleHeight = titleHeight ?? 100;
        this.titleY = titleY ?? 0;
        this.titleWidth = titleWidth ?? this.width - 100;
        this.titleX = titleX ?? (this.width - this.titleWidth) / 2;
        this.height = 150 + this.titleHeight + this.maxPhaseHeight;
    }

    setPhase(phaseName) {
        for (const phase in this.phases)
            if (phase === phaseName)
                this.phases[phaseName].show(this.app);
            else
                this.phases[phase].hide();
        this.currentPhase = this.phases[phaseName];
    }

    setColours(
        {
            titleColour = 0xb02525,
            tickerTextColour,
        }
    ) {
        this.titleColour = titleColour;
        this.tickerTextColour = tickerTextColour;
    }

    destroy() {
        // Destroy phase UIs
        Object.values(this.phases).forEach(phase => phase.destroy());
        // Stop render loop
        this.app.stop();

        // Destroy PIXI internals
        this.app.destroy(true, {
            children: true,
            texture: true,
            baseTexture: true,
        });

        // Remove canvas from DOM if still attached
        if (this.app.view?.parentNode) {
            this.app.view.parentNode.removeChild(this.app.view);
        }

        this.app = null;
    }

    createPixiApp() {
        this.app = new PIXI.Application({
            width: this.width,
            height: this.height,
            backgroundColour: this.backgroundColour,
        });
        document.getElementById("game-container").appendChild(this.app.view);
    }

    createTitleText(title) {
        this.titleText = new NiceText({
            app: this.app,
            text: title,
            width: this.titleWidth,
            height: this.titleHeight,
            x: this.titleX,
            y: this.titleY,
            textColour: this.titleColour,
        });

    }

    createSpinButtons() {
        this.spinButton = new Button({
            x: 0,
            y: this.height - 150,
            width: 200,
            height: 60,
            app: this.app,
            text: "Spin",
        });

        this.autoPlayButton = new Button({
            x: 0,
            y: this.height - 90,
            width: 200,
            height: 30,
            app: this.app,
            text: "Auto-Play",
            colour: "#7e32b8",
        });
    }

    createTicker(shell) {
        this.ticker = new Ticker({
            app: this.app,
            width: 580,
            height: 100,
            x: 220,
            y: this.height - 150,
            textFormat: {
                fontFamily: "Arial",
                fontSize: 100,
                fill: 0xffffff,
            },
            historyTextBox: shell.eventHistory,
        });
    }

    createBottomTickers() {
        this.bottomTickerContainer = new PIXI.Container();
        this.bottomTickerContainer.x = 0;
        this.bottomTickerContainer.y = this.height - 50;
        this.app.stage.addChild(this.bottomTickerContainer);

        this.balance = new Ticker({
            parent: this.bottomTickerContainer,
            width: 200,
            height: 20,
            pauseScale: 0,
            textFormat: {
                fontFamily: "Arial",
                fontSize: 100,
                fill: this.tickerTextColour ?? 0x000000,
            },
        });

        this.balance.update = async (value) => {
            this.balance.value = value;
            await this.balance.updateText("Balance: £" + (this.balance.value / 100).toFixed(2));
        }
    }

    createPayTable() {
        this.payTable = new PayTable({
            state: this.state,
            app: this.app,
            x: 50,
            y: 50,
            width: this.width - 100,
            height: this.height - 100,
        });
    }

    createForces() {
        this.forces = new Forces({
            state: this.state,
            app: this.app,
            x: 50,
            y: 50,
            width: this.width - 100,
            height: this.height - 100,
        });
    }

    createConfigPanel() {
        const configPanel = new PIXI.Container();
        configPanel.x = 0;
        configPanel.y = this.height - 30;
        const panelBackground = new PIXI.Graphics();
        panelBackground.beginFill(0x012d36);
        panelBackground.drawRect(0, 0, this.width, 30);
        panelBackground.endFill();
        configPanel.addChild(panelBackground);
        this.app.stage.addChild(configPanel);

        configPanel.payTableButton = new Button({
            x: this.width - 300,
            y: 0,
            width: 150,
            height: 30,
            onClick: () => {
                this.payTable.show();
            },
            text: "Pay Table",
            colour: "#7eb832",
            parent: configPanel,
        });

        configPanel.forcesButton = new Button({
            x: this.width - 150,
            y: 0,
            width: 150,
            height: 30,
            onClick: () => {
                this.forces.show();
            },
            text: "Forces",
            colour: "#7e32b8",
            parent: configPanel,
        });

        configPanel.betSelector = new BetSelector({
            x: 0,
            y: 0,
            parent: configPanel,
            defaultStake: this.maths.minBetCost,
        })

        this.configPanel = configPanel;
    }

    createPhaseContainer() {
        const container = new PIXI.Container();
        container.x = 0;
        container.y = this.titleHeight;
        container.maxHeight = this.maxPhaseHeight;
        container.maxWidth = this.width;
        this.app.stage.addChild(container);
        this.phaseContainer = container;
    }

    buildPhaseUIs(soundPlayer, imageMap) {
        Object.values(this.phases).forEach(phase => {
            phase.build(this.maths, this.phaseContainer, this.bottomTickerContainer, soundPlayer, this.tickerTextColour, imageMap);
        });
    }
}
