import ShellUI from "./shell/ShellUI.js";
import PhaseUI from "./uiElements/PhaseUI.js";
import spinSound from "./spin.mp3";
import {sound} from "@pixi/sound";
import importImageMap from "./utils/importImageMap.js";
import events from "../thisTitle/events.js";
import ApiClient from "./utils/ApiClient.js";
import GameUI from "../thisTitle/GameUI.js";

export default class Game {

    /**
     * Constructs a new Game instance with the specified configuration.
     *
     * @constructor
     * @param {Object} config - The configuration object for the game.
     * @param {number} config.defaultStake - Default stake/bet amount.
     * @param {number} [config.mainTickerPauseScale=50] - Adjust how long is allowed for user to read ticker updates.
     * @param {Object} config.mainTickerTextFormat - PIXI.TextStyle object for main ticker.
     */
    constructor() {
        return (async () => {
            this.state = {running: false}
            await this.setGlobalConfig();
            this.soundPlayer = this.getSoundPlayer();
            await this.buildShellUI();
            await this.createOrResetUI();
            this.autoPlayOn = false;
            this.numSpins = 0;
            this.freeSpins = {};
            this.simultaneousWinLineAnimations ||= false;
            await this.resetPlayerState();
            this.eventHandlers = events;
            return this;
        })();
    }

    async setGlobalConfig() {
        ({
            title: this.gameTitle,
            minBet: this.minBet,
            rtps: this.rtps,
            betTypes: this.betTypes,
        } = await ApiClient.getGlobalConfig());
        this.balance = 1_000_00;
        this.validBaseBets = 100 % this.minBet === 0 ?
            [...Array(100 / this.minBet - 1).keys().map(i => (i + 1) * this.minBet), 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000]
            : [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000].map(b => b * this.minBet);

    }

    async buildShellUI() {
        this.shell = new ShellUI();
        await this.shell.init(this.rtps, this.betTypes);
        this.setUpBetTypeSelector();
        this.setUpRtpSelector();
    }

    async createOrResetUI() {
        ({
            maths: this.maths,
            forceOptions: this.forceOptions
        } = await ApiClient.getMathsForRtpAndBetType(this.rtp, this.betType));
        if (!this.bet) this.bet = this.minBet * this.maths.betTypeCost;
        if (this.ui) this.ui.destroy();
        await this.createUI();
        const balancePromise = this.ui.balance.update(this.balance);
        this.registerClickCallbacks();
        this.ui.forces.forcesSetup(forceIndex => this.play({forceIndex}), this.forceOptions);
        this.ui.payTable.payTableSetup(this.maths.payTable, this.bet / this.minBet / this.maths.betTypeCost, (symbolName) =>
            this.ui.currentPhase.grid.getSymbolImageOrText(symbolName)
        );
        await balancePromise;
    }

    async createUI() {
        const imageMap = await importImageMap();
        this.ui = new GameUI(this.maths);
        this.ui.init(this.state, this.maths, this.soundPlayer, this.shell, imageMap);
    }

    registerClickCallbacks() {
        this.ui.spinButton.onClick = () => this.play({});
        this.ui.autoPlayButton.onClick = () => {
            this.autoPlayOn = !this.autoPlayOn;
            this.ui.autoPlayButton.changeColour(this.autoPlayOn ? "#37b832" : "#7e32b8");
            if (this.autoPlayOn && !this.state.running) this.ui.spinButton.click();
        }
        this.ui.configPanel.betSelector.lowerBet = (oldBet) => {
            if (this.state.running) return oldBet;
            if (oldBet !== this.bet) throw new Error("Displayed bet mismatch.")
            const oldBetIndex = this.validBaseBets.indexOf(oldBet / this.maths.betTypeCost);
            if (oldBetIndex === 0) return oldBet;
            const newStake = this.validBaseBets[oldBetIndex - 1] * this.maths.betTypeCost;
            this.updateBet(newStake);
            return newStake;
        };
        this.ui.configPanel.betSelector.raiseBet = (oldBet) => {
            if (this.state.running) return oldBet;
            if (oldBet !== this.bet) throw new Error("Displayed bet mismatch.")
            const oldBetIndex = this.validBaseBets.indexOf(oldBet / this.maths.betTypeCost);
            if (oldBetIndex === this.validBaseBets.length - 1) return oldBet;
            const newStake = this.validBaseBets[oldBetIndex + 1] * this.maths.betTypeCost;
            this.updateBet(newStake);
            return newStake;
        };
        this.ui.forces.addMoney = async (value) => {
            await this.ui.balance.increment(value);
        }
        this.ui.forces.setMoney = async (value) => {
            await this.ui.balance.update(value);
        }
    }

    async startNewSpin(seed, forceName) {
        const balancePromise = this.ui.balance.increment(-this.bet);
        this.shell.sessionStats.increment("rounds-played", 1);
        this.shell.sessionStats.increment("total-bet", this.bet / 100);
        const tickerPromise = this.ui.ticker.clearText();
        this.numSpins++;
        this.shell.eventHistory.addText(`****New Paid Spin - SEED: ${seed}${forceName ? ` FORCE: ${forceName}` : ""}****`);
        this.ui.setPhase("base");
        this.state.totalGameRoundWinnings = 0;
        await Promise.all([tickerPromise, balancePromise]);
    }

    async displayWinnings(macroRound) {
    }

    async displayFreeSpinWinnings(macroRound) {
        const winnings = macroRound.TOTAL_WIN;
        if (!winnings) return;
        const updateBalancePromise = this.freeSpins.balanceTicker.incrementBalance(winnings);
        const showWinningsPromise = this.ticker.updateText(`Spin winnings £${(winnings / 100).toFixed(2)}!`);
        await Promise.all([showWinningsPromise, updateBalancePromise]);
    }

    setUpBetTypeSelector() {
        const selector = document.getElementById("bet-type-selector");
        selector.selectedIndex = 0;
        this.betType = selector.value;
        selector.addEventListener("change", async () => {
            await this.setBetType(selector.value);
        });
    }

    setUpRtpSelector() {
        const selector = document.getElementById("rtp-selector");
        selector.selectedIndex = 0;
        this.rtp = parseInt(selector.value);
        selector.addEventListener("change", async () => {
            this.rtp = parseInt(selector.value);
            console.log(this.rtp);
            await this.createOrResetUI();
        })
    }

    async setBetType(betType) {
        const selector = document.getElementById("bet-type-selector");
        if (!this.state.running && Object.keys(this.betTypes).includes(betType)) {
            const previousBetTypeCost = this.betTypes[this.betType];
            this.betType = betType;
            await this.createOrResetUI();
            const oldBetIndex = this.validBaseBets.indexOf(this.bet / previousBetTypeCost);
            const newStake = this.validBaseBets[oldBetIndex] * this.maths.betTypeCost;
            this.updateBet(newStake);
        } else {
            for (let option of selector.options)
                if (this.betType === option.value) selector.selectedIndex = selector.options.indexOf(option);
        }
    }

    async play({forceIndex, seed}) {
        if (this.state.running) return;

        this.state.running = true;
        this.ui.spinButton.changeColour("#bc4120");

        if (!this.session
            || !(this.session.bet === this.bet
                && this.session.betType === this.betType
                && this.session.rtp === this.rtp
            )
        )
            this.session = await ApiClient.getNewSession(this.session, this.bet, this.betType, this.rtp);
        const {
            session,
            events,
            winnings,
            forceName,
            seed: newSeed
        } = await ApiClient.getNewGameCycle(this.session, forceIndex, seed);
        this.session = session;
        await this.displayGameRound(events, winnings, newSeed, forceName);

        this.state.running = false;
        this.ui.spinButton.changeColour(0x0000ff);
        if (this.autoPlayOn) this.ui.ticker.wait(() => this.ui.spinButton.click(), 600);
    }

    async getGameRound(forceIndex) {
        const requestUri = `/api/spin`;
        const request = new Request(requestUri);
        const response = await fetch(request);
        return response.json();
    }

    async displayGameRound(events, winnings, seed, forceName) {
        console.log(events);
        await this.startNewSpin(seed, forceName);
        for (let eventDetails of events) {
            const {eventType} = eventDetails;
            const eventHandler = this.eventHandlers[eventType];
            if (!eventHandler || typeof eventHandler != "function") {
                await this.warnUnhandledEvent(eventType, eventDetails);
            } else {
                await eventHandler(eventDetails, this);
            }
        }
        await this.endGameRound(seed, winnings);
    }

    async endGameRound(seed, winnings) {
        if (winnings !== this.state.totalGameRoundWinnings) throw new Error("Winnings calculated in frontend do not match winnings from server.");
        this.updateHistory(seed);
        if (!this.state.totalGameRoundWinnings) return;
        this.shell.sessionStats.increment("total-won", this.state.totalGameRoundWinnings / 100);
        await Promise.all([
            this.ui.ticker.updateText(`Total game round winnings: £${(this.state.totalGameRoundWinnings / 100).toFixed(2)}!`),
            this.ui.balance.increment(this.state.totalGameRoundWinnings)
        ]);
    }

    async warnUnhandledEvent(eventType, eventDetails) {
        const eventHandler = this.eventHandlers[eventType];
        const notFunction = !!eventHandler;
        if (notFunction) console.log(`Event handler for type ${eventType} is not a function.`);
        else console.log(`No event handler for ${eventType}.`);

        console.log(`In your extended game class, please create handle${String(eventType).charAt(0).toUpperCase() + String(eventType).slice(1)}() method and overwrite getEventHandlers as follows:

    getEventHandlers() {
      const eventHandlers = super.getEventHandlers();
      eventHandlers.${eventType} = this.handle${String(eventType).charAt(0).toUpperCase() + String(eventType).slice(1)};
      return eventHandlers;
    }`);
        console.log('Event info:');
        console.log(eventDetails);

        if (notFunction) await this.ticker.updateText(`${eventType} handler is not a function!`);
        else await this.ticker.updateText(`${eventType} is unhandled!`);

        await this.ticker.updateText("Check console for details.");
    }


    async resetPlayerState() {
        await Promise.all([
            this.ui.balance.update(100000)
        ]);
        this.freeSpins.spinsLeft = 0;
        this.freeSpins.balance = 0;
        this.shell.sessionStats.reset();
    }


    updateHistory(seed) {
        const spinID = this.numSpins;
        var spinWin = this.state.totalGameRoundWinnings;
        spinWin = spinWin / 100;
        const spinBalanceAfter = this.balance / 100;
        const spinBet = this.bet / 100;
        this.shell.spinHistory.addText(["SpinID: " + spinID, "Bet: £" + spinBet.toFixed(2), "Win: £" + spinWin.toFixed(2), "Balance: £" + spinBalanceAfter.toFixed(2), "Seed: " + seed],
            () => this.play({seed}))
    }

    getSoundPlayer() {
        sound.useLegacy = true;

        sound.add("spin", spinSound);

        return {
            play: (soundName) =>
                new Promise((resolve) => {
                    sound.play(soundName, {
                        complete: resolve,
                    });
                }),
            playAsync: async (soundName) => {
                const playingSound = await sound.play(soundName);
                if (playingSound?._source?.play instanceof Function) {
                    const soundPromise = playingSound._source.play();
                    if (soundPromise instanceof Promise) {
                        await soundPromise;
                    }
                }
                return playingSound;
            },
            add: (name, file) => sound.add(name, file),
        };
    }

    updateBet(newBet) {
        this.bet = newBet;
        this.ui.configPanel.betSelector.updateBet(newBet);
        this.ui.payTable.payTableSetup(this.maths.payTable, this.bet / this.minBet / this.maths.betTypeCost, (symbolName) =>
            this.ui.currentPhase.grid.getSymbolImageOrText(symbolName));
    }


}
