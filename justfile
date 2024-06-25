
clone_freertos:
    rm -rf FreeRTOS-Kernel
    git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS-Kernel

checkout_main:
    cd FreeRTOS-Kernel && git checkout main

checkout_v10_3_1:
    cd FreeRTOS-Kernel && git checkout V10.3.1-kernel-only

checkout_v11_1_0:
    cd FreeRTOS-Kernel && git checkout V11.1.0
