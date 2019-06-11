#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <rpc/client.h>

using namespace glm;

struct avatarPos {
	glm::mat4 view;
	glm::vec3 viewPos; //sizeof(avatarPos);

	avatarPos() {
		view = glm::mat4(0);
		viewPos = glm::vec3(0);
	}

	MSGPACK_DEFINE_MAP(view[0][0], view[0][1], view[0][2], view[0][3],
		view[1][0], view[1][1], view[1][2], view[1][3],
		view[2][0], view[2][1], view[2][2], view[2][3],
		view[3][0], view[3][1], view[3][2], view[3][3],
		viewPos.x, viewPos.y, viewPos.z);
};

struct vector3_10 {
	glm::vec3 vector[10];

	MSGPACK_DEFINE_MAP(vector[0].x, vector[0].y, vector[0].z,
		vector[1].x, vector[1].y, vector[1].z,
		vector[2].x, vector[2].y, vector[2].z,
		vector[3].x, vector[3].y, vector[3].z,
		vector[4].x, vector[4].y, vector[4].z,
		vector[5].x, vector[5].y, vector[5].z,
		vector[6].x, vector[6].y, vector[6].z,
		vector[7].x, vector[7].y, vector[7].z,
		vector[8].x, vector[8].y, vector[8].z,
		vector[9].x, vector[9].y, vector[9].z);
};

struct vector3 {
	glm::vec3 vector;

	MSGPACK_DEFINE_MAP(vector.x, vector.y, vector.z);
};

struct quaterion {
	glm::quat quaterion;

	MSGPACK_DEFINE_MAP(quaterion.w, quaterion.x, quaterion.y, quaterion.z);
};

struct quaterion_10 {
	glm::quat quaterion[10];

	MSGPACK_DEFINE_MAP(quaterion[0].w, quaterion[0].x, quaterion[0].y, quaterion[0].z,
		quaterion[1].w, quaterion[1].x, quaterion[1].y, quaterion[1].z,
		quaterion[2].w, quaterion[2].x, quaterion[2].y, quaterion[2].z,
		quaterion[3].w, quaterion[3].x, quaterion[3].y, quaterion[3].z,
		quaterion[4].w, quaterion[4].x, quaterion[4].y, quaterion[4].z,
		quaterion[5].w, quaterion[5].x, quaterion[5].y, quaterion[5].z,
		quaterion[6].w, quaterion[6].x, quaterion[6].y, quaterion[6].z,
		quaterion[7].w, quaterion[7].x, quaterion[7].y, quaterion[7].z,
		quaterion[8].w, quaterion[8].x, quaterion[8].y, quaterion[8].z,
		quaterion[9].w, quaterion[9].x, quaterion[9].y, quaterion[9].z);
};

struct matrix4 {
	glm::mat4 matrix;

	MSGPACK_DEFINE_MAP(matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
		matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
		matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
		matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
};