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
const char *WINDOW_TITLE = "Movable splines";
const double FRAME_RATE_MS = 1000.0 / 60.0;

int windowWidth = 0;
int windowHeight = 0;


// Control Points requirements
//---------------------------------------------------------------------------
const int CONTROL_POINTS = 12;

const int POINT_VERTICES = 4;
const int CONTROL_POINT_VERTICES = CONTROL_POINTS*POINT_VERTICES;

const float CROSS_LENGTH = 0.05f;
const float PROXIMITY = 2*CROSS_LENGTH;
bool firstClick = true;
int pointClicked = 0;

glm::vec4 controlPointVertices[CONTROL_POINT_VERTICES];


glm::vec4 initialControlPoints[CONTROL_POINTS] = {
    /*glm::vec4(-0.5, 0.5, 0.0, 1.0),
    glm::vec4( 0.5, 0.5, 0.0, 1.0),
    glm::vec4( 0.5,-0.5, 0.0, 1.0),
    glm::vec4(-0.5,-0.5, 0.0, 1.0)*/
  glm::vec4( -0.6,  0.6, 0.0, 1.0),
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
    glm::vec4( -0.6,  0.2, 0.0, 1.0) 
};


// Splines requirements
//-----------------------------------------------
//Maximum number of subdivisions for the curves
int T_SUBDIVISIONS[5] = {1,2,4,10,20};
const int TOTAL_SUBDIVISIONS = (4+8+12+16+20)*2;

const int SPLINE_VERTICES = CONTROL_POINTS*TOTAL_SUBDIVISIONS;

glm::vec4 bezierVertices[SPLINE_VERTICES];
int BezierSegments[5] = {0,0,0,0,0};
glm::vec4 catmullVertices[SPLINE_VERTICES];
int CatmullSegments[5] = {0,0,0,0,0};
glm::vec4 uniformVertices[SPLINE_VERTICES];
int UniformSegments[5] = {0,0,0,0,0};


// Bezier
//----------------------------------------------------
glm::mat4 BezierBasis = glm::mat4(-1, 3,-3,1,
                                   3,-6, 3,0,
                                  -3, 3, 0,0,
                                   1, 0, 0,0);

// Catmull-Rom
//-----------------------------------------------------
glm::mat4 BasisCatmull = glm::mat4(-1, 2,-1, 0,
                                    3,-5, 0, 2,
                                   -3, 4, 1, 0,
                                    1,-1, 0, 0);

// Uniform
//------------------------------------------------------
glm::mat4 UniformBasis = glm::mat4(-1, 3,-3,1,
                                    3,-6, 3,0,
                                   -3, 0, 3,0,
                                    1, 4, 1,0);

// Program-required variables
//----------------------------------------------------------------------------

// Need global access to VAOs
GLuint VAOs[4];

enum { Bezier = 0, CatmullRom = 1, UniformRational = 2 };
int mode = Bezier;

float T_VALUES[5] = {1.0f,0.5f,0.25f,0.1f,0.05f};
enum { t1 = 0, t2 = 1, t3 = 2, t4 = 3, t5 = 4 };
float t = T_VALUES[t1];
int tMode = t1;

// Shader program variables
//------------------------------------------------------------
//S for spline, CP for control points
GLuint ProgramS, ProgramCP;

// Model-view matrix uniform location
GLuint ModelViewS, ModelViewCP;



// Splines Functions
//----------------------------------------------------------------------------

int counterBezier = 0;
// Used to create the vertices along a bezier spline segment.
// Takes in a variable for the offset positions of the control points.
void createBezierCurve(int p0, int p1, int p2, int p3) {
    float x = 0;
    float y = 0;
    glm::vec4 tValues;
    glm::vec4 xVertex;
    glm::vec4 yVertex;
    glm::mat4 xValues;
    xValues[0] = glm::vec4(initialControlPoints[p0].x,
                            initialControlPoints[p1].x,
                            initialControlPoints[p2].x,
                            initialControlPoints[p3].x);
    glm::mat4 yValues;
    yValues[0] = glm::vec4(initialControlPoints[p0].y,
                            initialControlPoints[p1].y,
                            initialControlPoints[p2].y,
                            initialControlPoints[p3].y);
    for ( float i=0; i<=1; i+=t ) {
        tValues = glm::vec4(pow(i,3),pow(i,2),i,1);
        xVertex = tValues*BezierBasis*xValues[0];
        x = xVertex[0]+xVertex[1]+xVertex[2]+xVertex[3];
        yVertex = tValues*BezierBasis*yValues[0];
        y = yVertex[0]+yVertex[1]+yVertex[2]+yVertex[3];
        bezierVertices[counterBezier++] = glm::vec4(x,y,0.0,1.0);
    }//end for
}//end createBezierCurve

int counterCatmull = 0;
// Used to create the vertices along a catmull-rom spline segment.
// Takes in a variable for the offset position of the control points.
void createCatmullCurve(int p0, int p1, int p2, int p3) {
    float x = 0;
    float y = 0;
    glm::vec4 tValues;
    glm::vec4 xVertex;
    glm::vec4 yVertex;
    glm::mat4 xValues;
    xValues[0] = glm::vec4(initialControlPoints[p0].x,
                                  initialControlPoints[p1].x,
                                  initialControlPoints[p2].x,
                                  initialControlPoints[p3].x);
    glm::mat4 yValues;
    yValues[0] = glm::vec4(initialControlPoints[p0].y,
                                  initialControlPoints[p1].y,
                                  initialControlPoints[p2].y,
                                  initialControlPoints[p3].y);
    for ( float i=0; i<=1; i+=t ) {
        tValues = glm::vec4(pow(i,3),pow(i,2),i,1);
        xVertex = tValues*BasisCatmull*xValues[0];
        x = xVertex[0]+xVertex[1]+xVertex[2]+xVertex[3];
        x /= 2;
        yVertex = tValues*BasisCatmull*yValues[0];
        y = yVertex[0]+yVertex[1]+yVertex[2]+yVertex[3];
        y /= 2;
        catmullVertices[counterCatmull++] = glm::vec4(x,y,0.0,1.0);
    }//end for
}//end createCatmullCurve

//Used research from this link https://www2.cs.uregina.ca/~anima/UniformBSpline.pdf
//for the formulation of the parametric equations.
int counterUniform = 0;
float B0(float i) {
    return pow(1-i,3)/6;
}//end B0
float B1(float i) {
    return (3*pow(i,3)-6*pow(i,2)+4)/6;
}//end B1
float B2(float i) {
    return (-3*pow(i,3)+3*pow(i,2)+3*i+1)/6;
}//end B2
float B3(float i) {
    return pow(i,3)/6;
}//end B3

// Used to create the vertices along a uniform rational B-spline segment.
// Takes in a variable for the offset position of the control points.
void createUniformCurve(int p0, int p1, int p2, int p3) {
    float x = 0.0f;
    float y = 0.0f;
    glm::mat4 xValues;
    xValues[0] = glm::vec4(initialControlPoints[p0].x,
                            initialControlPoints[p1].x,
                            initialControlPoints[p2].x,
                            initialControlPoints[p3].x);
    glm::mat4 yValues;
    yValues[0] = glm::vec4(initialControlPoints[p0].y,
                            initialControlPoints[p1].y,
                            initialControlPoints[p2].y,
                            initialControlPoints[p3].y);
    for ( float i=0; i<=1; i+=t ) {
        x = B0(i)*xValues[0][0]+B1(i)*xValues[0][1]+B2(i)*xValues[0][2]+B3(i)*xValues[0][3];
        y = B0(i)*yValues[0][0]+B1(i)*yValues[0][1]+B2(i)*yValues[0][2]+B3(i)*yValues[0][3];
        uniformVertices[counterUniform++] = glm::vec4(x,y,0.0,1.0);
    }//end for
}//end createUniformCurve

// Used to actually build all of the vertices of the splines
// by calling the proper createCurve function for all of the control
// points. It will then store those vertices in their own spline
// vertices array.
void buildSplines() {
    //Making splines for each t value
    for (int j=0; j<5; j++) {
        t = T_VALUES[j];
        printf("\tJ: %d\n",j);
        for (int i = 0; i<CONTROL_POINTS; i++) {
            //Setting the control points
            int points[4] = {i-1,i,i+1,i+2};
            printf("\t\tI: %d\n",i);
            if ( i == 0 ) {
                points[0] = CONTROL_POINTS-1;
            } else if ( (i+2) % CONTROL_POINTS == 0 ) {
                points[3] = 0;
            } else if ( (i+1) % CONTROL_POINTS == 0 ) {
                points[2] = 0;
                points[3] = 1;
            }//end if-else
            if ( i % 4 == 0 ) {
                createBezierCurve(points[0],points[1],points[2],points[3]);
            }//end if
            createCatmullCurve(points[0],points[1],points[2],points[3]);
            createUniformCurve(points[0],points[1],points[2],points[3]);
        }//end for
        BezierSegments[j] = counterBezier;
        CatmullSegments[j] = counterCatmull;
        UniformSegments[j] = counterUniform;
    }//end for
    bezierVertices[counterBezier] = bezierVertices[0];
    catmullVertices[counterCatmull] = catmullVertices[0];
    uniformVertices[counterUniform] = uniformVertices[0];
    t = T_VALUES[t1];
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
                                                initialPosition.y-CROSS_LENGTH,
                                                0.0f, 1.0f );
    controlPointVertices[index++] = glm::vec4( initialPosition.x+CROSS_LENGTH,
                                                initialPosition.y+CROSS_LENGTH,
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


// Start of OpenGL drawing
//-------------------------------------------------------------------

// Used to load the buffers for the splines, as necessary
void loadSplinesBuffer(GLuint vPosition) {
    // Here for size of stack allocation
    GLuint buffer;

    //**************Bezier Splines second****************
    // Loading in the buffer for the bezier splines
    glBindVertexArray(VAOs[1]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the splines
    glBufferData(GL_ARRAY_BUFFER, sizeof(bezierVertices), bezierVertices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //**************Catmull-Rom Splines third****************
    // Loading in the buffer for the catmull-rom splines
    glBindVertexArray(VAOs[2]);

    // Creating and initializing a buffer object
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Making sure it has enough space for just the splines
    glBufferData(GL_ARRAY_BUFFER, sizeof(catmullVertices), catmullVertices, GL_STATIC_DRAW);
    
    // Set up vertex data for this vao
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //**************Uniform Splines fourth****************
    // Loading in the buffer for the uniform splines
    glBindVertexArray(VAOs[3]);

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
    glUseProgram(ProgramS);
    GLuint vPosition = glGetAttribLocation(ProgramS, "vPosition");
    counterBezier = 0;
    counterCatmull = 0;
    counterUniform = 0;
    buildSplines();
    loadSplinesBuffer(vPosition);
}//end reloadSplinesBuffer

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
void reloadControlPointsBuffer(int pointClicked) {
    glUseProgram(ProgramCP);
    GLuint vPosition = glGetAttribLocation(ProgramCP, "vPosition");
    index = 0;
    buildControlPoints();
    loadControlPointsBuffer(vPosition);
}//end reloadControlPointsBuffer

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
    // Retrieve transformation uniform variable locations
    ModelViewS = glGetUniformLocation(ProgramS, "ModelView");

    // Load shader set 4
    // Now the control points
    ProgramCP = InitShader("a3v_cps.glsl", "a3f_cps.glsl");
    glUseProgram(ProgramCP);
    vPosition = glGetAttribLocation(ProgramCP, "vPosition");

    loadControlPointsBuffer(vPosition);
    // Retrieve transformation uniform variable locations
    ModelViewCP = glGetUniformLocation(ProgramCP, "ModelView");

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
    int start = 0;
    int end = 0;

    if ( tMode == t1 ) {
        if ( mode == Bezier ) {
            glBindVertexArray(VAOs[1]);
            start = 0;
            end = BezierSegments[0];
        } else if ( mode == CatmullRom ) {
            glBindVertexArray(VAOs[2]);
            start = 0;
            end = CatmullSegments[0];
        } else {
            glBindVertexArray(VAOs[3]);
            start = 0;
            end = UniformSegments[0];
        }//end if-else
    } else if ( tMode == t2 ) {
        if ( mode == Bezier ) {
            glBindVertexArray(VAOs[1]);
            start = BezierSegments[0];
            end = BezierSegments[1];
        } else if ( mode == CatmullRom ) {
            glBindVertexArray(VAOs[2]);
            start = CatmullSegments[0];
            end = CatmullSegments[1];
        } else {
            glBindVertexArray(VAOs[3]);
            start = UniformSegments[0];
            end = UniformSegments[1];
        }//end if-else
    } else if ( tMode == t3 ) {
        if ( mode == Bezier ) {
            glBindVertexArray(VAOs[1]);
            start = BezierSegments[1];
            end = BezierSegments[2];
        } else if ( mode == CatmullRom ) {
            glBindVertexArray(VAOs[2]);
            start = CatmullSegments[1];
            end = CatmullSegments[2];
        } else {
            glBindVertexArray(VAOs[3]);
            start = UniformSegments[1];
            end = UniformSegments[2];
        }//end if-else
    } else if ( tMode == t4 ) {
        if ( mode == Bezier ) {
            glBindVertexArray(VAOs[1]);
            start = BezierSegments[2];
            end = BezierSegments[3];
        } else if ( mode == CatmullRom ) {
            glBindVertexArray(VAOs[2]);
            start = CatmullSegments[2];
            end = CatmullSegments[3];
        } else {
            glBindVertexArray(VAOs[3]);
            start = UniformSegments[2];
            end = UniformSegments[3];
        }//end if-else
    } else {
        if ( mode == Bezier ) {
            glBindVertexArray(VAOs[1]);
            start = BezierSegments[3];
            end = BezierSegments[4];
        } else if ( mode == CatmullRom ) {
            glBindVertexArray(VAOs[2]);
            start = CatmullSegments[3];
            end = CatmullSegments[4];
        } else {
            glBindVertexArray(VAOs[3]);
            start = UniformSegments[3];
            end = UniformSegments[4];
        }//end if-else
    }//end if-else

    //Draw all of the lines to make up the spline
    //printf("End: %d\n", end);
    for ( int i=start; i<end; i++ ) {
        glDrawArrays(GL_LINES, i, 2);
    }//end for
}//end drawSplines

//Used for drawing the control points in our scene
void drawControlPoints ( glm::mat4 model_view ) {
    glUseProgram(ProgramCP);

    glUniformMatrix4fv(ModelViewCP, 1, GL_FALSE, glm::value_ptr(model_view));
    glBindVertexArray(VAOs[0]);

    //Draw the control points
    for ( int i=0; i<CONTROL_POINT_VERTICES; i+=2 ) {
        glDrawArrays(GL_LINE_STRIP, i, 2);
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

void update(void){}

//For mouse inputs
//Clicking on any control point will cause us to move that point to
//the next spot that is clicked
void mouse(int button, int state, int x, int y) {
    if ( state==GLUT_DOWN) {
        float centreX = (float)windowWidth/2.0f;
        float centreY = (float)windowHeight/2.0f;

        //Convert x and y to have the origin at the centre of the screen
        float offsetX = centreX - x;
        float offsetY = y - centreY; //Account for OpenGL coordinate system
        
        float newX = offsetX/centreX;
        float newY = offsetY/centreY;

        newY = -newY;
        newX = -newX;

        if ( firstClick ) {
            bool found = false;
            for ( int i=0; i<CONTROL_POINTS && !found; i++ ) {
                //Checking x
                if ( initialControlPoints[i].x-PROXIMITY <= newX && initialControlPoints[i].x+PROXIMITY >= newX ) {
                    //Checking y
                    if ( initialControlPoints[i].y-PROXIMITY <= newY && initialControlPoints[i].y+PROXIMITY >= newY ) {
                        firstClick = false;
                        found = true;
                        pointClicked = i;
                    }//end if
                }//end if
            }//end for
        } else {
            initialControlPoints[pointClicked].x = newX;
            initialControlPoints[pointClicked].y = newY;
            reloadControlPointsBuffer(pointClicked);
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
                mode = Bezier;
            }//end if
            break;
        case '1':
            t = T_VALUES[t1];
            tMode = t1;
            break;
        case '2':
            t = T_VALUES[t2];
            tMode = t2;
            break;
        case '3':
            t = T_VALUES[t3];
            tMode = t3;
            break;
        case '4':
            t = T_VALUES[t4];
            tMode = t4;
            break;
        case '5':
            t = T_VALUES[t5];
            tMode = t5;
            break;    
    }//end switch-case
}//end keyboard


//Kept here as black magic, may need to be modified?
void reshape (int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
}//end reshape
