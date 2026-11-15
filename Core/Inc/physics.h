#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdint.h>

#define SCREEN_WIDTH  240.0f
#define SCREEN_HEIGHT 320.0f
#define BALL_RADIUS   8.0f
#define RESTITUTION   0.75f  
#define FRICTION      0.985f

typedef struct {
    float x; 
    float y;
} Vector2D;

typedef struct {
    int16_t x; 
    int16_t y; 
} Point2D;

typedef struct {
    Vector2D position;     
    Vector2D velocity;     
    Vector2D acceleration; 

    float radius;          
    float restitution;     
    float friction;        
} Ball;

void Ball_Init(Ball *ball);
void Ball_Update(Ball *ball, float gyro_angle_x, float gyro_angle_y, float dt);
Point2D Ball_GetScreenPosition(const Ball *ball);

#endif // PHYSICS_H
