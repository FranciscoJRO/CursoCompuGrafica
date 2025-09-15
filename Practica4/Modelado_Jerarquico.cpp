// Francisco Javier Reynoso Ortega
// Practica 5
// 14/09/2025
// 421056697

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ===== Shaders embebidos =====
static const char* kVertexSrc = R"(#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";

static const char* kFragmentSrc = R"(#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main()
{
    FragColor = vec4(color, 1.0);
}
)";

// Compilación y linkeo de shaders embebidos
static GLuint CompileShader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << log << std::endl;
    }
    return sh;
}
static GLuint CreateProgramFromEmbedded(const char* vs, const char* fs) {
    GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    GLint ok = GL_FALSE;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, 1024, nullptr, log);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << log << std::endl;
    }
    return p;
}

void Inputs(GLFWwindow* window);

const GLint WIDTH = 1200, HEIGHT = 800;

// For Keyboard
float movX = 0.0f, movY = 0.0f, movZ = -5.0f, rot = 0.0f;

// For model (articulaciones y dedos)
float hombro = 0.0f,
codo = 0.0f,
muneca = 0.0f,
dedo1_1 = 0.0f, dedo1_2 = 0.0f, dedo1_3 = 0.0f, dedo1_4 = 0.0f, dedo1_5 = 0.0f,
dedo2_1 = 0.0f, dedo2_2 = 0.0f, dedo2_3 = 0.0f, dedo2_4 = 0.0f, dedo2_5 = 0.0f; // <- ; importante

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Practica 5 Francisco Javier Reynoso Ortega", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cerr << "Failed to initialise GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    // Info GPU
    std::cout << "> Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "> Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "> Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "> SL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Programa desde shaders embebidos
    GLuint program = CreateProgramFromEmbedded(kVertexSrc, kFragmentSrc);
    glUseProgram(program);

    // Vértices de un cubo (36 vértices -> 12 triángulos)
    float vertices[] = {
        // Frente
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
         // Atrás
         -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
          0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
          // Derecha
           0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
           0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
           // Izquierda
           -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
           -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
           // Abajo
           -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
            0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
            // Arriba
            -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
             0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f,
    };

    GLuint VBO = 0, VAO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Posición (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Uniform locations
    GLint uModelLoc = glGetUniformLocation(program, "model");
    GLint uViewLoc = glGetUniformLocation(program, "view");
    GLint uProjLoc = glGetUniformLocation(program, "projection");
    GLint uColorLoc = glGetUniformLocation(program, "color");

    // Proyección
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (GLfloat)screenWidth / (GLfloat)screenHeight,
        0.1f, 100.0f);
    glUseProgram(program);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    while (!glfwWindowShouldClose(window)) {
        Inputs(window);
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        // View
        glm::mat4 view(1.0f);
        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Model base y temporales
        glm::mat4 model(1.0f);
        glm::mat4 modelTemp(1.0f);
        glm::mat4 modelTemp2(1.0f);

        glBindVertexArray(VAO);

        // ===== Bíceps =====
        model = glm::rotate(model, glm::radians(hombro), glm::vec3(0.0f, 0.0f, 1.0f)); // hombro
        modelTemp = model = glm::translate(model, glm::vec3(1.5f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 1.0f, 1.0f));
        glm::vec3 color = glm::vec3(1.0f, 0.73f, 0.75f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ===== Antebrazo =====
        model = glm::translate(modelTemp, glm::vec3(1.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(codo), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTemp = model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ===== Palma =====
        model = glm::translate(modelTemp, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(muneca), glm::vec3(1.0f, 0.0f, 0.0f));
        modelTemp2 = modelTemp = model = glm::translate(model, glm::vec3(0.25f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 1.0f, 1.0f));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ----- Dedos -----
        const glm::vec3 ESC_FALANGE = glm::vec3(0.85f, 0.24f, 0.14f);
        const float HALF_X = 0.425f;

        // PULGAR
        modelTemp = modelTemp2;
        // Falange 1
        model = glm::translate(modelTemp, glm::vec3(0.10f, 0.12f, 0.30f));
        model = glm::rotate(model, glm::radians(-40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(dedo1_1), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Falange 2
        model = glm::translate(modelTemp, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(dedo2_1), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ÍNDICE
        modelTemp = modelTemp2;
        // Falange 1
        model = glm::translate(modelTemp, glm::vec3(0.25f, 0.35f, 0.32f));
        model = glm::rotate(model, glm::radians(dedo1_2), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Falange 2
        model = glm::translate(modelTemp, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(dedo2_2), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // MEDIO
        modelTemp = modelTemp2;
        // Falange 1
        model = glm::translate(modelTemp, glm::vec3(0.25f, 0.35f, 0.12f));
        model = glm::rotate(model, glm::radians(dedo1_3), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Falange 2
        model = glm::translate(modelTemp, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(dedo2_3), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ANULAR
        modelTemp = modelTemp2;
        // Falange 1
        model = glm::translate(modelTemp, glm::vec3(0.25f, 0.35f, -0.12f));
        model = glm::rotate(model, glm::radians(dedo1_4), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Falange 2
        model = glm::translate(modelTemp, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(dedo2_4), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // MEÑIQUE
        modelTemp = modelTemp2;
        // Falange 1
        model = glm::translate(modelTemp, glm::vec3(0.25f, 0.35f, -0.32f));
        model = glm::rotate(model, glm::radians(dedo1_5), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Falange 2
        model = glm::translate(modelTemp, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(dedo2_5), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(HALF_X, 0.0f, 0.0f));
        model = glm::scale(model, ESC_FALANGE);
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

// Entradas de teclado
void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movX += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movX -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) movY += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) movY -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movZ -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movZ += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rot += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  rot -= 0.18f;

    // Hombro
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) hombro += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) hombro -= 0.18f;
    if (hombro > 90.0f)  hombro = 90.0f;
    if (hombro < -90.0f) hombro = -90.0f;

    // Codo
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) codo += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) codo -= 0.18f;
    if (codo > 0.0f)  codo = 0.0f;
    if (codo < -90.0f) codo = -90.0f;

    // Muñeca
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) muneca += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) muneca -= 0.18f;
    if (muneca > 90.0f)  muneca = 90.0f;
    if (muneca < -90.0f) muneca = -90.0f;

    // Primeras falanges (J/U)
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        dedo1_1 += 0.18f; dedo1_2 += 0.18f; dedo1_3 += 0.18f; dedo1_4 += 0.18f; dedo1_5 += 0.18f;
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        dedo1_1 -= 0.18f; dedo1_2 -= 0.18f; dedo1_3 -= 0.18f; dedo1_4 -= 0.18f; dedo1_5 -= 0.18f;
    }
    if (dedo1_1 > 6.0f)  dedo1_1 = 6.0f;   if (dedo1_1 < -12.0f) dedo1_1 = -12.0f;
    if (dedo1_2 > 6.0f)  dedo1_2 = 6.0f;   if (dedo1_2 < -12.0f) dedo1_2 = -12.0f;
    if (dedo1_3 > 6.0f)  dedo1_3 = 6.0f;   if (dedo1_3 < -12.0f) dedo1_3 = -12.0f;
    if (dedo1_4 > 6.0f)  dedo1_4 = 6.0f;   if (dedo1_4 < -12.0f) dedo1_4 = -12.0f;
    if (dedo1_5 > 6.0f)  dedo1_5 = 6.0f;   if (dedo1_5 < -12.0f) dedo1_5 = -12.0f;

    // Segundas falanges (K/I)
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        dedo2_1 += 0.18f; dedo2_2 += 0.18f; dedo2_3 += 0.18f; dedo2_4 += 0.18f; dedo2_5 += 0.18f;
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        dedo2_1 -= 0.18f; dedo2_2 -= 0.18f; dedo2_3 -= 0.18f; dedo2_4 -= 0.18f; dedo2_5 -= 0.18f;
    }
    if (dedo2_1 > 0.0f)  dedo2_1 = 0.0f;   if (dedo2_1 < -105.0f) dedo2_1 = -105.0f;
    if (dedo2_2 > 0.0f)  dedo2_2 = 0.0f;   if (dedo2_2 < -105.0f) dedo2_2 = -105.0f;
    if (dedo2_3 > 0.0f)  dedo2_3 = 0.0f;   if (dedo2_3 < -105.0f) dedo2_3 = -105.0f;
    if (dedo2_4 > 0.0f)  dedo2_4 = 0.0f;   if (dedo2_4 < -105.0f) dedo2_4 = -105.0f;
    if (dedo2_5 > 0.0f)  dedo2_5 = 0.0f;   if (dedo2_5 < -105.0f) dedo2_5 = -105.0f;
}
