#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;

float topRotorAngle = 0.0f;
float tailRotorAngle = 0.0f;

void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = 1;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

void DrawCube() {
    glBegin(GL_QUADS);

    //front face
    glVertex3f(-0.5f, -0.5f, 0.5f); 
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);    
    glVertex3f(-0.5f, 0.5f, 0.5f);

    //back face
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);   
    glVertex3f(0.5f, -0.5f, -0.5f);

    //top face
    glVertex3f(-0.5f, 0.5f, -0.5f);  
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);    
    glVertex3f(0.5f, 0.5f, -0.5f);

    //bottom face
    glVertex3f(-0.5f, -0.5f, -0.5f); 
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);   
    glVertex3f(-0.5f, -0.5f, 0.5f);

    //right face
    glVertex3f(0.5f, -0.5f, -0.5f);  
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);    
    glVertex3f(0.5f, -0.5f, 0.5f);

    //left face
    glVertex3f(-0.5f, -0.5f, -0.5f); 
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);   
    glVertex3f(-0.5f, 0.5f, -0.5f);

    glEnd();
}

void DrawHelicopter() {
    //main body
    glPushMatrix();
    glColor3f(0.2f, 0.5f, 0.8f);         
    glScalef(3.0f, 1.0f, 1.0f);           
    DrawCube();
    glPopMatrix();

    //tail
    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.7f);           
    glTranslatef(-2.5f, 0.0f, 0.0f);        
    glScalef(2.0f, 0.2f, 0.2f);          
    DrawCube();
    glPopMatrix();

    //top rotor
    glPushMatrix();
    glColor3f(0.8f, 0.8f, 0.8f);        
    glTranslatef(0.0f, 0.6f, 0.0f);         
    glRotatef(topRotorAngle, 0, 1, 0);  
    glScalef(0.1f, 0.05f, 4.0f);     
    DrawCube();
    glPopMatrix();

    //tail rotor
    glPushMatrix();
    glColor3f(0.8f, 0.8f, 0.8f);            
    glTranslatef(-4.0f, 0.0f, 0.3f);        
    glRotatef(tailRotorAngle, 0, 0, 1);
    glScalef(0.05f, 1.0f, 0.05f);
    DrawCube();
    glPopMatrix();
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return SDL_APP_FAILURE;

    window = SDL_CreateWindow("Simple Helicopter", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) return SDL_APP_FAILURE;

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45.0, (double)WINDOW_WIDTH / WINDOW_HEIGHT, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(2.0f, -5.0f, -15.0f);

    topRotorAngle += 10.0f;
    if (topRotorAngle > 360.0f) topRotorAngle -= 360.0f;

    tailRotorAngle += 20.0f;
    if (tailRotorAngle > 360.0f) tailRotorAngle -= 360.0f;

    DrawHelicopter();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
}
