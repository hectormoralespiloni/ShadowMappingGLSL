uniform sampler2DShadow shadowMap;
varying vec4 diffuseColor;
varying vec4 specularColor;
varying vec4 texCoord;

void main()
{
	float shadowFactor;
	float s; 
	
	//shadow2DProj returns 1 if pixel is in shadow and 0 otherwise
	s = shadow2DProj(shadowMap, texCoord).r;
	
	//attenuate shadow value instead of using completely black color
	if(s < 0.99) 
		shadowFactor = 0.4;
	else 
		shadowFactor = 1.0;
		
	gl_FragColor = (diffuseColor * gl_Color * shadowFactor) + specularColor;
}

