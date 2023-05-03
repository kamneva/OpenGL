#define _CRT_SECURE_NO_WARNINGS
#include <glad\gl.h>
#include <GLFW\glfw3.h> 

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "gladd.lib")
#include <fstream>
#include <iostream>
using namespace std;

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

// Шейдеры для света:
auto lvs = R"( // вершинный
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
})";

auto lfs = R"(  // фрагментный
 #version 330 core
out vec4 FragColor;

in vec3 Normal;   // передан из вершинного шейдера
in vec3 FragPos; // передан из вершинного шейдера

  uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
void main() {
      // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
      // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
        // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}  
)";

auto vs = R"(  // вершинный шейдер куба
 #version 330 core

layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
  gl_Position = mvp * vec4(aPos, 1.0);
 }
)";

auto fs = R"( // фрагментный  шейдер куба
 #version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0); // делаем белым
}
)";

struct  sh1 {
	unsigned int ID;
	sh1(const char* vs, const char* fs, const char* gs = nullptr) {
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;

		vertexCode = vs;
		fragmentCode = fs;

		// Если путь к геометрическому шейдеру присутствует, то также загружаем и геометрический шейдер
		if (gs != nullptr)
		{
			geometryCode = gs;
		}

		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		// 2. Компилируем шейдеры
		unsigned int vertex, fragment;

		// Вершинный шейдер
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		// Фрагментный шейдер
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		// ..  Продолжение
		   // Если был дан геометрический шейдер, то компилируем его
		unsigned int geometry;
		if (gs != nullptr)
		{
			const char* gShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY");
		}

		// теперь - Шейдерная программа
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		if (gs != nullptr)
			glAttachShader(ID, geometry);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// ..  Продолжение
			// После того, как мы связали шейдеры с нашей программой,
			//  удаляем их, т.к. они нам больше не нужны
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (gs != nullptr)
			glDeleteShader(geometry);
	}

	void use() const
	{    // применение программы
		glUseProgram(ID);
	}
	// проверка на ошибки
	void checkCompileErrors(GLuint shader, std::string type) {
		GLint success;         GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n" << std::endl;
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n" << std::endl;
			}
		}
	}

};  // конец     class


// главная функция
int main(void)
{

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(1024, 768, "Tutorial 0 - Keyboard and Mouse", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);
	glViewport(0, 0, 1024, 768);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	// Компилирование   шейдерной программы
	// инициализация
	sh1  lightingShader(lvs, lfs);
	sh1  lightCubeShader(vs, fs);
	// атрибуты вершин  // layout (location = 0) 
	

	float vertex[] = {
	-1.0f,-1.0f,-1.0f, -1,0,0,
	-1.0f,-1.0f, 1.0f, -1,0,0,
	-1.0f, 1.0f, 1.0f, -1,0,0,

	 1.0f, 1.0f,-1.0f, 0,0,-1,
	-1.0f,-1.0f,-1.0f, 0,0,-1,
	-1.0f, 1.0f,-1.0f, 0,0,-1,

	 1.0f,-1.0f, 1.0f,  0,-1,0,
	-1.0f,-1.0f,-1.0f,  0,-1,0,
	 1.0f,-1.0f,-1.0f,  0,-1,0,

	 1.0f, 1.0f,-1.0f,  0,0,-1,
	 1.0f,-1.0f,-1.0f,  0,0,-1,
	-1.0f,-1.0f,-1.0f,  0,0,-1,

	-1.0f,-1.0f,-1.0f, -1,0,0,
	-1.0f, 1.0f, 1.0f, -1,0,0,
	-1.0f, 1.0f,-1.0f, -1,0,0,

	 1.0f,-1.0f, 1.0f, 0,-1,0,
	-1.0f,-1.0f, 1.0f, 0,-1,0,
	-1.0f,-1.0f,-1.0f, 0,-1,0,

	-1.0f, 1.0f, 1.0f, 0,0,1,
	-1.0f,-1.0f, 1.0f, 0,0,1,
	 1.0f,-1.0f, 1.0f, 0,0,1,

	 1.0f, 1.0f, 1.0f, 1,0,0,
	 1.0f,-1.0f,-1.0f, 1,0,0,
	 1.0f, 1.0f,-1.0f, 1,0,0,

	 1.0f,-1.0f,-1.0f, 1,0,0,
	 1.0f, 1.0f, 1.0f, 1,0,0,
	 1.0f,-1.0f, 1.0f, 1,0,0,

	 1.0f, 1.0f, 1.0f, 0,1,0,
	 1.0f, 1.0f,-1.0f, 0,1,0,
	-1.0f, 1.0f,-1.0f, 0,1,0,

	 1.0f, 1.0f, 1.0f, 0,1,0,
	-1.0f, 1.0f,-1.0f, 0,1,0,
	-1.0f, 1.0f, 1.0f, 0,1,0,

	 1.0f, 1.0f, 1.0f, 0,0,1,
	-1.0f, 1.0f, 1.0f, 0,0,1,
	 1.0f,-1.0f, 1.0f, 0,0,1,
	};

	GLuint VBO;
	glGenBuffers(1, &VBO);
	
	//генерация cubeVAO
	GLuint cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	// 1. Привязываем VAO
	glBindVertexArray(cubeVAO);
	// 2. Копируем наш массив вершин в буфер для OpenGL
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	// 3. Устанавливаем указатели на вершинные атрибуты 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// атрибуты нормалей // layout (location = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//4. Отвязываем VAO
	glBindVertexArray(0);

	//генерация lightCubeVAO
	GLuint lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);
	// 2. Копируем наш массив вершин в буфер для OpenGL
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	// 3. Устанавливаем указатели на вершинные атрибуты 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// атрибуты нормалей // layout (location = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//4. Отвязываем VAO
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, -0.9f, -2.0f); // позиция лампы

	while (!glfwWindowShouldClose(window)) {    // render цикл
		
		glClearColor(0.1f, 0.1f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	     // рисуем куб, как образ лампы
			glm::mat4 projection = glm::perspective(45.f, 4.0f / 3.0f, 0.1f, 1000.0f);
			glm::mat4 view = glm::mat4(1.0);
			lightCubeShader.use(); // включаем программу рисования лампы
			glm::mat4    model = glm::mat4(1.0f);
			model = glm::translate(model, lightPos);
			model = glm::scale(model, glm::vec3(0.2f)); // уменьшаем размер куба
			glm::mat4 mvp = projection * view * model;

			glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.ID, "mvp"),1, GL_FALSE, &mvp[0][0]);


			glBindVertexArray(lightCubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);


			// теперь рисуем освещенный куб – включаем программу освещения куба
			lightingShader.use();
			
			glUniform3f(glGetUniformLocation(lightingShader.ID, "objectColor"),1.0f, 0.5f, 0.31f);
			glUniform3f(glGetUniformLocation(lightingShader.ID, "lightColor"), 1, 1, 1);
			glUniform3fv(glGetUniformLocation(lightingShader.ID, "lightPos"),1, &lightPos[0]);
			glUniform3f(glGetUniformLocation(lightingShader.ID, "viewPos"), 0, 0, 3);
			glUniformMatrix4fv(glGetUniformLocation(lightingShader.ID, "projection"),1, GL_FALSE, &projection[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(lightingShader.ID, "view"),1, GL_FALSE, &view[0][0]);		

			model[3][0] = 0;
			model[3][1] = 0;
			model[3][2] = -8; // сдвиг по Z
			static int mouseState = 0, xx = 0, yy = 0, xprev = 0, yprev = 0;
			static float dx = glm::radians((float)1024 / 2.), dy = glm::radians((float)768 / 2.);
			glfwSetCursorPosCallback(
				window,
				[](auto* window, double x, double y)
				{
					if (mouseState) {
						xx = x;
						dx += glm::radians((float)(xx - xprev));

						yy = y;
						dy += glm::radians((float)(yy - yprev));
					}
					xprev = x; yprev = y;
				}
			);
			glm::mat4 RotXMatrix = glm::mat4(1.0);
			RotXMatrix[1][1] = cos(dy);
			RotXMatrix[1][2] = -sin(dy);
			RotXMatrix[2][1] = sin(dy);
			RotXMatrix[2][2] = cos(dy);
			glm::mat4 RotYMatrix = glm::mat4(1.0);

			RotYMatrix[0][0] = cos(dx);
			RotYMatrix[0][2] = sin(dx);
			RotYMatrix[2][0] = -sin(dx);
			RotYMatrix[2][2] = cos(dx);

			model = model * RotXMatrix * RotYMatrix;
			glfwSetMouseButtonCallback(
				window,
				[](auto* window, int button, int action, int mods)
				{
					if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
						mouseState = 1;
					else
						mouseState = 0;
				}
			);
			model = glm::scale(model, glm::vec3(8.0f)); // уменьшаем размер куба
			glUniformMatrix4fv(glGetUniformLocation(lightingShader.ID, "model"), 1, GL_FALSE, &model[0][0]);

			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36); // рисуем освещенный куб
			glBindVertexArray(0);

			glfwSwapBuffers(window);
			glfwPollEvents();

	}
	// не забываем освободить ресурсы

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &VBO);

	return 0;
}
