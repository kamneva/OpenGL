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


//   иной   вершинный шейдер
auto ps = R"(#version 330 core
	layout(location = 0) in vec3 vertexPosition_modelspace;
	layout(location = 1) in vec2 vertexUV;
	out vec2 UV;
	uniform mat4 MVP;

	void main() {
			gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
			UV = vertexUV;
				}
)";

// фрагментный шейдер
auto ss = R"(
#version 330 core
in vec2 UV;
out vec3 color;
uniform sampler2D myTextureSampler;
void main() {
	color = texture(myTextureSampler, UV).rgb;
})";

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



// загрузка текстуры:
GLuint loadDDS(const char* imagepath) {
	string  header(124, 0);
	std::ifstream is(imagepath, ios::binary);
	if (!is) {
		printf("%s could not be opened. Are you in the right directory ? Don't 				forget to read the FAQ !\n", imagepath);
		return 0;
	}
	string filecode(4, 0);
	is.read(&filecode[0], filecode.size());
	if (filecode != "DDS ") {
		return 0;
	}
	// получить описание поверхности 
	is.read(&header[0], 124);
	unsigned int height = *(unsigned int*)&(header[8]);
	unsigned int width = *(unsigned int*)&(header[12]);
	unsigned int linearSize = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC = *(unsigned int*)&(header[80]);

	// насколько большим   будет буфер, включая все mipmaps?
	unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	// buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
	string buffer(bufsize * sizeof(unsigned char), 0);
	is.read(&buffer[0], buffer.size());
	unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
	unsigned int format;
	switch (fourCC) {
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		return 0;
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (format ==
		GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	/* load the mipmaps */
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) {
		unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
			0, size, &buffer[offset]);

		offset += size;
		width /= 2;
		height /= 2;

		if (width < 1) width = 1;
		if (height < 1) height = 1;
	}

	return textureID;
}


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

	// Компилирование   шейдерной программы
	sh1 sh(ps, ss);
	sh.use();

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	unsigned int programID;
	programID = sh.ID;
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Загружаем текстуру и устанавливаем данные вершин куба
	GLuint Texture = loadDDS("uvtemplate.DDS");
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	static const GLfloat g_vertex_buffer_data[] = {
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	 1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	 1.0f,-1.0f,-1.0f,
	 1.0f, 1.0f,-1.0f,
	 1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	 1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	 1.0f,-1.0f, 1.0f,
	 1.0f, 1.0f, 1.0f,
	 1.0f,-1.0f,-1.0f,
	 1.0f, 1.0f,-1.0f,
	 1.0f,-1.0f,-1.0f,
	 1.0f, 1.0f, 1.0f,
	 1.0f,-1.0f, 1.0f,
	 1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	 1.0f,-1.0f, 1.0f
	};
	// по две UV-  текстурные координаты для каждой вершины.  
	static const GLfloat g_uv_buffer_data[] = {
	0.000059f, 0.000004f,
	0.000103f, 0.336048f,
	0.335973f, 0.335903f,
	1.000023f, 0.000013f,
	0.667979f, 0.335851f,
	0.999958f, 0.336064f,
	0.667979f, 0.335851f,
	0.336024f, 0.671877f,
	0.667969f, 0.671889f,
	1.000023f, 0.000013f,
	0.668104f, 0.000013f,
	0.667979f, 0.335851f,
	0.000059f, 0.000004f,
	0.335973f, 0.335903f,
	0.336098f, 0.000071f,
	0.667979f, 0.335851f,
	0.335973f, 0.335903f,
	0.336024f, 0.671877f,
	1.000004f, 0.671847f,
	0.999958f, 0.336064f,
	0.667979f, 0.335851f,
	0.668104f, 0.000013f,
	0.335973f, 0.335903f,
	0.667979f, 0.335851f,
	0.335973f, 0.335903f,
	0.668104f, 0.000013f,
	0.336098f, 0.000071f,
	0.000103f, 0.336048f,
	0.000004f, 0.671870f,
	0.336024f, 0.671877f,
	0.000103f, 0.336048f,
	0.336024f, 0.671877f,
	0.335973f, 0.335903f,
	0.667969f, 0.671889f,
	1.000004f, 0.671847f,
	0.667979f, 0.335851f
	};
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data),g_uv_buffer_data, GL_STATIC_DRAW);
	glUseProgram(programID); //  … используйте шейдерную программу
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 ProjectionMatrix = glm::perspective(45.f, 4.0f / 3.0f, 0.1f, 1000.0f);
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		ModelMatrix[3][0] = 0;
		ModelMatrix[3][1] = 0;
		ModelMatrix[3][2] = -5; // сдвиг по Z
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

		ModelMatrix = ModelMatrix * RotXMatrix * RotYMatrix;
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
		glm::mat4  ViewMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. 
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            //   offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute.  
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          //   offset
		);
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // проверить не нажата ли клавиша выхода ESC 
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// завершение
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	glfwTerminate();

	return 0;
}
