#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;

float posx = 0.0f;
float posz = 0.0f;
float angle = 0.0f;
float speed = 0.1f;
float rotation_speed = 0.05f;

bool keyUp = false;
bool keyDown = false;
bool keyLeft = false;
bool keyRight = false;

void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = zNear * tanf(fovY * M_PI / 360.0f);
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

void drawCar() {
    glPushMatrix();
    glTranslatef(posx, 0.0f, posz);
    glRotatef(angle * 180.0f / (float)M_PI, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, 0.5f, -1.0f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -1.0f);

    glColor3f(0.5f, 0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -1.0f);
    glVertex3f(0.5f, -0.5f, -1.0f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    glColor3f(0.0f, 0.0f, 0.5f);
    glVertex3f(-0.5f, -0.5f, -1.0f);
    glVertex3f(-0.5f, 0.5f, -1.0f);
    glVertex3f(0.5f, 0.5f, -1.0f);
    glVertex3f(0.5f, -0.5f, -1.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -1.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -1.0f);

    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, -1.0f);
    glVertex3f(0.5f, 0.5f, -1.0f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    glEnd();
    glPopMatrix();
}

void drawGround() {
    glBegin(GL_LINES);
    glColor3f(0.5f, 0.5f, 0.5f);
    for (int i = -10; i <= 10; i++) {
        glVertex3f(-10.0f, -0.5f, (GLfloat)i);
        glVertex3f(10.0f, -0.5f, (GLfloat)i);
    }
    for (int i = -10; i <= 10; i++) {
        glVertex3f((GLfloat)i, -0.5f, -10.0f);
        glVertex3f((GLfloat)i, -0.5f, 10.0f);
    }
    glEnd();
}

void processInput() {
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    float moveSpeed = speed * deltaTime * 60.0f;
    float turnSpeed = rotation_speed * deltaTime * 60.0f;

    if (keyLeft) {
        angle += turnSpeed;
        if (angle >= 2.0f * (float)M_PI) angle -= 2.0f * (float)M_PI;
    }
    if (keyRight) {
        angle -= turnSpeed;
        if (angle < 0.0f) angle += 2.0f * (float)M_PI;
    }

    float dx = moveSpeed * cosf(angle);
    float dz = moveSpeed * sinf(angle);

    if (keyUp) {
        posx += dx;
        posz -= dz;
    }
    if (keyDown) {
        posx -= dx;
        posz += dz;
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Car Movement", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("Car Movement", "1.0", "OpenGL Car Demo");

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 500.0f);

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.key) {
        case SDLK_UP: keyUp = true; break;
        case SDLK_DOWN: keyDown = true; break;
        case SDLK_LEFT: keyLeft = true; break;
        case SDLK_RIGHT: keyRight = true; break;
        case SDLK_ESCAPE: return SDL_APP_SUCCESS;
        }
    }
    else if (event->type == SDL_EVENT_KEY_UP) {
        switch (event->key.key) {
        case SDLK_UP: keyUp = false; break;
        case SDLK_DOWN: keyDown = false; break;
        case SDLK_LEFT: keyLeft = false; break;
        case SDLK_RIGHT: keyRight = false; break;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    processInput();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    drawGround();
    drawCar();

    SDL_GL_SwapWindow(window);
    SDL_Delay(16); // ~60 FPS

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
