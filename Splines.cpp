/*----------------------------------------------------
   COMP 4490 Winter 2024 Assignment 3
   Tara Boulanger (7922331)
   John Braico
   File name:   Splines.cpp

   Description: This program draws a 2D set of splines
   that are all controlled by a set of control points.
   We can move these control points to modify the shape
   of the curves.
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


// Control Points requirements
//---------------------------------------------------------------------------
const int CONTROL_POINTS = 4;//12;
const float CP_SIZE = 0.5f; //Just for the line length

const int POINT_VERTICES = 4;
const int CONTROL_POINT_VERTICES = CONTROL_POINTS*POINT_VERTICES;

glm::vec4 controlPointVertices[CONTROL_POINT_VERTICES];
//may not be needed if I don't use draw_elements
int controlPointIndices[CONTROL_POINT_VERTICES];


// Splines requirements
//-----------------------------------------------
const int SPLINE_SEGMENTS = 16;
const int T = 4;
//Maximum number of subdivisions for the curves
const int MAX_SUBDIVISIONS = T*SPLINE_SEGMENTS;
//random logic
const int SPLINE_VERTICES = CONTROL_POINTS*SPLINE_SEGMENTS;

glm::vec4 splineVertices[SPLINE_VERTICES];
//may not be needed if I don't use draw_elements
int splineIndices[SPLINE_VERTICES*3];


// Program-required variables
//----------------------------------------------------------------------------

// Need global access to VAOs
GLuint VAOs[2];

//Probably won't need these
// Array of rotation angles for each coordinate axis, in angles (taken from ex8)
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int Axis = Yaxis;
GLfloat Theta[NumAxes] = { 15.0, 0.0, 0.0 };

enum { Bezier = 0, CatmullRom = 1, UniformRational = 2 };
int mode = Bezier;


// Shader program variables
//------------------------------------------------------------
//S for spline, CP for control points
GLuint ProgramS, ProgramCP;

// Model-view and projection matrices uniform location
GLuint ModelViewS, ModelViewCP;

// Other uniform locations
//GLuint IsWire;



// Hourglass Functions
//----------------------------------------------------------------------------

// Used to set the order of the vertices of the splines for
// drawing the edges.
void orderSplineIndices() {
    int index = 0;
    for ( int i=0; i<SPLINE_VERTICES/4; i++ ) {
        splineIndices[index++] = i;
        splineIndices[index++] = i+SPLINE_VERTICES;
        splineIndices[index++] = i+SPLINE_VERTICES+1;
        splineIndices[index++] = i+1;
    }//end for
}//end orderSplineIndices

// Used to create the vertices along the Bezier spline segment.
// Takes in a variable for the offset position of the curve and
// the starting position of the curve.
void createCurve(int offset, glm::vec4 initialPosition) {
    glm::vec4 curvePoint;
    float x = 0.0f;
    float y = 0.0f;
    for ( int i=0; i<=SPLINE_SEGMENTS; i++ ) {
        break;
    }//end for
}//end createCurve

// Used to actually build all of the vertices of the slpines
// by calling the createCurve function a given amount of times.
// It will then store those vertices into the spline vertices
// array.
void buildSplines() {
    glm::vec4 startPoint;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    for (int i = 0; i<=SPLINE_VERTICES; i++) {
        //Input the 4 point segment we would calculate?
        //Maybe pass all of the indices of those points we'd
        //need to build the curve
        createCurve(i,startPoint);
    }//end for
}//end buildCurve


// Control Point Functions
//---------------------------------------------------------------------------

// Used to get the order of the vertices of the control points.
//This function may not be needed
void orderControlPointIndices() {
    int index = 0;
    for ( int i=0; i<CONTROL_POINT_VERTICES/4; i++ ) {
        //Remains to be changed
        controlPointIndices[index++] = i;
        controlPointIndices[index++] = i+SPLINE_VERTICES;
        controlPointIndices[index++] = i+SPLINE_VERTICES+1;
        controlPointIndices[index++] = i+1;
    }//end for
}//end orderControlPointIndices

// Used to create the vertices for a control point.
// Takes in a variable for the offset position of the control
// point and the starting position of the point.
void createControlPoint(int offset, glm::vec4 initialPosition) {
    glm::vec4 controlPoint;
    float x = 0.0f;
    float z = 0.0f;
    for ( int i=0; i<=POINT_VERTICES; i++ ) {
        controlPointVertices[offset+(i*POINT_VERTICES)] = controlPoint;
    }//end for
}//end createControlPoint

// Used to build all of the vertices of the control points
// by calling the createControlPoint function a given amount of times.
// It will then store those vertices into the control point vertices
// array.
void buildControlPoints() {
    glm::vec4 startPoint;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    for (int i = 0; i<=CONTROL_POINTS; i++) {
        //Creating a line on the x-y plane
        startPoint = glm::vec4(x,y,z,1);
        createControlPoint(i,startPoint);
    }//end for
}//end buildControlPoints


// Start of OpenGL drawing
//-------------------------------------------------------------------

// Used to load the buffers for the splines, as necessary
void loadSplinesBuffer(GLuint vPosition) {
    // Here for size of stack allocation
    GLuint buffer;

    //**************Splines first****************
    // Loading in the buffer for the spline
    glBindVertexArray(VAOs[0]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the splines
    glBufferData(GL_ARRAY_BUFFER, sizeof(splineVertices), splineVertices, GL_STATIC_DRAW);
    // Another for the splines index buffer
    //THESE MAY NOT BE NEEDED
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(splineIndices), splineIndices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}//end loadSplinesBuffer

// Used to initialize the variables for the splines from the shaders
void setupSplinesShaders() {
    // Retrieve transformation uniform variable locations
    ModelViewS = glGetUniformLocation(ProgramS, "ModelView");
}//end setupSplinesShaders

// Used to load the buffers for the control points, as necessary
void loadControlPointsBuffer(GLuint vPosition) {
    // Here for size of stack allocation
    GLuint buffer;

    //**************Control Points second****************
    // Loading in the buffer for the control points
    glBindVertexArray(VAOs[1]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the control points
    glBufferData(GL_ARRAY_BUFFER, sizeof(controlPointVertices), controlPointVertices, GL_STATIC_DRAW);
    // Another for the curtain index buffer
    //MAY NOT BE NEEDED
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(controlPointIndices), controlPointIndices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}//end loadControlPointsBuffer

//Setup the shaders variables for the control points 
void setupControlPointsShader() {
    // Retrieve transformation uniform variable locations
    ModelViewCP = glGetUniformLocation(ProgramCP, "ModelView");
}//end setupControlPointsShader

// OpenGL initialization
void init() {
    buildSplines();
    //Now we can order all of the indices of the splines
    orderSplineIndices();
    buildControlPoints();
    //Now we can order all of the indices of the control points
    orderControlPointIndices();

    // Create vertex array objects
    glGenVertexArrays(2, VAOs);

    // Load shader set 1
    // First for the hourglass
    ProgramS = InitShader("a3v_splines.glsl", "a3f_splines.glsl");
    glUseProgram(ProgramS);
    GLuint vPosition = glGetAttribLocation(ProgramS, "vPosition");
 
    loadSplinesBuffer(vPosition);
    setupSplinesShaders();


    // Load shader set 2
    // Now the curtain
    ProgramCP = InitShader("a3v_cps.glsl", "a3f_cps.glsl");
    glUseProgram(ProgramCP);
    vPosition = glGetAttribLocation(ProgramCP, "vPosition");

    loadControlPointsBuffer(vPosition);
    setupControlPointsShader();


    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    glEnable(GL_DEPTH_TEST);

    glShadeModel(GL_FLAT);

    glClearColor(1.0, 1.0, 1.0, 1.0);
}//end init


// Helper drawing functions
//----------------------------------------------------------------

//Used for drawing the splines in our scene
void drawSplines( glm::mat4 model_view ) {
    glUseProgram(ProgramS);

    //Setting the color to be the black wire
    //glUniform1f(IsWire, true);

    glUniformMatrix4fv(ModelViewS, 1, GL_FALSE, glm::value_ptr(model_view));
    glBindVertexArray(VAOs[0]);

    //Draw and connect all of the vertices together for the wire mesh
    for ( int i=0; i<SPLINE_VERTICES; i+=4 ) {
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void *)(i*sizeof(GLuint)));
    }//end for
}//end drawSplines

//Used for drawing the control points in our scene
void drawControlPoints ( glm::mat4 model_view ) {
    glUseProgram(ProgramCP);

    glUniformMatrix4fv(ModelViewCP, 1, GL_FALSE, glm::value_ptr(model_view));
    glBindVertexArray(VAOs[1]);

    //Draw the control points
    for ( int bufferStart=0; bufferStart<CONTROL_POINT_VERTICES; bufferStart+=2 ) {
        //Rework logic
        glDrawElements(GL_TRIANGLE_STRIP, 2, GL_UNSIGNED_INT, (void *)(bufferStart * sizeof(GLuint)));
    }//end for
}//end drawControlPoints


// OpenGL display
//------------------------------------------------------------------
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Generate the model-view matrix
    const glm::vec3 viewer_pos(0.0, 0.0, 2.0);
    
    glm::mat4 trans, cameraRot, rot, model_view;

    //Drawing the splines
    drawSplines(model_view);
    
    //Drawing the control points
    drawControlPoints(model_view);

    glutSwapBuffers();
}//end display


//Other OpenGL drawing functions
//----------------------------------------------------------------------------
void update(void) {
    //Nothing needs to go in here...?
}//end update


//For mouse inputs
//Clicking on any control point will cause us to move that point to a specific
//spot on the screen.
void mouse(int button, int state, int x, int y) {
    if ( state==GLUT_DOWN) {

    }//end if
}//end mouse


//Space bar cycles between curve types.
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 033: // Escape Key
        case 'q':
        case 'Q':
            exit(EXIT_SUCCESS);
            break;
        case ' ':
            //Switch between the splines modes
            mode++;
            if ( mode-3 == 0 ) {
                mode = 0;
            }//end if
            break;
    }//end switch-case
}//end keyboard


//Kept here as black magic, may need to be modified?
void reshape (int width, int height) {
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    //glm::mat4  projection = glm::perspective( glm::radians(45.0f), aspect, 0.5f, 8.0f );
}//end reshape
