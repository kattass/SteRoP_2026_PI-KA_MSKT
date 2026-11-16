#include "sdram_ltdc.h"
#include "main.h"

extern SDRAM_HandleTypeDef hsdram1;
extern LTDC_HandleTypeDef hltdc;
extern SPI_HandleTypeDef hspi5;

// Wskaźnik na bufor, po którym aktualnie rysujemy w tle (Back Buffer)
static volatile uint16_t *back_buffer = (volatile uint16_t *)FB_ADDRESS_1;
static uint8_t current_back_buf_idx = 1;

// SEKWENCJA STARTOWA PAMIĘCI SDRAM
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram) {
    FMC_SDRAM_CommandTypeDef cmd = {0};

    // Włączenie zegara dla kości SDRAM
    cmd.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    cmd.AutoRefreshNumber = 1;
    cmd.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(hsdram, &cmd, 0x1000);
    HAL_Delay(1);

    // Precharge wszystkich banków
    cmd.CommandMode = FMC_SDRAM_CMD_PALL;
    HAL_SDRAM_SendCommand(hsdram, &cmd, 0x1000);

    // Auto-refresh (minimum 8 cykli)
    cmd.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    cmd.AutoRefreshNumber = 8;
    HAL_SDRAM_SendCommand(hsdram, &cmd, 0x1000);

    // Konfiguracja Mode Register
    cmd.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    cmd.AutoRefreshNumber = 1;
    cmd.ModeRegisterDefinition = (uint32_t)0x0230;
    HAL_SDRAM_SendCommand(hsdram, &cmd, 0x1000);

    // Ustawienie licznika odświeżania wierszy (dla zegara SDRAM 84 MHz)
    HAL_SDRAM_ProgramRefreshRate(hsdram, 1292);
}

// POMOCNICZE FUNKCJE DLA KONTROLERA EKRANU (po SPI5)
static void ILI9341_WriteReg(uint8_t reg) {
    HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, GPIO_PIN_RESET); // Tryb komendy (DCX = 0)
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_RESET);         // CSX = 0 (Aktywny LCD)
    HAL_SPI_Transmit(&hspi5, &reg, 1, 10);
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);           // CSX = 1
}

static void ILI9341_WriteData(uint8_t data) {
    HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, GPIO_PIN_SET);   // Tryb danych (DCX = 1)
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_RESET);         // CSX = 0
    HAL_SPI_Transmit(&hspi5, &data, 1, 10);
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);           // CSX = 1
}

static void ILI9341_Init(void) {
    // Upewniamy się, że Chip Select żyroskopu (PC1) i ekranu (PC2) są w stanie wysokim
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);
    HAL_Delay(5);

    // Software Reset sterownika ILI9341
    ILI9341_WriteReg(0x01);
    HAL_Delay(50);

    // Konfiguracja orientacji (MAC) i formatu piksela (RGB565)
    ILI9341_WriteReg(0x36);
    ILI9341_WriteData(0xC8);

    ILI9341_WriteReg(0x3A);
    ILI9341_WriteData(0x55);

    // Przełączenie interfejsu na zewnętrzny sygnał RGB (LTDC) i pamięć GRAM
    ILI9341_WriteReg(0xB0);
    ILI9341_WriteData(0xC0);

    ILI9341_WriteReg(0xF6);
    ILI9341_WriteData(0x01);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x06);

    // Wyjście z trybu uśpienia (Sleep Out)
    ILI9341_WriteReg(0x11);
    HAL_Delay(120);

    // Włączenie wyświetlania (Display ON)
    ILI9341_WriteReg(0x29);
    ILI9341_WriteReg(0x2C);
}

// GŁÓWNE FUNKCJE GRAFICZNE I DOUBLE BUFFERING

// Inicjalizacja podsystemu graficznego (SDRAM, bufory, ekran LCD)
void Graphic_Init(void) {
    // Uruchomienie kości SDRAM
    SDRAM_Initialization_Sequence(&hsdram1);

    // Wyczyszczenie obu buforów w SDRAM na biało na starcie
    uint16_t *buf0 = (uint16_t *)FB_ADDRESS_0;
    uint16_t *buf1 = (uint16_t *)FB_ADDRESS_1;
    for (uint32_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT); i++) {
        buf0[i] = LCD_COLOR_WHITE;
        buf1[i] = LCD_COLOR_WHITE;
    }

    // Wybudzenie ekranu LCD po SPI5 i przełączenie w tryb LTDC
    ILI9341_Init();
}

// Wypełnienie bufora w tle zadanym kolorem
void Graphic_ClearBackBuffer(uint16_t color) {
    // Wypełnianie bufora w tle (po 2 piksele naraz jako 32-bit dla przyspieszenia)
    uint32_t packed_color = ((uint32_t)color << 16) | color;
    uint32_t *ptr32 = (uint32_t *)back_buffer;
    uint32_t count = (LCD_WIDTH * LCD_HEIGHT) / 2;

    for (uint32_t i = 0; i < count; i++) {
        ptr32[i] = packed_color;
    }
}

// Rysowanie pojedynczego piksela w buforze w tle
void Graphic_DrawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT) {
        back_buffer[y * LCD_WIDTH + x] = color;
    }
}

// Narysowanie wypełnionego koła (piłki)
void Graphic_DrawBall(Point2D center, uint16_t radius, uint16_t color) {
    int16_t r = (int16_t)radius;
    int32_t r2 = r * r;

    // Rysowanie wypełnionego koła z równania x^2 + y^2 <= r^2
    for (int16_t dy = -r; dy <= r; dy++) {
        for (int16_t dx = -r; dx <= r; dx++) {
            if ((dx * dx + dy * dy) <= r2) {
                Graphic_DrawPixel(center.x + dx, center.y + dy, color);
            }
        }
    }
}

// Zamiana buforów
void Graphic_SwapBuffers(void) {
    // Zleenie kontrolerowi LTDC przełączenie adresu na nasz świeżo narysowany bufor

    uint32_t new_front_addr = (current_back_buf_idx == 1) ? FB_ADDRESS_1 : FB_ADDRESS_0;
    HAL_LTDC_SetAddress_NoReload(&hltdc, new_front_addr, 0);
    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);

    while ((hltdc.Instance->SRCR & LTDC_SRCR_VBR) != 0) {
    }

    // Zamiana roli bufora tylnego
    if (current_back_buf_idx == 1) {
        current_back_buf_idx = 0;
        back_buffer = (volatile uint16_t *)FB_ADDRESS_0;
    } else {
        current_back_buf_idx = 1;
        back_buffer = (volatile uint16_t *)FB_ADDRESS_1;
    }
}

// Pomocnicza funkcja rysująca wypełnione koło dla obiektów mapy
static void Graphic_DrawFilledCircle(Point2D center, uint16_t radius, uint16_t color) {
    int16_t r = (int16_t)radius;
    int32_t r2 = r * r;

    for (int16_t dy = -r; dy <= r; dy++) {
        for (int16_t dx = -r; dx <= r; dx++) {
            if ((dx * dx + dy * dy) <= r2) {
                Graphic_DrawPixel(center.x + dx, center.y + dy, color);
            }
        }
    }
}

// Rysowanie dziur
void Graphic_DrawHoles(const MapObject *holes, uint8_t hole_count) {
    for (uint8_t i = 0; i < hole_count; i++) {
        if (!holes[i].active) continue;
        Graphic_DrawFilledCircle(holes[i].pos, (uint16_t)holes[i].radius, LCD_COLOR_BLACK);
    }
}

// Rysowanie monet
void Graphic_DrawCoins(const MapObject *coins, uint8_t coin_count) {
    for (uint8_t i = 0; i < coin_count; i++) {
        if (!coins[i].active) continue;
        Graphic_DrawFilledCircle(coins[i].pos, (uint16_t)coins[i].radius, LCD_COLOR_YELLOW);
    }
}
