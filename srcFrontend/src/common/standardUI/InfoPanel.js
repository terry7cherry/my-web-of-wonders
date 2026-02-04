import * as PIXI from "pixi.js";
import MaskedContainer from "../uiElements/MaskedContainer.js";
import Button from "../uiElements/Button.js";

export default class InfoPanel {
    constructor({state, app, x, y, width, height}) {

        this.state = state;
        this.width = width;
        this.height = height;
        this.container = new MaskedContainer({app, x, y, width, height});
        this.reset();

        this.container.visible = false;
    }

    reset() {
        this.container.removeChildren().forEach(child => child.destroy());
        this.bg = this.createBackground(this.container, this.width, this.height);
        this.closeButton = new Button({
            x: 0,
            y: 0,
            width: 50,
            height: 50,
            colour: 0xffffff,
            textColour: 0x000000,
            text: "X",
            parent: this.container,
            onClick: () => {
                this.hide();
            }
        })
    }

    createBackground(container, width, height) {
        const bg = new PIXI.Graphics();
        bg.beginFill(0x000000);
        bg.alpha = 0.8;
        bg.drawRect(0, 0, width, height);
        bg.endFill();
        container.addChild(bg);
        return bg;
    }

    hide() {
        this.container.visible = false;
        this.state.running = false;
    }

    show() {
        if (this.state.running) return;

        this.container.visible = true;
        this.state.running = true;
    }
}
