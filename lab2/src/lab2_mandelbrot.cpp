#include <GL/glut.h>

GLsizei winWidth = 800, winHeight = 800;

GLfloat xComplexMin = -2.00, xComplexMax = 0.50;
GLfloat yComplexMin = -1.25, yComplexMax = 1.25;

GLfloat complexWidth = xComplexMax - xComplexMin;
GLfloat complexHeight = yComplexMax - yComplexMin;

class complexNum {
public:
    GLfloat x, y;
};

struct color {
    GLfloat r, g, b;
};

void init(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

void plotPoint(complexNum z)
{
    glBegin(GL_POINTS);
    glVertex2f(z.x, z.y);
    glEnd();
}

complexNum complexSquare(complexNum z)
{
    complexNum zSquare;

    zSquare.x = z.x * z.x - z.y * z.y;
    zSquare.y = 2 * z.x * z.y;
    return zSquare;
}

GLint mandelSqTransf(complexNum z0, GLint maxIter)
{
    complexNum z = z0;
    GLint count = 0;

    while ((z.x * z.x + z.y * z.y <= 4.0) && (count < maxIter)) {
        z = complexSquare(z);
        z.x += z0.x;
        z.y += z0.y;
        count++;
    }
    return count;
}

void mandelbrot(GLint nx, GLint ny, GLint maxIter)
{
    complexNum z, zIncr;
    color ptColor;

    GLint iterCount;

    zIncr.x = complexWidth / GLfloat(nx);
    zIncr.y = complexHeight / GLfloat(ny);

    for (z.x = xComplexMin; z.x < xComplexMax; z.x += zIncr.x)
        for (z.y = yComplexMin; z.y < yComplexMax; z.y += zIncr.y) {
            iterCount = mandelSqTransf(z, maxIter);
            if (iterCount >= maxIter) // black
                ptColor.r = ptColor.g = ptColor.b = 0.0;
            else if (iterCount > (maxIter / 8)) {
                ptColor.r = 1.0;  // orange
                ptColor.g = 0.5;
                ptColor.b = 0.0;
            }
            else if (iterCount > (maxIter / 10)) {
                ptColor.r = 1.0;  // red
                ptColor.g = ptColor.b = 0.0;
            }
            else if (iterCount > (maxIter / 20)) {
                ptColor.b = 0.5;  // blue
                ptColor.r = ptColor.g = 0.0;
            }
            else if (iterCount > (maxIter / 40)) {
                ptColor.r = ptColor.g = 1.0;  // yellow
                ptColor.b = 0.0;
            }
            else if (iterCount > (maxIter / 100)) {
                ptColor.r = ptColor.b = 0.0;  // green
                ptColor.g = 0.3;
            }
            else {
                ptColor.r = 0.0;  // cyan
                ptColor.g = ptColor.b = 1.0;
            }
            glColor3f(ptColor.r, ptColor.g, ptColor.b);
            plotPoint(z);
        }
}

// 全局变量
GLint nx = 1000, ny = 1000;
GLint maxIter[4] = { 50, 100, 200, 400 };  // 每个区域的最大迭代次数
GLint maxIterLimit = 1000;  // 最大迭代次数限制

// 定时器回调函数，自动更新迭代次数
void animate(int value)
{
    for (int i = 0; i < 4; ++i) {
        if (maxIter[i] < maxIterLimit) {
            maxIter[i] += 10;  // 每次增加10次迭代
        }
    }
    glutPostRedisplay();  // 请求重绘
    glutTimerFunc(100, animate, 0);  // 每100毫秒调用一次
}

void displayFcn(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // 定义四个区域的坐标范围
    GLfloat xMin[4] = { xComplexMin, xComplexMin + complexWidth / 2, xComplexMin, xComplexMin + complexWidth / 2 };
    GLfloat xMax[4] = { xComplexMin + complexWidth / 2, xComplexMax, xComplexMin + complexWidth / 2, xComplexMax };
    GLfloat yMin[4] = { yComplexMax - complexHeight / 2, yComplexMax - complexHeight / 2, yComplexMin, yComplexMin + complexHeight / 2 };
    GLfloat yMax[4] = { yComplexMax, yComplexMax, yComplexMin + complexHeight / 2, yComplexMin + complexHeight / 2 };

    // 将窗口分为四个部分，每个部分显示不同区域的曼德布罗特图案
    for (int i = 0; i < 4; ++i) {
        // 设置视口
        glViewport((i % 2) * (winWidth / 2), (i / 2) * (winHeight / 2), winWidth / 2, winHeight / 2);  // 每块大小为 winWidth/2 x winHeight/2

        // 绘制曼德布罗特集合，传递不同的坐标范围和最大迭代次数
        mandelbrot(nx, ny, maxIter[i]);
    }

    glFlush();
}

void winReshapeFcn(GLint newWidth, GLint newHeight)
{
    glViewport(0, 0, newWidth, newHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(xComplexMin, xComplexMax, yComplexMin, yComplexMax);
    glClear(GL_COLOR_BUFFER_BIT);
}

void main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Mandelbrot Set - Multiple Views");

    init();
    glutDisplayFunc(displayFcn);
    glutReshapeFunc(winReshapeFcn);

    // 启动定时器进行自动迭代更新
    glutTimerFunc(1000, animate, 0);  // 100ms 后启动动画

    glutMainLoop();
}
