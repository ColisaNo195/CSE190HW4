#ifndef OBJOBJECT_H
#define OBJOBJECT_H

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <math.h>
class OBJObject
{
public:
	std::vector<unsigned int> indices;
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLuint> tvertices;
	
	std::vector<glm::vec3> tnormals;

	glm::mat4 toWorld;
	glm::mat4 temp;

	static int width;
	static int height;
	GLfloat angle;
	
	GLint size;
	GLint size2;
	GLfloat avgX;
	GLfloat avgY; 
	GLfloat avgZ;
	glm::vec3 position;
	GLfloat oangle;
	GLfloat rangle;
	GLfloat scales;
	glm::vec3 axis;
	glm::vec3 curr;
	glm::vec3 prev;
	GLfloat vel;
	GLuint VBO, VAO, EBO, VBO2;
	GLuint uProjection, uModelview, uModel, viewPos;
	

	OBJObject(const char* filepath);
	~OBJObject();
	void parse(const char* filepath);
	void draw(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view);
	//void draw_dot();
	glm::mat4 spin(GLfloat);
	glm::mat4 move();
	glm::mat4 scale();
	//glm::mat4 orbit(GLfloat degree);
	glm::mat4 rotate();
	glm::mat4 init();
	//glm::mat4 trackBallMapping(GLfloat x, GLfloat y);
	void update();
	void passing(glm::mat4 I);
	//Material stuff
	GLint object_mode;
	glm::vec3 color_diff;
	glm::vec3 color_spec;
	glm::vec3 color_ambi;

	//Light stuff
	int light_mode;
	glm::vec3 light_color;
	glm::vec3 light_pos;
	glm::vec3 light_dir;
	float cutoff_angle;
	float exponent;
};

#endif