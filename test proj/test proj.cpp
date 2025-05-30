#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define BIRD_RADIUS 15.0f
#define PIPE_WIDTH 80.0f
#define PIPE_GAP 200.0f
#define PIPE_SPEED 200.0f
#define NUM_PIPES 3

// Bird state
float birdY = 0.0f;
float birdVelocityY = 0.0f;

// Physics constants
const float GRAVITY = -900.0f;
const float FLAP_STRENGTH = 300.0f;

// Ground Y position (bottom boundary)
const float GROUND_Y = -WINDOW_HEIGHT / 2.0f + 50.0f;

// Game state flags
bool isGameOver = false;
bool hasPrintedGameOverMessage = false;

// SDL window and OpenGL context
SDL_Window* window = NULL;
SDL_GLContext glContext = NULL;

// Timing variables
Uint64 previousTime = 0;
Uint64 currentTime = 0;

// Pipe structure
typedef struct {
    float x;
    float gapY;
} Pipe;

Pipe pipes[NUM_PIPES];

// Initialize pipes with starting positions and random gaps
void initPipes() {
    for (int i = 0; i < NUM_PIPES; ++i) {
        pipes[i].x = WINDOW_WIDTH / 2.0f + i * 300.0f;
        pipes[i].gapY = (rand() % 300) - 150; // random gap vertical position
    }
}

// Draw the bird as a circle using triangle fan
void drawBird() {
    glColor3f(0.918f, 0.675f, 0.545f);  // Bird color

    glBegin(GL_TRIANGLE_FAN);
    float birdX = -WINDOW_WIDTH / 4.0f; // Fixed X position
    glVertex2f(birdX, birdY);            // Center vertex

    for (int angleDeg = 0; angleDeg <= 360; angleDeg += 10) {
        float angleRad = angleDeg * M_PI / 180.0f;
        float x = BIRD_RADIUS * cosf(angleRad);
        float y = BIRD_RADIUS * sinf(angleRad);
        glVertex2f(birdX + x, birdY + y);
    }
    glEnd();
}

// Draw all pipes (top and bottom parts)
void drawPipes() {
    glColor3f(0.427f, 0.349f, 0.478f);  // Pipe color

    for (int i = 0; i < NUM_PIPES; ++i) {
        float x = pipes[i].x;
        float gapY = pipes[i].gapY;

        // Top pipe
        glBegin(GL_QUADS);
        glVertex2f(x, gapY + PIPE_GAP / 2.0f);
        glVertex2f(x + PIPE_WIDTH, gapY + PIPE_GAP / 2.0f);
        glVertex2f(x + PIPE_WIDTH, WINDOW_HEIGHT / 2.0f);
        glVertex2f(x, WINDOW_HEIGHT / 2.0f);
        glEnd();

        // Bottom pipe
        glBegin(GL_QUADS);
        glVertex2f(x, -WINDOW_HEIGHT / 2.0f);
        glVertex2f(x + PIPE_WIDTH, -WINDOW_HEIGHT / 2.0f);
        glVertex2f(x + PIPE_WIDTH, gapY - PIPE_GAP / 2.0f);
        glVertex2f(x, gapY - PIPE_GAP / 2.0f);
        glEnd();
    }
}

// Draw ground as a rectangle at bottom
void drawGround() {
    glColor3f(0.710f, 0.396f, 0.463f);  // Ground color

    glBegin(GL_QUADS);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, -WINDOW_HEIGHT + GROUND_Y);
    glVertex2f(-WINDOW_WIDTH / 2.0f, -WINDOW_HEIGHT + GROUND_Y);
    glEnd();
}

// Update bird physics and pipe positions
void updatePhysics(float deltaTime) {
    // Update bird position and velocity
    birdY += birdVelocityY * deltaTime;
    birdVelocityY += GRAVITY * deltaTime;

    // Update pipe positions and reset if off-screen
    for (int i = 0; i < NUM_PIPES; ++i) {
        pipes[i].x -= PIPE_SPEED * deltaTime;

        if (pipes[i].x + PIPE_WIDTH < -WINDOW_WIDTH / 2.0f) {
            pipes[i].x += NUM_PIPES * 300.0f; // Move pipe to right side
            pipes[i].gapY = (rand() % 300) - 150; // New random gap
        }
    }

    // Prevent bird from falling below ground
    if (birdY - BIRD_RADIUS < GROUND_Y) {
        birdY = GROUND_Y + BIRD_RADIUS;
        birdVelocityY = 0.0f;
    }
}

// Check collisions between bird and pipes or ground
void checkCollision() {
    float birdX = -WINDOW_WIDTH / 4.0f;

    for (int i = 0; i < NUM_PIPES; ++i) {
        float pipeLeft = pipes[i].x;
        float pipeRight = pipeLeft + PIPE_WIDTH;
        float gapY = pipes[i].gapY;

        bool birdWithinPipeX = (birdX + BIRD_RADIUS > pipeLeft) && (birdX - BIRD_RADIUS < pipeRight);
        bool birdOutsidePipeGap = (birdY + BIRD_RADIUS > gapY + PIPE_GAP / 2.0f) ||
            (birdY - BIRD_RADIUS < gapY - PIPE_GAP / 2.0f);

        if (birdWithinPipeX && birdOutsidePipeGap) {
            isGameOver = true;
            return;
        }
    }

    if (birdY - BIRD_RADIUS < GROUND_Y) {
        isGameOver = true;
    }
}

// Make the bird flap upward
void flap() {
    birdVelocityY = FLAP_STRENGTH;
}

// Reset game state for a new round
void resetGame() {
    birdY = 0.0f;
    birdVelocityY = 0.0f;
    initPipes();
    isGameOver = false;
    hasPrintedGameOverMessage = false;
    previousTime = SDL_GetTicks();  // Reset timer to avoid large deltaTime
}

// Draw the Game Over screen with an X
void drawGameOver() {
    // Draw semi-transparent dark background
    glColor3f(0.110f, 0.114f, 0.129f);
    glBegin(GL_QUADS);
    glVertex2f(-200.0f, 200.0f);
    glVertex2f(200.0f, 200.0f);
    glVertex2f(200.0f, -200.0f);
    glVertex2f(-200.0f, -200.0f);
    glEnd();

    // Draw red X with thick lines
    glLineWidth(10.0f);
    glColor3f(0.733f, 0.608f, 0.690f);
    glBegin(GL_LINES);
    glVertex2f(-150.0f, -150.0f);
    glVertex2f(150.0f, 150.0f);

    glVertex2f(-150.0f, 150.0f);
    glVertex2f(150.0f, -150.0f);
    glEnd();

    // Print Game Over message once
    if (!hasPrintedGameOverMessage) {
        printf("GAME OVER - Press SPACE to restart\n");
        hasPrintedGameOverMessage = true;
    }
}

// SDL app initialization
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Flappy Bird Clone", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    glContext = SDL_GL_CreateContext(window);

    // Setup clear color and orthographic projection
    glClearColor(0.208f, 0.314f, 0.439f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2.0f, WINDOW_WIDTH / 2.0f,
        -WINDOW_HEIGHT / 2.0f, WINDOW_HEIGHT / 2.0f,
        -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    resetGame();
    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

// SDL event processing
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_SPACE) {
            if (isGameOver) {
                resetGame();
            }
            else {
                flap();
            }
        }
    }

    return SDL_APP_CONTINUE;
}

// Main loop iteration
SDL_AppResult SDL_AppIterate(void* appstate) {
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    if (!isGameOver) {
        updatePhysics(deltaTime);
        checkCollision();
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGround();
    drawPipes();
    drawBird();

    if (isGameOver) {
        drawGameOver();
    }

    SDL_GL_SwapWindow(window);
    SDL_Delay(16); // Roughly 60 FPS

    return SDL_APP_CONTINUE;
}

// Cleanup SDL and OpenGL
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
