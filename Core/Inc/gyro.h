#ifndef GYRO_H
#define GYRO_H

#include "stm32f4xx_hal.h"

#define L3GD20_REG_WHO_AM_I      0x0F
#define L3GD20_REG_CTRL_REG1     0x20
#define L3GD20_REG_CTRL_REG4     0x23
#define L3GD20_REG_OUT_X_L       0x28

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} GyroData_t;

void Gyro_Init(void);
void Gyro_ReadXYZ(GyroData_t *data);

#endif /* GYRO_H */
