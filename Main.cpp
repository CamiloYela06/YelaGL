#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

// Vertex shader source code
// Fuente del vertex shader (define la posición de los vértices en clip space)

// Fragment shader source code
// Fuente del fragment shader (color único para el triángulo)
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
" FragColor = vec4(0.8f, 0.2f, 0.2f, 1.0f);\n"
"}\n\0";
/// Punto de entrada del programa.
/// Inicializa GLFW/GLAD, compila shaders, crea buffers, ejecuta el bucle de render y limpia recursos.
int main()
{
	// Initialize GLFW
	glfwInit();

	//Diciendole a GLFW que version de OpenGL queremos usar (3.3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Diciendole a GLFW que queremos usar el perfil CORE de OpenGL (sin funciones obsoletas)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//Coordenadas de los vertices del triangulo
	GLfloat vertices[] = {
		-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Bottom left
		 0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Bottom right
		 0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f,  // Top center
		 - 0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Inner left
		0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Inner right
		0.0f,  -0.5f * float(sqrt(3)) / 3, 0.0f  // Inner top
	};

	GLuint indices[] = {
		0, 3, 5, // Left triangle
		3, 2, 4, // Top triangle
		5, 4, 1  // Right triangle
	};

	//Creando la ventana llamada "YelaGL" con un tamaño de 800x800
	GLFWwindow* window = glfwCreateWindow(800, 800, "YelaGL", NULL, NULL);
	//Verificando que la ventana se haya creado correctamente
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//Haciendo el contexto de la ventana actual
	glfwMakeContextCurrent(window);

	//Cargando todas las funciones de OpenGL con GLAD
	gladLoadGL();


	// Definiendo el tamaño del viewport y el color de fondo de la ventana
	glViewport(0, 0, 800, 800);



	// Compilar y enlazar shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader); // liberamos shader individual
	glDeleteShader(fragmentShader); // liberamos shader individual


	GLuint VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Configurar VAO/VBO para el triángulo
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Primer clear y swap para mostrar el fondo inicial
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	// Limpiar el buffer de color
	glClear(GL_COLOR_BUFFER_BIT);
	// Intercambiando los buffers de la ventana
	glfwSwapBuffers(window);
	
	// Bucle principal: renderizar hasta que se cierre la ventana
	while (!glfwWindowShouldClose(window))
	{

		// Limpiar, usar programa y dibujar
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		// Procesar eventos de entrada
		glfwPollEvents();
	}


	//Borrar los recursos de OpenGL
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	// Limpiando los recursos de GLFW (ventana y contexto)
	glfwDestroyWindow(window);
	// Terminando GLFW
	glfwTerminate();
	return 0;
}