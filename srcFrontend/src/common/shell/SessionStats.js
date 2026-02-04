export default class SessionStats {
    constructor() {
        this.stats = {};
        const elements = document.querySelectorAll('[id^="session-"]');
        elements.forEach(input => {
            const stat = {input};
            stat.name = input.id.replace("session-", "");
            stat.isCurrency = input.classList.contains("currency");
            stat.isPercent = input.classList.contains("percent");
            stat.isCalculated = input.classList.contains("calculated");
            stat.feeds = [...input.classList].filter(c => c.startsWith("feeds-")).map(c => c.replace("feeds-", ""));
            if (stat.isCalculated) {
                const numeratorClass = [...input.classList].find(c => c.startsWith("numerator-"));
                const divisorClass = [...input.classList].find(c => c.startsWith("divisor-"));
                if (!numeratorClass || !divisorClass) throw new Error("Percent stat must have numerator and divisor classes");
                stat.numerator = numeratorClass && (numeratorClass.replace("numerator-", ""));
                stat.divisor = divisorClass && (divisorClass.replace("divisor-", ""));
            }
            this.stats[stat.name] = stat;
        });
        document.getElementById("session-reset-button").onclick = this.reset.bind(this);
    }

    increment(statName, increment) {
        if (!this.stats[statName]) throw new Error("Tried to increment non-existent session stat: " + statName);
        const {input, isCurrency, feeds} = this.stats[statName];
        if (isCurrency) {
            input.numberValue = Number(input.numberValue) + (increment ?? 1);
            input.value = "£" + input.numberValue.toFixed(2);
        } else {
            input.value = Number(input.value) + (increment ?? 1);
        }
        if (!feeds.length) return;
        feeds.forEach(feed => {
            this.updateCalculatedStat(feed);
        });
    }

    updateCalculatedStat(statName) {
        const feedStat = this.stats[statName];
        if (!feedStat || !feedStat.isCalculated) throw new Error("Class list includes feed for stat that does not exist or does not have 'calculated' class:" + statName);
        const {input, isPercent, numerator, divisor} = feedStat;
        const numeratorInput = this.stats[numerator].input;
        const divisorInput = this.stats[divisor].input;
        if (isPercent) {
            input.value = (numeratorInput.numberValue / divisorInput.numberValue * 100).toFixed(2) + "%";
        } else {
            input.value = (numeratorInput.numberValue / divisorInput.numberValue).toFixed(2);
        }
    }

    reset() {
        Object.values(this.stats).forEach(({input, isCurrency, isPercent}) => {
            if (isCurrency) {
                input.numberValue = 0;
                input.value = "£0.00";
            } else if (isPercent) {
                input.value = "";
            } else {
                input.value = 0;
            }
        });
    }
}