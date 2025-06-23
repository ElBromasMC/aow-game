FROM docker.io/emscripten/emsdk:4.0.10-arm64 AS base

# Create and switch to non-root user
RUN useradd -m runner
USER runner

# Create required folders and change to source directory
RUN mkdir -p /home/runner/src
WORKDIR /home/runner/src
COPY --chown=runner:runner . .

#
# Development
#

FROM base AS development

# Change to non-root user
USER runner

# Change to source directory
WORKDIR /home/runner/src

ENTRYPOINT ["/bin/bash"]

#
# Production
#

FROM base AS builder

# Change to non-root user
USER runner

WORKDIR /home/runner/src/raylib-5.5/src

# Compile the project for the web platform
RUN make clean \
    && make -j4 PLATFORM=PLATFORM_WEB

WORKDIR /home/runner/src
RUN make clean \
    && make -j4 PLATFORM=PLATFORM_WEB BUILD_WEB_ASYNCIFY=TRUE

RUN ls -lah

FROM docker.io/ubuntu:jammy AS production

USER root
RUN apt-get update \
    && apt-get install -y \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Create and switch to non-root user
RUN useradd -m runner
USER runner

# Create required folders and change to source directory
RUN mkdir -p /home/runner/app
WORKDIR /home/runner/app
ENV PROJECT_NAME=/home/runner/src/aow_game
COPY --from=builder --chown=runner:runner ${PROJECT_NAME}.data ${PROJECT_NAME}.html ${PROJECT_NAME}.js ${PROJECT_NAME}.wasm .
ENTRYPOINT ["python3", "-m", "http.server", "8080"]

