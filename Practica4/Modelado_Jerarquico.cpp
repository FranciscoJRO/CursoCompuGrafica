// Francisco Javier Reynoso Ortega 
// 421056697
// Previo 5
// 08/09/2025
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ===== Shaders embebidos =====
static const char* kVertexSrc = R"(#version 330 core
layout (location = 0) in vec3 position;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;
uniform vec3 color;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    ourColor = color;
}
)";

static const char* kFragmentSrc = R"(#version 330 core
in vec3 ourColor;
out vec4 color_out;

void main()
{
    color_out = vec4(ourColor, 1.0);
}
)";

// ===== Implementación mínima de Shader =====
struct Shader {
    GLuint Program = 0;
    GLint uniformColor = -1;

    static GLuint compile(GLenum type, const char* src) {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        GLint ok = GL_FALSE;
        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0; glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(sh, len, nullptr, log.data());
            std::cerr << "[Shader] Error compilando "
                << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
                << ":\n" << log << "\n";
            glDeleteShader(sh);
            return 0;
        }
        return sh;
    }

    Shader(const char* vsSrc, const char* fsSrc) {
        GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
        GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);
        if (!vs || !fs) return;

        Program = glCreateProgram();
        glAttachShader(Program, vs);
        glAttachShader(Program, fs);
        glLinkProgram(Program);

        GLint ok = GL_FALSE;
        glGetProgramiv(Program, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0; glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(Program, len, nullptr, log.data());
            std::cerr << "[Shader] Error enlazando programa:\n" << log << "\n";
            glDeleteProgram(Program);
            Program = 0;
        }

        glDeleteShader(vs);
        glDeleteShader(fs);

        if (Program) {
            uniformColor = glGetUniformLocation(Program, "color");
        }
    }

    void Use() const { glUseProgram(Program); }
};

void Inputs(GLFWwindow* window);

const GLint WIDTH = 1200, HEIGHT = 800;

// Controles cámara
float movX = 0.0f, movY = 0.0f, movZ = -5.0f, rot = 0.0f;

// Articulaciones
float hombro = 0.0f;
float antebrazo = 0.0f; // (se mantiene)
float codo = 0.0f;
float muneca = 0.0f;
float dedo1 = 0.0f;
float dedo2 = 0.0f;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //  glfwWindowHint(GLFW_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Francisco Javier Reynoso Ortega Previo 5", nullptr, nullptr);

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialise GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader ourShader(kVertexSrc, kFragmentSrc);

    // Cubo
    float vertices[] = {
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,

        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
         0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,

         0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
         0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,

        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
         0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,

        -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (GLfloat)screenWidth / (GLfloat)screenHeight,
        0.1f, 100.0f);
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        Inputs(window);
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.Use();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 modelTemp = glm::mat4(1.0f);
        glm::mat4 modelTemp2 = glm::mat4(1.0f);

        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projecLoc = glGetUniformLocation(ourShader.Program, "projection");
        GLint transformLoc = glGetUniformLocation(ourShader.Program, "transform");
        GLint uniformColor = ourShader.uniformColor;

        glm::mat4 transform = glm::mat4(1.0f);
        glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        glBindVertexArray(VAO);

        // ===== Brazo (VERDE) =====
        model = glm::rotate(model, glm::radians(hombro), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(1.5f, 0.0f, 0.0f)); // guardar base antes del scale
        model = glm::scale(model, glm::vec3(3.0f, 1.0f, 1.0f));
        color = glm::vec3(0.0f, 1.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ===== Base del codo =====
        glm::mat4 baseCodo = modelTemp;
        baseCodo = glm::translate(baseCodo, glm::vec3(1.5f, 0.0f, 0.0f));
        baseCodo = glm::rotate(baseCodo, glm::radians(codo), glm::vec3(0.0f, 0.0f, 1.0f));

        // ===== Antebrazo (ROJO, largo 2.0) =====
        model = baseCodo;
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ===== Muñeca (BLANCA, largo 0.5) =====
        model = baseCodo;
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f)); // fin del antebrazo
        model = glm::rotate(model, glm::radians(muneca), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp2 = model; // por si agregas mano
        model = glm::translate(model, glm::vec3(0.25f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 1.0f, 1.0f));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ===== Base de la muñeca para dedos =====
        glm::mat4 baseMuneca = modelTemp;
        baseMuneca = glm::translate(baseMuneca, glm::vec3(1.5f, 0.0f, 0.0f)); // codo
        baseMuneca = glm::rotate(baseMuneca, glm::radians(codo), glm::vec3(0.0f, 0.0f, 1.0f));
        baseMuneca = glm::translate(baseMuneca, glm::vec3(2.0f, 0.0f, 0.0f)); // fin antebrazo
        baseMuneca = glm::rotate(baseMuneca, glm::radians(muneca), glm::vec3(0.0f, 0.0f, 1.0f));

        // ===== Dedo1 (CIAN, falange A) =====
        model = glm::translate(baseMuneca, glm::vec3(0.25f, 0.35f, 0.375f)); // posición en la "palma"
        model = glm::rotate(model, glm::radians(dedo1), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f)); // centro de falange A (largo 1.0)
        model = glm::scale(model, glm::vec3(1.0f, 0.3f, 0.25f));
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // C

        // ===== Dedo1 B (MAGENTA, falange B) =====
        // Usa 'modelTemp' que quedó en el CENTRO de la falange A
        model = glm::translate(modelTemp, glm::vec3(0.5f, 0.0f, 0.0f));          // ir al extremo distal de A
        model = glm::rotate(model, glm::radians(dedo2), glm::vec3(0.0f, 0.0f, 1.0f)); // rotación del nudillo interfalángico
        model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));              // centrar falange B (largo 1.0)
        model = glm::scale(model, glm::vec3(1.0f, 0.3f, 0.25f));                 // mismas dimensiones
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // D

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (ourShader.Program) glDeleteProgram(ourShader.Program);

    glfwTerminate();
    return EXIT_SUCCESS;
}

void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movX += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movX -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) movY += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) movY -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movZ -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movZ += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rot += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rot -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) hombro += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) hombro -= 0.18f;

    // Articulaciones
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) codo += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) codo -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) muneca += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) muneca -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) dedo1 += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) dedo1 -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) dedo2 += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) dedo2 -= 0.18f;
}
