#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ---------------- 全局变量 ----------------
float lamp_x = 0.0f;        // 灯罩在底座上的X坐标
float lamp_y = 0.0f;        // 灯罩在底座上的Y坐标

float base_radius = 2.0f;   // 底座半径
float base_height = 0.1f;   // 底座厚度
float hemisphere_radius = 0.85f; // 半球半径

float max_distance = 2.0f;  // 距离最大值，用于光强衰减
int   use_custom_atten = 0; // 是否使用自定义光衰减（0不使用，1使用）

// 相机参数
float x_pos = 0.0f;
float y_pos = -4.0f;
float z_pos = 1.3f;

// 观察目标点
float lookAtX = 0.0f;
float lookAtY = 0.0f;
float lookAtZ = 0.0f;

// 是否显示帮助信息
int show_help = 1;

// 为了使模型更精细，增加细分度
int slices = 100;
int stacks = 100;

// 函数声明
void initLighting();
void initProjection(int w, int h);
void updateLighting();
void drawBase();
void drawLampCover();
void drawFloor();
void display();
void reshape(int w, int h);
void handleKeyboard(unsigned char key, int x, int y);
void handleSpecialKey(int key, int x, int y);
void drawText(float x, float y, const char *string);
void drawInfoText();
void createMenu();
void menuFunc(int option);
void initLampCoverMaterial(float intensity);

// ---------------- 初始化光照 ----------------
void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat lightAmbient[]  = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // 初始光位置
    GLfloat lightPos[] = {0.0f, 0.0f, base_height + hemisphere_radius/2.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // 默认不开启衰减
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,   0.0f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel(GL_SMOOTH);
}

// ---------------- 改进后的灯罩材质 ----------------
void initLampCoverMaterial(float intensity) {
    // 使用较柔和的半透明材质，模拟磨砂玻璃感
    GLfloat matAmbient[]   = {0.4f, 0.5f, 0.7f, 0.3f}; 
    GLfloat matDiffuse[]   = {0.4f, 0.5f, 0.7f, 0.3f};
    GLfloat matSpecular[]  = {0.8f, 0.8f, 0.8f, 0.3f};
    GLfloat matShininess[] = {90.0f};
    // 稍微有些发光，随强度变化
    GLfloat matEmission[]  = {intensity*0.5f, intensity*0.5f, intensity*0.6f, 0.3f};

    glMaterialfv(GL_FRONT, GL_AMBIENT,   matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glMaterialfv(GL_FRONT, GL_EMISSION,  matEmission);
}

// ---------------- 初始化投影 ----------------
void initProjection(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

// ---------------- 光照更新 ----------------
void updateLighting() {
    float dist = sqrtf(lamp_x*lamp_x + lamp_y*lamp_y);
    float intensity = 1.0f - (dist / max_distance);
    if (intensity < 0.1f) intensity = 0.1f; 

    GLfloat lightPos[] = {lamp_x, lamp_y, base_height + hemisphere_radius/2.0f, 1.0f}; 
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    if (use_custom_atten == 0) {
        // 手动调整光强
        GLfloat lightDiffuse[] = {intensity, intensity, intensity, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,   0.0f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,0.0f);
    } else {
        // 使用自定义衰减，使光照更自然
        GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.7f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,   0.1f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,0.02f);
    }

    initLampCoverMaterial(intensity);
}

// ---------------- 绘制底座（增加层次感与“雕刻”） ----------------
void drawBase() {
    // 增强底座材质，略有金属质感
    GLfloat matAmbient[]   = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat matDiffuse[]   = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat matSpecular[]  = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat matShininess[] = {30.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT,   matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    glPushMatrix();
    // 底层圆柱
    gluCylinder(quad, base_radius, base_radius, base_height, slices, stacks);

    // 底部封底
    glPushMatrix();
    glRotatef(180.0f,1.0f,0.0f,0.0f);
    gluDisk(quad,0.0f, base_radius, slices, stacks);
    glPopMatrix();

    // 顶部平面
    glTranslatef(0.0f,0.0f,base_height);
    gluDisk(quad,0.0f, base_radius, slices, stacks);

    // 在顶部增加一层较小的圆柱，形成层次感
    float innerHeight = 0.05f;
    float innerRadius = base_radius * 0.8f;
    glTranslatef(0.0f,0.0f,innerHeight);
    gluCylinder(quad, innerRadius, innerRadius, innerHeight, slices, stacks);
    glTranslatef(0.0f,0.0f,innerHeight);
    gluDisk(quad, 0.0f, innerRadius, slices, stacks);

    // 再增加一环略小的圆盘作为装饰
    float ringRadiusOuter = innerRadius * 0.9f;
    float ringRadiusInner = ringRadiusOuter * 0.8f;
    glTranslatef(0.0f,0.0f,0.01f);
    gluDisk(quad, ringRadiusInner, ringRadiusOuter, slices, stacks);

    glPopMatrix();
    gluDeleteQuadric(quad);
}

// ---------------- 绘制半球罩（增大分辨率，更平滑） ----------------
void drawLampCover() {
    GLdouble eqn[] = {0.0, 0.0, 1.0, 0.0}; 
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, eqn);

    glPushMatrix();
    // 将原本朝上的半球翻转
    glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
    // 按半球半径缩放
    glScalef(hemisphere_radius, hemisphere_radius, hemisphere_radius);
    glutSolidSphere(1.0, slices, stacks);
    glPopMatrix();

    glDisable(GL_CLIP_PLANE0);
}

// ---------------- 绘制地面（棋盘格） ----------------
void drawFloor() {
    int gridCount = 20;
    float size = 10.0f;
    float step = (size * 2) / gridCount;

    for (int i = 0; i < gridCount; i++) {
        for (int j = 0; j < gridCount; j++) {
            float x = -size + i * step;
            float y = -size + j * step;
            // 简单棋盘颜色
            if ((i + j) % 2 == 0) {
                GLfloat c[] = {0.7f, 0.7f, 0.7f, 1.0f};
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c);
            } else {
                GLfloat c[] = {0.5f, 0.5f, 0.5f, 1.0f};
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c);
            }
            glNormal3f(0.0f,0.0f,1.0f);
            glBegin(GL_QUADS);
                glVertex3f(x, y, 0.0f);
                glVertex3f(x+step, y, 0.0f);
                glVertex3f(x+step, y+step, 0.0f);
                glVertex3f(x, y+step, 0.0f);
            glEnd();
        }
    }
}

// ---------------- 显示文本 ----------------
void drawText(float x, float y, const char *string) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], 0, viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (size_t i = 0; i < strlen(string); i++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);
    }

    glEnable(GL_LIGHTING);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ---------------- 显示信息 ----------------
void drawInfoText() {
    if (!show_help) return;

    int line = 20;
    drawText(10, line, "[W/S]: Move camera up/down");
    line += 15;
    drawText(10, line, "[A/D]: Move camera left/right");
    line += 15;
    drawText(10, line, "[Z/X]: Move camera forward/backward");
    line += 15;
    drawText(10, line, "[Arrow keys]: Move lamp cover (can't exceed base)");
    line += 15;
    drawText(10, line, "[H]: Toggle this help");
    line += 15;
    drawText(10, line, "[M]: Toggle custom attenuation mode");
    line += 15;
    drawText(10, line, "[ESC]: Exit");

    line += 20;
    char buf[128];
    snprintf(buf, 128, "Lamp pos: (%.2f, %.2f)", lamp_x, lamp_y);
    drawText(10, line, buf);
    line += 15;
    snprintf(buf, 128, "Camera pos: (%.2f, %.2f, %.2f)", x_pos, y_pos, z_pos);
    drawText(10, line, buf);
    line += 15;
    snprintf(buf, 128, "Custom attenuation: %s", use_custom_atten ? "ON" : "OFF");
    drawText(10, line, buf);
}

// ---------------- 显示回调函数 ----------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(x_pos, y_pos, z_pos, lookAtX, lookAtY, lookAtZ, 0.0,1.0,0.0);

    updateLighting(); // 更新光照与材质

    // 绘制地面
    drawFloor();

    // 绘制底座（已增强）
    drawBase();

    // 将半球稍微放低，让它更贴合底座
    float hemisphere_z = base_height + hemisphere_radius * 0.2f;

    glPushMatrix();
    glTranslatef(lamp_x, lamp_y, hemisphere_z);
    drawLampCover();
    glPopMatrix();

    // 绘制UI信息
    drawInfoText();

    glutSwapBuffers();
}

// ---------------- 窗口重塑回调 ----------------
void reshape(int w, int h) {
    if(h==0) h=1;
    initProjection(w, h);
}

// ---------------- 键盘控制 ----------------
void handleKeyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'w': y_pos += 0.1f; break;
        case 's': y_pos -= 0.1f; break;
        case 'a': x_pos -= 0.1f; break;
        case 'd': x_pos += 0.1f; break;
        case 'z': z_pos -= 0.1f; break;
        case 'x': z_pos += 0.1f; break;
        case 'h': show_help = !show_help; break;
        case 'm': use_custom_atten = !use_custom_atten; break;
        case 27: exit(0); // ESC退出
    }
    glutPostRedisplay();
}

// ---------------- 特殊键控制灯罩位置 ----------------
void handleSpecialKey(int key, int x, int y) {
    float step = 0.05f;
    switch (key) {
        case GLUT_KEY_UP:    lamp_y += step; break;
        case GLUT_KEY_DOWN:  lamp_y -= step; break;
        case GLUT_KEY_LEFT:  lamp_x -= step; break;
        case GLUT_KEY_RIGHT: lamp_x += step; break;
    }

    // 保证半球不超出底座范围
    float boundary = base_radius - hemisphere_radius;
    float dist2 = lamp_x*lamp_x + lamp_y*lamp_y;
    float boundary2 = boundary * boundary;
    if (dist2 > boundary2) {
        float dist = sqrtf(dist2);
        lamp_x = lamp_x * (boundary/dist);
        lamp_y = lamp_y * (boundary/dist);
    }

    glutPostRedisplay();
}

// ---------------- 菜单功能 ----------------
void menuFunc(int option) {
    switch(option) {
        case 1:
            use_custom_atten = 0;
            break;
        case 2:
            use_custom_atten = 1;
            break;
        case 3:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void createMenu() {
    int menu = glutCreateMenu(menuFunc);
    glutAddMenuEntry("Use manual intensity (no attenuation)", 1);
    glutAddMenuEntry("Use custom attenuation", 2);
    glutAddMenuEntry("Exit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// ---------------- 主函数 ----------------
int main(int argc, char** argv) {
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Soft Glowing Lamp with Transparent Hemisphere (Improved)");

    initLighting();
    initProjection(800, 600);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutSpecialFunc(handleSpecialKey);

    createMenu();

    glutMainLoop();
    return 0;
}