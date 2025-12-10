import * as PIXI from "pixi.js";

export default class Button {
  constructor({ x, y, width, height, text, onClick, app, color, parent, textColor }) {
    const buttonContainer = new PIXI.Container();
    this.buttonContainer = buttonContainer;

    // Create the button background using PIXI.Graphics
    const buttonBackground = new PIXI.Graphics();
    this.buttonBackground = buttonBackground;
    this.buttonWidth = width;
    this.buttonHeight = height;
    buttonBackground.beginFill(color ?? 0x0000ff); // Red color
    buttonBackground.drawRect(0, 0, this.buttonWidth, this.buttonHeight);
    buttonBackground.endFill();

    // Create the button text using PIXI.Text
    this.buttonText = new PIXI.Text(text, {
      fontFamily: "Arial",
      fontSize: 24,
      fill: textColor ?? 0xffffff,
      align: "center",
    });

    // Center the text within the button background
    this.buttonText.x = (this.buttonWidth - this.buttonText.width) / 2;
    this.buttonText.y = (this.buttonHeight - this.buttonText.height) / 2;

    // Add the text and background to the button container
    buttonContainer.addChild(buttonBackground);
    buttonContainer.addChild(this.buttonText);

    // Position the button on the screen
    buttonContainer.x = x;
    buttonContainer.y = y;

    // Add the button container to the stage
    if(parent) parent.addChild(buttonContainer);
    else app.stage.addChild(buttonContainer);

    // Make the button interactive
    buttonContainer.eventMode = "static";
    buttonContainer.buttonMode = true; // Shows a hand cursor when hovering

    // Add interactivity (e.g., click event)
    this.onClick = onClick;
    buttonContainer.on("pointerdown", this.onClick);
  }

  click() {
    this.onClick();
  }

  changeColor(color) {
    this.buttonBackground.beginFill(color); // Red color
    this.buttonBackground.drawRect(0, 0, this.buttonWidth, this.buttonHeight);
    this.buttonBackground.endFill();
  }

  setVisible(visible) {
    this.buttonContainer.visible = visible;
  }

  updateText(text) {
    this.buttonText.text = text;
    this.buttonText.x = (this.buttonWidth - this.buttonText.width) / 2;
    this.buttonText.y = (this.buttonHeight - this.buttonText.height) / 2;
  }
}
