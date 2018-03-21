#version 400 core

out vec4 colorOutput;
in vec3 pos;
uniform vec4 lightColor;


void main()
{
    colorOutput = vec4(pos, 1.0);
}
