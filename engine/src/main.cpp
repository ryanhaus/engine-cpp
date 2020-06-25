#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

struct ModelData
{
	std::vector<float> position_vertices;
	std::vector<int> position_indices;
	std::vector<float> normal_vertices;
	std::vector<int> normal_indices;
};

enum ModelType
{
	OBJ
};

struct Model
{
	glm::vec3 position;
	glm::vec3 orientation;
	glm::vec3 size;

	glm::vec3 color;

	ModelData md;

	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
	unsigned int nvbo;
	unsigned int nibo;
};

Model createModelFromFile(std::string file, ModelType fileType);
ModelData parseObj(std::string file);

void DrawCallModel(Model model, unsigned int program, glm::mat4 view, glm::mat4 projection);

void fbSizeCallback(GLFWwindow* window, int width, int height);

int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(1080, 720, "Game", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(window, fbSizeCallback);

	glewInit();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	{
		unsigned int vertexShader;
		{
			std::stringstream vertexStream;
			std::ifstream vertexShaderF("res/shaders/vert.shader", std::ios_base::binary);
			vertexStream << vertexShaderF.rdbuf();
			std::string vertexShaderSourceStr = vertexStream.str();
			const char* vertexShaderSource = vertexShaderSourceStr.c_str();
			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			glCompileShader(vertexShader);
			vertexShaderF.close();
		}

#ifdef DEBUG
		GLint vertex_compiled;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_compiled);
		if (vertex_compiled != GL_TRUE)
		{
			GLsizei log_length = 0;
			GLchar message[1024];
			glGetShaderInfoLog(vertexShader, 1024, &log_length, message);
			std::cout << "VERTEX ERRORS: " << std::endl << message << std::endl;
		}
#endif

		unsigned int fragmentShader;
		{
			std::stringstream fragmentStream;
			std::ifstream fragmentShaderF("res/shaders/frag.shader", std::ios_base::binary);
			fragmentStream << fragmentShaderF.rdbuf();
			std::string fragmentShaderSourceStr = fragmentStream.str();
			const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
			glCompileShader(fragmentShader);
			fragmentShaderF.close();
		}

#ifdef DEBUG
		GLint fragment_compiled;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragment_compiled);
		if (fragment_compiled != GL_TRUE)
		{
			GLsizei log_length = 0;
			GLchar message[1024];
			glGetShaderInfoLog(fragmentShader, 1024, &log_length, message);
			std::cout << "FRAGMENT ERRORS: " << std::endl << message << std::endl;
		}
#endif

		unsigned int program;
		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		glUseProgram(program);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		glm::vec3 cameraPos(0, 0, 10);
		glm::vec3 cameraRot(0, 0, 0);
		glm::vec3 lightPosition(6, -12, 18);

		float deltaSeconds = 0.0f;

		double previousCursorX, previousCursorY;
		glfwGetCursorPos(window, &previousCursorX, &previousCursorY);
		double throwMouseToX, throwMouseToY;

		bool lastFrameMouseRtClick = false;

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		Model dragon = createModelFromFile("res/3d/dragon.obj", OBJ);
		dragon.position.y = -5.0f;
		dragon.color = glm::vec3(0.25f, 0.25f, 0.9f);

		Model baseplate = createModelFromFile("res/3d/cube.obj", OBJ);
		baseplate.position.y = -5.5f;
		baseplate.size = glm::vec3(7.5f, 0.5f, 7.5f);

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			auto s = std::chrono::high_resolution_clock::now();

			{
				double camDiv = 750;
				double cursorX, cursorY;
				glfwGetCursorPos(window, &cursorX, &cursorY);

				double deltaCursorX = cursorX - previousCursorX;
				double deltaCursorY = cursorY - previousCursorY;

				previousCursorX = cursorX;
				previousCursorY = cursorY;

				if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) && !lastFrameMouseRtClick)
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					throwMouseToX = cursorX;
					throwMouseToY = cursorY;
				}

				if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) && lastFrameMouseRtClick)
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					glfwSetCursorPos(window, throwMouseToX, throwMouseToY);
				}

				if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
					cameraRot += glm::vec3(deltaCursorY / camDiv, deltaCursorX / camDiv, 0);

				lastFrameMouseRtClick = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

				if (cameraRot.x > asin(1)) cameraRot.x = (float)asin(1);
				if (cameraRot.x < asin(-1)) cameraRot.x = (float)asin(-1);

				float speed = -8.0f;

				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
					cameraPos += glm::vec3(cos(cameraRot.y + asin(1)) * deltaSeconds * speed, 0, sin(cameraRot.y + asin(1)) * deltaSeconds * speed);
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					cameraPos += glm::vec3(cos(cameraRot.y + asin(1)) * deltaSeconds * -speed, 0, sin(cameraRot.y + asin(1)) * deltaSeconds * -speed);
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
					cameraPos += glm::vec3(cos(cameraRot.y) * deltaSeconds  * speed, 0, sin(cameraRot.y) * deltaSeconds * speed);
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					cameraPos += glm::vec3(cos(cameraRot.y) * deltaSeconds * -speed, 0, sin(cameraRot.y) * deltaSeconds * -speed);
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(program);

			glm::mat4 view = glm::mat4(1.0f);
			view = glm::rotate(view, cameraRot.x, glm::vec3(1, 0, 0));
			view = glm::rotate(view, cameraRot.y, glm::vec3(0, 1, 0));
			view = glm::rotate(view, cameraRot.z, glm::vec3(0, 0, 1));
			view = glm::translate(view, cameraPos * glm::vec3(-1, -1, -1));

			int width, height;
			glfwGetWindowSize(window, &width, &height);
			glm::mat4 projection = glm::perspective(70.0f, (float)width / (float)height, 0.1f, 1000.0f);

			glUniform3fv(glGetUniformLocation(program, "vPos"), 1, &cameraRot[0]);
			glUniform3fv(glGetUniformLocation(program, "lPos"), 1, &lightPosition[0]);

			{
				dragon.orientation.y += deltaSeconds / 4.0f;
				baseplate.orientation.y += deltaSeconds / 4.0f;
			}

			DrawCallModel(dragon, program, view, projection);
			DrawCallModel(baseplate, program, view, projection);

			glfwSwapBuffers(window);

			auto f = std::chrono::high_resolution_clock::now();

			deltaSeconds = float(std::chrono::duration_cast<std::chrono::milliseconds>(s-f).count()) / -1000.0f * (float)asin(1);
		}

		glDeleteProgram(program);
	}

	glfwTerminate();
	return 0;
}

ModelData parseObj(std::string name)
{
	std::vector<float> position_vertices;
	std::vector<int> position_indices;
	std::vector<float> normal_vertices;
	std::vector<int> normal_indices;

	std::ifstream stream(name);
	std::string line;
	while (std::getline(stream, line))
	{
		std::string mode = line.substr(0, line.find(" "));
		line = line.substr(line.find(" ") + 1);
		for (int i = 0; i < 3; i++)
		{
			if (mode == "v") position_vertices.push_back(std::stof(line.substr(0, line.find(" "))));
			if (mode == "vt") if (i == 2) break;
			if (mode == "vn") normal_vertices.push_back(std::stof(line.substr(0, line.find(" "))));
			if (mode == "f")
			{
				std::string lineChunk = line.substr(0, line.find(" "));
				for (int j = 0; j < 3; j++)
				{
					std::string current = lineChunk.substr(0, lineChunk.find("/"));

					if (j == 0) position_indices.push_back(std::stoi(current) - 1);
					if (j == 2) normal_indices.push_back(std::stoi(current) - 1);

					lineChunk = lineChunk.substr(lineChunk.find("/") + 1);
				}
			}
			line = line.substr(line.find(" ") + 1);
		}
	}
	stream.close();

	return { position_vertices, position_indices, normal_vertices, normal_indices };
}

Model createModelFromFile(std::string file, ModelType fileType)
{
	glm::vec3 position(0, 0, 0);
	glm::vec3 orientation(0, 0, 0);
	glm::vec3 size(1, 1, 1);

	glm::vec3 color(1, 1, 1);

	ModelData model;
	switch (fileType)
	{
	case OBJ:
		model = parseObj(file);
		break;
	};

	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.position_vertices.size(), &model.position_vertices[0], GL_STATIC_DRAW);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * model.position_indices.size(), &model.position_indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int nvbo;
	glGenBuffers(1, &nvbo);
	glBindBuffer(GL_ARRAY_BUFFER, nvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.normal_vertices.size(), &model.normal_vertices[0], GL_STATIC_DRAW);

	unsigned int nibo;
	glGenBuffers(1, &nibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model.normal_indices.size(), &model.normal_indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(2);

	return {
		position, orientation, size,
		color,
		model,
		vao, vbo, ibo, nvbo, nibo
	};
}

void DrawCallModel(Model model, unsigned int program, glm::mat4 view, glm::mat4 projection)
{
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, model.position);
	modelMat = glm::rotate(modelMat, model.orientation.x, glm::vec3(1, 0, 0));
	modelMat = glm::rotate(modelMat, model.orientation.y, glm::vec3(0, 1, 0));
	modelMat = glm::rotate(modelMat, model.orientation.z, glm::vec3(0, 0, 1));
	modelMat = glm::scale(modelMat, model.size);
	glm::mat4 mvp = projection * view * modelMat;

	glUniformMatrix4fv(glGetUniformLocation(program, "mvp"), 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMat"), 1, GL_FALSE, &modelMat[0][0]);
	glUniform3fv(glGetUniformLocation(program, "objectColor"), 1, &model.color[0]);

	glBindVertexArray(model.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo);

	glDrawElements(GL_TRIANGLES, model.md.position_indices.size(), GL_UNSIGNED_INT, nullptr);
}

void fbSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}