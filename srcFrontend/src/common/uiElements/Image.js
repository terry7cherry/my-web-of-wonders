import * as PIXI from "pixi.js";

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
        this.sprite = new PIXI.Sprite(texture);
        const scale = Math.min(this.targetWidth / this.sprite.width, this.targetHeight / this.sprite.height);
        this.sprite.scale.set(scale, scale);
        this.sprite.x = (this.targetWidth - this.sprite.width) / 2;
        this.sprite.y = (this.targetHeight - this.sprite.height) / 2;
        this.addChild(this.sprite);
        this.x = x;
        this.y = y;
        parent.addChild(this);
    }
}
