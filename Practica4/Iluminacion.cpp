// Francisco Javier Reynoso Ortega
// Practica 8 Materiales e Iluminación
// 421056697
// 12/10/2025

// Std. Includes
#include <string>
#include <iostream>
#include <cmath> // sinf/cosf

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// Properties
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
bool keys[1024]{};
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

// Animación misc
glm::vec3 lightPos(0.5f, 0.5f, 2.5f); // aún lo dejo por si mueves con teclas
float movelightPos = 0.0f;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
float rot = 0.0f;
bool activanim = false;

// =================== Trayectorias lineales y alternancia ===================
// Luz día (E <-> O) y luz noche (N <-> S)
const glm::vec3 DAY_START = glm::vec3(-8.0f, 4.0f, 0.0f); // Oeste
const glm::vec3 DAY_END = glm::vec3(8.0f, 4.0f, 0.0f); // Este
const glm::vec3 NIGHT_START = glm::vec3(0.0f, 3.2f, -8.0f); // Norte (z-)
const glm::vec3 NIGHT_END = glm::vec3(0.0f, 3.2f, 8.0f); // Sur   (z+)

// Progreso 0..1 y dirección (+1 hacia END, -1 hacia START)
float sDay = 0.0f, sNight = 0.0f;
int   dirDay = +1, dirNight = +1;

// Alternancia: comienza moviéndose la luz de día
bool activeDay = true;
bool activeNight = false;

// Velocidad (fracción del camino por segundo)
const float PATH_SPEED = 0.20f;
// ==========================================================================

int main()
{
    if (!glfwInit()) { std::cout << "Failed to init GLFW\n"; return EXIT_FAILURE; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Francisco Javier Reynoso Ortega Previo 8", nullptr, nullptr);
    if (!window) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) { std::cout << "Failed to initialize GLEW\n"; return EXIT_FAILURE; }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader lampshader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");

    // Modelos
    Model red_dog((char*)"Models/RedDog.obj");
    Model ali((char*)"Models/Aligator_Quad.obj");
    Model toiletBrush((char*)"Models/Toilet_Brush.obj");
    Model bath((char*)"Models/Old_bath.obj");
    Model faucet((char*)"Models/Faucet.obj");
    Model water((char*)"Models/Water.obj");  // si no lo usas, comenta su Draw

    // Proyección
    glm::mat4 projection = glm::perspective(
        camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Cubo para lámparas (sol/luna)
    float vertices[] = {
        // pos                // normal
        -0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,
         0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,
        -0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,
        -0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,

        -0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,

         0.5f, 0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
         0.5f, 0.5f,-0.5f,   1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,   1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,   1.0f, 0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,   1.0f, 0.0f, 0.0f,

        -0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,
         0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,

        -0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Textura opcional
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    int textureWidth, textureHeight, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    image = stbi_load("Models/Texture_albedo.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
    if (image) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else { std::cout << "Failed to load texture\n"; }
    stbi_image_free(image);

    // ====== Conjunto BAÑO (padre + locales) ======
    float sSet = 0.04f;                              // escala global
    glm::vec3 setPos(-5.0f, -1.5f, -2.0f);          // posición del conjunto
    glm::vec3 faucetLocalOffset(0.0f, 1.0f, 0.2f);  // offset local
    float     faucetLocalScale = 0.4f;              // escala local
    glm::vec3 waterLocalOffset(0.0f, 0.05f, 0.0f);
    float     waterLocalScale = 1.0f;

    // Loop principal
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ======= POSICIONES DINÁMICAS DE LAS DOS LUCES (lineal + alternancia) =======
        auto clamp01 = [](float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };

        if (activeDay) {
            sDay += dirDay * PATH_SPEED * deltaTime;
            if (sDay >= 1.0f) { sDay = 1.0f; activeDay = false; activeNight = true;  dirDay = -1; }
            if (sDay <= 0.0f) { sDay = 0.0f; activeDay = false; activeNight = true;  dirDay = +1; }
        }
        if (activeNight) {
            sNight += dirNight * PATH_SPEED * deltaTime;
            if (sNight >= 1.0f) { sNight = 1.0f; activeNight = false; activeDay = true; dirNight = -1; }
            if (sNight <= 0.0f) { sNight = 0.0f; activeNight = false; activeDay = true; dirNight = +1; }
        }
        sDay = clamp01(sDay);
        sNight = clamp01(sNight);

        glm::vec3 dayPos = (1.0f - sDay) * DAY_START + sDay * DAY_END;    // Oeste <-> Este
        glm::vec3 nightPos = (1.0f - sNight) * NIGHT_START + sNight * NIGHT_END;  // Norte <-> Sur

        // ======= UNIFORMS DE LUZ/CÁMARA (2 luces) =======
        lightingShader.Use();

        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // LUZ DE DÍA (cálida)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].position"),
            dayPos.x, dayPos.y, dayPos.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].ambient"),
            0.35f, 0.32f, 0.25f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].diffuse"),
            1.00f, 0.95f, 0.85f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].specular"),
            1.00f, 1.00f, 1.00f);

        // LUZ DE NOCHE (más tenue y fría)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[1].position"),
            nightPos.x, nightPos.y, nightPos.z);

        // antes:
        // ambient 0.05,0.08,0.12 | diffuse 0.20,0.35,0.55 | specular 0.6,0.7,0.9
        // ahora (mucho más tenue):
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[1].ambient"),
            0.01f, 0.015f, 0.030f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[1].diffuse"),
            0.06, 0.10, 0.18);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[1].specular"),
            0.20f, 0.25f, 0.35f);


        // Matrices de cámara
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Material
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.8f, 0.8f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 0.8f);

        glBindVertexArray(VAO);

        // Perro
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(3.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            red_dog.Draw(lightingShader);
        }

        // Aligator
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(1.5f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(3.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            ali.Draw(lightingShader);
        }

        // Cepillo animado
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(0.8f, -1.5f, -1.0f));
            model = glm::scale(model, glm::vec3(0.3f));
            if (!activanim && rot <= 0.0f) rot += 0.1f;
            if (rot >= -90.0f) activanim = true;
            model = glm::rotate(model, glm::radians(rot), glm::vec3(1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            toiletBrush.Draw(lightingShader);
        }

        // ================= Conjunto BAÑO (padre + locales) =================
        glm::mat4 bathSet(1.0f);
        bathSet = glm::translate(bathSet, setPos);
        bathSet = glm::scale(bathSet, glm::vec3(sSet));

        // Tina
        {
            glm::mat4 model = bathSet;
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            bath.Draw(lightingShader);
        }
        // Grifo
        {
            glm::mat4 model = bathSet;
            model = glm::translate(model, faucetLocalOffset);
            model = glm::scale(model, glm::vec3(faucetLocalScale));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            faucet.Draw(lightingShader);
        }
        // Agua
        {
            glm::mat4 model = bathSet;
            model = glm::translate(model, waterLocalOffset);
            model = glm::scale(model, glm::vec3(waterLocalScale));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            water.Draw(lightingShader);
        }
        // ==================================================================

        glBindVertexArray(0);

        // ===== Visualizar sol y luna con cubitos =====
        lampshader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Sol (día)
        glm::mat4 lampModel(1.0f);
        lampModel = glm::translate(lampModel, dayPos);
        lampModel = glm::scale(lampModel, glm::vec3(0.25f));
        glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Luna (noche)
        glm::mat4 lampModel2(1.0f);
        lampModel2 = glm::translate(lampModel2, nightPos);
        lampModel2 = glm::scale(lampModel2, glm::vec3(0.20f));
        glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampModel2));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        // ==============================================

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);

    if (activanim) { if (rot > -90.0f) rot -= 0.1f; }
}

void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)  keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    if (keys[GLFW_KEY_O]) movelightPos += 0.1f;
    if (keys[GLFW_KEY_L]) movelightPos -= 0.1f;
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = (GLfloat)xPos; lastY = (GLfloat)yPos; firstMouse = false; }

    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;  // invertido
    lastX = (GLfloat)xPos; lastY = (GLfloat)yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}
