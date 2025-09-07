// Previo4 - escena voxel (GLFW + GLEW + GLM) - Personajes estilo Adventure Time
// Reynoso Ortega Francisco Javier - 421056697
// Compilar como C++11+. Vincular: opengl32.lib, glew32.lib, glfw3.lib
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Inputs(GLFWwindow* window);
void PrintHelpToConsole();

const char* WINDOW_TITLE = "Previo 4 - Francisco Javier Reynoso Ortega - 421056697";

// --- Shaders embebidos ---
// A�adimos uniformes para forzar color por instancia
const char* kVertexSrc = R"(#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int  useOverride;
uniform vec3 overrideColor;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    vColor = (useOverride == 1) ? overrideColor : color;
}
)";

const char* kFragmentSrc = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// utilidades de shader
static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        if (len <= 0) len = 1;
        std::string log(len, '\0');
        glGetShaderInfoLog(s, len, nullptr, &log[0]);
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
        glGetProgramInfoLog(p, len, nullptr, &log[0]);
        std::cerr << "ERROR linking program:\n" << log << std::endl;
        exit(EXIT_FAILURE);
    }
    glDetachShader(p, vs); glDetachShader(p, fs);
    glDeleteShader(vs); glDeleteShader(fs);
    return p;
}

// estado global de entrada
const GLint WIDTH = 800, HEIGHT = 600;
float movX = 0.0f, movY = -0.2f, movZ = -8.0f, rot = 15.0f;

// --- V�rtices del cubo (triangulado) con un color por cara (se ignora si usamos override) ---
static float kCubeVerts[] = {
    // Frente (rojo)
    -0.5f,-0.5f, 0.5f, 1,0,0,  0.5f,-0.5f, 0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0,
     0.5f, 0.5f, 0.5f, 1,0,0, -0.5f, 0.5f, 0.5f, 1,0,0, -0.5f,-0.5f, 0.5f, 1,0,0,
     // Atr�s (verde)
     -0.5f,-0.5f,-0.5f, 0,1,0,  0.5f,-0.5f,-0.5f, 0,1,0,  0.5f, 0.5f,-0.5f, 0,1,0,
      0.5f, 0.5f,-0.5f, 0,1,0, -0.5f, 0.5f,-0.5f, 0,1,0, -0.5f,-0.5f,-0.5f, 0,1,0,
      // Derecha (azul)
       0.5f,-0.5f, 0.5f, 0,0,1,  0.5f,-0.5f,-0.5f, 0,0,1,  0.5f, 0.5f,-0.5f, 0,0,1,
       0.5f, 0.5f,-0.5f, 0,0,1,  0.5f, 0.5f, 0.5f, 0,0,1,  0.5f,-0.5f, 0.5f, 0,0,1,
       // Izquierda (amarillo)
       -0.5f, 0.5f, 0.5f, 1,1,0, -0.5f, 0.5f,-0.5f, 1,1,0, -0.5f,-0.5f,-0.5f, 1,1,0,
       -0.5f,-0.5f,-0.5f, 1,1,0, -0.5f,-0.5f, 0.5f, 1,1,0, -0.5f, 0.5f, 0.5f, 1,1,0,
       // Abajo (cian)
       -0.5f,-0.5f,-0.5f, 0,1,1,  0.5f,-0.5f,-0.5f, 0,1,1,  0.5f,-0.5f, 0.5f, 0,1,1,
        0.5f,-0.5f, 0.5f, 0,1,1, -0.5f,-0.5f, 0.5f, 0,1,1, -0.5f,-0.5f,-0.5f, 0,1,1,
        // Arriba (magenta claro)
        -0.5f, 0.5f,-0.5f, 1,0.2,0.5,  0.5f, 0.5f,-0.5f, 1,0.2,0.5,  0.5f, 0.5f, 0.5f, 1,0.2,0.5,
         0.5f, 0.5f, 0.5f, 1,0.2,0.5, -0.5f, 0.5f, 0.5f, 1,0.2,0.5, -0.5f, 0.5f,-0.5f, 1,0.2,0.5
};

GLuint VAO = 0, VBO = 0;

// Helper para dibujar una caja con posici�n/rotaci�n/escala/color
inline void drawBox(GLuint program, GLint uModel, GLint uUseOv, GLint uOvColor,
    const glm::vec3& pos, const glm::vec3& scl,
    const glm::vec3& color, const glm::vec3& rotEulerDeg = glm::vec3(0))
{
    glm::mat4 M(1.f);
    // OJO al orden: primero trasladar, luego rotar, al final escalar
    M = glm::translate(M, pos);
    M = glm::rotate(M, glm::radians(rotEulerDeg.x), glm::vec3(1, 0, 0));
    M = glm::rotate(M, glm::radians(rotEulerDeg.y), glm::vec3(0, 1, 0));
    M = glm::rotate(M, glm::radians(rotEulerDeg.z), glm::vec3(0, 0, 1));
    M = glm::scale(M, scl);
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
    glUniform1i(uUseOv, 1);
    glUniform3fv(uOvColor, 1, glm::value_ptr(color));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main() {
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

    PrintHelpToConsole();

    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentSrc);
    GLuint program = linkProgram(vs, fs);
    glUseProgram(program);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVerts), kCubeVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Proyecci�n
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // Uniform locations
    GLint uModel = glGetUniformLocation(program, "model");
    GLint uView = glGetUniformLocation(program, "view");
    GLint uProj = glGetUniformLocation(program, "projection");
    GLint uUseOv = glGetUniformLocation(program, "useOverride");
    GLint uOvCol = glGetUniformLocation(program, "overrideColor");
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));

    // Paleta r�pida
    auto RGB = [](float r, float g, float b) { return glm::vec3(r, g, b); };
    const glm::vec3 ORANGE = RGB(1.0f, 0.60f, 0.10f);  // Jake
    const glm::vec3 ORANGE_D = RGB(0.95f, 0.50f, 0.05f);
    const glm::vec3 SKIN = RGB(1.00f, 0.85f, 0.75f); // piel Finn
    const glm::vec3 WHITE = RGB(1.00f, 1.00f, 1.00f);
    const glm::vec3 SHIRT = RGB(0.10f, 0.70f, 0.75f); // azul/verde
    const glm::vec3 SHORTS = RGB(0.10f, 0.35f, 0.90f);
    const glm::vec3 SOCKS = RGB(0.95f, 0.95f, 0.95f);
    const glm::vec3 SHOES = RGB(0.10f, 0.10f, 0.10f);
    const glm::vec3 BACKPACK = RGB(0.20f, 0.70f, 0.25f);
    const glm::vec3 BMO_BODY = RGB(0.20f, 0.65f, 0.50f);
    const glm::vec3 BMO_SCREEN = RGB(0.70f, 0.95f, 0.85f);
    const glm::vec3 BLUE_ARMS = RGB(0.20f, 0.55f, 0.95f);
    const glm::vec3 GOLD = RGB(1.00f, 0.76f, 0.18f);
    const glm::vec3 GREY = RGB(0.45f, 0.45f, 0.50f);
    const glm::vec3 BLACK = RGB(0.0f, 0.0f, 0.0f);          // negro puro
    const glm::vec3 BROWN = RGB(0.55f, 0.27f, 0.07f);       // caf� estilo "saddle brown"
    const glm::vec3 BROWN_D = glm::vec3(0.35f, 0.18f, 0.10f);// Caf� oscuro para interior de la boca
    const glm::vec3 BMO_BEZEL = glm::vec3(0.16f, 0.55f, 0.43f); // marco oscuro de pantalla
    const glm::vec3 RED_BTN = glm::vec3(0.90f, 0.20f, 0.25f);
    const glm::vec3 BLUE_BTN = glm::vec3(0.20f, 0.55f, 0.95f);
    const glm::vec3 LEG_DARK = glm::vec3(0.12f, 0.18f, 0.20f); // piernas oscuras




    while (!glfwWindowShouldClose(window)) {
        Inputs(window);
        glfwPollEvents();

        glClearColor(0.60f, 0.00f, 0.60f, 1.0f); // fondo morado
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // C�mara
        glm::mat4 view(1.0f);
        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0, 1, 0));
        glUseProgram(program);
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);
        glUniform1i(uUseOv, 1); // usaremos color por uniforme

        //// ------------ Piso (plataforma delgada gris) -------------
        //drawBox(program, uModel, uUseOv, uOvCol,
        //    /*pos*/  glm::vec3(0, -1.0f, 0),
        //    /*scl*/  glm::vec3(10.0f, 0.15f, 6.0f),
        //    /*col*/  GREY);

        // ==================== JAKE (derecha) ====================
		// cuerpo/cabeza (bloque grande)                    //izquierrda,arriba,ADELANTE    ANCHO,ALT,PROF
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.5f, 1.0f), ORANGE);  
        // hocico (cubito frontal)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.5f, 0.14f, 0.0f), glm::vec3(0.10f, 0.25f, 0.60f), ORANGE_D);
		// nariz (cubito m�s peque�o)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.45f, 0.15f, 0.0f), glm::vec3(0.1f, 0.25f, 0.20f), BLACK);
		// pupilas
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.5f, 0.4f, 0.25f), glm::vec3(0.11f, 0.4f, 0.4f), BLACK );//derecho
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.5f, 0.4f, -0.25f), glm::vec3(0.11f, 0.4f, 0.4f), BLACK);//izquierdo
        // ojos (cubitos blancos)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.45f, 0.4f, 0.25f), glm::vec3(0.09f, 0.25f, 0.30f), WHITE);//derecho
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.45f, 0.4f, -0.25f), glm::vec3(0.09f, 0.25f, 0.30f), WHITE);//izquierdo
        // patas
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.9f, 0.35f), glm::vec3(0.20f, 0.60f, 0.20f), ORANGE);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.9f, -0.35f), glm::vec3(0.20f, 0.60f, 0.20f), ORANGE);
        // brazos                                           x      y       z
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.10f, 0.55f), glm::vec3(0.18f, 0.18f, 0.70f), ORANGE); //brazo izquierdo
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.15f, -0.10f, -0.56f), glm::vec3(0.70f, 0.18f, 0.18f), ORANGE);
        // barra larga que rodea (simulaci�n del brazo de Jake)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.6f, 0.8f), glm::vec3(0.18f, 1.2f, 0.18f), ORANGE);
        // --- BOCA DE JAKE: opci�n C (borde caf� + interior negro) ---
        // Borde/labio (ligeramente m�s grande)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(1.48f, 0.02f, 0.0f),   // posici�n (x, y, z)
            glm::vec3(0.18f, 0.10f, 0.32f),  // escala (ancho, alto, profundidad)
            BROWN);

        // Interior (un poco m�s adentro y m�s peque�o)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(1.49f, 0.02f, 0.0f),   // empujado 0.01 en +X para �hueco�
            glm::vec3(0.12f, 0.06f, 0.26f),  // m�s peque�o para que se vea el borde
            BLACK);




        // ==================== FINN (izquierda) ====================
        // � Cabeza / gorro (cubo blanco) �
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.20f, 0.80f, 0.00f),  // pos
            glm::vec3(0.70f, 0.70f, 0.70f),  // escala
            WHITE);
        // �orejas� del gorro (dos cubitos arriba)
        drawBox(program, uModel, uUseOv, uOvCol,
                        //x,y,z
            glm::vec3(-2.45f, 1.15f, 0.05f), glm::vec3(0.18f, 0.18f, 0.18f), WHITE);//oreja izquierda
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-1.95f, 1.15f, -0.05f), glm::vec3(0.18f, 0.18f, 0.18f), WHITE);

        // � Cara (placa delgada SKIN al frente sobre +Z) �
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.20f, 0.78f, 0.35f),  // desplaza hacia +Z para �asomar� del gorro
            glm::vec3(0.55f, 0.55f, 0.05f),
            SKIN);

        // Ojos (p�xeles negros)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.07f, 0.90f, 0.40f), glm::vec3(0.08f, 0.08f, 0.05f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.35f, 0.90f, 0.40f), glm::vec3(0.08f, 0.08f, 0.05f), BLACK);

        // Boca con diente (cartoon)
        drawBox(program, uModel, uUseOv, uOvCol,  // interior oscuro
            glm::vec3(-2.20f, 0.72f, 0.40f), glm::vec3(0.14f, 0.08f, 0.05f), BROWN_D);
        drawBox(program, uModel, uUseOv, uOvCol,  // diente blanco
            glm::vec3(-2.20f, 0.76f, 0.41f), glm::vec3(0.14f, 0.04f, 0.05f), WHITE);

        // � Torso (playera) y shorts �
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.20f, 0.11f, 0.00f), glm::vec3(0.80f, 0.70f, 0.45f), SHIRT);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.20f, -0.38f, 0.00f), glm::vec3(0.75f, 0.30f, 0.45f), SHORTS);

        // (Opcional) cintur�n/pretina para separaci�n playera/shorts
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.20f, -0.07f, 0.00f), glm::vec3(0.78f, 0.06f, 0.46f),
            glm::vec3(0.0f, 0.55f, 0.75f)); // tono intermedio

        // � Piernas �
        drawBox(program, uModel, uUseOv, uOvCol,   // pierna izq
            glm::vec3(-2.45f, -0.80f, 0.15f), glm::vec3(0.18f, 0.52f, 0.18f), SKIN);
        drawBox(program, uModel, uUseOv, uOvCol,   // pierna der
            glm::vec3(-1.95f, -0.80f, -0.15f), glm::vec3(0.18f, 0.52f, 0.18f), SKIN);

        // Calcetas
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.45f, -1.05f, 0.15f), glm::vec3(0.18f, 0.10f, 0.18f), SOCKS);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-1.95f, -1.05f, -0.15f), glm::vec3(0.18f, 0.10f, 0.18f), SOCKS);

        // Zapatos
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.45f, -1.18f, 0.15f), glm::vec3(0.20f, 0.12f, 0.28f), SHOES);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-1.95f, -1.18f, -0.15f), glm::vec3(0.20f, 0.12f, 0.28f), SHOES);

        // � Brazos �
        // Izquierdo: extendido hacia el costado/c�mara (+Z), para que �reciba� el brazo de Jake
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.75f, 0.25f, 0.25f), glm::vec3(0.40f, 0.16f, 0.16f), SKIN);
        // antebrazo vertical para remate (como si saludara)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.87f, 0.34f, 0.25f), glm::vec3(0.16f, 0.16f, 0.16f), SKIN);


        // Derecho: levantado diagonal (usamos rotaci�n Z)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-1.55f, 0.25f, 0.22f), glm::vec3(0.55f, 0.16f, 0.16f), SKIN);
        // antebrazo vertical para remate (como si saludara)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-1.35f, 0.34f, 0.22f), glm::vec3(0.08f, 0.08f, 0.08f), SKIN);

        // � Mochila y tirantes �
        drawBox(program, uModel, uUseOv, uOvCol,  // mochila
            glm::vec3(-2.20f, 0.20f, -0.35f), glm::vec3(0.55f, 0.55f, 0.30f), BACKPACK);
        // tirantes finos al frente
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-1.90f, 0.25f, 0.32f), glm::vec3(0.10f, 0.50f, 0.06f), BACKPACK);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-2.45f, 0.25f, 0.32f), glm::vec3(0.10f, 0.50f, 0.06f), BACKPACK);


        //// ============== �Brazo el�stico� (tubo rectangular) ==============
        //// barra larga que rodea (simulaci�n del brazo de Jake)
        //drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.0f, -0.2f, 0.75f), glm::vec3(3.0f, 0.10f, 0.10f), ORANGE);
        //drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.50f, 0.65f, 0.75f), glm::vec3(0.10f, 1.60f, 0.10f), ORANGE);
        //drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.50f, 0.65f, -0.75f), glm::vec3(0.10f, 1.60f, 0.10f), ORANGE);
        //drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.0f, -0.2f, -0.75f), glm::vec3(3.0f, 0.10f, 0.10f), ORANGE);

        // ==================== BMO (centro) ====================
        // ==================== BMO (centro) ====================
// Cuerpo (bloque principal)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-0.10f, -0.38f, 0.00f),  // POS (centro de BMO)
            glm::vec3(0.60f, 0.95f, 0.45f),    // DIM (ancho, alto, prof.)
            BMO_BODY);

        // ---- Pantalla con marco (bezel) ----
        drawBox(program, uModel, uUseOv, uOvCol,  // marco (un poco m�s grande y m�s �adentro�)
            glm::vec3(-0.10f, -0.15f, 0.23f),
            glm::vec3(0.46f, 0.36f, 0.04f),
            BMO_BEZEL);
        drawBox(program, uModel, uUseOv, uOvCol,  // pantalla
            glm::vec3(-0.10f, -0.15f, 0.26f),
            glm::vec3(0.36f, 0.26f, 0.03f),
            BMO_SCREEN);

        // Cara (ojos y boca sobre la pantalla)
        drawBox(program, uModel, uUseOv, uOvCol,  // ojo izq
            glm::vec3(-0.18f, 0.-0.10f, 0.28f),
            glm::vec3(0.05f, 0.06f, 0.03f),
            BLACK);
        drawBox(program, uModel, uUseOv, uOvCol,  // ojo der
            glm::vec3(-0.02f, -0.10f, 0.28f),
            glm::vec3(0.05f, 0.06f, 0.03f),
            BLACK);
        drawBox(program, uModel, uUseOv, uOvCol,  // boca
            glm::vec3(-0.10f, -0.2f, 0.28f),
            glm::vec3(0.1f, 0.04f, 0.03f),
            BLACK);

        // ---- Botonera frontal ----
        // Cruz dorada (5 piezas: centro + 4 brazos)
        drawBox(program, uModel, uUseOv, uOvCol,  // centro
            glm::vec3(-0.17f, -0.60f, 0.29f),
            glm::vec3(0.10f, 0.10f, 0.03f),
            GOLD);
        drawBox(program, uModel, uUseOv, uOvCol,  // arriba
            glm::vec3(-0.17f, -0.52f, 0.29f),
            glm::vec3(0.10f, 0.08f, 0.03f),
            GOLD);
        drawBox(program, uModel, uUseOv, uOvCol,  // abajo
            glm::vec3(-0.17f, -0.68f, 0.29f),
            glm::vec3(0.10f, 0.08f, 0.03f),
            GOLD);
        drawBox(program, uModel, uUseOv, uOvCol,  // izquierda
            glm::vec3(-0.25f, -0.60f, 0.29f),
            glm::vec3(0.08f, 0.10f, 0.03f),
            GOLD);
        drawBox(program, uModel, uUseOv, uOvCol,  // derecha
            glm::vec3(-0.09f, -0.60f, 0.29f),
            glm::vec3(0.08f, 0.10f, 0.03f),
            GOLD);

        // Botones red y blue
        drawBox(program, uModel, uUseOv, uOvCol,   // azul (abajo-der)
            glm::vec3(0.10f, -0.50f, 0.29f),
            glm::vec3(0.08f, 0.08f, 0.03f),
            RED_BTN);
        drawBox(program, uModel, uUseOv, uOvCol,   // rojo (arriba-der)
            glm::vec3(0.10f, -0.70f, 0.29f),
            glm::vec3(0.08f, 0.08f, 0.03f),
            BLUE_BTN);

        // ---- Rejillas/puertos laterales (izquierda) ----
        drawBox(program, uModel, uUseOv, uOvCol,  // cuadritos superiores
            glm::vec3(-0.38f, -0.10f, 0.12f), glm::vec3(0.05f, 0.05f, 0.05f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-0.38f, -0.10f, -0.02f), glm::vec3(0.05f, 0.05f, 0.05f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-0.38f, -0.10f, -0.16f), glm::vec3(0.05f, 0.05f, 0.05f), BLACK);
        // ranura
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-0.38f, -0.30f, 0.00f), glm::vec3(0.06f, 0.10f, 0.28f), BLACK);

        // ---- Brazos (con �codo�) ----
        // izquierdo: hombro (horizontal) + antebrazo (vertical)
        drawBox(program, uModel, uUseOv, uOvCol,  // hombro
            glm::vec3(-0.42f, -0.50f, 0.00f), glm::vec3(0.28f, 0.08f, 0.08f), BLUE_ARMS);
        drawBox(program, uModel, uUseOv, uOvCol,  // antebrazo hacia abajo
            glm::vec3(-0.52f, -0.65f, 0.00f), glm::vec3(0.08f, 0.30f, 0.08f), BLUE_ARMS);

        // derecho
        drawBox(program, uModel, uUseOv, uOvCol,  // hombro
            glm::vec3(0.20f, -0.50f, 0.00f), glm::vec3(0.28f, 0.08f, 0.08f), BLUE_ARMS);
        drawBox(program, uModel, uUseOv, uOvCol,  // antebrazo hacia abajo
            glm::vec3(0.30f, -0.65f, 0.00f), glm::vec3(0.08f, 0.30f, 0.08f), BLUE_ARMS);

        // ---- Piernas y pies ----
        drawBox(program, uModel, uUseOv, uOvCol,  // pierna izq
            glm::vec3(-0.20f, -0.95f, 0.10f), glm::vec3(0.10f, 0.25f, 0.10f), LEG_DARK);
        drawBox(program, uModel, uUseOv, uOvCol,  // pierna der
            glm::vec3(0.00f, -0.95f, -0.10f), glm::vec3(0.10f, 0.25f, 0.10f), LEG_DARK);
        // pies (m�s anchos y oscuros)
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(-0.20f, -1.10f, 0.10f), glm::vec3(0.12f, 0.10f, 0.18f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol,
            glm::vec3(0.00f, -1.10f, -0.10f), glm::vec3(0.12f, 0.10f, 0.18f), BLACK);


        //// ==================== Espada (al fondo) ====================
        //drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(3.5f, -0.50f, -2.0f), glm::vec3(0.10f, 1.30f, 0.10f), GOLD); // hoja dorada (estilo voxel)
        //drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(3.5f, 0.30f, -2.0f), glm::vec3(0.50f, 0.10f, 0.20f), GOLD); // guarda
        //drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(3.5f, 0.55f, -2.0f), glm::vec3(0.12f, 0.35f, 0.12f), GREY); // empu�adura

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return EXIT_SUCCESS;
}

// Entrada de teclado
void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)         movX += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)         movX -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)   movY += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) movY -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)         movZ -= 0.08f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)         movZ += 0.08f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)     rot += 0.6f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)      rot -= 0.6f;
}

// Gu�a
void PrintHelpToConsole() {
    std::cout <<
        "================ GUIA DE COMANDOS ================\n"
        " ESC        : Cerrar la ventana\n"
        " W / S      : Acercar / Alejar (eje Z)\n"
        " A / D      : Mover izquierda / derecha (eje X)\n"
        " PageUp/Down: Subir / bajar (eje Y)\n"
        " <- / ->    : Rotar sobre Y (yaw)\n"
        "==================================================\n" << std::endl;
}
