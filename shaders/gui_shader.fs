#version 330 core

in vec2 fragmentTextureCoords;

out vec4 finalColor;

uniform bool useTexture = false;
uniform vec4 color;
uniform sampler2D tex;

void main()
{
	if (useTexture)
		finalColor = texture(tex, fragmentTextureCoords);
	else
		finalColor = color;
}