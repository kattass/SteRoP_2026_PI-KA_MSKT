#ifndef SDRAM_LTDC_H
#define SDRAM_LTDC_H

#include "stm32f4xx_hal.h"
#include "physics.h"

#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// Adresy dwóch buforów w pamięci SDRAM
#define FB_ADDRESS_0  ((uint32_t)0xD0000000)
#define FB_ADDRESS_1  ((uint32_t)0xD0025800)

#define LCD_COLOR_WHITE   0xFFFF
#define LCD_COLOR_BLACK   0x0000
#define LCD_COLOR_RED     0xF800
#define LCD_COLOR_GREEN   0x07E0
#define LCD_COLOR_BLUE    0x001F
#define LCD_COLOR_YELLOW  0xF5A7
#define LCD_COLOR_PINK    0xF81F

// Inicjalizacja podsystemu graficznego
void Graphic_Init(void);

// Funkcje podwójnego buforowania i rysowania
void Graphic_ClearBackBuffer(uint16_t color);
void Graphic_DrawPixel(int16_t x, int16_t y, uint16_t color);
void Graphic_DrawBall(Point2D center, uint16_t radius, uint16_t color);
void Graphic_SwapBuffers(void);

// Rysowanie dziur na planszy
void Graphic_DrawHoles(const MapObject *holes, uint8_t hole_count);
// Rysowanie monet na planszy
void Graphic_DrawCoins(const MapObject *coins, uint8_t coin_count);

#endif /* SDRAM_LTDC_H */
