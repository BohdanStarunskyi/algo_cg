#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define STEP_RATE_IN_MILLISECONDS  25

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;
float rotationAngle = 0.0f;

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

    window = SDL_CreateWindow("3D OpenGL House", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("3D OpenGL House", "1.0", "com.bohdanstarunskyi.house3d");

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
        rotationAngle += 0.5f;
        if (rotationAngle > 360.0f) {
            rotationAngle -= 360.0f;
        }
        previousTime = currentTime;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, -1.0f, -15.0f);

    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    float width = 5.0f;   
    float height = 3.0f;  
    float depth = 5.0f;  
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;

    float wallColor[3] = { 0.8f, 0.6f, 0.2f };  

    //4 walls
    glBegin(GL_QUADS);
    glColor3fv(wallColor);

    //front
    glVertex3f(-halfWidth, 0.0f, halfDepth);       
    glVertex3f(halfWidth, 0.0f, halfDepth);        
    glVertex3f(halfWidth, height, halfDepth);      
    glVertex3f(-halfWidth, height, halfDepth);    

    //back
    glColor3fv(wallColor);
    glVertex3f(-halfWidth, 0.0f, -halfDepth);      
    glVertex3f(halfWidth, 0.0f, -halfDepth);       
    glVertex3f(halfWidth, height, -halfDepth);     
    glVertex3f(-halfWidth, height, -halfDepth);   

    //left
    glColor3fv(wallColor);
    glVertex3f(-halfWidth, 0.0f, halfDepth);       
    glVertex3f(-halfWidth, 0.0f, -halfDepth);     
    glVertex3f(-halfWidth, height, -halfDepth);    
    glVertex3f(-halfWidth, height, halfDepth);     

    //right
    glColor3fv(wallColor);
    glVertex3f(halfWidth, 0.0f, halfDepth);        
    glVertex3f(halfWidth, 0.0f, -halfDepth);       
    glVertex3f(halfWidth, height, -halfDepth);    
    glVertex3f(halfWidth, height, halfDepth);  

	//floor
    glColor3f(0.5f, 0.5f, 0.5f);  
    glVertex3f(-halfWidth, 0.0f, halfDepth);       
    glVertex3f(halfWidth, 0.0f, halfDepth);       
    glVertex3f(halfWidth, 0.0f, -halfDepth);       
    glVertex3f(-halfWidth, 0.0f, -halfDepth); 

    glEnd();

    float roofHeight = 3.0f;
    float roofPeak = height + roofHeight;
    float roofColor[3] = { 0.9f, 0.2f, 0.1f }; 

    //roof
    glBegin(GL_TRIANGLES);
    glColor3fv(roofColor);

    //front roof
    glVertex3f(-halfWidth, height, halfDepth);     
    glVertex3f(0.0f, roofPeak, 0.0f);              
    glVertex3f(halfWidth, height, halfDepth);   

	//back roof
    glColor3fv(roofColor);
    glVertex3f(-halfWidth, height, -halfDepth);    
    glVertex3f(0.0f, roofPeak, 0.0f);             
    glVertex3f(halfWidth, height, -halfDepth);    

	//left roof
    glColor3fv(roofColor);
    glVertex3f(-halfWidth, height, halfDepth);     
    glVertex3f(0.0f, roofPeak, 0.0f);            
    glVertex3f(-halfWidth, height, -halfDepth); 

	//right roof
    glColor3fv(roofColor);
    glVertex3f(halfWidth, height, halfDepth);      
    glVertex3f(0.0f, roofPeak, 0.0f);             
    glVertex3f(halfWidth, height, -halfDepth);   
    glEnd();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
}