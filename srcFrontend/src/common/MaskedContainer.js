import * as PIXI from "pixi.js";
import Mask from "./Mask.js";

export default class MaskedContainer extends PIXI.Container {
    constructor({app, parent, x, y, width, height}) {
        super();
        this.x = x;
        this.y = y;
        this.mask = new Mask({parent, x, y, width, height});
        if(app) {
            app.stage.addChild(this);
        } else {
            parent.addChild(this);
        }
    }
}