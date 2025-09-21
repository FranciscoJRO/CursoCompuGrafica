// Francisco Javier Reynoso Ortega
// 421056697
// Previo 6 Carga de Modelos
// 16/09/2025

#include <iostream>
#include <string>
#include <cstring>

// GLEW
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// Props
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Protos
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool   keys[1024];
double lastX = 400.0, lastY = 300.0;
bool   firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
    if (!glfwInit()) return EXIT_FAILURE;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Previo 6 Francisco Javier Reynoso Ortega", nullptr, nullptr);
    if (!window) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cout << "Failed to initialize GLEW\n"; return EXIT_FAILURE; }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    std::memset(keys, 0, sizeof(keys));

    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // ===== Carga de modelos =====
    Model dog((char*)"Models/RedDog.obj");


    // Nuevo: Alligator
    Model gator((char*)"Models/Aligator_Quad.obj");

    //Modelo toilet brush
	Model toiletBrush((char*)"Models/Toilet_Brush.obj");


    // Nuevos
    Model bath((char*)"Models/Old_bath.obj");
    Model faucet((char*)"Models/Faucet.obj");
    Model water((char*)"Models/Water.obj");  // malla de agua


    // Proyección (una sola vez)
    glm::mat4 projection = glm::perspective(
        camera.GetZoom(),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 100.0f
    );

    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // ===== matrices comunes =====
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // ===== localiza uniforms una vez por frame =====
        const GLint locModel = glGetUniformLocation(shader.Program, "model");
        const GLint locIsTransparent = glGetUniformLocation(shader.Program, "uIsTransparent");
        const GLint locAlphaFallback = glGetUniformLocation(shader.Program, "uAlphaFallback");

        // ------------------------------------------------------------------
        // 1) OPAQUE FIRST (perro, caimán, tina, llave)
        // ------------------------------------------------------------------
        glUniform1i(locIsTransparent, GL_FALSE);
        glUniform1f(locAlphaFallback, 1.0f);

        glm::mat4 model(1.0f);

        // Dog
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        dog.Draw(shader);

        // Gator (a la izquierda del perro si quieres)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.8f, -0.1f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        gator.Draw(shader);

        // Old Bath (escalada pequeña)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, -0.20f, 0.0f));
        model = glm::scale(model, glm::vec3(0.005f));   // ajusta a gusto (0.05–0.20)
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        bath.Draw(shader);

        // Faucet (llave) cerca de la tina
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.20f, 0.00f, 0.05f));
        model = glm::scale(model, glm::vec3(0.005f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        faucet.Draw(shader);

        // ===== Toilet Brush (a la derecha del perrito) =====
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.2f, -0.8f, 0.0f)); // x positivo = derecha
        model = glm::scale(model, glm::vec3(0.5f));                   // un poco más pequeño
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        toiletBrush.Draw(shader);

        // ------------------------------------------------------------------
// 2) TRANSPARENT LAST (agua)
// ------------------------------------------------------------------
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // No escribir en el z-buffer (pero sí leerlo) para mezclar bien
        glDepthMask(GL_FALSE);

        // Uniforms para agua
        glUniform1i(locIsTransparent, GL_TRUE);
        glUniform1f(locAlphaFallback, 1.0f);             // << opacidad que verás (0.25–0.5 va bien)

        // Tinte opcional
        GLint locUseTint = glGetUniformLocation(shader.Program, "uUseTint");
        GLint locTint = glGetUniformLocation(shader.Program, "uTint");
        glUniform1i(locUseTint, GL_TRUE);                 // pon GL_FALSE si no quieres tinte
        glUniform3f(locTint, 0.25f, 0.45f, 0.55f);        // azul verdoso suave

        // (Opcional) evitar problemas por caras traseras del plano del agua
        // glDisable(GL_CULL_FACE);

        model = glm::mat4(1.0f);                  // ✅ solo reasignas
        model = glm::translate(model, glm::vec3(2.0f, -0.18f, 0.0f));
        model = glm::scale(model, glm::vec3(0.005f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        water.Draw(shader);
        // Restaurar estado
        // glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // Deja listos los uniforms para lo opaco
        glUniform1i(locIsTransparent, GL_FALSE);
        glUniform1f(locAlphaFallback, 1.0f);
        glUniform1i(locUseTint, GL_FALSE);


        glfwSwapBuffers(window);
    }


    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)      keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }

    float xOffset = static_cast<float>(xPos - lastX);
    float yOffset = static_cast<float>(lastY - yPos);

    lastX = xPos; lastY = yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}
