#include "gyro.h"

extern SPI_HandleTypeDef hspi5;

#define GYRO_CS_ENABLE()  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET)
#define GYRO_CS_DISABLE() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET)

static void Gyro_WriteReg(uint8_t reg, uint8_t data) {
    uint8_t txData[2] = {reg & 0x7F, data};

    GYRO_CS_ENABLE();
    HAL_SPI_Transmit(&hspi5, txData, 2, HAL_MAX_DELAY);
    GYRO_CS_DISABLE();
}

void Gyro_Init(void) {
    Gyro_WriteReg(L3GD20_REG_CTRL_REG1, 0x0F);
    Gyro_WriteReg(L3GD20_REG_CTRL_REG4, 0x90);
}

void Gyro_ReadXYZ(GyroData_t *data) {
    uint8_t reg = L3GD20_REG_OUT_X_L | 0x80 | 0x40;

    uint8_t txData[7] = {reg, 0, 0, 0, 0, 0, 0};
    uint8_t rxData[7] = {0};

    GYRO_CS_ENABLE();
    HAL_SPI_TransmitReceive(&hspi5, txData, rxData, 7, HAL_MAX_DELAY);
    GYRO_CS_DISABLE();

    data->x = (int16_t)((rxData[2] << 8) | rxData[1]);
    data->y = (int16_t)((rxData[4] << 8) | rxData[3]);
    data->z = (int16_t)((rxData[6] << 8) | rxData[5]);
}
