#define SDL_MAIN_USE_CALLBACKS 1 
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define STEP_RATE_IN_MILLISECONDS 25

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;
float mainBladeRotation = 0.0f;
float tailBladeRotation = 0.0f;
float viewAngle = 30.0f;

void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = 1;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

void DrawCube() {
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);

    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);

    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);

    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();
}

void DrawHelicopter() {
    glPushMatrix();
    glColor3f(0.2f, 0.5f, 0.8f); 
    glScalef(3.0f, 1.0f, 1.2f);  
    DrawCube();
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.7f);
    glTranslatef(-2.5f, 0.0f, 0.0f);
    glScalef(3.0f, 0.3f, 0.3f);
    DrawCube();
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.1f, 0.1f, 0.1f);
    glTranslatef(0.0f, 0.6f, 0.0f);
    glScalef(0.3f, 0.2f, 0.3f);
    DrawCube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.7f, 0.0f);
    glRotatef(mainBladeRotation, 0.0f, 1.0f, 0.0f);
    glColor3f(0.7f, 0.7f, 0.7f);

    glScalef(0.1f, 0.05f, 6.0f);
    DrawCube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.7f, 0.0f);
    glRotatef(mainBladeRotation, 0.0f, 1.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); 
    glColor3f(0.7f, 0.7f, 0.7f);

    glScalef(0.1f, 0.05f, 6.0f);
    DrawCube();
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.1f, 0.1f, 0.1f);
    glTranslatef(-4.0f, 0.0f, 0.3f);
    glScalef(0.2f, 0.2f, 0.2f);
    DrawCube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.0f, 0.0f, 0.4f); 
    glRotatef(tailBladeRotation, 0.0f, 0.0f, 1.0f); 
    glColor3f(0.7f, 0.7f, 0.7f);

    glScalef(0.05f, 2.0f, 0.05f);
    DrawCube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.0f, 0.0f, 0.4f);
    glRotatef(tailBladeRotation, 0.0f, 0.0f, 1.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(0.7f, 0.7f, 0.7f);

    glScalef(0.05f, 2.0f, 0.05f);
    DrawCube();
    glPopMatrix();
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Simple Helicopter with Rotating Blades", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("Simple Helicopter with Rotating Blades", "1.0", "com.bohdanstarunskyi.helicopter");

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

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    currentTime = SDL_GetTicks();
    if (currentTime > previousTime + STEP_RATE_IN_MILLISECONDS) {
        mainBladeRotation += 15.0f;
        if (mainBladeRotation > 360.0f) mainBladeRotation -= 360.0f;

        tailBladeRotation += 30.0f;
        if (tailBladeRotation > 360.0f) tailBladeRotation -= 360.0f;

        viewAngle += 0.2f;
        if (viewAngle > 360.0f) viewAngle -= 360.0f;

        previousTime = currentTime;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -15.0f);
    glRotatef(10.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(viewAngle, 0.0f, 1.0f, 0.0f); 

    DrawHelicopter();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
}