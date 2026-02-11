import * as PIXI from "pixi.js";
import MaskedContainer from "./MaskedContainer.js";
import scaleAndCentreObject from "../utils/scaleAndCentreObject.js";

export default class NiceText {
    constructor({app, parent, text, width, height, x, y, textFormat, textColour}) {
        this.textColour = textColour;
        this.textFormat = textFormat ?? {
            fontFamily: "Arial",
            fontSize: 60,
            fontWeight: "bold",
            fill: this.textColour ?? "#ffffff",
            stroke: "#000000",
            strokeThickness: 5,
            dropShadow: true,
            dropShadowColour: "#000000",
            dropShadowBlur: 4,
            dropShadowAngle: Math.PI / 6,
            dropShadowDistance: 6,
        };
        this.text = text;
        this.width = width;
        this.height = height;
        this.container = new MaskedContainer({app, parent, x, y, width, height});
        this.updateText(text);
    }

    updateText = (text) => {
        this.text = text;
        const textObject = new PIXI.Text(text, this.textFormat);

        scaleAndCentreObject(textObject, this.width, this.height);

        this.setText(textObject);
    };

    updateColour = (colour) => {
        this.textFormat.fill = colour;
        const textObject = new PIXI.Text(this.text, this.textFormat);

        scaleAndCentreObject(textObject, this.width, this.height);

        this.setText(textObject);
    };

    setText(textObject) {
        this.container.removeChildren();
        if (this.currentText) this.currentText.destroy({children: true, texture: true, baseTexture: true});
        this.container.addChild(textObject);
        this.currentText = textObject;
    }

    setVisible(visible) {
        this.container.visible = visible;
    }
}
