import StandardGameUI from "../common/standardUI/StandardGameUI.js"
import PhaseUI from "../common/uiElements/PhaseUI.js";
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
}