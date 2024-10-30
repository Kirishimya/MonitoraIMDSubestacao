#ifndef M5STICKCPLUSUTILS_H
#define M5STICKCPLUSUTILS_H
#include <Arduino.h>
#include <M5StickCPlus.h>
#include <driver/i2s.h>
#include <cmath>
#include <WiFi.h>
#define PIN_LED 10
#define PIN_CLK 0
#define PIN_DATA 34
#define READ_LEN (2 * 256)
#define GAIN_FACTOR 3
#define H_SCALING 1.2
#define V_SCALING 2.5
#define PIN_DOOR 26

#define MIC_OFFSET_DB     3.0103      // Default offset (sine-wave RMS vs. dBFS). Modify this value for linear calibration
// Customize these values from microphone datasheet
#define MIC_SENSITIVITY   -25         // dBFS value expected at MIC_REF_DB (Sensitivity value from datasheet)
#define MIC_REF_DB        94.0        // Value at which point sensitivity is specified in datasheet (dB)
#define MIC_OVERLOAD_DB   115.0       // dB - Acoustic overload point
#define MIC_NOISE_DB      29          // dB - Noise floor
#define MIC_BITS          24          // valid number of bits in I2S data
#define MIC_CONVERT(s)    (s >> (SAMPLE_BITS - MIC_BITS))
#define MIC_TIMING_SHIFT  0           // Set to one to fix MSB timing for some microphones, i.e. SPH0645LM4H-x

// Calculate reference amplitude value at compile time
#define MIC_REF_AMPL (pow(10, double(MIC_SENSITIVITY)/20) * ((1<<(MIC_BITS-1))-1))
#define BG_COLOR BLACK
#define TXT_COLOR WHITE

void showSignal();
float calculateRMS(int16_t *buffer, int length);
void monitor_task(void *arg);
void i2sInit();
void setupM5StickCPlus(void *zab_s);


#endif