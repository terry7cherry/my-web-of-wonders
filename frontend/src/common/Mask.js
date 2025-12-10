import * as PIXI from "pixi.js";

export default class Mask extends PIXI.Graphics {
    constructor({parent, x, y, width, height}) {
        super();
        this.beginFill(0xffffff);
        this.parentGlobalPosition = parent?.getGlobalPosition() ?? {x:0,y:0};
        this.drawRect(
            this.parentGlobalPosition.x + x,
            this.parentGlobalPosition.y + y,
            width,
            height
        );
        this.endFill();
    }
}