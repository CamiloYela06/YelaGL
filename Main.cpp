#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

// Vertex shader source code
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    " gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
// Fragment shader source code
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
" FragColor = vec4(0.8f, 0.2f, 0.2f, 1.0f);\n"
"}\n\0";
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
		 0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f  // Top center
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



	// Creando y compilando el vertex shader y el fragment shader, y luego creando un programa de shader y enlazando los shaders a él
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

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	// Limpiando el buffer de color
	glClear(GL_COLOR_BUFFER_BIT);
	// Intercambiando los buffers de la ventana
	glfwSwapBuffers(window);
	
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
		// Limpiando el buffer de color y cuidando todos los eventos de GLFW (como el cierre de la ventana)
		glfwPollEvents();
	}


	//Borrar los recursos de OpenGL
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	// Limpiando los recursos de GLFW (ventana y contexto)
	glfwDestroyWindow(window);
	// Terminando GLFW
	glfwTerminate();
	return 0;
}