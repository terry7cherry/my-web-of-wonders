import Ticker from "../uiElements/Ticker.js";
import PayTable from "./PayTable.js";
import BetSelector from "./BetSelector.js"
import Forces from "./Forces.js";
import Button from "../uiElements/Button.js";
import NiceText from "../uiElements/NiceText.js";
import * as PIXI from "pixi.js";
import toCurrency from "../utils/toCurrency.js";
import Panel from "../uiElements/Panel.js";

export default class StandardGameUI {
    /**
     * Constructs a new Game instance with the specified configuration.
     *
     * @constructor
     // param that is an Object where each value is a PhaseUI instance.
     * @param {Object<string, PhaseUI>} phases - Object with named phase UIs.
     * @param {Object} layout - Optional layout parameters providing the size and position of the main UI elements.
     * @param {Object} colours - Optional colours for the main UI elements.
     * @param {number} [layout.topPanelHeight] - Height of the top game UI panel.
     * @param {number} [layout.bottomPanelHeight] - Height of the bottom game UI panel.
     * @param {number} [layout.leftPanelWidth] - Width of the left game UI panel.
     * @param {number} [layout.rightPanelWidth] - Width of the right game UI panel.
     * @param {number} [layout.titleWidth] - Width of the game title.
     * @param {number} [layout.titleHeight] - Height of the game title.
     * @param {number} [layout.titleX] - X position of the game title.
     * @param {number} [layout.titleY] - Y position of the game title.
     * @param {number} [colours.topPanelColour] - Colour of the top panel.
     * @param {number} [colours.bottomPanelColour] - Colour of the bottom panel.
     * @param {number} [colours.leftPanelColour] - Colour of the left panel.
     * @param {number} [colours.rightPanelColour] - Colour of the right panel.
     * @param {number} [colours.titleColour=0xb02525] - Colour of the title text.
     * @param {number} [colours.tickerTextColour] - Colour of text in tickers.
     */
    constructor(
        layout,
        colours,
        phases,
    ) {
        this.phases = phases;
        this.layout = layout;
        this.colours = colours;
    }

    /**
     * Initialises the game UI.
     * @param {Object} dependencies - Dependencies for the game UI.
     * @param {Object} dependencies.state - State object containing running bool.
     * @param {Object} dependencies.maths - RTP- and betType-specific maths info sent by server.
     * @param {Object} dependencies.soundPlayer - PIXI sound player.
     * @param {ShellUI} dependencies.shell - ShellUI instance.
     * @param {Object<string, PIXI.ImageBitmapResource>} dependencies.imageMap - Map of images by symbol name.
     * @returns {Promise<void>}
     */
    async init({state, maths, soundPlayer, shell, imageMap}) {
        this.state = state;
        this.maths = maths;
        this.imageMap = imageMap;
        this.setDimensions({
            ...this.layout
        });
        this.createPixiApp();
        this.createTitleText(maths.gameTitle);
        this.createSpinButtons();
        this.createTicker(shell);
        this.createBottomTickers();
        this.createPhaseContainer();
        this.buildPhaseUIs(soundPlayer, imageMap);
        this.createMarginPanels();
        this.createPayTable();
        this.createForces();
        this.createConfigPanel();
        await this.setPhase("base", true);
    }

    setDimensions({
                      titleWidth,
                      titleHeight,
                      titleX,
                      titleY,
                      leftPanelWidth = 0,
                      rightPanelWidth = 0,
                      topPanelHeight = 0,
                      bottomPanelHeight = 0,
                  }) {
        Object.values(this.phases).forEach(phase => phase.setDimensions(this.maths));
        this.layout.maxPhaseHeight = Math.max(...Object.values(this.phases).map(phase => phase.layout.height));
        this.layout.leftPanelWidth = leftPanelWidth ?? 0;
        this.layout.rightPanelWidth = rightPanelWidth ?? 0;
        this.layout.topPanelHeight = topPanelHeight ?? 0;
        this.layout.bottomPanelHeight = bottomPanelHeight ?? 0;
        this.layout.maxPhaseWidth = Math.max(...Object.values(this.phases).map(phase => phase.layout.width));
        this.layout.width = this.layout.maxPhaseWidth + this.layout.leftPanelWidth + this.layout.rightPanelWidth;
        this.layout.titleHeight = titleHeight ?? 100;
        this.layout.titleY = titleY ?? 0;
        this.layout.titleWidth = titleWidth ?? this.layout.width - 100;
        this.layout.titleX = titleX ?? (this.layout.width - this.layout.titleWidth) / 2;
        this.layout.height = 150 + this.layout.titleHeight + this.layout.topPanelHeight + this.layout.maxPhaseHeight + this.layout.bottomPanelHeight;
    }

    async setPhase(phaseID, startOfRound) {
        if (!startOfRound) {
            await this.endCurrentPhase();
        }
        this.currentPhase?.close();
        this.phases[phaseID].show(this.app);
        this.currentPhase = this.phases[phaseID];
        this.currentPhaseID = phaseID;
    }

    async endCurrentPhase(serverWin, displayForBase) {
        if (this.currentPhase.ended) return;
        this.currentPhase.ended = true;
        this.state.totalGameRoundWinnings += this.currentPhase.balance.value;
        if (typeof serverWin !== "undefined") await this.currentPhase.checkBalanceAgainstServer(serverWin, this.ticker);
        if (this.currentPhaseID === "base" && !displayForBase) return;
        await this.ticker.updateText(`${this.currentPhase.name} over!`);
        await this.ticker.updateText(`${this.currentPhase.name} total: ${toCurrency(this.currentPhase.balance.value)}!`);
    }

    destroy() {
        // Destroy phase UIs
        Object.values(this.phases).forEach(phase => phase.destroy());
        // Stop render loop
        this.app.stop();

        // Destroy PIXI internals
        // IMPORTANT: Do NOT destroy textures/baseTextures here because they are shared
        // via the global imageMap (loaded once through PIXI.Assets). Destroying them
        // invalidates those references on subsequent reloads, causing errors like
        // "texture.baseTexture is undefined" when creating new Sprites.
        this.app.destroy(true, {
            children: true,
            texture: false,
            baseTexture: false,
        });

        // Remove canvas from DOM if still attached
        if (this.app.view?.parentNode) {
            this.app.view.parentNode.removeChild(this.app.view);
        }

        this.app = null;
    }

    createPixiApp() {
        /** @type {PIXI.Application} */
        this.app = new PIXI.Application({
            width: this.layout.width,
            height: this.layout.height,
        });
        document.getElementById("game-container").appendChild(this.app.view);
    }

    createTitleText(title) {
        this.titleText = new NiceText({
            app: this.app,
            text: title,
            width: this.layout.titleWidth,
            height: this.layout.titleHeight,
            x: this.layout.titleX,
            y: this.layout.titleY,
            textColour: this.colours.titleColour ?? 0xb02525,
        });

    }

    createSpinButtons() {
        this.spinButton = new Button({
            x: 0,
            y: this.layout.height - 150,
            width: 200,
            height: 60,
            app: this.app,
            text: "Spin",
        });

        this.autoPlayButton = new Button({
            x: 0,
            y: this.layout.height - 90,
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
            width: this.layout.width - this.spinButton.buttonWidth,
            height: 100,
            x: 220,
            y: this.layout.height - 150,
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
        this.bottomTickerContainer.y = this.layout.height - 50;
        this.app.stage.addChild(this.bottomTickerContainer);

        this.balance = new Ticker({
            parent: this.bottomTickerContainer,
            width: 200,
            height: 20,
            pauseScale: 0,
            textFormat: {
                fontFamily: "Arial",
                fontSize: 100,
                fill: this.colours.tickerTextColour ?? 0x000000,
            },
        });

        this.balance.formatValue = (value) => "Balance: " + toCurrency(value);
    }

    createPayTable() {
        this.payTable = new PayTable({
            state: this.state,
            app: this.app,
            x: 50,
            y: 50,
            width: this.layout.width - 100,
            height: this.layout.height - 100,
        });
    }

    createForces() {
        this.forces = new Forces({
            state: this.state,
            app: this.app,
            x: 50,
            y: 50,
            width: this.layout.width - 100,
            height: this.layout.height - 100,
        });
    }

    createConfigPanel() {
        const configPanel = new PIXI.Container();
        configPanel.x = 0;
        configPanel.y = this.layout.height - 30;
        const panelBackground = new PIXI.Graphics();
        panelBackground.beginFill(0x012d36);
        panelBackground.drawRect(0, 0, this.layout.width, 30);
        panelBackground.endFill();
        configPanel.addChild(panelBackground);
        this.app.stage.addChild(configPanel);

        configPanel.payTableButton = new Button({
            x: this.layout.width - 300,
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
            x: this.layout.width - 150,
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

    createMarginPanels() {
        if (this.layout.topPanelHeight)
            this.topPanel = new Panel({
                x: this.phaseContainer.x,
                y: this.phaseContainer.y - this.layout.topPanelHeight,
                width: this.phaseContainer.maxWidth,
                height: this.layout.topPanelHeight,
                colour: this.colours.topPanelColour,
                parent: this.app.stage
            });

        if (this.layout.bottomPanelHeight)
            this.bottomPanel = new Panel({
                x: this.phaseContainer.x,
                y: this.phaseContainer.y + this.phaseContainer.maxHeight,
                width: this.phaseContainer.maxWidth,
                height: this.layout.bottomPanelHeight,
                colour: this.colours.bottomPanelColour,
                parent: this.app.stage
            })

        if (this.layout.leftPanelWidth)
            this.leftPanel = new Panel({
                x: 0,
                y: this.phaseContainer.y,
                width: this.layout.leftPanelWidth,
                height: this.phaseContainer.maxHeight,
                colour: this.colours.leftPanelColour,
                parent: this.app.stage
            })

        if (this.layout.rightPanelWidth)
            this.rightPanel = new Panel({
                x: this.phaseContainer.maxWidth + this.phaseContainer.x,
                y: this.phaseContainer.y,
                width: this.layout.rightPanelWidth,
                height: this.phaseContainer.maxHeight,
                colour: this.colours.rightPanelColour,
                parent: this.app.stage
            })
    }

    createPhaseContainer() {
        const container = new PIXI.Container();
        container.x = this.layout.leftPanelWidth;
        container.y = this.layout.titleHeight + this.layout.topPanelHeight;
        container.maxHeight = this.layout.maxPhaseHeight;
        container.maxWidth = this.layout.maxPhaseWidth;
        this.app.stage.addChild(container);
        this.phaseContainer = container;
    }

    buildPhaseUIs(soundPlayer, imageMap) {
        Object.values(this.phases).forEach(phase => {
            phase.build(this.maths, this.phaseContainer, this.bottomTickerContainer, soundPlayer, this.colours.tickerTextColour ?? 0x000000, imageMap);
        });
    }

    updateUiAndStateToMatchSession(session, state) {
        state.bet = session.bet;
        state.betType = session.betType;
        state.rtp = session.rtp;
        this.configPanel.betSelector.updateBet(this.state.bet);
    }
}
