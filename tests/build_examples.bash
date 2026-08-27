#!/bin/env bash
set -e

if [ "${SKIP_STM32L476RGT6_FreeRTOS:-0}" != "1" ]; then
    echo "===== Configuring STM32L476RGT6_FreeRTOS ====="
    cd examples/STM32L476RGT6_FreeRTOS
    rm -rf build
    mkdir build
    cmake -G Ninja -B build .
    echo "==== Building STM32L476RGT6_FreeRTOS ===="
    ninja -C build
    cd ../../
else
    @echo "===== Skipping STM32L476RGT6_FreeRTOS ====="
fi

if [ "${SKIP_POSIX_FreeRTOS:-0}" != "1" ]; then
    echo "==== Configuring POSIX_FreeRTOS ===="
    cd examples/POSIX_FreeRTOS
    rm -rf build
    mkdir build
    cmake -G Ninja -B build .
    echo "==== Building POSIX_FreeRTOS ===="
    ninja -C build
    cd ../../
else
    echo "==== Skipping POSIX_FreeRTOS ===="
fi

if [ "${SKIP_QEMU_MPS2_AN385_FREERTOS:-0}" != "1" ]; then
    echo "==== Configuring QEMU_MPS2_AN385_FREERTOS ===="
    cd examples/QEMU_MPS2_AN385_FREERTOS
    rm -rf build
    mkdir build
    cmake -G Ninja -B build .
    echo "==== Building QEMU_MPS2_AN385_FREERTOS ===="
    ninja -C build
    cd ../../
else
    echo "==== Skipping QEMU_MPS2_AN385_FREERTOS ===="
fi

echo "==== OK ===="
