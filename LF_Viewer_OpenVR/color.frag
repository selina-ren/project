#version 150

uniform int depth_flag;
uniform sampler2D myTexture;
uniform float Zfar_scale;

in vec2 tex_coord;

out vec4 outputF;
 
void main()
{

	if(depth_flag == 1)
	{
		

		outputF.r = gl_FragCoord.z/ gl_FragCoord.w*0.0004995+0.0009995;
		outputF.g = outputF.r;
		outputF.b = outputF.r;
		outputF.a = 1;
		

	}
	else
	{

		outputF = texture2D(myTexture,tex_coord.xy).rgba;

	}

	
	
}