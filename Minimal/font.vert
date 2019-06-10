#version 410 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
//layout (location = 0) in vec3 position;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 modelview;

void main()
{
    //gl_Position = projection * vec4(vertex.xy, -1.0, 1.0);
	//gl_Position = vec4(vertex.xy, -1.0, 1.0);
    TexCoords = vertex.zw;
	gl_Position = projection * modelview * vec4(vertex.xy, -1.0, 1.0);
	//TexCoords = vec3(vertex.z,vertex.w,0.0);
}  