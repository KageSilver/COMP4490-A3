/*----------------------------------------------------
   Tara Boulanger (7922331)
   File name:   a3v_cps.glsl

   Description: 
------------------------------------------------------*/
#version 150

in vec4 vPosition;

uniform mat4 ModelView, Projection;

void main() {
    gl_Position = ModelView * Projection * vPosition;
}//end main

