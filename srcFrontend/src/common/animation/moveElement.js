import * as PIXI from "pixi.js";

export default ({
                    element,
                    newX = 0,
                    newY = 0,
                    pixelsPerFrame,
                    totalFrames
                }) => new Promise((resolve, reject) => {
    if (!pixelsPerFrame && !totalFrames) reject("Either pixelsPerFrame (speed) or totalFrames must be provided.");
    const oldX = element.x;
    const oldY = element.y;
    const xDistance = newX - oldX;
    const yDistance = newY - oldY;
    const totalDistance = Math.sqrt(Math.pow(xDistance, 2) + Math.pow(yDistance, 2));
    if (!totalFrames) totalFrames = totalDistance / pixelsPerFrame;
    let elapsedFrames = 0;
    const animate = (delta) => {
        elapsedFrames += delta;
        const progress = Math.min(1, elapsedFrames / totalFrames);
        element.x = oldX + xDistance * progress;
        element.y = oldY + yDistance * progress;
        if (progress === 1) {
            PIXI.Ticker.shared.remove(animate);
            resolve();
        }
    }
    PIXI.Ticker.shared.add(animate);
})