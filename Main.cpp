#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <string>

// Tamaño de la ventana
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// Shaders básicos
const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec4 aColor;\n"
"out vec4 ourColor;\n"
"uniform mat4 uMVP;\n"
"void main() {\n"
"    gl_Position = uMVP * vec4(aPos, 1.0);\n"
"    ourColor = aColor;\n"
"}\0";
        
const char* fragmentShaderSource =
"#version 330 core\n"
"in vec4 ourColor;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = ourColor;\n"
"}\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Variables de la cámara
glm::vec3 cameraPos = glm::vec3(1.5f, 0.8f, 3.0f);
glm::vec3 cameraFront = glm::normalize(
    glm::vec3(0.0f, 0.0f, 0.0f) - cameraPos
);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float cameraSpeed = 2.5f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Procesar movimiento de la cámara
void processInput(GLFWwindow* window) {

    // Salir del programa
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Dirección horizontal de la cámara
    glm::vec3 front = glm::normalize(
        glm::vec3(cameraFront.x, 0.0f, cameraFront.z)
    );

    // Dirección hacia la derecha
    glm::vec3 right = glm::normalize(
        glm::cross(front, cameraUp)
    );

    // Movimiento con WASD
    float velocity = cameraSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += front * velocity;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= front * velocity;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= right * velocity;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += right * velocity;
}

int main() {
    // Imprimir nombre del proyecto en consola
    std::cout << "==========================================" << std::endl;
    std::cout << " Proyecto: Triangulos_ CAMILO YELA" << std::endl;
    std::cout << "==========================================" << std::endl;

    // 1. Inicialización de GLFW
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW" << std::endl;
        return -1;
    }

    // Indicamos a GLFW la versión de OpenGL que queremos usar (3.3).
    // Esto hace que el contexto creado exponga las funciones de esa versión.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Seleccionamos el perfil CORE para excluir funciones obsoletas de compatibilidad.
    // En perfil CORE algunas funciones antiguas (fixed-function pipeline) no están disponibles.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    
    // Crear la ventana con el nombre del proyecto y estudiante
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangulos_ CAMILO YELA", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 2. Cargar GLAD
    // Cargar punteros a las funciones de OpenGL para la plataforma actual.
    // IMPORTANTE: debe llamarse después de crear el contexto (glfwMakeContextCurrent).
    // gladLoadGLLoader devuelve 0/1 según éxito; comprobamos el retorno para manejar errores.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD" << std::endl;
        return -1;
    }

    // Activar Depth Test y Blending para transparencia Alpha
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 3. Compilar Shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Tras compilar conviene comprobar el estado con glGetShaderiv(..., GL_COMPILE_STATUS, ...)
    // y obtener mensajes con glGetShaderInfoLog en caso de error para depuración.

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Igual que con el vertex shader, comprobar GL_COMPILE_STATUS tras compilar.

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // También es buena práctica comprobar el estado de enlace del programa con
    // glGetProgramiv(shaderProgram, GL_LINK_STATUS) y usar glGetProgramInfoLog si falla.

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 4. VÉRTICES DE LOS 3 TRIÁNGULOS
    // Alineados de la base en Y = -0.3, organizados uno detrás de otro en Z y con tamaños crecientes.
    // Cada bloque de 7 floats corresponde a: pos.x, pos.y, pos.z, r, g, b, a.
    // Las coordenadas Z se usan para situar los triángulos a distintas profundidades.
    float verticesTriangulos[] = {
        // --- TRIÁNGULO 1: (Al frente, Z = 0.4, Pequeño) ---
        // Posición (X, Y, Z)      Color (R, G, B, A)
        -0.2f, -0.3f,  0.4f,     0.0f, 1.0f, 0.0f, 0.6f,
         0.2f, -0.3f,  0.4f,     0.0f, 1.0f, 0.0f, 0.6f,
         0.0f,  0.1f,  0.4f,     0.0f, 1.0f, 0.0f, 0.6f,

         // --- TRIÁNGULO 2: MEDIANO (En el centro, Z = 0.0) 
         -0.4f, -0.3f,  0.0f,     1.0f, 0.92f, 0.23f, 0.6f,
          0.4f, -0.3f,  0.0f,     1.0f, 0.92f, 0.23f, 0.6f,
          0.0f,  0.3f,  0.0f,     1.0f, 0.92f, 0.23f, 0.6f,

          // --- TRIÁNGULO 3: GRANDE (Atrás, Z = -0.4) 
          -0.6f, -0.3f, -0.4f,     0.4f, 0.7f, 1.0f, 0.6f,
           0.6f, -0.3f, -0.4f,     0.4f, 0.7f, 1.0f, 0.6f,
           0.0f,  0.5f, -0.4f,     0.4f, 0.7f, 1.0f, 0.6f
    };

    // Configurar VAO/VBO para los Triángulos
    unsigned int vaoTriangulos, vboTriangulos;
    // Crear objetos de vértices: VAO guarda la configuración de atributos, VBO contiene los datos.
    glGenVertexArrays(1, &vaoTriangulos);
    glGenBuffers(1, &vboTriangulos);

    glBindVertexArray(vaoTriangulos);
    glBindBuffer(GL_ARRAY_BUFFER, vboTriangulos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTriangulos), verticesTriangulos, GL_STATIC_DRAW);

    // Describir el formato del atributo de posición:
    //  index = 0 (coincide con layout(location = 0) en el vertex shader)
    //  size = 3 (x,y,z)
    //  type = GL_FLOAT
    //  normalized = GL_FALSE (no normalizamos los floats)
    //  stride = 7 * sizeof(float) (distancia entre vértices consecutivos en bytes)
    //  pointer = (void*)0 (offset dentro del VBO donde empiezan las posiciones)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Describir el formato del atributo de color:
    //  index = 1 (layout location 1)
    //  size = 4 (r,g,b,a)
    //  pointer = offset de 3 floats desde el inicio del vértice
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Desenlazar para evitar modificaciones accidentales desde fuera de la configuración.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    int mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");

    // 5. RENDER LOOP
    while (!glfwWindowShouldClose(window)) {

        // Establecer el color de fondo que se usará al limpiar el buffer de color.
        // glClearColor define el color utilizado por glClear(GL_COLOR_BUFFER_BIT).
        // Además limpiamos el buffer de profundidad con GL_DEPTH_BUFFER_BIT porque
        // hemos activado GL_DEPTH_TEST para obtener correcto orden de profundidad.
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Matriz de Proyección
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = width > 0 ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);

        // Matriz de Vista
        glm::mat4 view = glm::lookAt(
            glm::vec3(1.5f, 0.8f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // Matriz de Modelo (Rotación suave)
        float t = (float)glfwGetTime();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, t * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        // Pasos por frame:
        // 1) limpiar buffers (ya hecho arriba con glClear)
        // 2) usar el programa de shaders (glUseProgram)
        // 3) enlazar el VAO que contiene la descripción de atributos y los VBOs
        // 4) dibujar con glDrawArrays (GL_TRIANGLES toma grupos de 3 vértices)
        // 5) intercambiar buffers y procesar eventos
        glBindVertexArray(vaoTriangulos);
        glDrawArrays(GL_TRIANGLES, 0, 9);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Liberar recursos
    glDeleteVertexArrays(1, &vaoTriangulos);
    glDeleteBuffers(1, &vboTriangulos);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
