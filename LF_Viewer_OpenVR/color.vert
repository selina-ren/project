#version 150
 
uniform mat4 viewMatrix, projMatrix;
uniform float Zfar_scale;
 
in vec3 position;
//in vec2 uv;
 
out float far_scale;
 
void main()
{
    //uv_out = uv;
	far_scale = Zfar_scale;
    gl_Position = projMatrix * viewMatrix * vec4(position,1) ;
}