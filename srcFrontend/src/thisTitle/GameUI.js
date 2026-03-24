import * as PIXI from "pixi.js";
import StandardGameUI from "../common/standardUI/StandardGameUI.js"
import PhaseUI from "../common/uiElements/PhaseUI.js";
import Image from "../common/uiElements/Image.js";
import Button from "../common/uiElements/Button.js";
import NiceText from "../common/uiElements/NiceText.js";
import Panel from "../common/uiElements/Panel.js";
import MaskedContainer from "../common/uiElements/MaskedContainer.js";
import Ticker from "../common/uiElements/Ticker.js";
import collectorPositions from "./collectorPositions.js";
import moveElement from "../common/animation/moveElement.js";

/**
 * When editing the constructor parameters for this class, particularly the layout, colours, and gridOptions objects,
 * help is available by pressing ctrl+space with the cursor in the curly brackets. This will show a list of all available options.
 * To get more information on a specific option, press enter to insert it, then type a colon after it (so CLion knows it is a property).
 * Then hover over the property name or press ctrl+Q while the cursor is on it to see the description.
 */
export default class GameUI extends StandardGameUI {
    constructor(maths) {
        super(
            {
                // Uncomment these and edit the values to add outer panels to the game UI.
                // These panels will be present throughout all phases of the game.
                topPanelHeight: 80,
                leftPanelWidth: 80,
                rightPanelWidth: 80,
                bottomPanelHeight: 80,
            },
            {
                // Uncomment these to set the colours of panels created above.
                // topPanelColour: 0xFFFFFF,
                // bottomPanelColour: 0xFFFFFF,
                // leftPanelColour: 0xFFFFFF,
                // rightPanelColour: 0xFFFFFF,
            },
            {
                "base": new PhaseUI(
                    "Base Game",
                    // If hasSpins is true, a "spins left" counter will be shown at the bottom of the screen.
                    false,
                    {
                        // Uncomment these and edit the values to create phase UI panels.
                        // These panels will only be present during this phase.
                        symbolHeight: 80,
                        reelWidth: 80
                    },
                    {
                        // Changes the colour of the entire background for the duration of this phase.
                        backgroundColour: 0x1099bb,
                        // Uncomment these to set the colours of panels created above.
                        // topPanelColour: 0xFFFFFF,
                        // bottomPanelColour: 0xFFFFFF,
                        // leftPanelColour: 0xFFFFFF,
                        // rightPanelColour: 0xFFFFFF,
                    },
                    {
                        displayReelSet: maths.displayReelSet,
                        suspenseSymbol: maths.suspenseSymbol,
                        // isHoldAndSpin: true,
                        // holdAndSpinBlankSymbols: maths.holdAndSpinBlankSymbols,
                        // weightedDisplayReelSet: maths.holdAndSpinDisplayReelSet,
                        // simultaneousWinLineAnimations: true,

                    }),
                "freeSpins": new PhaseUI(
                    "Free Spins",
                    true,
                    {
                        symbolHeight: 80,
                        reelWidth: 80
                    },
                    {backgroundColour: 0x11a878},
                    {
                        suspenseSymbol: maths.suspenseSymbol,
                        displayReelSet: maths.displayReelSet
                    }),
            },
        );
        this.maths = maths;
    }

    async init(dependencies) {
        await super.init(dependencies);
        const mainCollectorTexture = this.imageMap.mainCollector;
        this.mainCollector = new Image({
            texture: mainCollectorTexture,
            width: 80,
            height: 80,
            x: -100,
            y: -100,
            parent: this.app.stage,
        });
        this.mainCollector.originalScale = {
            x: this.mainCollector.sprite.scale.x,
            y: this.mainCollector.sprite.scale.y,
        }
        this.mainCollector.originalWidth = this.mainCollector.sprite.width;
        this.mainCollector.originalHeight = this.mainCollector.sprite.height;
        //this.mainCollector = new PIXI.Sprite(mainCollectorTexture);
        //this.mainCollector.size = 100;
        //this.mainCollector.scale.set(
        //    this.mainCollector.size / this.mainCollector.width,
        //    this.mainCollector.size / this.mainCollector.height
        //);
        //this.mainCollector.x = -this.mainCollector.size;
        //this.mainCollector.y = -this.mainCollector.size;
        //this.app.stage.addChild(this.mainCollector);
        // Add UI elements to this.topPanel, this.bottomPanel, this.leftPanel, or this.rightPanel here. E.g.
        // this.topPanel.text = new NiceText({
        //     width: 800,
        //     height: 50,
        //     text: "Top Test",
        //     colour: 0x000000,
        //     parent: this.topPanel
        // });
        // (You must ensure you have created the panel by passing a height/width for it in the layout property for the StandardGameUI constructor.)
    }

    buildPhaseUIs(soundPlayer, imageMap) {
        super.buildPhaseUIs(soundPlayer, imageMap);
        this.phases.freeSpins.secondaryControlAreas = [];
        // Add UI elements to phase panels here. E.g.
        // this.phases.base.bottomPanel.testButton = new Button({
        //     width: 800,
        //     height: 50,
        //     y: 25,
        //     text: "Bottom Test",
        //     colour: 0x000000,
        //     parent: this.phases.base.bottomPanel
        // });
        // (You must ensure you have created the panel by passing a height/width for it in the layout property for the relevant PhaseUI constructor.)
    }

    async updateUiAndStateToMatchSession(session, state) {
        await super.updateUiAndStateToMatchSession(session, state);
        if (state.mainCollectorPosition !== session.mainCollectorPosition) {
            state.mainCollectorPosition = session.mainCollectorPosition;
            let {
                x,
                y,
                controlX,
                controlY
            } = collectorPositions(this, this.mainCollector)[state.mainCollectorPosition];
            await moveElement({element: this.mainCollector, newX: x, newY: y, totalFrames: 50});
            if (!this.mainControlArea) {
                this.mainControlArea = new Button({
                    x: 0, y: 0, width: 240, height: 240, parent: this.currentPhase.grid.reelBackground,
                    colour: 0x4B0082,
                })
                this.mainControlArea.buttonContainer.zIndex = 10;
            }

            this.mainControlArea.buttonContainer.x = controlX;
            this.mainControlArea.buttonContainer.y = controlY;
        }
        // Use this method to update the UI to match the current session.
        // Where you have persistent game elements reflected in the UI, you will update the UI elements when
        // events trigger that should change them, and your changes will persist between spins.
        // However, there are two situations where this falls down:
        //  - When the user changes the bet type (or the rtp), the UI reloads and all UI elements reset.
        //    This is fine if persistence is locked to the bet type (so changing the bet type resets all
        //    persistent values) but this may not always be the case. This method allows you to resolve that
        //    problem. Before the UI is reloaded, the frontend requests a new session from the backend based
        //    on the current one and using the correct bet locking rules for the game. This method is then
        //    called with the new session, so you can add code here to update the UI to match the new session.
        //  - When running using RN stream in the backend on a game with persistence, you may want to play
        //    a particular problem game in the frontend to help diagnose the problem. The backend gives you a
        //    string containing the session details and RN stream that you can enter in the frontend. However,
        //    if you have persistent UI elements, they will not match the correct starting state for the
        //    problem game cycle. Again this method will be triggered before the game starts and allows you
        //    to update the UI to match the problem game.
        // Note if you are storing anything important on the state object you can also update that to match
        // the session here.
    }
}