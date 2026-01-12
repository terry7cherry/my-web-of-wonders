import * as PIXI from "pixi.js";
import MaskedContainer from "./MaskedContainer.js";
import Button from "./Button.js";

export default class InfoPanel {
    constructor({game, app, x, y, width, height}) {

        this.game = game;
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
            color: 0xffffff,
            textColor: 0x000000,
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
        this.game.running = false;
    }

    show() {
        if (this.game.running) return;

        this.container.visible = true;
        this.game.running = true;
    }
}
