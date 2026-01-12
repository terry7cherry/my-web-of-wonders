import * as PIXI from "pixi.js";
import MaskedContainer from "./MaskedContainer.js";

export default class NiceText {
  constructor({ app, text, width, height, x, y, textFormat, textColor }) {
    this.textColor = textColor;
    this.textFormat = textFormat ?? {
      fontFamily: "Arial",
      fontSize: 60,
      fontWeight: "bold",
      fill: this.textColor,
      stroke: "#000000",
      strokeThickness: 5,
      dropShadow: true,
      dropShadowColor: "#000000",
      dropShadowBlur: 4,
      dropShadowAngle: Math.PI / 6,
      dropShadowDistance: 6,
    };
    this.text = text;
    this.width = width;
    this.height = height;
    this.container = new MaskedContainer({app, x, y, width, height});
    this.updateText(text);
  }

  updateText = (text) => {
    this.text = text;
    const textObject = new PIXI.Text(text, this.textFormat);

    const scaleX = this.width / textObject.width;
    const scaleY = this.height / textObject.height;
    const scale = Math.min(scaleX, scaleY);

    textObject.scale.set(scale, scale);
    const x = (this.width - textObject.width) / 2;
    const y = (this.height - textObject.height) / 2;
    const pause = text.length * this.pauseScale;

    this.setText(textObject, x, y);
  };

  updateColor = (color) => {
    this.textFormat.fill = color;
    const textObject = new PIXI.Text(this.text, this.textFormat);

    const scaleX = this.width / textObject.width;
    const scaleY = this.height / textObject.height;
    const scale = Math.min(scaleX, scaleY);

    textObject.scale.set(scale, scale);
    const x = (this.width - textObject.width) / 2;
    const y = (this.height - textObject.height) / 2;
    const pause = this.text.length * this.pauseScale;

    this.setText(textObject, x, y);
  };

  setText(textObject, x, y) {
    textObject.x = x;
    textObject.y = y;
    this.container.removeChildren();
    if (this.currentText) this.currentText.destroy({ children: true, texture: true, baseTexture: true });
    this.container.addChild(textObject);
    this.currentText = textObject;
  }

  setVisible(visible) {
    this.container.visible = visible;
  }
}
