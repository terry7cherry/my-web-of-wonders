import * as PIXI from "pixi.js";

import Mask from "./Mask.js";
import moveElement from "../animation/moveElement.js";

export default class Image extends PIXI.Container {
    constructor({
                    texture,
                    height,
                    width,
                    x,
                    y,
                    parent
                }) {
        super();
        this.targetHeight = height;
        this.targetWidth = width;
        this.sprite = this.getScaledSprite(texture);
        this.imageContainer = new PIXI.Container();
        this.addChild(this.imageContainer);
        this.imageContainer.addChild(this.sprite);
        this.x = x;
        this.y = y;
        this.overlayContainer = new PIXI.Container();
        this.addChild(this.overlayContainer);
        parent.addChild(this);
    }

    transformImage(newTexture, speed = 4) {
        return new Promise((resolve) => {
            const mask = new Mask({
                parent: this.parent,
                x: this.x,
                y: this.y,
                width: this.targetWidth,
                height: this.targetHeight
            });
            const newSprite = this.getScaledSprite(newTexture);
            newSprite.x = this.sprite.x;
            newSprite.y = this.sprite.y - this.targetHeight;
            newSprite.mask = mask;
            this.sprite.mask = mask;
            this.imageContainer.addChild(newSprite);
            const animateOldImage = moveElement({
                    element: this.sprite,
                    newX: this.sprite.x,
                    newY: this.sprite.y + this.targetHeight,
                    pixelsPerFrame: speed
                }
            )
            const animateNewImage = moveElement({
                    element: newSprite,
                    newX: newSprite.x,
                    newY: newSprite.y + this.targetHeight,
                    pixelsPerFrame: speed
                }
            )
            Promise.all([animateOldImage, animateNewImage]).then(() => {
                this.imageContainer.removeChild(this.sprite);
                this.sprite.destroy({children: true});
                this.sprite = newSprite;
                this.sprite.mask.destroy({children: true});
                this.sprite.mask = null;
                resolve();
            })
        })
    }

    getScaledSprite(texture) {
        const sprite = new PIXI.Sprite(texture);
        const scale = Math.min(this.targetWidth / sprite.width, this.targetHeight / sprite.height);
        sprite.scale.set(scale, scale);
        sprite.x = (this.targetWidth - sprite.width) / 2;
        sprite.y = (this.targetHeight - sprite.height) / 2;
        return sprite;
    }

    addOrUpdateText(text, textStyle) {
        this.removeText();
        textStyle = {
            fontFamily: "Arial",
            fontSize: 32,
            fontWeight: "bold",
            fill: "#FFFFFF",
            stroke: "#000000",
            strokeThickness: 5,
            dropShadow: true,
            dropShadowColour: "#000000",
            dropShadowBlur: 4,
            dropShadowAngle: Math.PI / 6,
            dropShadowDistance: 3,
            align: "center",
            ...textStyle
        }
        const textObject = new PIXI.Text(text, textStyle);
        const scale = Math.min(
            (textStyle.width ?? this.targetWidth) / textObject.width,
            (textStyle.height ?? this.targetHeight) / textObject.height,
        )
        textObject.scale.set(scale, scale);
        textObject.x = textStyle.x ?? (this.targetWidth - textObject.width) / 2;
        textObject.y = textStyle.y ?? (this.targetHeight - textObject.height);
        this.overlayContainer.addChild(textObject)
        this.overlayText = textObject;
    }

    removeText() {
        if (this.overlayText) {
            this.overlayContainer.removeChild(this.overlayText);
            this.overlayText.destroy({children: true});
            this.overlayText = null;
        }
    }
}
