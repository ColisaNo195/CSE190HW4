#ifndef CAVE_H
#define CAVE_H

#include <GL/glew.h>
#include <iostream>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cave 
{
public:
	Cave(char * filename);
	//Cave(unsigned int fbo_l, unsigned int rbo_l, unsigned int texture_l);
	   //unsigned int fbo_r, unsigned int rbo_r, unsigned int texture_r);
  ~Cave();

  glm::mat4 toWorld;
  glm::vec4 pa, pb, pc, pd;

  void draw(unsigned int shader, const glm::mat4& p, const glm::mat4& v);// , bool isLeft
  void update();
  void spin(float);
  glm::mat4 offCenter(glm::vec3 eyepos);
  GLuint texture;

  // These variables are needed for the shader program
  GLuint vertexBuffer, normalBuffer, VAO;
  //GLuint vertexBuffer2, normalBuffer2, VAO2; // for triangles
  GLuint uProjection, uModelview, uoffset;
  //GLuint tProjection, tModelview;

  GLuint getTexture();

  int width, height, channels;
};
#endif
