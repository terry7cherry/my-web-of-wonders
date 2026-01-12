import Game from "./common/Game.js"
import moveElement from "./common/animation/moveElement.js"

// Useful functions:
// await this.grid.transformSymbol(reelnum, rowNum, newSymbolCode) - Updates symbol with a scrolling animation.
// await this.grid.updateGrid(newGrid) - Updates only symbols that have changed from existing grid with scrolling animation.
// await moveElement({ element, newX, newY, pixlesPerFrame OR totalFrames}) - Moves any PIXI element to new location at given speed OR within given timeframe.
// await this.grid.addBackgroundToSymbol(reelNum, symbolNum, color) - Adds a background colour to a symbol. (Will only be visible if symbol has transparent parts.)
// await this.grid.removeBackgroundFromSymbol(reelNum, symbolNum)
// await this.grid.addFrameToSymbol(reelNum, symbolNum[, color, width]) - Adds a frame on top of symbol (default white, 8px).
// await this.grid.removeFrameFromSymbol(reelNum, symbolNum)
// await this.grid.addTextToSymbol(reelNum, symbolNum, text[, textStyle]) - Adds text at bottom of symbol. textStyle will overwrite whichever style parameters you add (fill etc. - look up PIXI.TextStyle). You can also override default placement with x and y parameters in textStyle.
// await this.grid.removeTextFromSymbol(reelNum, symbolNum)

export default class CustomGame extends Game {
    constructor(config) {
        return (async () => {
            const game = await super(config);
            // game.extraProperty = params.extraProperty;
            // Note: due to async constructor pattern, instance properties are stored on "game" rather than "this".
            // They are accesible on "this" inside class methods though.
            return game;
        })();
    }

    /**
     * Returns a dictionary of macro round type handlers.
     * @returns {Object<string, Function>} A mapping of macro round types to handler methods.
     */
    getMacroRoundHandlers() {
        const macroRoundHandlers = super.getMacroRoundHandlers();
        // Add additional game modes like this:
        // macroRoundHandlers[modeName] = this.handleModeName;
        return macroRoundHandlers;
    }

    /**
     * Handles main game macro round (reel spin and winnings display handled by default).
     * @param {Object} macroRound - Macro round data from server.
     * @returns {Promise<void>}
     */
    // async handleMGame(macroRound) {
    //     await Promise.all([
    //         this.grid.spinReels(macroRound.PARAMETERS.grid),
    //         // this.asyncMethodToBeExecutedAtTheSameTimeAsSpin(macroRound)
    //     ])
    //     // await this.asyncMethodToBeExecutedAfterSpin(macroRound);
    //     await this.displayWinnings(macroRound);
    //     if(macroRound.PARAMETERS.freespins_collected) await this.launchFreeSpins(macroRound.PARAMETERS);
    // }

    /**
     * Handles a balance saver macro round. Default updates symbols that have changed with a scrolling animation or, if fullSpinForBalanceSaver is true, performs a full respin.
     * @param {Object} macroRound - Macro round data.
     * @returns {Promise<void>}
     */
    // async handleBSaver(macroRound) {
    //     // await this.asyncMethodToBeExecutedBeforeSymbolsAreUpdated(macroRound);
    //     await Promise.all([
    //         this.fullSpinForBalanceSaver ? this.grid.spinReels(macroRound.PARAMETERS.grid) : this.grid.updateGrid(macroRound.PARAMETERS.grid, macroRound),
    //         // ^ Updates only symbols that have changed, unless this.fullSpinForBalanceSaver is true.
    //         this.ticker.updateText("Balance Saver activated!")
    //         // this.asyncMethodToBeExecutedAtTheSameTimeAsSymbolsAreUpdated(macroRound);
    //     ]);
    //     // await this.asyncMethodToBeExecutedAfterSymbolsAreUpdated(macroRound);
    //     await this.displayWinnings(macroRound);
    //     if(macroRound.PARAMETERS.freespins_collected) await this.launchFreeSpins(macroRound.PARAMETERS);
    // }

    /**
     * Handles a free game macro round.
     * @param {Object} macroRound - Macro round data.
     * @returns {Promise<void>}
     */
    // async handleFGame(macroRound) {
    //     this.ticker.clearText();
    //     this.eventHistory.addText("~~~~New Freespin~~~~");
    //     this.incrementSessionStat("freespins-played");
    //     await this.freeSpins.spinsLeftTicker.incrementSpins(-1);
    //     await this.grid.spinReels(macroRound.PARAMETERS.grid);
    //     // await this.asyncMethodToBeExecutedAfterSpin(macroRound);
    //     await this.displayFreeSpinWinnings(macroRound);
    //     if(macroRound.PARAMETERS.freespins_collected) await this.awardRetriggerSpins(macroRound.PARAMETERS);
    //     if(!this.freeSpins.spinsLeft) await this.endFreeSpinsSession();
    // }

    /**
     * Launches the free spins round.
     * By default, shows free spins left and free spisn balance tickers, display free spins awarded in main ticker, increments session stats, changes UI background color.
     * @param {Object} params - Parameters from macroRound including number of free spins awarded.
     * @returns {Promise<void>}
     */
    async launchFreeSpins(PARAMETERS) {
        const { freespins_collected } = PARAMETERS;
        await super.launchFreeSpins(PARAMETERS);
        //Add additional free spins setup tasks.
    }

    /**
     * Ends the free spins session and processes total winnings.
     * By default, displays free spins winnings in main ticker, adds winnings to main ticker, increments session stats, changes background color for MGame.
     * @returns {Promise<void>}
     */
    async endFreeSpinsSession() {
        await super.endFreeSpinsSession();
        //Add additional free spins teardown tasks.
    }

    /**
     * Begins a new paid spin. By default, decrements balance, updates session stats, clears main ticker, adds spin seed to text box, makes sure UI is correct for MGame.
     * @param {string} seed - Random seed for the spin.
     */
    startNewSpin(seed) {
        super.startNewSpin(seed);
        //Note: not async so not a suitable place for animations that the rest of the game should wait for.
    }

}