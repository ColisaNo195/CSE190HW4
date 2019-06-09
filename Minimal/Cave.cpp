#include "Cave.h"
#include <string>
#include <GLFW/glfw3.h>
#include <iostream>
#include "lodepng.h"

#define TEXTURE_WIDTH 1344
#define TEXTURE_HEIGHT 1600

using namespace std;

// vertices for one side of square; 1.0 * 1.0 due to texture limit
const GLfloat vertices[] = {
   // front side
	0.0f, 1.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	1.0f, 0.0f, -1.0f,
	1.0f, 0.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	0.0f, 1.0f, -1.0f
};

// 12 vertices
GLfloat triangles[24] = { 0 };

Cave::Cave(char * filename)
{
	toWorld = glm::mat4(1.0f);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	
	// Load file and decode image.
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);

	// If there's an error, display it.
	if (error != 0) {
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		return;
	}
	// Texture size must be power of two for the primitive OpenGL version this is written for. Find next power of two.
	/*size_t u2 = 1; while (u2 < width) u2 *= 2;
	size_t v2 = 1; while (v2 < height) v2 *= 2;
	// Ratio for power of two version compared to actual version, to render the non power of two image with proper size.
	double u3 = (double)width / u2;
	double v3 = (double)height / v2;

	// Make power of two version of the image.
	std::vector<unsigned char> image2(u2 * v2 * 4);
	for (size_t y = 0; y < height; y++)
		for (size_t x = 0; x < width; x++)
			for (size_t c = 0; c < 4; c++) {
				image2[4 * u2 * y + 4 * x + c] = image[4 * width * y + 4 * x + c];
			}*/

	// Enable the texture for OpenGL.
	glEnable(GL_TEXTURE_2D);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &normalBuffer);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(VAO);

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0); 
	glVertexAttribPointer(
		0,
		// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3,
		// This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat),
		// Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0);
	// Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 1. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		1,
		// This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		sizeof(GLfloat),
		// Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0);
	// Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
}
Cave::~Cave()
{
  // delete texture
  glDeleteTextures(1, &texture);

  // Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
  // large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &vertexBuffer);
  glDeleteBuffers(1, &normalBuffer);
}

GLuint Cave::getTexture() {
	return texture;
}

void Cave::draw(unsigned int shaderProgram, const glm::mat4& projection, const glm::mat4& view)// bool isLeft
{
	/* render scene */
	glEnable(GL_BLEND);
	glUseProgram(shaderProgram);
	// Calculate the combination of the model and view (camera inverse) matrices
	glm::mat4 modelview = view * toWorld;
	//glm::mat4 modelview = glm::mat4(1.0f) * toWorld;
	// We need to calculate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
	// Consequently, we need to forward the projection, view, and model matrices to the shader programs
	// Get the location of the uniform variables "projection" and "modelview"
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");
	// Now send these values to the shader program
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	// Now draw the cube. We simply need to bind the VAO associated with it.
	glBindVertexArray(VAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture); 
	glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0); // suggested to be 0?
	// Tell OpenGL to draw with triangles
	glDrawArrays(GL_TRIANGLES, 0, 3 * 2); // 3 vertices per triangle, 2 triangles per face
	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);
	
}

void Cave::update() {
	spin(1.0f);
}

void Cave::spin(float deg) {
	// If you haven't figured it out from the last project, this is how you fix spin's behavior
	toWorld = toWorld * glm::rotate(glm::mat4(1.0f), 1.0f / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
}