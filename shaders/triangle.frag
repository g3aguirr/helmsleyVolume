#version 460

uniform float Center = .7;
uniform float Width = 1.0;

out vec4 FragColor;

in vs_out {
	vec2 uv;
} i;



void main() {
	float alpha = 1.0;
	//Width is scalar for triangle

	float xPos = (i.uv.x - .5) * Width;
	alpha = 1.0 - step(1.0, xPos + Center);
	if(alpha == 1.0)
		alpha = step(0.0, Center + xPos);
	vec3 col = vec3(0.0, 0.0, 1.0);
	FragColor = vec4(col, alpha);
}