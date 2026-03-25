// @ts-check
/** @typedef {InstanceType<typeof import('../common/Game.js').default>} Game */

/**
 * @typedef {(eventDetails: Object, game: Game) => void | Promise<void>} EventHandler
 */

/** @brief This file lists how the frontend should respond to events from the backend.
 * To add behaviour for a custom event, add a new entry with the key set to the eventType property from the toJson() method of the Event class in the backend.
 * The value should be an asynchronous function that takes two parameters: the event details (you can destructure this to get the relevant properties as below), and the game instance.
 * (When the frontend hits an unhandled event, it will log a warning to the console which gives a template showing exactly how to set up your event handler.)
 * Look at the existing event handlers for examples. Note how await is used when an animation is occurring that the frontend needs to wait for before continuing.
 * await Promise.all([...]) to execute multiple animations at the same time and wait until they are all finished.*/

import toCurrency from "../common/utils/toCurrency.js";
import moveElement from "../common/animation/moveElement.js";
import popElementOut from "../common/animation/popElementOut.js";
import collectorPositions from "./collectorPositions.js";
import Image from "../common/uiElements/Image.js";
import * as PIXI from "pixi.js";
import Button from "../common/uiElements/Button.js";


/** @type {Record<string, EventHandler>} */
export default {
    "reelSpin": async ({reelSymbolArray, coinValues}, game) => {
        await game.ui.currentPhase.grid.spinReels(reelSymbolArray, {textOverlays: coinValues.map(r => r.map(v => v === 0 ? "" : toCurrency(v)))});
    },
    /**
     * @param {{reelSymbolArray: Array<Array<string>>, stickyPositions: Array<{reel:number, row:number}>}} params
     * @param game
     */
    "reelSpinWithStickySymbols": async ({reelSymbolArray, stickyPositions}, game) => {
        await game.ui.currentPhase.grid.spinReels(reelSymbolArray, {stickyPositions});
    },
    "winAnalysis": async ({totalWin, wins, analysisStage}, game) => {
        if (!totalWin) return;
        game.shell.addWinsToHistory(wins);
        const payAnywhereWins = wins.filter(({winType}) => winType === "scatter");
        const coinWins = wins.filter(({winType}) => winType === "coin");
        if (payAnywhereWins.length)
            await Promise.all([
                game.ui.currentPhase.grid.animateWins(payAnywhereWins),
                game.ui.currentPhase.balance.increment(payAnywhereWins.reduce((sum, {totalWin}) => sum + totalWin, 0)),
                game.ui.ticker.updateText(`You won ${toCurrency(payAnywhereWins.reduce((sum, {totalWin}) => sum + totalWin, 0))}!`)
            ])
        if (coinWins.length)
            await Promise.all([
                ...coinWins.map(({positions, multiplier}) => (async () => {
                    const {reel, row} = positions[0];
                    const coinSymbol = game.ui.currentPhase.grid.reels[reel].apertures[row].container;

                    popElementOut({element: coinSymbol, targetParent: game.ui.app.stage})
                    const multiplierText = new PIXI.Text(multiplier > 1 ? `x${multiplier}` : "", {
                        fontFamily: "Arial",
                        fontSize: 32,
                        fontWeight: "bold",
                        fill: 0xFF00FF,
                        stroke: "#000000",
                        strokeThickness: 5,
                        dropShadow: true,
                        dropShadowColour: "#000000",
                        dropShadowBlur: 4,
                        dropShadowAngle: Math.PI / 6,
                        dropShadowDistance: 6,
                        align: "center",
                    });
                    multiplierText.x = coinSymbol.x + coinSymbol.width;
                    multiplierText.y = coinSymbol.y + 25;
                    coinSymbol.parent.addChild(multiplierText);
                    await Promise.all([...[coinSymbol, multiplierText].map(el => moveElement({
                        element: el,
                        newX: el.x,
                        newY: el.y - 500,
                        pixelsPerFrame: 6
                    })), ...[coinSymbol, multiplierText].map(el => new Promise(resolve => {
                        const animate = (delta) => {
                            el.alpha -= delta / 50;
                            if (el.alpha <= 0) PIXI.Ticker.shared.remove(animate);
                            resolve();
                        };
                        PIXI.Ticker.shared.add(animate);
                    }))]);
                    coinSymbol.parent.removeChild(coinSymbol);
                    coinSymbol.destroy({texture: true, baseTexture: true});
                    multiplierText.destroy({texture: true, baseTexture: true});
                })()),
                game.ui.currentPhase.balance.increment(coinWins.reduce((sum, {totalWin}) => sum + totalWin, 0)),
                game.ui.ticker.updateText(`${toCurrency(coinWins.reduce((sum, {totalWin}) => sum + totalWin, 0))} cash awarded!`)
            ])
    },
    "freeSpinsTrigger": async ({spins, multiplier}, game) => {
        game.state.inactiveCollectors = 3;
        game.shell.sessionStats.increment("freespins-sessions");
        await game.ui.ticker.updateText(`${spins} free spins awarded${
            multiplier !== 1
                ? `with x${multiplier} multiplier`
                : ``
        }!`);
        game.ui.phases["freeSpins"].secondaryCollectors = {};
        const collectorImages = [];
        const secondaryCollectors = game.ui.phases["freeSpins"].secondaryCollectors;
        for (const reel of game.ui.currentPhase.grid.reels)
            for (const aperture of reel.apertures)
                if (aperture.symbolName === "BN")
                    collectorImages.push(aperture.symbol);

        let positions = ["topLeft", "topRight", "bottomLeft", "bottomRight"]
            .filter(p => game.state.mainCollectorPosition !== p);
        await Promise.all(collectorImages.map(collector => {
            popElementOut({element: collector, targetParent: game.ui.app.stage});
            const position = positions.shift();
            return moveElement({
                element: collector,
                newX: collectorPositions(game.ui, {sprite: collector})[position].x,
                newY: collectorPositions(game.ui, {sprite: collector})[position].y,
                pixelsPerFrame: 10
            }).then(() => {
                secondaryCollectors[position] = new Image({
                    texture: game.imageMap.BN,
                    x: collectorPositions(game.ui, {sprite: collector})[position].x,
                    y: collectorPositions(game.ui, {sprite: collector})[position].y,
                    parent: game.ui.app.stage,
                    width: 80,
                    height: 80,
                });
                secondaryCollectors[position].originalWidth = secondaryCollectors[position].sprite.width;
                secondaryCollectors[position].originalHeight = secondaryCollectors[position].sprite.height;
                collector.parent.removeChild(collector);
                collector.destroy({children: true});
            });
        }));
        game.ui.mainCollector.addOrUpdateText("x2", {
            y: 50,
            height: 50
        });
        game.ui.phases.freeSpins.secondaryControlAreas.forEach(button => button.buttonContainer.destroy({
            children: true,
            texture: true,
            baseTexture: true
        }));
        game.ui.phases.freeSpins.secondaryControlAreas = [];
        await game.ui.setPhase("freeSpins");
        game.ui.currentPhase.grid.reelBackground.addChild(game.ui.mainControlArea.buttonContainer);
        await game.ui.currentPhase.spinsLeft.update(spins, true);
    },
    "bonusSymbolCollection": async ({reel, row}, game) => {
        const grid = game.ui.currentPhase.grid;
        const symbol = grid.reels[reel].apertures[row].symbol;
        popElementOut({element: symbol, targetParent: game.ui.app.stage});
        const graySymbol = new PIXI.Sprite(game.imageMap.BG);
        grid.reels[reel].apertures[row].symbol = graySymbol;
        graySymbol.isImage = true;
        graySymbol.scale.set(
            grid.reelWidth / graySymbol.width,
            grid.symbolHeight / graySymbol.height,
        );
        graySymbol.x = (grid.reelWidth - graySymbol.width) / 2;
        graySymbol.y = (grid.symbolHeight - graySymbol.height) / 2;
        grid.reels[reel].apertures[row].container.addChild(graySymbol);
        const {x, y} = collectorPositions(game.ui, game.ui.mainCollector)[game.state.mainCollectorPosition];
        await moveElement({
            element: symbol,
            newX: x,
            newY: y,
            pixelsPerFrame: 10
        });
        symbol.destroy({children: true});
        const {mainCollector} = game.ui;
        if (mainCollector.sprite.width >= mainCollector.originalWidth * 2)
            return;
        mainCollector.sprite.scale.set(
            mainCollector.sprite.scale.x * (1 + 0.02 * (1 - (mainCollector.sprite.width - mainCollector.originalWidth) / mainCollector.originalWidth)),
            mainCollector.sprite.scale.y * (1 + 0.02 * (1 - (mainCollector.sprite.width - mainCollector.originalWidth) / mainCollector.originalWidth)),
        );
        const newPosition = collectorPositions(game.ui, mainCollector)[game.state.mainCollectorPosition];
        mainCollector.x = newPosition.x;
        mainCollector.y = newPosition.y;
    },
    "collectorAwardedBonus": async () => {
        await game.ui.ticker.updateText("Spider awarded super eggwakening!");
    },
    "freeSpinsCollectionTrigger": async ({spins, initialSecondaryCollectors}, game) => {
        game.state.inactiveCollectors = 3;
        const mainCollector = game.ui.mainCollector;
        game.shell.sessionStats.increment("freespins-sessions");
        await game.ui.ticker.updateText(`${spins} free spins awarded with x${initialSecondaryCollectors} hatched spiders!`);
        game.ui.phases["freeSpins"].secondaryCollectors = {};
        const secondaryCollectors = game.ui.phases["freeSpins"].secondaryCollectors;
        await Promise.all(["topLeft", "topRight", "bottomLeft", "bottomRight"]
            .filter(p => game.state.mainCollectorPosition !== p).map(((position, i) => {
                secondaryCollectors[position] = new Image({
                    texture: game.imageMap.BN,
                    x: -100 - i * 200,
                    y: -100,
                    parent: game.ui.app.stage,
                    width: 80,
                    height: 80,
                });
                secondaryCollectors[position].originalWidth = secondaryCollectors[position].sprite.width;
                secondaryCollectors[position].originalHeight = secondaryCollectors[position].sprite.height;
                return moveElement({
                    element: secondaryCollectors[position],
                    newX: collectorPositions(game.ui, secondaryCollectors[position])[position].x,
                    newY: collectorPositions(game.ui, secondaryCollectors[position])[position].y,
                    pixelsPerFrame: 10
                })
            })));
        mainCollector.addOrUpdateText("x2", {
            y: 50,
            height: 50
        });
        const {x, y} = mainCollector.originalScale;
        mainCollector.sprite.scale.set(x, y);
        const newPosition = collectorPositions(game.ui, mainCollector)[game.state.mainCollectorPosition];
        mainCollector.x = newPosition.x;
        mainCollector.y = newPosition.y;
        await game.ui.setPhase("freeSpins");
        game.ui.currentPhase.grid.reelBackground.addChild(game.ui.mainControlArea.buttonContainer);
        await game.ui.currentPhase.spinsLeft.update(spins, true);
    },
    "freeSpinsRetrigger": async ({spins, multiplier}, game) => {
        await Promise.all([
            game.ui.ticker.updateText(`${spins} retrigger spins awarded${
                multiplier !== 1
                    ? `with x${multiplier} multiplier`
                    : ``
            }!`),
            game.ui.currentPhase.spinsLeft.increment(spins)
        ]);
    },
    "freeSpinsComplete": async ({totalWin}, game) => {
        for (const collector of Object.values(game.ui.currentPhase.secondaryCollectors)) {
            collector.parent.removeChild(collector);
            collector.overlayText?.destroy({children: true, texture: true, baseTexture: true});
            collector.destroy({children: true});
        }
        game.ui.mainCollector.removeText();
        game.ui.currentPhase.secondaryCollectors = {};
        game.ui.phases.base.grid.reelBackground.addChild(game.ui.mainControlArea.buttonContainer);
        game.ui.phases.freeSpins.secondaryControlAreas.forEach(button => button.buttonContainer.destroy({
            children: true,
            texture: true,
            baseTexture: true
        }));
        game.ui.phases.freeSpins.secondaryControlAreas = [];
        await game.ui.endCurrentPhase(totalWin);
    },
    "newFreeSpin": async ({multiplier}, game) => {
        await game.ui.ticker.clearText();
        game.shell.eventHistory.addText("~~~~New Free Spin~~~~");
        if (multiplier > 1) await game.ui.ticker.updateText(`${multiplier}x multiplier for spin!`)
        game.shell.sessionStats.increment("freespins-played");
        await game.ui.currentPhase.spinsLeft.increment(-1);
    },
    "gridUpdate": async ({reelSymbolArray, message}, game) => {
        await Promise.all([
            game.ui.ticker.updateText(message),
            game.ui.currentPhase.grid.updateGrid(reelSymbolArray)
        ]);
    },
    "rnStreamProblem": async ({shortMessage, longMessage}, game) => {
        await game.reportRnStreamProblem(shortMessage, longMessage);
    },
    "cascade": async ({removedSymbols, removedSymbolsPerReel, coinValues, reelSymbolArray}, game) => {
        await game.ui.currentPhase.grid.cascade(removedSymbols, reelSymbolArray, {textOverlays: coinValues.map(r => r.map(v => v === 0 ? "" : toCurrency(v)))});
    },
    "coinCollection": async ({reel, row, collectorPosition}, game) => {
        const symbol = game.ui.currentPhase.grid.reels[reel].apertures[row].symbol;
        const grid = game.ui.currentPhase.grid;
        const graySymbol = new PIXI.Sprite(game.imageMap.CG);
        grid.reels[reel].apertures[row].symbol = graySymbol;
        graySymbol.isImage = true;
        graySymbol.scale.set(
            grid.reelWidth / graySymbol.width,
            grid.symbolHeight / graySymbol.height,
        );
        graySymbol.x = (grid.reelWidth - graySymbol.width) / 2;
        graySymbol.y = (grid.symbolHeight - graySymbol.height) / 2;
        grid.reels[reel].apertures[row].container.sortableChildren = true;
        graySymbol.zIndex = -Infinity;
        grid.reels[reel].apertures[row].container.addChild(graySymbol);
        popElementOut({element: symbol, targetParent: game.ui.app.stage});
        const {x, y} = collectorPositions(game.ui,
            game.ui.currentPhase.secondaryCollectors[collectorPosition])[collectorPosition];
        await moveElement({
            element: symbol,
            newX: x,
            newY: y,
            pixelsPerFrame: 10
        });
        symbol.destroy({children: true, texture: !symbol.isImage, baseTexture: !symbol.isImage});
        const collector = game.ui.currentPhase.secondaryCollectors[collectorPosition]
        if (collector.sprite.width >= collector.originalWidth * 2)
            return;
        collector.sprite.scale.set(
            collector.sprite.scale.x * (1 + 0.1 * (1 - (collector.sprite.width - collector.originalWidth) / collector.originalWidth)),
            collector.sprite.scale.y * (1 + 0.1 * (1 - (collector.sprite.width - collector.originalWidth) / collector.originalWidth)),
        );
        const newPosition = collectorPositions(game.ui, collector)[collectorPosition];
        collector.x = newPosition.x;
        collector.y = newPosition.y;
    },
    "collectorActivation": async ({position, collectorMultipliers}, game) => {
        await Promise.all([
            game.ui.currentPhase.secondaryCollectors[position].transformImage(game.imageMap.activeSecondaryCollector).then(() => {
                const collector = game.ui.currentPhase.secondaryCollectors[position];
                const newPosition = collectorPositions(game.ui, collector)[position];
                collector.x = newPosition.x;
                collector.y = newPosition.y;
            }),
            game.ui.ticker.updateText("A spider hatched!")
        ]);
        const {
            controlX,
            controlY
        } = collectorPositions(game.ui, game.ui.currentPhase.secondaryCollectors[position])[position];
        const colours = {
            3: 0x6A0DAD,
            2: 0x9932CC,
            1: 0xC020A0
        }
        const button = new Button({
            x: controlX,
            y: controlY,
            height: 240,
            width: 240,
            parent: game.ui.currentPhase.grid.reelBackground,
            colour: colours[game.state.inactiveCollectors],
        })
        button.buttonContainer.zIndex = game.state.inactiveCollectors;
        game.ui.currentPhase.secondaryControlAreas.push(button);
        game.state.inactiveCollectors--;
        Object.entries(collectorMultipliers).forEach(([collectorPosition, multiplier]) => {
            if (multiplier > 1)
                if (collectorPosition === game.state.mainCollectorPosition)
                    game.ui.mainCollector.addOrUpdateText(`x${multiplier}`, {
                        y: 40,
                        height: 50
                    });
                else
                    game.ui.currentPhase.secondaryCollectors[collectorPosition].addOrUpdateText(`x${multiplier}`, {
                        y: 40,
                        height: 50
                    });
        })
        await game.ui.ticker.updateText("Spider multipliers updated!");
    },
    "maxedOutMultipliers": async ({}, game) => {
        await game.ui.ticker.updateText("All spiders hatched!");
        ["topLeft", "topRight", "bottomLeft", "bottomRight"].forEach(collectorPosition => {
            if (collectorPosition === game.state.mainCollectorPosition)
                game.ui.mainCollector.addOrUpdateText(`x5`, {
                    y: 50,
                    height: 40
                });
            else
                game.ui.currentPhase.secondaryCollectors[collectorPosition].addOrUpdateText(`x5`, {
                    y: 50,
                    height: 40
                });
        })
        const button = new Button({
            height: 400,
            width: 400,
            parent: game.ui.currentPhase.grid.reelBackground,
            colour: 0x4B0082,
        })
        button.buttonContainer.zIndex = Infinity;
        game.ui.currentPhase.secondaryControlAreas.push(button);
        await game.ui.ticker.updateText("Multipliers maxed out!");
    }
};
