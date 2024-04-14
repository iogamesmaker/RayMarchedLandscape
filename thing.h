//
// My own little "simplification" of the way overcomplicated naming of some of the SDL fucntion.
// If you happen to stumble upon this, and for some reason want to use it, feel free to. Credit is, as always, appreciated, but not neccesary.
// Or you can just say "hey thanks for making that really weird "simplification" of SDL2 back in 2024" to give me an ego boost.
// but yeah, use it for anything, modify it to shits and bits
//
// - iogamesplayer (or iogamesmaker, for now ;) ) 5 jan. 2024, 21:01 (Why am I acting as if this is historical...)
//

// I really need to keep up this commenting I am doing now, almost everything is commented fine in the first 30 minutes!!1!!
#ifndef THING_H
#define THING_H

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <iostream>

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int WIDTH = 1366, HEIGHT = 768, mouseX, mouseY, prevMouseX, prevMouseY; // Initial width, height, and mouse coords.
bool running, lmb=false, rmb=false, mmb=false, mouseLocked = false, menu, wireframe = true; // Mouse buttons, could allocate them to -1,-2,-3 for the keyStates but did this instead
float FPS, maxFPS, deltaTime = 0.0f, lastFrame = 0.0, moveSpeed = 0.1;

#include "shaders.h"

GLuint textureVBO, normalVBO, VBO, quadVBO, skyboxVAO, skyboxVBO;
GLuint VAO, quadVAO;

std::vector<GLuint> textureIDs;
std::vector<int> faceCount, objectCount;

Shader* mainShader = nullptr;

glm::vec3 position = glm::vec3(0.0f, 27.0f, 0.0f);

std::unordered_map<int, bool> keyStates; // All the keystates to be looked up with bool keyPressed


SDL_Window*   window = nullptr; // Declare the Window, Renderer, and OpenGL context globally
SDL_Renderer* renderer = nullptr;
SDL_GLContext GL = nullptr;

void quit() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void init() {
    keyStates.clear(); // Clear the keyStates

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){std::cout << "SDL could not initialize. Error: " << SDL_GetError() << "\n";exit(1);} // Initialize SDL

    window = SDL_CreateWindow("OpenGL + SDL2 test", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE); // Create window
    if(window == nullptr){std::cout << "SDL window could not be created. Error: " << SDL_GetError() << "\n";exit(1);}

    std::cout << "Created SDL2 window on tick: " << SDL_GetTicks() << std::endl;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC); // Create Renderer
    if(renderer == nullptr){std::cout << "SDL renderer could not be created. Error: " << SDL_GetError() << "\n";exit(1);}

    std::cout << "Created SDL2 renderer on tick: " << SDL_GetTicks() << std::endl;

    GL       = SDL_GL_CreateContext(window);
    if(GL == nullptr){std::cout << "SDL GL context could not be created. Error: " << SDL_GetError() << "\n";exit(1);} // Create GL context

    std::cout << "Created GL context on tick: " << SDL_GetTicks() << std::endl;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // OpenGL 4.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // SDL_GL_SetAttribute()
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER         , 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK , SDL_GL_CONTEXT_PROFILE_CORE); // Only the new features of GL
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
    SDL_RenderClear(renderer); // "Clear" the screen in the current draw color

    running = true;

    // Initialize openGL
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){std::cout << "GLAD could not initialize\n";exit(1);} else {std::cout << "Initialised GLAD on tick: " << SDL_GetTicks() << std::endl;}

    mainShader = new Shader("./shaders/vertexShader.glsl", "./shaders/fragmentShader.glsl");

    std::cout << "Compiled shaders on tick: " << SDL_GetTicks() << std::endl;

    glEnable(GL_DEPTH_TEST);
}

void redisplay() { // Updates the screen
    SDL_GL_SwapWindow(window);
}

bool keyPressed(SDL_Keycode key) {
    return keyStates[key];
}
void releaseKey (SDL_Keycode key) {
    keyStates[key] = false;
}

void toggleMouseLock() {
    mouseLocked = !mouseLocked;
    SDL_SetRelativeMouseMode(mouseLocked ? SDL_TRUE : SDL_FALSE);
    SDL_ShowCursor(mouseLocked);
}

void toggleFullscreen(SDL_Window* Window) {
    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
    bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
    SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
    SDL_ShowCursor(IsFullscreen);
}

void inputHandler() { // Adds all keys pressed to the array of KeyStates, to be used in the lookup function.
    SDL_Event e; // Declare events variable, for if anything related to input happens.
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {std::cout << "closing up and saving game...\n"; running = false;quit();}
        if (e.type == SDL_KEYDOWN) {
            keyStates[e.key.keysym.sym] = true;
        }
        if (e.type == SDL_KEYUP) {
            keyStates[e.key.keysym.sym] = false;
        }
        else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                WIDTH = e.window.data1;
                HEIGHT = e.window.data2;

                // Calculate aspect ratio
                float aspectRatio = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);

                // Adjust viewport and logical size
                glViewport(0, 0, WIDTH, HEIGHT);
                SDL_RenderSetLogicalSize(renderer, WIDTH, static_cast<int>(WIDTH / aspectRatio));

                // Update projection matrix
                // projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
            }
        } if (e.type == SDL_MOUSEBUTTONDOWN) {if (e.button.button == SDL_BUTTON_LEFT) {lmb = true; } else if (e.button.button == SDL_BUTTON_RIGHT) {rmb = true; } else if (e.button.button == SDL_BUTTON_MIDDLE) {mmb = true; }
        } if (e.type == SDL_MOUSEBUTTONUP  ) {if (e.button.button == SDL_BUTTON_LEFT) {lmb = false;} else if (e.button.button == SDL_BUTTON_RIGHT) {rmb = false;} else if (e.button.button == SDL_BUTTON_MIDDLE) {mmb = false;}
        }
        if (e.type == SDL_MOUSEMOTION) {
            SDL_GetMouseState(&mouseX, &mouseY);
        }
    }
}

void cleanUp() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    SDL_GL_DeleteContext(GL);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#endif
