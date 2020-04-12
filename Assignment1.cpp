#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>

GLsizei winWidth = 800, winHeight = 600;

vec2 points[11] = {

	{225 , 285 },//P0
	{192 , 279 },//P1
	{80  , 484 },//P2
	{201 , 543 },//P3
	{413 , 524 },//P4
	{478 , 424 },//P5

	{478 , 424 },//P6
	{506 , 154 },//P7
	{269 , 132 },//P8

	{269 , 132 },//P9
	{225 , 285 },//P10

};




GLint dragged = -1;
vec2 tangent;
vec2 v1;
vec2 v2;
vec2 tangent_start;
vec2 tangent_end;

void init()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, winWidth, 0.0, winHeight);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5);
	glLineWidth(2.0);
}

void drawBezier() {
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(0, 0, 0);
	glBegin(GL_POINTS);//draw the points
	for (int i = 0; i < 6; i++) {
		glVertex2f(points[i].x, points[i].y);
	}
	glEnd();
	glBegin(GL_LINE_STRIP); //draw the control polygon
	for (int i = 0; i < 6; i++) {
		glVertex2f(points[i].x, points[i].y);
	}
	glEnd();

	glColor3f(1, 0, 0);
	glBegin(GL_LINE_STRIP);
	int n = 5; // we have 6 points, thus n = 5 (number of points - 1)
	float binomials[6] = { 1,5,10,10,5,1 }; // we use the (5 i) binomial coefficients: 1, 5, 10, 10, 5,1
	for (float t = 0; t <= 1; t += 0.01) { // t is in [0,1]

		float curveX = 0; // we need to initialize the curve point
		float curveY = 0;

		// we create the sum with a loop
		// each point is multiplied with the Bernstein polynomial, and we add these together
		for (int i = 0; i <= n; i++) { // i = {0,1,2,3,4}

			curveX += points[i].x * binomials[i] * pow(t, i) * pow(1 - t, n - i);
			curveY += points[i].y * binomials[i] * pow(t, i) * pow(1 - t, n - i);
		}
		// after the loop, all values are added together, and curveX and curveY contain the final values
		glVertex2f(curveX, curveY);
	}
	glVertex2f(points[5].x, points[5].y);
	glEnd();

	// calculate and draw tangent vector at the starting and ending point
	glColor3f(0, 0, 0);
	tangent_start = n * (points[1] - points[0]);

	/*glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xFF00);
	glBegin(GL_LINES);
	glVertex2f(points[0].x, points[0].y);
	glVertex2f(points[0].x + tangent_start.x, points[0].y + tangent_start.y);
	glEnd();
	*/
	tangent_end = n * (points[5] - points[4]);




}
void drawHermite3() {

	v1 = tangent_end;

	glColor3f(0, 0, 0);
	glBegin(GL_POINTS);
	for (int i = 6; i < 9; i++)
		glVertex2f(points[i].x, points[i].y);
	glEnd();


	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xFF00);
	glBegin(GL_LINES);
	glVertex2f(points[6].x, points[6].y);
	glVertex2f(points[6].x + tangent_end.x, points[6].y + tangent_end.y);
	glEnd();





	mat24 G = { points[6],  points[7],   points[8],  v1 };

	float t[3] = { 0.0f, 1.0f, 2.0f };

	vec4 columns[4];

	for (int i = 0; i < 3; i++) {
		columns[i] = vec4(t[i] * t[i] * t[i], t[i] * t[i], t[i], 1);
	}
	columns[3] = vec4(3 * t[0] * t[0], 2 * t[0], 1, 0);

	mat4 M = { columns[0],  columns[1],  columns[2],  columns[3], true };
	mat4 M_ = inverse(M);
	mat24 C = G * M_;

	glColor3f(0, 0, 1);
	glDisable(GL_LINE_STIPPLE);
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
	v2 = tangent_start;

	glColor3f(0, 0, 0);
	glBegin(GL_POINTS);
	for (int i = 9; i <= 10; i++)
		glVertex2f(points[i].x, points[i].y);
	glEnd();


	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xFF00);
	glBegin(GL_LINES); // drawing the tangent vectors from the Hermite3
	glVertex2f(points[9].x, points[9].y);
	glVertex2f(points[9].x + tangent.x, points[9].y + tangent.y);

	glVertex2f(points[10].x, points[10].y);
	glVertex2f(points[10].x + v2.x, points[10].y + v2.y);
	glEnd();


	mat24 G = { points[9],  points[10],  tangent,  v2 }; //2x4 matrix. Filled up with the geometric data (2 points, 2 tangent vectors) in column-major order

	float tvalues[2] = { 0.0f, 1.0f }; // the t paramter values when the curve should go through p1 and p2

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
	glColor3f(0.0, 1.0, 0.0);
	glDisable(GL_LINE_STIPPLE);
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

void display()
{
	drawBezier();
	drawHermite3();
	drawHermite2WithMat();
	glutSwapBuffers();
}


GLint getActivePoint1(vec2 p[], GLint size, GLint sens, GLint x, GLint y)
{
	GLint i, s = sens * sens;
	vec2 mouse = { (float)x, (float)y };

	for (i = 0; i < size; i++)
		if (dist2(p[i], mouse) < s)
			return i;
	return -1;
}

void processMouse(GLint button, GLint action, GLint xMouse, GLint yMouse)
{
	GLint i;
	if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN)
		if ((i = getActivePoint1(points, 11, 8, xMouse, winHeight - yMouse)) != -1)
			dragged = i;
	if (button == GLUT_LEFT_BUTTON && action == GLUT_UP)
		dragged = -1;

}

void processMouseActiveMotion(GLint xMouse, GLint yMouse)
{
	GLint i;
	if (dragged >= 0) {
		points[dragged].x = xMouse;
		points[dragged].y = winHeight - yMouse;

		if (dragged == 5 || dragged == 6) {
			points[5].x = points[6].x = xMouse;
			points[5].y = points[6].y = winHeight - yMouse;;

		}
		if (dragged == 9 || dragged == 8) {
			points[8].x = points[9].x = xMouse;
			points[8].y = points[9].y = winHeight - yMouse;;

		}
		if (dragged == 0 || dragged == 10) {
			points[0].x = points[10].x = xMouse;
			points[0].y = points[10].y = winHeight - yMouse;;

		}

		glutPostRedisplay();
	}

	/*printf("-------------\n");
	for  (vec2 point : points)
	{
		printf(" %f , %f \n",point.x, point.y);


	}*/


}

int main(int argc, char** argv)
{
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


