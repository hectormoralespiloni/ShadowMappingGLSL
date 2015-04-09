varying vec4 diffuseColor;
varying vec4 specularColor;
varying vec4 texCoord;

void main()
{
	float dotNL;
	float dotHV;
	float sp = 0.0;
	
	vec3 lightVector;
	vec3 normal;
	
	//transform vertices
	gl_Position = ftransform();
	
	//get texture coordinates
	texCoord =  gl_TextureMatrix[1] * gl_ModelViewMatrix * gl_Vertex;
	
	//get material color
	gl_FrontColor = gl_FrontMaterial.ambient + gl_FrontMaterial.diffuse;
		
	//calculate vertex normals
	normal = gl_NormalMatrix * gl_Normal;
	normal = normalize(normal);
	
	//calculate diffuse color based on light position
	lightVector = gl_LightSource[0].position.xyz - gl_Vertex.xyz;
	lightVector = normalize(lightVector);
	dotNL = max(0.0, dot(normal, lightVector));
	diffuseColor = gl_LightSource[0].diffuse * dotNL;
	
	//calculate specular color
	dotHV = max(0.0, dot(normal, vec3(gl_LightSource[0].halfVector)));
	if(dotNL != 0.0) sp = pow(dotHV, gl_FrontMaterial.shininess);
	specularColor = gl_FrontMaterial.specular * sp;	
}
