#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>

#ifdef __3DS__
#include <3ds.h>
#include <GL/picaGL.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengles.h>
#include <SDL2/SDL_opengl_glext.h>
#include "SDLGL.h"
#include "ZipFile.h"

#include "CAppContainer.h"
#include "App.h"
#include "Image.h"
#include "Resource.h"
#include "Render.h"
#include "GLES.h"

#include "Canvas.h"
#include "Graphics.h"
#include "Player.h"
#include "Game.h"
#include "Graphics.h"
#include "Utils.h"
#include "TinyGL.h"
#include "Input.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <unistd.h>

void drawView(SDLGL* sdlGL);
#undef SDL_Main
#define main main
int main()
{
    //socketInitializeDefault();
    //nxlinkStdio();
    int		UpTime = 0;

    ZipFile zipFile;
    #ifdef __aarch64__
    zipFile.openZipFile("/switch/doom2rpg/DoomIIRPG.ipa");
    #elif __3DS__
    freopen("/3ds/stderr.log", "w", stderr);
    freopen("/3ds/stddut.log", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    zipFile.openZipFile("/3ds/doom2rpg/DoomIIRPG.ipa");
    pglInitEx(0x040000, 0x1000000);
    #endif
    printf("Opened ZIP-File & inited PGL\n");

    SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
    SDLGL sdlGL;
    sdlGL.Initialize();

    Input input;
    input.init();

    CAppContainer::getInstance()->Construct(&sdlGL, &zipFile);
    sdlGL.updateVideo(); // [GEC]

    SDL_Event ev;
    printf("reached mainloop\n");
    while (CAppContainer::getInstance()->app->closeApplet != true) {

        int currentTimeMillis = CAppContainer::getInstance()->getTimeMS();

        if (currentTimeMillis > UpTime) {
            input.handleEvents();
            UpTime = currentTimeMillis + 15;
            drawView(&sdlGL);
            input.consumeEvents();
        }
    }

    printf("APP_QUIT\n");
    CAppContainer::getInstance()->~CAppContainer();
    zipFile.closeZipFile();
    sdlGL.~SDLGL();
    input.~Input();
    return 0;
}
void SDL_Main() {
    main();
}


static uint32_t lastTimems = 0;

void drawView(SDLGL *sdlGL) {

    int cx, cy;
    int w = sdlGL->vidWidth;
    int h = sdlGL->vidHeight;

    if (lastTimems == 0) {
        lastTimems = CAppContainer::getInstance()->getTimeMS();
    }

    SDL_GetWindowSize(sdlGL->window, &cx, &cy);
    cx = 400;
    cy = 240;
    if (w != cx || h != cy) {
        w = cx; h = cy;
    }

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef __3DS__
    glOrtho(0.0, Applet::IOS_WIDTH, Applet::IOS_HEIGHT, 0.0, -1.0, 1.0);
#else
    Render::glOrthof(0.0, Applet::IOS_WIDTH, Applet::IOS_HEIGHT, 0.0, -1.0, 1.0);
#endif
    //glRotatef(90.0, 0.0, 0.0, 1.0);
    //glTranslatef(0.0, -320.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    

    uint32_t startTime = CAppContainer::getInstance()->getTimeMS();
    uint32_t passedTime = startTime - lastTimems;
    lastTimems = startTime;

    if (passedTime >= 125) {
        passedTime = 125;
    }
    //printf("passedTime %d\n", passedTime);

    CAppContainer::getInstance()->DoLoop(passedTime);

    SDL_GL_SwapWindow(sdlGL->window);  // Swap the window/pBmp to display the result.
    
}