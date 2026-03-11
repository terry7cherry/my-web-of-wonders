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
                ...coinWins.map(({collectorPosition, positions, multiplier}) => (async () => {
                    const {x, y} = collectorPositions(game.ui)[collectorPosition];
                    const {reel, row} = positions[0];
                    const coinSymbol = game.ui.currentPhase.grid.reels[reel].apertures[row].container;

                    popElementOut({element: coinSymbol, targetParent: game.ui.app.stage})
                    await moveElement({
                        element: coinSymbol,
                        newX: x,
                        newY: y,
                        pixelsPerFrame: 10
                    })
                    coinSymbol.parent.removeChild(coinSymbol);
                    coinSymbol.destroy({texture: true, baseTexture: true});
                })()),
                game.ui.currentPhase.balance.increment(coinWins.reduce((sum, {totalWin}) => sum + totalWin, 0)),
                game.ui.ticker.updateText(`${toCurrency(coinWins.reduce((sum, {totalWin}) => sum + totalWin, 0))} cash awarded!`)
            ])
    },
    "freeSpinsTrigger": async ({spins, multiplier}, game) => {
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
                newX: collectorPositions(game.ui)[position].x,
                newY: collectorPositions(game.ui)[position].y,
                pixelsPerFrame: 10
            }).then(() => {
                collector.parent.removeChild(collector);
                collector.destroy({children: true});
                secondaryCollectors[position] = new Image({
                    texture: game.imageMap.BN,
                    x: collectorPositions(game.ui)[position].x,
                    y: collectorPositions(game.ui)[position].y,
                    parent: game.ui.app.stage,
                    width: 100,
                    height: 100,
                });
            });
        }));
        game.ui.mainCollector.addOrUpdateText("x2", {
            y: 50,
            height: 50
        });
        await game.ui.setPhase("freeSpins");
        await game.ui.currentPhase.spinsLeft.update(spins, true);
    },
    "bonusSymbolCollection": async ({reel, row}) => {
        const symbol = game.ui.currentPhase.grid.reels[reel].apertures[row].symbol;
        popElementOut({element: symbol, targetParent: game.ui.app.stage});
        const {x, y} = collectorPositions(game.ui)[game.state.mainCollectorPosition];
        await moveElement({
            element: symbol,
            newX: x,
            newY: y,
            pixelsPerFrame: 10
        });
        symbol.destroy({children: true});
        game.ui.mainCollector.sprite.scale.set(
            game.ui.mainCollector.sprite.scale.x * 1.03,
            game.ui.mainCollector.sprite.scale.y * 1.03,
        )
        game.ui.mainCollector.sprite.x = game.ui.mainCollector.sprite.x - game.ui.mainCollector.sprite.height * 0.015
        game.ui.mainCollector.sprite.y = game.ui.mainCollector.sprite.y - game.ui.mainCollector.sprite.width * 0.015
    },
    "collectorAwardedBonus": async () => {
        await game.ui.ticker.updateText("Spider awarded super egggwakening!");
    },
    "freeSpinsCollectionTrigger": async ({spins, initialSecondaryCollectors}, game) => {
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
                    width: 100,
                    height: 100,
                });
                return moveElement({
                    element: secondaryCollectors[position],
                    newX: collectorPositions(game.ui)[position].x,
                    newY: collectorPositions(game.ui)[position].y,
                    pixelsPerFrame: 10
                })
            })));
        game.ui.mainCollector.addOrUpdateText("x2", {
            y: 50,
            height: 50
        });
        await game.ui.setPhase("freeSpins");
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
        popElementOut({element: symbol, targetParent: game.ui.app.stage});
        const {x, y} = collectorPositions(game.ui)[collectorPosition];
        await moveElement({
            element: symbol,
            newX: x,
            newY: y,
            pixelsPerFrame: 10
        });
        symbol.destroy({children: true, texture: !symbol.isImage, baseTexture: !symbol.isImage});
        const collector = game.ui.currentPhase.secondaryCollectors[collectorPosition]
        collector.sprite.scale.set(
            collector.sprite.scale.x * 1.06,
            collector.sprite.scale.y * 1.06,
        )
        collector.sprite.x = collector.sprite.x - collector.sprite.height * 0.03
        collector.sprite.y = collector.sprite.y - collector.sprite.width * 0.03
    },
    "collectorActivation": async ({position, collectorMultipliers}, game) => {
        await Promise.all([
            game.ui.currentPhase.secondaryCollectors[position].transformImage(game.imageMap.activeSecondaryCollector),
            game.ui.ticker.updateText("A spider hatched!")
        ]);
        Object.entries(collectorMultipliers).forEach(([collectorPosition, multiplier]) => {
            if (multiplier > 1)
                if (collectorPosition === game.state.mainCollectorPosition)
                    game.ui.mainCollector.addOrUpdateText(`x${multiplier}`, {
                        y: 50,
                        height: 50
                    });
                else
                    game.ui.currentPhase.secondaryCollectors[collectorPosition].addOrUpdateText(`x${multiplier}`, {
                        y: 50,
                        height: 50
                    });
        })
        await game.ui.ticker.updateText("Spider multipliers updated!");
    }
};
