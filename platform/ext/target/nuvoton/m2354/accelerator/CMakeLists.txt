#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
# Copyright (c) 2020, Nuvoton Technology Corp. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

################################ BL2 ###########################################
if(BL2)

    target_sources(bl2_crypto_hw
        PUBLIC
            ${PLATFORM_DIR}/ext/target/nuvoton/common/accelerator/entropy_alt.c
            ${PLATFORM_DIR}/ext/target/nuvoton/common/accelerator/nuvoton.c
    )

    target_include_directories(bl2_crypto_hw
        PRIVATE
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/accelerator/
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/partition
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/include
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/config
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Device/Nuvoton/M2354/Include
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Library/StdDriver/inc
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/Include
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/StdDriver/inc
            ${PLATFORM_DIR}/ext/cmsis
    )

    target_link_libraries(bl2_crypto_hw
        PRIVATE
            bl2_mbedcrypto
            platform_bl2
    )

    target_link_libraries(bl2_mbedcrypto
        PUBLIC
            bl2_crypto_hw
    )
    target_include_directories(bl2_mbedcrypto
        PUBLIC
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/accelerator/
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/partition
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/include
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/config
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Device/Nuvoton/M2354/Include
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Library/StdDriver/inc
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/Include
            ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/StdDriver/inc
            ${PLATFORM_DIR}/ext/cmsis
    )

endif()
############################ Crypto Service ####################################

target_sources(crypto_service_crypto_hw
    PRIVATE
        ${PLATFORM_DIR}/ext/target/nuvoton/common/accelerator/entropy_alt.c
        ${PLATFORM_DIR}/ext/target/nuvoton/common/accelerator/nuvoton.c
)

target_include_directories(crypto_service_crypto_hw
    PRIVATE
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/accelerator/
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/partition
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/include
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/config
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Device/Nuvoton/M2354/Include
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Library/StdDriver/inc
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/Include
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/StdDriver/inc
        ${PLATFORM_DIR}/ext/cmsis
)

target_include_directories(crypto_service_mbedcrypto
    PUBLIC
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/accelerator/
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/partition
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/include
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/device/config
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Device/Nuvoton/M2354/Include
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/bsp/Library/StdDriver/inc
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/Include
        ${PLATFORM_DIR}/ext/target/${TFM_PLATFORM}/../common/bsp/StdDriver/inc
        ${PLATFORM_DIR}/ext/cmsis
)

target_link_libraries(crypto_service_crypto_hw
    PRIVATE
        crypto_service_mbedcrypto
        platform_s
)
