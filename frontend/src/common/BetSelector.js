import * as PIXI from "pixi.js"

export default class BetSelector {
    constructor({x, y, parent, raiseBet, lowerBet, titleColor, betColor, plusColor, minusColor, defaultStake }) {
        this.lowerBet = lowerBet;
        this.raiseBet = raiseBet;
        this.container = new PIXI.Container();
        this.container.y = y;
        this.container.x = x;
        this.stake = defaultStake;

        this.addTitleText(titleColor);
        this.addMinusButton(minusColor);
        this.addBetText(betColor);
        this.addPlusButton(plusColor);

        parent.addChild(this.container);
    }

    updateStake(newStake) {
        this.stake = newStake;
        this.betText.text = "£" + (newStake/100).toFixed(2);
        this.plusButton.x = 2 + this.titleText.width + this.minusButton.width + 4 + this.betText.width + 2;
    }

    addTitleText(titleColor) {
        this.titleText = new PIXI.Text("Bet: ", {
              fontFamily: "Arial",
                fontSize: 23,
                fill: titleColor ?? 0xffffff,
                align: "center",
        })
        this.titleText.x = 2;
        this.container.addChild(this.titleText);
    }

    addMinusButton(minusColor) {
        const minusButton = new PIXI.Text("-", {
              fontFamily: "Arial",
                fontSize: 26,
                fill: minusColor ?? 0xFF0000,
                align: "center",
        })
        minusButton.x = 2 + this.titleText.width;
        minusButton.y = -5;
        minusButton.interactive = true;
        minusButton.buttonMode = true;
        minusButton.on("pointerdown", () => {
            const newStake = this.lowerBet(this.stake);
            this.updateStake(newStake);
        });

        this.container.addChild(minusButton);
        this.minusButton = minusButton;
    }

    addPlusButton(plusColor) {
        const plusButton = new PIXI.Text("+", {
              fontFamily: "Arial",
                fontSize: 26,
                fill: plusColor ?? 0x00FF00,
                align: "center",
        })
        plusButton.x = 2 + this.titleText.width + this.minusButton.width + 4 + this.betText.width + 2;
        plusButton.y = -3;
        plusButton.interactive = true;
        plusButton.interactive = true;
        plusButton.buttonMode = true;
        plusButton.on("pointerdown", () => {
            const newStake = this.raiseBet(this.stake);
            this.updateStake(newStake);
        });

        this.container.addChild(plusButton);
        this.plusButton = plusButton;
    }

    addBetText(betColor) {
        const betText = new PIXI.Text("£" + (this.stake / 100).toFixed(2), {
              fontFamily: "Arial",
                fontSize: 23,
                fill: betColor ?? 0xFFFFFF,
                align: "center",
        })
        betText.x = 2 + this.titleText.width + this.minusButton.width + 4;

        this.container.addChild(betText);
        this.betText = betText;
    }
}