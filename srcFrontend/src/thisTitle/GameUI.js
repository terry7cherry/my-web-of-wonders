import StandardGameUI from "../common/standardUI/StandardGameUI.js"
import PhaseUI from "../common/uiElements/PhaseUI.js";

/**
 * When editing the constructor parameters for this class, particularly the layout, colours, and gridOptions objects,
 * help is available by pressing ctrl+space with the cursor in the curly brackets. This will show a list of all available options.
 * To get more information on a specific option, press enter to insert it, then type a colon after it (so CLion knows it is a property).
 * Then hover over the property name or press ctrl+Q while the cursor is on it to see the description.
 */
export default class GameUI extends StandardGameUI {
    constructor(maths) {
        super({
                "base": new PhaseUI(
                    "Base Game",
                    false,
                    {},
                    {backgroundColour: 0x1099bb},
                    {displayReelSet: maths.displayReelSet}),
                "freeSpins": new PhaseUI(
                    "Free Spins",
                    true,
                    {},
                    {backgroundColour: 0x11a878},
                    {displayReelSet: maths.displayReelSet}),
            },
            {},
            {}
        )
    }
}