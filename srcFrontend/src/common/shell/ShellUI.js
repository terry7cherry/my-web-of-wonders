import ScrollingTextBox from "./ScrollingTextBox.js";
import SessionStats from "./SessionStats.js";
import statsHtml from "../../thisTitle/sessionStats.js";
import * as PIXI from "pixi.js";
import toCurrency from "../utils/toCurrency.js";

export default class ShellUI {
    constructor() {
        // Build the Session table before SessionStats wires up inputs
        this.eventHistory = new ScrollingTextBox("eventHistory");
        this.spinHistory = new ScrollingTextBox("spinHistory");
        this.rtpSelector = document.getElementById("rtp-selector");
        this.betTypeSelector = document.getElementById("bet-type-selector");
        this.buildSessionTable();
        this.sessionStats = new SessionStats();
        this.setupPlaybackControls();
    }

    async init(rtps, betTypes) {
        await this.populateRtpAndBetTypeOptions(rtps, betTypes);
        this.sessionStats.reset();
    }

    async populateRtpAndBetTypeOptions(rtps, betTypes) {
        this.rtpSelector.innerHTML = rtps
            .sort((a, b) => b - a)
            .map(rtp => `<option value="${rtp}">${rtp}</option>`)
            .join("");
        Object.keys(betTypes)
            .sort((a, b) => betTypes[a] - betTypes[b])
            .forEach(betType => {
                this.betTypeSelector.options.add(new Option(
                    `${betType}${betTypes[betType] === 1 ? "" : " (X" + betTypes[betType] + " bet)"}`,
                    betType
                ));
            })
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

    buildSessionTable() {
        const table = document.querySelector('#side-panel table');
        if (!table) return;

        table.innerHTML = '';
        statsHtml.forEach(html => table.insertAdjacentHTML('beforeend', html.trim()));
    }

    addWinsToHistory(wins) {
        this.eventHistory.addText("Wins:")
        for (const {symbol, oak, multiplier, win, totalWin, winType} of wins)
            this.eventHistory.addText(
                `&#9;- ${winType} win: ${symbol}x${oak} for ${toCurrency(win)}${
                    multiplier !== 1
                        ? ` x ${multiplier} = ${toCurrency(totalWin)}`
                        : ``
                }`
            );
    }
}