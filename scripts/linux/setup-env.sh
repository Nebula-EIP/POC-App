#!/usr/bin/env bash
set -e

echo "Setting up Linux build environment..."

sudo apt-get update

sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    clang-format \
    clang-tidy \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libasound2-dev \
    mesa-common-dev

echo "Environment ready."
