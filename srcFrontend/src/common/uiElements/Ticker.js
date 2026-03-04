import * as PIXI from "pixi.js";
import MaskedContainer from "./MaskedContainer.js";
import moveElement from "../animation/moveElement.js";
import scaleAndCentreObject from "../utils/scaleAndCentreObject.js";

export default class Ticker {
    constructor({app, parent, width, height, x, y, pauseScale, textFormat, textColour, historyTextBox}) {
        this.textColour = textColour;
        this.historyTextBox = historyTextBox;
        this.textFormat = textFormat ?? {
            fontFamily: "Arial",
            fontSize: 100,
            fill: this.textColour ?? "#000000",
        };
        this.width = width;
        this.height = height;
        this.pauseScale = pauseScale ?? 50;
        this.container = new MaskedContainer({app, parent, x, y, width, height});
        this.value = 0;
        this.isAnimating = false;
    }

    setVisible(visible) {
        this.container.visible = visible;
    }

    updateTextInstant = (text, colour) => {
        const textFormat = {...this.textFormat};
        if (colour) textFormat.fill = colour;
        const textObject = new PIXI.Text(text, textFormat);
        scaleAndCentreObject(textObject, this.width, this.height);
        this.container.removeChildren(0);
        if (this.currentText) this.currentText.destroy({
            children: true,
            texture: true,
            baseTexture: true
        });
        this.container.addChild(textObject);
        this.currentText = textObject;
    }

    updateText = async (text, instant, pause, noHistoryEntry, colour) => {
        if (this.historyTextBox && !noHistoryEntry && text !== "⠀") this.historyTextBox.addText(text);
        const textFormat = {...this.textFormat};
        if (colour) textFormat.fill = colour;
        textFormat.align = "center";
        const textObject = new PIXI.Text(text, textFormat);
        scaleAndCentreObject(textObject, this.width, this.height);
        if (instant) {
            await this.setText(textObject, pause ?? 0);
        } else {
            pause = pause ?? Math.pow(text.length, 1 / 2) * this.pauseScale;
            if (!this.currentText) await this.setText(textObject, pause);
            else await this.scrollText(textObject, pause);
        }
    };

    setText(textObject, pause) {
        return new Promise((resolve, reject) => {
            this.container.removeChildren(0);
            if (this.currentText) this.currentText.destroy({
                children: true,
                texture: true,
                baseTexture: true
            });
            this.container.addChild(textObject);
            this.currentText = textObject;
            Ticker.wait(resolve, pause);
        });
    }

    waitUntilFalse(predicate, interval = 0) {
        function tick() {
            if (!predicate()) {
                return Promise.resolve();
            }

            return new Promise(resolve =>
                setTimeout(resolve, interval)
            ).then(tick);
        }

        return tick();
    }

    scrollText(newText, pause) {
        return new Promise((resolve, reject) => {
            newText.y = newText.y + this.height;
            this.container.addChild(newText);
            this.waitUntilFalse(() => this.isAnimating);
            this.isAnimating = true;
            const animateOldText = this.currentText ? moveElement({
                element: this.currentText,
                newX: this.currentText.x,
                newY: this.currentText.y - this.height,
                pixelsPerFrame: 4
            }) : Promise.resolve();
            const animateNewText = moveElement({
                element: newText,
                newX: newText.x,
                newY: newText.y - this.height,
                pixelsPerFrame: 4
            });
            Promise.all([animateOldText, animateNewText]).then(() => {
                this.container.removeChild(this.currentText);
                this.currentText.destroy({children: true, texture: true, baseTexture: true});
                this.currentText = newText;
                this.isAnimating = false;
                Ticker.wait(resolve, pause);
            })
        });
    }


    static wait(resolve, pause, frames) {
        const pauseInFrames = frames ? pause : pause / 1000 * 60;
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


    clearText = async (instant) => {
        await this.updateText("⠀", instant);
    };

    updateInstant(number) {
        this.value = number;
        this.updateTextInstant(this.formatValue(this.value));
    }

    async update(number, instant) {
        this.value = number;
        await this.updateText(this.formatValue(this.value), instant);
    }

    formatValue(value) {
        return value;
    }

    async increment(number) {
        await this.update(this.value + number);
    }
}
