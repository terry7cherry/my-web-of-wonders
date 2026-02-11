import * as PIXI from 'pixi.js';

export default class Panel extends PIXI.Container {
    constructor({x, y, width, height, colour, parent}) {
        super();
        this.x = x ?? 0;
        this.y = y ?? 0;
        this.shape = {
            width, height
        };
        parent.addChild(this);
        if (typeof colour !== "undefined") this.addChild(this.createPanelBackground(width, height, colour))
    }

    createPanelBackground(width, height, colour) {
        const bg = new PIXI.Sprite(PIXI.Texture.WHITE);
        bg.width = width;
        bg.height = height;
        bg.tint = colour;
        return bg;
    }
}