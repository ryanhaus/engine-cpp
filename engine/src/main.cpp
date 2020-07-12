#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <time.h>

#define DEBUG

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

std::map<int, Model> render;
int nextRenderId = 0;

glm::vec3 cameraPos(0, 0, 0);
glm::vec3 cameraRot(0, 0, 0);
glm::vec3 lightPosition(-0.5f, 1.0f, -1.0f);

GLFWwindow* window;

std::map<std::string, std::array<float, 6>> players;
std::vector<std::pair<std::string, std::array<float, 6>>> newPlayers;

extern "C"
{
	#include <luaFunctions.h>
}

#include <network.h>

int main()
{
	// ShowWindow(GetConsoleWindow(), SW_HIDE);
	glfwInit();

	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(1080, 720, "Game", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(window, fbSizeCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);

	glewInit();

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

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

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		Lua = luaL_newstate();
		luaL_openlibs(Lua);

		registerLua(Lua);

		luaL_dofile(Lua, "res/lua/main.lua");

		std::thread recHeadT(socketReceiveHeads, &s, &players, &newPlayers);

		srand(time(NULL));
		std::string localName = std::to_string(rand()).substr(0, 12);

		lua_getglobal(Lua, "Game");
		lua_getfield(Lua, -1, "Local");
		lua_getfield(Lua, -1, "LocalPlayer");
		lua_pushstring(Lua, localName.c_str());
		lua_setfield(Lua, -2, "Name");

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(program);

			lua_settop(Lua, 0);
			lua_getglobal(Lua, "Game");
			lua_getfield(Lua, -1, "Camera");
			lua_getfield(Lua, -1, "Position");
			lua_getfield(Lua, -1, "x");
			lua_getfield(Lua, -2, "y");
			lua_getfield(Lua, -3, "z");
			cameraPos = glm::vec3(lua_tonumber(Lua, -3), lua_tonumber(Lua, -2), lua_tonumber(Lua, -1));

			lua_settop(Lua, 2);
			lua_getfield(Lua, -1, "Orientation");
			lua_getfield(Lua, -1, "x");
			lua_getfield(Lua, -2, "y");
			lua_getfield(Lua, -3, "z");
			cameraRot = glm::vec3(lua_tonumber(Lua, -3), lua_tonumber(Lua, -2), lua_tonumber(Lua, -1));

			lua_settop(Lua, 0);

			glm::mat4 view = glm::mat4(1.0f);
			view = glm::rotate(view, cameraRot.x, glm::vec3(1, 0, 0));
			view = glm::rotate(view, cameraRot.y, glm::vec3(0, 1, 0));
			view = glm::rotate(view, cameraRot.z, glm::vec3(0, 0, 1));
			view = glm::translate(view, cameraPos * glm::vec3(-1, -1, -1));

			int width, height;
			glfwGetWindowSize(window, &width, &height);

			if (width == 0 || height == 0)
			{
				width = 1;
				height = 1;
			}

			glm::mat4 projection = glm::perspective(70.0f, (float)width / (float)height, 0.1f, 1000.0f);

			glUniform3fv(glGetUniformLocation(program, "vPos"), 1, &cameraRot[0]);
			glUniform3fv(glGetUniformLocation(program, "lPos"), 1, &lightPosition[0]);

			{
				int j = 0;
				std::map<std::string, std::array<float, 6>>::iterator it;
				lua_settop(Lua, 0);
				lua_newtable(Lua);
				for (it = players.begin(); it != players.end(); it++)
				{
					lua_newtable(Lua);
					lua_pushstring(Lua, it->first.c_str());
					lua_setfield(Lua, -2, "Name");

					lua_newtable(Lua);
					lua_newtable(Lua);
					lua_pushnumber(Lua, it->second[0]);
					lua_setfield(Lua, -2, "x");
					lua_pushnumber(Lua, it->second[1]);
					lua_setfield(Lua, -2, "y");
					lua_pushnumber(Lua, it->second[2]);
					lua_setfield(Lua, -2, "z");
					lua_setfield(Lua, -2, "Position");

					lua_newtable(Lua);
					lua_pushnumber(Lua, it->second[3]);
					lua_setfield(Lua, -2, "x");
					lua_pushnumber(Lua, it->second[4]);
					lua_setfield(Lua, -2, "y");
					lua_pushnumber(Lua, it->second[5]);
					lua_setfield(Lua, -2, "z");
					lua_setfield(Lua, -2, "Orientation");
					lua_setfield(Lua, -2, "head");

					int r = luaL_ref(Lua, LUA_REGISTRYINDEX);

					lua_pushnumber(Lua, j++);
					lua_rawgeti(Lua, LUA_REGISTRYINDEX, r);
					lua_settable(Lua, -3);
					luaL_unref(Lua, LUA_REGISTRYINDEX, r);
				}

				int r = luaL_ref(Lua, LUA_REGISTRYINDEX);

				lua_getglobal(Lua, "Game");
				lua_rawgeti(Lua, LUA_REGISTRYINDEX, r);
				lua_setfield(Lua, -2, "Players");

				luaL_unref(Lua, LUA_REGISTRYINDEX, r);
			}
			
			tickCallback();
			socketSendHead();

			for (int i = 0; i < newPlayers.size(); i++)
			{
				auto player = newPlayers[i];
				if (player.first == localName)
					continue;
				for (int functionRef : luaEventFunctions["PlayerJoin"])
				{
					if (lua_gettop(Lua) > 0)
						lua_settop(Lua, 0);
					lua_rawgeti(Lua, LUA_REGISTRYINDEX, functionRef);

					lua_newtable(Lua);
					lua_pushstring(Lua, player.first.c_str());
					lua_setfield(Lua, -2, "Name");

					lua_newtable(Lua);
					lua_newtable(Lua);
					lua_pushnumber(Lua, player.second[0]);
					lua_setfield(Lua, -2, "x");
					lua_pushnumber(Lua, player.second[1]);
					lua_setfield(Lua, -2, "y");
					lua_pushnumber(Lua, player.second[2]);
					lua_setfield(Lua, -2, "z");
					lua_setfield(Lua, -2, "Position");

					lua_newtable(Lua);
					lua_pushnumber(Lua, player.second[3]);
					lua_setfield(Lua, -2, "x");
					lua_pushnumber(Lua, player.second[4]);
					lua_setfield(Lua, -2, "y");
					lua_pushnumber(Lua, player.second[5]);
					lua_setfield(Lua, -2, "z");
					lua_setfield(Lua, -2, "Orientation");
					lua_setfield(Lua, -2, "head");
					
					lua_call(Lua, 1, 0);
				}
			}

			newPlayers.clear();

			{
				std::map<int, Model>::iterator it;
				for (it = render.begin(); it != render.end(); it++)
				{
					lua_settop(Lua, 0);
					lua_rawgeti(Lua, LUA_REGISTRYINDEX, it->first);
					lua_getfield(Lua, -1, "Position");
					lua_getfield(Lua, -1, "x");
					lua_getfield(Lua, -2, "y");
					lua_getfield(Lua, -3, "z");
					it->second.position = glm::vec3(lua_tonumber(Lua, -3), lua_tonumber(Lua, -2), lua_tonumber(Lua, -1));

					lua_settop(Lua, 1);
					lua_getfield(Lua, -1, "Orientation");
					lua_getfield(Lua, -1, "x");
					lua_getfield(Lua, -2, "y");
					lua_getfield(Lua, -3, "z");
					it->second.orientation = glm::vec3(lua_tonumber(Lua, -3) * asin(1) / 90, lua_tonumber(Lua, -2) * asin(1) / 90, lua_tonumber(Lua, -1) * asin(1) / 90);

					lua_settop(Lua, 1);
					lua_getfield(Lua, -1, "Size");
					lua_getfield(Lua, -1, "x");
					lua_getfield(Lua, -2, "y");
					lua_getfield(Lua, -3, "z");
					it->second.size = glm::vec3(lua_tonumber(Lua, -3), lua_tonumber(Lua, -2), lua_tonumber(Lua, -1));

					lua_settop(Lua, 1);
					lua_getfield(Lua, -1, "Color");
					lua_getfield(Lua, -1, "r");
					lua_getfield(Lua, -2, "g");
					lua_getfield(Lua, -3, "b");
					it->second.color = glm::vec3(lua_tonumber(Lua, -3), lua_tonumber(Lua, -2), lua_tonumber(Lua, -1));

					DrawCallModel(it->second, program, view, projection);
				}
			}

			glfwSwapBuffers(window);
		}

		glfwTerminate();
		recHeadT.detach();
		lua_close(Lua);
		glDeleteProgram(program);
	}

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

	glm::mat4 normalMat = glm::mat4(1.0f);
	normalMat = glm::rotate(normalMat, model.orientation.x, glm::vec3(1, 0, 0));
	normalMat = glm::rotate(normalMat, model.orientation.y, glm::vec3(0, 1, 0));
	normalMat = glm::rotate(normalMat, model.orientation.z, glm::vec3(0, 0, 1));

	glUniformMatrix4fv(glGetUniformLocation(program, "mvp"), 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMat"), 1, GL_FALSE, &modelMat[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "normalMat"), 1, GL_FALSE, &normalMat[0][0]);
	glUniform3fv(glGetUniformLocation(program, "objectColor"), 1, &model.color[0]);

	glBindVertexArray(model.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo);

	glDrawElements(GL_TRIANGLES, model.md.position_indices.size(), GL_UNSIGNED_INT, nullptr);
}

void fbSizeCallback(GLFWwindow* window, int width, int height)
{
	if(width != 0 && height != 0)
		glViewport(0, 0, width, height);
}