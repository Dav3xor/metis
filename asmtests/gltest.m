*simple test for gl functions

SHADER vertex_shader

#version 400
in vec3 vp;
in vec2 intex;
out vec2 texcoord;
void main () {
  texcoord = intex;
  gl_Position = vec4(vp.x, vp.y, 0.0, 1.0);
}

END-SHADER

SHADER fragment_shader
#version 330 core
out vec4 color;
in vec2 texcoord;
uniform sampler2D tex;
void main(){
  //color = vec4(1.0,1.0,1.0,1.0);
  color = texture(tex, texcoord);
}

END-SHADER









