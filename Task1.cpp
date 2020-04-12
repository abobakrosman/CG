#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>


GLsizei winWidth = 800, winHeight = 600;



vec2 inputs[7] = {
    {235.0f, 330.0f},//p1
    {200.0f, 454.0f},//p2
    {281.0f, 487.0f},//p3 hermit3
    
    {281.0f, 487.0f}, // p4 c0 connectivity at the beginning 
    {525.0f, 354.0f},  //p5

    {100.0f,425.0f},//v1 hermit 3
    {602.0f , 288.0f}//hermit2 v2

}; 

vec2 tangent;
vec2 v1;
vec2 v2;
GLint dragged = -1;

void init() {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, winWidth, 0.0, winHeight);
    glShadeModel(GL_FLAT);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(5.0);
    glLineWidth(1.0);

}

void drawHermite3() {
    
    v1 = inputs[5] - inputs[0];
    
    glColor3f(0, 0, 0);
    glBegin(GL_POINTS);
    for (int i = 0; i < 3; i++)
        glVertex2f(inputs[i].x, inputs[i].y);
    glEnd();

    glBegin(GL_POINTS);
    glVertex2f(inputs[5].x, inputs[5].y);
    glEnd();



    glEnable(GL_LINE);
    glLineStipple(1, 0xFF00);
    glBegin(GL_LINES); // drawing the tangent vectors
    glVertex2f(inputs[0].x, inputs[0].y);
    glVertex2f(inputs[5].x, inputs[5].y);
    glEnd();
    


    mat24 G = { inputs[0],  inputs[1],   inputs[2],  v1 };

    float t[3] = { 0.0f, 1.0f, 2.0f };

    vec4 columns[4];

    for (int i = 0; i < 3; i++) {
        columns[i] = vec4(t[i] * t[i] * t[i], t[i] * t[i], t[i], 1);
    }
    columns[3] = vec4(3 * t[0] * t[0], 2 * t[0], 1, 0);

    mat4 M = { columns[0],  columns[1],  columns[2],  columns[3], true };
    mat4 M_ = inverse(M);
    mat24 C = G * M_;

    glColor3f(1, 0, 0);
    glBegin(GL_LINE_STRIP);
    for (float t = 0; t <= 2.0; t += 0.01) {
        vec4 T = { t * t * t, t * t, t, 1 };
        vec2 curvePoint = C * T;
        glVertex2f(curvePoint.x, curvePoint.y);
    }

    glEnd();

    vec4 T_ = { 3 * t[2] * t[2], 2 * t[2], 1, 0 };
    tangent = C * T_;




}

void drawHermite2WithMat() {
    // recalculate v2 from the endpoints
    v2 = inputs[6] - inputs[4];
    
    glColor3f(0, 0, 0);
    glBegin(GL_POINTS);
    for (int i = 3; i <= 4; i++)
        glVertex2f(inputs[i].x, inputs[i].y);
    glEnd();

    glBegin(GL_POINTS);
    glVertex2f(inputs[6].x, inputs[6].y);
    glEnd();

 
    glLineStipple(1, 0xFF00);
    glBegin(GL_LINES); // drawing the tangent vectors from the Hermite3
    glVertex2f(inputs[3].x, inputs[3].y);
    glVertex2f(inputs[3].x+tangent.x, inputs[3].y+ tangent.y);

    glVertex2f(inputs[4].x, inputs[4].y);
    glVertex2f(inputs[6].x, inputs[6].y);
    glEnd();
 

    mat24 G = { inputs[3],  inputs[4],  tangent,  v2 }; //2x4 matrix. Filled up with the geometric data (2 points, 2 tangent vectors) in column-major order

    float tvalues[2] = { -3.0f, 2.0f }; // the t paramter values when the curve should go through p1 and p2

    vec4 columns[4];

    columns[0] = vec4( // column for p1 with t1
        tvalues[0] * tvalues[0] * tvalues[0],
        tvalues[0] * tvalues[0],
        tvalues[0],
        1);
    columns[1] = vec4( // column for p2 with t2
        tvalues[1] * tvalues[1] * tvalues[1],
        tvalues[1] * tvalues[1],
        tvalues[1],
        1);
    columns[2] = vec4( // column for v1 with t1
        3 * tvalues[0] * tvalues[0],
        2 * tvalues[0],
        1,
        0);
    columns[3] = vec4(// column for v2 with t2
        3 * tvalues[1] * tvalues[1],
        2 * tvalues[1],
        1,
        0);

    mat4 M = { columns[0],  columns[1],  columns[2],  columns[3], true }; // filling the matrix in column-major order
    mat4 M_ = inverse(M); // invert the matrix

    mat24 C = G * M_; // since G and M do not change, we can calculate here C
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINE_STRIP);
    for (float t = tvalues[0]; t <= tvalues[1]; t += 0.01) { // loop through the interval of t
        vec4 T = { t * t * t, // construct the matrix T with the actual t values
            t * t,
            t,
            1 };
        //mat24 C = G * M_; // it is not efficient to calculate C inside the loop
        vec2 curvePoint = C * T; // C*T=G*M*T --- the final curve point at paramater t
        glVertex2f(curvePoint.x, curvePoint.y);
    }
    glEnd();

}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0, 0.0, 0.0);

    drawHermite3();
    drawHermite2WithMat();

    glutSwapBuffers();

}

GLint getActivePoint1(vec2 p[], GLint size, GLint sens, GLint x, GLint y) {
    GLint i, s = sens * sens;
    vec2 P = { (float)x, (float)y };

    for (i = 0; i < size; i++)
        if (dist(p[i], P) < s)
            return i;
    return -1;
}

void processMouse(GLint button, GLint action, GLint xMouse, GLint yMouse) {
    GLint i;
    if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN)
        if ((i = getActivePoint1(inputs, 7, 5, xMouse, winHeight - yMouse)) != -1)
            dragged = i;
    if (button == GLUT_LEFT_BUTTON && action == GLUT_UP)
        dragged = -1;

    glutPostRedisplay();

}

void processMouseActiveMotion(GLint xMouse, GLint yMouse) {
    GLint i;
    if (dragged >= 0) {
        inputs[dragged].x = xMouse;
        inputs[dragged].y = winHeight - yMouse;

        if (dragged == 2 || dragged == 3) {
            inputs[2].x = inputs[3].x = xMouse;
            inputs[2].y = inputs[3].y = winHeight - yMouse;;

        }

        glutPostRedisplay();

    }

}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Computer Graphics");
    init();
    glutDisplayFunc(display);
    glutMouseFunc(processMouse);
    glutMotionFunc(processMouseActiveMotion);
    glutMainLoop();
    return 0;
}

