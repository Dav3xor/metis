* simple test for gl functions

SHADER                vertex_shader

#version 400
in  vec3 vp;
in  vec3 color;
out vec3 vcolor;

void main () {
  gl_Position = vec4(vp.x, vp.y, 0.0, 1.0);
}

END-SHADER

SHADER                fragment_shader
#version 330 core
in  vec3 vcolor;
out vec3 color;
uniform sampler2D tex;
void main(){
  color = vcolor;
}

END-SHADER

BUFFER                      triangle 9

-0.8 -0.8 0.0
 0.8 -0.8 0.0
 0.0  0.8 0.0

BUFFER                      colors 9

1.0 0.5 0.0
0.0 1.0 0.5
0.5 0.0 1.0

LOC                         init

GLGENVERTEXARRAYS           1 0
GLBINDVERTEXARRAY           0

GLGENBUFFERS                2 1


GLBINDBUFFER                GL_ARRAY_BUFFER 1
GLBUFFERDATA                GL_ARRAY_BUFFER 36 triangle GL_STATIC_DRAW

GLBINDBUFFER                GL_ARRAY_BUFFER 2
GLBUFFERDATA                GL_ARRAY_BUFFER 36 colors GL_STATIC_DRAW


GLCREATESHADER              GL_VERTEX_SHADER 3
GLSHADERSOURCE              vertex_shader 3
GLCOMPILESHADER             3

GLCREATESHADER              GL_FRAGMENT_SHADER 4
GLSHADERSOURCE              fragment_shader 4
GLCOMPILESHADER             4

GLCREATEPROGRAM             5
GLATTACHSHADER              5 3
GLATTACHSHADER              5 4
GLLINKPROGRAM               5
GLUSEPROGRAM                5
END

LOC                         mainloop
GLENABLEVERTEXATTRIBARRAY   0
GLBINDBUFFER                GL_ARRAY_BUFFER 1
GLVERTEXATTRIBPOINTER       0 3 GL_FLOAT GL_FALSE 12 0
GLENABLEVERTEXATTRIBARRAY   1
GLBINDBUFFER                GL_ARRAY_BUFFER 2
GLVERTEXATTRIBPOINTER       1 3 GL_FLOAT GL_FALSE 12 0
GLDRAWARRAYS                GL_TRIANGLES 0 3

END

