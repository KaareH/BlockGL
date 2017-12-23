#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <linmath.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "blockgl.h"

#if false
GLfloat cube_vertices[] = {
		// front
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// back
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
};

GLuint cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 0, 3,
		3, 7, 4,
		// left
		4, 5, 1,
		1, 0, 4,
		// right
		3, 2, 6,
		6, 7, 3,
};

/* Frustum configuration */
static GLfloat view_angle = 45.0f;
static GLfloat aspect_ratio = 4.0f/3.0f;
static GLfloat z_near = 1.0f;
static GLfloat z_far = 100.f;

/* Projection matrix */
static GLfloat projection_matrix[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
};

/* Model view matrix */
static GLfloat modelview_matrix[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
};
#endif

#if true
GLfloat vertices[] = {
		// front
		-1.0, -1.0,  1.0,      0,0,0,0,0,0,
		1.0, -1.0,  1.0,      0,0,0,0,0,0,
		1.0,  1.0,  1.0,      0,0,0,0,0,0,
		-1.0,  1.0,  1.0,      0,0,0,0,0,0,
		// back
		-1.0, -1.0, -1.0,      0,0,0,0,0,0,
		1.0, -1.0, -1.0,      0,0,0,0,0,0,
		1.0,  1.0, -1.0,      0,0,0,0,0,0,
		-1.0,  1.0, -1.0,      0,0,0,0,0,0,
};

GLuint indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 0, 3,
		3, 7, 4,
		// left
		4, 5, 1,
		1, 0, 4,
		// right
		3, 2, 6,
		6, 7, 3,
};

const char *vertexShaderSource = "#version 330 core\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"\tvec4 positionRelativeToCam = view * vec4(aPos, 1.0f);\n"
		"\tgl_Position = projection * positionRelativeToCam;\n"
		"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n\0";
#endif

int main(void) {
	initMessage();
	GLFWwindow* window = initWindow();
#if false

	GLuint shaderProgram;
	buildShader(&shaderProgram, vertexShaderSource, fragmentShaderSource);

	int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
	int viewLocation = glGetUniformLocation(shaderProgram, "view");

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);



	struct Camera camera;
	double time;
	while (!glfwWindowShouldClose(window)) {
		double DT = getDelta(&time);
		handleCameraInput(&camera, window, DT);

		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		mat4x4 proj, view;
		mat4x4_identity(view);

		mat4x4_perspective(proj, 45, ratio, 0.1f, 1000.f);

		getCameraMatrix(&camera, &view);

		glUseProgram(shaderProgram);
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const GLfloat*) view);
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (const GLfloat*) proj);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
#endif


#if true

	/*
	 * Shader
	 */
	GLuint program;
	buildShader(&program, vertex_shader_text, fragment_shader_text);

	GLint view_location, projection_location;
	projection_location = glGetUniformLocation(program, "projection");
	view_location = glGetUniformLocation(program, "view");

	GLint lightPos_location, lightColor_location, fogColor_location;
	lightPos_location = glGetUniformLocation(program, "lightPos");
	lightColor_location = glGetUniformLocation(program, "lightColor");
	fogColor_location = glGetUniformLocation(program, "fogColor");

	/*
	 * Texture
	 */
	int w;
	int h;
	int comp;
	unsigned char* image = stbi_load("stone.png", &w, &h, &comp, STBI_rgb_alpha);

	if(image == NULL)
		assert(false);

	GLuint texture;
	GLubyte texels[16 * 16 * 4 * 1];
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			int tindex = ((x + (y * 16)) * 4);// + (1024 * 1024 * 4 * 0);
			int rindex = ((x + (y * 16)) * 4);
			texels[tindex] = image[rindex];
			texels[tindex + 1] = image[rindex + 1];
			texels[tindex + 2] = image[rindex + 2];
			texels[tindex + 3] = image[rindex + 3];
			/*texels[tindex] = 160;
			texels[tindex + 1] = 210;
			texels[tindex + 2] = 128;
			texels[tindex + 3] = 255;*/
		}
	}

	GLsizei width = 16;
	GLsizei height = 16;
	GLsizei layerCount = 1;
	GLsizei mipLevelCount = 2;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, width, height, layerCount);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, layerCount, GL_RGBA, GL_UNSIGNED_BYTE, texels);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


	stbi_image_free(image);

	struct World* world = malloc(sizeof(struct World));
	initWorld(world);

	struct Camera camera;

	double time;
	initTime(&time);
	double DT = 0;
	while (!glfwWindowShouldClose(window)) {
		DT = getDelta(&time);
		handleCameraInput(&camera, window, DT);
		printf("Delta: %f\n", DT);
		printf("CamPos: %f, %f, %f. CamDir: %f, %f, %f.\n", camera.position[0], camera.position[1], camera.position[2],
			   camera.rotation[0], camera.rotation[1], camera.rotation[2]);

		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height;
		glViewport(0, 0, width, height);
		glClearColor(world->skyColor[0], world->skyColor[1], world->skyColor[2], 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4x4 proj, view;
		mat4x4_identity(view);

		mat4x4_perspective(proj, 45, ratio, 0.1f, 10000.f);

		getCameraMatrix(&camera, &view);

		glUseProgram(program);
		glUniformMatrix4fv(view_location, 1, GL_FALSE, (const GLfloat*) view);
		glUniformMatrix4fv(projection_location, 1, GL_FALSE, (const GLfloat*) proj);

		glUniform3f(fogColor_location, world->skyColor[0], world->skyColor[1], world->skyColor[2]);
		glUniform3f(lightColor_location, world->lightColor[0], world->lightColor[1], world->lightColor[2]);
		glUniform3f(lightPos_location, world->lightPos[0], world->lightPos[1], world->lightPos[2]);

		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		struct Vec3i chp = toChunkPos(camera.position); //Camera chunk position
		for(int x = chp.x - BGL_LoadRadius; x <= chp.x + (int)BGL_LoadRadius; x++) {
			for(int y = chp.y - BGL_LoadRadius; y <= chp.y + (int)BGL_LoadRadius; y++) {
				for(int z = chp.z - BGL_LoadRadius; z <= chp.z + (int)BGL_LoadRadius; z++) {
					struct Vec3i chunkPos;
					set(&chunkPos, x, y, z);
					struct Vec3i memPos = toMemoryPos(chunkPos);
					struct Chunk* chunk = &world->chunks[memPos.x][memPos.y][memPos.z];
					if(!chunk->isGenerated || memcmp(&chunkPos, &chunk->position, sizeof(struct Vec3i)) != 0) { // If the positions are not equal
						chunk->position = chunkPos;
						chunk->isGenerated = true;
						generateTerrain(chunk);
						generateMesh(chunk);
						//printf("Generated at: %i, %i, %i\n", x, y, z);
						//printf("Mempos at: %i, %i, %i\n", memPos.x, memPos.y, memPos.z);
					}
					if(!chunk->noMesh) {
						//printf("Drawing: %i, %i, %i. Indices: %i\n", x, y, z, chunk->indicesSize);
						glBindVertexArray(chunk->VAO);
						glDrawElements(GL_TRIANGLES, chunk->indicesSize, GL_UNSIGNED_INT, 0);
						glBindVertexArray(0);
					}
				}
			}
		}

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			printf("OpenGL error: %i\n", err);
		}

		vec3 test;
		test[0] = -5.f;
		struct Vec3i integer = toChunkPos(test);
		printf("Test: %i\n", integer.x);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
#endif

#if false
	GLuint shaderProgram;
	buildShader(&shaderProgram, vertexShaderSource, fragmentShaderSource);

	int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
	int viewLocation = glGetUniformLocation(shaderProgram, "view");


	//VAO
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * sizeof(GLuint), indices, GL_STATIC_DRAW);

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

	struct Camera camera;
	double time;
	while (!glfwWindowShouldClose(window)) {
		double DT = getDelta(&time);
		handleCameraInput(&camera, window, DT);

		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		mat4x4 proj, view;
		mat4x4_identity(view);

		mat4x4_perspective(proj, 45, ratio, 0.1f, 1000.f);

		getCameraMatrix(&camera, &view);

		glUseProgram(shaderProgram);
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const GLfloat*) view);
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (const GLfloat*) proj);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

#endif

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
