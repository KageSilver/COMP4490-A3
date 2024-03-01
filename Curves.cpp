/*----------------------------------------------------
   COMP 4490 Winter 2024 Assignment 2
   Tara Boulanger (7922331)
   John Braico
   File name:   hourglass.cpp

   Description: This program draws a 3D hourglass using
   sinusoidal curves and a wire mesh. It will also draw
   sand going from the top of the hourglass and then
   filling the bottom of it. There is a curtain that
   displays behind the hourglass as well.
------------------------------------------------------*/

//Includes
#include "common.h"

#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

//Program Constants
const char *WINDOW_TITLE = "Spinning Hourglass";
const double FRAME_RATE_MS = 1000.0 / 60.0;

const float PI = 3.14159265358979323846;

// Curtain requirements
//-----------------------------------------------
const int CURTAIN_CURVES = 16;
const int CURTAIN_CURVE_VERTICES = 16;

const float CURTAIN_FREQUENCY = PI*4;
const float CURTAIN_HEIGHT = 2.0f;

const float CURTAIN_TOP = 2.0f;
const float CURTAIN_RIGHT = 2.0f;
const float CURTAIN_BOTTOM = -1.0f;
const float CURTAIN_LEFT = -1.0f;

const int CURTAIN_VERTICES = CURTAIN_CURVES*CURTAIN_CURVE_VERTICES*4;
glm::vec4 curtainVertices[CURTAIN_VERTICES];
int curtainIndices[CURTAIN_VERTICES*3];

int lastCurtainIndex = 0;


// Hourglass requirements
//------------------------------------------------
const int NUM_CURVES = 32; // Curves that consist of the wire mesh
const int CURVE_VERTICES = 32; // Number of vertices needed to make a curve

const float HOURGLASS_HEIGHT = 0.15f;
const float HOURGLASS_RADIUS = 0.5f;

//Vector to hold all of the vertices of the hourglass
const int HOURGLASS_VERTICES = NUM_CURVES*CURVE_VERTICES*4;
glm::vec4 hourglassVertices[HOURGLASS_VERTICES];
int hourglassIndices[HOURGLASS_VERTICES*3];

int lastHourglassIndex = 0;
float incrementor = 0.0f;


// Program-required variables
//----------------------------------------------------------------------------

// Need global access to VAOs
GLuint VAOs[2];

// Array of rotation angles for each coordinate axis, in angles (taken from ex8)
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int Axis = Yaxis;
GLfloat Theta[NumAxes] = { 15.0, 0.0, 0.0 };
GLfloat Movement[NumAxes] = { 0.0, 0.0, 0.0 };

// Camera requirements
float windowWidth = 500.0f;
float windowHeight = 500.0f;
glm::vec3 CameraPosition = glm::vec3(Theta[Xaxis],0.0,0.0);
const float EXTREME = 60.0f;

// shader program variables (for switching between)
GLuint ProgramH, ProgramC;

// Model-view and projection matrices uniform location
GLuint ModelViewH, ProjectionH;
GLuint ModelViewC, ModelViewInverseTranspose, ProjectionC, Time;

// Other uniform locations
GLuint IsWire;
GLuint HourglassMiddle;
GLuint HourglassTop;
GLuint HourglassBottom;
GLuint Counter;



// Hourglass Functions
//----------------------------------------------------------------------------

// Used to set the order of the vertices of the hourglass for
// both drawing the edges and filling them in.
void orderHourglassIndices() {
    int index = 0;
    int colorIndex = HOURGLASS_VERTICES;
    for ( int i=0; i<HOURGLASS_VERTICES/4; i++ ) {
        hourglassIndices[index++] = i;
        hourglassIndices[index++] = i+CURVE_VERTICES;
        hourglassIndices[index++] = i+CURVE_VERTICES+1;
        hourglassIndices[index++] = i+1;

        //Now making the triangles for this quad:
        hourglassIndices[colorIndex++] = i;
        hourglassIndices[colorIndex++] = i+CURVE_VERTICES+1;
        hourglassIndices[colorIndex++] = i+1;

        hourglassIndices[colorIndex++] = i;
        hourglassIndices[colorIndex++] = i+CURVE_VERTICES+1;
        hourglassIndices[colorIndex++] = i+CURVE_VERTICES;
    }//end for
    lastHourglassIndex = colorIndex-1;
}//end orderHourglassIndices

// Used to create the vertices along a single sinusoidal curve.
// There will be 2 curves for each part of the hourglass.
// Takes in a variable for the offset position of the curve and
// the starting position of the curve.
void createHourglassCurve(int offset, glm::vec4 initialPosition) {
    glm::vec4 curvePoint;
    float x = 0.0f;
    float y = 0.0f;
    //However many radians we need
    float incrementor = 2.0f*PI/CURVE_VERTICES;
    //The current angle of our curve
    float angle = 0.0f;
    for ( int i=0; i<=CURVE_VERTICES; i++ ) {
        x = (float)HOURGLASS_RADIUS * sin(angle);
        y = HOURGLASS_HEIGHT * angle;
        curvePoint = glm::vec4(initialPosition[0] + x,
                               initialPosition[1] + y,
                               initialPosition[2],
                               1);
        //We need to rotate the curve around the circle
        glm::mat4 rot = glm::rotate(glm::mat4(), incrementor*offset, glm::vec3(0,1,0));
        curvePoint = curvePoint*rot;
        //It'll be offset by the circle position and the current vertex index which is
        //multiplied as each curve vertex is stored in a specific offset
        hourglassVertices[offset+(i*CURVE_VERTICES)] = curvePoint;
        angle += incrementor;
    }//end for
}//end createHourglassCurve

// Used to actually build all of the vertices of the hourglass
// by calling the createHourglassCurve function a given amount of times.
// It will then store those vertices into the hourglass vertices
// array.
void buildHourglass() {
    glm::vec4 startPoint;
    float x = 0.0f;
    float y = -(float)HOURGLASS_HEIGHT/2.0f;
    float z = 0.0f;
    float angle = 0.0f;
    //The angle we increase by each time on the circle
    float incrementor = 2.0f*PI/NUM_CURVES;
    for (int i = 0; i<=NUM_CURVES; i++) {
        //Creating a circle on the x-z plane
        x = HOURGLASS_RADIUS*sin(angle);
        z = HOURGLASS_RADIUS*cos(angle);
        startPoint = glm::vec4(x,y,z,1);
        createHourglassCurve(i,startPoint);
        angle += incrementor;
    }//end for
}//end buildHourglass


// Curtain Functions
//---------------------------------------------------------------------------

// Used to get the order of the vertices of the curtian.
void orderCurtainIndices() {
    int colorIndex = 0;
    for ( int i=0; i<CURTAIN_VERTICES/4; i++ ) {
        //Now making the triangles for this quad:
        curtainIndices[colorIndex++] = i+CURTAIN_CURVE_VERTICES+1;
        curtainIndices[colorIndex++] = i;
        curtainIndices[colorIndex++] = i+1;

        curtainIndices[colorIndex++] = i+CURTAIN_CURVE_VERTICES+1;
        curtainIndices[colorIndex++] = i;
        curtainIndices[colorIndex++] = i+CURTAIN_CURVE_VERTICES;
    }//end for
    lastCurtainIndex = colorIndex-1;
}//end orderCurtainIndices

// Used to create the vertices along a cosine curve.
// There will be 4 curves for each part of the curtain.
// Takes in a variable for the offset position of the curve and
// the starting position of the curve.
void createCurtainCurve(int offset, glm::vec4 initialPosition) {
    glm::vec4 curvePoint;
    float x = 0.0f;
    float z = 0.0f;
    //However many radians we need to increase by
    float incrementor = CURTAIN_FREQUENCY/CURTAIN_CURVE_VERTICES;
    //The current angle of our curve
    float angle = 0.0f;
    for ( int i=0; i<=CURTAIN_CURVE_VERTICES; i++ ) {
        z = (float)HOURGLASS_RADIUS * cos(angle);
        x = HOURGLASS_HEIGHT * angle;
        curvePoint = glm::vec4(initialPosition[0] + x,
                               initialPosition[1],
                               initialPosition[2] + z,
                               1);
        //It'll be offset by the y position and the current vertex index which is
        //multiplied as each curve vertex is stored in a specific index
        curtainVertices[offset+(i*CURTAIN_CURVE_VERTICES)] = curvePoint;
        angle += incrementor;
    }//end for
}//end createCurtainCurve

// Used to build all of the vertices of the curtain
// by calling the createCurtainCurve function a given amount of times.
// It will then store those vertices into the curtian vertices
// array. It starts in the botton left of the curtain
void buildCurtain() {
    glm::vec4 startPoint;
    float x = CURTAIN_LEFT;
    float y = CURTAIN_BOTTOM;
    float z = 0.0f;
    //The height we increase by each time for the curtain
    float incrementor = CURTAIN_HEIGHT/CURTAIN_CURVES;
    for (int i = 0; i<=CURTAIN_CURVES; i++) {
        //Creating a line on the x-y plane
        startPoint = glm::vec4(x,y,z,1);
        createCurtainCurve(i,startPoint);
        y += incrementor;
    }//end for
}//end buildCurtain


// Start of OpenGL drawing
//-------------------------------------------------------------------

// Used to load the buffers for the hourglass, as necessary
void loadHourglassBuffer(GLuint vPosition) {
    // Here for size of stack allocation
    GLuint buffer;

    //**************Hourglass first****************
    // Loading in the buffer for the cube
    glBindVertexArray(VAOs[0]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the hourglass
    glBufferData(GL_ARRAY_BUFFER, sizeof(hourglassVertices), hourglassVertices, GL_STATIC_DRAW);
    // Another for the hourglass index buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hourglassIndices), hourglassIndices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}//end loadHourglassBuffer

// Used to initialize the variables for the hourglass
void setupHourglassColour() {
    glm::vec4 hourglassColor(0.914,0.9375,0.9375,1);
    glm::vec4 wireColor(0.0,0.0,0.0,1.0);
    glm::vec4 sandColor(0.9609, 0.8398, 0.6875, 1);
    glUniform4fv(glGetUniformLocation(ProgramH, "HourglassColour"), 1, glm::value_ptr(hourglassColor));
    glUniform4fv(glGetUniformLocation(ProgramH, "WireColour"), 1, glm::value_ptr(wireColor));
    glUniform4fv(glGetUniformLocation(ProgramH, "SandColour"), 1, glm::value_ptr(sandColor));

    //Setting the value of the y values for the sand
    float hourglassTop = hourglassVertices[HOURGLASS_VERTICES/4].y;
    float hourglassBottom = hourglassVertices[0].y;
    float hourglassMiddle = (hourglassTop-hourglassBottom)/2.4f;
    HourglassMiddle = glGetUniformLocation(ProgramH, "HourglassMiddle");
    glUniform1f(HourglassMiddle, hourglassMiddle);
    HourglassTop = glGetUniformLocation(ProgramH, "HourglassTop");
    glUniform1f(HourglassTop, hourglassTop);
    HourglassBottom = glGetUniformLocation(ProgramH, "HourglassBottom");
    glUniform1f(HourglassBottom, hourglassBottom);

    // Retrieve transformation uniform variable locations
    ModelViewH = glGetUniformLocation(ProgramH, "ModelView");
    ProjectionH = glGetUniformLocation(ProgramH, "Projection");
    IsWire = glGetUniformLocation(ProgramH, "IsWire");
    Counter = glGetUniformLocation(ProgramH, "Counter");
}//end setupHourglassColour

// Used to load the buffers for the curtain, as necessary
void loadCurtainBuffer(GLuint vPosition) {
    // Here for size of stack allocation
    GLuint buffer;

    //**************Curtain second****************
    // Loading in the buffer for the curtain
    glBindVertexArray(VAOs[1]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the curtain
    glBufferData(GL_ARRAY_BUFFER, sizeof(curtainVertices), curtainVertices, GL_STATIC_DRAW);
    // Another for the curtain index buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(curtainIndices), curtainIndices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}//end loadCurtainBuffer

//Code taken from example0 to setup the shading for the curtain
void setupCurtainShading() {
    // Initialize shader lighting parameters
    glm::vec4 light_position( 0.0, 0.0, -1.0, 0.0 );
    glm::vec4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
    glm::vec4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    glm::vec4 light_specular( 1.0, 1.0, 1.0, 1.0 );

    // Going for a ruby-ish colour
    glm::vec4 material_ambient( 0.1745, 0.01175, 0.01175, 1.0 );
    glm::vec4 material_diffuse( 0.61424, 0.04136, 0.04136, 1.0 );
    glm::vec4 material_specular( 0.727811, 0.626959, 0.626959, 1.0 );
    float material_shininess = 60.0;

    glm::vec4 ambient_product = light_ambient * material_ambient;
    glm::vec4 diffuse_product = light_diffuse * material_diffuse;
    glm::vec4 specular_product = light_specular * material_specular;

    glUniform4fv( glGetUniformLocation(ProgramC, "AmbientProduct"), 1, glm::value_ptr(ambient_product) );
    glUniform4fv( glGetUniformLocation(ProgramC, "DiffuseProduct"), 1, glm::value_ptr(diffuse_product) );
    glUniform4fv( glGetUniformLocation(ProgramC, "SpecularProduct"), 1, glm::value_ptr(specular_product) );
        
    glUniform4fv( glGetUniformLocation(ProgramC, "LightPosition"), 1, glm::value_ptr(light_position) );
    glUniform1f( glGetUniformLocation(ProgramC, "Shininess"), material_shininess );
    glUniform1f( glGetUniformLocation(ProgramC, "Distance"), 1.0f/CURTAIN_CURVES );

    
    // Retrieve transformation uniform variable locations
    ModelViewC = glGetUniformLocation(ProgramC, "ModelView");
    ModelViewInverseTranspose = glGetUniformLocation(ProgramC, "ModelViewInverseTranspose");
    ProjectionC = glGetUniformLocation(ProgramC, "Projection");
    Time = glGetUniformLocation(ProgramC, "Time");
}//end setupCurtainShading

// OpenGL initialization
void init() {
    buildHourglass();
    //Now we can order all of the indices of the hourglass
    orderHourglassIndices();
    buildCurtain();
    //Now we can order all of the indices of the curtain
    orderCurtainIndices();

    // Create vertex array objects
    glGenVertexArrays(2, VAOs);

    // Load shader set 1
    // First for the hourglass
    ProgramH = InitShader("a2v_hourglass.glsl", "a2f_hourglass.glsl");
    glUseProgram(ProgramH);
    GLuint vPosition = glGetAttribLocation(ProgramH, "vPosition");
 
    loadHourglassBuffer(vPosition);
    setupHourglassColour();


    // Load shader set 2
    // Now the curtain
    ProgramC = InitShader("a2v_curtain.glsl", "a2f_curtain.glsl");
    glUseProgram(ProgramC);
    vPosition = glGetAttribLocation(ProgramC, "vPosition");

    loadCurtainBuffer(vPosition);
    setupCurtainShading();


    // this is used with "flat" in the shaders to get the same solid
    // colour for each face of the cube
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    glEnable(GL_DEPTH_TEST);

    glShadeModel(GL_FLAT);

    glClearColor(1.0, 1.0, 1.0, 1.0);
}//end init


// Helper drawing functions
//----------------------------------------------------------------

//Used for drawing the hourglass in our scene
void drawHourglass( glm::mat4 model_view ) {
    glUseProgram(ProgramH);

    //Setting the color to be the black wire
    glUniform1f(IsWire, true);

    glUniformMatrix4fv(ModelViewH, 1, GL_FALSE, glm::value_ptr(model_view));
    glBindVertexArray(VAOs[0]);

    //Draw and connect all of the vertices together for the wire mesh
    for ( int i=0; i<HOURGLASS_VERTICES; i+=4 ) {
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void *)(i*sizeof(GLuint)));
    }//end for

    //Setting the color to be the glass type
    glUniform1f(IsWire, false);

    //Filling the hourglass
    incrementor += 1.0;
    glUniform1f(Counter, incrementor);

    model_view = model_view * glm::scale(glm::mat4(), glm::vec3(0.9999,0.9999,0.9999));
    glUniformMatrix4fv(ModelViewH, 1, GL_FALSE, glm::value_ptr(model_view));
    int bufferOffset = HOURGLASS_VERTICES;

    for ( int bufferStart = bufferOffset; bufferStart<lastHourglassIndex; bufferStart+=3) {
        glDrawElements( GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_INT, (void *)(bufferStart * sizeof(GLuint)));
    }//end for

}//end drawHourglass

//Used for drawing the curtain in our scene
void drawCurtain ( glm::mat4 model_view ) {
    glUseProgram(ProgramC);

    long ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
    glUniform1f(Time, (ms % 1000000)/1000.0f);
    glUniformMatrix4fv(ModelViewC, 1, GL_FALSE, glm::value_ptr(model_view));
    glUniformMatrix4fv(ModelViewInverseTranspose, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(model_view))));
    glBindVertexArray(VAOs[1]);

    //Draw the colours of the curtain
    for ( int bufferStart=0; bufferStart<lastCurtainIndex; bufferStart+=3 ) {
        glDrawElements(GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_INT, (void *)(bufferStart * sizeof(GLuint)));
    }//end for
}//end drawCurtain


// OpenGL display
//------------------------------------------------------------------
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Generate the model-view matrix
    const glm::vec3 viewer_pos(0.0, 0.0, 2.0);
    const glm::vec3 hourglass_trans(0.0, -0.5, -0.5);
    const glm::vec3 curtain_trans(0.0, 0.0, -3.0);
    
    glm::mat4 trans, cameraRot, rot, model_view;

    //Points the camera down
    cameraRot = glm::rotate(cameraRot, glm::radians(Theta[Xaxis]), glm::vec3(1,0,0));
    
    cameraRot = glm::rotate(cameraRot, glm::radians(Movement[Xaxis]), glm::vec3(1,0,0));
    cameraRot = glm::rotate(cameraRot, glm::radians(Movement[Yaxis]), glm::vec3(0,1,0));
    cameraRot = glm::rotate(cameraRot, glm::radians(Movement[Zaxis]), glm::vec3(0,0,1));
    //Resetting the movement of the camera so it doesn't keep going
    Movement[Xaxis], Movement[Yaxis], Movement[Zaxis] = 0.0f;

    //Modify the view for the models
    trans = glm::translate(glm::mat4(), -viewer_pos);

    //Spins the hourglass
    rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0,1,0));
    model_view = trans * cameraRot * rot * glm::translate(glm::mat4(), hourglass_trans);

    //Drawing the hourglass
    drawHourglass(model_view);
    
    //Modifying the view for the curtain
    model_view = trans * cameraRot * glm::translate(glm::mat4(), curtain_trans);

    //Drawing the curtain
    drawCurtain(model_view);

    glutSwapBuffers();
}//end display


//Other OpenGL drawing functions
//----------------------------------------------------------------------------
void update(void) {
    //Update the rotation of the vertices for the hourglass.
    Theta[Axis] += 0.5;

    if ( Theta[Axis] > 360.0 ) {
        Theta[Axis] -= 360.0;
    }//end if
}//end update


//For mouse inputs
//Clicking on any point of the screen will cause the camera to move in that direction.
void mouse(int button, int state, int x, int y) {
    float rate = 15.0f;
    if ( state==GLUT_DOWN) {
        float centreX = (float)windowWidth/2.0f;
        float centreY = (float)windowHeight/2.0f;

        //Convert x and y to have the origin at the centre of the screen
        float offsetX = centreX - x;
        float offsetY = y - centreY; //Account for OpenGL coordinate system
        
        float newX = offsetX/centreX;
        float newY = offsetY/centreY;

        //x and y values are swapped because of how rotations work
        if ( (CameraPosition.x + newX*rate) < EXTREME && (CameraPosition.x + newX*rate) >- EXTREME) {
            CameraPosition.x += newX*rate;
            Movement[Yaxis] += newX*rate;
        }//end if
        if ( (CameraPosition.y - newY*rate) > -EXTREME && (CameraPosition.y - newY*rate) < EXTREME ) {
            CameraPosition.y -= newY*rate;
            Movement[Xaxis] -= newY*rate;
        }//end if
    }//end if
}//end mouse


//Rotate the camera in some reasonable direction. It affects all geometry.
//w = up, a = left, s = down, d = right
//x = reset the camera to the original position
void keyboard(unsigned char key, int x, int y) {
    float rate = 15.0f;
    switch (key) {
        case 033: // Escape Key
        case 'q':
        case 'Q':
            exit(EXIT_SUCCESS);
            break;
        case 'w':
            //Move the camera up, by rotating up on x axis
            if ( CameraPosition.y > -EXTREME ) {
                CameraPosition.y -= rate;
                Movement[Xaxis] += rate;
            }//end if
            break;
        case 'a':
            //Move the camera left, by rotating left on y axis
            if ( CameraPosition.x < EXTREME ) {
                CameraPosition.x += rate;
                Movement[Yaxis] -= rate;
            }//end if
            break;
        case 's':
            //Move the camera down, by rotating down on x axis
            if ( CameraPosition.y < EXTREME ) {
                CameraPosition.y += rate;
                Movement[Xaxis] -= rate;
            }//end if
            break;
        case 'd':
            //Move the camera right, by rotating right on y axis
            if ( CameraPosition.x > -EXTREME ) {
                CameraPosition.x -= rate;
                Movement[Yaxis] += rate;
            }//end if
            break;
        case 'x':
            //Reset the camera position
            CameraPosition = glm::vec3();
            Movement[Xaxis] = 0.0;
            Movement[Yaxis] = 0.0;
            Movement[Zaxis] = 0.0;
            break;
        case ' ':
            //Reset the sand
            incrementor = 0.0f;
            break;
    }//end switch-case
}//end keyboard


//Kept here as black magic
void reshape (int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    glm::mat4  projection = glm::perspective( glm::radians(45.0f), aspect, 0.5f, 8.0f );

    glUseProgram(ProgramH);
    glUniformMatrix4fv( ProjectionH, 1, GL_FALSE, glm::value_ptr(projection) );
    glUseProgram(ProgramC);
    glUniformMatrix4fv( ProjectionC, 1, GL_FALSE, glm::value_ptr(projection) );
}//end reshape
