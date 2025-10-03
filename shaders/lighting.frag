#version 330 core

in vec2 fragTexCoord;       // Fragment input attribute: texture coordinate
in vec4 fragColor;          // Fragment input attribute: color
in vec3 worldPosition;
out vec4 finalColor;        // Fragment output: color

uniform sampler2D texture0; // Fragment input texture (always required, could be a white pixel)
uniform sampler2D texture1;
uniform vec4 colDiffuse;    // Fragment input color diffuse (multiplied by texture color)

uniform vec2 renderMinimum;
uniform vec2 renderMaximum;

void main() {
  vec4 texColor = texture(texture0, fragTexCoord);

  vec2 uv = (worldPosition.xy - renderMinimum) / (renderMaximum - renderMinimum);
  uv = clamp(uv, 0.0, 1.0);
  vec4 lightColor = texture(texture1, uv);

  vec4 tint = fragColor * colDiffuse;
//  finalColor = lightColor; 
  finalColor = tint * (texColor * lightColor);
}
