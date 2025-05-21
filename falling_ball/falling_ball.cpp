#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <stdbool.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BALL_RADIUS 20.0f

//initial value (middle of screen)
float ballY = WINDOW_HEIGHT / 3.0f;
float velocityY = 0.0f;

const float GRAVITY = -9.81f;
const float GROUND_Y = -WINDOW_HEIGHT / 2.0f + 50;

SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;

void drawBall() {
    glColor3f(1.0f, 0.5f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, ballY);

    for (int i = 0; i <= 360; i += 10) {
        float angle = i * M_PI / 180.0f;
        float x = BALL_RADIUS * cosf(angle);
        float y = BALL_RADIUS * sinf(angle);
        glVertex2f(x, ballY + y);
    }

    glEnd();
}

void drawGround() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glEnd();
}

void updatePhysics(float dt) {
    ballY += velocityY * dt + 0.5f * GRAVITY * dt * dt;
    velocityY += GRAVITY * dt;

    if (ballY - BALL_RADIUS < GROUND_Y) {
        ballY = GROUND_Y + BALL_RADIUS;
        velocityY = 0.0f;
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Gravity Ball", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2.0f, WINDOW_WIDTH / 2.0f,
        -WINDOW_HEIGHT / 2.0f, WINDOW_HEIGHT / 2.0f,
        -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    currentTime = SDL_GetTicks();
    float dt = (currentTime - previousTime) / 1000.0f; // seconds
    previousTime = currentTime;

    updatePhysics(dt);

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGround();
    drawBall();

    SDL_GL_SwapWindow(window);
    SDL_Delay(16); // ~60 FPS

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}