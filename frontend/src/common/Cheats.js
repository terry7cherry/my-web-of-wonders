import InfoPanel from "./InfoPanel.js"
import Button from "./Button.js"

export default class Cheats extends InfoPanel {
    constructor({ game, app, x, y, width, height }) {
        super({ game, app, x, y, width, height });
        this.cheatsPerCol = Math.floor((this.height - 75) / 75);
        this.cheatsSetup();
    }

    cheatsSetup() {
        this.reset();
        this.cheatButtons = [];
        this.establishFrontEndCheats();
        //this.establishBetTypeCheats();
    }

    establishFrontEndCheats() {
        this.addCheat({ text: "Balance + £100", action: () => this.addMoney(10000) });
        this.addCheat({ text: "Balance = £1,000", action: () => this.setMoney(100000) });
    }

    establishBetTypeCheats() {
        this.modes = this.game.betTypesAndModes[this.game.currentBetType];
        this.betTypeCheatsButtons = new Array();
        this.modes.forEach((modeName) => {
            this.addCheat({ text: "Trigger " + modeName, hideInfoPanel: true, action: () => this.game.play(modeName) })
        })
    }

    addCheat({ text, action, hideInfoPanel }) {
        const cheatNum = this.cheatButtons.length;
        const col = Math.floor(cheatNum / this.cheatsPerCol);
        const row = cheatNum % this.cheatsPerCol;
        const button =
            new Button({
                x: 75 + col * 225,
                y: 75 + row * 75,
                width: 200,
                height: 50,
                color: 0xffbbbb,
                textColor: 0x000000,
                text,
                parent: this.container,
                onClick: () => {
                    if (hideInfoPanel) this.hide();
                    action();
                }
            })

        this.cheatButtons.push(button);
    }


    addMoney(value) {
        this.game.balance += value;
        this.game.updateBalance();
    }

    setMoney(value) {
        this.game.balance = value;
        this.game.updateBalance();
    }
}
