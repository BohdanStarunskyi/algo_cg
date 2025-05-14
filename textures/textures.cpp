#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <cmath>
#include <iostream>
#include <Windows.h>

#define STBI_NO_SIMD
#define STBI_NO_LINEAR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float posX = 0.0f, posZ = 0.0f;
float angle = 0.0f;
const float speed = 0.1f;

// Texture IDs
GLuint textureGrass = 0;
GLuint textureWood = 0;

GLuint LoadTexture(const char* filename)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
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

    // Top face
    glTexCoord2f(1, 1); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, 0.5f, 0.5f);

    // Bottom face
    glTexCoord2f(1, 1); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, -0.5f, -0.5f);

    // Front face
    glTexCoord2f(1, 1); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, -0.5f, 0.5f);

    // Back face
    glTexCoord2f(1, 1); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(1, 0); glVertex3f(0.5f, 0.5f, -0.5f);

    // Left face
    glTexCoord2f(1, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0, 0); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1, 0); glVertex3f(-0.5f, -0.5f, 0.5f);

    // Right face
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

void Perspective(float fovY, float aspect, float nearZ, float farZ)
{
    float fH = tanf((fovY / 360.0f) * static_cast<float>(M_PI)) * nearZ;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, nearZ, farZ);
}

void init()
{
    glClearColor(0.3f, 0.5f, 0.9f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    Perspective(45.0f, 640.0f / 480.0f, 1.0f, 500.0f);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    textureGrass = LoadTexture("grass.jpg");
    textureWood = LoadTexture("wood.jpg");
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, -1.5f, -10.0f);
    DrawGround();
    glPushMatrix();
    glTranslatef(posX, 0.0f, posZ);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    DrawCube();
    glPopMatrix();
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
// Standard entry point for other platforms
int main(int argc, char* argv[])
#endif
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow("Movement with Textures (SDL3)", 640, 480, SDL_WINDOW_OPENGL);

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    if (!glcontext) {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_MakeCurrent(window, glcontext);
    SDL_GL_SetSwapInterval(1);

    init();

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                SDL_Keycode key = event.key.key;
                if (key == SDLK_LEFT) {
                    angle += 5.0f;
                }
                else if (key == SDLK_RIGHT) {
                    angle -= 5.0f;
                }
                else if (key == SDLK_UP) {
                    float rad = (angle * static_cast<float>(M_PI)) / 180.0f;
                    posX += speed * cosf(rad);
                    posZ -= speed * sinf(rad);
                }
                else if (key == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        display();
        SDL_GL_SwapWindow(window);
    }

    glDeleteTextures(1, &textureGrass);
    glDeleteTextures(1, &textureWood);
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}