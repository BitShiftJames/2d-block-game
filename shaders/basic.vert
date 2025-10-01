#version 330 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec3 worldPosition;
out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;

void main() {
  fragTexCoord = vertexTexCoord;
  worldPosition = vertexPosition;
  fragColor = vertexColor;

  gl_Position = mvp*vec4(vertexPosition, 1.0);
}   
