#include <GL/glut.h>
#include <cstdio>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 贴图 ID
GLuint texSun, texEarth, texMoon, texBackground;

// 全局变量
float speedFactor = 1.0f;            // 速度因子
float earthOrbitAngle = 0.0f;        // 地球公转角度
float earthRotationAngle = 0.0f;     // 地球自转角度
float moonOrbitAngle = 0.0f;         // 月亮公转角度
float moonRotationAngle = 0.0f;      // 月亮自转角度

int prevMouseX, prevMouseY;          // 上一次鼠标位置
bool mouseLeftDown = false;          // 左键是否按下
float cameraAngleX = 20.0f, cameraAngleY = 0.0f; // 视角旋转角度
int windowWidth = 800, windowHeight = 600;       // 窗口尺寸

//在屏幕上绘制字符串
void renderBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    while (*string) glutBitmapCharacter(font, *string++);
}

GLuint LoadTextureWithSTB(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        printf("Failed to load texture: %s\n", filename);
        return 0;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texID;
}

// 绘制带贴图的球体 
void drawTexturedSphere(GLuint textureID, float radius) {
    GLUquadric* quad = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, radius, 50, 50);
    gluDeleteQuadric(quad);
    glDisable(GL_TEXTURE_2D);
}

//主绘制函数 
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 绘制背景贴图
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texBackground);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(1, 0); glVertex2f(1, -1);
    glTexCoord2f(1, 1); glVertex2f(1, 1);
    glTexCoord2f(0, 1); glVertex2f(-1, 1);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // 设置摄像机
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -10.0f);
    glRotatef(cameraAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraAngleY, 0.0f, 1.0f, 0.0f);

    // 光源
    GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHT0);

    // 绘制太阳
    glPushMatrix();
    drawTexturedSphere(texSun, 1.0f);
    glPopMatrix();

    // 绘制地球轨道
    glColor3f(0.0f, 0.5f, 1.0f); // 轨道颜色：浅蓝色
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; ++i) {
        float angle = i * 3.14159f / 180.0f; // 弧度
        float x = 4.0f * cos(angle);         // 半径 = 4.0f（与地球轨道对应）
        float z = 4.0f * sin(angle);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();

    // 地球公转 + 自转
    glPushMatrix();
    glRotatef(earthOrbitAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(4.0f, 0.0f, 0.0f);

    // 地球自转
    glPushMatrix();
    glRotatef(earthRotationAngle, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(texEarth, 0.5f);
    glPopMatrix();

    // 绘制月亮轨道（围绕地球）
    glColor3f(0.6f, 0.6f, 0.6f); // 灰色轨道
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; ++i) {
        float angle = i * 3.14159f / 180.0f;
        float x = 1.0f * cos(angle); // 半径 = 1.0f
        float z = 1.0f * sin(angle);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();

    // 月亮公转 + 自转
    glRotatef(moonOrbitAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(1.0f, 0.0f, 0.0f);
    glRotatef(moonRotationAngle, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(texMoon, 0.2f);

    glPopMatrix();

    // 屏幕文字信息
    char buf[128];
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buf, "Speed: %.1f", speedFactor);
    renderBitmapString(10, windowHeight - 20, GLUT_BITMAP_HELVETICA_18, buf);
    sprintf_s(buf, "Earth Orbit: %.1f", earthOrbitAngle);
    renderBitmapString(10, windowHeight - 40, GLUT_BITMAP_HELVETICA_18, buf);
    sprintf_s(buf, "Earth Rotation: %.1f", earthRotationAngle);
    renderBitmapString(10, windowHeight - 60, GLUT_BITMAP_HELVETICA_18, buf);
    sprintf_s(buf, "Moon Orbit: %.1f", moonOrbitAngle);
    renderBitmapString(10, windowHeight - 80, GLUT_BITMAP_HELVETICA_18, buf);
    sprintf_s(buf, "Moon Rotation: %.1f", moonRotationAngle);
    renderBitmapString(10, windowHeight - 100, GLUT_BITMAP_HELVETICA_18, buf);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

// 定时器函数：更新角度并重绘
void update(int value) {
    // 根据速度因子调整各角度增量
    earthOrbitAngle += speedFactor * 0.5f;
    earthRotationAngle += speedFactor * 2.0f;
    moonOrbitAngle += speedFactor * 1.0f;
    moonRotationAngle += speedFactor * 2.0f;
    // 保持角度在 0-360 之间
    if (earthOrbitAngle > 360) earthOrbitAngle -= 360;
    if (earthRotationAngle > 360) earthRotationAngle -= 360;
    if (moonOrbitAngle > 360) moonOrbitAngle -= 360;
    if (moonRotationAngle > 360) moonRotationAngle -= 360;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 每 ~16ms 调用一次，实现 ~60 FPS
}

void keyboard(unsigned char key, int, int) {
    if (key == 'f' || key == 'F') speedFactor += 0.1f;
    else if (key == 's' || key == 'S') {
        speedFactor -= 0.1f;
        if (speedFactor < 0.1f) speedFactor = 0.1f;
    }
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        mouseLeftDown = (state == GLUT_DOWN);
        prevMouseX = x;
        prevMouseY = y;
    }
}

void mouseMotion(int x, int y) {
    if (mouseLeftDown) {
        cameraAngleY += (x - prevMouseX) * 0.5f;
        cameraAngleX += (y - prevMouseY) * 0.5f;
        prevMouseX = x;
        prevMouseY = y;
        glutPostRedisplay();
    }
}

// 窗口大小改变时调整视口和投影
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// -------------------- 主函数 --------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("太阳-地球-月亮系统模拟");

    // 启用深度测试和光照
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // 设置全局环境光
    GLfloat globalAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };   // 全局亮度加倍
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // 设置光源属性（增强亮度）
    GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };   // 更强的环境光
    GLfloat lightDiffuse[] = { 1.2f, 1.2f, 1.2f, 1.0f };   // 强 diffuse 光
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };   // 强反射
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // 设置材质高光属性
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // 加载贴图
    texSun = LoadTextureWithSTB("sun.jpg");
    texEarth = LoadTextureWithSTB("earth.jpg");
    texMoon = LoadTextureWithSTB("moon.jpg");
    texBackground = LoadTextureWithSTB("space.jpg");

    // 注册回调函数
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}

