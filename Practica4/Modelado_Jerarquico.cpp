#include <iostream>
#include <vector>
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
uniform mat4 transform; // no se usa, se deja por compatibilidad
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

void Inputs(GLFWwindow* window);

// ===== Helpers para compilar y linkear shaders embebidos =====
static GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<GLchar> log(std::max(1, logLen));
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::cerr << "[Shader Compile Error] "
            << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
            << " shader:\n" << log.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint CreateProgramFromEmbedded() {
    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVertexSrc);
    if (!vs) return 0;
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFragmentSrc);
    if (!fs) { glDeleteShader(vs); return 0; }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<GLchar> log(std::max(1, logLen));
        glGetProgramInfoLog(program, logLen, nullptr, log.data());
        std::cerr << "[Program Link Error]\n" << log.data() << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

const GLint WIDTH = 1200, HEIGHT = 800;

// For Keyboard
float movX = 0.0f, movY = 0.0f, movZ = -5.0f, rot = 0.0f;

// For model
float hombro = 0.0f, codo = 0.0f, muneca = 0.0f, dedo1 = 0.0f, dedo2 = 0.0f;

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return EXIT_FAILURE;
    }

    // Verificación de compatibilidad 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Previo 5 Juan Soria", nullptr, nullptr);
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
    glViewport(0, 0, screenWidth, screenHeight);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // === Crear y usar el programa desde shaders embebidos ===
    GLuint program = CreateProgramFromEmbedded();
    if (!program) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glUseProgram(program);

    // Set up vertex data (cube) y buffers
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

    // Posición (location = 0 en el VS)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Matrices de cámara/proyección
    glm::mat4 projection(1.0f);
    projection = glm::perspective(glm::radians(45.0f),
        (GLfloat)screenWidth / (GLfloat)screenHeight,
        0.1f, 100.0f);

    glm::vec3 color(0.0f, 0.0f, 1.0f);

    // Ubicar uniform locations UNA sola vez
    GLint uModelLoc = glGetUniformLocation(program, "model");
    GLint uViewLoc = glGetUniformLocation(program, "view");
    GLint uProjLoc = glGetUniformLocation(program, "projection");
    GLint uColorLoc = glGetUniformLocation(program, "color");

    // Set estático de proyección
    glUseProgram(program);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    while (!glfwWindowShouldClose(window)) {
        Inputs(window);
        glfwPollEvents();

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        // View
        glm::mat4 view(1.0f);
        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Model
        glm::mat4 model(1.0f);
        glm::mat4 modelTemp(1.0f);
        glm::mat4 modelTemp2(1.0f);

        glBindVertexArray(VAO);

        // ===== Bíceps =====
        model = glm::rotate(model, glm::radians(hombro), glm::vec3(0.0f, 0.0f, 1.0f)); // hombro
        modelTemp = model = glm::translate(model, glm::vec3(1.5f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 1.0f, 1.0f));
        color = glm::vec3(0.0f, 1.0f, 0.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // A

        // ===== Antebrazo =====
        model = glm::translate(modelTemp, glm::vec3(1.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(codo), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTemp = model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
        color = glm::vec3(1.0f, 0.0f, 0.0f); // rojo (antebrazo)
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // B

        // ===== Palma =====
        model = glm::translate(modelTemp, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(muneca), glm::vec3(1.0f, 0.0f, 0.0f));
        modelTemp2 = modelTemp = model = glm::translate(model, glm::vec3(0.25f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 1.0f, 1.0f));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // C

        // ===== Dedo 1 A =====
        model = glm::translate(modelTemp, glm::vec3(0.25f, 0.35f, 0.375f));
        model = glm::rotate(model, glm::radians(dedo1), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTemp = model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.3f, 0.25f));
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // D

        // ===== Dedo 1 B =====
        model = glm::translate(modelTemp, glm::vec3(0.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(dedo2), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.3f, 0.25f));
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uColorLoc, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // E

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movX += 0.008f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movX -= 0.008f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) movY += 0.008f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) movY -= 0.008f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movZ -= 0.008f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movZ += 0.008f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rot += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rot -= 0.18f;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) hombro += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) hombro -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) codo += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) codo -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) muneca += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) muneca -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) dedo1 += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) dedo1 -= 0.18f;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) dedo2 += 0.18f;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) dedo2 -= 0.18f;
}
