export default class ApiClient {
    static async getMathsAndForcesForRtpAndBetType(rtp, betType) {
        const jsonBody = {rtp, betType};

        const response = await fetch("/api/maths", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Accept": "application/json",
            },
            body: JSON.stringify(jsonBody),
        });

        /** @type {{ forceOptions: Array.<{index: number, name: string}>, maths: { betTypeCost: number, displayReelSet: Array.<Array.<number>>, gameTitle: string, minBet: number, minBetCost: number, numReels: number, numRows: number, payTable: Object.<string, Array.<number>>, suspenseSymbol: number, symbolNames: Array.<string>}}} */
        const responseJson = await response.json();
        console.log(responseJson);
        if (responseJson.error) {
            throw new Error(
                "Error from server when fetching session and config: " + responseJson.error,
            );
        }
        return responseJson;
    }

    static async getGlobalConfig() {
        const requestUri = `/api/options`;
        const request = new Request(requestUri);
        const response = await fetch(request);

        /** @type {{ gameTitle: string, rtps: any, betTypes: Object.<string, number>, minBet: number, error?: string }} */
        const responseJson = await response.json();
        if (responseJson.error) {
            throw new Error(
                "Error from server when fetching RTP and Bet Type options: " + responseJson.error,
            );
        }
        return responseJson;
    }

    static async getNewSession(oldSession, bet, betType, rtp) {
        const jsonBody = {oldSession, bet, betType, rtp};
        const response = await fetch("/api/newSession", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Accept": "application/json",
            },
            body: JSON.stringify(jsonBody),
        });
        const responseJson = await response.json();
        if (responseJson.error)
            throw new Error(
                "Error from server when fetching new session: " + responseJson.error,
            );

        return responseJson;
    }

    static async getNewGameCycle(session, forceIndex, seed) {
        const jsonBody = {session, forceIndex, seed};
        const response = await fetch("/api/spin", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Accept": "application/json",
            },
            body: JSON.stringify(jsonBody),
        });
        const responseJson = await response.json();
        if (responseJson.error)
            throw new Error(
                "Error from server when fetching new spin: " + responseJson.error,
            );
        return responseJson;
    }
}