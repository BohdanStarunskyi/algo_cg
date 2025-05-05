#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define STEP_RATE_IN_MILLISECONDS  25

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;
float angle_of_rotation = 0.0f;

/* Replace gluPerspective with this helper */
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

    SDL_SetAppMetadata("OpenGL House", "1.0", "sites.google.com/view/kamilniedzialomski");

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // Sky blue background

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 500.0f);  // replaces gluPerspective

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set the z-coordinate to -15 to make the house fit in the window
    float z = -15.0f;

    // Draw the house wall (rectangle/quad)
    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.6f, 0.2f);  // Brown for the wall
    glVertex3f(-2.0f, -2.0f, z);  // Bottom left
    glVertex3f(2.0f, -2.0f, z);   // Bottom right
    glVertex3f(2.0f, 1.0f, z);    // Top right
    glVertex3f(-2.0f, 1.0f, z);   // Top left
    glEnd();

    // Draw the roof (triangle)
    glBegin(GL_TRIANGLES);
    glColor3f(0.9f, 0.2f, 0.1f);  // Red for the roof
    glVertex3f(-2.5f, 1.0f, z);   // Bottom left
    glVertex3f(0.0f, 3.0f, z);    // Top middle
    glVertex3f(2.5f, 1.0f, z);    // Bottom right
    glEnd();

    // Draw the door
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.2f, 0.1f);  // Dark brown for the door
    glVertex3f(-0.6f, -2.0f, z);  // Bottom left
    glVertex3f(0.6f, -2.0f, z);   // Bottom right
    glVertex3f(0.6f, 0.0f, z);    // Top right
    glVertex3f(-0.6f, 0.0f, z);   // Top left
    glEnd();

    // Draw a window
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.9f, 1.0f);  // Light blue for the window
    glVertex3f(-1.5f, -0.5f, z);  // Bottom left
    glVertex3f(-0.7f, -0.5f, z);  // Bottom right
    glVertex3f(-0.7f, 0.5f, z);   // Top right
    glVertex3f(-1.5f, 0.5f, z);   // Top left
    glEnd();

    // Draw another window
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.9f, 1.0f);  // Light blue for the window
    glVertex3f(0.7f, -0.5f, z);   // Bottom left
    glVertex3f(1.5f, -0.5f, z);   // Bottom right
    glVertex3f(1.5f, 0.5f, z);    // Top right
    glVertex3f(0.7f, 0.5f, z);    // Top left
    glEnd();

    // Draw the window frames
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);  // Black for the window frames

    // First window frame
    glVertex3f(-1.5f, 0.0f, z);
    glVertex3f(-0.7f, 0.0f, z);
    glVertex3f(-1.1f, -0.5f, z);
    glVertex3f(-1.1f, 0.5f, z);

    // Second window frame
    glVertex3f(0.7f, 0.0f, z);
    glVertex3f(1.5f, 0.0f, z);
    glVertex3f(1.1f, -0.5f, z);
    glVertex3f(1.1f, 0.5f, z);
    glEnd();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
}