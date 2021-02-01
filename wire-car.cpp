#include <iostream>
#include <windows.h>
#include <time.h>
#include <math.h>
#define GLUT_DISABLE_ATEXIT_HACK // In case of errors like undefined reference to `__glutInitWithExit@12’, should be before including glut.h 
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glut.h>

constexpr auto PI = 3.14159265;
constexpr auto SCREEN_WIDTH = 600;
constexpr auto SCREEN_HEIGHT = 600;

int delay = 10;

// As alpha increases, the car moves forward. As alpha decreases it goes backwards.
double alfa = 0.0f;

// mouse movement variables
int    fx = 0, fy = 0, fz = 0;
float  sphi = 0.0, stheta = 0.0;
float  sside = 0, sdepth = -5;
float  sx = 0, sy = 0;
bool  mouse_left_click;
int   mouseX, mouseY;

///<<<<<<<<Center of Cube>>>>>>>>
double cntr[3] = { 0.0f, 0.0f, 0.0f };
///<<<<<<<<the coefficients of the cube with respect to Ratio>>>>>>>>
double Ratio = 1; // Coefficient value between cube's edge lengths
double heightlenRatio = 1; // k
double minorlenRatio = 3; // 3k
double majorlenRatio = 4; // 4k

//The radius of circles is always thought to be half the height of the cube.
double r = heightlenRatio * Ratio / 2; // Radius of All Circles

///<<<<<<<<Roots of Cube>>>>>>>>
/// front left bottom -> 0
/// front left top -> 1
/// front right top -> 2
/// front right bottom -> 3
/// back left bottom -> 4
/// back left top -> 5
/// back right top -> 6
/// back right bottom -> 7
double MatriceOfDiagPointsOfCube[8][3];

// CubeRotateAngle represents the angle the car has made with the x-axis. As the angle increases, 
// the car turns to the right. It turns left as the angle decreases.
double CubeRotateAngle = 30.0f;

///<<<<<<<<Tires>>>>>>>>
/// front left -> 0
/// front right -> 1
/// back left -> 2
/// back right -> 3
double TiresCenterPoints[4][3];

// Since the iteration in figure 2 is defined as 20, a matrix of dimensions [20] [3] is formed for each circle.
// For 4 wheels, an array of all wheels of size [4] [20] [3] is obtained.
double MatricesOfTires[4][20][3];
// The reason we want to reach this matrix is that the coordinate information of each point must be 
// known to turn the wheels left and right.

// 2 points are required to create a line in the tire, so 8 points are required for 4 tires.
double SpokesOfFrontLeftTireUp[3];
double SpokesOfFrontLeftTireDown[3];

double SpokesOfFrontRightTireUp[3];
double SpokesOfFrontRightTireDown[3];

double SpokesOfBackLeftTireUp[3];
double SpokesOfBackLeftTireDown[3];

double SpokesOfBackRightTireUp[3];
double SpokesOfBackRightTireDown[3];

///<<<<<<<<Steering Weel>>>>>>>>
// represents the center points of the steering wheel.
double CenterOfSteeringWheel[3];

//represents the point where the steering column meets the axles.
double MiddlePointOfFrontAxe[3];

//The necessary points for drawing the lines inside the steering wheel.
double SteeringWheelNorthPoint[3];
double SteeringWheelSouthPoint[3];
double SteeringWheelWestPoint[3];
double SteeringWheelEastPoint[3];

//It is the angle that allows the car to turn left and right. 
//'x = cx + x' and 'y = cy + x' are used in calculations. 
//This causes all drawn wheels to make an extra 45 degree angle.
//Therefore, when defining beta, it is aimed to deduce that difference.
double beta = CubeRotateAngle - 45;
double InitialBeta = beta;

//It is the flag that questions whether the vehicle is manual or not. If it is true, w key is activated to 
//go forward, d key to go backward. It is necessary to press the m key for this.
bool isManual = FALSE;

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<< myIdle >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//   Animation routine which calls itself after “delay” miliseconds.
void myIdle(int frame)
{
    // If it is manual, control of the car is linked to the user's keyboard controls.
    if (!isManual)
    {
        // In this function, alpha increases continuously, making the tires appear to be turning forward.
        alfa += 10;
        if (alfa > 360) alfa -= 360;
    }
    // Calling Itself
    glutTimerFunc(delay, myIdle, 0);
    glutPostRedisplay();
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<< myinit >>>>>>>>>>>>>>>>>>>>>>>>>>>.
void myInit()
{
    glColor3f(0.0f, 0.0f, 0.0f);  // set color of stuff
    glShadeModel(GL_FLAT);	// or can be GL_SMOOTH

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Produce the perspective projection // Set the camera parameters
    gluPerspective(45.0f, 1.0, 1.0, 100.0);
    gluLookAt(0, -15, -6, 0, -5, 0, 0, 1, 0);

    glMatrixMode(GL_MODELVIEW);

    // Start animation
    myIdle(0);
}

/// <summary>
/// It is a function that manages mouse clicks.
/// </summary>
/// <param name="button">Represent buttons on which the mouse will be pressed. The button parameter is one of GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON.</param>
/// <param name="state">The state parameter is either GLUT_UP or GLUT_DOWN indicating whether the callback was due to a release or press respectively.</param>
/// <param name="x">The x callback parameters indicate the window relative coordinates when the mouse button state changed.</param>
/// <param name="y">The y callback parameters indicate the window relative coordinates when the mouse button state changed.</param>
void MouseClick(int button, int state, int x, int y)
{
    mouseX = x; mouseY = y;

    mouse_left_click = ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN));

    glutPostRedisplay();
}

/// <summary>
/// It is a function that converts mouse click information into action.
/// </summary>
/// <param name="x">x represents the x coordinate of the point where the mouse will be moved.</param>
/// <param name="y">y represents the y coordinate of the point where the mouse will be moved.</param>
void MouseMove(int x, int y)
{
    if (mouse_left_click)
    {
        sphi += (float)(x - mouseX) / 200.0;
        stheta += (float)(mouseY - y) / 200.0;
        // if (stheta<0) stheta=0;
    }
}

/// <summary>
/// It provides key features that allow the user to play with the keyboard controls.
/// </summary>
/// <param name="key">The key callback parameter is the generated ASCII character.</param>
/// <param name="x">x represents the x coordinate of the mouse position when the keyboard is pressed.</param>
/// <param name="y">y represents the y coordinate of the mouse position when the keyboard is pressed.</param>
void Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:  // Escape
        exit(-1);
    case 'a': // Left
    case 'A':
        beta -= 1;
        //When we subtract the beta with the defined value, we get 0. The difference will always give the change in beta. Thus, errors in left-right turning angles are prevented.
        if ((beta - (CubeRotateAngle - 45.0)) < -60.0)
        {
            beta += 1;
        }
        break;
    case 'd': // Right
    case 'D':
        beta += 1;
        if ((beta - (CubeRotateAngle - 45.0)) > 60.0)
        {
            beta -= 1;
        }
        break;
    case 'w': // Forward
    case 'W':
        if (isManual) // If it is manual, control of the car is linked to the user's keyboard controls.
        {
            alfa += 8;
            // Depending on the angle beta, it travels one tenth of the radius of the circle.
            cntr[0] += r / 10.0 * cosf((beta + 45) * PI / 180);
            cntr[1] += r / 10.0 * sinf((beta + 45) * PI / 180);
            CubeRotateAngle -= (CubeRotateAngle - beta - 45) / 40.0;
        }
        break;
    case 's': // Backward
    case 'S':
        if (isManual) // If it is manual, control of the car is linked to the user's keyboard controls.
        {
            alfa -= 8;
            // Depending on the angle CubeRotateAngle, it travels one twenty-five of the radius of the circle.
            cntr[0] -= r / 25.0 * cosf((CubeRotateAngle)*PI / 180);
            cntr[1] -= r / 25.0 * sinf((CubeRotateAngle)*PI / 180);
            CubeRotateAngle += (CubeRotateAngle - beta - 45) / 80.0;
            beta += (CubeRotateAngle - beta - 45) / 40.0;
        }
        break;
    case 'm': // is manual
    case 'M':
        if (!isManual)
        {
            isManual = TRUE;
        }
        else
        {
            isManual = FALSE;
        }
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

/// <summary>
/// It is the function that forms the steering parts of the wire car.
/// </summary>
/// <param name="radius">The radius of the steering wheel.</param>
/// <param name="num_segments">The number of iterations when creating the steering wheel.</param>
void SteeringWheel(double radius, double num_segments)
{
    // Determining the values of Middle Point Of Front Axe.
    MiddlePointOfFrontAxe[0] = (TiresCenterPoints[0][0] + TiresCenterPoints[1][0]) / 2; // x-axis
    MiddlePointOfFrontAxe[1] = (TiresCenterPoints[0][1] + TiresCenterPoints[1][1]) / 2; // y-axis
    MiddlePointOfFrontAxe[2] = (TiresCenterPoints[0][2] + TiresCenterPoints[1][2]) / 2; // z-axis

    // Determining the center values of Steering Wheel.
    CenterOfSteeringWheel[0] = MiddlePointOfFrontAxe[0];                                // x-axis
    CenterOfSteeringWheel[1] = MiddlePointOfFrontAxe[1];                                // y-axis
    CenterOfSteeringWheel[2] = MiddlePointOfFrontAxe[2] - (2 * heightlenRatio * Ratio); // z-axis

    double cx = CenterOfSteeringWheel[0];
    double cy = CenterOfSteeringWheel[1];
    double cz = CenterOfSteeringWheel[2];
    double theta = 2 * PI / float(num_segments); // the angle of rotation required for each iteration.
    double c = cosf(theta);//precalculate the sine and cosine
    double s = sinf(theta);
    float x = radius;
    float y = 0; //we start at angle = 0 
    float t;
    glBegin(GL_LINE_LOOP);
    for (int ii = 0; ii < num_segments; ii++)
    {
        // Drawing circle
        glVertex3f(x + cx, y + cy, cz);//output vertex
        //apply the rotation matrix
        t = x;
        x = c * x - s * y;
        y = s * t + c * y;
    }
    glEnd();

    // Find the northernmost, southernmost, western and eastern points of the steering wheel according to the rotation angle of the cube.
    SteeringWheelNorthPoint[0] = CenterOfSteeringWheel[0] - radius * sinf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelNorthPoint[1] = CenterOfSteeringWheel[1] + radius * cosf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelNorthPoint[2] = CenterOfSteeringWheel[2];

    SteeringWheelSouthPoint[0] = CenterOfSteeringWheel[0] + radius * sinf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelSouthPoint[1] = CenterOfSteeringWheel[1] - radius * cosf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelSouthPoint[2] = CenterOfSteeringWheel[2];

    SteeringWheelWestPoint[0] = CenterOfSteeringWheel[0] + radius * cosf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelWestPoint[1] = CenterOfSteeringWheel[1] + radius * sinf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelWestPoint[2] = CenterOfSteeringWheel[2];

    SteeringWheelEastPoint[0] = CenterOfSteeringWheel[0] - radius * cosf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelEastPoint[1] = CenterOfSteeringWheel[1] - radius * sinf((CubeRotateAngle + beta - InitialBeta) * PI / 180);
    SteeringWheelEastPoint[2] = CenterOfSteeringWheel[2];

    
    glBegin(GL_LINES);
    // Drawing Steering Wheel column
    glVertex3dv(MiddlePointOfFrontAxe);
    glVertex3dv(CenterOfSteeringWheel);

    // Drawing lines in the Steering Wheel.
    glVertex3dv(SteeringWheelNorthPoint);
    glVertex3dv(SteeringWheelSouthPoint);

    glVertex3dv(SteeringWheelWestPoint);
    glVertex3dv(SteeringWheelEastPoint);
    glEnd();
}

/// <summary>
/// It is the function that creates the 4 tires of the car.
/// </summary>
/// <param name="radius">The radius of the steering wheel.</param>
/// <param name="num_segments">The number of iterations when creating the steering wheel.</param>
void Tires(double radius, double num_segments)
{
    double height = heightlenRatio * Ratio;
    double majorlen = majorlenRatio * Ratio;
    double minorlen = minorlenRatio * Ratio;

    double x = majorlen / 2.0;
    double y = minorlen / 2.0 + 0.7;
    double z = height / 2.0;

    // The distance from the center of the wheel to the main center
    double d = sqrt(pow(x, 2) + pow(y, 2));

    double initialAngle = atan(y / x);

    // The angle that the diagonal length of the cube makes with the x-axis is ‘initialAngle + CubeRotateAngle’.
    double coP = cosf(initialAngle + CubeRotateAngle * PI / 180);
    double siP = sinf(initialAngle + CubeRotateAngle * PI / 180);

    double coM = cosf(initialAngle - CubeRotateAngle * PI / 180);
    double siM = sinf(initialAngle - CubeRotateAngle * PI / 180);

    // Rotation
    double c_b = cosf((beta)*PI / 180);//precalculate the sine and cosine
    double s_b = sinf((beta)*PI / 180);

    // Front Left Tire
    TiresCenterPoints[0][0] = cntr[0] + coM * d; TiresCenterPoints[0][1] = cntr[1] - siM * d; TiresCenterPoints[0][2] = cntr[2] + z;
    // Front Right Tire
    TiresCenterPoints[1][0] = cntr[0] + coP * d; TiresCenterPoints[1][1] = cntr[1] + siP * d; TiresCenterPoints[1][2] = cntr[2] + z;
    // Back Left Tire
    TiresCenterPoints[2][0] = cntr[0] - coP * d; TiresCenterPoints[2][1] = cntr[1] - siP * d; TiresCenterPoints[2][2] = cntr[2] + z;
    // Back Right Tire
    TiresCenterPoints[3][0] = cntr[0] - coM * d; TiresCenterPoints[3][1] = cntr[1] + siM * d; TiresCenterPoints[3][2] = cntr[2] + z;

    for (int tireCount = 0; tireCount < 4;tireCount++)
    {
        double cx = TiresCenterPoints[tireCount][0];
        double cy = TiresCenterPoints[tireCount][1];
        double cz = TiresCenterPoints[tireCount][2];
        double theta = 2 * PI / float(num_segments);
        double c = cosf(theta);//precalculated the sine and cosine
        double s = sinf(theta);
        double t;

        double x = radius;//we start at angle = 0 
        double y = 0;

        glBegin(GL_LINE_LOOP);
        for (int ii = 0; ii < num_segments; ii++)
        {
            if (tireCount < 2) // Front Tires
            {
                MatricesOfTires[tireCount][ii][0] = (c_b - s_b) * x + cx;
                MatricesOfTires[tireCount][ii][1] = (c_b + s_b) * x + cy;
                MatricesOfTires[tireCount][ii][2] = y + cz;
            }
            else // Back Tires
            {
                MatricesOfTires[tireCount][ii][0] = cosf(CubeRotateAngle * PI / 180) * x + cx;
                MatricesOfTires[tireCount][ii][1] = sinf(CubeRotateAngle * PI / 180) * x + cy;
                MatricesOfTires[tireCount][ii][2] = y + cz;
            }
            glVertex3f(MatricesOfTires[tireCount][ii][0], MatricesOfTires[tireCount][ii][1], MatricesOfTires[tireCount][ii][2]);//output vertex 
            //apply the rotation matrix
            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
        }
        glEnd();
    }
    double co_a = cosf(alfa * PI / 180);
    double si_a = sinf(alfa * PI / 180);
    // Front Left
    SpokesOfFrontLeftTireUp[0] = TiresCenterPoints[0][0] - ((c_b - s_b) * (r * co_a));
    SpokesOfFrontLeftTireUp[1] = TiresCenterPoints[0][1] - ((c_b + s_b) * (r * co_a));
    SpokesOfFrontLeftTireUp[2] = TiresCenterPoints[0][2] - (r * si_a);

    SpokesOfFrontLeftTireDown[0] = TiresCenterPoints[0][0] + ((c_b - s_b) * (r * co_a));
    SpokesOfFrontLeftTireDown[1] = TiresCenterPoints[0][1] + ((c_b + s_b) * (r * co_a));
    SpokesOfFrontLeftTireDown[2] = TiresCenterPoints[0][2] + (r * si_a);

    // Front Right
    SpokesOfFrontRightTireUp[0] = TiresCenterPoints[1][0] + ((c_b - s_b) * (r * co_a));
    SpokesOfFrontRightTireUp[1] = TiresCenterPoints[1][1] + ((c_b + s_b) * (r * co_a));
    SpokesOfFrontRightTireUp[2] = TiresCenterPoints[1][2] + (r * si_a);

    SpokesOfFrontRightTireDown[0] = TiresCenterPoints[1][0] - ((c_b - s_b) * (r * co_a));
    SpokesOfFrontRightTireDown[1] = TiresCenterPoints[1][1] - ((c_b + s_b) * (r * co_a));
    SpokesOfFrontRightTireDown[2] = TiresCenterPoints[1][2] - (r * si_a);

    // Back Left
    SpokesOfBackLeftTireUp[0] = TiresCenterPoints[2][0] + r * co_a * cosf(CubeRotateAngle * PI / 180);
    SpokesOfBackLeftTireUp[1] = TiresCenterPoints[2][1] + r * co_a * sinf(CubeRotateAngle * PI / 180);
    SpokesOfBackLeftTireUp[2] = TiresCenterPoints[2][2] + r * si_a;

    SpokesOfBackLeftTireDown[0] = TiresCenterPoints[2][0] - r * co_a * cosf(CubeRotateAngle * PI / 180);
    SpokesOfBackLeftTireDown[1] = TiresCenterPoints[2][1] - r * co_a * sinf(CubeRotateAngle * PI / 180);
    SpokesOfBackLeftTireDown[2] = TiresCenterPoints[2][2] - r * si_a;

    // Back Right
    SpokesOfBackRightTireUp[0] = TiresCenterPoints[3][0] + r * co_a * cosf(CubeRotateAngle * PI / 180);
    SpokesOfBackRightTireUp[1] = TiresCenterPoints[3][1] + r * co_a * sinf(CubeRotateAngle * PI / 180);
    SpokesOfBackRightTireUp[2] = TiresCenterPoints[3][2] + r * si_a;

    SpokesOfBackRightTireDown[0] = TiresCenterPoints[3][0] - r * co_a * cosf(CubeRotateAngle * PI / 180);
    SpokesOfBackRightTireDown[1] = TiresCenterPoints[3][1] - r * co_a * sinf(CubeRotateAngle * PI / 180);
    SpokesOfBackRightTireDown[2] = TiresCenterPoints[3][2] - r * si_a;

    glBegin(GL_LINES);
    glVertex3dv(SpokesOfFrontLeftTireUp);
    glVertex3dv(SpokesOfFrontLeftTireDown);

    glVertex3dv(SpokesOfFrontRightTireUp);
    glVertex3dv(SpokesOfFrontRightTireDown);

    glVertex3dv(SpokesOfBackLeftTireUp);
    glVertex3dv(SpokesOfBackLeftTireDown);

    glVertex3dv(SpokesOfBackRightTireUp);
    glVertex3dv(SpokesOfBackRightTireDown);
    glEnd();
}

/// <summary>
/// This function creates a cube in the ratio you specify and the sides with certain coefficients of that ratio.
/// Ratio = k, heightlenRatio = k, minorlenRatio = 2k, majorlenRatio = 4k.
/// </summary>
/// <param name="center">The intersection of the diagonals of the cube, the center.</param>
/// <param name="heightlenRatio">The ratio of the height of the cube.</param>
/// <param name="minorlenRatio">The ratio of the length of the shortest side of the cube at the base.</param>
/// <param name="majorlenRatio">The ratio of the length of the highest side of the cube at the base.</param>
/// <param name="Ratio">The ratio between the sides of the cube.</param>
void DrawCube(double* center, double heightlenRatio, double minorlenRatio, double majorlenRatio, double Ratio)
{
    double cx = center[0];
    double cy = center[1];
    double cz = center[2];

    double height = heightlenRatio * Ratio;
    double majorlen = majorlenRatio * Ratio;
    double minorlen = minorlenRatio * Ratio;

    double x = majorlen / 2.0;
    double y = minorlen / 2.0;
    double z = height / 2.0;

    double initialAngle = atan(y / x);

    double coP = cosf(initialAngle + CubeRotateAngle * PI / 180);
    double siP = sinf(initialAngle + CubeRotateAngle * PI / 180);

    double coM = cosf(initialAngle - CubeRotateAngle * PI / 180);
    double siM = sinf(initialAngle - CubeRotateAngle * PI / 180);

    /// When thought in 2 dimensions, the d represents half the length of diagonals.
    /// ______
    /// |\  /|
    /// | \/ |            ' \ '     ' / '
    /// | /\ |               \  and  /  = represents d.
    /// |/__\|
    double d = sqrt(pow(x, 2) + pow(y, 2));

    // front-left-bottom
    MatriceOfDiagPointsOfCube[0][0] = cntr[0] + coM * d; MatriceOfDiagPointsOfCube[0][1] = cntr[1] - siM * d; MatriceOfDiagPointsOfCube[0][2] = cntr[2] + z;
    // front-left-top
    MatriceOfDiagPointsOfCube[1][0] = cntr[0] + coM * d; MatriceOfDiagPointsOfCube[1][1] = cntr[1] - siM * d; MatriceOfDiagPointsOfCube[1][2] = cntr[2] - z;
    // front-right-top
    MatriceOfDiagPointsOfCube[2][0] = cntr[0] + coP * d; MatriceOfDiagPointsOfCube[2][1] = cntr[1] + siP * d; MatriceOfDiagPointsOfCube[2][2] = cntr[2] - z;
    // front-right-bottom
    MatriceOfDiagPointsOfCube[3][0] = cntr[0] + coP * d; MatriceOfDiagPointsOfCube[3][1] = cntr[1] + siP * d; MatriceOfDiagPointsOfCube[3][2] = cntr[2] + z;
    // back-left-bottom
    MatriceOfDiagPointsOfCube[4][0] = cntr[0] - coP * d; MatriceOfDiagPointsOfCube[4][1] = cntr[1] - siP * d; MatriceOfDiagPointsOfCube[4][2] = cntr[2] + z;
    // back-left-top
    MatriceOfDiagPointsOfCube[5][0] = cntr[0] - coP * d; MatriceOfDiagPointsOfCube[5][1] = cntr[1] - siP * d; MatriceOfDiagPointsOfCube[5][2] = cntr[2] - z;
    // back-right-top
    MatriceOfDiagPointsOfCube[6][0] = cntr[0] - coM * d; MatriceOfDiagPointsOfCube[6][1] = cntr[1] + siM * d; MatriceOfDiagPointsOfCube[6][2] = cntr[2] - z;
    // back-right-bottom
    MatriceOfDiagPointsOfCube[7][0] = cntr[0] - coM * d; MatriceOfDiagPointsOfCube[7][1] = cntr[1] + siM * d; MatriceOfDiagPointsOfCube[7][2] = cntr[2] + z;

    // Drawing Cube
    glBegin(GL_LINE_LOOP);

    // Front Vertical Surface
    glVertex3dv(MatriceOfDiagPointsOfCube[0]);
    glVertex3dv(MatriceOfDiagPointsOfCube[1]);
    glVertex3dv(MatriceOfDiagPointsOfCube[2]);
    glVertex3dv(MatriceOfDiagPointsOfCube[3]);
    glVertex3dv(MatriceOfDiagPointsOfCube[0]);

    // Back Vertical Surface
    glVertex3dv(MatriceOfDiagPointsOfCube[4]);
    glVertex3dv(MatriceOfDiagPointsOfCube[5]);
    glVertex3dv(MatriceOfDiagPointsOfCube[6]);
    glVertex3dv(MatriceOfDiagPointsOfCube[7]);
    glVertex3dv(MatriceOfDiagPointsOfCube[4]);

    // Side Vertical Surfaces
    glVertex3dv(MatriceOfDiagPointsOfCube[5]);
    glVertex3dv(MatriceOfDiagPointsOfCube[1]);
    glVertex3dv(MatriceOfDiagPointsOfCube[2]);
    glVertex3dv(MatriceOfDiagPointsOfCube[6]);
    glVertex3dv(MatriceOfDiagPointsOfCube[7]);
    glVertex3dv(MatriceOfDiagPointsOfCube[3]);

    glEnd();
}

/// <summary>
/// It is the function that forms the axles parts of the wire car.
/// </summary>
void Axles()
{
    glBegin(GL_LINES);
    glVertex3dv(TiresCenterPoints[0]);
    glVertex3dv(TiresCenterPoints[1]);

    glVertex3dv(TiresCenterPoints[2]);
    glVertex3dv(TiresCenterPoints[3]);

    glEnd();
}

//<<<<<<<<<<<<<<<<<<<<<<< Display >>>>>>>>>>>>>>>>>>>>>>>>>>
void Display(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0); // black background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear screen

    // Set the camera location
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(sside, 0, -sdepth);
    glRotatef(-stheta, 1, 0, 0);
    glRotatef(sphi, 0, 1, 0);
    glTranslatef(sx, 0, -sy);

    glColor3f(1, 1, 1);

    //Drawing skeletons of the wire car
    DrawCube(cntr, heightlenRatio, minorlenRatio, majorlenRatio, Ratio);
    Tires(r, 20);
    Axles();
    SteeringWheel(r, 20);

    glutSwapBuffers();
}

////<<<<<<<<<<<<<<<<<<<<<<< main >>>>>>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutInitWindowPosition(0, 0);

    glutCreateWindow("Wire Car");

    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseClick);
    glutMotionFunc(MouseMove);
    myInit();

    glutMainLoop();
}

