#define SDL_MAIN_USE_CALLBACKS 1 
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;

float groundLevel = 0.0f;

float verticalVelocity = 0.0f;
float gravity = -0.09f;

float obstacleX = WINDOW_WIDTH - 50.0f;
float obstacleMovementSpeed = 0.5f;
float obstacleStartX = WINDOW_WIDTH - 50.0f;
float obstacleResetDistance = 300.0f;

void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = 1;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("OpenGL House", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("OpenGL House", "1.0", "com.bohdanstarunskyi.house2d");

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_SPACE) {
            verticalVelocity += 300.0f;
        }
    }
    return SDL_APP_CONTINUE;
}

void DrawBird() {
    float bottom = groundLevel;
    float ballRadius = 15.0f;
    float ballCenterX = 50.0f;
    float ballCenterY = bottom + ballRadius + verticalVelocity;

    glColor3f(0.2f, 0.4f, 0.9f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(ballCenterX, ballCenterY);
    int numSegments = 32;
    for (int i = 0; i <= numSegments; ++i) {
        float angle = 2.0f * 3.1415926f * i / numSegments;
        float dx = cosf(angle) * ballRadius;
        float dy = sinf(angle) * ballRadius;
        glVertex2f(ballCenterX + dx, ballCenterY + dy);
    }
    glEnd();
}

void DrawObstacleShape() {
    float obstacleY = groundLevel + 50.0f;
    glColor3f(0.8f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(obstacleX - 20.0f, obstacleY - 50.0f);
    glVertex2f(obstacleX + 20.0f, obstacleY - 50.0f);
    glVertex2f(obstacleX + 20.0f, obstacleY + 50.0f);
    glVertex2f(obstacleX - 20.0f, obstacleY + 50.0f);
    glEnd();
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    if (verticalVelocity > 0) {
        verticalVelocity += gravity;
    }
    obstacleX -= obstacleMovementSpeed;
    if (obstacleStartX - obstacleX >= obstacleResetDistance) {
        obstacleX = obstacleStartX;
    }
    DrawBird();
    DrawObstacleShape();
    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
}
