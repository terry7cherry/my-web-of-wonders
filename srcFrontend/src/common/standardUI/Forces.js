import InfoPanel from "./InfoPanel.js"
import Button from "../uiElements/Button.js"
import ScrollableContainer from "./ScrollableContainer.js";

const COLUMNS = 2;
const BUTTON_HEIGHT = 50;
const BUTTON_GAP = 25;   // horizontal gap between columns & to the edges
const ROW_HEIGHT = 75;
const TOP_OFFSET = 75;

export default class Forces extends InfoPanel {
    constructor({state, app, x, y, width, height}) {
        super({state, app, x, y, width, height});
    }

    forcesSetup(playForce, forceOptions) {
        this.reset();
        this.forceButtons = [];
        this._setupScrollArea();
        this.establishFrontEndForces();
        this.establishBetTypeForces(playForce, forceOptions);
        this._scrollArea.refresh();
    }

    _setupScrollArea() {
        const SCROLLBAR_WIDTH = 12;
        // Leave TOP_OFFSET at the top for the close button, a small bottom margin
        const scrollHeight = this.height - TOP_OFFSET - BUTTON_GAP;
        this._scrollArea = new ScrollableContainer({
            x: 0,
            y: TOP_OFFSET,
            width: this.width,
            height: scrollHeight,
            parent: this.container,
        });

        // Button width: split available width into 2 columns with gaps
        // Available = total width - scrollbar - left/right gaps - gap between columns
        this._buttonWidth = (this.width - SCROLLBAR_WIDTH - BUTTON_GAP * 3) / COLUMNS;
    }

    establishFrontEndForces() {
        this.addForce({text: "Balance + £100", action: () => this.addMoney(10000)});
        this.addForce({text: "Balance = £1,000", action: () => this.setMoney(100000)});
    }

    establishBetTypeForces(playForce, forceOptions) {
        forceOptions.forEach(({index, name}) =>
            this.addForce({
                text: name,
                hideInfoPanel: true,
                action: () => playForce(index)
            })
        )
    }

    addForce({text, action, hideInfoPanel}) {
        const forceNum = this.forceButtons.length;
        const row = Math.floor(forceNum / COLUMNS);
        const col = forceNum % COLUMNS;

        const x = BUTTON_GAP + col * (this._buttonWidth + BUTTON_GAP);
        const y = BUTTON_GAP + row * ROW_HEIGHT;

        const button = new Button({
            x,
            y,
            width: this._buttonWidth,
            height: BUTTON_HEIGHT,
            colour: 0xffbbbb,
            textColour: 0x000000,
            text,
            parent: this._scrollArea.content,
            onClick: () => {
                if (hideInfoPanel) this.hide();
                action();
            }
        });

        this.forceButtons.push(button);
    }

    addMoney(value) {
    }

    setMoney(value) {
    }
}
