import * as PIXI from "pixi.js"

export default class BetSelector {
    constructor({x, y, parent, raiseBet, lowerBet, titleColour, betColour, plusColour, minusColour, defaultStake }) {
        this.lowerBet = lowerBet;
        this.raiseBet = raiseBet;
        this.container = new PIXI.Container();
        this.container.y = y;
        this.container.x = x;
        this.stake = defaultStake;

        this.addTitleText(titleColour);
        this.addMinusButton(minusColour);
        this.addBetText(betColour);
        this.addPlusButton(plusColour);

        parent.addChild(this.container);
    }

    updateBet(newStake) {
        this.stake = newStake;
        this.betText.text = "£" + (newStake/100).toFixed(2);
        this.plusButton.x = 2 + this.titleText.width + this.minusButton.width + 4 + this.betText.width + 2;
    }

    addTitleText(titleColour) {
        this.titleText = new PIXI.Text("Bet: ", {
              fontFamily: "Arial",
                fontSize: 23,
                fill: titleColour ?? 0xffffff,
                align: "center",
        })
        this.titleText.x = 2;
        this.container.addChild(this.titleText);
    }

    addMinusButton(minusColour) {
        const minusButton = new PIXI.Text("-", {
              fontFamily: "Arial",
                fontSize: 26,
                fill: minusColour ?? 0xFF0000,
                align: "center",
        })
        minusButton.x = 2 + this.titleText.width;
        minusButton.y = -5;
        minusButton.interactive = true;
        minusButton.buttonMode = true;
        minusButton.on("pointerdown", () => {
            const newStake = this.lowerBet(this.stake);
            this.updateBet(newStake);
        });

        this.container.addChild(minusButton);
        this.minusButton = minusButton;
    }

    addPlusButton(plusColour) {
        const plusButton = new PIXI.Text("+", {
              fontFamily: "Arial",
                fontSize: 26,
                fill: plusColour ?? 0x00FF00,
                align: "center",
        })
        plusButton.x = 2 + this.titleText.width + this.minusButton.width + 4 + this.betText.width + 2;
        plusButton.y = -3;
        plusButton.interactive = true;
        plusButton.interactive = true;
        plusButton.buttonMode = true;
        plusButton.on("pointerdown", () => {
            const newStake = this.raiseBet(this.stake);
            this.updateBet(newStake);
        });

        this.container.addChild(plusButton);
        this.plusButton = plusButton;
    }

    addBetText(betColour) {
        const betText = new PIXI.Text("£" + (this.stake / 100).toFixed(2), {
              fontFamily: "Arial",
                fontSize: 23,
                fill: betColour ?? 0xFFFFFF,
                align: "center",
        })
        betText.x = 2 + this.titleText.width + this.minusButton.width + 4;

        this.container.addChild(betText);
        this.betText = betText;
    }
}