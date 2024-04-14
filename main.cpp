#include "../includes/thing.h" // my horrible "simplification", i just pump every possible function into here. I N C L U D E   T H I S   F I R S T ! ! !

// lazy definitions
float seed = 42405.4;

void vertexSpec() { // stuff happens here
    GLfloat quadVertices[] = {
        // Positions    // Texture coordinates
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
        1.0f, -1.0f,   1.0f, 0.0f,

        -1.0f,  1.0f,   0.0f, 1.0f,
        1.0f, -1.0f,   1.0f, 0.0f,
        1.0f,  1.0f,   1.0f, 1.0f
    };

// quad for post processing
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Unbind VAO
    glBindVertexArray(0);

    // bind stuff to vertex array index 0
    glBindVertexArray(0);
    // clean up
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void input() {
    if(keyPressed(SDLK_EQUALS)) {
        releaseKey(SDLK_EQUALS);
        moveSpeed *= 2;
    }
    if(keyPressed(SDLK_MINUS)) {
        releaseKey(SDLK_MINUS);
        moveSpeed /= 2;
        if(moveSpeed < 0.001) moveSpeed = 0.001;
    }
    if(keyPressed(SDLK_w)) {
        position.z -= moveSpeed;
    }
    if(keyPressed(SDLK_s)) {
        position.z += moveSpeed;
    }
    if(keyPressed(SDLK_a)) {
        position.x -= moveSpeed;
    }
    if(keyPressed(SDLK_d)) {
        position.x += moveSpeed;
    }
    if(keyPressed(SDLK_q)) {
        position.y -= moveSpeed;
    }
    if(keyPressed(SDLK_z)) {
        seed -= 0.01;
    }
    if(keyPressed(SDLK_x)) {
        seed += 0.01;
    }
    if(keyPressed(SDLK_e)) {
        position.y += moveSpeed;
    }
    if(keyPressed(SDLK_F5)) {
        mainShader = new Shader("./shaders/vertexShader.glsl", "./shaders/fragmentShader.glsl");
        releaseKey(SDLK_F5);
    }
    if(keyPressed(SDLK_o)) {
        std::cout << "Stats:\n";
        std::cout << "Seed: \"" << std::to_string(seed) << "\".\n";
        std::cout << "Position:\n";
        std::cout << std::to_string(position.x) << " x,\n";
        std::cout << std::to_string(position.y) << " y,\n";
        std::cout << std::to_string(position.z) << " z.\n";
    }
}

void drawQuad() {
    mainShader->use();
    mainShader->setVec2("screen", glm::vec2(WIDTH, HEIGHT));
    mainShader->setVec3("camPos", position);
    mainShader->setFloat("seed", seed);

    glBindVertexArray(quadVAO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void frame() {
    glViewport(0, 0, WIDTH, HEIGHT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawQuad();
}

void mainLoop() {
    glEnable(GL_CULL_FACE);
    glClearColor(0.7f, 0.0f, 0.0f, 1.0f);
    while (running) {
        inputHandler();
        input();

        frame();

        redisplay();
    }
}

int main() { // yeah...
    init();

    vertexSpec();

    mainLoop();

    cleanUp();

    return 0;
}
