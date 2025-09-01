// Previo4 - cubo con shaders embebidos (GLFW + GLEW + GLM)
// Reynoso Ortega Francisco Javier
// 421056697
// Fecha de entrega: 01/09/2025
// Compilar como C++11+. Vincular: opengl32.lib, glew32.lib, glfw3.lib

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Inputs(GLFWwindow* window);
void PrintHelpToConsole();

// --- Título fijo de la ventana ---
const char* WINDOW_TITLE =
"Previo 4 - Francisco Javier Reynoso Ortega - 421056697";

// --- Shaders embebidos ---
const char* kVertexSrc = R"(#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    vColor = color;
}
)";

const char* kFragmentSrc = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// --- utilidades de shader ---
static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        if (len <= 0) len = 1;
        std::string log(len, '\0');
        glGetShaderInfoLog(s, len, nullptr, &log[0]); // usar &log[0] (no-const)
        std::cerr << "ERROR compiling " << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
            << " shader:\n" << log << std::endl;
        exit(EXIT_FAILURE);
    }
    return s;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        if (len <= 0) len = 1;
        std::string log(len, '\0');
        glGetProgramInfoLog(p, len, nullptr, &log[0]); // usar &log[0] (no-const)
        std::cerr << "ERROR linking program:\n" << log << std::endl;
        exit(EXIT_FAILURE);
    }
    glDetachShader(p, vs);
    glDetachShader(p, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

// --- estado global de entrada ---
const GLint WIDTH = 800, HEIGHT = 600;
float movX = 0.0f, movY = 0.0f, movZ = -5.0f, rot = 0.0f;

int main() {
    // Init GLFW
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return EXIT_FAILURE; }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) { std::cerr << "Failed to initialise GLEW\n"; return EXIT_FAILURE; }

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Guía de comandos en consola
    PrintHelpToConsole();

    // Compilar y linkear shaders
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentSrc);
    GLuint program = linkProgram(vs, fs);

    // Datos del cubo (posicion xyz + color rgb) -> 6 floats por vertice
    float vertices[] = {
        // Frente (rojo)
        -0.5f,-0.5f, 0.5f, 1.0f,0.0f,0.0f,
         0.5f,-0.5f, 0.5f, 1.0f,0.0f,0.0f,
         0.5f, 0.5f, 0.5f, 1.0f,0.0f,0.0f,
         0.5f, 0.5f, 0.5f, 1.0f,0.0f,0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f,0.0f,0.0f,
        -0.5f,-0.5f, 0.5f, 1.0f,0.0f,0.0f,
        // Atrás (verde)
        -0.5f,-0.5f,-0.5f, 0.0f,1.0f,0.0f,
         0.5f,-0.5f,-0.5f, 0.0f,1.0f,0.0f,
         0.5f, 0.5f,-0.5f, 0.0f,1.0f,0.0f,
         0.5f, 0.5f,-0.5f, 0.0f,1.0f,0.0f,
        -0.5f, 0.5f,-0.5f, 0.0f,1.0f,0.0f,
        -0.5f,-0.5f,-0.5f, 0.0f,1.0f,0.0f,
        // Derecha (azul)
         0.5f,-0.5f, 0.5f, 0.0f,0.0f,1.0f,
         0.5f,-0.5f,-0.5f, 0.0f,0.0f,1.0f,
         0.5f, 0.5f,-0.5f, 0.0f,0.0f,1.0f,
         0.5f, 0.5f,-0.5f, 0.0f,0.0f,1.0f,
         0.5f, 0.5f, 0.5f, 0.0f,0.0f,1.0f,
         0.5f,-0.5f, 0.5f, 0.0f,0.0f,1.0f,
         // Izquierda (amarillo)
         -0.5f, 0.5f, 0.5f, 1.0f,1.0f,0.0f,
         -0.5f, 0.5f,-0.5f, 1.0f,1.0f,0.0f,
         -0.5f,-0.5f,-0.5f, 1.0f,1.0f,0.0f,
         -0.5f,-0.5f,-0.5f, 1.0f,1.0f,0.0f,
         -0.5f,-0.5f, 0.5f, 1.0f,1.0f,0.0f,
         -0.5f, 0.5f, 0.5f, 1.0f,1.0f,0.0f,
         // Abajo (cian)
         -0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
          0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
          0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
          0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
         -0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
         -0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
         // Arriba (magenta claro)
         -0.5f, 0.5f,-0.5f, 1.0f,0.2f,0.5f,
          0.5f, 0.5f,-0.5f, 1.0f,0.2f,0.5f,
          0.5f, 0.5f, 0.5f, 1.0f,0.2f,0.5f,
          0.5f, 0.5f, 0.5f, 1.0f,0.2f,0.5f,
         -0.5f, 0.5f, 0.5f, 1.0f,0.2f,0.5f,
         -0.5f, 0.5f,-0.5f, 1.0f,0.2f,0.5f,
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // layout(location=0) -> position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // layout(location=1) -> color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // Ubicaciones de uniformes
    glUseProgram(program);
    GLint uModel = glGetUniformLocation(program, "model");
    GLint uView = glGetUniformLocation(program, "view");
    GLint uProj = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));

    while (!glfwWindowShouldClose(window)) {
        Inputs(window);
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Cámara
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        glUseProgram(program);
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);

        // -------- OBJETO 1: "MESA" (caja aplanada roja) --------
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(3.0f, 0.1f, 2.0f));  // ancho, alto, profundidad
        model = glm::translate(model, glm::vec3(0.0f, 0.6f, 0.0f)); // baja la mesa
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model)); // subir DESPUÉS de escalar
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // -------- OBJETO 2: "PATA1 DE LA MESA" --------
        model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.1f, 0.6f, 0.1f));  // dimension de la pata
        model = glm::translate(model, glm::vec3(2.9f, -0.6f, 1.9f));//posisicion de la pata 
		glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model)); // Mandar llamar DESPUÉS de escalar
		glDrawArrays(GL_TRIANGLES, 0, 36); //dibujar la pata

        // -------- OBJETO 2: "PATA2 DE LA MESA" --------
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f, 0.6f, 0.1f));  // dimension de la pata
        model = glm::translate(model, glm::vec3(-2.9f, -0.6f, 1.9f));//posisicion de la pata 
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model)); // Mandar llamar DESPUÉS de escalar
        glDrawArrays(GL_TRIANGLES, 0, 36); //dibujar la pata


        // -------- OBJETO 2: "PATA3 DE LA MESA" --------
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f, 0.6f, 0.1f));  // dimension de la pata
        model = glm::translate(model, glm::vec3(-2.9f, -0.6f, -1.9f));//posisicion de la pata 
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model)); // Mandar llamar DESPUÉS de escalar
        glDrawArrays(GL_TRIANGLES, 0, 36); //dibujar la pata

        // -------- OBJETO 2: "PATA4 DE LA MESA" --------
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f, 0.6f, 0.1f));  // dimension de la pata
        model = glm::translate(model, glm::vec3(2.9f, -0.6f, -1.9f));//posisicion de la pata 
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model)); // Mandar llamar DESPUÉS de escalar
        glDrawArrays(GL_TRIANGLES, 0, 36); //dibujar la pata


        //// -------- OBJETO 2: CUBO normal encima (opcional) --------
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return EXIT_SUCCESS;
}

// --- Entrada de teclado ---
void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)         movX += 0.08f; // +X
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)         movX -= 0.08f; // -X
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)   movY += 0.08f; // +Y
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) movY -= 0.08f; // -Y
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)         movZ -= 0.08f; // acercar (Z-)
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)         movZ += 0.08f; // alejar  (Z+)
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)     rot += 0.4f;  // rotar +Y
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)      rot -= 0.4f;  // rotar -Y
}

// --- Guía en consola (se mantiene) ---
void PrintHelpToConsole() {
    std::cout <<
        "================ GUIA DE COMANDOS ================\n"
        " ESC        : Cerrar la ventana\n"
        " W / S      : Acercar / Alejar (eje Z)\n"
        " A / D      : Mover izquierda / derecha (eje X)\n"
        " PageUp/Down: Subir / bajar (eje Y)\n"
        " <- / ->    : Rotar sobre Y (yaw)\n"
        "==================================================\n"
        << std::endl;
}
