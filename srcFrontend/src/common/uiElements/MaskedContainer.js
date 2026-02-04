import * as PIXI from "pixi.js";
import Mask from "./Mask.js";

export default class MaskedContainer extends PIXI.Container {
    constructor({app, parent, x, y, width, height}) {
        super();
        this.x = x ?? 0;
        this.y = y ?? 0;
        this.mask = new Mask({parent, x: this.x, y: this.y, width, height});
        if (app)
            app.stage.addChild(this);
        else
            parent.addChild(this);
    }
}