#include "physics.h"

void Ball_Init(Ball *ball) {
    ball->position.x = SCREEN_WIDTH / 2.0f;
    ball->position.y = SCREEN_HEIGHT / 2.0f;
    ball->velocity.x = 0.0f;
    ball->velocity.y = 0.0f;
    ball->acceleration.x = 0.0f;
    ball->acceleration.y = 0.0f;

    ball->radius = BALL_RADIUS;
    ball->restitution = RESTITUTION;
    ball->friction = FRICTION;
}

Point2D Ball_GetScreenPosition(const Ball *ball) {
    Point2D pos;
    pos.x = (int16_t)(ball->position.x + 0.5f);
    pos.y = (int16_t)(ball->position.y + 0.5f);
    return pos;
}

void Ball_Update(Ball *ball, float gyro_angle_x, float gyro_angle_y, float dt) {
    const float ACCEL_SCALE = 150.0f;
    ball->acceleration.x = gyro_angle_x * ACCEL_SCALE;
    ball->acceleration.y = gyro_angle_y * ACCEL_SCALE;

    ball->velocity.x += ball->acceleration.x * dt;
    ball->velocity.y += ball->acceleration.y * dt;

    ball->velocity.x *= ball->friction;
    ball->velocity.y *= ball->friction;

    ball->position.x += ball->velocity.x * dt;
    ball->position.y += ball->velocity.y * dt;

    if (ball->position.x - ball->radius < 0.0f) {
        ball->position.x = ball->radius;
        ball->velocity.x = -ball->velocity.x * ball->restitution;
    } else if (ball->position.x + ball->radius > SCREEN_WIDTH) {
        ball->position.x = SCREEN_WIDTH - ball->radius;
        ball->velocity.x = -ball->velocity.x * ball->restitution;
    }

    if (ball->position.y - ball->radius < 0.0f) {
        ball->position.y = ball->radius;
        ball->velocity.y = ball->radius;
        ball->velocity.y = -ball->velocity.y * ball->restitution;
    } else if (ball->position.y + ball->radius > SCREEN_HEIGHT) {
        ball->position.y = SCREEN_HEIGHT - ball->radius;
        ball->velocity.y = -ball->velocity.y * ball->restitution;
    }
}
