/*----------------------------------------------------
   Tara Boulanger (7922331)
   File name:   a2f_curtain.glsl

   Description: This file is heavily based on the example0
   fragment file. It calculates the colour of that pixel
   based on the vectors it received from the vertex
   shader and outputs the corresponding colour.
------------------------------------------------------*/
#version 150

in vec3 N, L, E;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float Shininess;
out vec4 out_colour;

void main() {
	vec3 H = normalize(L+E);
  	vec4 ambient = AmbientProduct;

    float Kd = max( dot(L, N), 0.0 );
    vec4  diffuse = Kd * DiffuseProduct;

    float Ks = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = Ks * SpecularProduct;

    if ( dot(L, N) < 0.0 ) {
      specular = vec4(0.0, 0.0, 0.0, 1.0);
    }//end if

    out_colour = ambient + diffuse + specular;
    out_colour.a = 1.0;
}//end main
