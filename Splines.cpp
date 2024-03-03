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
float T_VALUES[5] = {1.0f,0.75f,0.5f,0.2f,0.05f};
//Maximum number of subdivisions for the curves
const int MAX_SUBDIVISIONS = 20;

const int SPLINE_VERTICES = CONTROL_POINTS*MAX_SUBDIVISIONS;

glm::vec4 bezierVertices[SPLINE_VERTICES];
glm::vec4 catmullVertices[SPLINE_VERTICES];
glm::vec4 uniformVertices[SPLINE_VERTICES];


// Program-required variables
//----------------------------------------------------------------------------

// Need global access to VAOs
GLuint VAOs[4];

enum { Bezier = 0, CatmullRom = 1, UniformRational = 2 };
int mode = Bezier;

enum { t1 = 0, t2 = 1, t3 = 2, t4 = 3, t5 = 4 };
float t = T_VALUES[t1];
int tMode = t1;

// Shader program variables
//------------------------------------------------------------
//S for spline, CP for control points
GLuint ProgramS, ProgramCP;

// Model-view and projection matrices uniform location
GLuint ModelViewS, ModelViewCP;
GLuint ProjectionS, ProjectionCP;


// Splines Functions
//----------------------------------------------------------------------------

int counter = 0;
// Used to create the vertices along a bezier spline segment.
// Takes in a variable for the offset positions of the control points.
void createBezierCurve(int p0, int p1, int p2, int p3) {
    float x = 0;
    float y = 0;
    for ( float i=0; i<1; i+=t ) {
        x = pow((1-i),3)*controlPointVertices[p0].x +
            3*pow((1-i),2)*i*controlPointVertices[p1].x +
            3*(1-i)*pow(i,2)*controlPointVertices[p2].x +
            pow(i,3)*controlPointVertices[p3].x;
        y = pow((1-i),3)*controlPointVertices[p0].y +
            3*pow((1-i),2)*i*controlPointVertices[p1].y +
            3*(1-i)*pow(i,2)*controlPointVertices[p2].y +
            pow(i,3)*controlPointVertices[p3].y;
        bezierVertices[counter++] = glm::vec4(x,y,0.0,1.0);
    }//end for
}//end createBezierCurve

int counter = 0;
// Used to create the vertices along a catmull-rom spline segment.
// Takes in a variable for the offset position of the control points.
void createCatmullCurve(int p0, int p1, int p2, int p3) {
    float x = 0;
    float y = 0;
    for ( float i=0; i<1; i+=t ) {
        x = pow(i,3)*(-controlPointVertices[p0].x +
             3*controlPointVertices[p1].x -
             3*controlPointVertices[p2].x + 
             controlPointVertices[p3].x);
        x += pow(i,2)*(2*controlPointVertices[p0].x -
             5*controlPointVertices[p1].x +
             4*controlPointVertices[p2].x - 
             controlPointVertices[p3].x);
        x += i*(-controlPointVertices[p0].x +
             controlPointVertices[p2].x);
        x += 2*controlPointVertices[p1].x;
        x/= 2;
        y = pow(i,3)*(-controlPointVertices[p0].y +
             3*controlPointVertices[p1].y -
             3*controlPointVertices[p2].y + 
             controlPointVertices[p3].y);
        y += pow(i,2)*(2*controlPointVertices[p0].y -
             5*controlPointVertices[p1].y +
             4*controlPointVertices[p2].y - 
             controlPointVertices[p3].y);
        y += i*(-controlPointVertices[p0].y +
             controlPointVertices[p2].y);
        y += 2*controlPointVertices[p1].y;
        y/= 2;
        catmullVertices[counter++] = glm::vec4(x,y,0.0,1.0);
    }//end for
}//end createCatmullCurve

int counter = 0;
// Used to create the vertices along a uniform rational B-spline segment.
// Takes in a variable for the offset position of the control points.
void createUniformCurve(int p0, int p1, int p2, int p3) {
    float x = 0;
    float y = 0;
    for ( float i=0; i<=1; i+=t ) {
        x = (-pow(i,3)+3*pow(i,2)-3*i+1)*controlPointVertices[p0].x;
        x += (3*pow(i,3)-6*pow(i,2)+3*t)*controlPointVertices[p1].x;
        x += (-3*pow(i,3)+3*pow(i,2)+3*t)*controlPointVertices[p2].x;
        x += (pow(i,3)+4*pow(i,2)+1)*controlPointVertices[p3].x;
        x /= 6;
        y = (-pow(i,3)+3*pow(i,2)-3*i+1)*controlPointVertices[p0].y;
        y += (3*pow(i,3)-6*pow(i,2)+3*t)*controlPointVertices[p1].y;
        y += (-3*pow(i,3)+3*pow(i,2)+3*t)*controlPointVertices[p2].y;
        y += (pow(i,3)+4*pow(i,2)+1)*controlPointVertices[p3].y;
        y /= 6;
        uniformVertices[counter++] = glm::vec4(x,y,0.0,1.0);
    }//end for
}//end createUniformCurve

// Used to actually build all of the vertices of the splines
// by calling the proper createCurve function for all of the control
// points. It will then store those vertices in their own spline
// vertices array.
void buildSplines() {
    for (int i = 0; i<CONTROL_POINTS; i++) {
        //Setting the control points
        int points[4] = {i-1,i,i+1,i+2};
        if ( i == 0 ) {
            points[0] = CONTROL_POINTS-1;
        } else if ( i+2 % CONTROL_POINTS == 0 ) {
            points[3] = 0;
        } else if ( i+1 % CONTROL_POINTS == 0 ) {
            points[2] = 0;
            points[3] = 1;
        }//end if-else
        createBezierCurve(points[0],points[1],points[2],points[3]);
        createCatmullCurve(points[0],points[1],points[2],points[3]);
        createUniformCurve(points[0],points[1],points[2],points[3]);
    }//end for
}//end buildSplines


// Control Point Functions
//---------------------------------------------------------------------------

int index = 0;
// Used to create the vertices of the cross for a control point.
// Takes in a variable for the starting position of the point.
void createControlPoint(glm::vec4 initialPosition) {
    controlPointVertices[index++] = glm::vec4( initialPosition.x-CROSS_LENGTH,
                                                initialPosition.y+CROSS_LENGTH,
                                                0.0f, 1.0f );
    controlPointVertices[index++] = glm::vec4( initialPosition.x+CROSS_LENGTH,
                                                initialPosition.y+CROSS_LENGTH,
                                                0.0f, 1.0f );
    controlPointVertices[index++] = glm::vec4( initialPosition.x+CROSS_LENGTH,
                                                initialPosition.y-CROSS_LENGTH,
                                                0.0f, 1.0f );
    controlPointVertices[index++] = glm::vec4( initialPosition.x-CROSS_LENGTH,
                                                initialPosition.y-CROSS_LENGTH,
                                                0.0f, 1.0f );
}//end createControlPoint

// Used to build all of the vertices of the control points crosses
// by calling the createControlPoint function a given amount of times
// which stores those vertices into the control point vertices array.
void buildControlPoints() {
    for (int i = 0; i<CONTROL_POINTS; i++) {
        createControlPoint(initialControlPoints[i]);
    }//end for
}//end buildControlPoints

//Used to move one of the control points and set the vertices for
//drawing accordingly.
void updateControlPoint() {

}//end updateControlPoint

// Start of OpenGL drawing
//-------------------------------------------------------------------

// Used to load the buffers for the splines, as necessary
void loadSplinesBuffer(GLuint vPosition) {
    // Here for size of stack allocation
    GLuint buffer;

    //**************Bezier Splines first****************
    // Loading in the buffer for the bezier splines
    glBindVertexArray(VAOs[0]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the splines
    glBufferData(GL_ARRAY_BUFFER, sizeof(bezierVertices), bezierVertices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //**************Catmull-Rom Splines second****************
    // Loading in the buffer for the catmull-rom splines
    glBindVertexArray(VAOs[1]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the splines
    glBufferData(GL_ARRAY_BUFFER, sizeof(catmullVertices), catmullVertices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //**************Uniform Splines third****************
    // Loading in the buffer for the uniform splines
    glBindVertexArray(VAOs[2]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the splines
    glBufferData(GL_ARRAY_BUFFER, sizeof(uniformVertices), uniformVertices, GL_STATIC_DRAW);
    
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

    //**************Control Points first****************
    // Loading in the buffer for the control points
    glBindVertexArray(VAOs[0]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the control points
    glBufferData(GL_ARRAY_BUFFER, sizeof(controlPointVertices), controlPointVertices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}//end loadControlPointsBuffer

// Needed for when one of the control points have been moved
void reloadControlPointsBuffer() {

}//end reloadControlPointsBuffer

// Setup the shaders variables for the control points 
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
    glGenVertexArrays(4, VAOs);

    // Load shader sets 1-3
    // First for the splines
    ProgramS = InitShader("a3v_splines.glsl", "a3f_splines.glsl");
    glUseProgram(ProgramS);
    GLuint vPosition = glGetAttribLocation(ProgramS, "vPosition");
 
    loadSplinesBuffer(vPosition);
    setupSplinesShaders();

    // Load shader set 4
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

    //Setting which types of splines we're drawing based on what mode we're in
    if ( mode == Bezier ) {
        glBindVertexArray(VAOs[0]);
    } else if ( mode == CatmullRom ) {
        glBindVertexArray(VAOs[1]);
    } else {
        glBindVertexArray(VAOs[2]);
    }//end if-else

    //Draw all of the lines to make up the spline
    for ( int i=0; i<SPLINE_VERTICES; i+=2 ) {
        glDrawElements(GL_LINE_STRIP, 2, GL_UNSIGNED_INT, (void *)(i*sizeof(GLuint)));
    }//end for
}//end drawSplines

//Used for drawing the control points in our scene
void drawControlPoints ( glm::mat4 model_view ) {
    glUseProgram(ProgramCP);

    glUniformMatrix4fv(ModelViewCP, 1, GL_FALSE, glm::value_ptr(model_view));
    glBindVertexArray(VAOs[3]);

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
            reloadControlPointsBuffer();
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
            if ( mode == 3 ) {
                mode = 0;
            }//end if
            break;
        case '1':
            t = T_VALUES[t1];
            break;
        case '2':
            t = T_VALUES[t2];
            break;
        case '3':
            t = T_VALUES[t3];
            break;
        case '4':
            t = T_VALUES[t4];
            break;
        case '5':
            t = T_VALUES[t5];
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
