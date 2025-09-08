// Practica4 - escena voxel (GLFW + GLEW + GLM) - Personajes estilo Adventure Time
// Reynoso Ortega Francisco Javier - 421056697
// 07/09/2025

#include <iostream>
#include <GL/glew.h>          // Extensiones modernas de OpenGL
#include <GLFW/glfw3.h>       // Ventana, contexto y entradas
#include <glm/glm.hpp>        // Matemáticas (vec/mat)
#include <glm/gtc/matrix_transform.hpp> // Transformaciones (translate/rotate/scale/perspective)
#include <glm/gtc/type_ptr.hpp>         // Pasar datos de glm* a OpenGL por puntero

// Prototipos
void Inputs(GLFWwindow* window);
void PrintHelpToConsole();

// Título de la ventana
const char* WINDOW_TITLE = "Previo 4 - Francisco Javier Reynoso Ortega - 421056697";

// ============================================================================
// Shaders embebidos (en texto). Se compilan en tiempo de ejecución.
// Vertex: transforma cada vértice y decide el color a usar
// Fragment: pinta cada fragmento/píxel con el color recibido
// ============================================================================

// Vertex Shader:
//  - Entrada location 0: posición 3D
//  - Entrada location 1: color por vértice (se ignora si usamos override)
//  - Uniformes: model, view, projection (MVP), bandera useOverride y color overrideColor
//  - Salida vColor: color que viajará al fragment shader
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

// Fragment Shader:
//  - Recibe vColor y lo escribe como rgba con alfa = 1.0
const char* kFragmentSrc = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// ============================================================================
// Utilidades: compilar y enlazar shaders con manejo de errores
// ============================================================================
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
    // Limpieza: ya no se necesitan los objetos shader por separado
    glDetachShader(p, vs); glDetachShader(p, fs);
    glDeleteShader(vs); glDeleteShader(fs);
    return p;
}

// ============================================================================
// Estado global de la aplicación / cámara
// WIDTH/HEIGHT: tamaño ventana. movX/movY/movZ: desplazamientos de cámara.
// rot: rotación yaw de la cámara (grados).
// ============================================================================
const GLint WIDTH = 800, HEIGHT = 600;
float movX = 0.0f, movY = -0.2f, movZ = -8.0f, rot = 15.0f;

// ============================================================================
// Geometría base: un cubo triangulado con color por vértice (6 caras * 2 triángulos * 3 vértices)
// Si useOverride == 1 en el shader, se ignoran estos colores y se usa el uniforme overrideColor.
// ============================================================================
static float kCubeVerts[] = {
    // Frente (rojo)
    -0.5f,-0.5f, 0.5f, 1,0,0,  0.5f,-0.5f, 0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0,
     0.5f, 0.5f, 0.5f, 1,0,0, -0.5f, 0.5f, 0.5f, 1,0,0, -0.5f,-0.5f, 0.5f, 1,0,0,
     // Atrás (verde)
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

// IDs del VAO (atributos) y VBO (buffer de vértices)
GLuint VAO = 0, VBO = 0;

// ============================================================================
// drawBox: helper para dibujar una caja (cubo escalado) con:
//  - pos:    posición del centro (X, Y, Z) en unidades de mundo
//  - scl:    dimensiones finales (ancho, alto, profundidad). Base = cubo unidad [-0.5,0.5]
//  - color:  color uniforme (si useOverride=1)
//  - rotEulerDeg: rotaciones (grados) en X,Y,Z (opcional)
// IMPORTANTE: el orden de transformaciones es T -> R -> S
// ============================================================================
inline void drawBox(GLuint program, GLint uModel, GLint uUseOv, GLint uOvColor,
    const glm::vec3& pos, const glm::vec3& scl,
    const glm::vec3& color, const glm::vec3& rotEulerDeg = glm::vec3(0))
{
    glm::mat4 M(1.f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, glm::radians(rotEulerDeg.x), glm::vec3(1, 0, 0));
    M = glm::rotate(M, glm::radians(rotEulerDeg.y), glm::vec3(0, 1, 0));
    M = glm::rotate(M, glm::radians(rotEulerDeg.z), glm::vec3(0, 0, 1));
    M = glm::scale(M, scl);

    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
    glUniform1i(uUseOv, 1);                       // usar color uniforme
    glUniform3fv(uOvColor, 1, glm::value_ptr(color));
    glDrawArrays(GL_TRIANGLES, 0, 36);           // 36 vértices = 12 triángulos
}

int main() {
    // ------------------------------------------------------------------------
    // Inicialización de GLFW + creación de ventana/contexto
    // ------------------------------------------------------------------------
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return EXIT_FAILURE; }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // ventana fija
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window);            // Hace el contexto actual

    // ------------------------------------------------------------------------
    // Inicialización de GLEW (para cargar funciones modernas de OpenGL)
    // ------------------------------------------------------------------------
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) { std::cerr << "Failed to initialise GLEW\n"; return EXIT_FAILURE; }

    // Ajuste del viewport y opciones GL
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);                   // Z-buffer para ocultar caras traseras
    glEnable(GL_BLEND);                        // Habilitar transparencia (si hiciera falta)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    PrintHelpToConsole();                      // Muestra controles en consola

    // ------------------------------------------------------------------------
    // Compilar/enlazar shaders y activar el programa
    // ------------------------------------------------------------------------
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentSrc);
    GLuint program = linkProgram(vs, fs);
    glUseProgram(program);

    // ------------------------------------------------------------------------
    // Configurar VAO/VBO con la geometría del cubo
    //  - Atrib 0: posición (3 floats) stride 6 floats
    //  - Atrib 1: color    (3 floats) offset 3 floats
    // ------------------------------------------------------------------------
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

    // ------------------------------------------------------------------------
    // Proyección en perspectiva (FOV 45°, aspect según framebuffer)
    // ------------------------------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // Uniform locations (guardamos handles a los uniformes del shader)
    GLint uModel = glGetUniformLocation(program, "model");
    GLint uView = glGetUniformLocation(program, "view");
    GLint uProj = glGetUniformLocation(program, "projection");
    GLint uUseOv = glGetUniformLocation(program, "useOverride");
    GLint uOvCol = glGetUniformLocation(program, "overrideColor");
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection)); // fija proyección

    // ------------------------------------------------------------------------
    // Paleta de colores (helper RGB y constantes)
    // ------------------------------------------------------------------------
    auto RGB = [](float r, float g, float b) { return glm::vec3(r, g, b); };
    const glm::vec3 ORANGE = RGB(1.0f, 0.60f, 0.10f);   // Jake
    const glm::vec3 ORANGE_D = RGB(0.95f, 0.50f, 0.05f);
    const glm::vec3 SKIN = RGB(1.00f, 0.85f, 0.75f);    // piel Finn
    const glm::vec3 WHITE = RGB(1.00f, 1.00f, 1.00f);
    const glm::vec3 SHIRT = RGB(0.10f, 0.70f, 0.75f);   // playera
    const glm::vec3 SHORTS = RGB(0.10f, 0.35f, 0.90f);  // shorts
    const glm::vec3 SOCKS = RGB(0.95f, 0.95f, 0.95f);
    const glm::vec3 SHOES = RGB(0.10f, 0.10f, 0.10f);
    const glm::vec3 BACKPACK = RGB(0.20f, 0.70f, 0.25f);
    const glm::vec3 BMO_BODY = RGB(0.20f, 0.65f, 0.50f);
    const glm::vec3 BMO_SCREEN = RGB(0.70f, 0.95f, 0.85f);
    const glm::vec3 BLUE_ARMS = RGB(0.20f, 0.55f, 0.95f);
    const glm::vec3 GOLD = RGB(1.00f, 0.76f, 0.18f);
    const glm::vec3 GREY = RGB(0.45f, 0.45f, 0.50f);
    const glm::vec3 BLACK = RGB(0.0f, 0.0f, 0.0f);          // negro puro
    const glm::vec3 BROWN = RGB(0.55f, 0.27f, 0.07f);       // café estilo "saddle brown"
    const glm::vec3 BROWN_D = glm::vec3(0.35f, 0.18f, 0.10f);// café oscuro interior boca
    const glm::vec3 BMO_BEZEL = glm::vec3(0.16f, 0.55f, 0.43f); // marco pantalla BMO
    const glm::vec3 RED_BTN = glm::vec3(0.90f, 0.20f, 0.25f);
    const glm::vec3 BLUE_BTN = glm::vec3(0.20f, 0.55f, 0.95f);
    const glm::vec3 LEG_DARK = glm::vec3(0.12f, 0.18f, 0.20f);   // piernas oscuras

    // ========================================================================
    // Bucle principal de render
    // ========================================================================
    while (!glfwWindowShouldClose(window)) {
        Inputs(window);          // Lee teclado y actualiza movX/movY/movZ/rot
        glfwPollEvents();        // Procesa eventos de ventana

        // Limpieza del frame (color de fondo y z-buffer)
        glClearColor(0.60f, 0.00f, 0.60f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --------------------------------------------------------------------
        // Cámara (vista). Se aplica translate y luego rotación yaw sobre Y.
        // Nota de orientación de escena:
        //   +X = derecha, +Y = arriba, +Z = hacia la cámara.
        // --------------------------------------------------------------------
        glm::mat4 view(1.0f);
        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0, 1, 0));
        glUseProgram(program);
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        // Activamos VAO del cubo y forzamos usar color por uniforme
        glBindVertexArray(VAO);
        glUniform1i(uUseOv, 1);

        // --------------------------------------------------------------------
        // (Opcional) Piso
        // drawBox(... GREY);
        // --------------------------------------------------------------------

        // ==================== JAKE (derecha) ====================
        // Bloque cuerpo/cabeza
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.5f, 1.0f), ORANGE);
        // Hocico frontal
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.5f, 0.14f, 0.0f), glm::vec3(0.10f, 0.25f, 0.60f), ORANGE_D);
        // Nariz
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.45f, 0.15f, 0.0f), glm::vec3(0.1f, 0.25f, 0.20f), BLACK);
        // Ojos/pupilas
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.5f, 0.4f, 0.25f), glm::vec3(0.11f, 0.4f, 0.4f), BLACK); // der
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.5f, 0.4f, -0.25f), glm::vec3(0.11f, 0.4f, 0.4f), BLACK); // izq
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.45f, 0.4f, 0.25f), glm::vec3(0.09f, 0.25f, 0.30f), WHITE);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.45f, 0.4f, -0.25f), glm::vec3(0.09f, 0.25f, 0.30f), WHITE);
        // Patas
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.9f, 0.35f), glm::vec3(0.20f, 0.60f, 0.20f), ORANGE);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.9f, -0.35f), glm::vec3(0.20f, 0.60f, 0.20f), ORANGE);
        // Brazos
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.10f, 0.55f), glm::vec3(0.18f, 0.18f, 0.70f), ORANGE); // izq
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.15f, -0.10f, -0.56f), glm::vec3(0.70f, 0.18f, 0.18f), ORANGE);
        // Barra lateral (brazo largo que baja)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(2.0f, -0.6f, 0.8f), glm::vec3(0.18f, 1.2f, 0.18f), ORANGE);

        // Boca (borde + interior)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.48f, 0.02f, 0.0f), glm::vec3(0.18f, 0.10f, 0.32f), BROWN);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(1.49f, 0.02f, 0.0f), glm::vec3(0.12f, 0.06f, 0.26f), BLACK);

        // ============== Brazo elástico de Jake hacia Finn ==============
        // Tramo horizontal principal (pasa frente a BMO)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.4f, -1.1f, 0.80f), glm::vec3(4.9f, 0.18f, 0.20f), ORANGE);
        // Subida cerca de Finn
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.8f, -0.4f, 0.75f), glm::vec3(0.18f, 1.5f, 0.18f), ORANGE);
        // Codo hacia dentro (Z)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.80f, 0.25f, 0.50f), glm::vec3(0.18f, 0.18f, 0.50f), ORANGE);

        // ==================== FINN (izquierda) ====================
        // Cabeza / gorro
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, 0.80f, 0.00f), glm::vec3(0.70f, 0.70f, 0.70f), WHITE);
        // Orejas del gorro
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.45f, 1.15f, 0.05f), glm::vec3(0.18f, 0.18f, 0.18f), WHITE);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.95f, 1.15f, -0.05f), glm::vec3(0.18f, 0.18f, 0.18f), WHITE);
        // Cara (placa al frente)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, 0.78f, 0.35f), glm::vec3(0.55f, 0.55f, 0.05f), SKIN);
        // Ojos
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.07f, 0.90f, 0.40f), glm::vec3(0.08f, 0.08f, 0.05f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.35f, 0.90f, 0.40f), glm::vec3(0.08f, 0.08f, 0.05f), BLACK);
        // Boca (interior + diente)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, 0.72f, 0.40f), glm::vec3(0.14f, 0.08f, 0.05f), BROWN_D);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, 0.76f, 0.41f), glm::vec3(0.14f, 0.04f, 0.05f), WHITE);
        // Torso y shorts
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, 0.11f, 0.00f), glm::vec3(0.80f, 0.70f, 0.45f), SHIRT);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, -0.38f, 0.00f), glm::vec3(0.75f, 0.30f, 0.45f), SHORTS);
        // Pretina de separación (decorativa)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, -0.07f, 0.00f), glm::vec3(0.78f, 0.06f, 0.46f), glm::vec3(0.0f, 0.55f, 0.75f));
        // Piernas
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.45f, -0.80f, 0.15f), glm::vec3(0.18f, 0.52f, 0.18f), SKIN);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.95f, -0.80f, -0.15f), glm::vec3(0.18f, 0.52f, 0.18f), SKIN);
        // Calcetas
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.45f, -1.05f, 0.15f), glm::vec3(0.18f, 0.10f, 0.18f), SOCKS);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.95f, -1.05f, -0.15f), glm::vec3(0.18f, 0.10f, 0.18f), SOCKS);
        // Zapatos
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.45f, -1.18f, 0.15f), glm::vec3(0.20f, 0.12f, 0.28f), SHOES);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.95f, -1.18f, -0.15f), glm::vec3(0.20f, 0.12f, 0.28f), SHOES);
        // Brazos Finn
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.75f, 0.25f, 0.25f), glm::vec3(0.40f, 0.16f, 0.16f), SKIN); // izq
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.87f, 0.34f, 0.25f), glm::vec3(0.16f, 0.16f, 0.16f), SKIN);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.55f, 0.25f, 0.22f), glm::vec3(0.55f, 0.16f, 0.16f), SKIN); // der
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.35f, 0.34f, 0.22f), glm::vec3(0.08f, 0.08f, 0.08f), SKIN);
        // Mochila y tirantes
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.20f, 0.20f, -0.35f), glm::vec3(0.55f, 0.55f, 0.30f), BACKPACK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-1.90f, 0.25f, 0.32f), glm::vec3(0.10f, 0.50f, 0.06f), BACKPACK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-2.45f, 0.25f, 0.32f), glm::vec3(0.10f, 0.50f, 0.06f), BACKPACK);

        // ==================== BMO (centro) ====================
        // Cuerpo principal
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.10f, -0.38f, 0.00f), glm::vec3(0.60f, 0.95f, 0.45f), BMO_BODY);
        // Pantalla (marco + pantalla)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.10f, -0.15f, 0.23f), glm::vec3(0.46f, 0.36f, 0.04f), BMO_BEZEL);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.10f, -0.15f, 0.26f), glm::vec3(0.36f, 0.26f, 0.03f), BMO_SCREEN);
        // Cara (OJO: en tu fuente había "0.-0.10f" que no compila; debería ser -0.10f)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.18f, -0.10f, 0.28f), glm::vec3(0.05f, 0.06f, 0.03f), BLACK); // ojo izq
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.02f, -0.10f, 0.28f), glm::vec3(0.05f, 0.06f, 0.03f), BLACK); // ojo der
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.10f, -0.20f, 0.28f), glm::vec3(0.10f, 0.04f, 0.03f), BLACK); // boca
        // Botonera (cruz + botones)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.17f, -0.60f, 0.29f), glm::vec3(0.10f, 0.10f, 0.03f), GOLD);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.17f, -0.52f, 0.29f), glm::vec3(0.10f, 0.08f, 0.03f), GOLD);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.17f, -0.68f, 0.29f), glm::vec3(0.10f, 0.08f, 0.03f), GOLD);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.25f, -0.60f, 0.29f), glm::vec3(0.08f, 0.10f, 0.03f), GOLD);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.09f, -0.60f, 0.29f), glm::vec3(0.08f, 0.10f, 0.03f), GOLD);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.10f, -0.50f, 0.29f), glm::vec3(0.08f, 0.08f, 0.03f), RED_BTN);  // azul/rojo intercambiados en paleta original, se deja como está
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.10f, -0.70f, 0.29f), glm::vec3(0.08f, 0.08f, 0.03f), BLUE_BTN);
        // Rejillas laterales
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.38f, -0.10f, 0.12f), glm::vec3(0.05f, 0.05f, 0.05f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.38f, -0.10f, -0.02f), glm::vec3(0.05f, 0.05f, 0.05f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.38f, -0.10f, -0.16f), glm::vec3(0.05f, 0.05f, 0.05f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.38f, -0.30f, 0.00f), glm::vec3(0.06f, 0.10f, 0.28f), BLACK);
        // Brazos BMO (hombro + antebrazo)
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.42f, -0.50f, 0.00f), glm::vec3(0.28f, 0.08f, 0.08f), BLUE_ARMS);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.52f, -0.65f, 0.00f), glm::vec3(0.08f, 0.30f, 0.08f), BLUE_ARMS);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.20f, -0.50f, 0.00f), glm::vec3(0.28f, 0.08f, 0.08f), BLUE_ARMS);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.30f, -0.65f, 0.00f), glm::vec3(0.08f, 0.30f, 0.08f), BLUE_ARMS);
        // Piernas y pies
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.20f, -0.95f, 0.10f), glm::vec3(0.10f, 0.25f, 0.10f), LEG_DARK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.00f, -0.95f, -0.10f), glm::vec3(0.10f, 0.25f, 0.10f), LEG_DARK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(-0.20f, -1.10f, 0.10f), glm::vec3(0.12f, 0.10f, 0.18f), BLACK);
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(0.00f, -1.10f, -0.10f), glm::vec3(0.12f, 0.10f, 0.18f), BLACK);

        // ==================== Espada (al fondo) ====================
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(3.5f, -0.40f, -2.0f), glm::vec3(0.10f, 1.30f, 0.10f), GOLD); // hoja
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(3.5f, 0.20f, -2.0f), glm::vec3(0.50f, 0.10f, 0.20f), GOLD); // guarda
        drawBox(program, uModel, uUseOv, uOvCol, glm::vec3(3.5f, 0.40f, -2.0f), glm::vec3(0.12f, 0.35f, 0.12f), GREY); // empuñadura

        // Final del frame
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    // Limpieza de recursos GL
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return EXIT_SUCCESS;
}

// ============================================================================
// Manejo de entradas de teclado (Movimiento de cámara)
//  ESC: salir
//  W/S: acercar/alejar (Z)
//  A/D: izquierda/derecha (X)
//  PageUp/Down: arriba/abajo (Y)
//  ←/→: rotar cámara (yaw sobre Y)
// ============================================================================
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

// ============================================================================
// Ayuda en consola con el mapa de controles
// ============================================================================
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
