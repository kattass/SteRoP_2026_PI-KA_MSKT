#include "game_state.h"
#include "physics.h"

void Game_Init(GameSession *session, MapObject *coins, uint8_t coin_count, MapObject *holes, uint8_t hole_count) {
    session->state = GAME_STATE_PLAYING;
    session->score = 0;
    session->time_ms = 0;
    session->coins_left = coin_count;

    // Reset aktywacji monet
    for (uint8_t i = 0; i < coin_count; i++) {
        coins[i].active = true;
    }

    // Reset aktywacji dziur
    for (uint8_t i = 0; i < hole_count; i++) {
        holes[i].active = true;
    }
}

void Game_Update(GameSession *session, Ball *ball, MapObject *holes, uint8_t hole_count, MapObject *coins, uint8_t coin_count) {
    if (session->state != GAME_STATE_PLAYING) return;

    // Sprawdzenie kolizji
    GameEvent event = Physics_CheckCollisions(ball, holes, hole_count, coins, coin_count);

    if (event == GAME_EVENT_HOLE_HIT) {
        // Przegrana – piłka wpada do dziury
        session->state = GAME_STATE_GAME_OVER;
    }
    else if (event == GAME_EVENT_COIN_COLLECTED) {
        // Zebranie monety
        session->score += 100;
        if (session->coins_left > 0) {
            session->coins_left--;
        }

        // Sprawdzenie warunku wygranej (zebrane wszystkie monety)
        if (session->coins_left == 0) {
            session->state = GAME_STATE_VICTORY;
        }
    }
}
