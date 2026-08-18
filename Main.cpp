#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

int main()
{
	// Initialize GLFW
	glfwInit();

	//Diciendole a GLFW que version de OpenGL queremos usar (3.3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Diciendole a GLFW que queremos usar el perfil CORE de OpenGL (sin funciones obsoletas)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	// Limpiando el buffer de color
	glClear(GL_COLOR_BUFFER_BIT);
	// Intercambiando los buffers de la ventana
	glfwSwapBuffers(window);
	
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		// Limpiando el buffer de color y cuidando todos los eventos de GLFW (como el cierre de la ventana)
		glfwPollEvents();
	}

	// Limpiando los recursos de GLFW (ventana y contexto)
	glfwDestroyWindow(window);
	// Terminando GLFW
	glfwTerminate();
	return 0;
}