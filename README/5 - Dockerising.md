### Dockerising

When you're ready for other team members to play your game, follow these instructions to prepare it for
hosting.

#### Prerequisites

- Docker Desktop (Windows/macOS)
    - Go to https://docs.docker.com/desktop/
    - Follow "Install Docker Desktop" links at the bottom of the page.

#### To build the docker image

1) Make sure Docker Desktop is running.
2) From the repo root:
   ```
   docker build --platform linux/amd64 -t cpp-web-of-wonders .
   # (rename to your game name)
   ```

#### To test

1) Run:
   ```
   docker run --rm -p 8080:8080 cpp-web-of-wonders
   ```
2) Open your browser at:
   ```
   http://localhost:8080
   ```
