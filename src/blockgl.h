#ifndef BLOCKGL_H
#define BLOCKGL_H

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

//BGL for short BlockGL
// Variable constants
const unsigned int		BGL_ChunkSize			= 16;
const unsigned int		BGL_LoadRadius			= 6;
const float				BGL_MouseSensitivity	= 0.05f;
const unsigned int		BGL_TextureSize			= 16;
const unsigned int		BGL_BlockCount			= 4;
const unsigned int		BGL_TextureCount		= 4;

// Calculated constants
const unsigned int BGL_LoadSize = BGL_LoadRadius * 2 + 1;
//const unsigned int BGL_MaxFaces = (BGL_ChunkSize * BGL_ChunkSize * BGL_ChunkSize + 1) / 2; //Max number of possible faces in a chunk
const unsigned int BGL_MaxFaces = (BGL_ChunkSize * BGL_ChunkSize * BGL_ChunkSize) * 6; //Max number of possible faces in a chunk // <----- Temporary

static const char* vertex_shader_text =
		"#version 330 core\n"
				"layout (location = 0) in vec3 position;\n"
				"layout (location = 1) in vec3 texCoord;\n"
				"layout (location = 2) in vec3 normal;\n"
				"\n"
				"out vec3 TexCoord;\n"
				"out vec3 Normal;\n"
				"out vec3 FragPos;\n"
				"out float Visibility;\n"
				"//out vec3 toCameraVector;\n"
				"\n"
				"uniform mat4 view;\n"
				"uniform mat4 projection;\n"
				"\n"
				"const float gradient = 20;\n"
				"const float density = 0.011;\n"
				"\n"
				"void main() {\n"
				"\tvec4 positionRelativeToCam = view * vec4(position, 1.0f);\n"
				"\tgl_Position = projection * positionRelativeToCam;\n"
				"\tTexCoord = texCoord;\n"
				"\tFragPos = position;\n"
				"\tNormal = normal;\n"
				"\t\n"
				"\tfloat distance = length(positionRelativeToCam.xyz);\n"
				"\tVisibility = exp(-pow((distance * density), gradient));\n"
				"\tVisibility = clamp(Visibility, 0.0, 1.0);\n"
				"}";

static const char* fragment_shader_text =
		"#version 330 core\n"
				"\n"
				"in vec3 TexCoord;\n"
				"in vec3 Normal;\n"
				"in vec3 FragPos;\n"
				"in float Visibility;\n"
				"//in vec3 toCameraVector;\n"
				"\n"
				"out vec4 color;\n"
				"\n"
				"uniform sampler2DArray textureArray;\n"
				"uniform vec3 lightPos;\n"
				"uniform vec3 lightColor;\n"
				"uniform vec3 fogColor;\n"
				"\n"
				"void main() {\n"
				"    // Ambient\n"
				"    float ambientStrength = 0.1f;\n"
				"    vec3 ambient = ambientStrength * lightColor;\n"
				"  \t\n"
				"    // Diffuse \n"
				"    vec3 norm = normalize(Normal);\n"
				"    vec3 lightDir = normalize(lightPos - FragPos);\n"
				"    float diff = max(dot(norm, lightDir), 0.0);\n"
				"    vec3 diffuse = diff * lightColor;\n"
				"\t\n"
				"\tcolor = vec4(ambient + diffuse, 1.0) * texture(textureArray, TexCoord);\n"
				"\tcolor = mix(vec4(fogColor, 1.0), color, Visibility);\t\n"
				"}";

void insertTexture(GLubyte* texture_array, GLubyte* texture, unsigned int index) {
	for (int x = 0; x < BGL_TextureSize; x++) {
		for (int y = 0; y < BGL_TextureSize; y++) {
			int rindex = ((x + (y * BGL_TextureSize)) * 4);
			int tindex = rindex + (BGL_TextureSize * BGL_TextureSize * 4 * index);
			texture_array[tindex + 0] = texture[rindex + 0];
			texture_array[tindex + 1] = texture[rindex + 1];
			texture_array[tindex + 2] = texture[rindex + 2];
			texture_array[tindex + 3] = texture[rindex + 3];
		}
	}
}

double getDelta(double* t1) {
	double t2 = glfwGetTime();
	double elapsedTime = t2 - *t1;
	*t1 = t2;

	return elapsedTime;
}

void initTime(double* t1) {
	*t1 = glfwGetTime();
}

struct Vec3i {
	int x, y, z;
};

void set(struct Vec3i* vec, int x, int y, int z) {
	vec->x = x;
	vec->y = y;
	vec->z = z;
}

int modulo(int x,int N){
	return (x % N + N) %N;
}

struct Vec3i toChunkPos(const vec3 v) {
	struct Vec3i chunkPos;
	chunkPos.x = (int)floor(v[0] / (float)BGL_ChunkSize);
	chunkPos.y = (int)floor(v[1] / (float)BGL_ChunkSize);
	chunkPos.z = (int)floor(v[2] / (float)BGL_ChunkSize);
	return chunkPos;
}

struct Vec3i toMemoryPos(const struct Vec3i v) {
	struct Vec3i pos;
	pos.x = modulo(v.x, BGL_LoadSize);
	pos.y = modulo(v.y, BGL_LoadSize);
	pos.z = modulo(v.z, BGL_LoadSize);
	return pos;
}

struct Block {
	unsigned char id;
};

struct Chunk {
	struct Block blocks[BGL_ChunkSize][BGL_ChunkSize][BGL_ChunkSize];
	struct Vec3i position;
	bool isGenerated;
	bool noMesh;
	GLuint VAO, VBO, EBO;
	GLuint indicesSize;
};

struct World {
	struct Chunk chunks[BGL_LoadSize][BGL_LoadSize][BGL_LoadSize];
	vec3 skyColor;
	vec3 lightColor;
	vec3 lightPos;
};

struct Camera {
	vec3 position;
	vec3 rotation;
	double mouseX, mouseY;
};

inline double toDegree(double radians) {
	return radians * (180.0 / M_PI);
}

double toRadian(double degrees) {
	return degrees / 180.0 * M_PI;
}

void getCameraMatrix(struct Camera* camera, mat4x4* mat) {
	mat4x4_rotate(*mat, *mat, 1, 0, 0, toRadian(camera->rotation[0]));
	mat4x4_rotate(*mat, *mat, 0, 1, 0, toRadian(camera->rotation[1]));
	mat4x4_rotate(*mat, *mat, 0, 0, 1, toRadian(camera->rotation[2]));
	mat4x4_translate_in_place(*mat, -camera->position[0], -camera->position[1], -camera->position[2]);
}

void handleCameraInput(struct Camera* cam, GLFWwindow* window, const double DT) {
	//Mouse
	double xpos, ypos;
	double xpos1, ypos1;
	glfwGetCursorPos(window, &xpos1, &ypos1);

	xpos = xpos1 - cam->mouseX;
	ypos = ypos1 - cam->mouseY;
	cam->mouseX = xpos1;
	cam->mouseY = ypos1;

	cam->rotation[1] += xpos * BGL_MouseSensitivity;
	cam->rotation[0] += ypos * BGL_MouseSensitivity;

	float forward = 0;
	float backwards = 0;
	float right = 0;
	float left = 0;
	float up = 0;
	float down = 0;

	float speed = 10;
	float speed_boost = 35;

	//Keyboard
	int state_w = glfwGetKey(window, GLFW_KEY_W);
	if (state_w == GLFW_PRESS) {
		forward = 1;
	}

	int state_s = glfwGetKey(window, GLFW_KEY_S);
	if (state_s == GLFW_PRESS) {
		backwards = 1;
	}

	int state_d = glfwGetKey(window, GLFW_KEY_D);
	if (state_d == GLFW_PRESS) {
		right = 1;
	}

	int state_a = glfwGetKey(window, GLFW_KEY_A);
	if (state_a == GLFW_PRESS) {
		left = 1;
	}

	int state_space = glfwGetKey(window, GLFW_KEY_SPACE);
	if (state_space == GLFW_PRESS) {
		up = 1;
	}

	int state_shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
	if (state_shift == GLFW_PRESS) {
		down = 1;
	}

	int state_ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
	if (state_ctrl == GLFW_PRESS) {
		speed = speed_boost;
	}

	//Limit pitch
	if (cam->rotation[0] < -90) {
		cam->rotation[0] = -90;
	}

	if (cam->rotation[0] > 90) {
		cam->rotation[0] = 90;
	}

	//Handle input
	float rotY = toRadian(cam->rotation[1]);
	cam->position[0] += forward * sin(rotY) * DT * speed;
	cam->position[2] -= forward * cos(rotY) * DT * speed;

	cam->position[0] -= backwards * sin(rotY) * DT * speed;
	cam->position[2] += backwards * cos(rotY) * DT * speed;

	cam->position[0] += right * sin(rotY + toRadian(90.0)) * DT * speed;
	cam->position[2] -= right * cos(rotY + toRadian(90.0)) * DT * speed;

	cam->position[0] -= left * sin(rotY + toRadian(90.0)) * DT * speed;
	cam->position[2] += left * cos(rotY + toRadian(90.0)) * DT * speed;

	cam->position[1] += up * DT * speed;

	cam->position[1] -= down * DT * speed;
}

void buildShader(GLuint* program, const char* vertSource, const char* fragSource) {
	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
	}
	// link shaders
	int shaderProgram = glCreateProgram();
	*program = shaderProgram;
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void initChunk(struct Chunk* chunk) {
	//VAO
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	//Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//TexCoord attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); //Unbind VAO

	chunk->VAO = VAO;
	chunk->VBO = VBO;
	chunk->EBO = EBO;
}

void deinitChunk(struct Chunk* chunk) {
	glDeleteVertexArrays(1, &chunk->VAO);
	glDeleteBuffers(1, &chunk->VBO);
	glDeleteBuffers(1, &chunk->EBO);
}

static const GLfloat cube_vertices[] = {
		// RIGHT
		0.5, 0.5, 0.5,
		0.5, 0.5, -0.5,
		0.5, -0.5, 0.5,
		0.5, -0.5, -0.5,

		// LEFT
		-0.5, 0.5, -0.5,
		-0.5, 0.5, 0.5,
		-0.5, -0.5, -0.5,
		-0.5, -0.5, 0.5,

		// TOP
		-0.5, 0.5, -0.5,
		0.5, 0.5, -0.5,
		-0.5, 0.5, 0.5,
		0.5, 0.5, 0.5,

		// BOTTOM
		0.5, -0.5, -0.5,
		-0.5, -0.5, -0.5,
		0.5, -0.5, 0.5,
		-0.5, -0.5, 0.5,

		// Front
		-0.5, 0.5, 0.5,
		0.5, 0.5, 0.5,
		-0.5, -0.5, 0.5,
		0.5, -0.5, 0.5,

		// BACK
		0.5,  0.5, -0.5,
		-0.5,  0.5, -0.5,
		0.5, -0.5, -0.5,
		-0.5, -0.5, -0.5,
};

static const GLfloat cube_texture[] = {
		0,0,
		1,0,
		0,1,
		1,1,
};

static const GLfloat cube_normals[] = {
		1, 0, 0,
		-1, 0, 0,
		0, 1, 0,
		0, -1, 0,
		0, 0, 1,
		0, 0, -1,
};

void defineBlockTexture(GLuint array[BGL_BlockCount][6], GLuint id, GLuint right, GLuint left, GLuint top, GLuint bottom, GLuint front, GLuint back) {
	array[id][0] = right;
	array[id][1] = left;
	array[id][2] = top;
	array[id][3] = bottom;
	array[id][4] = front;
	array[id][5] = back;
}

static GLuint block_textureIds[BGL_BlockCount][6];

// Returns false if the neighbour doesn't exist within the chunk.
bool checkNeighbour(struct Chunk* chunk, unsigned char* id, int x, int y, int z) {
	if(x >= 0 && x < BGL_ChunkSize && y >= 0 && y < BGL_ChunkSize && z >= 0 && z < BGL_ChunkSize) {
		*id = chunk->blocks[x][y][z].id;
		return true;
	}
	else {
		return false;
	}
}

void generateMesh(struct Chunk* chunk) {
	//printf("Generating mesh\n");
	// Create buffers with the maximum necessary size
	GLfloat vertices[BGL_MaxFaces * 4 * 9]; // 4 corners and 9 attributes for each vertex. xyz texX texY texId normX normY normZ
	GLuint indices[BGL_MaxFaces * 6]; // 6 indices to make a square face
	unsigned int verticesSize = 0, indicesSize = 0;
	unsigned int indicesCount = 0;

	for(int x = 0; x < BGL_ChunkSize; x++) {
		for (int y = 0; y < BGL_ChunkSize; y++) {
			for (int z = 0; z < BGL_ChunkSize; z++) {
				float gx = (int)BGL_ChunkSize * chunk->position.x + x;
				float gy = (int)BGL_ChunkSize * chunk->position.y + y;
				float gz = (int)BGL_ChunkSize * chunk->position.z + z;
				const unsigned char id = chunk->blocks[x][y][z].id;
				if(id > 0) {
					for(int i = 0; i < 6; i++) {
						int normalIndex = i * 3;
						unsigned char neighbour = 0;
						bool outOfBounds = !checkNeighbour(chunk, &neighbour, x + cube_normals[0 + normalIndex], y + cube_normals[1 + normalIndex], z + cube_normals[2 + normalIndex]);

						if (neighbour == 0 || outOfBounds) {
							for (int j = 0; j < 4; j++) {
								int posIndex = j * 3 + i * 12;
								vertices[verticesSize] = cube_vertices[0 + posIndex] + gx;
								++verticesSize;
								vertices[verticesSize] = cube_vertices[1 + posIndex] + gy;
								++verticesSize;
								vertices[verticesSize] = cube_vertices[2 + posIndex] + gz;
								++verticesSize;

								int textureIndex = j * 2;
								vertices[verticesSize] = cube_texture[0 + textureIndex];
								++verticesSize;
								vertices[verticesSize] = cube_texture[1 + textureIndex];
								++verticesSize;
								vertices[verticesSize] = block_textureIds[id][i];
								++verticesSize;

								vertices[verticesSize] = cube_normals[0 + normalIndex];
								++verticesSize;
								vertices[verticesSize] = cube_normals[1 + normalIndex];
								++verticesSize;
								vertices[verticesSize] = cube_normals[2 + normalIndex];
								++verticesSize;
							}

							indices[indicesSize] = (2 + indicesCount);
							++indicesSize;
							indices[indicesSize] = (1 + indicesCount);
							++indicesSize;
							indices[indicesSize] = (0 + indicesCount);
							++indicesSize;
							indices[indicesSize] = (2 + indicesCount);
							++indicesSize;
							indices[indicesSize] = (3 + indicesCount);
							++indicesSize;
							indices[indicesSize] = (1 + indicesCount);
							++indicesSize;
							indicesCount += 4;
						}
					}
				}
			}
		}
	}

	// Ensure that no overflow has occurred.
	assert(verticesSize <= sizeof(vertices) / sizeof(vertices[0]));
	assert(indicesSize <= sizeof(indices) / sizeof(indices[0]));

	if(verticesSize > 1) {
		// Ensure that the expected size ratio is met
		assert((verticesSize / indicesSize == 6) && (verticesSize % indicesSize == 0));
		chunk->noMesh = false;
	}
	else {
		assert(verticesSize == 0 && indicesSize == 0 && indicesCount == 0);
		chunk->noMesh = true;
	}
	chunk->indicesSize = indicesSize;

	glBindVertexArray(chunk->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(GLuint), indices, GL_STATIC_DRAW);
}

void initWorld(struct World* world) {
	world->skyColor[0] = 0.1f;
	world->skyColor[1] = 0.6f;
	world->skyColor[2] = 0.9f;

	world->lightColor[0] = 1.f;
	world->lightColor[1] = 1.f;
	world->lightColor[2] = 1.f;

	world->lightPos[0] = 100000.f;
	world->lightPos[1] = 200000.f;
	world->lightPos[2] = 100000.f;

	for(int x = 0; x < BGL_LoadSize; x++) {
		for(int y = 0; y < BGL_LoadSize; y++) {
			for(int z = 0; z < BGL_LoadSize; z++) {
				struct Chunk* chunk = &world->chunks[x][y][z];
				chunk->isGenerated = false;
				initChunk(chunk);
			}
		}
	}
}

void generateCosineTerrain(struct Chunk* chunk) { // <----------- Possible optimization. Traverse memory block differently
	//Previous memory should be cleared
	const struct Vec3i pos = chunk->position;
	float x1 = (int)BGL_ChunkSize * pos.x;
	float y1 = (int)BGL_ChunkSize * pos.y;
	float z1 = (int)BGL_ChunkSize * pos.z;
	for(int x = 0; x < BGL_ChunkSize; x++) {
		for(int z = 0; z < BGL_ChunkSize; z++) {
			float val = cosf((x + x1)/20.f) * cosf((z + z1)/20.f) * 10 + 20;
			for(int y = 0; y < BGL_ChunkSize; y++) {
				unsigned char id;
				if(val > y + y1) {
					id = 1;
				}
				else {
					id = 0;
				}
				chunk->blocks[x][y][z].id = id;
			}
		}
	}
}

void generatePerlinTerrain(struct Chunk* chunk) { // <----------- Possible optimization. Traverse memory block differently
	//Previous memory should be cleared
	const struct Vec3i pos = chunk->position;
	float x1 = (int)BGL_ChunkSize * pos.x;
	float y1 = (int)BGL_ChunkSize * pos.y;
	float z1 = (int)BGL_ChunkSize * pos.z;
	for(int x = 0; x < BGL_ChunkSize; x++) {
		for(int z = 0; z < BGL_ChunkSize; z++) {
			float val = stb_perlin_turbulence_noise3((x1 + x) / 100.f, 0, (z1 + z) / 100.f, 2.f, 0.5f, 6, 0, 0, 0);
			for(int y = 0; y < BGL_ChunkSize; y++) {
				unsigned char id;
				if(val * 50 > y1 + y) {
					id = 1;
				}
					else if(val * 51 > y1 + y) {
					id = 2;
				}
				else {
					id = 0;
				}
				chunk->blocks[x][y][z].id = id;
			}
		}
	}
}

static void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void initMessage() {
	printf("Chunk size: %i\n", BGL_ChunkSize);
	printf("Loading radius: %i\n", BGL_LoadRadius);
	printf("Max faces per chunk: %i\n", BGL_MaxFaces);
}

GLFWwindow* initWindow() {
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //for mac compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return window;
}

#endif /* BLOCKGL_H */