#pragma once
#define MAX_OBJECTS 10000
#define DEFAULT_POINT_SIZE 10
#define DEFAULT_LINE_WIDTH 5
#define DEFAULT_BEZIER_CONTROL_POINTS_NUM 4
#define MAX_BEZIER_CONTROL_POINTS_NUM 13
#define DEFAULT_PICK_BUFFER_SIZE 32
#define DEFAULT_PICK_WINDOW_SIZE 10
#define MAX_BEZIER_LINE_NUM 5
#define PI 3.14159265358979323846 // pi
#include<math.h>
#include<GL/glut.h>
#include<random>
#include<set>
const GLdouble derta = 10;
enum GLState {
	MoveObject,
	MakePoint,
	AutoMove
};

struct GLColor;
struct GLTransform;

class GLObject;
class GLWorld;

// GLObject 派生类
class GLPoint; // 点
class GLBezier; // 贝塞尔曲线

struct GLColor
{
	GLfloat r = 0, g = 0, b = 0;
};

struct GLTransform
{
	// 世界坐标，轴旋转，轴缩放
	GLdouble x = 0, y = 0, z = 0;
	GLdouble rx = 0, ry = 0, rz = 0;
	GLdouble sx = 1, sy = 1, sz = 1;
};

class GLObject
{
public:
	GLint id = 0;
	GLColor color;
	GLTransform transform;
	bool movable = false;
	bool visible = true;
	
	GLWorld * parentWorld;
	
	virtual void Draw(GLenum RenderMode = GL_RENDER) {}
	
	virtual void MakePoint(GLdouble x, GLdouble y, GLdouble z) {}
	
	virtual void KeyboardMove(GLdouble right,GLdouble up){}

	virtual void AutoMove(){}

	void SetPosition(GLdouble x, GLdouble y, GLdouble z) {
		transform.x = x;
		transform.y = y;
		transform.z = z;
	}
};

class GLWorld
{
public:
	GLObject * objects[MAX_OBJECTS];
	GLint count = 0;
	
	GLWorld() = default;
	~GLWorld() {
		for (int i = count - 1; i >= 0; --i)
		{
			delete objects[i];
		}
	
	}
	
	template < typename T>
	T * NewObject() {
		if (count < MAX_OBJECTS)
		{
			T * ret = new T;
			objects[count] = (GLObject*)ret;
			objects[count]->parentWorld = this;
			objects[count]->id = count;
			++count;
			return ret;
		}
		else return nullptr;
	
	}
	
	void DeleteObject(GLint id) {
		delete objects[id];
		for (int i = id; i < count - 1; i++)
		{
			objects[i] = objects[i + 1];
		}
		count -= 1;
	}
	void Empty() {
		for (int i = 0; i < count; i++)
		{
			delete objects[i];
		}
		count = 0;
	}
	
	void DrawObjects(GLenum RenderMode = GL_RENDER) {
		for (int i = 0; i < count; ++i)
		{
			objects[i]->Draw(RenderMode);
		}
	}

	std::set<GLint> id;
	GLint pickedObjectId = -1;
	void PickObject(GLint xMouse, GLint yMouse) {
		pickRects(GLUT_LEFT_BUTTON, GLUT_DOWN, xMouse, yMouse);

	}
private:
	void pickRects(GLint button, GLint action, GLint xMouse, GLint yMouse);
	void processPicks(GLint nPicks, GLuint pickBuffer[]);
};

// 使用外部定义的世界坐标系裁剪空间，供设置选取时的裁剪窗口
extern GLfloat xwcMin, xwcMax, ywcMin, ywcMax;

void GLWorld::pickRects(GLint button, GLint action, GLint xMouse,
	GLint yMouse)
{
	GLuint pickBuffer[DEFAULT_PICK_BUFFER_SIZE];
	GLint nPicks, vpArray[4];
		
	if(button != GLUT_LEFT_BUTTON || action != GLUT_DOWN)
		return;
	
	glSelectBuffer(DEFAULT_PICK_BUFFER_SIZE, pickBuffer); // Designate pick buffer.
		
	glRenderMode(GL_SELECT); // Activate picking operations.
		
	glInitNames(); // Initialize the object-ID stack.
		
		
	// 只使用一个栈顶元素，后续为图形命名使用 glLoadName(id) 替换栈顶
	
	glPushName(MAX_OBJECTS);
		
	/* Save current viewing matrix. */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
		/* Obtain the parameters for the current viewport. Set up
		25.
		* a 5 x 5 pick window, and invert the input yMouse value
		26.
		* using the height of the viewport, which is the fourth
		27.
		* element of vpArray.
		28.
		*/
	glGetIntegerv(GL_VIEWPORT, vpArray);
	gluPickMatrix(GLdouble(xMouse), GLdouble(vpArray[3] - yMouse),DEFAULT_PICK_WINDOW_SIZE, DEFAULT_PICK_WINDOW_SIZE,vpArray);
	
	gluOrtho2D(xwcMin, xwcMax, ywcMin, ywcMax);
	//rects (GL_SELECT); // Process the rectangles in selection mode.
	this->DrawObjects(GL_SELECT);
	/* Restore original viewing matrix. */
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();
		/* Determine the number of picked objects and return to the
		* normal rendering mode.
		*/
	nPicks = glRenderMode(GL_RENDER);
	processPicks(nPicks, pickBuffer); // Process picked objects.
	
		
	glutPostRedisplay();
	}

void GLWorld::processPicks(GLint nPicks, GLuint pickBuffer[])
{
	
	// 储存获取到的最后一个 ID，即认为后绘制的图像在先绘制的图形之上

	if (nPicks == 0) pickedObjectId = -1;
	else
	{
		
		// 因为只使用一个栈顶元素，所以可以由公式计算得出最后一个图形ID
		int chunkId = 4 * (nPicks - 1);
		pickedObjectId = pickBuffer[chunkId + 3];
		id.insert(pickedObjectId);
	}
	return;
}

class GLPoint : public GLObject
{
public:
	GLPoint() {
		color.r = 1;
	}

	virtual void KeyboardMove(GLdouble right, GLdouble up) override
	{
		if (transform.x + right > xwcMax || transform.x + right < xwcMin) {
			if (transform.x + right > xwcMax)
				transform.x = right - xwcMax;
			else
				transform.x = xwcMax + right;
			return;
		}
		transform.x += right;
		if (transform.y + up > ywcMax || transform.y + up < ywcMin) {
			if (transform.y + up > ywcMax)
				transform.y = up - ywcMax;
			else
				transform.y = up + ywcMax;
			return;
		}
		transform.y += up;
	}
	virtual void AutoMove() override {
		// 创建随机数生成器
		std::random_device rd;                     // 随机设备，提供种子
		std::mt19937 gen(rd());                    // Mersenne Twister 生成器
		std::uniform_real_distribution<double> dis(-10.0, 10.0); // [0, 10) 均匀分布

		// 生成随机数
		GLdouble a[2];
		for (int i = 0; i < 2; i++)
		{
			a[i] = GLdouble(dis(gen));
		}
		KeyboardMove(a[0], a[1]);

	}
	
	virtual void Draw(GLenum RenderMode = GL_RENDER) {
		// 需要在绘制前调用 SetPostion，否则绘制在原点
		if (!visible) return;
		if (RenderMode == GL_SELECT) glLoadName((GLuint)id);
		glColor3f(color.r, color.g, color.b);
		glPointSize(DEFAULT_POINT_SIZE);
		glBegin(GL_POINTS);
		glVertex3d(transform.x, transform.y, transform.z);
		glEnd();
	}

};

class GLBezier : public GLObject
{
public:
	GLPoint * ctrlPoints[MAX_BEZIER_CONTROL_POINTS_NUM];
	int count = 0;
	GLint nBezCurvePoints = 1000;

	// Usage:
	//GLWorld::NewObject<GLBezier>()->MakePoint(x,y,z);
	virtual void MakePoint(GLdouble x, GLdouble y, GLdouble z) {
		if (count >= MAX_BEZIER_CONTROL_POINTS_NUM) return;
		ctrlPoints[count] = parentWorld->NewObject<GLPoint>();
		ctrlPoints[count]->SetPosition(x, y, z); // 初始位置
		ctrlPoints[count]->movable = true; // 可以移动
		count++;
	}
	
	virtual void Draw(GLenum RenderMode = GL_RENDER) {
		// 需要在绘制前初始化控制点
		if (count == 0) return;
		for (int i = 0; i < count; ++i)
		{
			if (!ctrlPoints[i]) return;
		}
		if (RenderMode == GL_SELECT) glLoadName((GLuint)id);
		glColor3f(color.r, color.g, color.b);
		glPointSize(DEFAULT_LINE_WIDTH);
		bezier(ctrlPoints, nBezCurvePoints);
	}
	
	// ...
private:
	class wcPt3D {
	public:
		GLfloat x, y, z;
	};
	void plotPoint(wcPt3D bezCurvePt) {
			glBegin(GL_POINTS);
			glVertex2f(bezCurvePt.x, bezCurvePt.y);
			glEnd();
	}
	void binomialCoeffs(GLint n, GLint * C) {
			GLint k, j;
			for (k = 0; k <= n; k++)
			{
					C[k] = 1;
					for (j = n; j >= k + 1; j--)
						C[k] *= j;
					for (j = n - k; j >= 2; j--)
						C[k] /= j;
			}
	}
	void computeBezPt(GLfloat u, wcPt3D * bezPt, GLint nCtrlPts, wcPt3D * ctrlPts, GLint * C){
		GLint k, n = nCtrlPts - 1;
		GLfloat bezBlendFcn;
		bezPt->x = bezPt->y = 0.0;
		for (k = 0; k < nCtrlPts; k++) {
				bezBlendFcn = C[k] * pow(1 - u, n - k) * pow(u, k);
				bezPt->x += ctrlPts[k].x * bezBlendFcn;
				bezPt->y += ctrlPts[k].y * bezBlendFcn;
				//bezPt->z += ctrlPts[k].z * bezBlendFcn;
		}
}
	void bezier(wcPt3D* ctrlPts, GLint nCtrlPts, GLint nBezCurvePts) {
			wcPt3D bezCurvePt;
			GLfloat u;
			GLint * C, k;
			C = new GLint[nCtrlPts];
			binomialCoeffs(nCtrlPts - 1, C);
			for (k = 0; k <= nBezCurvePts; k++)
			{
					u = GLfloat(k) / GLfloat(nBezCurvePts);
					computeBezPt(u, &bezCurvePt, nCtrlPts, ctrlPts, C);
					plotPoint(bezCurvePt);
					//printf("%d/t", k);
			}
			delete[] C;
	}
	void bezier(GLPoint * ctrlPts[], GLint nBezCurvePts) {
		// 函数重载转发
		wcPt3D innerCtrlPts[MAX_BEZIER_CONTROL_POINTS_NUM];
		for (int i = 0; i < count; ++i)
		{
			innerCtrlPts[i].x = (GLfloat)ctrlPts[i]->transform.x;
			innerCtrlPts[i].y = (GLfloat)ctrlPts[i]->transform.y;
			innerCtrlPts[i].z = (GLfloat)ctrlPts[i]->transform.z;
		}

		bezier(innerCtrlPts, count, nBezCurvePts);
		
		
		// 绘制控制点连线
		if (ctrlPts[0] && ctrlPts[0]->visible == false) return;
		glColor3f(0.0, 1.0, 0.0);
		glLineWidth(4.0);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < count; ++i)
		{
			glVertex3d(ctrlPts[i]->transform.x, ctrlPts[i]->transform.y, ctrlPts[i]->transform.z);
		}
		glEnd();
	}
};