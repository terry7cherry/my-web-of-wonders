import Grid from "./Grid.js";
import Ticker from "./Ticker.js";
import ScrollingTextBox from "./ScrollingTextBox.js"
import PayTable from "./PayTable.js";
import BetSelector from "./BetSelector.js"
import Cheats from "./Cheats.js";
import Button from "./Button.js";
import NiceText from "./NiceText.js";
import spinSound from "./spin.mp3";
import {sound} from "@pixi/sound";
import * as PIXI from "pixi.js";

export default class Game {
    /**
     * Constructs a new Game instance with the specified configuration.
     *
     * @constructor
     * @param {Object} config - The configuration object for the game. All optional except displayReelSet.
     * @param {number} config.gridWidth - Width of the reel grid.
     * @param {number} config.gridHeight - Height of the reel grid.
     * @param {number} config.gridX - X-position of the reel grid.
     * @param {number} config.gridY - Y-position of the reel grid.
     * @param {number} config.reelWidth - Width of each reel.
     * @param {number} config.symbolHeight - Height of each symbol.
     * @param {Array} config.displayReelSet - Reels to use for spinning animations and starting grid. Object with 2D array property named "bands".
     * @param {Function} config.symbolColorMapper - Where no symbol images (imageMap) is provided. Function that takes symbolCode and returns color in hex format (#ffffff).
     * @param {number} config.uiWidth - Width of the full main UI (PIXI app) including side panels.
     * @param {number} config.uiHeight - Height of the full main UI (PIXI app) including top/bottom panels.
     * @param {number} config.topPanelHeight - Height of the top UI panel.
     * @param {number} config.bottomPanelHeight - Height of the bottom UI panel.
     * @param {number} config.leftPanelWidth - Width of the left UI panel.
     * @param {number} config.rightPanelWidth - Width of the right UI panel.
     * @param {number} config.topPanelColor - Color of the top panel.
     * @param {number} config.bottomPanelColor - Color of the bottom panel.
     * @param {number} config.leftPanelColor - Color of the left panel.
     * @param {number} config.rightPanelColor - Color of the right panel.
     * @param {string} [config.backgroundColor=0x1099bb] - PIXI app background color for MGame etc.
     * @param {string} [config.freeSpinsBackgroundColor=0x11a878] - PIXI app Background color during free spins.
     * @param {string} [config.gameName="Prototype"] - Name of the game. Appears as title of app.
     * @param {number} config.titleWidth - Width of the game title.
     * @param {number} config.titleHeight - Height of the game title.
     * @param {number} config.titleX - X position of the game title.
     * @param {number} config.titleY - Y position of the game title.
     * @param {number} [config.titleColor=0xb02525] - Color of the title text.
     * @param {Function} config.reelSpinGridTransformer - To adjust grid delivered by server before reel spin. Function that takes 2D array of symbols from server and returns the mutated array.
     * @param {boolean} config.fullSpinForBalanceSaver - If true, balance saver activation will result in full reelspin animation. Otherwise, only symbols that have changed from the MGame grid will be animated.
     * @param {Object} config.symbolMap - When no symbol images (imageMap) provided. Dictionary of symbol integer codes (0, 1, 2, etc.) mapped to friendlier strings ("WI", "HP3", etc.)
     * @param {Object} config.imageMap - Dictionary of integer symbol codes mapped to image resources. (0: await PIXI.Assets.load(importedPNGFile))
     * @param {number} config.defaultStake - Default stake/bet amount.
     * @param {number} [config.mainTickerPauseScale=50] - Adjust how long is allowed for user to read ticker updates.
     * @param {Object} config.mainTickerTextFormat - PIXI.TextStyle object for main ticker.
     * @param {number} config.tickerTextColor - Color of text in tickers.
     * @param {string} config.suspenseSymbolCode - Integer symbol codes that should triggers suspense animations when 2 or more are present (Codes mapped to "BN" and "SC" included by default).
     * @param {boolean} config.simultaneousWinLineAnimations - Determines if win lines should be displayed sequentially or all at once. Defaults to true if more than 20 win lines present in game.
     * @param {string} [config.winLineColor=0xf80000] - Color for win line animations.
     */
    constructor({
                    gridWidth,
                    gridHeight,
                    gridX,
                    gridY,
                    reelWidth,
                    symbolHeight,
                    displayReelSet,
                    weightedDisplayReelSet,
                    symbolColorMapper,
                    uiWidth,
                    uiHeight,
                    topPanelHeight,
                    bottomPanelHeight,
                    leftPanelWidth,
                    rightPanelWidth,
                    topPanelColor,
                    bottomPanelColor,
                    leftPanelColor,
                    rightPanelColor,
                    backgroundColor = 0x1099bb,
                    freeSpinsBackgroundColor = 0x11a878,
                    gameName = "Prototype",
                    titleWidth,
                    titleHeight,
                    titleX,
                    titleY,
                    titleColor = 0xb02525,
                    reelSpinGridTransformer,
                    fullSpinForBalanceSaver,
                    symbolMap,
                    imageMap,
                    defaultStake,
                    mainTickerPauseScale = 50,
                    mainTickerTextFormat,
                    tickerTextColor,
                    suspenseSymbolCode,
                    simultaneousWinLineAnimations,
                    winLineColor = 0xf80000,
                }) {
        return (async () => {
            this.running = false;
            this.autoPlayOn = false;
            this.app = new PIXI.Application({});
            this.numSpins = 0;
            this.numReels = 5;
            this.numRows = 3;
            this.gridWidth = gridWidth;
            this.gridHeight = gridHeight;
            this.reelWidth = reelWidth;
            this.symbolHeight = symbolHeight;
            this.uiWidth = uiWidth;
            this.uiHeight = uiHeight;
            this.gameName = gameName;
            this.serverTotalBet = 25;
            this.stake = defaultStake ?? 100;
            this.stakeMultiplier = this.stake / this.serverTotalBet;
            this.tickerTextColor = tickerTextColor;
            this.topPanelHeight = topPanelHeight ?? 0;
            this.bottomPanelHeight = bottomPanelHeight ?? 0;
            this.leftPanelWidth = leftPanelWidth ?? 0;
            this.rightPanelWidth = rightPanelWidth ?? 0;
            this.topPanelColor = topPanelColor;
            this.bottomPanelColor = bottomPanelColor;
            this.leftPanelColor = leftPanelColor;
            this.rightPanelColor = rightPanelColor;
            this.titleHeight = titleHeight ?? 100;
            this.titleY = titleY ?? 0;
            if (!gridWidth && !reelWidth) this.reelWidth = 160;
            if (!gridWidth) this.gridWidth = this.reelWidth * this.numReels;
            if (!uiWidth) this.uiWidth = this.gridWidth + this.leftPanelWidth + this.rightPanelWidth;
            this.titleWidth = titleWidth ?? this.uiWidth - 100;
            this.titleX = titleX ?? (this.uiWidth - this.titleWidth) / 2;
            if (!reelWidth) this.reelWidth = this.gridWidth / this.numReels;
            if (!gridHeight && !symbolHeight) this.symbolHeight = 160;
            if (!gridHeight) this.gridHeight = this.symbolHeight * this.numRows;
            if (!symbolHeight) this.symbolHeight = this.gridHeight / this.numRows;
            if (!uiHeight) this.uiHeight = this.gridHeight + 150 + this.titleHeight + this.topPanelHeight + this.bottomPanelHeight;
            this.gridX = gridX ?? 0 + this.leftPanelWidth;
            this.gridY = gridY ?? this.titleHeight + this.topPanelHeight;
            this.displayReelSet = displayReelSet;
            this.weightedDisplayedReelSet = weightedDisplayReelSet;
            this.symbolColorMapper = symbolColorMapper;
            this.symbolMap = symbolMap;
            this.imageMap = imageMap;
            this.reelSpinGridTransformer = reelSpinGridTransformer;
            this.backgroundColor = backgroundColor;
            this.freeSpinsBackgroundColor = freeSpinsBackgroundColor;
            this.titleColor = titleColor;
            this.freeSpins = {};
            this.suspenseSymbolCode = suspenseSymbolCode;
            this.fullSpinForBalanceSaver = fullSpinForBalanceSaver;
            this.simultaneousWinLineAnimations = simultaneousWinLineAnimations;
            this.winLineColor = winLineColor;
            this.simultaneousWinLineAnimations ||= false;
            this.mainTickerPauseScale = mainTickerPauseScale;
            this.mainTickerTextFormat = mainTickerTextFormat ?? {
                fontFamily: "Arial",
                fontSize: 100,
                fill: 0xffffff,
            };
            this.soundPlayer = this.getSoundPlayer();
            this.resetPlayerState();
            document.getElementById("session-reset-button").onclick = this.resetSessionStats;
            this.setupPlaybackControls();
            this.eventHandlers = this.getEventHandlers();
            this.buildUI();
            //this.setBetType(0);
            return this;
        })();
    }

    getEventHandlers() {
        const eventHandlers = {
            reelSpin: this.handleReelSpin,
            winAnalysis: this.handleWinAnalysis,
            freeSpinsTrigger: this.handleFreeSpinsTrigger,
            freeSpinsRetrigger: this.handleFreeSpinsRetrigger,
            freeSpinsEnd: this.handleFreeSpinsEnd,
            newFreeSpin: this.handleNewFreeSpin,
            gridUpdate: this.handleGridUpdate,
            gameCycleComplete: () => {
            },
        };
        return eventHandlers;
    }

    async handleReelSpin(eventDetails) {
        await this.grid.spinReels(eventDetails.reelSymbolArray);
    }

    async handleGridUpdate({reelSymbolArray, message}) {
        const promises = [];
        promises.push(this.ticker.updateText(message));
        promises.push(this.grid.updateGrid(reelSymbolArray));
        await Promise.all(promises);
    }

    async handleWinAnalysis({totalWin, analysisStage, wins}) {
        const winnings = totalWin * this.stakeMultiplier;
        if (!winnings) return;
        this.totalSpinsWinnings += winnings;
        const animateWinLinesPromise = this.grid.animateWinLines(wins);
        this.eventHistory.addText("Wins:")
        for (const {symbol, oak, multiplier, win, totalWin, winType} of wins) {
            this.eventHistory.addText(
                `&#9;- ${winType[0].toUpperCase() + winType.slice(1)} win: ${this.symbolMap[symbol]}x${oak} for $${((win * this.stakeMultiplier) / 100).toFixed(2)}${multiplier != 1 ? ` x ${multiplier} = $${((totalWin * this.stakeMultiplier) / 100).toFixed(2)}` : ``}`
            );
        }
        let showWinningsPromise, updateBalancePromise;
        switch (analysisStage) {
            case "base game":
                this.totalGameRoundWinnings += winnings;
                updateBalancePromise = this.incrementBalance(winnings);
                showWinningsPromise = this.ticker.updateText(`You won £${(winnings / 100).toFixed(2)} from ${analysisStage}!`);
                await Promise.all([showWinningsPromise, updateBalancePromise, animateWinLinesPromise]);
                this.incrementSessionStat("total-won", winnings / 100);
                break;
            case "free spin":
                updateBalancePromise = this.freeSpins.balanceTicker.incrementBalance(winnings);
                showWinningsPromise = this.ticker.updateText(`Spin winnings £${(winnings / 100).toFixed(2)}!`);
                await Promise.all([showWinningsPromise, updateBalancePromise, animateWinLinesPromise]);
                break;
        }
    }

    async handleMGame(macroRound) {
        await this.grid.spinReels(macroRound.PARAMETERS.grid);
        await this.displayWinnings(macroRound);
        if (macroRound.PARAMETERS.freespins_collected) await this.launchFreeSpins(macroRound.PARAMETERS);
    }

    async handleBSaver(macroRound) {
        await Promise.all([
            this.fullSpinForBalanceSaver ? this.grid.spinReels(macroRound.PARAMETERS.grid) : this.grid.updateGrid(macroRound.PARAMETERS.grid),
            this.ticker.updateText("Balance Saver activated!")
        ]);
        await this.displayWinnings(macroRound);
        if (macroRound.PARAMETERS.freespins_collected) await this.launchFreeSpins(macroRound.PARAMETERS);
    }

    async handleNewFreeSpin({multiplier}) {
        await this.ticker.clearText();
        this.eventHistory.addText("~~~~New Freespin~~~~");
        if (multiplier > 1) this.ticker.updateText(`${multiplier}x multiplier for spin!`)
        this.incrementSessionStat("freespins-played");
        await this.freeSpins.spinsLeftTicker.incrementSpins(-1);
    }

    async handleFGame(macroRound) {
        this.ticker.clearText();
        this.eventHistory.addText("~~~~New Freespin~~~~");
        this.incrementSessionStat("freespins-played");
        await this.freeSpins.spinsLeftTicker.incrementSpins(-1);
        await this.grid.spinReels(macroRound.PARAMETERS.grid);
        await this.displayFreeSpinWinnings(macroRound);
        if (macroRound.PARAMETERS.freespins_collected) await this.awardRetriggerSpins(macroRound.PARAMETERS);
        if (!this.freeSpins.spinsLeft) await this.endFreeSpinsSession();
    }

    async handleFreeSpinsTrigger({spins, multiplier}) {
        this.freeSpins.spinsLeft = spins;
        const {spinsLeftTicker, balanceTicker} = this.freeSpins;
        spinsLeftTicker.updateSpins();
        spinsLeftTicker.setVisible(true);
        balanceTicker.updateBalance();
        balanceTicker.setVisible(true);
        await this.ticker.updateText(`${spins} free spins awarded${multiplier != 1 ? `with x${multiplier} multiplier` : ``}!`);
        this.app.renderer.background.color = this.freeSpinsBackgroundColor;
        this.incrementSessionStat("freespins-sessions");
    }

    async handleFreeSpinsEnd({totalWin}) {
        await this.ticker.updateText("Free spins over!")
        const winnings = totalWin * this.stakeMultiplier;
        if (winnings != this.freeSpins.balance) {
            this.eventHistory.addText(`Server: $${(winnings / 100).toFixed(2)}`)
            this.eventHistory.addText(`Frontend: $${(this.freeSpins.balance / 100).toFixed(2)}`)
            await this.ticker.updateText("Free spins total win mismatch.")
            throw new Error("Free spins total win mismatch.");
        }
        const promises = [
            this.ticker.updateText(`Free spins total: £${(this.freeSpins.balance / 100).toFixed(2)}!`),
            this.incrementBalance(this.freeSpins.balance),
        ];

        this.incrementSessionStat("total-won", this.freeSpins.balance / 100);
        this.totalGameRoundWinnings += this.freeSpins.balance;
        this.freeSpins.balance = 0;
        promises.push(this.freeSpins.balanceTicker.updateBalance());
        await Promise.all(promises);
        this.app.renderer.background.color = this.backgroundColor;
    }

    async handleFreeSpinsRetrigger({spins, multiplier}) {
        const tickerPromise = this.ticker.updateText(`${spins} retrigger spins awarded${multiplier != 1 ? `with x${multiplier} multiplier` : ``}!`)
        const updateSpinsPromise = this.freeSpins.spinsLeftTicker.incrementSpins(spins);
        await Promise.all([tickerPromise, updateSpinsPromise]);
    }

    async startNewSpin(seed) {
        this.incrementBalance(-this.stake);
        this.incrementSessionStat("rounds-played", 1);
        this.incrementSessionStat("total-bet", this.stake / 100);
        this.ticker.clearText();
        this.numSpins++;
        this.eventHistory.addText(`****New Paid Spin - SEED: ${seed}****`);
        this.app.renderer.background.color = this.backgroundColor;
        this.totalGameRoundWinnings = 0;
        this.freeSpins.balance = 0;
        this.freeSpins.spinsLeft = 0;
        this.freeSpins.balanceTicker.setVisible(false);
        this.freeSpins.spinsLeftTicker.setVisible(false);
    }

    async displayWinnings(macroRound) {
    }

    async displayFreeSpinWinnings(macroRound) {
        const winnings = macroRound.TOTAL_WIN * this.stakeMultiplier;
        if (!winnings) return;
        const updateBalancePromise = this.freeSpins.balanceTicker.incrementBalance(winnings);
        const showWinningsPromise = this.ticker.updateText(`Spin winnings £${(winnings / 100).toFixed(2)}!`);
        await Promise.all([showWinningsPromise, updateBalancePromise]);
    }

    async establishBetTypes() {
        const betTypesRequest = new Request("/get-possible-types_and-modes");
        const res = await fetch(betTypesRequest).then(res => res.json());
        this.betTypesAndModes = res;
        this.betTypes = Object.keys(res);
        console.log(this.betTypes);
    }

    async retrieveValues() {
        const gameInfoRequest = new Request("/game-specifications");
        const res = await fetch(gameInfoRequest).then(res => res.json());
        this.serverProvidedConfig = new Array();
        for (const betType of this.betTypes) {
            var gameInfoReq = new Request("/" + betType + "/game-specifications");
            var result = await fetch(gameInfoReq).then(res => res.json());
            this.serverProvidedConfig.push(result);
            document.getElementById("bet-type").add(new Option(betType));
        }
        return res;
    }


    betTypeSelectorSetup() {
        const selector = document.getElementById("bet-type");
        selector.selectedIndex = 0;
        this.currentBetType = this.betTypes[0];
        selector.addEventListener("change", () => {
            this.setBetType(selector.selectedIndex);
        });
    }

    setBetType(betTypeIndex) {
        const selector = document.getElementById("bet-type");
        if (!this.running && betTypeIndex < this.betTypes.length) {
            this.currentBetType = this.betTypes[betTypeIndex];
            this.cheats.cheatsSetup();
            const oldStakeIndex = this.validBets.indexOf(this.stake);
            this.validBets = this.serverProvidedConfig[betTypeIndex].general.betSizes;
            this.serverTotalBet = this.serverProvidedConfig[0].general.TotalBet;
            const newStake = this.validBets[oldStakeIndex];
            this.updateStake(newStake);
        } else {
            for (let choice = 0; choice < this.betTypes.length; choice++) {
                if (this.currentBetType == this.betTypes[choice]) selector.selectedIndex = choice;
            }
        }
    }

    async play(gameMode) {
        if (this.running) return;

        this.running = true;
        this.spinButton.changeColor("#bc4120");

        this.gameRound = await this.getGameRound(gameMode);
        await this.displayGameRound(this.gameRound);

        this.running = false;
        this.spinButton.changeColor(0x0000ff);
        if (this.autoPlayOn) this.ticker.wait(() => this.spinButton.click(), 600);
    }

    async getGameRound(gameMode) {
        const requestUri = `/api/spin`;
        const request = new Request(requestUri);
        const response = await fetch(request);
        return response.json();
    }

    async displayGameRound(gameRound) {
        console.log(gameRound);
        await this.startNewSpin("rand");
        for (let [eventType, eventDetails] of gameRound.events) {
            const eventHandler = this.eventHandlers[eventType];
            if (!eventHandler || typeof eventHandler != "function") {
                await this.warnUnhandledEvent(eventType, eventDetails);
            } else {
                await eventHandler.bind(this)(eventDetails);
            }
        }
        await this.endGameRound();
    }

    async endGameRound() {
        this.updateRTP();
        this.updateHistory();
        if (!this.totalGameRoundWinnings) return;
        await this.ticker.updateText(`Total game round winnings: £${(this.totalGameRoundWinnings / 100).toFixed(2)}!`);
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


    resetPlayerState() {
        this.balance = 100000;
        this.freeSpins.spinsLeft = 0;
        this.freeSpins.balance = 0;
        this.resetSessionStats();
    }

    resetSessionStats() {
        document.querySelectorAll(".session-data").forEach((el) => {
            if (el.classList.contains("currency")) {
                el.numberValue = 0;
                el.value = "£0.00";
            } else if (el.classList.contains("percent")) {
                el.value = "";
            } else {
                el.value = 0;
            }
        });
    }

    incrementSessionStat(stat, increment) {
        const input = document.getElementById("session-" + stat);
        const isCurrency = input.classList.contains("currency");
        if (isCurrency) {
            input.numberValue = Number(input.numberValue) + (increment ?? 1);
            input.value = "£" + input.numberValue.toFixed(2);
        } else {
            input.value = Number(input.value) + (increment ?? 1);
        }
    }

    updateRTP() {
        const totalBet = document.getElementById("session-total-bet").numberValue;
        const totalWon = document.getElementById("session-total-won").numberValue;
        const rtp = totalWon / totalBet;
        document.getElementById("session-rtp").value = (rtp * 100).toFixed(2) + "%";
    }

    updateHistory() {
        const historyBox = document.getElementById("spinHistory");
        const spinID = this.numSpins;
        const spinSeed = this.gameRound.SEED;
        var spinWin = this.totalGameRoundWinnings;
        spinWin = spinWin / 100;
        const spinBalanceAfter = this.balance / 100;
        const spinBet = this.stake / 100;
        this.spinHistory.addText(["SpinID: " + spinID, "Bet: £" + spinBet.toFixed(2), "Win: £" + spinWin.toFixed(2), "Balance: £" + spinBalanceAfter.toFixed(2), "Seed: " + spinSeed])
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

    updateStake(newStake) {
        this.stake = newStake;
        this.stakeMultiplier = this.stake / this.serverTotalBet;
        this.betSelector.updateStake(newStake);
        this.payTable.payTableSetup(this.basePayTable, this.stakeMultiplier);
    }


    setupPlaybackControls() {
        document.getElementById("pause").onclick = () => PIXI.Ticker.shared.stop();
        document.getElementById("play").onclick = () => PIXI.Ticker.shared.start();
        document.getElementById("half-speed").onclick = () => PIXI.Ticker.shared.speed = 0.5;
        document.getElementById("quarter-speed").onclick = () => PIXI.Ticker.shared.speed = 0.25;
        document.getElementById("double-speed").onclick = () => PIXI.Ticker.shared.speed = 2;
        document.getElementById("quadrupal-speed").onclick = () => PIXI.Ticker.shared.speed = 4;
        document.getElementById("twenty-speed").onclick = () => PIXI.Ticker.shared.speed = 20;
        document.getElementById("max-speed").onclick = () => PIXI.Ticker.shared.speed = 10000;
        document.getElementById("full-speed").onclick = () => PIXI.Ticker.shared.speed = 1;
    }

    buildUI() {
        this.app = new PIXI.Application({
            width: this.uiWidth,
            height: this.uiHeight,
            backgroundColor: this.backgroundColor,
        });

        document.getElementById("game-container").appendChild(this.app.view);

        this.titleText = new NiceText({
            app: this.app,
            text: this.gameName,
            width: this.titleWidth,
            height: this.titleHeight,
            x: this.titleX,
            y: this.titleY,
            textColor: this.titleColor,
        });

        this.gridContainer = new PIXI.Container();
        this.gridContainer.x = this.gridX;
        this.gridContainer.y = this.gridY;
        this.app.stage.addChild(this.gridContainer);

        this.grid = new Grid({
            parent: this.gridContainer,
            width: this.gridWidth,
            height: this.gridHeight,
            numReels: this.numReels,
            numRows: this.numRows,
            displayReelSet: this.displayReelSet,
            weightedDisplayReelSet: this.weightedDisplayReelSet,
            symbolColorMapper: this.symbolColorMapper,
            sound: this.soundPlayer,
            reelSpinGridTransformer: this.reelSpinGridTransformer,
            symbolMap: this.symbolMap,
            imageMap: this.imageMap,
            suspenseSymbolCode: this.suspenseSymbolCode,
            simultaneousWinLineAnimations: this.simultaneousWinLineAnimations,
            winLineColor: this.winLineColor,
            winLines: this.winLines,
        });

        const createPanelBackground = (width, height, color) => {
            var bg = new PIXI.Sprite(PIXI.Texture.WHITE);
            bg.width = width;
            bg.height = height;
            bg.tint = color;
            return bg;
        }

        const createPanel = (x, y, width, height, color) => {
            const container = new PIXI.Container();
            container.x = x;
            container.y = y;
            container.shape = {
                width, height
            }
            this.app.stage.addChild(container);
            if (color) {
                container.addChild(createPanelBackground(width, height, color))
            }
            return container;
        }

        if (this.topPanelHeight) {
            this.topPanel = createPanel(this.gridX, this.gridY - this.topPanelHeight, this.window.width, this.topPanelHeight, this.topPanelColor);
        }

        if (this.bottomPanelHeight) {
            this.bottomPanel = createPanel(
                this.windowX,
                this.windowY + this.window.height,
                this.window.width,
                this.bottomPanelHeight,
                this.bottomPanelColor,
            )
        }

        if (this.leftPanelWidth) {
            this.leftPanel = createPanel(
                0,
                this.window.y,
                this.leftPanelWidth,
                this.window.height,
                this.leftPanelColor
            )
        }

        if (this.rightPanelWidth) {
            this.rightPanel = createPanel(
                this.window.width + this.windowX,
                this.windowY,
                this.rightPanelWidth,
                this.window.height,
                this.rightPanelColor
            )
        }


        this.spinButton = new Button({
            x: 0,
            y: this.uiHeight - 150,
            width: 200,
            height: 60,
            app: this.app,
            onClick: () => this.play(),
            text: "Spin",
        });

        this.autoPlayButton = new Button({
            x: 0,
            y: this.uiHeight - 90,
            width: 200,
            height: 30,
            app: this.app,
            onClick: () => {
                this.autoPlayOn = !this.autoPlayOn;
                this.autoPlayButton.changeColor(this.autoPlayOn ? "#37b832" : "#7e32b8");
                if (this.autoPlayOn && !this.running) this.spinButton.click();
            },
            text: "Auto-Play",
            color: "#7e32b8",
        });

        this.eventHistory = new ScrollingTextBox("eventHistory");
        this.spinHistory = new ScrollingTextBox("spinHistory");

        this.ticker = new Ticker({
            app: this.app,
            width: 580,
            height: 100,
            x: 220,
            y: this.uiHeight - 150,
            pauseScale: this.mainTickerPauseScale,
            textFormat: this.mainTickerTextFormat,
            historyTextBox: this.eventHistory,
        });


        this.balanceTicker = new Ticker({
            app: this.app,
            width: 200,
            height: 20,
            x: 0,
            y: this.uiHeight - 50,
            pauseScale: 0,
            textFormat: {
                fontFamily: "Arial",
                fontSize: 100,
                fill: this.tickerTextColor ?? 0x000000,
            },
        });
        this.updateBalance = () => this.balanceTicker.updateText("Balance: £" + (this.balance / 100).toFixed(2));
        this.incrementBalance = async (increment) => {
            this.balance += increment;
            await this.updateBalance();
        }
        this.updateBalance();

        this.configPanel = new PIXI.Container();
        this.configPanel.x = 0;
        this.configPanel.y = this.uiHeight - 30;
        const panelBackground = new PIXI.Graphics();
        panelBackground.beginFill(0x012d36);
        panelBackground.drawRect(0, 0, this.uiWidth, 30);
        panelBackground.endFill();
        this.configPanel.addChild(panelBackground);
        this.app.stage.addChild(this.configPanel);

        this.payTable = new PayTable({
            game: this,
            app: this.app,
            x: 50,
            y: 50,
            width: this.uiWidth - 100,
            height: this.uiHeight - 100,
            getSymbol: (symbolCode) => this.window.getSymbol(symbolCode),
            stakeMultiplier: this.stakeMultiplier
        });
        this.payTable.payTableSetup(this.basePayTable, this.stakeMultiplier);

        this.cheats = new Cheats({
            game: this, app: this.app, x: 50, y: 50, width: this.uiWidth - 100, height: this.uiHeight - 100,
        });


        this.payTableButton = new Button({
            x: this.uiWidth - 300,
            y: 0,
            width: 150,
            height: 30,
            onClick: () => {
                this.payTable.show();
            },
            text: "Paytable",
            color: "#7eb832",
            parent: this.configPanel,
        });


        this.cheatsButton = new Button({
            x: this.uiWidth - 150,
            y: 0,
            width: 150,
            height: 30,
            onClick: () => {
                this.cheats.show();
            },
            text: "Cheats",
            color: "#7e32b8",
            parent: this.configPanel,
        });

        this.betSelector = new BetSelector({
            x: 0,
            y: 0,
            parent: this.configPanel,
            lowerBet: (oldStake) => {
                if (this.running) return oldStake;
                if (oldStake != this.stake) throw new Error("Displayed stake mismatch.")
                const oldStakeIndex = this.validBets.indexOf(oldStake);
                if (oldStakeIndex == 0) return oldStake;
                const newStake = this.validBets[oldStakeIndex - 1];
                this.updateStake(newStake);
                return newStake;
            },
            raiseBet: (oldStake) => {
                if (this.running) return oldStake;
                if (oldStake != this.stake) throw new Error("Displayed stake mismatch.")
                const oldStakeIndex = this.validBets.indexOf(oldStake);
                if (oldStakeIndex == this.validBets.length - 1) return oldStake;
                const newStake = this.validBets[oldStakeIndex + 1];
                this.updateStake(newStake);
                return newStake;
            },
            defaultStake: this.stake,
        })

        this.freeSpins.spinsLeftTicker = new Ticker({
            app: this.app,
            width: 200,
            height: 20,
            x: (this.uiWidth - 200) / 2,
            y: this.uiHeight - 50,
            pauseScale: 3,
            textFormat: {fontFamily: "Arial", fontSize: 100, fill: this.tickerTextColor ?? 0x000000},
        });

        this.freeSpins.spinsLeftTicker.setVisible(false);
        this.freeSpins.spinsLeftTicker.updateSpins = () =>
            this.freeSpins.spinsLeftTicker.updateText(`${this.freeSpins.spinsLeft} free spins left`);
        this.freeSpins.spinsLeftTicker.incrementSpins = async (increment) => {
            this.freeSpins.spinsLeft += increment;
            await this.freeSpins.spinsLeftTicker.updateSpins();
        }

        this.freeSpins.balanceTicker = new Ticker({
            app: this.app,
            width: 200,
            height: 20,
            x: this.uiWidth - 200,
            y: this.uiHeight - 50,
            pauseScale: 0,
            textFormat: {fontFamily: "Arial", fontSize: 100, fill: this.tickerTextColor ?? 0x000000},
        });


        this.freeSpins.balanceTicker.setVisible(false);
        this.freeSpins.balanceTicker.updateBalance = () =>
            this.freeSpins.balanceTicker.updateText(`Free spins winnings: £${(this.freeSpins.balance / 100).toFixed(2)}`);
        this.freeSpins.balanceTicker.incrementBalance = async (increment) => {
            this.freeSpins.balance += increment;
            await this.freeSpins.balanceTicker.updateBalance();
        }
    }

}
