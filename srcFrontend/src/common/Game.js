import ShellUI from "./shell/ShellUI.js";
import spinSound from "./spin.mp3";
import {sound} from "@pixi/sound";
import importImageMap from "./utils/importImageMap.js";
import events from "../thisTitle/events.js";
import ApiClient from "./utils/ApiClient.js";
import toCurrency from "./utils/toCurrency.js";
import GameUI from "../thisTitle/GameUI.js";
import Button from "../common/uiElements/Button.js";
import Ticker from "./uiElements/Ticker.js";

export default class Game {

    /**
     * Constructs a new Game instance.
     *
     * @constructor
     */
    constructor() {
        return (async () => {
            this.state = {running: false}
            await this.setGlobalConfig();
            this.soundPlayer = this.getSoundPlayer();
            this.imageMap = await importImageMap();
            await this.buildShellUI();
            await this.getMathsAndCreateUI();
            this.session = await ApiClient.getNewSession(this.session, this.state.bet, this.state.betType, this.state.rtp, this.sid);
            if (this.session && this.session.sid) this.sid = this.session.sid;
            this.state.autoPlayOn = false;
            this.state.numSpins = 0;
            this.freeSpins = {};
            this.simultaneousWinLineAnimations ||= false;
            this.eventHandlers = events;
            return this;
        })();
    }

    async setGlobalConfig() {
        const jsonResponse = await ApiClient.getGlobalConfig(this.sid);
        this.gameTitle = jsonResponse.gameTitle;
        this.minBet = jsonResponse.minBet;
        this.rtps = jsonResponse.rtps;
        this.state.betTypes = jsonResponse.betTypes;
        if (jsonResponse.sid) this.sid = jsonResponse.sid;

        this.state.balance = 1_000_00;
        this.validBaseBets = 100 % this.minBet === 0 ?
            [...Array(100 / this.minBet - 1).keys().map(i => (i + 1) * this.minBet), 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000]
            : [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000].map(b => b * this.minBet);

    }

    async buildShellUI() {
        this.shell = new ShellUI();
        await this.shell.init(this.rtps, this.state.betTypes);
        this.setUpBetTypeSelector();
        this.setUpRtpSelector();
        this.setUpRnStreamButton();
        this.setUpSeedButton();
        this.setUpUploadHandler();
    }

    async getMathsAndCreateUI() {
        await this.getMaths();
        if (!this.state.bet) this.state.bet = 100 * this.maths.betTypeCost;
        this.ui = new GameUI(this.maths);
        await this.ui.init({
            state: this.state,
            maths: this.maths,
            soundPlayer: this.soundPlayer,
            shell: this.shell,
            imageMap: this.imageMap
        });
        this.updateBet(this.state.bet);
        this.registerClickCallbacks();
        this.setupForcesAndPayTable();
        await this.ui.balance.update(this.state.balance);
    }

    setUpBetTypeSelector() {
        /** @type {HTMLSelectElement} */
        const selector = document.getElementById("bet-type-selector");
        selector.selectedIndex = 0;
        this.state.betType = selector.value;
        selector.addEventListener("change", async () => {
            await this.setBetType(selector.value);
            this.session = await ApiClient.getNewSession(this.session, this.state.bet, this.state.betType, this.state.rtp, this.sid);
            if (this.session && this.session.sid) this.sid = this.session.sid;
            await this.ui.updateUiAndStateToMatchSession(this.session, this.state);
        });
    }

    setUpRtpSelector() {
        /** @type {HTMLSelectElement} */
        const selector = document.getElementById("rtp-selector");
        selector.selectedIndex = 0;
        this.state.rtp = parseInt(selector.value);
        selector.addEventListener("change", async () => {
            if (this.state.running) {
                selector.value = this.state.rtp;
                return;
            }
            this.state.rtp = parseInt(selector.value);
            this.ui.destroy();
            await this.getMathsAndCreateUI();
            this.session = await ApiClient.getNewSession(this.session, this.state.bet, this.state.betType, this.state.rtp, this.sid);
            if (this.session && this.session.sid) this.sid = this.session.sid;
            await this.ui.updateUiAndStateToMatchSession(this.session, this.state);
        })
    }

    setUpRnStreamButton() {
        /** @type {HTMLButtonElement} */
        const rnStreamForm = document.getElementById("rn-stream-form");
        rnStreamForm.addEventListener("submit", (async (e) => {
            e.preventDefault();
            if (this.running) return;
            /** @type {HTMLInputElement} */
            const rnStreamBox = document.getElementById("rn-stream");
            let rnStream = rnStreamBox.value;
            if (!rnStream.length) return;
            this.session = JSON.parse(rnStream.split("|")[0]);
            this.state.rtp = this.session.rtp;
            document.getElementById("rtp-selector").value = this.state.rtp;
            await this.setBetType(this.session.betType);
            //this.session = await ApiClient.getNewSession(this.session, this.session.bet, this.session.betType, this.state.rtp);
            await this.ui.updateUiAndStateToMatchSession(this.session, this.state);
            rnStream = rnStream.split("|").slice(1).join("|");
            await this.play({rnStream});
        }).bind(this));
    }

    setUpSeedButton() {
        /** @type {HTMLFormElement} */
        const seedForm = document.getElementById("seed-form");
        if (!seedForm) return;
        seedForm.addEventListener("submit", (async (e) => {
            e.preventDefault();
            if (this.state.running) return;
            /** @type {HTMLInputElement} */
            const seedBox = document.getElementById("seed-entry");
            if (!seedBox) return;
            const raw = (seedBox.value || "").trim();
            if (!raw.length) return;
            // Support hex (0x...), decimal, and numeric strings
            let seed;
            if (/^0x[0-9a-f]+$/i.test(raw)) seed = parseInt(raw, 16);
            else seed = parseInt(raw, 10);
            if (!Number.isFinite(seed) || seed < 0) return;
            await this.play({seed});
        }).bind(this));
    }

    async setBetType(betType) {
        /** @type {HTMLSelectElement} */
        const selector = document.getElementById("bet-type-selector");
        if (!this.state.running && Object.keys(this.state.betTypes).includes(betType)) {
            const previousBetTypeCost = this.state.betTypes[this.state.betType];
            this.state.betType = betType;
            this.ui.destroy();
            await this.getMathsAndCreateUI();
            const oldBetIndex = this.validBaseBets.indexOf(this.state.bet / previousBetTypeCost);
            const newStake = this.validBaseBets[oldBetIndex] * this.maths.betTypeCost;
            this.updateBet(newStake);
            if (selector.value !== betType) selector.value = betType;
        } else {
            selector.value = this.state.betType;
        }
    }

    async getMaths() {
        const {
            maths,
            forceOptions
        } = await ApiClient.getMathsAndForcesForRtpAndBetType(this.state.rtp, this.state.betType, this.sid);
        this.maths = maths;
        this.forceOptions = forceOptions;
    }

    setupForcesAndPayTable() {
        this.ui.forces.forcesSetup(forceIndex => this.play({forceIndex}), this.forceOptions);
        this.ui.payTable.payTableSetup(this.maths.payTable, this.state.bet / this.minBet / this.maths.betTypeCost, (symbolName) =>
            this.ui.currentPhase.grid.getSymbolImageOrText(symbolName)
        );
    }

    registerClickCallbacks() {
        this.ui.spinButton.onClick = () => this.play({});
        this.ui.autoPlayButton.onClick = () => {
            this.state.autoPlayOn = !this.state.autoPlayOn;
            this.ui.autoPlayButton.changeColour(this.state.autoPlayOn ? "#37b832" : "#7e32b8");
            if (this.state.autoPlayOn && !this.state.running) this.ui.spinButton.click();
        }
        this.ui.configPanel.betSelector.lowerBet = (oldBet) => {
            if (this.state.running) return oldBet;
            if (oldBet !== this.state.bet) throw new Error("Displayed bet mismatch.")
            const oldBetIndex = this.validBaseBets.indexOf(oldBet / this.maths.betTypeCost);
            if (oldBetIndex === 0) return oldBet;
            const newStake = this.validBaseBets[oldBetIndex - 1] * this.maths.betTypeCost;
            this.updateBet(newStake);
            return newStake;
        };
        this.ui.configPanel.betSelector.raiseBet = (oldBet) => {
            if (this.state.running) return oldBet;
            if (oldBet !== this.state.bet) throw new Error("Displayed bet mismatch.")
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

    async play({forceIndex, seed, rnStream}) {
        if (this.state.running) return;

        this.state.running = true;
        this.ui.spinButton.changeColour("#bc4120");

        if (!this.session
            || !rnStream && !(this.session.bet === this.state.bet
                && this.session.betType === this.state.betType
                && this.session.rtp === this.state.rtp
            )
        )
            this.session = await ApiClient.getNewSession(this.session, this.state.bet, this.state.betType, this.state.rtp, this.sid);
        if (this.session && this.session.sid) this.sid = this.session.sid;

        try {
            const {
                session,
                events,
                winnings,
                forceName,
                seed: newSeed
            } = await ApiClient.getNewGameCycle(this.session, forceIndex, seed, rnStream, this.sid);
            this.session = session;
            await this.ui.updateUiAndStateToMatchSession(session, this.state);
            await this.displayGameRound(events, winnings, newSeed, forceName);
        } catch (e) {
            await this.ui.ticker.updateText(e.message);
            console.error(e);
        }


        this.state.running = false;
        this.ui.spinButton.changeColour(0x0000ff);
        if (this.state.autoPlayOn) Ticker.wait(() => this.ui.spinButton.click(), 600);
    }

    setUpUploadHandler() {
        /** @type {HTMLInputElement} */
        const input = document.getElementById("par-upload-input");
        /** @type {HTMLButtonElement} */
        const button = document.getElementById("par-upload-button");
        if (!input || !button) return;
        const toBase64 = (file) => new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = () => {
                const result = reader.result || "";
                const comma = typeof result === "string" ? result.indexOf(",") : -1;
                // If readAsDataURL was used, take the part after the comma (already base64-encoded)
                if (comma >= 0) return resolve(result.substring(comma + 1));
                // Fallback (shouldn't happen with readAsDataURL): return empty string
                resolve("");
            };
            reader.onerror = reject;
            reader.readAsDataURL(file);
        });
        button.addEventListener("click", async () => {
            if (!input.files || input.files.length === 0) return;
            const file = input.files[0];
            try {
                // Ensure we have a session id first
                if (!this.sid) {
                    const s = await ApiClient.getNewSession(this.session, this.state.bet, this.state.betType, this.state.rtp, this.sid);
                    if (s && s.sid) this.sid = s.sid;
                    this.session = s;
                }
                const dataBase64 = await toBase64(file);
                const resp = await ApiClient.uploadParSheet(this.sid, file.name, dataBase64);
                // Update options for this session
                if (resp.sid) this.sid = resp.sid;
                if (resp.rtps) this.rtps = resp.rtps;
                if (resp.betTypes) this.state.betTypes = resp.betTypes;
                await this.shell.init(this.rtps, this.state.betTypes);
                // Reset rtp/betType to defaults from new sheet
                const rtpSelector = document.getElementById("rtp-selector");
                if (rtpSelector && resp.rtps && resp.rtps.length > 0) {
                    rtpSelector.selectedIndex = 0;
                    this.state.rtp = parseInt(rtpSelector.value);
                }
                const betTypeSelector = document.getElementById("bet-type-selector");
                if (betTypeSelector) {
                    betTypeSelector.selectedIndex = 0;
                    this.state.betType = betTypeSelector.value;
                }
                // Rebuild UI with new maths
                this.ui.destroy();
                await this.getMathsAndCreateUI();
                this.session = await ApiClient.getNewSession(this.session, this.state.bet, this.state.betType, this.state.rtp, this.sid);
                if (this.session && this.session.sid) this.sid = this.session.sid;
                await this.ui.updateUiAndStateToMatchSession(this.session, this.state);
                await this.ui.ticker.updateText("Uploaded maths applied to this session.");
            } catch (e) {
                console.error(e);
                await this.ui.ticker.updateText(e.message || String(e));
            }
        });
    }

    async displayGameRound(events, winnings, seed, forceName) {
        console.log(events);
        const startSpinPromise = this.startNewSpin(seed, forceName);
        for (let eventDetails of events) {
            const {eventType} = eventDetails;
            const eventHandler = this.eventHandlers[eventType];
            if (!eventHandler || typeof eventHandler != "function") {
                await this.warnUnhandledEvent(eventType, eventDetails);
            } else {
                await Promise.all([
                    startSpinPromise,
                    eventHandler(eventDetails, this)
                ]);
            }
        }
        await this.endGameRound(seed, winnings);
    }

    async startNewSpin(seed, forceName) {
        this.state.balance -= this.state.bet;
        const balancePromise = this.ui.balance.increment(-this.state.bet);
        this.shell.sessionStats.increment("rounds-played", 1);
        this.shell.sessionStats.increment("total-bet", this.state.bet);
        const tickerPromise = this.ui.ticker.clearText();
        this.state.numSpins++;
        this.shell.eventHistory.addText(`****New Paid Spin - SEED: ${seed}${forceName ? ` FORCE: ${forceName}` : ""}****`, () => this.play({seed}));
        await this.ui.setPhase("base", true);
        this.state.totalGameRoundWinnings = 0;
        return Promise.all([tickerPromise, balancePromise]);
    }

    async endGameRound(seed, winnings) {
        await this.ui.endCurrentPhase();
        if (winnings !== this.state.totalGameRoundWinnings) {
            await this.ui.ticker.updateText("Winnings calculated in frontend \ndo not match winnings from server.");
            await this.ui.ticker.updateText("Make sure all winnings are added using \ngame.ui.currentPhase.balance.increment(winnings).");
            await this.ui.ticker.updateText("Server winnings: " + toCurrency(winnings) + ". Frontend winnings: " + toCurrency(this.state.totalGameRoundWinnings) + ".");
            throw new Error("Winnings calculated in frontend do not match winnings from server.");
        }
        if (this.state.totalGameRoundWinnings) {
            this.shell.sessionStats.increment("total-won", this.state.totalGameRoundWinnings);
            await Promise.all([
                this.ui.ticker.updateText(`Total game round winnings: ${toCurrency(this.state.totalGameRoundWinnings)}!`),
                this.state.balance += this.state.totalGameRoundWinnings,
                this.ui.balance.increment(this.state.totalGameRoundWinnings),
                this.ui.currentPhase.balance.update(0),
                (async () => this.updateHistory(seed))()
            ]);
        } else
            this.updateHistory(seed)
    }

    async warnUnhandledEvent(eventType, eventDetails) {
        const eventHandler = this.eventHandlers[eventType];
        const notFunction = !!eventHandler;
        if (notFunction) console.log(`Event handler for type ${eventType} is not a function.`);
        else console.log(`No event handler for ${eventType}.`);

        console.log(`In 'srcFrontend/src/thisTitle.events.js', please add a new entry like this:

    \"${eventType}\": async ({${Object.keys(eventDetails).filter(k => k !== "eventType").join(", ")}}, game) => {
      // your event handler code here.
    },`);

        console.log('Event info:');
        console.log(eventDetails);

        if (notFunction) await this.ui.ticker.updateText(`${eventType} event handler is not a function!`);
        else await this.ui.ticker.updateText(`${eventType} event is unhandled!`);

        await this.ui.ticker.updateText("Check console for details.");
    }


    updateHistory(seed) {
        const {numSpins, bet, totalGameRoundWinnings, balance} = this.state;
        this.shell.spinHistory.addText([
                "SpinID: " + numSpins, "Bet: " + toCurrency(bet), "Win: " + toCurrency(totalGameRoundWinnings), "Balance: " + toCurrency(balance), "Seed: " + seed],
            () => this.play({seed}))
    }

    getSoundPlayer() {
        const audioCache = new Map();

        const getOrCreateAudio = (name, url) => {
            if (!audioCache.has(name)) {
                const audio = new Audio(url);
                audio.preload = "auto";
                audio.load();
                audioCache.set(name, audio);
            }
            return audioCache.get(name);
        };

        // Pre-cache the spin sound
        getOrCreateAudio("spin", spinSound);

        return {
            play: (soundName) =>
                new Promise((resolve) => {
                    const audio = getOrCreateAudio(soundName);
                    audio.currentTime = 0;
                    audio.onended = () => resolve();
                    audio.play().catch(resolve);
                }),
            playAsync: async (soundName) => {
                const audio = getOrCreateAudio(soundName);
                audio.currentTime = 0;
                await audio.play();
                return audio;
            },
            add: (name, file) => getOrCreateAudio(file),
            stop: (soundName) => {
                const audio = audioCache.get(soundName);
                if (audio) {
                    audio.pause();
                    audio.currentTime = 0;
                }
            },
        };
    }

    updateBet(newBet) {
        this.state.bet = newBet;
        this.ui.configPanel.betSelector.updateBet(newBet);
        this.ui.payTable.payTableSetup(this.maths.payTable, this.state.bet / this.minBet / this.maths.betTypeCost, (symbolName) =>
            this.ui.currentPhase.grid.getSymbolImageOrText(symbolName));
    }

    async reportRnStreamProblem(shortMessage, longMessage) {
        this.ui.spinButton.buttonContainer.eventMode = "none";
        let continuePressed = false;
        const continueButton = new Button({
            x: this.ui.spinButton.buttonContainer.x,
            y: this.ui.spinButton.buttonContainer.y,
            width: this.ui.spinButton.buttonContainer.width,
            height: this.ui.spinButton.buttonContainer.height,
            parent: this.ui.spinButton.buttonContainer.parent,
            colour: "#FF0000",
            onClick: () => {
                continueButton.buttonContainer.destroy();
                continuePressed = true
                this.ui.spinButton.buttonContainer.eventMode = "static";
            },
            text: "Continue"
        });
        await this.ui.ticker.updateText(shortMessage);
        longMessage.split("\n").forEach(line =>
            this.shell.eventHistory.addText(line));
        await this.ui.ticker.updateText("Check event history for details.", false, undefined, true);
        await this.ui.ticker.waitUntilFalse(() => !continuePressed, 100);

    }

}
