import InfoPanel from "./InfoPanel.js"
import Button from "../uiElements/Button.js"

export default class Forces extends InfoPanel {
    constructor({state, app, x, y, width, height}) {
        super({state, app, x, y, width, height});
        this.forcesPerCol = Math.floor((this.height - 75) / 75);
    }

    forcesSetup(playForce, forceOptions) {
        this.reset();
        this.forceButtons = [];
        this.establishFrontEndForces();
        this.establishBetTypeForces(playForce, forceOptions);
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
        const col = Math.floor(forceNum / this.forcesPerCol);
        const row = forceNum % this.forcesPerCol;
        const button =
            new Button({
                x: 75 + col * 225,
                y: 75 + row * 75,
                width: 200,
                height: 50,
                colour: 0xffbbbb,
                textColour: 0x000000,
                text,
                parent: this.container,
                onClick: () => {
                    if (hideInfoPanel) this.hide();
                    action();
                }
            })

        this.forceButtons.push(button);
    }

    addMoney(value) {
    }

    setMoney(value) {
    }
}
