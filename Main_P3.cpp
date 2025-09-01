// Reynoso Ortega Francisco Javier
// Práctica 3
// 421056697

#include <iostream>

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const GLint WIDTH = 800, HEIGHT = 600;

// ================= Shaders embebidos =================
const char* kVertexSrc = R"(#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vColor;

void main(){
    vColor = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* kFragmentSrc = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main(){
    FragColor = vec4(vColor, 1.0);
}
)";

// Compilar y enlazar programa
static GLuint buildProgram(const char* vsSrc, const char* fsSrc) {
    GLint ok; GLchar log[1024];

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) { glGetShaderInfoLog(vs, 1024, nullptr, log); std::cerr << "[VS ERROR]\n" << log << std::endl; }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) { glGetShaderInfoLog(fs, 1024, nullptr, log); std::cerr << "[FS ERROR]\n" << log << std::endl; }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { glGetProgramInfoLog(prog, 1024, nullptr, log); std::cerr << "[LINK ERROR]\n" << log << std::endl; }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

int main() {
    // ===== GLFW/GLEW =====
    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return EXIT_FAILURE; }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
        "Practica 3 - Reynoso Ortega Francisco Javier", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) { std::cerr << "Failed to init GLEW\n"; return EXIT_FAILURE; }

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ===== Shaders =====
    GLuint program = buildProgram(kVertexSrc, kFragmentSrc);

    // ===== Vértices de un cubo con color por cara =====
    float vertices[] = {
        // Front (rojo)
        -0.5f,-0.5f, 0.5f, 1.0f,0.0f,0.0f,
         0.5f,-0.5f, 0.5f, 1.0f,0.0f,0.0f,
         0.5f, 0.5f, 0.5f, 1.0f,0.0f,0.0f,
         0.5f, 0.5f, 0.5f, 1.0f,0.0f,0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f,0.0f,0.0f,
        -0.5f,-0.5f, 0.5f, 1.0f,0.0f,0.0f,

        // Back (verde)
        -0.5f,-0.5f,-0.5f, 0.0f,1.0f,0.0f,
         0.5f,-0.5f,-0.5f, 0.0f,1.0f,0.0f,
         0.5f, 0.5f,-0.5f, 0.0f,1.0f,0.0f,
         0.5f, 0.5f,-0.5f, 0.0f,1.0f,0.0f,
        -0.5f, 0.5f,-0.5f, 0.0f,1.0f,0.0f,
        -0.5f,-0.5f,-0.5f, 0.0f,1.0f,0.0f,

        // Right (azul)
         0.5f,-0.5f, 0.5f, 0.0f,0.0f,1.0f,
         0.5f,-0.5f,-0.5f, 0.0f,0.0f,1.0f,
         0.5f, 0.5f,-0.5f, 0.0f,0.0f,1.0f,
         0.5f, 0.5f,-0.5f, 0.0f,0.0f,1.0f,
         0.5f, 0.5f,  0.5f, 0.0f,0.0f,1.0f,
         0.5f,-0.5f,  0.5f, 0.0f,0.0f,1.0f,

         // Left (amarillo)
         -0.5f, 0.5f, 0.5f, 1.0f,1.0f,0.0f,
         -0.5f, 0.5f,-0.5f, 1.0f,1.0f,0.0f,
         -0.5f,-0.5f,-0.5f, 1.0f,1.0f,0.0f,
         -0.5f,-0.5f,-0.5f, 1.0f,1.0f,0.0f,
         -0.5f,-0.5f, 0.5f, 1.0f,1.0f,0.0f,
         -0.5f, 0.5f, 0.5f, 1.0f,1.0f,0.0f,

         // Bottom (cian)
         -0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
          0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
          0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
          0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
         -0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
         -0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,

         // Top (magenta)
         -0.5f, 0.5f,-0.5f, 1.0f,0.2f,0.5f,
          0.5f, 0.5f,-0.5f, 1.0f,0.2f,0.5f,
          0.5f, 0.5f, 0.5f, 1.0f,0.2f,0.5f,
          0.5f, 0.5f, 0.5f, 1.0f,0.2f,0.5f,
         -0.5f, 0.5f, 0.5f, 1.0f,0.2f,0.5f,
         -0.5f, 0.5f,-0.5f, 1.0f,0.2f,0.5f,
    };

    // ===== VAO/VBO =====
    GLuint VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ===== Matrices =====
    glm::mat4 projection = glm::perspective(
        glm::radians(50.0f),
        (float)screenWidth / (float)screenHeight,
        0.1f, 100.0f
    );

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 2.0f, 18.0f),
        glm::vec3(0.0f, 2.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUseProgram(program);
    GLint uModel = glGetUniformLocation(program, "model");
    GLint uView = glGetUniformLocation(program, "view");
    GLint uProj = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

    const float S = 1.5f; // escala de los cubos

    // ===== Loop =====
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);

        // 🎲 9 cubos con sus posiciones y rotaciones
        glm::mat4 model(1.0f);

        // Cubo 1
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, 0.0f, 0.0f)); //Posicion
        model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1, 0, 0)); //Rota el cubo en sus diferentes caras y el angulo de inclinadcion del cubo 
        model = glm::scale(model, glm::vec3(2.0f)); // más grandes
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 2
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(2.0f)); // más grandes
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 3
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1, 0, 1));
        model = glm::scale(model, glm::vec3(2.0f)); // más grandes
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 4
        model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(60.0f), glm::vec3(1, 1, 0));
        model = glm::scale(model, glm::vec3(2.0f)); // más grandes
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 5
        model = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(75.0f), glm::vec3(1, 0.8f, 1));
        model = glm::scale(model, glm::vec3(2.0f)); // más grandes
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 6
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 3.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(1, 0, 1));
        model = glm::scale(model, glm::vec3(1.5f)); // tamaño normal
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 7
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 1));
        model = glm::scale(model, glm::vec3(1.5f)); // tamaño normal
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 8
        model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0.5f));
        model = glm::scale(model, glm::vec3(1.5f)); // tamaño normal
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cubo 9
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6.0f, 0.0f));
        model = glm::rotate(model, glm::radians(130.0f), glm::vec3(1, 1, 0));
        model = glm::scale(model, glm::vec3(1.0f)); // más chico
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    // ===== Limpieza =====
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return EXIT_SUCCESS;
}
