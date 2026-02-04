// @ts-check
/** @typedef {InstanceType<typeof import('../common/Game.js').default>} Game */

/**
 * @typedef {(eventDetails: Object, game: Game) => void | Promise<void>} EventHandler
 */

/** @type {Record<string, EventHandler>} */
export default {
    reelSpin: async (eventDetails, game) => {
        await game.ui.currentPhase.grid.spinReels(eventDetails.reelSymbolArray);
    },
    winAnalysis: async ({totalWin, wins}, game) => {
        const winnings = totalWin;
        if (!winnings) return;
        game.shell.eventHistory.addText("Wins:")
        for (const {symbol, oak, multiplier, win, totalWin, winType} of wins)
            game.shell.eventHistory.addText(
                `&#9;- ${winType[0].toUpperCase() + winType.slice(1)} win: ${game.maths.symbolNames[symbol]}x${oak} for $${((win) / 100).toFixed(2)}${multiplier !== 1 ? ` x ${multiplier} = $${((totalWin) / 100).toFixed(2)}` : ``}`
            );
        game.state.totalGameRoundWinnings += winnings;
        await Promise.all([
            game.ui.currentPhase.grid.animateWinLines(wins),
            game.ui.currentPhase.balance.increment(winnings),
            game.ui.ticker.updateText(`You won £${(winnings / 100).toFixed(2)}!`)
        ])
    },
    freeSpinsTrigger: async ({spins, multiplier}, game) => {
        game.shell.sessionStats.increment("freespins-sessions");
        game.ui.setPhase("freeSpins");
        const phaseUI = game.ui.currentPhase;
        await Promise.all([
            phaseUI.spinsLeft.update(spins, true),
            game.ui.ticker.updateText(`${spins} free spins awarded${multiplier !== 1 ? `with x${multiplier} multiplier` : ``}!`)
        ]);
    },
    freeSpinsRetrigger: async ({spins, multiplier}, game) => {
        const phaseUI = game.ui.currentPhase;
        await Promise.all([
            game.ui.ticker.updateText(`${spins} retrigger spins awarded${multiplier !== 1 ? `with x${multiplier} multiplier` : ``}!`),
            phaseUI.spinsLeft.increment(spins)
        ]);
    },
    freeSpinsComplete: async ({totalWin}, game) => {
        await game.ui.ticker.updateText("Free spins over!")
        const phaseUI = game.ui.currentPhase;
        if (totalWin !== phaseUI.balance.value) {
            game.shell.eventHistory.addText(`Server: $${(totalWin / 100).toFixed(2)}`)
            game.shell.eventHistory.addText(`Frontend: $${(phaseUI.balance.value / 100).toFixed(2)}`)
            await game.ui.ticker.updateText("Free spins total win mismatch.")
            throw new Error("Free spins total win mismatch.");
        }
        game.shell.sessionStats.increment("total-won", phaseUI.balance.value / 100);
        await Promise.all([
            game.ui.ticker.updateText(`Free spins total: £${(phaseUI.balance.value / 100).toFixed(2)}!`),
            game.ui.balance.increment(phaseUI.balance.value),
        ]);
    },
    newFreeSpin: async ({multiplier}, game) => {
        await game.ui.ticker.clearText();
        game.shell.eventHistory.addText("~~~~New Freespin~~~~");
        if (multiplier > 1) await game.ui.ticker.updateText(`${multiplier}x multiplier for spin!`)
        game.shell.sessionStats.increment("freespins-played");
        await game.ui.currentPhase.spinsLeft.increment(-1);
    },
    gridUpdate: async ({reelSymbolArray, message}, game) => {
        await Promise.all([
            game.ui.ticker.updateText(message),
            game.ui.currentPhase.grid.updateGrid(reelSymbolArray)
        ]);
    }

};
