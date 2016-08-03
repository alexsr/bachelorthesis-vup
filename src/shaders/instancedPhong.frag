#version 330 core

in vec3 pPos;
in vec3 pNormal;
in vec4 pColor;

out vec4 frag;

void main()
{
  vec3 lightPosition = vec3(0.0,1000.0,100.0);
	vec4 ambientLight = vec4(0.2, 0.2, 0.2, 1.0);
	
	vec3 lightVec = normalize(lightPosition-pPos);
	
  vec4 color = pColor;
  color.a = 0.0;
	
	float cos_phi = max(dot(pNormal, lightVec), 0.0f);
	
	frag = color * ambientLight + color * cos_phi;
}
