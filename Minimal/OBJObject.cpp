#include "OBJObject.h"
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
using namespace std;

OBJObject::OBJObject(const char *filepath) 
{
	const aiScene* scene = aiImportFile(filepath, aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scene)
	{
		std::cerr << "Could not load file " << filepath << std::endl;
		return;
	}
	aiMesh* mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; ++i) {
		aiVector3D vert = mesh->mVertices[i];
		aiVector3D norm = mesh->mNormals[i];
		vertices.push_back(vert.x);
		vertices.push_back(vert.y);
		vertices.push_back(vert.z);
		normals.push_back(norm.x);
		normals.push_back(norm.y);
		normals.push_back(norm.z);
	}
	for (std::uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; faceIdx++)
	{
		tvertices.push_back(mesh->mFaces[faceIdx].mIndices[0u]);
		tvertices.push_back(mesh->mFaces[faceIdx].mIndices[1u]);
		tvertices.push_back(mesh->mFaces[faceIdx].mIndices[2u]);
	}

	//parse(filepath);
	
	toWorld = glm::mat4(1.0f);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*(vertices.size()), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*(normals.size()), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*tvertices.size(), &tvertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	
	
	
}

OBJObject::~OBJObject() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO2);
}

void OBJObject::parse(const char *filepath) 
{
	FILE *fp;
	GLfloat x, y, z;
	GLfloat r, g, b;
	GLuint t1, t2, t3;//tvertices of triangle corner in vertices
	GLuint n1, n2, n3;//tvertices of triangle corner in vertex normals
	int c1, c2;
	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		cerr<< "error loading file" <<endl; 
		exit(-1); 
	}
	
	while (!feof(fp)) {
		c1 = fgetc(fp);
		c2 = fgetc(fp);
		if ((c1 == 'v') && (c2 == ' ')) {
			fscanf(fp, "%f %f %f", &x, &y, &z);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
		else if ((c1 == 'v') && (c2 == 'n')) {
			fscanf(fp, "%f %f %f", &x, &y, &z);
			//normals.push_back(glm::vec3(x, y, z));
			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(z);
		}
		else if ((c1 == 'f') && (c2 == ' ')) {
			fscanf(fp, "%u//%u %u//%u %u//%u", &t1, &n1, &t2, &n2, &t3, &n3);
			tvertices.push_back(t1);
			tvertices.push_back(t2);
			tvertices.push_back(t3);
			//tnormals.push_back(glm::vec3(n1, n2, n3));
			//cout << t1<<endl << t2 << endl << t3 << endl;
		}
		else{
			fscanf(fp, "\n");
		}
	}
	for (int i = 0; i < 3; i++) {
		//tvertices.pop_back();
	}
	fclose(fp);
	// Populate the face tvertices, vertices, and normals vectors with the OBJ Object data
}


void OBJObject::draw(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view)
{
	glUseProgram(shaderProgram);
	//glDisable(GL_CULL_FACE);
	
	// Calculate the combination of the model and view (camera inverse) matrices
	glm::mat4 modelview = view * toWorld;
	// We need to calcullate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
	// Consequently, we need to forward the projection, view, and model matrices to the shader programs
	// Get the location of the uniform variables "projection" and "modelview"
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");
	viewPos = glGetUniformLocation(shaderProgram, "camera_pos");
	// Now send these values to the shader program
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	//glUniform3fv(viewPos, 1, &camera_pos[0]);
	// Now draw the cube. We simply need to bind the VAO associated with it.
	glBindVertexArray(VAO);
	// Tell OpenGL to draw with triangles
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size / sizeof(GLuint), GL_UNSIGNED_INT, 0);// 3 vertices per triangle, 2 triangles per face, 6 faces
											  // Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);

}

void OBJObject::update() {
}

void OBJObject::passing(glm::mat4 I) {
	toWorld = I*init();
}

glm::mat4 OBJObject::spin(GLfloat deg)
{
	//toWorld = toWorld * glm::rotate(glm::mat4(1.0f), 1.0f / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
	
	this->angle += deg;
	if (this->angle > 360.0f || this->angle < -360.0f) this->angle = 0.0f;
	// This creates the matrix to rotate the obj
	return (glm::rotate(glm::mat4(1.0f), this->angle / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)));
}


glm::mat4 OBJObject::move(){
	return glm::translate(glm::mat4(1.0f),this->position);
}

glm::mat4 OBJObject::scale() {
	return glm::scale(glm::mat4(1.0f), glm::vec3(scales, scales, scales));
}

glm::mat4 OBJObject::rotate() {
	return glm::rotate(glm::mat4(1.0f), vel / 180.0f * glm::pi<float>(), axis);
}

glm::mat4 OBJObject::init() {
	return glm::scale(glm::mat4(1.0f), glm::vec3(scales, scales, scales))*
		glm::translate(glm::mat4(1.0f), glm::vec3(-avgX, -avgY, -avgZ));
}