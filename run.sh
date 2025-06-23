#!/bin/bash

exec podman-compose -p "aow-game_dev" \
    run --rm --build --service-ports aow-game

