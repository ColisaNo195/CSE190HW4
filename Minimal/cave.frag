#version 410 core
// This is a sample fragment shader.

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note that you do not have access to the vertex shader's default output, gl_Position.


in vec2 TexCoords;

// You can output many things. The first vec4 type output determines the color of the fragment
out vec4 fragColor;

uniform sampler2D screenTexture;

void main()
{
    vec4 texColor = texture(screenTexture, TexCoords);
	if(texColor.a < 0.1)
        discard;
    fragColor = texColor;
	//fragColor = vec4(1.0f);
}
