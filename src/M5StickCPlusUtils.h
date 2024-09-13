#ifndef M5STICKCPLUSUTILS_H
#define M5STICKCPLUSUTILS_H
#include <Arduino.h>
#include <M5StickCPlus.h>
#include <driver/i2s.h>
#include <math.h>
#define PIN_LED 10
#define PIN_CLK 0
#define PIN_DATA 34
#define READ_LEN (2 * 256)
#define GAIN_FACTOR 3
#define H_SCALING 1.2
#define V_SCALING 2.5
#define PIN_DOOR 26


void showSignal();
float calculateRMS(int16_t *buffer, int length);
void monitor_task(void *arg);
void i2sInit();
void setupM5StickCPlus(void *zab_s);


#endif