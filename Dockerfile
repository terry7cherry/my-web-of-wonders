# ============================
# Stage 1: Build (Node + C++)
# ============================
FROM node:20-bookworm AS build

# Install C++ toolchain and CMake for building the backend and tools needed for vcpkg install
RUN apt-get update && apt-get install -y \
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
RUN git clone https://github.com/microsoft/vcpkg.git && \
                  ./vcpkg/bootstrap-vcpkg.sh -disableMetrics

ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_FEATURE_FLAGS=manifests,versions

# Copy just manifest first for caching
WORKDIR /src
COPY vcpkg.json vcpkg.json
RUN ${VCPKG_ROOT}/vcpkg install --triplet x64-linux --x-manifest-root=/src

# Copy source
COPY CMakeLists.txt ./
COPY srcBackend ./srcBackend
COPY srcFrontend ./srcFrontend

# Install node modules
WORKDIR ./srcFrontend
RUN npm i


# Build (C++ and frontend)
WORKDIR /src

RUN cmake -B build -S . \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_CXX_FLAGS="-w" \
    -DFRONTEND=ON && \
    cmake --build build --target cpp-game -j$(nproc)


# ============================
# Stage 2: Runtime image
# ============================

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    ca-certificates && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/cpp-game /usr/local/bin/cpp-game
COPY --from=build /src/srcFrontend/dist /src/srcFrontend/dist
COPY --from=build /src/srcBackend/thisTitle/parSheets /src/srcBackend/thisTitle/parSheets

EXPOSE 8080
CMD ["cpp-game"]

