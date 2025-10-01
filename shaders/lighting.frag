#version 330 core

in vec2 fragTexCoord;       // Fragment input attribute: texture coordinate
in vec4 fragColor;          // Fragment input attribute: color
in vec3 worldPosition;
out vec4 finalColor;        // Fragment output: color

uniform sampler2D texture0; // Fragment input texture (always required, could be a white pixel)
uniform vec4 colDiffuse;    // Fragment input color diffuse (multiplied by texture color)

uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform vec4 ambient;

void main() {
  vec4 texColor = texture(texture0, fragTexCoord);
  float LightStrength = 150.0;

  float LightDistance = distance(worldPosition, lightPos);
  float LightHere = LightStrength / (LightDistance * LightDistance);

  float LightDistance2 = distance(worldPosition, lightPos2);
  float LightHere2 = LightStrength / (LightDistance2 * LightDistance2);

  float TotalLight = LightHere + LightHere2;

  vec4 tint = fragColor * colDiffuse;
  vec3 litColor = TotalLight * (texColor.rgb);

  finalColor = tint * vec4(litColor, 1.0);
  finalColor += texColor*(ambient/10.0)*tint;
}
