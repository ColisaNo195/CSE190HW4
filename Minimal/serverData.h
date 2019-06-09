#pragma once

#include <glm/glm.hpp>
#include <rpc/client.h>

struct avatarPos {
	glm::mat4 view;
	glm::vec3 viewPos;

	MSGPACK_DEFINE_MAP(view[0][0], view[0][1], view[0][2], view[0][3],
		view[1][0], view[1][1], view[1][2], view[1][3],
		view[2][0], view[2][1], view[2][2], view[2][3],
		view[3][0], view[3][1], view[3][2], view[3][3],
		viewPos.x, viewPos.y, viewPos.z);
};