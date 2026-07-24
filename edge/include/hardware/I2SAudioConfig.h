#ifndef I2S_AUDIO_CONFIG_H
#define I2S_AUDIO_CONFIG_H

#include <driver/i2s.h>
#include "controllers/ProtocolParser.h"  // AUDIO_SAMPLE_RATE_HZ, AUDIO_CHUNK_SIZE_BYTES, etc.

// ============================================================
// INMP441 Microphone (I2S0 — RX)
// ============================================================
constexpr i2s_port_t I2S_MIC_PORT    = I2S_NUM_0;
constexpr int        I2S_MIC_WS_PIN  = 25;   // Word Select (LRC)
constexpr int        I2S_MIC_SCK_PIN = 26;   // Bit Clock (BCLK)
constexpr int        I2S_MIC_SD_PIN  = 33;   // Serial Data (SD)

// ============================================================
// MAX98357A Amplifier (I2S1 — TX)  [Reserved for Task 4.2]
// ============================================================
constexpr i2s_port_t I2S_SPK_PORT    = I2S_NUM_1;
constexpr int        I2S_SPK_LRC_PIN = 12;   // Word Select (LRC)
constexpr int        I2S_SPK_BCK_PIN = 14;   // Bit Clock (BCLK)
constexpr int        I2S_SPK_DIN_PIN = 27;   // Data In (DIN)

// ============================================================
// Shared DMA Tuning
// ============================================================
constexpr int I2S_DMA_BUF_COUNT = 6;          // 6 buffers × 1024 = ~192ms buffer
constexpr int I2S_DMA_BUF_LEN   = AUDIO_CHUNK_SIZE_BYTES / (AUDIO_BIT_DEPTH / 8);
// 1024 bytes / 2 bytes-per-sample = 512 samples per DMA buffer

#endif
