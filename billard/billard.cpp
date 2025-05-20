#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define TABLE_WIDTH 700
#define TABLE_HEIGHT 400

#define BALL_RADIUS 10.0f
#define INITIAL_SPEED 0.1f

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;

Uint64 previousTime, currentTime;

float x = 0.0f;
float y = 0.0f;

float dx = 0.0f;
float dy = 0.0f;

float speed = INITIAL_SPEED;

//initial angle
float angle = 45.0f;

bool ballMoving = false;

float tableLeft = -TABLE_WIDTH / 2.0f;
float tableRight = TABLE_WIDTH / 2.0f;
float tableTop = TABLE_HEIGHT / 2.0f;
float tableBottom = -TABLE_HEIGHT / 2.0f;

void drawBall() {
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (int i = 0; i <= 360; i += 10) {
        float angle_rad = i * M_PI / 180.0f;
        float px = x + BALL_RADIUS * cosf(angle_rad);
        float py = y + BALL_RADIUS * sinf(angle_rad);
        glVertex2f(px, py);
    }
    glEnd();
}

void drawTable() {
    glColor3f(0.0f, 0.5f, 0.0f);

    //table surface
    glBegin(GL_QUADS);
    glVertex2f(tableLeft, tableBottom);
    glVertex2f(tableRight, tableBottom);
    glVertex2f(tableRight, tableTop);
    glVertex2f(tableLeft, tableTop);
    glEnd();

    //table border
    glColor3f(0.5f, 0.25f, 0.0f);  
    glLineWidth(10.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(tableLeft, tableBottom);
    glVertex2f(tableRight, tableBottom);
    glVertex2f(tableRight, tableTop);
    glVertex2f(tableLeft, tableTop);
    glEnd();
    glLineWidth(1.0f);
}

void updateBall() {
    if (ballMoving) {
        x += dx;
        y += dy;

      
        if (y + BALL_RADIUS > tableTop) {
            dy = -dy;
        }
        else if (y - BALL_RADIUS < tableBottom) {
            dy = -dy;
        }

      
        if (x + BALL_RADIUS > tableRight) {
            dx = -dx;
        }

        else if (x - BALL_RADIUS < tableLeft) {
            dx = -dx;
        }
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Billiard Simulation", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glcontext = SDL_GL_CreateContext(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.key) {
        case SDLK_SPACE:
            if (!ballMoving) {
                float rad_angle = angle * M_PI / 180.0f;
                dx = speed * cosf(rad_angle);
                dy = speed * sinf(rad_angle);
                ballMoving = true;
            }
            break;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    updateBall();

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawTable();
    drawBall();

    SDL_GL_SwapWindow(window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
