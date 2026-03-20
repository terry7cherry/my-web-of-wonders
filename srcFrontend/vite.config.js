import { defineConfig } from "vite";

export default defineConfig({
    base: "./",
    server: {
        host: true,
        port: 5173,
        strictPort: true,
        proxy: {
            "/api": {
                target: "http://localhost:8080/",
                changeOrigin: true,
                secure: false,
                ws: true
            }
        }
    },
    build: {
        outDir: "dist",
        target: "esnext",
        sourcemap: true,
        minify: false
    },
});
