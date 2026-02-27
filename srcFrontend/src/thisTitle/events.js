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

/** @type {Record<string, EventHandler>} */
export default {
    "reelSpin": async ({reelSymbolArray}, game) => {
        await game.ui.currentPhase.grid.spinReels(reelSymbolArray);
    },
    "winAnalysis": async ({totalWin, wins}, game) => {
        if (!totalWin) return;
        game.shell.addWinsToHistory(wins);
        await Promise.all([
            game.ui.currentPhase.grid.animateWins(wins),
            game.ui.currentPhase.balance.increment(totalWin),
            game.ui.ticker.updateText(`You won ${toCurrency(totalWin)}!`),
        ])
    },
    "freeSpinsTrigger": async ({spins, multiplier}, game) => {
        game.shell.sessionStats.increment("freespins-sessions");
        await game.ui.setPhase("freeSpins");
        await Promise.all([
            game.ui.currentPhase.spinsLeft.update(spins, true),
            game.ui.ticker.updateText(`${spins} free spins awarded${
                multiplier !== 1
                    ? `with x${multiplier} multiplier`
                    : ``
            }!`)
        ]);
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
    "cascade": async ({removedSymbols, reelSymbolArray}, game) => {
        await game.ui.currentPhase.grid.cascade(removedSymbols, reelSymbolArray);
    }
};
