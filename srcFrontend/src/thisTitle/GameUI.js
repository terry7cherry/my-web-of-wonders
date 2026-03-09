import * as PIXI from "pixi.js";
import StandardGameUI from "../common/standardUI/StandardGameUI.js"
import PhaseUI from "../common/uiElements/PhaseUI.js";
import Image from "../common/uiElements/Image.js";
import Button from "../common/uiElements/Button.js";
import NiceText from "../common/uiElements/NiceText.js";
import Panel from "../common/uiElements/Panel.js";
import MaskedContainer from "../common/uiElements/MaskedContainer.js";
import Ticker from "../common/uiElements/Ticker.js";

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
                // topPanelHeight: 100,
                // leftPanelWidth: 100,
                // rightPanelWidth: 100,
                // bottomPanelHeight: 100,
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
                        // topPanelHeight: 100,
                        // leftPanelWidth: 100,
                        // rightPanelWidth: 100,
                        // bottomPanelHeight: 100
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
                    // If hasSpins is true, a "spins left" counter will be shown at the bottom of the screen.
                    true,
                    {},
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

    updateUiAndStateToMatchSession(session, state) {
        super.updateUiAndStateToMatchSession(session, state);
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