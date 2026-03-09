import * as PIXI from "pixi.js";

export default async () => {
    // Vite: get URLs for all images in /src/images at build time
    const modules = import.meta.glob("../../thisTitle/images/*.{png,jpg,jpeg,webp,gif,svg}", {
        eager: true,
        query: '?url',
        import: 'default'
    });

    /** @type {Record<string, any>} */
    const imageMap = {};

    const entries = Object.entries(modules);
    await Promise.all(
        entries.map(async ([path, url]) => {
            const filename = path.split("/").pop() ?? path;       // e.g. "wild.png"
            const key = filename.replace(/\.[^.]+$/, "");         // e.g. "wild"
            imageMap[key] = await PIXI.Assets.load(url);
        }),
    );

    return imageMap;
}
