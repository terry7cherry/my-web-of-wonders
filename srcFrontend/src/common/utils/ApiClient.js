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

        const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

        const maxAttempts = 100;
        const baseDelayMs = 200;
        const maxDelayMs = 2_000;

        const attemptOnce = async () => {
            const request = new Request(requestUri, {
                headers: {"Accept": "application/json"},
            });

            return fetch(request)
                .then(async (response) => {
                    if (!response.ok) {
                        const retryable =
                            response.status === 500 ||
                            response.status === 502 ||
                            response.status === 503 ||
                            response.status === 504;

                        const text = await response.text().catch(() => "");
                        return {
                            ok: false,
                            retryable,
                            error: `Failed to fetch options (HTTP ${response.status})${text ? `: ${text}` : ""}`,
                        };
                    }

                    return response.json()
                        .then((json) => {
                            if (json && json.error) {
                                return {
                                    ok: false,
                                    retryable: false,
                                    error: "Error from server when fetching RTP and Bet Type options: " + json.error,
                                };
                            }
                            return {ok: true, data: json};
                        })
                        .catch((e) => ({
                            ok: false,
                            retryable: true,
                            error: `Failed to parse JSON: ${e instanceof Error ? e.message : String(e)}`,
                        }));
                })
                .catch((e) => ({
                    ok: false,
                    retryable: true,
                    error: e instanceof Error ? e.message : String(e),
                }));
        };

        let lastError = "Unknown error";
        for (let attempt = 1; attempt <= maxAttempts; attempt++) {
            const result = await attemptOnce();

            if (result.ok) return result.data;

            lastError = result.error;

            if (!result.retryable || attempt === maxAttempts) break;

            const exp = Math.min(maxDelayMs, baseDelayMs * (2 ** (attempt - 1)));
            const jitter = 0.2 * exp * (Math.random() * 2 - 1); // +/-20%
            const delay = Math.max(0, Math.round(exp + jitter));
            await sleep(delay);
        }

        // One throw at the end (no caught exceptions during retries).
        throw new Error(lastError);
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

    static async getNewGameCycle(session, forceIndex, seed, rnStream) {
        const jsonBody = {session, forceIndex, seed, rnStream};
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