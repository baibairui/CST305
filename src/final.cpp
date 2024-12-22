#include <GLUT/glut.h> // 修正后的GLUT头文件路径
#include <cmath>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 全局纹理变量
GLuint groundTexture, wallTexture,baseTexture,cupTexture;

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
// macOS-specific includes
#include <unistd.h>
#include <signal.h>
#endif

// ---------------- 全局变量 ----------------
// 1. 添加全局变量
GLfloat shadowMat[16];  // 阴影投影矩阵
GLfloat groundPlane[] = {0.0f, 0.0f, 1.0f, 5.0f};  // 地面平面方程

float lamp_x = 0.0f; // 灯罩在底座上的X坐标
float lamp_y = 0.0f; // 灯罩在底座上的Y坐标

float base_radius = 2.5f;       // 增大底座半径
float base_height = 0.3f;       // 增加底座厚度
float hemisphere_radius = 1.3f; // 增大灯罩半径

// 杯子全局位置和缩放参数
static float posX = 4.0f, posY = 2.0f, posZ = -4.8f; // 杯子底部中心位置
static float size = 1.0f;                           // 杯子和把手的整体缩放比例


//------------------------------------------------------↓
// 新

static float cupRadius = 1.0f;   // 杯外壁半径
static float cupHeight = 2.0f;   // 杯身高度
static float cupThickness = 0.08f; // 杯壁厚度

// 把手相关参数
static float handleRadius = 0.75f;   // 把手中心到杯身中心的距离
static float handleTubeRadius = 0.1f; // 把手本身的粗细
static float handleStartAngle = 180.0f; // 把手起始角度
static float handleEndAngle = 360.0f;   // 把手终止角度
static int handleSlices = 50;          // 把手横截面分段
static int handleStacks = 50;          // 把手环向分段

float max_distance = 2.0f; // 最大距离（用于光亮衰减）
int use_custom_atten = 0;  // 自定义光衰减开关

// 摄像机参数（基于角度的球面坐标）
float camera_distance = 5.0f;  // 摄像机与原点的距离
float camera_angle_x = 20.0f;  // 绕X轴的旋转角度（俯仰角）
float camera_angle_y = -30.0f; // 绕Y轴的旋转角度（偏航角）

float camera_x = 0.0f;
float camera_y = 0.0f;
float camera_z = 0.0f;

// 鼠标控制变量
int isDragging = 0;           // 是否正在拖动
int initial_mouse_x = 0;      // 初始鼠标X位置
int initial_mouse_y = 0;      // 初始鼠标Y位置
int last_mouse_x = 0;         // 上一次鼠标X位置
int last_mouse_y = 0;         // 上一次鼠标Y位置
float sensitivity = 0.3f;     // 旋转灵敏度
const int clickThreshold = 5; // 点击与拖动的阈值

// 灯光状态
bool lightOn = false;

// 光照强度相关变量
float light_intensity = 0.0f;       // 初始光照强度设置为0.0，表示完全关闭
const float intensity_step = 0.15f; // 每次调整的步长增加，使变化更平滑

// 光照强度范围
const float max_light_intensity = 3.0f; // 最大光照强度
float current_intensity = 0.0f;         // 当前光照强度

float min_light_intensity = 0.3f; // 最小光照强度
float center_x = 0.0f;            // 底座中心X坐标
float center_y = 0.0f;            // 底座中心Y坐标
float offsetY = -0.1f;            // 调整按钮向下偏移的量

// ---------------- 粒子效果相关 ----------------
// 粒子结构体
struct Particle {
    float x, y, z;    // 位置
    float vx, vy, vz; // 速度
    float life;       // 生命周期
    float r, g, b;    // 颜色
    float maxLife;    // 最大生命周期属性
};

// 预定义粒子颜色
const struct {
    float r, g, b;
} ParticleColors[] = {
    {1.0f, 0.5f, 0.0f},  // 橙色
    {1.0f, 0.8f, 0.0f},  // 金黄
    {0.8f, 0.2f, 0.2f},  // 红色
    {0.9f, 0.6f, 0.3f},  // 暖橙
    {1.0f, 0.4f, 0.4f},  // 粉红
    {0.8f, 0.8f, 0.4f},  // 暖黄
};
const int NUM_COLORS = sizeof(ParticleColors) / sizeof(ParticleColors[0]);
std::vector<Particle> particles;
const int MAX_PARTICLES = 10000;        // 增加粒子数量
const float PARTICLE_SPEED = 0.008f;  // 粒子移动速度

// 添加全局变量
const float MIN_LIFE_TIME = 6.0f;  // 最小生命周期（秒）
const float MAX_LIFE_TIME = 7.0f;  // 最大生命周期（秒）
const float LIFE_SPEED = 0.0002f;  // 生命周期变化速度

// 闹钟相关变量
struct AlarmTime
{
    int hour;
    int minute;
    int second;
    bool isSet;
} alarmTime = {0, 0, 0, false}; // 初始为未设置

bool fadeInStarted = false;
bool alarmTriggered = false;
const int fadeInDuration = 15; // 缓慢亮灯持续时间设置为15秒

float rotation_angle = 0.0f;
bool rotating = false;

#ifdef __APPLE__
pid_t alarm_pid = -1; // 全局变量，用于存储子进程的 PID
#endif

// ---------------- 模式定义 ----------------
enum Mode {
    NORMAL,
    ALARM,
    ATMOSPHERE
};

Mode currentMode = NORMAL; // 初始为Normal模式
// 在全局变量部分添加模式标志
// bool isAlarmMode = false; // false 为普通模式，true 为闹钟模式

// UI按钮相关结构体和变量
struct Button
{
    float x, y;       // 按钮位置
    float width;      // 按钮宽度
    float height;     // 按钮高度
    const char *text; // 按钮文字
    bool isPressed;   // 是否被按下
};

// 修改按钮数组定义，添加模式切换按钮
Button buttons[] = {
    {-0.9f, 0.9f, 0.35f, 0.18f, "Light ON/OFF", false},
    {-0.5f, 0.9f, 0.25f, 0.18f, "Dimmer -", false},
    {-0.2f, 0.9f, 0.25f, 0.18f, "Dimmer +", false},
    {0.1f, 0.9f, 0.35f, 0.18f, "Mode Switch", false}, 
};

// ---------------- 函数声明 ----------------
void initLighting();
void updateLighting();
void drawBase();
void drawDrinkingMug();
void drawLampCover();
void drawLightSource();
void drawGround();
void drawWalls();
void restrictLampPosition();
void updateCamera();
void handleMouse(int button, int state, int x, int y);
void handleMotion(int x, int y);
void handleKeyboard(unsigned char key, int x, int y);
void handleSpecialKey(int key, int x, int y);
void createMenu();
void menuFunc(int option);
void display();
GLuint loadTexture(const char* filename);
void loadTextures();
void reshape(int w, int h);
void checkAlarm();
void fadeInLight(int value);
void rotateLamp(int value);
void playAlarmSound();
#ifdef __APPLE__
void stopAlarmSound();
#endif
void renderBitmapString(float x, float y, float z, void *font, const char *string);
void resetAlarmMode();
void resetNormalMode();
void drawButton(const Button &btn);
// 粒子相关函数
bool isParticleIntersectingWalls(const Particle& p);
void initParticle(Particle& p);
void updateParticles();
void resetParticles();
void drawParticles();
void resetAtmosphereMode();

//------------------------------------------------------↓
void drawPartialTorus(float bigRadius, float smallRadius,
                      float startAngleDeg, float endAngleDeg,
                      int slices, int stacks)
{
    float startRad = startAngleDeg * (3.14159f / 180.0f);
    float endRad = endAngleDeg * (3.14159f / 180.0f);
    float angleRange = endRad - startRad;
    float dTheta = angleRange / slices;
    float dPhi = (2.0f * 3.14159f) / stacks;


    for (int i = 0; i < slices; i++)
    {
        float theta0 = startRad + i * dTheta;
        float theta1 = theta0 + dTheta;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= stacks; j++)
        {
            float phi = j * dPhi;
            float x0 = (bigRadius + smallRadius * cos(phi)) * cos(theta0);
            float y0 = (bigRadius + smallRadius * cos(phi)) * sin(theta0);
            float z0 = smallRadius * sin(phi);

            float x1 = (bigRadius + smallRadius * cos(phi)) * cos(theta1);
            float y1 = (bigRadius + smallRadius * cos(phi)) * sin(theta1);
            float z1 = smallRadius * sin(phi);

            glNormal3f(cos(phi) * cos(theta0), cos(phi) * sin(theta0), sin(phi));
            glVertex3f(x0, y0, z0);

            glNormal3f(cos(phi) * cos(theta1), cos(phi) * sin(theta1), sin(phi));
            glVertex3f(x1, y1, z1);
        }
        glEnd();
    }
}

//------------------------------------------------------
// 绘制水杯
//------------------------------------------------------
void drawDrinkingMug()
{
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    glPushMatrix();
    {
        glTranslatef(posX, posY, posZ);
        glScalef(size, size, size);

        // 绘制水杯外壁（仅贴纹理）
        glEnable(GL_TEXTURE_2D); // 启用 2D 纹理
        glBindTexture(GL_TEXTURE_2D, cupTexture); // 绑定水杯纹理
        gluQuadricTexture(quad, GL_TRUE); // 启用自动生成纹理坐标

        glColor3f(1.0f, 1.0f, 1.0f); // 确保纹理显示正常
        gluCylinder(quad, cupRadius, cupRadius, cupHeight, 36, 5);
        glDisable(GL_TEXTURE_2D); // 禁用纹理

        // 绘制水杯底部（无纹理）
        glPushMatrix();
        glColor3f(0.7f, 0.7f, 0.75f);
        gluDisk(quad, 0.0, cupRadius, 36, 1);
        glPopMatrix();

        // 绘制水杯内壁（无纹理）
        float innerRadius = cupRadius - cupThickness;
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, cupThickness);
        glColor3f(0.9f, 0.9f, 0.95f);
        gluCylinder(quad, innerRadius, innerRadius, cupHeight - cupThickness, 36, 5);
        glPopMatrix();

        // 绘制水杯把手（无纹理）
        glPushMatrix();
        glTranslatef(cupRadius, 0.0f, cupHeight * 0.5f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        glColor3f(0.7f, 0.7f, 0.75f);
        drawPartialTorus(handleRadius, handleTubeRadius,
                         handleStartAngle, handleEndAngle,
                         handleSlices, handleStacks);
        glPopMatrix();
    }
    glPopMatrix();

    gluDeleteQuadric(quad);
}

//------------------------------------------------------↑
// 2. 添加阴影矩阵计算函数
void calculateShadowMatrix(GLfloat shadowMat[16], GLfloat groundplane[4], GLfloat lightpos[4]) {
    GLfloat dot;
    
    dot = groundplane[0] * lightpos[0] +
          groundplane[1] * lightpos[1] +
          groundplane[2] * lightpos[2] +
          groundplane[3] * lightpos[3];

    shadowMat[0]  = dot - lightpos[0] * groundplane[0];
    shadowMat[4]  = 0.0f - lightpos[0] * groundplane[1];
    shadowMat[8]  = 0.0f - lightpos[0] * groundplane[2];
    shadowMat[12] = 0.0f - lightpos[0] * groundplane[3];

    shadowMat[1]  = 0.0f - lightpos[1] * groundplane[0];
    shadowMat[5]  = dot - lightpos[1] * groundplane[1];
    shadowMat[9]  = 0.0f - lightpos[1] * groundplane[2];
    shadowMat[13] = 0.0f - lightpos[1] * groundplane[3];

    shadowMat[2]  = 0.0f - lightpos[2] * groundplane[0];
    shadowMat[6]  = 0.0f - lightpos[2] * groundplane[1];
    shadowMat[10] = dot - lightpos[2] * groundplane[2];
    shadowMat[14] = 0.0f - lightpos[2] * groundplane[3];

    shadowMat[3]  = 0.0f - lightpos[3] * groundplane[0];
    shadowMat[7]  = 0.0f - lightpos[3] * groundplane[1];
    shadowMat[11] = 0.0f - lightpos[3] * groundplane[2];
    shadowMat[15] = dot - lightpos[3] * groundplane[3];
}
// 3. 添加阴影绘制函数
void drawShadow() {
    // 更新光源位置
    float base_z = -5.0f + base_height;
    float lamp_z = base_z + hemisphere_radius; 
    GLfloat lightPosition[] = {lamp_x, lamp_y, lamp_z, 1.0f};
    
    // 计算阴影矩阵
    calculateShadowMatrix(shadowMat, groundPlane, lightPosition);
    // 保存当前深度函数
    GLint savedDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &savedDepthFunc);
    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 修改深度测试函数，避免z-fighting
    glDepthFunc(GL_LEQUAL);
    glPushMatrix();
        // 应用阴影矩阵
        glMultMatrixf(shadowMat);
        
        // 设置阴影颜色（半透明黑色）
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
        
        // 1. 绘制灯罩阴影
        glPushMatrix();
            glTranslatef(lamp_x, lamp_y, lamp_z);
            GLUquadric *quad = gluNewQuadric();
            gluSphere(quad, hemisphere_radius, 50, 50);
            gluDeleteQuadric(quad);
        glPopMatrix();
        
        // 2. 绘制底座阴影
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, -5.0f);
            quad = gluNewQuadric();
            gluCylinder(quad, base_radius, base_radius, base_height, 50, 50);
            gluDeleteQuadric(quad);
        glPopMatrix();


        // （c）水杯阴影
        glPushMatrix();
            // 先把坐标变换到水杯所在的位置 (posX, posY, posZ)
            glTranslatef(posX, posY, posZ);
            glScalef(size, size, size);

            // 不要再调用 glColor3f(...) 或材质，但要画跟水杯一致的几何
            // （1）杯身
            GLUquadric *quad2 = gluNewQuadric();
            gluCylinder(quad2, cupRadius, cupRadius, cupHeight, 36, 5);

            // （2）杯底
            glPushMatrix();
                gluDisk(quad2, 0.0, cupRadius, 36, 1);
            glPopMatrix();

            // （3）杯内壁
            float innerRadius = cupRadius - cupThickness;
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, cupThickness);
                gluCylinder(quad2, innerRadius, innerRadius, cupHeight - cupThickness, 36, 5);
            glPopMatrix();

            // （4）杯把手 (局部环面)
            glPushMatrix();
                glTranslatef(cupRadius, 0.0f, cupHeight * 0.5f);
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
                glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
                // 这里同样画局部环面即可（把 drawPartialTorus(...) 的实现搬过来或直接调用）
                // 但记得别再换颜色
                drawPartialTorus(handleRadius, handleTubeRadius, handleStartAngle, handleEndAngle, handleSlices, handleStacks);
            glPopMatrix();

            gluDeleteQuadric(quad2);

        glPopMatrix();

    // 收尾
    glPopMatrix();
    
    glDepthFunc(savedDepthFunc);
    glEnable(GL_LIGHTING); 
    glDisable(GL_BLEND);
}
// 添加绘制正方体的函数
void drawCube() {
    glPushMatrix();
    
    // 设置正方体位置：靠近灯的底座
    float cube_size = 0.5f; // 正方体大小
    // 位置在底座边缘附近
    float cube_x = base_radius - cube_size + 2.0f;
    float cube_y = 2.0f;
    float cube_z = -5.0f + base_height; // 与底座顶部齐平
    
    glTranslatef(cube_x, cube_y, cube_z);
    
    // 设置材质属性
    GLfloat cube_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat cube_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat cube_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat cube_shininess[] = {100.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, cube_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cube_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, cube_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, cube_shininess);
    
    // 绘制正方体
    glBegin(GL_QUADS);
    
    // 前面
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-cube_size/2, -cube_size/2, cube_size/2);
    glVertex3f(cube_size/2, -cube_size/2, cube_size/2);
    glVertex3f(cube_size/2, -cube_size/2, -cube_size/2);
    glVertex3f(-cube_size/2, -cube_size/2, -cube_size/2);
    
    // 后面
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-cube_size/2, cube_size/2, -cube_size/2);
    glVertex3f(cube_size/2, cube_size/2, -cube_size/2);
    glVertex3f(cube_size/2, cube_size/2, cube_size/2);
    glVertex3f(-cube_size/2, cube_size/2, cube_size/2);
    
    // 顶面
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-cube_size/2, -cube_size/2, cube_size/2);
    glVertex3f(-cube_size/2, cube_size/2, cube_size/2);
    glVertex3f(cube_size/2, cube_size/2, cube_size/2);
    glVertex3f(cube_size/2, -cube_size/2, cube_size/2);
    
    // 底面
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-cube_size/2, -cube_size/2, -cube_size/2);
    glVertex3f(cube_size/2, -cube_size/2, -cube_size/2);
    glVertex3f(cube_size/2, cube_size/2, -cube_size/2);
    glVertex3f(-cube_size/2, cube_size/2, -cube_size/2);
    
    // 右面
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(cube_size/2, -cube_size/2, -cube_size/2);
    glVertex3f(cube_size/2, -cube_size/2, cube_size/2);
    glVertex3f(cube_size/2, cube_size/2, cube_size/2);
    glVertex3f(cube_size/2, cube_size/2, -cube_size/2);
    
    // 左面
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-cube_size/2, -cube_size/2, -cube_size/2);
    glVertex3f(-cube_size/2, cube_size/2, -cube_size/2);
    glVertex3f(-cube_size/2, cube_size/2, cube_size/2);
    glVertex3f(-cube_size/2, -cube_size/2, cube_size/2);
    
    glEnd();
    
    glPopMatrix();
}
// ---------------- 初始化光照 ----------------
void initLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // 设置颜色材质模式，允许材质属性由颜色定义
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // 设置 GL_LIGHT0 的属性（主光源）
    GLfloat ambient0[] = {0.7f, 0.7f, 0.7f, 1.0f};  // 偏暖的环境光
    GLfloat diffuse0[] = {2.0f, 1.8f, 1.6f, 1.0f};   // 偏黄的漫反射光
    GLfloat specular0[] = {1.0f, 0.95f, 0.9f, 1.0f}; // 略微发黄的高光
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);

    // 初始光源位置在灯罩中心
    GLfloat position0[] = {lamp_x, lamp_y, hemisphere_radius * 0.6f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, position0);

    // 设置 GL_LIGHT0 的初始衰减参数
    GLfloat constant = 1.0f;
    GLfloat linear = 0.014f;
    GLfloat quadratic = 0.0007f;

    // 如果灯是关闭的，使用强衰减
    if (!lightOn)
    {
        linear = 2.0f;
        quadratic = 1.0f;
    }

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel(GL_SMOOTH);

    // 设置背景色为温暖的深色
    glClearColor(0.15f, 0.12f, 0.1f, 1.0f); // 深褐色背景
}

// ---------------- 更新光照 ----------------
void updateLighting()
{
    // 计算当前位置到中心的距离
    float distance = sqrt(pow(lamp_x - center_x, 2) + pow(lamp_y - center_y, 2));
    float max_dist = base_radius - hemisphere_radius;

    // 根据距离计算强度因子（距离中心越近越亮）
    float intensity_factor = 1.0f - (distance / max_dist) * 0.7f; // 增加距离影响
    intensity_factor = fmax(intensity_factor, 0.3f);              // 保持最小亮度在30%

    // 计算最终光照强度
    float adjusted_intensity = current_intensity * intensity_factor;

    // 如果灯是关闭的，将强度设为很小的值以模拟环境光
    if (!lightOn)
    {
        adjusted_intensity = 0.1f;
    }

    // 应用光照强度
    GLfloat ambient0[] = {0.4f * adjusted_intensity, 0.4f * adjusted_intensity, 0.4f * adjusted_intensity, 1.0f};
    GLfloat diffuse0[] = {2.0f * adjusted_intensity, 1.9f * adjusted_intensity, 1.7f * adjusted_intensity, 1.0f};

        // 计算光源位置
    float base_z = -5.0f + base_height;
    float lamp_z = base_z + hemisphere_radius;
    
    GLfloat position0[] = {lamp_x, lamp_y, lamp_z, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, position0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);

    // 根据灯的状态设置衰减
    if (lightOn && use_custom_atten)
    {
        GLfloat constant = 1.0f;
        GLfloat linear = 0.02f;
        GLfloat quadratic = 0.005f;
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic);
    }
    else if (!lightOn)
    {
        // 灯关闭时，使用较强的衰减来限制光照范围
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 2.0f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1.0f);
    }
}

// ---------------- 绘制底座 ----------------
void drawBase()
{
    glPushMatrix();

    // 将底座放置在地面上 (z = -5.0f)
    glTranslatef(0.0f, 0.0f, -5.0f);

    //-------------------------------------
    // 1. 启用 2D 纹理、绑定到底座纹理ID
    //-------------------------------------
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, baseTexture); // 假设你把底座纹理存到 baseTexture 里

    // 设置材质属性（可根据需要调节）
    GLfloat mat_ambient[]  = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat mat_diffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f}; 
    GLfloat mat_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat mat_shininess[] = {32.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // 让颜色与材质配合，此处使用接近白色(主要依赖纹理颜色)
    glColor3f(1.0f, 1.0f, 1.0f);

    //-------------------------------------
    // 2. 创建 Quadric 并启用自动生成纹理坐标
    //-------------------------------------
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluQuadricTexture(quad, GL_TRUE); // 让该 quad 自动生成纹理坐标

    // 绘制底座圆柱体
    gluCylinder(quad, base_radius, base_radius, base_height, 50, 50);

    // 绘制底座顶面（一个圆盘）
    glTranslatef(0.0f, 0.0f, base_height);
    gluDisk(quad, 0.0f, base_radius, 50, 50);

    gluDeleteQuadric(quad);

    //-------------------------------------
    // 3. 关闭纹理
    //-------------------------------------
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

// ---------------- 计算当前亮度 ----------------
float calculateCurrentBrightness()
{
    float distance = sqrt(pow(lamp_x - center_x, 2) + pow(lamp_y - center_y, 2));
    float max_dist = base_radius - hemisphere_radius;

    // 增强距离对亮度的影响
    float intensity_factor = 1.0f - (distance / max_dist) * 0.7f;
    intensity_factor = fmax(intensity_factor, 0.3f); // 保持最小亮度

    // 根据当前模式调整亮度计算
    switch (currentMode) {
        case NORMAL:
            return light_intensity * intensity_factor;
        case ALARM:
            return min_light_intensity + (1.0f - min_light_intensity) * intensity_factor;
        case ATMOSPHERE:
            return 0.8f * intensity_factor; // 可以根据需要调整
        default:
            return 0.0f;
    }
}

void drawLampCover() {
    glPushMatrix();
    
    // 计算灯罩位置：地面(-5.0f) + 底座高度 + 灯罩半径
    float base_z = -5.0f + base_height;
    float lamp_z = base_z + hemisphere_radius*0.5;
    
    // 移动到灯罩位置
    glTranslatef(lamp_x, lamp_y, lamp_z);
    
    // 更透明的灯罩材质
    GLfloat glass_ambient[] = {0.1f, 0.1f, 0.1f, 0.2f};
    GLfloat glass_diffuse[] = {0.7f, 0.7f, 0.7f, 0.15f};  // 更透明
    GLfloat glass_specular[] = {1.0f, 1.0f, 1.0f, 0.2f};
    GLfloat glass_shininess[] = {128.0f};
    
    if (lightOn) {
        // 灯亮时的内部发光效果，根据位置调整亮度
        float brightness = calculateCurrentBrightness();
        GLfloat glass_emission[] = {
            0.4f * brightness,
            0.4f * brightness,
            0.35f * brightness,
            0.3f
        };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glass_emission);
    }
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glass_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glass_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glass_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, glass_shininess);
    
    // 双面渲染和混合设置
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 绘制内外两层灯罩
    GLUquadric *quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    
    // 外层灯罩
    gluSphere(quad, hemisphere_radius, 50, 50);
    
    if (lightOn) {
        // 内层发光层，同样根据位置调整亮度
        float brightness = calculateCurrentBrightness();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        GLfloat inner_emission[] = {
            0.6f * brightness,
            0.6f * brightness,
            0.5f * brightness,
            0.4f
        };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, inner_emission);
        gluSphere(quad, hemisphere_radius * 0.95f, 50, 50);
    }
    
    gluDeleteQuadric(quad);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    
    // 重置发光
    GLfloat no_emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, no_emission);
    
    glPopMatrix();
}

void drawLightSource() {
    if (!lightOn)
        return;

    glPushMatrix();
    
    // 计算光源位置：与灯罩位置相同
    float base_z = -5.0f + base_height;
    float lamp_z = base_z + hemisphere_radius;
    
    glTranslatef(lamp_x, lamp_y, lamp_z);
    
    // 计算当前亮度
    float brightness = calculateCurrentBrightness() * light_intensity;

    // 根据位置调整发光效果
    GLfloat bright_emissive[] = {
        1.0f * brightness,
        0.95f * brightness,
        0.8f * brightness,
        1.0f};
    GLfloat medium_emissive[] = {
        0.8f * brightness,
        0.75f * brightness,
        0.6f * brightness,
        0.7f};
    GLfloat soft_emissive[] = {
        0.6f * brightness,
        0.55f * brightness,
        0.4f * brightness,
        0.5f};

    // 增强发光效果
    glMaterialfv(GL_FRONT, GL_EMISSION, bright_emissive);
    GLUquadric *quad = gluNewQuadric();
    gluSphere(quad, 0.2f, 30, 30); // 增大中心灯泡

    // 启用混合以创建光晕效果
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // 绘制多层光晕
    glMaterialfv(GL_FRONT, GL_EMISSION, medium_emissive);
    gluSphere(quad, 0.3f, 20, 20); // 增大中等光晕

    glMaterialfv(GL_FRONT, GL_EMISSION, soft_emissive);
    gluSphere(quad, 0.4f, 20, 20); // 增大外层光晕

    glDisable(GL_BLEND);
    gluDeleteQuadric(quad);

    // 重置发光
    GLfloat no_emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);

    glPopMatrix();
}

/// 修改后的绘制地面和墙壁函数，应用纹理
void drawGround() {
    glPushMatrix();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    
    // 设置材质属性
    GLfloat mat_ambient[] = {0.25f, 0.22f, 0.2f, 1.0f};
    GLfloat mat_diffuse[] = {0.35f, 0.32f, 0.3f, 1.0f};
    GLfloat mat_specular[] = {0.5f, 0.47f, 0.45f, 1.0f};
    GLfloat mat_shininess[] = {25.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    
    // 绘制带纹理的地面
    glColor3f(1.0f, 1.0f, 1.0f); // 使用纯白色以显示纹理颜色
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, -10.0f, -5.0f);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(10.0f, -10.0f, -5.0f);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(10.0f, 10.0f, -5.0f);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-10.0f, 10.0f, -5.0f);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawWalls() {
    glPushMatrix();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    
    // 设置材质属性
    GLfloat mat_ambient[] = {0.25f, 0.22f, 0.2f, 1.0f};
    GLfloat mat_diffuse[] = {0.35f, 0.32f, 0.3f, 1.0f};
    GLfloat mat_specular[] = {0.5f, 0.47f, 0.45f, 1.0f};
    GLfloat mat_shininess[] = {25.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    
    glColor3f(1.0f, 1.0f, 1.0f); // 确保纹理颜色不被修改
    
    float wall_height = 10.0f; // 从z=-5到z=5
    float wall_length = 10.0f; // 从x=-5到x=5 或 y=-5到y=5
    
    // 左墙 (x = -5.0)
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f); // 法向量指向右侧
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, -10.0f, -10.0f);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(-5.0f, 10.0f, -10.0f);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(-5.0f, 10.0f, 10.0f);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-5.0f, -10.0f, 10.0f);
    glEnd();
    
    
    // 前墙 (y = 5.0)
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f); // 法向量指向后方
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 5.0f, -10.0f);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(10.0f, 5.0f, -10.0f);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(10.0f, 5.0f, 10.0f);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-10.0f, 5.0f, 10.0f);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}



// ---------------- 限制灯罩位置 ----------------
void restrictLampPosition()
{
    float max_range = base_radius - hemisphere_radius;
    float distance = sqrt(lamp_x * lamp_x + lamp_y * lamp_y);
    if (distance > max_range)
    {
        lamp_x *= max_range / distance;
        lamp_y *= max_range / distance;
    }
}

// ---------------- 更新摄像机位置 ----------------
// 更新摄像机位置的函数
void updateCamera() {
    float rad_x = camera_angle_x * M_PI / 180.0f;
    float rad_y = camera_angle_y * M_PI / 180.0f;

    camera_x = camera_distance * cos(rad_x) * sin(rad_y);
    camera_y = camera_distance * sin(rad_x);
    camera_z = camera_distance * cos(rad_x) * cos(rad_y);
}
// 重置氛围模式相关状态
void resetAtmosphereMode()
{
    fadeInStarted = false;
    alarmTriggered = false;
    rotating = false;
    alarmTime.isSet = false;

#ifdef __APPLE__
    stopAlarmSound();
#endif

    // 重置灯光状态为默认
    lightOn = false;
    light_intensity = 1.0f;
    current_intensity = 1.0f;

        // 启用粒子效果
    resetParticles();
}
// 检查粒子是否与墙面相交
bool isParticleIntersectingWalls(const Particle& p)
{
    const float WALL_THRESHOLD = 1.0f;     // 增加阈值使检测范围更大
    const float WALL_RANGE = 10.0f;        // 增加墙面范围
    const float FLOOR_LEVEL = -5.0f;       // 地面高度
    
    // 检测与左墙面的相交 (x = -5.0)
    if (fabs(p.x + 5.0f) < WALL_THRESHOLD &&
        p.y >= -WALL_RANGE && p.y <= WALL_RANGE &&
        p.z >= FLOOR_LEVEL && p.z <= WALL_RANGE)
    {
        return true;
    }

    // 检测与后墙面的相交 (y = 5.0)
    if (fabs(p.y - 5.0f) < WALL_THRESHOLD &&
        p.x >= -WALL_RANGE && p.x <= WALL_RANGE &&
        p.z >= FLOOR_LEVEL && p.z <= WALL_RANGE)
    {
        return true;
    }

    // 检测与地面的相交 (z = -5.0)
    if (fabs(p.z - FLOOR_LEVEL) < WALL_THRESHOLD &&
        p.x >= -WALL_RANGE && p.x <= WALL_RANGE &&
        p.y >= -WALL_RANGE && p.y <= WALL_RANGE)
    {
        return true;
    }

    return false;
}

// 初始化粒子
void initParticle(Particle& p)
{
    // 从光源位置发出
    p.x = lamp_x;
    p.y = lamp_y;
    p.z = 0.0f + hemisphere_radius * 0.6f; // 调整粒子起始高度

    // 随机生成方向向量（球面均匀分布）
    float theta = (rand() % 360) * M_PI / 180.0f;
    float phi = (rand() % 360) * M_PI / 180.0f;

    // 计算速度方向
    p.vx = PARTICLE_SPEED * sin(phi) * cos(theta);
    p.vy = PARTICLE_SPEED * sin(phi) * sin(theta);
    p.vz = PARTICLE_SPEED * cos(phi);

    // 为每个粒子随机分配生命周期
    p.maxLife = MIN_LIFE_TIME + (rand() % 1000) * (MAX_LIFE_TIME - MIN_LIFE_TIME) / 1000.0f;
    p.life = p.maxLife;

    // 随机选择一个预定义的颜色
    int colorIndex = rand() % NUM_COLORS;
    p.r = ParticleColors[colorIndex].r;
    p.g = ParticleColors[colorIndex].g;
    p.b = ParticleColors[colorIndex].b;
    // 添加一些随机变化
    float variation = (rand() % 20 - 10) / 100.0f; // -0.1 到 0.1 的随机变化
    p.r = std::min(1.0f, std::max(0.0f, p.r + variation));
    p.g = std::min(1.0f, std::max(0.0f, p.g + variation));
    p.b = std::min(1.0f, std::max(0.0f, p.b + variation));
}

// 更新粒子系统
void updateParticles()
{
    // 确保粒子数量
    while (particles.size() < MAX_PARTICLES)
    {
        Particle p;
        initParticle(p);
        particles.push_back(p);
    }

    // 更新所有粒子
    for (auto& p : particles)
    {
        // 更新位置
        p.x += p.vx;
        p.y += p.vy;
        p.z += p.vz;

        // 更新生命周期
        p.life -= LIFE_SPEED;

        // 如果生命周期结束，重新初始化粒子
        if (p.life <= 0.0f ||
            p.x < -10.0f || p.x > 10.0f ||
            p.y < -10.0f || p.y > 10.0f ||
            p.z < -10.0f || p.z > 10.0f)
        {
            initParticle(p);
        }
    }
}
// ---------------- 计算阴影矩阵函数 ----------------
void computeShadowMatrix(GLfloat shadowMat[4][4], const GLfloat plane[4], const GLfloat lightPos[4])
{
    GLfloat dot = plane[0] * lightPos[0] +
                  plane[1] * lightPos[1] +
                  plane[2] * lightPos[2] +
                  plane[3] * lightPos[3];
    
    shadowMat[0][0] = dot - lightPos[0] * plane[0];
    shadowMat[0][1] = 0.0f - lightPos[0] * plane[1];
    shadowMat[0][2] = 0.0f - lightPos[0] * plane[2];
    shadowMat[0][3] = 0.0f - lightPos[0] * plane[3];
    
    shadowMat[1][0] = 0.0f - lightPos[1] * plane[0];
    shadowMat[1][1] = dot - lightPos[1] * plane[1];
    shadowMat[1][2] = 0.0f - lightPos[1] * plane[2];
    shadowMat[1][3] = 0.0f - lightPos[1] * plane[3];
    
    shadowMat[2][0] = 0.0f - lightPos[2] * plane[0];
    shadowMat[2][1] = 0.0f - lightPos[2] * plane[1];
    shadowMat[2][2] = dot - lightPos[2] * plane[2];
    shadowMat[2][3] = 0.0f - lightPos[2] * plane[3];
    
    shadowMat[3][0] = 0.0f - lightPos[3] * plane[0];
    shadowMat[3][1] = 0.0f - lightPos[3] * plane[1];
    shadowMat[3][2] = 0.0f - lightPos[3] * plane[2];
    shadowMat[3][3] = dot - lightPos[3] * plane[3];
}

// 重置所有粒子到初始状态
void resetParticles()
{
    particles.clear(); // 清除现有粒子
    // 重新初始化所有粒子
    for(int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle p;
        initParticle(p);
        particles.push_back(p);
    }
}

// 绘制粒子
void drawParticles()
{
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glPointSize(5.0f); // 增大点的大小

    glBegin(GL_POINTS);
    for (const auto& p : particles)
    {   
        // printf("p_x:%f,p_y:%f,p_z:%f",p.x,p.y,p.z);
        // 增加亮度，使相交点更明显
        // 只绘制与墙面相交的粒子
        if (isParticleIntersectingWalls(p))
        {
            float brightness = 1.5f;
            // 使用归一化的生命周期值作为透明度
            float alpha = p.life / p.maxLife;
            glColor4f(p.r * brightness, p.g * brightness, p.b * brightness, alpha);
            glVertex3f(p.x, p.y, p.z);
        }
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}


// ---------------- 渲染位图字符串 ----------------
void renderBitmapString(float x, float y, float z, void *font, const char *string)
{
    const char *c;
    glRasterPos3f(x, y, z);
    for (c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(font, *c);
    }
}
void updateLightPosition()
{
    float base_z = -5.0f + base_height;
    float lamp_z = base_z + hemisphere_radius;
    // 假设 lamp_x, lamp_y 是根据用户输入或动画更新的
    GLfloat lightpos[] = {lamp_x, lamp_y, lamp_z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
}
// ---------------- 显示回调 ----------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateLightPosition();
    
    // 设置透视投影
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)glutGet(GLUT_WINDOW_WIDTH) / (double)glutGet(GLUT_WINDOW_HEIGHT), 1.0, 20.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    updateCamera();
    float base_z = -5.0f + base_height;
    float lamp_z = base_z + hemisphere_radius;
    gluLookAt(-camera_x, -camera_y, camera_distance ,
              lamp_x, lamp_y, lamp_z,
              0.0, 0.0, 0.01);

    updateLighting();
    drawGround();
    drawWalls();
    glEnable(GL_POLYGON_OFFSET_FILL);
    // 这两个参数可根据实际情况微调，(factor, units) 都可以适当加大一点
    glPolygonOffset(-1.0f, -1.0f);
    if (lightOn && currentMode!=ATMOSPHERE)
    {
        drawShadow();  
    }
    // 绘制完成后关闭偏移
    glDisable(GL_POLYGON_OFFSET_FILL);
    drawBase();
    drawLightSource();
    drawLampCover();

    if (currentMode != ATMOSPHERE){
        glDisable(GL_CULL_FACE);
        drawDrinkingMug();
        glEnable(GL_CULL_FACE);
    }
    
    // 如果是Atmosphere模式且灯光开启，则绘制粒子效果
    if (currentMode == ATMOSPHERE )
    {
        glPushMatrix();
        if (!lightOn) {  // 只在灯开启时更新和绘制粒子
            updateParticles();
            drawParticles();
        }
        glPopMatrix();
    }


    // 2. 绘制2D UI (按钮)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // 绘制按钮
    for (const Button &btn : buttons)
    {
        drawButton(btn);
    }

    // 3. 绘制时间信息 (在右下角)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH)+100, 0, glutGet(GLUT_WINDOW_HEIGHT));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 获取窗口尺寸
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

    // 获取当前时间
    time_t now = time(0);
    struct tm *local_time = localtime(&now);
    char currentTimeStr[9];
    snprintf(currentTimeStr, sizeof(currentTimeStr), "%02d:%02d:%02d",
             local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

    // 获取闹钟时间
    char alarmTimeStr[9];
    if (alarmTime.isSet)
    {
        snprintf(alarmTimeStr, sizeof(alarmTimeStr), "%02d:%02d:%02d",
                 alarmTime.hour, alarmTime.minute, alarmTime.second);
    }
    else
    {
        strcpy(alarmTimeStr, "Not Set");
    }

    if (currentMode == ALARM)
    {
        // 绘制背景框
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 半透明背景
        glBegin(GL_QUADS);
        glColor4f(0.2f, 0.2f, 0.4f, 0.8f); // 顶部颜色
        glVertex2f(windowWidth - 300, 110);
        glVertex2f(windowWidth - 20, 110);
        glColor4f(0.1f, 0.1f, 0.3f, 0.8f); // 底部颜色
        glVertex2f(windowWidth - 20, 20);
        glVertex2f(windowWidth - 300, 20);
        glEnd();

        // 发光边框
        glLineWidth(2.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f); // 边框颜色：白色
        glBegin(GL_LINE_LOOP);
        glVertex2f(windowWidth - 200, 20);
        glVertex2f(windowWidth + 80, 20);
        glVertex2f(windowWidth + 80, 110);
        glVertex2f(windowWidth - 200, 110);
        glEnd();
        glLineWidth(1.0f);

        // 禁用灯光以确保文字颜色
        glDisable(GL_LIGHTING);

        // 绘制文字
        void *font = GLUT_BITMAP_HELVETICA_18;

        // "Current Time" 文本
        float text_x1 = windowWidth - 200;
        float text_y1 = 70;                // 高度调整
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // 白色文字
        for (int i = 0; i < 3; i++)
        { // 模拟加粗效果
            glRasterPos2f(text_x1 + i * 0.5f+10, text_y1);
            const char *text1 = "Current Time:";
            for (const char *c = text1; *c != '\0'; c++)
            {
                glutBitmapCharacter(font, *c);
            }
        }

        // 当前时间
        float text_x2 = windowWidth - 200;
        for (int i = 0; i < 1; i++)
        { // 模拟加粗效果
            glRasterPos2f(text_x2 + i * 50.0f+150, text_y1);
            for (const char *c = currentTimeStr; *c != '\0'; c++)
            {
                glutBitmapCharacter(font, *c);
            }
        }

        // "Alarm Time" 文本
        float text_y2 = 40; // 高度调整
        for (int i = 0; i < 3; i++)
        { // 模拟加粗效果
            glRasterPos2f(text_x1 + i * 0.5f+10, text_y2);
            const char *text2 = "Alarm Time:";
            for (const char *c = text2; *c != '\0'; c++)
            {
                glutBitmapCharacter(font, *c);
            }
        }

        // 闹钟时间
        for (int i = 0; i < 1; i++)
        { // 模拟加粗效果
            glRasterPos2f(text_x2 + i * 50.0f+150, text_y2);
            for (const char *c = alarmTimeStr; *c != '\0'; c++)
            {
                glutBitmapCharacter(font, *c);
            }
        }

        // 恢复状态
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
    }

    // 显示模式信息
    glColor3f(1.0f, 1.0f, 1.0f);
    char modeStr[20];
    switch (currentMode) {
        case NORMAL:
            strcpy(modeStr, "Mode: Normal");
            break;
        case ALARM:
            strcpy(modeStr, "Mode: Alarm");
            break;
        case ATMOSPHERE:
            strcpy(modeStr, "Mode: Atmosphere");
            break;
        default:
            strcpy(modeStr, "Mode: Unknown");
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 半透明背景
    glBegin(GL_QUADS);
    glColor4f(0.2f, 0.2f, 0.4f, 0.8f); // 顶部颜色
    glVertex2f(windowWidth - 220, 110);
    glVertex2f(windowWidth - 20, 110);
    glColor4f(0.1f, 0.1f, 0.3f, 0.8f); // 底部颜色
    glVertex2f(windowWidth - 20, 20);
    glVertex2f(windowWidth - 220, 20);
    glEnd();

    // 发光边框
    glLineWidth(2.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f); // 边框颜色：白色
    glBegin(GL_LINE_LOOP);
    glVertex2f(windowWidth - 1490, 20);
    glVertex2f(windowWidth - 1290, 20);
    glVertex2f(windowWidth - 1290, 110);
    glVertex2f(windowWidth - 1490, 110);
    glEnd();
    glLineWidth(1.0f);

    // 禁用灯光以确保文字颜色
    glDisable(GL_LIGHTING);

    // 文字位置计算
    float text_width_calc = 0;
    for (const char *c = modeStr; *c != '\0'; c++)
    {
        text_width_calc += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    }
    float text_x =120 - text_width_calc / 2;
    float text_y = 65;                   // 垂直位置

    // 绘制主文本
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // 设置为完全不透明的白色文字
    glRasterPos2f(text_x, text_y);
    for (const char *c = modeStr; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // 恢复状态
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glutSwapBuffers();
}
GLuint loadTexture(const char* filename)
{
    int width, height, nrChannels;
    // Flip images vertically since OpenGL expects 0.0 on y to be bottom
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data)
    {
        printf("Failed to load texture %s\n", filename);
        return 0;
    }

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else
        format = GL_RGB;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // use mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data); // free image memory

    return texture;
}
void loadTextures()
{
    groundTexture = loadTexture("src/image copy 9.png"); // 替换为您的地面纹理文件
    wallTexture = loadTexture("src/image copy 11.png");     // 替换为您的墙壁纹理文件
    baseTexture = loadTexture("src/image copy 6.png");
    cupTexture = loadTexture("src/image.png");

    if (groundTexture == 0 || wallTexture == 0)
    {
        printf("Error loading textures.\n");
        exit(1);
    }
}
// ---------------- 窗口重塑回调 ----------------
void reshape(int w, int h)
{
    if (h == 0)
        h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

// ---------------- 键盘事件 ----------------
void handleKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'o': // 切换灯光状态
    case 'O':
        if (!fadeInStarted)
        { // 防止在淡入过程中切换灯光
            lightOn = !lightOn;
            printf("Light is %s\n", lightOn ? "ON" : "OFF");
            glutPostRedisplay();
        }
        break;
    case '+': // 增加光照强度
    case '=':
        if (!fadeInStarted)
        { // 防止在淡入过程中调整强度
            light_intensity += intensity_step;
            if (light_intensity > max_light_intensity)
                light_intensity = max_light_intensity;
            current_intensity = min_light_intensity + (max_light_intensity - min_light_intensity) * light_intensity;
            printf("Light intensity: %.2f\n", light_intensity);
            glutPostRedisplay();
        }
        break;
    case '-': // 减少光照强度
    case '_':
        if (!fadeInStarted)
        { // 防止在淡入过程中调整强度
            light_intensity -= intensity_step;
            if (light_intensity < 0.0f)
                light_intensity = 0.0f;
            current_intensity = min_light_intensity + (max_light_intensity - min_light_intensity) * light_intensity;
            printf("Light intensity: %.2f\n", light_intensity);
            glutPostRedisplay();
        }
        break;
    case 'm': // 快捷键切换模式
    case 'M':
        // 切换到下一个模式
        currentMode = static_cast<Mode>((currentMode + 1) % 3);
        if (currentMode == ALARM)
        {
            resetAlarmMode();
        }
        else if (currentMode == NORMAL)
        {
            resetNormalMode();
        }
        else if (currentMode == ATMOSPHERE)
        {
            resetAtmosphereMode();
        }
        printf("Switched to ");
        switch (currentMode) {
            case NORMAL:
                printf("Normal mode.\n");
                break;
            case ALARM:
                printf("Alarm mode.\n");
                break;
            case ATMOSPHERE:
                printf("Atmosphere mode.\n");
                break;
            default:
                printf("Unknown mode.\n");
        }
        glutPostRedisplay();
        break;
    case 'r': // 重置视角
    case 'R':
        camera_distance = 5.0f;
        camera_angle_x = 20.0f;
        camera_angle_y = -30.0f;
        printf("Camera reset to default position.\n");
        glutPostRedisplay();
        break;
    case 27: // ESC退出
#ifdef __APPLE__
        stopAlarmSound(); // 停止音频播放
#endif
        exit(0);
    }
}
void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    
    // 启用光照
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    
    glViewport(100, 100, 600, 400);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0);  // 修改正交投影范围
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // 在这里设置固定的视角
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // 绕X轴旋转30度
    glRotatef(-45.0f, 0.0f, 1.0f, 0.0f); // 绕Y轴旋转45度

    // 设置点的大小
    glPointSize(3.0f);
    
    // 设置光照参数
    GLfloat light_position[] = {-4.0f, -4.0f, -4.0f, 1.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 0.8f, 0.6f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
}

// ---------------- 特殊键事件 ----------------
void handleSpecialKey(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            camera_angle_x += 5.0f;
            break; // 向上看
        case GLUT_KEY_DOWN:
            camera_angle_x -= 5.0f;
            break; // 向下看
        case GLUT_KEY_LEFT:
            camera_angle_y -= 5.0f;
            break; // 向左看
        case GLUT_KEY_RIGHT:
            camera_angle_y += 5.0f;
            break; // 向右看
    }

    // 限制俯仰角在 -89 到 89 度之间
    if (camera_angle_x > 89.0f)
        camera_angle_x = 89.0f;
    if (camera_angle_x < -89.0f)
        camera_angle_x = -89.0f;

    glutPostRedisplay();
}

// ---------------- 鼠标事件回调 ----------------
void handleMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        float ortho_x = (2.0f * x / glutGet(GLUT_WINDOW_WIDTH)) - 1.0f;
        float ortho_y = 1.0f - (2.0f * y / glutGet(GLUT_WINDOW_HEIGHT));

        if (state == GLUT_DOWN)
        {
            // 检查是否点击了按钮
            for (Button &btn : buttons)
            {
                float adjustedY = btn.y + offsetY;
                if (ortho_x >= btn.x && ortho_x <= btn.x + btn.width &&
                    ortho_y >= adjustedY && ortho_y <= adjustedY + btn.height)
                {
                    btn.isPressed = true;

                    // 立即执行按钮动作
                    if (strcmp(btn.text, "Light ON/OFF") == 0)
                    {
                        lightOn = !lightOn;
                        if (currentMode == ALARM)
                        {
                            // 在普通模式下，开灯时恢复之前的强度
                            current_intensity = lightOn ? light_intensity : 0.0f;
                        }
                        printf("Light is %s\n", lightOn ? "ON" : "OFF");
                    }
                    else if (strcmp(btn.text, "Dimmer +") == 0)
                    {
                        light_intensity += intensity_step;
                        if (light_intensity > max_light_intensity)
                            light_intensity = max_light_intensity;
                        if (currentMode == ALARM)
                        {
                            current_intensity = light_intensity;
                        }
                    }
                    else if (strcmp(btn.text, "Dimmer -") == 0)
                    {
                        light_intensity -= intensity_step;
                        if (light_intensity < 0.2f)
                            light_intensity = 0.2f;
                        if (currentMode == ALARM)
                        {
                            current_intensity = light_intensity;
                        }
                    }
                    else if (strcmp(btn.text, "Mode Switch") == 0)
                    {
                        // 切换到下一个模式
                        currentMode = static_cast<Mode>((currentMode + 1) % 3);
                        if (currentMode == ALARM)
                        {
                            resetAlarmMode();
                        }
                        else if (currentMode == NORMAL)
                        {
                            resetNormalMode();
                        }
                        else if (currentMode == ATMOSPHERE)
                        {
                            resetAtmosphereMode();
                        }
                        printf("Switched to ");
                        switch (currentMode) {
                            case NORMAL:
                                printf("Normal mode.\n");
                                break;
                            case ALARM:
                                printf("Alarm mode.\n");
                                break;
                            case ATMOSPHERE:
                                printf("Atmosphere mode.\n");
                                break;
                            default:
                                printf("Unknown mode.\n");
                        }
                    }
                }
            }

            // 如果没有点击按钮，处理拖动
            isDragging = 1;
            initial_mouse_x = x;
            initial_mouse_y = y;
            last_mouse_x = x;
            last_mouse_y = y;
        }
        else if (state == GLUT_UP)
        {
            // 释放按钮
            for (Button &btn : buttons)
            {
                btn.isPressed = false;
            }
            isDragging = 0;
            glutPostRedisplay();
        }
    }
}

// ---------------- 鼠标移动事件 ----------------
void handleMotion(int x, int y)
{
    if (isDragging)
    {
        // 计算屏幕坐标的变化
        float delta_x = (x - last_mouse_x) * 0.01f;
        float delta_y = (y - last_mouse_y) * 0.01f;

        // 根据相机角度调整移动方向
        float angle = -camera_angle_y * M_PI / 180.0f;
        float cos_angle = cos(angle);
        float sin_angle = sin(angle);

        // 应用旋转变换
        lamp_x += cos_angle * delta_x - sin_angle * delta_y;
        lamp_y += sin_angle * delta_x + cos_angle * delta_y;

        restrictLampPosition();
        last_mouse_x = x;
        last_mouse_y = y;
        glutPostRedisplay();
    }
}

// ---------------- 菜单功能 ----------------
void menuFunc(int option)
{
    switch (option)
    {
    case 1:
        use_custom_atten = 0;
        break;
    case 2:
        use_custom_atten = 1;
        break;
    case 3:
#ifdef __APPLE__
        stopAlarmSound(); // 停止音频播放
#endif
        exit(0);
        break;
    }
    glutPostRedisplay();
}

void createMenu()
{
    int menu = glutCreateMenu(menuFunc);
    glutAddMenuEntry("Disable custom attenuation", 1);
    glutAddMenuEntry("Enable custom attenuation", 2);
    glutAddMenuEntry("Exit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// ---------------- 检查闹钟 ----------------
void checkAlarm()
{
    if (!alarmTime.isSet || alarmTriggered)
        return;

    time_t now = time(0);
    struct tm *local_time = localtime(&now);

    // 转换闹钟时间为 time_t
    struct tm alarm_tm = *local_time;
    alarm_tm.tm_hour = alarmTime.hour;
    alarm_tm.tm_min = alarmTime.minute;
    alarm_tm.tm_sec = alarmTime.second;
    time_t alarm_time_t = mktime(&alarm_tm);

    // 获取当前时间的 time_t
    time_t current_time_t = now;

    // 检查是否达到闹钟时间
    if (difftime(current_time_t, alarm_time_t) >= 0)
    {
        alarmTriggered = true;
        printf("Alarm Triggered!\n");
        // 确保光照强度达到最大
        light_intensity = max_light_intensity;
        current_intensity = max_light_intensity;
        // 开始旋转动画（如果尚未开始）
        if (!rotating)
        {
            rotating = true;
            glutTimerFunc(0, rotateLamp, 0);
        }
        // 播放闹钟音
        playAlarmSound();
    }

    // 检查是否到达缓慢亮灯的开始时间（闹钟前fadeInDuration秒）
    time_t fade_in_time = alarm_time_t - fadeInDuration;
    if (difftime(current_time_t, fade_in_time) >= 0 && !fadeInStarted && !alarmTriggered)
    {
        fadeInStarted = true;
        lightOn = true;           // 开启灯光，以便光照强度变化生效
        light_intensity = 0.0f;   // 重置光照强度
        current_intensity = 0.0f; // 重置当前亮度
        printf("Fade-in started.\n");
        // 开始旋转
        rotating = true;
        glutTimerFunc(0, rotateLamp, 0);
        // 播放音乐
        playAlarmSound();
        // 立即开始淡入
        glutTimerFunc(0, fadeInLight, 0);
    }
}

// ---------------- 渐亮灯光 ----------------
void fadeInLight(int value)
{
    if (!fadeInStarted)
        return;

    light_intensity += intensity_step;
    if (light_intensity > max_light_intensity)
        light_intensity = max_light_intensity;

    current_intensity = min_light_intensity + (max_light_intensity - min_light_intensity) * light_intensity * 0.6;

    printf("Fading in... light_intensity: %.2f, current_intensity: %.2f\n", light_intensity, current_intensity); // 调试输出
    glutPostRedisplay();

    if (light_intensity < max_light_intensity)
    {
        glutTimerFunc(1000, fadeInLight, 0); // 每秒增加一次
    }
    else
    {
        fadeInStarted = false;
        printf("Fade-in completed.\n");
    }
}

// ---------------- 旋转动画 ----------------
void rotateLamp(int value)
{
    if (!rotating)
        return;

    rotation_angle += 2.0f; // 每次旋转2度
    if (rotation_angle >= 360.0f)
        rotation_angle -= 360.0f;

    // 更新灯罩位置，使其围绕底座旋转
    float radians = rotation_angle * M_PI / 180.0f;
    lamp_x = (base_radius - hemisphere_radius) * cos(radians);
    lamp_y = (base_radius - hemisphere_radius) * sin(radians);

    // 调试输出
    printf("Rotating lamp to position: (%.2f, %.2f)\n", lamp_x, lamp_y);

    glutPostRedisplay();
    glutTimerFunc(50, rotateLamp, 0); // 每50毫秒旋转一次
}

// ---------------- 播放闹钟音 ----------------
void playAlarmSound()
{
#ifdef _WIN32
    Beep(750, 300); // Windows系统的蜂鸣器
#elif __APPLE__
    if (alarm_pid == -1)
    { // 确保不会启动多个循环
        alarm_pid = fork();
        if (alarm_pid == 0)
        {
            // 子进程：无限循环播放音频
            execlp("sh", "sh", "-c", "while true; do afplay iphone.wav; done", (char *)NULL);
            // 如果 execlp 失败，退出子进程
            exit(1);
        }
        else if (alarm_pid < 0)
        {
            printf("Failed to fork process for alarm sound.\n");
        }
    }
#else
    printf("\a"); // Unix-like系统的警报声音
#endif
}

#ifdef __APPLE__
void stopAlarmSound()
{
    if (alarm_pid > 0)
    {
        kill(alarm_pid, SIGTERM); // 发送终止信号
        alarm_pid = -1;
    }
}
#endif

// ---------------- 定时检查闹钟 ----------------
void timerCheck(int value)
{
    if (currentMode == ALARM)
    {
        checkAlarm();
    }
    glutPostRedisplay();
    glutTimerFunc(1000, timerCheck, 0);
}

// 添加模式重置函数
void resetAlarmMode()
{
    // 重置闹钟模式相关状态
    fadeInStarted = false;
    alarmTriggered = false;
    rotating = false;

    // 设置闹钟时间（示例：当前时间后20秒）
    time_t start_time = time(0);
    struct tm *start_tm = localtime(&start_time);

    alarmTime.hour = start_tm->tm_hour;
    alarmTime.minute = start_tm->tm_min;
    alarmTime.second = start_tm->tm_sec + 20;

    // 处理时间溢出
    if (alarmTime.second >= 60)
    {
        alarmTime.minute += alarmTime.second / 60;
        alarmTime.second %= 60;
    }
    if (alarmTime.minute >= 60)
    {
        alarmTime.hour += alarmTime.minute / 60;
        alarmTime.minute %= 60;
    }
    if (alarmTime.hour >= 24)
    {
        alarmTime.hour %= 24;
    }

    alarmTime.isSet = true;

    // 重置灯光状态
    lightOn = false;
    light_intensity = 0.0f;
    current_intensity = 0.0f;
}

void resetNormalMode()
{
    // 重置普通模式相关状态
    fadeInStarted = false;
    alarmTriggered = false;
    rotating = false;
    alarmTime.isSet = false;

// 停止闹钟声音
#ifdef __APPLE__
    stopAlarmSound();
#endif

    // 重置灯光状态为默认
    lightOn = false;
    light_intensity = 1.0f;
    current_intensity = 1.0f;
}

void drawButton(const Button &btn)
{
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST); // 禁用深度测试确保UI始终可见

    float adjustedY = btn.y + offsetY; // 调整后的 y 坐标

    // 按钮背景 - 半透明渐变
    if (btn.isPressed)
    {
        glBegin(GL_QUADS);
        glColor4f(0.2f, 0.2f, 0.4f, 0.8f); // 顶部颜色
        glVertex2f(btn.x, adjustedY + btn.height);
        glVertex2f(btn.x + btn.width, adjustedY + btn.height);
        glColor4f(0.1f, 0.1f, 0.3f, 0.8f); // 底部颜色
        glVertex2f(btn.x + btn.width, adjustedY);
        glVertex2f(btn.x, adjustedY);
        glEnd();
    }
    else
    {
        glBegin(GL_QUADS);
        glColor4f(0.3f, 0.3f, 0.5f, 0.8f); // 顶部颜色
        glVertex2f(btn.x, adjustedY + btn.height);
        glVertex2f(btn.x + btn.width, adjustedY + btn.height);
        glColor4f(0.2f, 0.2f, 0.4f, 0.8f); // 底部颜色
        glVertex2f(btn.x + btn.width, adjustedY);
        glVertex2f(btn.x, adjustedY);
        glEnd();
    }

    // 发光边框
    glLineWidth(2.0f);
    glColor4f(0.5f, 0.5f, 0.7f, 0.9f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn.x, adjustedY);
    glVertex2f(btn.x + btn.width, adjustedY);
    glVertex2f(btn.x + btn.width, adjustedY + btn.height);
    glVertex2f(btn.x, adjustedY + btn.height);
    glEnd();
    glLineWidth(1.0f);

    // 文字渲染
    void *font = GLUT_BITMAP_HELVETICA_18;
    float scale = 0.0005f; // 缩放因子调整

    // 计算文字宽度
    float text_width = 0.0f;
    for (const char *c = btn.text; *c != '\0'; c++)
    {
        text_width += glutBitmapWidth(font, *c) * scale;
    }

    // 文字位置计算（居中）
    float text_x = btn.x + (btn.width - text_width) / 2.5f;
    float text_y = adjustedY + btn.height * 0.5f; // 文字位置随着按钮调整

    // 绘制文字阴影
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glRasterPos2f(text_x, text_y);
    for (const char *c = btn.text; *c != '\0'; c++)
    {
        glutBitmapCharacter(font, *c);
    }

    // 绘制主文字
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glRasterPos2f(text_x, text_y);
    for (const char *c = btn.text; *c != '\0'; c++)
    {
        glutBitmapCharacter(font, *c);
    }

    // 恢复状态
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glPopMatrix();
}
void idle() {
    glutPostRedisplay();
}
// ---------------- 主函数 ----------------
int main(int argc, char **argv)
{
    // 设置启动时间为当前时间，并设置闹钟时间为启动时间后的20秒
    // 其中，fadeInDuration=15秒，用于测试
    // 即，缓慢亮灯将在启动时间后的5秒开始，持续15秒，最终在启动时间后的20秒触发闹钟
    time_t start_time = time(0);
    struct tm *start_tm = localtime(&start_time);

    // 设置闹钟时间为启动时间后的20秒
    alarmTime.hour = start_tm->tm_hour;
    alarmTime.minute = start_tm->tm_min;
    alarmTime.second = start_tm->tm_sec + 20;

    // 处理秒溢出
    if (alarmTime.second >= 60)
    {
        alarmTime.minute += alarmTime.second / 60;
        alarmTime.second %= 60;
    }

    // 处理分钟溢出
    if (alarmTime.minute >= 60)
    {
        alarmTime.hour += alarmTime.minute / 60;
        alarmTime.minute %= 60;
    }

    // 处理小时溢出
    if (alarmTime.hour >= 24)
    {
        alarmTime.hour %= 24;
    }

    alarmTime.isSet = true;
    alarmTriggered = false;

    printf("Alarm set to %02d:%02d:%02d\n", alarmTime.hour, alarmTime.minute, alarmTime.second);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Interactive Smart Lamp with Alarm");

    loadTextures(); // 加载纹理
    initLighting();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutSpecialFunc(handleSpecialKey);

    // 注册鼠标事件回调
    glutMouseFunc(handleMouse);
    glutMotionFunc(handleMotion);

    createMenu();

    // 设置定时器检查闹钟
    glutTimerFunc(1000, timerCheck, 0);
    init();
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}