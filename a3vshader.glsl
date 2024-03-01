/*----------------------------------------------------
   Tara Boulanger (7922331)
   File name:   a2v_curtain.glsl

   Description: This file is heavily based on the example0
   vertex file. It gives the vertices of the curtain a
   wavy effect based on time, passed in by hourglass.cpp.
   It also calculates all of the normals that are needed
   for the Blinn-Phong shader, passing the resulting
   vectors to the fragment shader.
------------------------------------------------------*/
#version 150

in vec4 vPosition;

uniform mat4 ModelView, ModelViewInverseTranspose, Projection;
uniform vec4 LightPosition;
uniform float Distance;
uniform float Time;

out vec3 N,L,E;

void main() {
    //Moving the curtain (from the height field shader)
    vec4 v = vPosition;
    vec4 u = sin(Time + 5*v);

    v.z = 0.1*u.x*u.y;


	//Calculating the vertex normal
	vec3 adjacentVertices[4];
	for ( int i=0; i<4; i++ ) {
		adjacentVertices[i] = vPosition.xyz;
	}//end for
	adjacentVertices[0].x -= Distance;
	adjacentVertices[1].z += Distance;
	adjacentVertices[2].x += Distance;
	adjacentVertices[3].z -= Distance;

	for ( int i=0; i<4; i++ ) {
		vec3 uAdjacent = sin(Time + 5*adjacentVertices[i]);
		adjacentVertices[i].y = 0.1 * uAdjacent.x * uAdjacent.z;
	}//end for

	//Turning them all into tangent vectors
	for ( int i=0; i<4; i++ ) {
		adjacentVertices[i] -= v.xyz;
	}//end for

	//Getting normals, which are cross products of each pair
	vec3 normals[4];
	for ( int i=0; i<4; i++ ) {
		normals[i] = cross(adjacentVertices[i].xyz, adjacentVertices[(i+1)%4].xyz);
		normals[i] = normalize(normals[i]);
	}//end for

	//Getting the surface normal
	vec3 surfaceNorm = vec3(0,0,0);
	for ( int i=0; i<4; i++ ) {
		surfaceNorm += normals[i];
	}//end for
	surfaceNorm /= 4.0f;

    //Lighting part, with a Blinn-Phong shader
    vec3 pos = (ModelView*v).xyz;
    L = LightPosition.xyz - pos;
    E = -pos;

    N = (ModelViewInverseTranspose*vec4(surfaceNorm,0)).xyz;

    gl_Position = Projection * ModelView * v;
}//end main

