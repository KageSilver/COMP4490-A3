/*----------------------------------------------------
   Tara Boulanger (7922331)
   File name:   a3v_lilman.glsl

   Description: 
------------------------------------------------------*/
#version 150

in vec4 vPosition;

uniform mat4 ModelView;

void main() {
    gl_Position = ModelView * vPosition;
}//end main

