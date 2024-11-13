#include <GL/glut.h>
#include <iostream>
using namespace std;
#include <cmath>

const double M_PI = 3.14159265358979323846; // pi
enum TransMode
{
    Translate,
    Rotate,
    Scale
};

GLfloat objectXform[4][4] = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0} };

TransMode transModeSelected = Translate;

float angle = 0.0, axis[3];

int lastX, lastY;
void startMotion(int x, int y)
{
    lastX = x;
    lastY = y;
}
void stopMotion(int x, int y)
{
    angle = 0.0f;
    memset(axis, 0, sizeof(axis));
    printf("objectXform:\n");
    for (int i = 0; i < 4; i++)
    {
        printf("\t");
        for (int j = 0; j < 4; j++)
        {
            printf("%.2f\t", objectXform[i][j]);
        }
        printf("\n");
    }
}

bool rotateCamera = false; // 相机漫游
void mouseButton(int button, int state, int x, int y)
{
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        rotateCamera = false;
        break;
    case GLUT_RIGHT_BUTTON: // 鼠标右键控制相机漫游
        rotateCamera = true;
        break;
    default:
        return;
    }
    switch (state)
    {
    case GLUT_DOWN:
        startMotion(x, y);
        break;
    case GLUT_UP:
        stopMotion(x, y);
        break;
    }
}

GLfloat xview = 0.0, yview = 0.0, zview = 0.0; // Viewing-coordinate origin.
GLfloat xref = 0.0, yref = 0.0, zref = -1.0;   // Look-at point.
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0;          // View-up vector.
GLfloat viewxform_x = 0.0, viewxform_y = 0.0, viewxform_z = -5.0;

void quatRotate(
    GLfloat theta, GLfloat ux, GLfloat uy, GLfloat uz,
    GLfloat& px, GLfloat& py, GLfloat& pz)
{
    GLfloat inpx = px;
    GLfloat inpy = py;
    GLfloat inpz = pz;
    px =
        (ux * ux * (1 - cos(theta)) + cos(theta)) * inpx +
        (ux * uy * (1 - cos(theta)) - uz * sin(theta)) * inpy +
        (ux * uz * (1 - cos(theta)) + uy * sin(theta)) * inpz;
    py =
        (uy * ux * (1 - cos(theta)) + uz * sin(theta)) * inpx +
        (uy * uy * (1 - cos(theta)) + cos(theta)) * inpy +
        (uy * uz * (1 - cos(theta)) - ux * sin(theta)) * inpz;
    pz =
        (uz * ux * (1 - cos(theta)) - uy * sin(theta)) * inpx +
        (uz * uy * (1 - cos(theta)) + ux * sin(theta)) * inpy +
        (uz * uz * (1 - cos(theta)) + cos(theta)) * inpz;
}

int winWidth, winHeight;
float mynear = 1.5; // near clipping plane in eye coords
float myfar = 8.0;  // far clipping plane in eye coords
bool flag = true;   // true表示透视投影，false表示正投影

// 自定义斜切矩阵
void setObliqueProjection(float angle, float factor) {
    float obliqueMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        factor * cos(angle), factor * sin(angle), 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    glMultMatrixf(obliqueMatrix);  // 将斜切矩阵应用到当前投影矩阵
}

bool flag_pro = false;//trre表示斜平行投影开启，false表示正平行投影
void myReshape(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (flag)
    {
        // 设置正交投影矩阵，保持图形大小不变
        if (w <= h)
        {
            // 如果窗口宽度小于等于高度，y轴范围更大
            glFrustum(-2.5f, 2.5f, -2.5f * (GLfloat)h / (GLfloat)w, 2.5f * (GLfloat)h / (GLfloat)w, mynear, myfar);
        }
        else
        {
            // 如果窗口宽度大于高度，x轴范围更大
            glFrustum(-2.5f * (GLfloat)w / (GLfloat)h, 2.5f * (GLfloat)w / (GLfloat)h, -2.5f * w / h, 2.5f * w / h, mynear, myfar);
        }
    }
    else
    {
        // 设置正交投影矩阵，保持图形大小不变
        if (w <= h)
        {
            // 如果窗口宽度小于等于高度，y轴范围更大
            glOrtho(-2.5f, 2.5f, -2.5f * (GLfloat)h / (GLfloat)w, 2.5f * (GLfloat)h / (GLfloat)w, mynear, myfar);
        }
        else
        {
            // 如果窗口宽度大于高度，x轴范围更大
            glOrtho(-2.5f * (GLfloat)w / (GLfloat)h, 2.5f * (GLfloat)w / (GLfloat)h, -2.5f * w / h, 2.5f * w / h, mynear, myfar);
        }

    }
    // 这里设置斜切矩阵，调整斜切的角度和因子
    if (flag_pro) {
        float angle = -0.5f;  // 斜切角度
        float factor = 0.5f;  // 斜切因子
        setObliqueProjection(angle, factor);
    }
    

    glMatrixMode(GL_MODELVIEW); // 切换回模型视图矩阵模式
}

GLfloat VRightx = (yref - yview) * Vz - (zref - zview) * Vy;
GLfloat VRighty = (zref - zview) * Vx - (xref - xview) * Vz;
GLfloat VRightz = (xref - xview) * Vy - (yref - yview) * Vx;
void mouseMotion(int x, int y)
{
    float dx = x - lastX;
    float dy = y - lastY;
    if (!dx && !dy)
        return;
    if (rotateCamera)
    {
        // 以参考点为中心，绕局部 X 轴旋转俯仰角，绕平行世界 Y 轴旋转环视；
        // 左乘局部向右向量(cross(v,vup))旋转，左乘世界 Y 轴向量(0,1,0)旋转
        VRightx = (yref - yview) * Vz - (zref - zview) * Vy;
        VRighty = (zref - zview) * Vx - (xref - xview) * Vz;
        VRightz = (xref - xview) * Vy - (yref - yview) * Vx;
        // 四元数旋转，旋转角由 dx 和 dy 确定
        GLfloat thetaX = -dy * (M_PI / (GLfloat)winHeight);
        GLfloat thetaY = -dx * (M_PI / (GLfloat)winWidth);
        GLfloat px = xref - xview;
        GLfloat py = yref - yview;
        GLfloat pz = zref - zview;
        quatRotate(thetaX, VRightx, VRighty, VRightz, px, py, pz);
        quatRotate(thetaY, 0.0, 1.0, 0.0, px, py, pz);
        xref = xview + px;
        yref = yview + py;
        zref = zview + pz;
        // 计算 vup 轴
        quatRotate(thetaX, VRightx, VRighty, VRightz, Vx, Vy, Vz);
        quatRotate(thetaY, 0.0, 1.0, 0.0, Vx, Vy, Vz);
    }
    else
    {
        if (transModeSelected == Translate)
        {
            axis[0] = dx * (10.0f / (GLfloat)winWidth);
            axis[1] = -dy * (10.0f / (GLfloat)winHeight);
            axis[2] = 0;
        }
        else if (transModeSelected == Rotate)
        {
            angle = 3.0;
            axis[0] = dy * (360.0f / (GLfloat)winHeight);
            axis[1] = dx * (360.0f / (GLfloat)winWidth);
            axis[2] = 0;
        }
        else if (transModeSelected == Scale)
        {
            axis[0] = dx * (4.0f / (GLfloat)winWidth);
            axis[1] = -dy * (4.0f / (GLfloat)winHeight);
            axis[2] = 0; // 在调用时需要 +1
        }
    }
    lastX = x;
    lastY = y;
    glutPostRedisplay();
}

void userEventAction(int key)
{
    switch (key)
    {
    case 'w':
        xview += 0.1 * (xref - xview);
        xref += 0.1 * (xref - xview);
        yview += 0.1 * (yref - yview);
        yref += 0.1 * (yref - yview);
        zview += 0.1 * (zref - zview);
        zref += 0.1 * (zref - zview);
        break;
    case 'a':
        VRightx = (yref - yview) * Vz - (zref - zview) * Vy;
        VRighty = (zref - zview) * Vx - (xref - xview) * Vz;
        VRightz = (xref - xview) * Vy - (yref - yview) * Vx;
        xview -= 0.1 * VRightx;
        xref -= 0.1 * VRightx;
        yview -= 0.1 * VRighty;
        yref -= 0.1 * VRighty;
        zview -= 0.1 * VRightz;
        zref -= 0.1 * VRightz;
        break;
    case 's':
        xview -= 0.1 * (xref - xview);
        xref -= 0.1 * (xref - xview);
        yview -= 0.1 * (yref - yview);
        yref -= 0.1 * (yref - yview);
        zview -= 0.1 * (zref - zview);
        zref -= 0.1 * (zref - zview);
        break;
    case 'd':
        VRightx = (yref - yview) * Vz - (zref - zview) * Vy;
        VRighty = (zref - zview) * Vx - (xref - xview) * Vz;
        VRightz = (xref - xview) * Vy - (yref - yview) * Vx;
        xview += 0.1 * VRightx;
        xref += 0.1 * VRightx;
        yview += 0.1 * VRighty;
        yref += 0.1 * VRighty;
        zview += 0.1 * VRightz;
        zref += 0.1 * VRightz;
        break;
    case 'q':
        yview -= 0.1 * 1.0;
        yref -= 0.1 * 1.0;
        break;
    case 'e':
        yview += 0.1 * 1.0;
        yref += 0.1 * 1.0;
        break;
    case '2': // 正视图
        xview = 0.0;
        yview = 0.0;
        zview = 0.0;
        xref = 0.0;
        yref = 0.0;
        zref = -5.0;
        Vx = 0.0;
        Vy = 1.0;
        Vz = 0.0;
        break;
    case '4': // 左视图
        xview = -5.0;
        yview = 0.0;
        zview = -5.0;
        xref = 0.0;
        yref = 0.0;
        zref = -5.0;
        Vx = 0.0;
        Vy = 1.0;
        Vz = 0.0;
        break;
    case '6': // 右视图
        xview = 5.0;
        yview = 0.0;
        zview = -5.0;
        xref = 0.0;
        yref = 0.0;
        zref = -5.0;
        Vx = 0.0;
        Vy = 1.0;
        Vz = 0.0;
        break;
    case '8': // 后视图
        xview = 0.0;
        yview = 0.0;
        zview = -10.0;
        xref = 0.0;
        yref = 0.0;
        zref = -5.0;
        Vx = 0.0;
        Vy = 1.0;
        Vz = 0.0;
        break;
    case '5': // 顶视图
        xview = 0.0;
        yview = 5.0;
        zview = -5.0;
        xref = 0.0;
        yref = 0.0;
        zref = -5.0;
        Vx = 0.0;
        Vy = 0.0;
        Vz = -1.0;
        break;
    case '0':
        angle = 0.0f;
        memset(axis, 0, sizeof(axis));
        memset(objectXform, 0, sizeof(objectXform)); // 变换矩阵重置
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (i == j)
                {
                    objectXform[i][j] = 1.0f;
                }
            }
        }
        break;
    case 'r': // 旋转
        transModeSelected = Rotate;
        break;
    case 't': // 平移
        transModeSelected = Translate;
        break;
    case 'c': // 缩放
        transModeSelected = Scale;
        break;

    case '1': // 切换正投影和透视投影
        flag = !flag;
        myReshape(winWidth, winHeight);
        break;
    case 'p'://切换斜平行投影和正平行投影
        flag_pro = !flag_pro;
        myReshape(winWidth, winHeight);
        break;
    case 27: // ESC 键（ASCII: 27）退出
        exit(0);
        break;
    default:
        break;
    }
    glutPostRedisplay(); // 重绘
}
void keyboard(unsigned char key, int x, int y)
{
    userEventAction(key);
}

GLfloat vertices[8][3] = {
    {-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0}, {1.0, 1.0, -1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, 1.0}, {-1.0, 1.0, 1.0} };

GLfloat vertices_tri[4][3] = {
    {0.0, 1.0, 0.0} // 上顶点
    ,
    {-1.0, -1.0, 1.0} // 左下顶点
    ,
    {1.0, -1.0, 1.0} // 右下顶点
    ,
    {-1.0, 1.0, -1.0} };
// 绘制三棱锥
void drawtriangle(void)
{
    glBegin(GL_TRIANGLES);

    // 前侧面
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(2.0, 1.0, 2.0); // 上顶点
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(1.0, -1.0, 3.0); // 左下顶点
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(3.0, -1.0, 3.0); // 右下顶点

    // 右侧面
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(2.0, 1.0, 2.0); // 上顶点
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(3.0, -1.0, 3.0); // 左下顶点
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(3.0, -1.0, 1.0); // 右下顶点

    // 后侧面
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(2.0, 1.0, 2.0); // 上顶点
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(3.0, -1.0, 1.0); // 左下顶点
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0); // 右下顶点

    // 左侧面
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(2.0, 1.0, 2.0); // 上顶点
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0); // 左下顶点
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(1.0, -1.0, 3.0); // 右下顶点

    glEnd();
}
GLfloat colors[8][3] = {
    {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 1.0}, {0.0, 1.0, 1.0} };

void polygon(int a, int b, int c, int d)
{
    // draw a polygon via list of vertices
    glBegin(GL_POLYGON);

    glColor3fv(colors[a]);
    glVertex3fv(vertices[a]);

    glColor3fv(colors[b]);
    glVertex3fv(vertices[b]);

    glColor3fv(colors[c]);
    glVertex3fv(vertices[c]);

    glColor3fv(colors[d]);
    glVertex3fv(vertices[d]);

    glEnd();
}

void drawcube(void)
{ // map vertices to faces

    polygon(1, 0, 3, 2);
    polygon(3, 7, 6, 2);
    polygon(7, 3, 0, 4);
    polygon(2, 6, 5, 1);
    polygon(4, 5, 6, 7);
    polygon(5, 4, 0, 1);

} // 绘制正方体

float time_a = 0.0;
float deltaTime = 0.01;

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // view transform
    glLoadIdentity();
    glTranslatef(0.0, 0.0, viewxform_z);

    if (transModeSelected == Translate)
    {
        // 平移
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(axis[0], axis[1], axis[2]);
        glMultMatrixf((GLfloat*)objectXform);
        glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)objectXform);
        glPopMatrix();
    }
    else if (transModeSelected == Rotate)
    {
        // 旋转部分
        glPushMatrix();
        glLoadIdentity();
        glRotatef(angle, axis[0], axis[1], axis[2]);
        glMultMatrixf((GLfloat*)objectXform);
        glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)objectXform);
        glPopMatrix();
    }
    else if (transModeSelected == Scale)
    {
        // 缩放部分
        glPushMatrix();
        glLoadIdentity();
        glScalef(axis[0] + 1.0f, axis[1] + 1.0f, axis[2] + 1.0f);
        glMultMatrixf((GLfloat*)objectXform);
        glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)objectXform);
        glPopMatrix();
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(xview, yview, zview, xref, yref, zref, Vx, Vy, Vz);
    glTranslatef(viewxform_x, viewxform_y, viewxform_z);

    glPushMatrix();
    glMultMatrixf((GLfloat*)objectXform);
    // 绘制正方体
    drawcube();
    // 绘制三角锥
    drawtriangle();
    // 绘制平台
    glTranslatef(0.0, -1.0, 0.0); // 中间值控制平台离物体高低距离
    glScalef(7.0, 0.1, 7.0);
    glColor3f(0.5, 0.5, 0.5);
    glutSolidCube(1.0);
    glPopMatrix();

    glFlush();
    glutSwapBuffers();
    // glutPostRedisplay();//都会触发 display() 函数以重新绘制窗口。
}

struct menuEntryStruct
{
    const char* label;
    char key;
};
static menuEntryStruct mainMenu[] = {
    "Reset", '0',
    "Rotate", 'r',
    "Translate", 't',
    "Scale", 'c',
    "glOrtho to glFrustum", '1',
    "quit", 27,             // ESC 键（ASCII: 27）
    "front view", '2',      // 正视图
    "left view", '4',       // 左视图
    "right view", '6',      // 右视图
    "back view", '8',       // 后视图
    "Bird's-eye view", '5', // 正视图
    "oblique or orthogonal projection",'p'//切换斜平行投影or正平行投影
};
int mainMenuEntries = sizeof(mainMenu) / sizeof(menuEntryStruct);

void selectMain(int choice) // 序号->key 映射
{
    userEventAction(mainMenu[choice].key); // 调用通用动作解释函数
}

void init(void)
{
    glLineWidth(3.0);
    glEnable(GL_DEPTH_TEST); // 启用深度测试剔除遮挡面

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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("Example");

    init();

    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);

    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);

    glutMainLoop();

    return 0;
}