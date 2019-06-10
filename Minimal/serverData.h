#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <rpc/client.h>

struct avatarPos {
	glm::mat4 view;
	glm::vec3 viewPos; //sizeof(avatarPos);

	MSGPACK_DEFINE_MAP(view[0][0], view[0][1], view[0][2], view[0][3],
		view[1][0], view[1][1], view[1][2], view[1][3],
		view[2][0], view[2][1], view[2][2], view[2][3],
		view[3][0], view[3][1], view[3][2], view[3][3],
		viewPos.x, viewPos.y, viewPos.z);
};

struct vector3 {
	glm::vec3 vector;

	MSGPACK_DEFINE_MAP(vector.x, vector.y, vector.z);
};

struct quaterion {
	glm::quat quaterion;

	MSGPACK_DEFINE_MAP(quaterion.w, quaterion.x, quaterion.y, quaterion.z);
};

struct matrix4 {
	glm::mat4 matrix;

	MSGPACK_DEFINE_MAP(matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
		matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
		matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
		matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
};