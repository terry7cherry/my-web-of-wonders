import * as PIXI from "pixi.js";
import MaskedContainer from "./MaskedContainer.js";
import moveElement from "./animation/moveElement.js";

export default class Ticker {
    constructor({ app, width, height, x, y, pauseScale, textFormat, textColor, historyTextBox }) {
        this.textColor = textColor;
        this.historyTextBox = historyTextBox;
        this.textFormat = textFormat ?? {
            fontFamily: "Arial",
            fontSize: 100,
            fill: this.textColor ?? "#000000",
        };
        this.width = width;
        this.height = height;
        this.pauseScale = pauseScale;
        this.container = new MaskedContainer({ app, x, y, width, height });
    }

    setVisible(visible) {
        this.container.visible = visible;
    }

    updateText = async (text, instant, pause, noHistoryEntry, color) => {
        if (this.historyTextBox && !noHistoryEntry && text != "⠀") this.historyTextBox.addText(text);
        const textFormat = { ...this.textFormat };
        if (color) textFormat.fill = color;
        const textObject = new PIXI.Text(text, textFormat);

        const scaleX = this.width / textObject.width;
        const scaleY = this.height / textObject.height;
        const scale = Math.min(scaleX, scaleY);

        textObject.scale.set(scale, scale);
        const x = (this.width - textObject.width) / 2;
        const y = (this.height - textObject.height) / 2;
        if (instant) await this.setText(textObject, x, y, pause);
        else {
            pause = pause ?? text.length * this.pauseScale;
            if (!this.currentText) await this.setText(textObject, x, y, pause);
            else await this.scrollText(textObject, x, y, pause);
        }
    };

    setText(textObject, x, y, pause) {
        return new Promise((resolve, reject) => {
            textObject.x = x;
            textObject.y = y;
            this.container.removeChildren(0);
            if (this.currentText) this.currentText.destroy({ children: true, texture: true, baseTexture: true });
            this.container.addChild(textObject);
            this.currentText = textObject;
            this.wait(resolve, pause);
        });
    }

    scrollText(newText, x, y, pause) {
        return new Promise((resolve, reject) => {
            newText.x = x;
            newText.y = y + this.height;
            this.container.addChild(newText);
            const animateOldText = this.currentText ? moveElement({
                element: this.currentText,
                newX: this.currentText.x,
                newY: this.currentText.y - this.height,
                pixelsPerFrame: 4
            }) : Promise.resolve();
            const animateNewText = moveElement({
                element: newText,
                newX: x,
                newY: y,
                pixelsPerFrame: 4
            });
            Promise.all([animateOldText, animateNewText]).then(() => {
                this.container.removeChild(this.currentText);
                this.currentText.destroy({ children: true, texture: true, baseTexture: true });
                this.currentText = newText;
                this.wait(resolve, pause);
            })
        });
    }

    wait(resolve, pause) {
        const pauseInFrames = pause / 1000 * 60;
        let elapsedFrames = 0;
        const tick = (delta) => {
            elapsedFrames += delta;
            if (elapsedFrames >= pauseInFrames) {
                PIXI.Ticker.shared.remove(tick);
                resolve();
            }
        }
        PIXI.Ticker.shared.add(tick);
    }

    clearText = async () => {
        await this.updateText("⠀");
    };
}
