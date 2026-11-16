#include "gyro.h"

// Uchwyt magistrali SPI5 zdefiniowany w pliku main.c
extern SPI_HandleTypeDef hspi5;

// Makra sterujące linią Chip Select (aktywacja żyroskopu stanem niskim)
#define GYRO_CS_ENABLE()  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET)
#define GYRO_CS_DISABLE() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET)

// Zapis pojedynczego rejestru żyroskopu po SPI
static void Gyro_WriteReg(uint8_t reg, uint8_t data) {
    // Wyczyszczenie 7. bitu (maska 0x7F) wymusza operację zapisu
    uint8_t txData[2] = {reg & 0x7F, data};

    GYRO_CS_ENABLE();
    HAL_SPI_Transmit(&hspi5, txData, 2, 10);
    GYRO_CS_DISABLE();
}

// Inicjalizacja żyroskopu
void Gyro_Init(void) {
    uint8_t reg = L3GD20_REG_WHO_AM_I | 0x80;
    uint8_t txData[2] = {reg, 0};
    uint8_t rxData[2] = {0};

    GYRO_CS_ENABLE();
    HAL_SPI_TransmitReceive(&hspi5, txData, rxData, 2, 10);
    GYRO_CS_DISABLE();
    // Jeśli czujnik odpowiada, w rxData[1] znajduje się wartość 0xD4

    // Włączenie osi X, Y, Z oraz wyjście z trybu uśpienia
    Gyro_WriteReg(L3GD20_REG_CTRL_REG1, 0x0F);
    // Włączenie ochrony przed nadpisaniem
    Gyro_WriteReg(L3GD20_REG_CTRL_REG4, 0x90);
}

// Odczyt aktualnych prędkości kątowych ze wszystkich trzech osi
void Gyro_ReadXYZ(GyroData_t *data) {
    uint8_t reg = L3GD20_REG_OUT_X_L | 0x80 | 0x40;
    uint8_t txData[7] = {reg, 0, 0, 0, 0, 0, 0};
    uint8_t rxData[7] = {0};

    GYRO_CS_ENABLE();
    HAL_SPI_TransmitReceive(&hspi5, txData, rxData, 7, 10);
    GYRO_CS_DISABLE();

    // Sklejanie młodszych i starszych bajtów dla każdej osi w 16-bitową liczbę
    data->x = (int16_t)((rxData[2] << 8) | rxData[1]);
    data->y = (int16_t)((rxData[4] << 8) | rxData[3]);
    data->z = (int16_t)((rxData[6] << 8) | rxData[5]);
}
