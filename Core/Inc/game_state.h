#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "physics.h"

struct Ball;
struct MapObject;

// Stany gry
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER,
    GAME_STATE_VICTORY
} GameState;

// Główna struktura stanu gry
typedef struct {
    GameState state;
    uint32_t score;
    uint32_t time_ms;
    uint8_t coins_left;
} GameSession;

void Game_Init(GameSession *session, MapObject *coins, uint8_t coin_count, MapObject *holes, uint8_t hole_count);
void Game_Update(GameSession *session, Ball *ball, MapObject *holes, uint8_t hole_count, MapObject *coins, uint8_t coin_count);

#endif // GAME_STATE_H
