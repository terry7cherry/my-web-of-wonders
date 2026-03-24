# ============================
# Stage 1: Build (Node + C++)
# ============================
FROM node:20-bookworm AS build

# Install C++ toolchain and CMake for building the backend and tools needed for vcpkg install
# hadolint ignore=DL3008
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# Install vcpkg
WORKDIR /opt
ARG VCPKG_VERSION=2025.12.12
RUN git clone --no-single-branch https://github.com/microsoft/vcpkg.git
WORKDIR /opt/vcpkg
RUN git checkout ${VCPKG_VERSION} && \
    ./bootstrap-vcpkg.sh -disableMetrics

ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_FEATURE_FLAGS=manifests,versions

# Copy just manifest first for caching
WORKDIR /src
COPY vcpkg.json vcpkg.json
RUN ${VCPKG_ROOT}/vcpkg install --triplet x64-linux --x-manifest-root=/src

# Copy source
COPY CMakeLists.txt ./

# Install node modules (copy manifests first to cache this layer)
WORKDIR /src/srcFrontend
COPY srcFrontend/package.json srcFrontend/package-lock.json ./
RUN npm i

# Copy remaining source (after npm i to avoid busting its cache)
WORKDIR /src
COPY srcBackend ./srcBackend
COPY srcFrontend ./srcFrontend

# Build (C++ and frontend)
RUN cmake -B build -S . \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_CXX_FLAGS="-w" \
    -DFRONTEND=ON && \
    cmake --build build --target cpp-game -j"$(nproc)"

# ============================
# Stage 2: Runtime image
# ============================

FROM debian:bookworm-slim

# hadolint ignore=DL3008
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates && rm -rf /var/lib/apt/lists/*

RUN groupadd --system appuser && useradd --system --gid appuser appuser

COPY --from=build /src/build/cpp-game /usr/local/bin/cpp-game
COPY --from=build /src/srcFrontend/dist /src/srcFrontend/dist
COPY --from=build /src/srcBackend/thisTitle/parSheets /src/srcBackend/thisTitle/parSheets

# Create a dir for drogon to store requests larger than client_max_body_size
RUN mkdir -p /src/uploads && chown appuser:appuser /src/uploads

USER appuser

WORKDIR /src

EXPOSE 8080
CMD ["cpp-game"]
