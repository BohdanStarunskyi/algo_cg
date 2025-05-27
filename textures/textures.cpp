#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define STEP_RATE_IN_MILLISECONDS 25

#include <cmath>
#include <cstdio>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;

float rotationAngle = 0.0f;
float posX = 0.0f, posZ = 0.0f;
float moveSpeed = 0.1f;

GLuint textureGrass = 0;
GLuint textureWood = 0;

void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = tanf((fovY * M_PI / 180.0f) * 0.5f) * zNear;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

GLuint LoadTexture(const char* filename)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        SDL_Log("Failed to load texture %s\n", filename);
        return 0;
    }
    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return texId;
}

void DrawCube()
{
    glBindTexture(GL_TEXTURE_2D, textureWood);
    glBegin(GL_QUADS);

    //top
    glTexCoord2f(1, 1); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, 0.5f, 0.5f);

    //bottom
    glTexCoord2f(1, 1); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, -0.5f, -0.5f);

    //front
    glTexCoord2f(1, 1); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, -0.5f, 0.5f);

    //back
    glTexCoord2f(1, 1); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, 0.5f, -0.5f);

    //left
    glTexCoord2f(1, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1, 0); glVertex3f(-0.5f, -0.5f, 0.5f);

    //right
    glTexCoord2f(1, 1); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, 1); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 0); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, -0.5f, -0.5f);

    glEnd();
}

void DrawGround()
{
    glBindTexture(GL_TEXTURE_2D, textureGrass);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, -0.5f, 10.0f);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(10.0f, -0.5f, 10.0f);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(10.0f, -0.5f, -10.0f);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-10.0f, -0.5f, -10.0f);
    glEnd();
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Textured Cube (SDL3 + OpenGL)", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("Textured Cube Example", "1.0", "com.example.sdl3texturedcube");

    glcontext = SDL_GL_CreateContext(window);
    if (!glcontext) {
        SDL_Log("Couldn't create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        return SDL_APP_FAILURE;
    }

    glClearColor(0.3f, 0.5f, 0.9f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 500.0f);

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    textureGrass = LoadTexture("grass.jpg");
    textureWood = LoadTexture("wood.jpg");

    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event->key.key;
        switch (key) {
            case SDLK_LEFT:
                rotationAngle += 5.0f;
                if (rotationAngle > 360.0f) rotationAngle -= 360.0f;
                break;
            case SDLK_RIGHT:
                rotationAngle -= 5.0f;
                if (rotationAngle < 0.0f) rotationAngle += 360.0f;
                break;
            case SDLK_UP: {
                float rad = rotationAngle * M_PI / 180.0f;
                posX += sinf(rad) * moveSpeed;
                posZ += cosf(rad) * moveSpeed;
                break;
            }
            case SDLK_DOWN: {
                float rad = rotationAngle * M_PI / 180.0f;
                posX -= sinf(rad) * moveSpeed;
                posZ -= cosf(rad) * moveSpeed;
                break;
            }
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    currentTime = SDL_GetTicks();
    if (currentTime > previousTime + STEP_RATE_IN_MILLISECONDS) {
        previousTime = currentTime;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -15.0f);

    glTranslatef(posX, 0.0f, posZ);
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    DrawCube();

    DrawGround();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    if (textureGrass) {
        glDeleteTextures(1, &textureGrass);
    }
    if (textureWood) {
        glDeleteTextures(1, &textureWood);
    }
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
