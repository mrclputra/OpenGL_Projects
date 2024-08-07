// implement vertex code glsl here

#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

// for movement
uniform float x_offset;
uniform float y_offset;

void main() {
	gl_Position = vec4(aPos.x + x_offset, aPos.y + y_offset, aPos.z, 1.0);
	ourColor = aColor;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}