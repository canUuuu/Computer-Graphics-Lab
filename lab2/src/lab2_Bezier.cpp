#include "GLObject.h"
#include<iostream>
#include<vector>

GLWorld MyWorld;

GLState MyState = MoveObject;
/* Set initial size of the display window. */
GLsizei winWidth = 900, winHeight = 600;

/* Set size of world-coordinate clipping window. */
GLfloat xwcMin = -450.0, xwcMax = 450.0;
GLfloat ywcMin = -300.0, ywcMax = 300.0;

struct menuEntryStruct {
	const char* label;
	char key;
};

static menuEntryStruct mainMenu[] = {
"Reset", '0',
"Create Bezier/Finish Bezier", '1',
"Quit", 27, //ESC 键（ASCII: 27）
"Auto Moving",'2',//Automoving
//"MovePoint",'3'
};
int mainMenuEntries = sizeof(mainMenu) / sizeof(menuEntryStruct);

void Auto(int value) {
	if (MyWorld.id.empty())
		return;
	for (auto ite = MyWorld.id.begin(); ite != MyWorld.id.end(); ite++) {
		MyWorld.objects[*ite]->AutoMove();
	}
	
	glutPostRedisplay();
	glutTimerFunc(600, Auto, 0); // ~1000ms = 1s
}


bool globalControlPointsVisible = true;

void userEventAction(int key) {
	switch (key) {
	case '0':
		MyWorld.Empty();
		MyWorld.pickedObjectId = -1;
		MyState = MoveObject;
		break;
	case '1': // bezier
		if (MyState == MoveObject)
		{
			GLBezier * bezPtr = MyWorld.NewObject<GLBezier>();
			MyState = MakePoint;
			MyWorld.pickedObjectId = bezPtr->id;
		
		}
		else if (MyState == MakePoint)
		{
			MyState = MoveObject;
			MyWorld.pickedObjectId = -1;
		}
		break;

	case '2'://Auto Move
		if (MyState == MakePoint)
		{
			MyState = MoveObject;
			MyWorld.pickedObjectId = -1;
			break;
		}
		if (MyState == AutoMove)
		{
			MyState = MoveObject;
			break;
		}
		if (MyWorld.pickedObjectId == -1) {
			std::printf("Error:U've not choosen a point!\n");
			break;
		}
		std::printf("Auto Moving No.%d\n", MyWorld.pickedObjectId);
		MyState = AutoMove;
		glutTimerFunc(0, Auto, 0); // 启动定时器
		break;
	case 32://space
		if (MyState == AutoMove)
			MyState = MoveObject;
		break;
	case 'w':
		if (MyWorld.pickedObjectId != -1) {
			MyWorld.objects[MyWorld.pickedObjectId]->KeyboardMove(0, derta);
		}
		break;
	case 's':
		if (MyWorld.pickedObjectId != -1) {
			MyWorld.objects[MyWorld.pickedObjectId]->KeyboardMove(0, -derta);
		}
		break;
	case 'a':
		if (MyWorld.pickedObjectId != -1) {
			MyWorld.objects[MyWorld.pickedObjectId]->KeyboardMove(-derta, 0);
		}
		break;
	case 'd':
		if (MyWorld.pickedObjectId != -1) {
			MyWorld.objects[MyWorld.pickedObjectId]->KeyboardMove(derta, 0);
		}
		break;
	case 27: // ESC 键（ASCII: 27）退出
		MyWorld.Empty();
		exit(0);
		break;
	default:
		break;
	}
	glutPostRedisplay(); // 重绘
}

void keyboard(unsigned char key, int x, int y) {

	userEventAction(key);
}

void selectMain(int choice) // 序号->key 映射
{
	userEventAction(mainMenu[choice].key); // 调用通用动作解释函数
}

void displayFcn(void)
{
	glClear(GL_COLOR_BUFFER_BIT); // Clear display window.
	MyWorld.DrawObjects(GL_RENDER);
	glFlush();
}

void myReshape(GLint Width, GLint Height) {
	glViewport(0, 0, Width, Height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(xwcMin, xwcMax, ywcMin, ywcMax);

	glClear(GL_COLOR_BUFFER_BIT);
}

void unProject(GLdouble winx, GLdouble winy, GLdouble winz, GLdouble * objx,
	GLdouble * objy, GLdouble * objz)
	{
		// 从窗口到世界坐标系
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport);
		gluUnProject(winx, winy, winz, modelview, projection, viewport, objx, objy, objz);
}

void mouseButton(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		break;
	default:
		return;
	}
	switch (state) {
	case GLUT_DOWN:
		if (MyState == MoveObject)
		{
			MyWorld.PickObject(x, y);
			std::printf("Pick No.%d\n", MyWorld.pickedObjectId);
			std::printf("U've Choosen:");
			for (auto ite = MyWorld.id.begin(); ite != MyWorld.id.end(); ite++) {
				std::printf("No.%d ", *ite);
			}
			std::printf("\n");
		}
		else if (MyState == MakePoint)
		{
			if(GLObject* ptr = MyWorld.objects[MyWorld.pickedObjectId])
			{
				// 从窗口到世界坐标系
				GLdouble winx = (GLdouble)x;
				GLdouble winy = (GLdouble)winHeight - (GLdouble)y;
				GLdouble winz = 0;
				GLdouble objx = 0;
				GLdouble objy = 0;
				GLdouble objz = 0;
				unProject(winx, winy, winz, &objx, &objy, &objz);
				ptr->MakePoint(objx, objy, objz);
			}
		}
		break;
	case GLUT_UP:
		if (MyState == MoveObject)
		{
			//MyWorld.pickedObjectId = -1;
		}
		glutPostRedisplay();
		break;
	}
}

void mouseMotion(int x, int y)
{
	if (MyWorld.pickedObjectId == -1) return;
	if (!MyWorld.objects[MyWorld.pickedObjectId]->movable) return;
	
	GLObject * ptr = MyWorld.objects[MyWorld.pickedObjectId];
	
	// 从窗口到世界坐标系
	GLdouble winx = (GLdouble)x;
	GLdouble winy = (GLdouble)winHeight - (GLdouble)y;
	GLdouble winz = 0;
	GLdouble objx = 0;
	GLdouble objy = 0;
	GLdouble objz = 0;
	unProject(winx, winy, winz, &objx, &objy, &objz);
	
	ptr->SetPosition(objx, objy, objz);
	glutPostRedisplay();
}

void init(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);//白色背景

	// 绑定菜单
	glutCreateMenu(selectMain); // 使用 selectMain 作为菜单调用函数
	for (int i = 0; i < mainMenuEntries; i++)
	{
		glutAddMenuEntry(mainMenu[i].label, i);
	}
	glutAttachMenu(GLUT_MIDDLE_BUTTON); // 菜单绑定在鼠标中键
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Example");

	init();

	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	glutKeyboardFunc(keyboard);

	//glutReshapeFunc(myReshape);
	glutDisplayFunc(displayFcn);
	glutReshapeFunc(myReshape);

	glutMainLoop();

	return 0;
}