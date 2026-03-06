import * as PIXI from "pixi.js";
import MaskedContainer from "./MaskedContainer.js";
import Mask from "./Mask.js";
import moveElement from "../animation/moveElement.js"
import Ticker from "../uiElements/Ticker.js"

export default class Grid {
    constructor({
                    parent,
                    width,
                    height,
                    x,
                    y,
                    numReels,
                    numRows,
                    symbolColourMapper,
                    symbolFontSize = 32,
                    displayReelSet,
                    symbolMap,
                    imageMap,
                    sound,
                    suspenseSymbol,
                    simultaneousWinLineAnimations,
                    winLineColour,
                    waysWinColour,
                    isHoldAndSpin,
                    holdAndSpinBlankSymbols,
                    weightedDisplayReelSet
                }) {
        this.reelWidth = Math.round(width / numReels);
        this.symbolHeight = Math.round(height / numRows);
        this.numReels = numReels;
        this.numRows = numRows;
        this.x = x ?? 0;
        this.y = y ?? 0;
        this.width = width;
        this.height = height;
        this.sound = sound;
        this.symbolFontSize = symbolFontSize;
        this.symbolColourMapper = symbolColourMapper;
        this.suspenseSymbol = suspenseSymbol;
        this.displayReelSet = displayReelSet;
        this.symbolMap = symbolMap;
        this.imageMap = imageMap;
        this.container = new MaskedContainer({
            parent,
            x: this.x,
            y: this.y,
            width: this.width,
            height: this.height
        });
        this.simultaneousWinLineAnimations = simultaneousWinLineAnimations;
        this.winLineColour = winLineColour;
        this.waysWinColour = waysWinColour;
        this.isHoldAndSpin = isHoldAndSpin;
        this.holdAndSpinBlankSymbols = holdAndSpinBlankSymbols;
        this.weightedDisplayReelSet = weightedDisplayReelSet;
        this.reels = this.createReels();
        this.getReelStartingPositions();
    }

    createReels() {
        const reels = [];
        for (let i = 0; i < this.numReels; i++) {
            const container = new PIXI.Container();
            container.sortableChildren = true;
            container.x = i * this.reelWidth;
            this.container.addChild(container);
            const apertures = [];
            const reel = {container, apertures};
            reels.push(reel);
        }
        return reels;
    }

    getReelStartingPositions() {
        if (this.isHoldAndSpin)
            this.reels.forEach((_, r) => {
                for (let i = 0; i < this.numRows; i++)
                    this.addSymbolToBottomOfReel(r, this.holdAndSpinBlankSymbols[0])
            })
        else
            this.reels.forEach((_, r) => {
                const symbols = this.getRandomSymbols(r, this.numRows);
                symbols.forEach(symbol => this.addSymbolToBottomOfReel(r, symbol));
            });
    }

    getRandomSymbols(reelNum, numSymbols) {
        const symbols = [];
        if (this.displayReelSet) {
            const displayReel = this.displayReelSet[reelNum];
            const reel = this.reels[reelNum];
            reel.landingPosition = Math.floor(Math.random() * displayReel.length);

            for (let i = 0; i < numSymbols; i++) {
                const symbol = displayReel[(reel.landingPosition + i) % displayReel.length];
                symbols.push(symbol);
            }
        } else if (this.weightedDisplayReelSet) {
            const singleReel = typeof this.weightedDisplayReelSet[0].symbol !== "undefined";
            /** @type {Array<{weight: number, outcome: string}>} */
            const reel = singleReel ? this.weightedDisplayReelSet : this.weightedDisplayReelSet[reelNum];
            const totalWeight = reel.map(({weight}) => weight).reduce((a, b) => a + b, 0);
            for (let i = 0; i < numSymbols; i++) {
                const random = Math.floor(Math.random() * totalWeight);
                let runningTotal = 0;
                const outcome = reel.find(({weight}) => {
                    runningTotal += weight;
                    return random < runningTotal;
                }).outcome;
                symbols.push(outcome);
            }
        } else throw new Error("No display reel set found. displayReelSet or weightedDisplayReelSet must be set in game constructor.")
        return symbols;
    }

    /**
     * @typedef {Object} SpinOptions
     * @property {Array<number>} [stickyReels] - An array of indexes of reels that should not be spun.
     * @property {Array<{reel: number, row: number}>} [stickyPositions] - An array of {reel, row} objects
     * @property {boolean} [spinAll] - For hold and spin only, spin all mini-reels disregarding locked symbols.
     */

    /**
     * Spins the reels, with various options.
     * @param {Array<Array<string>>} symbols - The symbols to be displayed on the reels.
     * @param {SpinOptions} [options] - The options for spinning the reels.
     * @returns {Promise<unknown>}
     */
    spinReels(symbols, {stickyReels, stickyPositions, spinAll} = {}) {
        return new Promise(async (resolve, reject) => {
            const spinSound = await this.sound.playAsync("spin");
            let reelSpinAnimations;
            if (this.isHoldAndSpin) {
                reelSpinAnimations = this.getHoldAndSpinAnimations(this.reels, symbols, stickyReels, stickyPositions, spinAll);
            } else {
                this.setAddedTimePerReel(this.reels, symbols);
                reelSpinAnimations = this.getReelSpinAnimations(this.reels, symbols, stickyReels, stickyPositions);
            }
            await Promise.all(reelSpinAnimations)
            await spinSound.stop();
            resolve();
        });
    }

    spinReelsWithTextOverlays(symbols, stickyReels, stickyPositions, textOverlays, spinAll) {
        return new Promise(async (resolve, reject) => {
            const spinSound = await this.sound.playAsync("spin");
            let reelSpinAnimations;
            if (this.isHoldAndSpin) {
                reelSpinAnimations = this.getHoldAndSpinAnimations(this.reels, symbols, stickyReels, stickyPositions, spinAll, textOverlays);
            } else {
                this.setAddedTimePerReel(this.reels, symbols);
                reelSpinAnimations = this.getReelSpinAnimations(this.reels, symbols, stickyReels, stickyPositions, textOverlays);
            }
            await Promise.all(reelSpinAnimations)
            await spinSound.stop();
            resolve();
        });
    }

    getReelSpinAnimations(reels, symbols, stickyReels, stickyPositions, textOverlays) {
        return reels.map((reel, r) => new Promise((resolve, reject) => {
            if (stickyReels && stickyReels.includes(r)) {
                return resolve();
            }
            const numRandomSymbolsToAdd = Math.floor((r + reel.addedTime + 1) * (2 + Math.random() * 2));
            this.addRandomSymbolsToReel(reel, r, numRandomSymbolsToAdd);
            if (textOverlays) textOverlays[r] = textOverlays[r].reverse();
            [...symbols[r]].reverse().forEach((symbol, a) => {
                this.addSymbolToTopOfReel(r, symbol, this.height / symbols[r].length, textOverlays?.[r]?.[a]);
            });
            const totalFrames = (r + reel.addedTime + 1) * 20;
            const apertureAnimations = reel.apertures.map((aperture, a) => {
                const {container} = aperture;
                if (stickyPositions?.find((p) => p.reel === r && (p.row + numRandomSymbolsToAdd + symbols[r].length === a))) {
                    container.zIndex = Infinity;
                    return;
                }
                const newY = container.y + this.symbolHeight * (this.numRows + numRandomSymbolsToAdd);
                return moveElement({element: container, newX: container.x, newY, totalFrames})
            })
            Promise.all(apertureAnimations).then(() => {
                const oldApertures = reel.apertures.splice(this.numRows);

                for (const aperture of oldApertures) {
                    // detach from display tree
                    reel.container.removeChild(aperture.container);

                    // destroy full container hierarchy
                    aperture.container.destroy({
                        children: true,
                    });
                }

                resolve();
            })
        }))
    }

    getHoldAndSpinAnimations(reels, symbols, stickyReels, stickyPositions, spinAll, textOverlays) {
        return reels.map(
            (reel, r) => reel.apertures.map((aperture, a) => (async () => {
                if (!spinAll) {
                    if (!this.holdAndSpinBlankSymbols.includes(aperture.symbolName)) return;
                    if (stickyReels?.includes(r) || stickyPositions?.find(p => p.reel === r && p.row === a)) return;
                }
                const numRandomSymbolsToAdd = Math.floor((4 + Math.random() * 25));
                const randomSymbols = this.getRandomSymbols(r, numRandomSymbolsToAdd);
                for (let i = 0; i < numRandomSymbolsToAdd; i++) {
                    await this.transformSymbol(r, a, randomSymbols[i], 8 + (numRandomSymbolsToAdd - i) * 2);
                }
                await this.transformSymbol(r, a, symbols[r][a], 5, textOverlays?.[r]?.[a]);
            })())
        ).flat()
    }

    setAddedTimePerReel(reels, symbols) {
        let addedTime = 0;
        reels.forEach((reel, r) => {
            const numScatters = symbols
                .slice(0, r)
                .map((reel) => reel.filter((s) =>
                        s === this.suspenseSymbol
                    ).length
                )
                .reduce((a, b) => a + b, 0);
            if (numScatters >= 2) addedTime += 4;
            reel.addedTime = addedTime;
        })
    }

    addRandomSymbolsToReel(reel, reelIndex, numRandomSymbolsToAdd) {
        const symbols = this.getRandomSymbols(reelIndex, numRandomSymbolsToAdd);
        symbols.forEach(symbol => this.addSymbolToTopOfReel(reelIndex, symbol))
    }

    addSymbolToBottomOfReel(reelNum, symbolName) {
        const reel = this.reels[reelNum];
        const symbol = this.getSymbol(symbolName);
        symbol.container.y = reel.apertures.length * this.symbolHeight;
        reel.apertures.push(symbol);
        reel.container.addChild(symbol.container);
    }

    addSymbolToTopOfReel(reelNum, symbolName, height, textOverlay) {
        const reel = this.reels[reelNum];
        const symbol = this.getSymbol(symbolName, height);
        symbol.container.y = reel.apertures[0].container.y - symbol.height;
        reel.apertures.unshift(symbol);
        reel.container.addChild(symbol.container);
        this.addTextToSymbol(reelNum, 0, textOverlay)
    }

    getSymbol(symbolName, height) {
        const container = new PIXI.Container();
        const backgroundContainer = new PIXI.Container();
        container.addChild(backgroundContainer);
        const symbol = this.getSymbolImageOrText(symbolName, height);
        symbol.x = (this.reelWidth - symbol.width) / 2;
        symbol.y = ((height ?? this.symbolHeight) - symbol.height) / 2;
        container.addChild(symbol);
        const overlayContainer = new PIXI.Container();
        container.addChild(overlayContainer);
        return {
            container,
            backgroundContainer,
            overlayContainer,
            height: height ?? this.symbolHeight,
            symbol,
            symbolCode: symbolName,
            symbolName
        };
    }

    getSymbolImageOrText(symbolName, height) {
        const texture = this.imageMap[symbolName];
        if (!texture) {
            return this.getSymbolText(symbolName);
        }
        const symbol = new PIXI.Sprite(texture);
        symbol.scale.set(
            this.reelWidth / symbol.width,
            (height ?? this.symbolHeight) / symbol.height,
        );
        symbol.isImage = true;
        return symbol;
    }

    getSymbolText(text, textStyle) {
        return new PIXI.Text(text, textStyle ?? this.getSymbolTextStyle(text));
    }

    async updateGrid(newGrid) {
        await Promise.all(this.reels
            .map((reel, r) => reel.apertures.map(({symbolCode}, a) => {
                if (symbolCode === newGrid[r][a]) return;
                return this.transformSymbol(r, a, newGrid[r][a]);
            }))
            .flat()
            .filter((p) => p));
    }

    addBackgroundToSymbol(reelNum, symbolNum, colour) {
        const aperture = this.reels[reelNum].apertures[symbolNum];
        const background = new PIXI.Graphics();
        background.beginFill(colour);
        background.drawRect(0, 0, this.reelWidth, aperture.height);
        background.endFill();
        aperture.backgroundContainer.addChild(background);
        aperture.background = background;
    }

    removeBackgroundFromSymbol(reelNum, symbolNum) {
        const aperture = this.reels[reelNum].apertures[symbolNum];
        aperture.backgroundContainer.removeChild(aperture.background);
        aperture.background.destroy({children: true});
    }

    addFrameToSymbol(reelNum, symbolNum, colour = "0xFFFFFF", width = 8) {
        const aperture = this.reels[reelNum].apertures[symbolNum];
        const frame = new PIXI.Graphics();
        frame.moveTo(width / 2, width / 2);
        frame.lineStyle(width, colour, 1, 0.5);
        frame.lineTo(width / 2, aperture.height - width / 2);
        frame.lineTo(this.reelWidth - width / 2, aperture.height - width / 2);
        frame.lineTo(this.reelWidth - width / 2, width / 2);
        frame.lineTo(0, width / 2);
        aperture.overlayContainer.addChild(frame);
        aperture.frame = frame;
    }

    removeFrameFromSymbol(reelNum, symbolNum) {
        const aperture = this.reels[reelNum].apertures[symbolNum];
        aperture.overlayContainer.removeChild(aperture.frame);
        aperture.frame.destroy({children: true});
    }

    addTextToSymbol(reelNum, symbolNum, text, textStyle) {
        const aperture = this.reels[reelNum].apertures[symbolNum];
        textStyle = {
            fontFamily: "Arial",
            fontSize: this.symbolFontSize,
            fontWeight: "bold",
            fill: "#FFFFFF",
            stroke: "#000000",
            strokeThickness: 5,
            dropShadow: true,
            dropShadowColour: "#000000",
            dropShadowBlur: 4,
            dropShadowAngle: Math.PI / 6,
            dropShadowDistance: 6,
            align: "center",
            ...textStyle
        }
        const textObject = this.getSymbolText(text, textStyle);
        textObject.x = textStyle.x ?? (this.reelWidth - textObject.width) / 2;
        textObject.y = textStyle.y ?? (aperture.height - textObject.height);
        aperture.overlayContainer.addChild(textObject)
        aperture.overlayText = textObject;
    }

    removeTextFromSymbol(reelNum, symbolNum) {
        const aperture = this.reels[reelNum].apertures[symbolNum];
        aperture.overlayContainer.removeChild(aperture.overlayText);
        aperture.overlayText.destroy({children: true});
    }

    transformSymbol(reelNum, symbolNum, newSymbolName, speed, textOverlay) {
        return new Promise((resolve, reject) => {
            const reel = this.reels[reelNum];
            const oldAperture = reel.apertures[symbolNum];
            const newAperture = this.getSymbol(newSymbolName, oldAperture.height);
            const newSymbol = newAperture.symbol;
            newAperture.container.x = oldAperture.container.x;
            newAperture.container.y = oldAperture.container.y;
            newSymbol.y = newSymbol.y + oldAperture.height;
            const mask = new Mask({
                parent: this.container,
                x: reelNum * this.reelWidth,
                y: oldAperture.container.y,
                width: this.reelWidth,
                height: oldAperture.height
            });
            oldAperture.container.mask = mask;
            newAperture.container.mask = mask;
            reel.container.addChild(newAperture.container);
            reel.apertures[symbolNum] = newAperture;
            this.addTextToSymbol(reelNum, symbolNum, textOverlay);
            const oldSymbol = oldAperture.symbol;
            const animateOldSymbol = moveElement({
                element: oldSymbol,
                newX: oldSymbol.x,
                newY: oldSymbol.y - oldAperture.height,
                pixelsPerFrame: speed ?? 4
            })
            const animateNewSymbol = moveElement({
                element: newSymbol,
                newX: newSymbol.x,
                newY: newSymbol.y - newAperture.height,
                pixelsPerFrame: speed ?? 4
            })
            Promise.all([animateOldSymbol, animateNewSymbol]).then(() => {
                oldAperture.container.destroy({
                    children: true, texture: !oldSymbol.isImage, baseTexture: !oldSymbol.isImage
                });
                newAperture.container.mask = null;
                mask.destroy();
                resolve();
            })
        });
    }

    getSymbolTextStyle(symbolCode) {
        const colour = (this.symbolColourMapper && this.symbolColourMapper(symbolCode)) ?? {
            L: "#9cd141", M: "#1b8c44", H: "#07ba04", B: "#d1672e", S: "#d1672e", W: "#2e5cd1"
        }[symbolCode[0]] ?? "#ffffff";
        return {
            fontFamily: "Arial",
            fontSize: this.symbolFontSize,
            fontWeight: "bold",
            fill: colour,
            stroke: "#000000",
            strokeThickness: 5,
            dropShadow: true,
            dropShadowColour: "#000000",
            dropShadowBlur: 4,
            dropShadowAngle: Math.PI / 6,
            dropShadowDistance: 6,
            align: "center",
        };
    }

    animateWinLine(win) {
        return new Promise((resolve, reject) => {
            let flashing = true;
            let elapsedTime = 0;
            const flashTime = 40;
            const offTime = 20;
            const totalTime = flashTime + offTime;
            const lineGraphic = new PIXI.Graphics();
            const OAK = win.oak;
            const centrePositions = win.payLine.map((y, x) => ({
                x: (x + 0.5) * this.reelWidth,
                y: (y + 0.5) * this.symbolHeight,
            }));
            lineGraphic.lineStyle(4, this.winLineColour);
            lineGraphic.moveTo(centrePositions[0].x, centrePositions[0].y);
            centrePositions.slice(1).forEach(({x, y}) => {
                lineGraphic.lineTo(x, y);
            });
            this.container.addChild(lineGraphic);
            const symbolGraphics = [];
            win.payLine.slice(0, OAK).forEach((y, x) => {
                const container = this.reels[x].apertures[y].overlayContainer;
                const symbolGraphic = new PIXI.Graphics();
                symbolGraphic.lineStyle(4, this.winLineColour);
                symbolGraphic.drawCircle(this.reelWidth / 2, this.symbolHeight / 2, 50);
                container.addChild(symbolGraphic);
                symbolGraphics.push({container, symbolGraphic});
            });
            const animate = (delta) => {
                elapsedTime += delta;
                const shouldBeFlashing = elapsedTime < flashTime;
                if (elapsedTime > flashTime && flashing) {
                    this.container.removeChild(lineGraphic);
                    lineGraphic.destroy();
                    while (symbolGraphics.length) {
                        const {container, symbolGraphic} = symbolGraphics.pop();
                        container.removeChild(symbolGraphic);
                        symbolGraphic.destroy();
                    }
                    flashing = false;
                }
                if (elapsedTime > totalTime) {
                    PIXI.Ticker.shared.remove(animate);
                    return resolve();
                }
            };
            PIXI.Ticker.shared.add(animate);
        })
    }

    animateScatterOrWaysWin(win) {
        return new Promise((resolve, reject) => {
            const flashTime = 40;
            const offTime = 20;
            win.positions.forEach(({reel, row}) => {
                this.addFrameToSymbol(reel, row, this.waysWinColour, 10);
            })
            let elapsedFrames = 0;
            let flashing = true;
            const animate = (delta) => {
                elapsedFrames += delta;
                if (elapsedFrames > flashTime && flashing) {
                    flashing = false;
                    win.positions.forEach(({reel, row}) => this.removeFrameFromSymbol(reel, row));
                } else if (elapsedFrames > flashTime + offTime) {
                    PIXI.Ticker.shared.remove(animate);
                    return resolve();
                }
            }
            PIXI.Ticker.shared.add(animate);
        })
    }

    async animateWins(wins) {
        const lineWins = wins.filter(({winType}) => winType == "line");
        if (this.simultaneousWinLineAnimations) {
            await Promise.all(lineWins.map((win) => this.animateWinLine(win)))
        } else {
            for (let win of lineWins) {
                await this.animateWinLine(win);
            }
        }
        const waysAndScatterWins = wins.filter(({winType}) => ["way", "ways", "scatter"].includes(winType));
        for (let win of waysAndScatterWins) {
            await this.animateScatterOrWaysWin(win);
        }
    }

    async cascade(removedSymbols, reelSymbolArray) {
        await this.removeSymbols(removedSymbols);
        await this.fillReels(reelSymbolArray);
    }

    async removeSymbols(removedSymbols) {
        for (let {reel, row} of removedSymbols) {
            const aperture = this.reels[reel].apertures[row];
            aperture.container.destroy({
                children: true,
                texture: !aperture.symbol.isImage,
                baseTexture: !aperture.symbol.isImage
            });
            this.reels[reel].apertures.splice(row, 1);
        }
    }

    async fillReels(reelSymbolArray) {
        const pixelsPerFrame = 10;
        await Promise.all(reelSymbolArray.map((newReel, reelNum) => (async () => {
            const reel = this.reels[reelNum];
            const newSymbols = newReel.slice(0, newReel.length - reel.apertures.length);
            const symbolHeight = this.height / newReel.length;
            newSymbols.reverse().forEach((symbolName, i) => {
                const symbol = this.getSymbol(symbolName, symbolHeight);
                symbol.container.y = -symbol.height * (i + 1);
                reel.apertures.unshift(symbol);
                reel.container.addChild(symbol.container);
            })
            await new Promise(resolve => Ticker.wait(resolve, reelNum * 100));
            await Promise.all(reel.apertures.map((aperture, row) => {
                return moveElement({
                    element: aperture.container,
                    newY: row * symbolHeight,
                    pixelsPerFrame
                });
            }));
        })()))
    }
}
