#pragma once

/**
 * @file I2SAudioConfig.h
 * @brief Header for I2SAudioConfig.h.
 */

#include <driver/i2s.h>

#include "controllers/ProtocolParser.h"  // AUDIO_SAMPLE_RATE_HZ, AUDIO_CHUNK_SIZE_BYTES, etc.

/**
 * @brief ============================================================
 * INMP441 Microphone (I2S0 — RX)
 * @brief ============================================================
 */
constexpr i2s_port_t I2S_MIC_PORT = I2S_NUM_0;
constexpr int I2S_MIC_WS_PIN = 25;   // Word Select (LRC)
constexpr int I2S_MIC_SCK_PIN = 26;  // Bit Clock (BCLK)
constexpr int I2S_MIC_SD_PIN = 33;   // Serial Data (SD)

/**
 * @brief ============================================================
 * MAX98357A Amplifier (I2S1 — TX)  [Reserved for Task 4.2]
 * @brief ============================================================
 */
constexpr i2s_port_t I2S_SPK_PORT = I2S_NUM_1;
constexpr int I2S_SPK_LRC_PIN = 12;  // Word Select (LRC)
constexpr int I2S_SPK_BCK_PIN = 14;  // Bit Clock (BCLK)
constexpr int I2S_SPK_DIN_PIN = 27;  // Data In (DIN)

/**
 * @brief ============================================================
 * Shared DMA Tuning
 * @brief ============================================================
 */
constexpr int I2S_DMA_BUF_COUNT = 8;  // Increased count for stability
constexpr int I2S_DMA_BUF_LEN =
    256;  // Crucial fix: 1024 frames * 32-bit stereo exceeds 4092 byte DMA limit!
