import displayReelSet from "./display_reels.json" with { type: "json" };

import Game from "./CustomGame.js";
import * as PIXI from "pixi.js";

import wildSym from "./symbols/wild.png";
import scatterSym from "./symbols/scatter.png";
import hp1Sym from "./symbols/HP1.png";
import hp2Sym from "./symbols/HP2.png";
import hp3Sym from "./symbols/HP3.png";
import hp4Sym from "./symbols/HP4.png";
import lp1Sym from "./symbols/LP1.png";
import lp2Sym from "./symbols/LP2.png";
import lp3Sym from "./symbols/LP3.png";
import lp4Sym from "./symbols/LP4.png";

document.addEventListener("DOMContentLoaded", async () => {
    const game = await new Game({
        // Note Game constructor can take numerous other parameters to overwrite default appearance.
        gameName: "Example game",
        displayReelSet, // reels to use when loading page and spinning reels.
        // symbolMap allows to display symbol text other than that delivered by server.
        symbolMap: {
            0: "WI",
            1: "H1",
            2: "H2",
            3: "H3",
            4: "H4",
            5: "L1",
            6: "L2",
            7: "L3",
            8: "L4",
            9: "SC",
        },
        // Symbols are automatically coloured according to their first character.
        // symbolColorMapper can be used to define a different symbol color.
        // The function must take the symbol code (after mapping) and return a colour in hex format.
        // If the function returns undefined, the default colouring for that symbol is used.
        symbolColorMapper: (symbolCode) => {
            if (symbolCode.toString().startsWith("H1"))
                return "#ffffff";
        },
        //If fullSpinForBalanceSaver is true, all reels will spin when balance saver is activated.
        //Otherwise, only those symbols that have changed will be updated.
        fullSpinForBalanceSaver: true,
        // Images can be supplied for symbols by providing an image map below.
        // Commented out because it needs debugging...
        imageMap: {
            0: await PIXI.Assets.load(wildSym),
            1: await PIXI.Assets.load(hp1Sym),
            2: await PIXI.Assets.load(hp2Sym),
            3: await PIXI.Assets.load(hp3Sym),
            4: await PIXI.Assets.load(hp4Sym),
            5: await PIXI.Assets.load(lp1Sym),
            6: await PIXI.Assets.load(lp2Sym),
            7: await PIXI.Assets.load(lp3Sym),
            8: await PIXI.Assets.load(lp4Sym),
            9: await PIXI.Assets.load(scatterSym)
        },
    });
    window.game = game;
});
