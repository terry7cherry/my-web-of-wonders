import Game from "./Game.js";

document.addEventListener("DOMContentLoaded", async () => {
    try {
        window.game = await new Game();
    } catch (e) {
        console.error(e);
    }
});