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

int main(void) {
	initMessage();
	GLFWwindow* window = initWindow();

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
	GLubyte texels[BGL_TextureSize * BGL_TextureSize * 4 * BGL_TextureCount];
	int w;
	int h;
	int comp;
	unsigned char* stone_tex = stbi_load("stone.png", &w, &h, &comp, STBI_rgb_alpha);
	insertTexture(texels, stone_tex, 0);
	stbi_image_free(stone_tex);

	unsigned char* grass_top_tex = stbi_load("grass_top.png", &w, &h, &comp, STBI_rgb_alpha);
	insertTexture(texels, grass_top_tex, 1);
	stbi_image_free(grass_top_tex);

	unsigned char* grass_side_tex = stbi_load("grass_side.png", &w, &h, &comp, STBI_rgb_alpha);
	insertTexture(texels, grass_side_tex, 2);
	stbi_image_free(grass_side_tex);

	unsigned char* dirt_tex = stbi_load("dirt.png", &w, &h, &comp, STBI_rgb_alpha);
	insertTexture(texels, dirt_tex, 3);
	stbi_image_free(dirt_tex);

	// Air has id 0
	// Stone block
	defineBlockTexture(block_textureIds, 1, 0, 0, 0, 0, 0, 0);

	// Grass block
	defineBlockTexture(block_textureIds, 2, 2, 2, 1, 3, 2, 2);

	// Dirt block
	defineBlockTexture(block_textureIds, 3, 3, 3, 3, 3, 3, 3);

	GLuint texture;
	GLsizei mipLevelCount = 2;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, BGL_TextureSize, BGL_TextureSize, BGL_TextureCount);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, BGL_TextureSize, BGL_TextureSize, BGL_TextureCount, GL_RGBA, GL_UNSIGNED_BYTE, texels);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

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
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
						generatePerlinTerrain(chunk);
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

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	free(world);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
