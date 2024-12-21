#include <GL/glut.h>
#include <cmath>
#include <cstdio>

// ---------------- 全局变量 ----------------
float lamp_x = 0.0f; // 灯罩在底座上的X坐标
float lamp_y = 0.0f; // 灯罩在底座上的Y坐标

float base_radius = 2.0f;       // 底座半径
float base_height = 0.2f;       // 底座厚度
float hemisphere_radius = 0.8f; // 半球半径

float max_distance = 2.0f; // 最大距离（用于光亮衰减）
int use_custom_atten = 0;  // 自定义光衰减开关

// 摄像机参数（基于角度的球面坐标）
float camera_distance = 5.0f; // 摄像机与原点的距离
float camera_angle_x = 20.0f; // 绕X轴的旋转角度（俯仰角）
float camera_angle_y = -30.0f; // 绕Y轴的旋转角度（偏航角）

float camera_x = 0.0f;
float camera_y = 0.0f;
float camera_z = 0.0f;

// 鼠标控制变量
int isDragging = 0;        // 是否正在拖动
int initial_mouse_x = 0;   // 初始鼠标X位置
int initial_mouse_y = 0;   // 初始鼠标Y位置
int last_mouse_x = 0;      // 上一次鼠标X位置
int last_mouse_y = 0;      // 上一次鼠标Y位置
float sensitivity = 0.3f;  // 旋转灵敏度
const int clickThreshold = 5; // 点击与拖动的阈值

// 灯光状态
bool lightOn = false;

// ---------------- 函数声明 ----------------
void initLighting();
void updateLighting();
void drawBase();
void drawLampCover();
void drawLightSource();
void drawGround();
void restrictLampPosition();
void updateCamera();
void handleMouse(int button, int state, int x, int y);
void handleMotion(int x, int y);
void handleKeyboard(unsigned char key, int x, int y);
void handleSpecialKey(int key, int x, int y);
void createMenu();
void menuFunc(int option);
void display();
void reshape(int w, int h);

// ---------------- 初始化光照 ----------------
void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    // 设置颜色材质模式，允许材质属性由颜色定义
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    
    // 初始光源位置
    GLfloat position[] = {lamp_x, lamp_y, hemisphere_radius * 0.6f, 1.0f}; // 光源初始位置在灯罩中心
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    // 设置光照衰减
    GLfloat constant = 1.0f;
    GLfloat linear = 0.09f;
    GLfloat quadratic = 0.032f;
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glShadeModel(GL_SMOOTH);
}

// ---------------- 更新光照 ----------------
void updateLighting() {
    float distance = sqrt(lamp_x * lamp_x + lamp_y * lamp_y);
    float intensity = 1.0f - (distance / max_distance);
    if (intensity < 0.2f) intensity = 0.2f;
    
    GLfloat diffuse[] = {intensity, intensity, intensity, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    
    // 更新光源位置，与灯罩中心一致
    float scale_z = 0.6f; // 与灯罩缩放因子一致
    float lamp_z = hemisphere_radius * scale_z;
    GLfloat position[] = {lamp_x, lamp_y, lamp_z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, position);
}

// ---------------- 绘制底座 ----------------
void drawBase() {
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f); // 灰色底座
    glTranslatef(0.0f, 0.0f, -base_height / 2);
    GLUquadric *quad = gluNewQuadric();
    gluCylinder(quad, base_radius, base_radius, base_height, 50, 50);
    glTranslatef(0.0f, 0.0f, base_height);
    gluDisk(quad, 0.0f, base_radius, 50, 50);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

// ---------------- 绘制灯罩（改进版） ----------------
void drawLampCover() {
    glPushMatrix();
    
    // 计算灯罩中心的z坐标，使其底部与底座平齐
    float scale_z = 0.6f; // Z轴缩放因子，使灯罩更扁
    float lamp_z = hemisphere_radius * scale_z;
    glTranslatef(lamp_x, lamp_y, lamp_z);
    
    // 设置玻璃材质属性
    GLfloat glass_ambient[] = {0.2f, 0.2f, 0.2f, 0.3f}; // 增加透明度
    GLfloat glass_diffuse[] = {0.6f, 0.6f, 0.8f, 0.3f};
    GLfloat glass_specular[] = {0.9f, 0.9f, 0.9f, 0.3f};
    GLfloat glass_shininess[] = {50.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, glass_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glass_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, glass_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, glass_shininess);
    
    // 启用透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 非均匀缩放，使灯罩更扁
    glScalef(1.0f, 1.0f, scale_z);
    
    // 绘制类球状灯罩
    GLUquadric *quad = gluNewQuadric();
    gluSphere(quad, hemisphere_radius, 50, 50);
    gluDeleteQuadric(quad);
    
    glPopMatrix();
    
    // 绘制平底圆盘，使灯罩底部平整并贴合底座
    glPushMatrix();
    
    // 平移到灯罩底部位置（z=0）
    glTranslatef(lamp_x, lamp_y, 0.0f);
    
    // 设置相同的玻璃材质属性
    glMaterialfv(GL_FRONT, GL_AMBIENT, glass_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glass_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, glass_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, glass_shininess);
    
    // 启用透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 绘制圆盘
    quad = gluNewQuadric();
    gluDisk(quad, 0.0f, hemisphere_radius, 50, 50);
    gluDeleteQuadric(quad);
    
    // 禁用透明度（如果不需要其他透明对象）
    glDisable(GL_BLEND);
    
    glPopMatrix();
}

// ---------------- 绘制光源（灯泡） ----------------
void drawLightSource() {
    if (!lightOn) return; // 仅在灯光开启时绘制
    
    glPushMatrix();
    
    // 光源的位置与灯罩中心位置相同
    float scale_z = 0.6f; // 与灯罩缩放因子一致
    float lamp_z = hemisphere_radius * scale_z;
    glTranslatef(lamp_x, lamp_y, lamp_z);
    
    // 设置发光材质属性
    GLfloat emissive[] = {1.0f, 1.0f, 0.8f, 1.0f}; // 发光颜色（暖白色）
    glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
    
    // 绘制一个小球体作为光源
    GLUquadric *quad = gluNewQuadric();
    gluSphere(quad, 0.1f, 30, 30); // 小球体半径为0.1
    gluDeleteQuadric(quad);
    
    // 取消发光材质属性
    GLfloat no_emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    
    glPopMatrix();
}

// ---------------- 绘制地面 ----------------
void drawGround() {
    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.3f); // 灰色地面
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, -5.0f, -0.01f);
    glVertex3f(5.0f, -5.0f, -0.01f);
    glVertex3f(5.0f, 5.0f, -0.01f);
    glVertex3f(-5.0f, 5.0f, -0.01f);
    glEnd();
    glPopMatrix();
}

// ---------------- 限制灯罩位置 ----------------
void restrictLampPosition() {
    float max_range = base_radius - hemisphere_radius;
    float distance = sqrt(lamp_x * lamp_x + lamp_y * lamp_y);
    if (distance > max_range) {
        lamp_x *= max_range / distance;
        lamp_y *= max_range / distance;
    }
}

// ---------------- 更新摄像机位置 ----------------
void updateCamera() {
    float rad_x = camera_angle_x * M_PI / 180.0f;
    float rad_y = camera_angle_y * M_PI / 180.0f;

    camera_x = camera_distance * cos(rad_x) * sin(rad_y);
    camera_y = camera_distance * sin(rad_x);
    camera_z = camera_distance * cos(rad_x) * cos(rad_y);
}

// ---------------- 显示回调 ----------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCamera(); // 更新摄像机位置

    // 计算灯罩中心位置
    float scale_z = 0.6f; // 与灯罩缩放因子一致
    float lamp_z = hemisphere_radius * scale_z;
    gluLookAt(camera_x, camera_y, camera_z,  // 摄像机位置
              lamp_x, lamp_y, lamp_z,        // 观察点（灯罩中心）
              0.0, 0.0, 1.0);                // 上方向

    updateLighting();

    // 先绘制不透明的对象
    drawGround();
    drawBase();

    // 然后绘制发光的光源
    drawLightSource();

    // 最后绘制透明的灯罩
    drawLampCover();

    glutSwapBuffers();
}

// ---------------- 窗口重塑回调 ----------------
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

// ---------------- 键盘事件 ----------------
void handleKeyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': camera_distance -= 0.2f; if (camera_distance < 1.0f) camera_distance = 1.0f; break;
        case 's': camera_distance += 0.2f; break;
        case 'a': camera_angle_y -= 5.0f; break;
        case 'd': camera_angle_y += 5.0f; break;
        case 'z': camera_angle_x += 5.0f; if (camera_angle_x > 90.0f) camera_angle_x = 90.0f; break;
        case 'x': camera_angle_x -= 5.0f; if (camera_angle_x < -90.0f) camera_angle_x = -90.0f; break;
        case 27: exit(0); // ESC退出
    }
    glutPostRedisplay();
}

// ---------------- 特殊键事件 ----------------
void handleSpecialKey(int key, int x, int y) {
    float step = 0.1f;
    switch (key) {
        case GLUT_KEY_UP: lamp_y += step; break;
        case GLUT_KEY_DOWN: lamp_y -= step; break;
        case GLUT_KEY_LEFT: lamp_x -= step; break;
        case GLUT_KEY_RIGHT: lamp_x += step; break;
    }
    restrictLampPosition();
    glutPostRedisplay();
}

// ---------------- 鼠标事件回调 ----------------
void handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = 1;
            initial_mouse_x = x;
            initial_mouse_y = y;
            last_mouse_x = x;
            last_mouse_y = y;
        }
        else if (state == GLUT_UP) {
            // 计算移动距离
            int delta_x = x - initial_mouse_x;
            int delta_y = y - initial_mouse_y;
            if (abs(delta_x) < clickThreshold && abs(delta_y) < clickThreshold) {
                // 被认为是点击，切换灯光状态
                lightOn = !lightOn;
                glutPostRedisplay();
            }
            isDragging = 0;
        }
    }
}

// ---------------- 鼠标移动事件 ----------------
void handleMotion(int x, int y) {
    if (isDragging) {
        int delta_x = x - last_mouse_x;
        int delta_y = y - last_mouse_y;

        camera_angle_y += delta_x * sensitivity;
        camera_angle_x += delta_y * sensitivity;

        // 限制俯仰角在 -89 到 89 度之间，避免翻转
        if (camera_angle_x > 89.0f) camera_angle_x = 89.0f;
        if (camera_angle_x < -89.0f) camera_angle_x = -89.0f;

        last_mouse_x = x;
        last_mouse_y = y;

        glutPostRedisplay();
    }
}

// ---------------- 菜单功能 ----------------
void menuFunc(int option) {
    switch (option) {
        case 1: use_custom_atten = 0; break;
        case 2: use_custom_atten = 1; break;
        case 3: exit(0); break;
    }
    glutPostRedisplay();
}

void createMenu() {
    int menu = glutCreateMenu(menuFunc);
    glutAddMenuEntry("Disable custom attenuation", 1);
    glutAddMenuEntry("Enable custom attenuation", 2);
    glutAddMenuEntry("Exit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// ---------------- 主函数 ----------------
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Interactive Smart Lamp");

    initLighting();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutSpecialFunc(handleSpecialKey);

    // 注册鼠标事件回调
    glutMouseFunc(handleMouse);
    glutMotionFunc(handleMotion);

    createMenu();

    glutMainLoop();
    return 0;
}