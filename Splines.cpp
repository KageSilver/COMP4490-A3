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

const int POINT_VERTICES = 4;
const int CONTROL_POINT_VERTICES = CONTROL_POINTS*POINT_VERTICES;

const float CROSS_LENGTH = 0.05f;
const float PROXIMITY = 3*CROSS_LENGTH;
bool firstClick = true;
int pointClicked = 0;

glm::vec4 controlPointVertices[CONTROL_POINT_VERTICES];


glm::vec4 initialControlPoints[CONTROL_POINTS] = {
    glm::vec4(-0.5, 0.5, 0.0, 1.0),
    glm::vec4( 0.5, 0.5, 0.0, 1.0),
    glm::vec4( 0.5,-0.5, 0.0, 1.0),
    glm::vec4(-0.5,-0.5, 0.0, 1.0)
  /*glm::vec4( -0.6,  0.6, 0.0, 1.0),
    glm::vec4( -0.2,  0.8, 0.0, 1.0),
    glm::vec4(  0.1,  0.6, 0.0, 1.0),
    glm::vec4(  0.5,  0.5, 0.0, 1.0),
    glm::vec4( 0.65,  0.0, 0.0, 1.0),
    glm::vec4(  0.4, -0.4, 0.0, 1.0),
    glm::vec4( 0.25,-0.75, 0.0, 1.0),
    glm::vec4( -0.2, -0.6, 0.0, 1.0),
    glm::vec4(-0.35,-0.35, 0.0, 1.0),
    glm::vec4(-0.75,-0.25, 0.0, 1.0),
    glm::vec4(-0.25,  0.0, 0.0, 1.0),
    glm::vec4( -0.6,  0.2, 0.0, 1.0) */
};


// Splines requirements
//-----------------------------------------------
const int SPLINE_SEGMENTS = 16;
float t = 1.0f;
//Maximum number of subdivisions for the curves
const int MAX_SUBDIVISIONS = t*SPLINE_SEGMENTS;

//random logic
//The times 3 is because we need 3 different sets of vertices
//for the different types of splines
const int SPLINE_VERTICES = CONTROL_POINTS*SPLINE_SEGMENTS*3;

//0 is start, 1 is end
const int BEZIER_VERTICES[2] = {0, SPLINE_VERTICES/3};
const int CATMULL_VERTICES[2] = {SPLINE_VERTICES/3, 2*(SPLINE_VERTICES/3)};
const int UNIFORM_VERTICES[2] = {2*(SPLINE_VERTICES/3), SPLINE_VERTICES-1};

glm::vec4 splineVertices[SPLINE_VERTICES];


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
GLuint ProjectionS, ProjectionCP;


// Splines Functions
//----------------------------------------------------------------------------

// Used to create the vertices along the spline segment.
// Takes in a variable for the offset position of the curve and
// the starting position of the curve.
void createCurve(int offset, glm::vec4 initialPosition) {
    glm::vec4 curvePoint;
    float x = 0.0f;
    float y = 0.0f;
    for ( int i=0; i<=SPLINE_SEGMENTS; i++ ) {
        if ( mode == Bezier ) {
            //compute a bezier curve
        } else if ( mode == CatmullRom ) {
            //compute a Catmull-Rom curve
        } else {
            //compute a Uniform Rational Bezier curve
        }//end if-else
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
    //Build for Bezier first
    mode = Bezier;
    for (int i = BEZIER_VERTICES[0]; i<BEZIER_VERTICES[1]; i++) {
        //Input the 4 point segment we would calculate?
        //Maybe pass all of the indices of those points we'd
        //need to build the curve
        createCurve(i, startPoint);
    }//end for

    //Build for Catmull-Rom Second
    mode = CatmullRom;
    for (int i = CATMULL_VERTICES[0]; i<CATMULL_VERTICES[1]; i++) {
        //Input the 4 point segment we would calculate?
        //Maybe pass all of the indices of those points we'd
        //need to build the curve
        createCurve(i, startPoint);
    }//end for

    //Build for Uniform Rational Bezier last
    mode = UniformRational;
    for (int i = UNIFORM_VERTICES[0]; i<UNIFORM_VERTICES[1]; i++) {
        //Input the 4 point segment we would calculate?
        //Maybe pass all of the indices of those points we'd
        //need to build the curve
        createCurve(i, startPoint);
    }//end for

    //Reset the mode to default
    mode = Bezier;
}//end buildSplines


// Control Point Functions
//---------------------------------------------------------------------------

// Used to create the vertices of the cross for a control point.
// Takes in a variable for the offset vertex position of the control
// point and the starting position of the point.
void createControlPoint(int offset, glm::vec4 initialPosition) {
    controlPointVertices[offset++] = glm::vec4( initialPosition.x-CROSS_LENGTH,
                                                initialPosition.y+CROSS_LENGTH,
                                                0.0f,1.0f );
    controlPointVertices[offset++] = glm::vec4( initialPosition.x+CROSS_LENGTH,
                                                initialPosition.y+CROSS_LENGTH,
                                                0.0f,1.0f );
    controlPointVertices[offset++] = glm::vec4( initialPosition.x+CROSS_LENGTH,
                                                initialPosition.y-CROSS_LENGTH,
                                                0.0f,1.0f );
    controlPointVertices[offset] = glm::vec4( initialPosition.x-CROSS_LENGTH,
                                                initialPosition.y-CROSS_LENGTH,
                                                0.0f,1.0f );
}//end createControlPoint

// Used to build all of the vertices of the control points
// by calling the createControlPoint function a given amount of times
// which stores those vertices into the control point vertices
// array.
void buildControlPoints() {
    for (int i = 0; i<CONTROL_POINTS; i++) {
        createControlPoint(i,initialControlPoints[i]);
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
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}//end loadSplinesBuffer

// Called after a control point has been moved
void reloadSplinesBuffer() {

}//end reloadSplinesBuffer

// Used to initialize the variables for the splines from the shaders
void setupSplinesShaders() {
    // Retrieve transformation uniform variable locations
    ModelViewS = glGetUniformLocation(ProgramS, "ModelView");
    ProjectionS = glGetUniformLocation(ProgramS, "Projection");
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
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}//end loadControlPointsBuffer

//Only needed if we add the aditional control point function for A++
void reloadControlPointsBuffer() {

}//end reloadControlPointsBuffer

//Setup the shaders variables for the control points 
void setupControlPointsShader() {
    // Retrieve transformation uniform variable locations
    ModelViewCP = glGetUniformLocation(ProgramCP, "ModelView");
    ProjectionCP = glGetUniformLocation(ProgramCP, "Projection");
}//end setupControlPointsShader

// OpenGL initialization
void init() {
    buildSplines();
    buildControlPoints();

    // Create vertex array objects
    glGenVertexArrays(2, VAOs);

    // Load shader set 1
    // First for the splines
    ProgramS = InitShader("a3v_splines.glsl", "a3f_splines.glsl");
    glUseProgram(ProgramS);
    GLuint vPosition = glGetAttribLocation(ProgramS, "vPosition");
 
    loadSplinesBuffer(vPosition);
    setupSplinesShaders();

    // Load shader set 2
    // Now the control points
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

    glUniformMatrix4fv(ModelViewS, 1, GL_FALSE, glm::value_ptr(model_view));
    glBindVertexArray(VAOs[0]);

    //Setting which types of splines we're drawing based on what mode we're in
    int start = 0;
    int end = 0;

    if ( mode == Bezier ) {
        start = BEZIER_VERTICES[0];
        end = BEZIER_VERTICES[1];
    } else if ( mode == CatmullRom ) {
        start = CATMULL_VERTICES[0];
        end = CATMULL_VERTICES[1];
    } else {
        start = UNIFORM_VERTICES[0];
        end = UNIFORM_VERTICES[1];
    }//end if-else

    //Draw all of the lines to make up the spline
    for ( int i=start; i<end; i+=2 ) {
        glDrawElements(GL_LINE_STRIP, 2, GL_UNSIGNED_INT, (void *)(i*sizeof(GLuint)));
    }//end for
}//end drawSplines

//Used for drawing the control points in our scene
void drawControlPoints ( glm::mat4 model_view ) {
    glUseProgram(ProgramCP);

    glUniformMatrix4fv(ModelViewCP, 1, GL_FALSE, glm::value_ptr(model_view));
    glBindVertexArray(VAOs[1]);

    //Draw the control points
    for ( int i=0; i<CONTROL_POINT_VERTICES; i+=2 ) {
        glDrawElements(GL_LINE_STRIP, 2, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
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


//For mouse inputs
//Clicking on any control point will cause us to move that point to
//the next spot that is clicked
void mouse(int button, int state, int x, int y) {
    if ( state==GLUT_DOWN) {
        if ( firstClick ) {
            bool found = false;
            for ( int i=0; i<CONTROL_POINTS && !found; i++ ) {
                //Checking x
                if ( controlPointVertices[i].x-PROXIMITY <= x && controlPointVertices[i].x+PROXIMITY >= x ) {
                    //Checking y
                    if ( controlPointVertices[i].y-PROXIMITY <= y && controlPointVertices[i].y+PROXIMITY >= y ) {
                        firstClick = false;
                        found = true;
                        pointClicked = i;
                    }//end if
                }//end if
            }//end for
        } else {
            controlPointVertices[pointClicked].x = x;
            controlPointVertices[pointClicked].y = y;
            reloadSplinesBuffer();
            firstClick = true;
        }//end if-else
    }//end if
}//end mouse


//Space bar cycles between curve types.
//Number keys change the t-values accordingly.
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
        case '1':
            t = 1.0f;
            break;
        case '2':
            t = 0.75f;
            break;
        case '3':
            t = 0.5f;
            break;
        case '4':
            t = 0.2f;
            break;
        case '5':
            t = 0.05f;
            break;    
    }//end switch-case
}//end keyboard


//Kept here as black magic, may need to be modified?
void reshape (int width, int height) {
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    glm::mat4  projection = glm::ortho( glm::radians(45.0f), aspect, 0.5f, 8.0f );

    glUseProgram(ProgramS);
    glUniformMatrix4fv(ProjectionS, 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(ProgramCP);
    glUniformMatrix4fv(ProjectionCP, 1, GL_FALSE, glm::value_ptr(projection));
}//end reshape
