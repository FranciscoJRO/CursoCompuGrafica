// Francisco Javier Reynoso Ortega
// Funciones de fuentes de luz en OpenGL
// Practica 9
// 19/10/2025

#include <iostream>
#include <string>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// --- Prototipos ---
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// --- Ventana ---
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// --- Cámara ---
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024] = { false };
bool firstMouse = true;

// --- Luces ---
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f), glm::vec3(0.0f),
    glm::vec3(0.0f), glm::vec3(0.0f)
};

// -2 = ninguna (oscuro) | -1 = todas | 0..3 = una
int activeLight = -2;

// --- Vértices para cubos (si quisieras depurar) ---
float vertices[] = {
     -0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,0.5f,-0.5f, 0,0,-1,
      0.5f,0.5f,-0.5f, 0,0,-1, -0.5f,0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1,
     -0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f,0.5f, 0.5f, 0,0, 1,
      0.5f,0.5f, 0.5f, 0,0, 1, -0.5f,0.5f, 0.5f, 0,0, 1, -0.5f,-0.5f, 0.5f, 0,0, 1,
     -0.5f,0.5f, 0.5f,-1,0,0, -0.5f,0.5f,-0.5f,-1,0,0, -0.5f,-0.5f,-0.5f,-1,0,0,
     -0.5f,-0.5f,-0.5f,-1,0,0, -0.5f,-0.5f, 0.5f,-1,0,0, -0.5f,0.5f, 0.5f,-1,0,0,
      0.5f,0.5f, 0.5f, 1,0,0,  0.5f,0.5f,-0.5f, 1,0,0,  0.5f,-0.5f,-0.5f, 1,0,0,
      0.5f,-0.5f,-0.5f, 1,0,0,  0.5f,-0.5f, 0.5f, 1,0,0,  0.5f,0.5f, 0.5f, 1,0,0,
     -0.5f,-0.5f,-0.5f,0,-1,0,  0.5f,-0.5f,-0.5f,0,-1,0,  0.5f,-0.5f, 0.5f,0,-1,0,
      0.5f,-0.5f, 0.5f,0,-1,0, -0.5f,-0.5f, 0.5f,0,-1,0, -0.5f,-0.5f,-0.5f,0,-1,0,
     -0.5f,0.5f,-0.5f,0, 1,0,  0.5f,0.5f,-0.5f,0, 1,0,  0.5f,0.5f, 0.5f,0, 1,0,
      0.5f,0.5f, 0.5f,0, 1,0, -0.5f,0.5f, 0.5f,0, 1,0, -0.5f,0.5f,-0.5f,0, 1,0
};

// --- DeltaTime ---
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main() {
    // Init GLFW
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Francisco Javier Reynoso Ortega Practica 9", nullptr, nullptr);
    if (!window) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) { std::cout << "Failed to initialize GLEW\n"; return EXIT_FAILURE; }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB); // gamma-correct

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag"); // no lo usamos, pero déjalo si luego quieres depurar

    Model Dog((char*)"Models/redDog.obj");
    Model Piso((char*)"Models/piso.obj");
    Model Ball((char*)"Models/ball.obj");
    Model Fogata((char*)"Models/Campfire.obj");
    Model Lamp((char*)"Models/magic_lamp_Without_ring.obj");
    Model Hanging_Lamp((char*)"Models/Hanging_lamp.obj");
    Model OilLamp((char*)"Models/OilLamp.obj");

    // (cubos de debug, desactivados)
    bool drawLightCubes = false;
    GLuint VBO = 0, VAO = 0;
    if (drawLightCubes) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.emissive"), 2);
    glUniform1f(glGetUniformLocation(lightingShader.Program, "emissiveStrength"), 1.0f);

    glm::mat4 projection = glm::perspective(camera.GetZoom(),
        (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

    // --- Posiciones mundo ---
    glm::vec3 fogataPos = glm::vec3(2.0f, -0.3f, -1.0f);
    glm::vec3 magicLampPos = glm::vec3(-2.0f, -0.4f, -1.0f);
    glm::vec3 hangingLampPos = glm::vec3(1.5f, 0.9f, 1.0f);
    glm::vec3 oilLampPos = glm::vec3(-1.5f, -0.4f, 1.5f);

    // --- Offsets internos (luz dentro del modelo) ---
    glm::vec3 fogataOffset = glm::vec3(0.0f, 0.60f, 0.0f);
    glm::vec3 magicLampOffset = glm::vec3(0.0f, 0.12f, 0.0f);
    glm::vec3 hangingLampOffset = glm::vec3(0.0f, -0.05f, 0.0f);
    glm::vec3 oilLampOffset = glm::vec3(0.0f, 0.35f, 0.0f);

    // --- Helpers de atenuación por radio ---
    auto coeffsFromRange = [](float range, float& linear, float& quadratic) {
        linear = 4.5f / range;
        quadratic = 75.0f / (range * range);
        };

    auto setPointLight = [&](int i, const glm::vec3& pos, const glm::vec3& color, float range, float enabled)
        {
            float lin, quad; coeffsFromRange(range, lin, quad);
            std::string base = "pointLights[" + std::to_string(i) + "].";
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "position").c_str()), pos.x, pos.y, pos.z);

            glm::vec3 amb = color * 0.05f * enabled;   // bajito para no quemar
            glm::vec3 dif = color * 1.00f * enabled;
            glm::vec3 spe = color * 0.80f * enabled;

            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "ambient").c_str()), amb.x, amb.y, amb.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "diffuse").c_str()), dif.x, dif.y, dif.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "specular").c_str()), spe.x, spe.y, spe.z);

            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "linear").c_str()), lin);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "quadratic").c_str()), quad);
        };

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.Use();

        // Cámara
        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // --- Direccional MUY tenue; solo sube un poco cuando usas '0' (todas) ---
        bool allOn = (activeLight == -1);
        glm::vec3 dirAmb = allOn ? glm::vec3(0.06f) : glm::vec3(0.02f);
        glm::vec3 dirDiff = allOn ? glm::vec3(0.12f) : glm::vec3(0.02f);
        glm::vec3 dirSpec = glm::vec3(0.0f);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), dirAmb.x, dirAmb.y, dirAmb.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), dirDiff.x, dirDiff.y, dirDiff.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), dirSpec.x, dirSpec.y, dirSpec.z);

        // --- Luces puntuales dentro de cada modelo ---
        float t = glfwGetTime();
        auto on = [&](int i) { return (activeLight == -1 || activeLight == i) ? 1.0f : 0.0f; };

        pointLightPositions[0] = fogataPos + fogataOffset;
        pointLightPositions[1] = magicLampPos + magicLampOffset;
        pointLightPositions[2] = hangingLampPos + hangingLampOffset;
        pointLightPositions[3] = oilLampPos + oilLampOffset;

        glm::vec3 cFogata = glm::vec3(1.00f, 0.55f, 0.18f);
        glm::vec3 cMagic = glm::vec3(1.00f, 0.88f, 0.60f);
        glm::vec3 cHanging = glm::vec3(0.85f, 0.92f, 1.00f);
        glm::vec3 cOil = glm::vec3(1.00f, 0.80f, 0.55f);

        float rFogata = 8.0f;
        float rMagic = 6.0f;
        float rHanging = 11.0f;
        float rOil = 5.0f;

        float flick = 0.75f + 0.25f * (0.5f * std::sin(2.7f * t) + 0.5f * std::sin(3.5f * t + 1.7f));
        setPointLight(0, pointLightPositions[0], cFogata * flick, rFogata, on(0));
        setPointLight(1, pointLightPositions[1], cMagic, rMagic, on(1));
        setPointLight(2, pointLightPositions[2], cHanging, rHanging, on(2));
        setPointLight(3, pointLightPositions[3], cOil, rOil, on(3));

        // Spotlight cámara (tenue, casi apagada)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.02f, 0.02f, 0.05f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.05f, 0.05f, 0.20f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

        // Material
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

        // Matrices
        glm::mat4 view = camera.GetViewMatrix();
        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        auto setEmissive = [&](int enabled) {
            glUniform1i(glGetUniformLocation(lightingShader.Program, "useEmissive"), enabled);
            };

        glm::mat4 model(1.0f);

        // Piso (opaco, sin emisivo)
        setEmissive(0);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Piso.Draw(lightingShader);

        // Perro (opaco, sin emisivo)
        setEmissive(0);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Dog.Draw(lightingShader);

        // Magic Lamp
        setEmissive(on(1)); // solo brilla cuando es la activa (o todas)
        model = glm::mat4(1.0f);
        model = glm::translate(model, magicLampPos);
        model = glm::scale(model, glm::vec3(0.005f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Lamp.Draw(lightingShader);

        // Fogata
        setEmissive(on(0));
        model = glm::mat4(1.0f);
        model = glm::translate(model, fogataPos);
        model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Fogata.Draw(lightingShader);

        // Hanging Lamp
        setEmissive(on(2));
        model = glm::mat4(1.0f);
        model = glm::translate(model, hangingLampPos);
        model = glm::scale(model, glm::vec3(0.004f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Hanging_Lamp.Draw(lightingShader);

        // Oil Lamp
        setEmissive(on(3));
        model = glm::mat4(1.0f);
        model = glm::translate(model, oilLampPos);
        model = glm::scale(model, glm::vec3(0.02f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        OilLamp.Draw(lightingShader);

        // (cubos de debug: desactivados)
        if (drawLightCubes) {
            glBindVertexArray(VAO);
            modelLoc = glGetUniformLocation(lampShader.Program, "model");
            viewLoc = glGetUniformLocation(lampShader.Program, "view");
            projLoc = glGetUniformLocation(lampShader.Program, "projection");
            lampShader.Use();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            for (int i = 0; i < 4; ++i) {
                if (!(activeLight == -1 || activeLight == i)) continue;
                model = glm::mat4(1.0f);
                model = glm::translate(model, pointLightPositions[i]);
                model = glm::scale(model, glm::vec3(0.12f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// --- Movimiento cámara ---
void DoMovement() {
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])      camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])      camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])     camera.ProcessKeyboard(RIGHT, deltaTime);
}

// --- Teclado ---
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_0) activeLight = -1; // todas
        if (key == GLFW_KEY_1) activeLight = 0; // solo Fogata
        if (key == GLFW_KEY_2) activeLight = 1; // solo Magic
        if (key == GLFW_KEY_3) activeLight = 2; // solo Hanging
        if (key == GLFW_KEY_4) activeLight = 3; // solo Oil
        if (key == GLFW_KEY_5) activeLight = -2; // ninguna (oscuro)
    }
}

// --- Mouse ---
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = (GLfloat)xPos; lastY = (GLfloat)yPos; firstMouse = false; }
    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;
    lastX = (GLfloat)xPos; lastY = (GLfloat)yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}
