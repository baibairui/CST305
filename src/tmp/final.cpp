#include <GLUT/glut.h> // 修正后的GLUT头文件路径
#include <cmath>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
// macOS-specific includes
#include <unistd.h>
#include <signal.h>
#endif

// ---------------- 全局变量 ----------------
float lamp_x = 0.0f; // 灯罩在底座上的X坐标
float lamp_y = 0.0f; // 灯罩在底座上的Y坐标

float base_radius = 2.5f;       // 增大底座半径
float base_height = 0.3f;       // 增加底座厚度
float hemisphere_radius = 1.3f; // 增大灯罩半径

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
float light_intensity = 1.0f;       // 初始光照强度设置为0.0，表示完全关闭
const float intensity_step = 0.15f; // 每次调整的步长增加，使变化更平滑

// 光照强度范围
const float max_light_intensity = 3.0f; // 最大光照强度
float current_intensity = 0.0f;         // 当前光照强度

float min_light_intensity = 0.3f; // 最小光照强度
float center_x = 0.0f;            // 底座中心X坐标
float center_y = 0.0f;            // 底座中心Y坐标
float offsetY = -0.1f;            // 调整按钮向下偏移的量

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

// 在全局变量部分添加模式标志
bool isAlarmMode = false; // false 为普通模式，true 为闹钟模式

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

// ---------------- 初始化光照 ----------------
void initLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // 设置颜色材质模式，允许材质属性由颜色定义
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // 设置 GL_LIGHT0 的属性（主光源）
    GLfloat ambient0[] = {0.5f, 0.45f, 0.4f, 1.0f};  // 偏暖的环境光
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

    // 更新光源位置
    float scale_z = 0.6f;
    float lamp_z = hemisphere_radius * scale_z;
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

    // 设置材质属性
    GLfloat mat_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat mat_diffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat mat_specular[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat mat_shininess[] = {50.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glColor3f(0.5f, 0.5f, 0.5f); // 灰色底座
    glTranslatef(0.0f, 0.0f, -base_height / 2);
    GLUquadric *quad = gluNewQuadric();
    gluCylinder(quad, base_radius, base_radius, base_height, 50, 50);
    glTranslatef(0.0f, 0.0f, base_height);
    gluDisk(quad, 0.0f, base_radius, 50, 50);
    gluDeleteQuadric(quad);
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

    // 在普通模式下，直接使用 light_intensity
    if (!isAlarmMode)
    {
        return light_intensity * intensity_factor;
    }

    // 在闹钟模式下保持原有行为
    return min_light_intensity + (1.0f - min_light_intensity) * intensity_factor;
}

// ---------------- 绘制灯罩（改进版） ----------------
void drawLampCover()
{
    glPushMatrix();

    // 计算灯罩中心的z坐标，使其底部与底座平齐
    float scale_z = 0.6f; // Z轴缩放因子，使灯罩更扁
    float lamp_z = hemisphere_radius * scale_z;
    glTranslatef(lamp_x, lamp_y, lamp_z);

    // 更透明的灯罩材质
    GLfloat glass_ambient[] = {0.1f, 0.1f, 0.1f, 0.2f};
    GLfloat glass_diffuse[] = {0.7f, 0.7f, 0.7f, 0.15f}; // 更透明
    GLfloat glass_specular[] = {1.0f, 1.0f, 1.0f, 0.2f};
    GLfloat glass_shininess[] = {128.0f};

    if (lightOn)
    {
        // 灯亮时的内部发光效果，根据位置调整亮度
        float brightness = calculateCurrentBrightness();
        GLfloat glass_emission[] = {
            0.4f * brightness,
            0.4f * brightness,
            0.35f * brightness,
            0.3f};
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
    glScalef(1.0f, 1.0f, scale_z);

    GLUquadric *quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    // 外层灯罩
    gluSphere(quad, hemisphere_radius, 50, 50);

    if (lightOn)
    {
        // 内层发光层，同样根据位置调整亮度
        float brightness = calculateCurrentBrightness();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        GLfloat inner_emission[] = {
            0.6f * brightness,
            0.6f * brightness,
            0.5f * brightness,
            0.4f};
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

    // 绘制平底圆盘，使灯罩底部平整并贴合底座
    glPushMatrix();

    // 平移到灯罩底部位置（z=0）
    glTranslatef(lamp_x, lamp_y, 0.0f);

    // 设置相同的玻璃材质属性
    glMaterialfv(GL_FRONT, GL_AMBIENT, glass_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glass_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, glass_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, glass_shininess);

    // 如果灯是开着的，为底部添加发光效果
    if (lightOn)
    {
        float brightness = calculateCurrentBrightness();
        GLfloat bottom_emission[] = {
            0.3f * brightness, // 稍微减弱底部的发光强度
            0.3f * brightness,
            0.25f * brightness,
            0.2f};
        glMaterialfv(GL_FRONT, GL_EMISSION, bottom_emission);
    }
    else
    {
        // 使用已定义的 no_emission
        glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    }

    // 启用透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 绘制圆盘
    quad = gluNewQuadric();
    gluDisk(quad, 0.0f, hemisphere_radius, 50, 50);
    gluDeleteQuadric(quad);

    // 重置发光属性（使用已定义的 no_emission）
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);

    glDisable(GL_BLEND);

    glPopMatrix();
}

// ---------------- 绘制光源（灯泡） ----------------
void drawLightSource()
{
    if (!lightOn)
        return;

    glPushMatrix();
    float scale_z = 0.6f;
    float lamp_z = hemisphere_radius * scale_z;
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
    glutSolidSphere(0.3f, 20, 20); // 增大中等光晕

    glMaterialfv(GL_FRONT, GL_EMISSION, soft_emissive);
    glutSolidSphere(0.4f, 20, 20); // 增大外层光晕

    glDisable(GL_BLEND);
    gluDeleteQuadric(quad);

    // 重置发光
    GLfloat no_emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);

    glPopMatrix();
}

// ---------------- 绘制地面 ----------------
void drawGround()
{
    glPushMatrix();

    // 设置材质属性
    GLfloat mat_ambient[] = {0.25f, 0.22f, 0.2f, 1.0f};  // 暖灰色环境光反射
    GLfloat mat_diffuse[] = {0.35f, 0.32f, 0.3f, 1.0f};  // 暖灰色漫反射
    GLfloat mat_specular[] = {0.5f, 0.47f, 0.45f, 1.0f}; // 暖色调高光
    GLfloat mat_shininess[] = {25.0f};                   // 降低光泽度使其看起来更柔和

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glColor3f(0.35f, 0.32f, 0.3f); // 暖灰色地面
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); // 法向量朝上
    glVertex3f(-5.0f, -5.0f, -0.01f);
    glVertex3f(5.0f, -5.0f, -0.01f);
    glVertex3f(5.0f, 5.0f, -0.01f);
    glVertex3f(-5.0f, 5.0f, -0.01f);
    glEnd();
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
void updateCamera()
{
    float rad_x = camera_angle_x * M_PI / 180.0f;
    float rad_y = camera_angle_y * M_PI / 180.0f;

    camera_x = camera_distance * cos(rad_x) * sin(rad_y);
    camera_y = camera_distance * sin(rad_x);
    camera_z = camera_distance * cos(rad_x) * cos(rad_y);
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

// ---------------- 显示回调 ----------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. 绘制3D场景
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)glutGet(GLUT_WINDOW_WIDTH) / (double)glutGet(GLUT_WINDOW_HEIGHT), 1.0, 20.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    updateCamera();
    float scale_z = 0.6f;
    float lamp_z = hemisphere_radius * scale_z;
    gluLookAt(camera_x, camera_y, camera_z,
              lamp_x, lamp_y, lamp_z,
              0.0, 0.0, 1.0);

    updateLighting();
    drawGround();
    drawBase();
    drawLightSource();
    drawLampCover();

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
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

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

    if (isAlarmMode)
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
        glVertex2f(windowWidth - 300, 20);
        glVertex2f(windowWidth - 20, 20);
        glVertex2f(windowWidth - 20, 110);
        glVertex2f(windowWidth - 300, 110);
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
            glRasterPos2f(text_x1 + i * 0.5f, text_y1);
            const char *text1 = "Current Time:";
            for (const char *c = text1; *c != '\0'; c++)
            {
                glutBitmapCharacter(font, *c);
            }
        }

        // 当前时间
        float text_x2 = windowWidth - 100;
        for (int i = 0; i < 3; i++)
        { // 模拟加粗效果
            glRasterPos2f(text_x2 + i * 0.5f, text_y1);
            for (const char *c = currentTimeStr; *c != '\0'; c++)
            {
                glutBitmapCharacter(font, *c);
            }
        }

        // "Alarm Time" 文本
        float text_y2 = 40; // 高度调整
        for (int i = 0; i < 3; i++)
        { // 模拟加粗效果
            glRasterPos2f(text_x1 + i * 0.5f, text_y2);
            const char *text2 = "Alarm Time:";
            for (const char *c = text2; *c != '\0'; c++)
            {
                glutBitmapCharacter(font, *c);
            }
        }

        // 闹钟时间
        for (int i = 0; i < 3; i++)
        { // 模拟加粗效果
            glRasterPos2f(text_x2 + i * 0.5f, text_y2);
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
    glVertex2f(windowWidth - 220, 20);
    glVertex2f(windowWidth - 20, 20);
    glVertex2f(windowWidth - 20, 110);
    glVertex2f(windowWidth - 220, 110);
    glEnd();
    glLineWidth(1.0f);

    // 禁用灯光以确保文字颜色
    glDisable(GL_LIGHTING);

    // 模式文本
    snprintf(modeStr, sizeof(modeStr), "Mode: %s", isAlarmMode ? "Alarm" : "Normal");
    void *font = GLUT_BITMAP_HELVETICA_18;

    // 文本位置计算
    float text_width = 0;
    for (const char *c = modeStr; *c != '\0'; c++)
    {
        text_width += glutBitmapWidth(font, *c);
    }
    float text_x = 120 - text_width / 2; // 水平居中 (20 + 200)/2
    float text_y = 65;                   // 垂直居中

    // 绘制主文本
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // 设置为完全不透明的白色文字
    glRasterPos2f(text_x, text_y);
    for (const char *c = modeStr; *c != '\0'; c++)
    {
        glutBitmapCharacter(font, *c);
    }

    // 恢复状态
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glutSwapBuffers();
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

// ---------------- 特殊键事件 ----------------
void handleSpecialKey(int key, int x, int y)
{
    switch (key)
    {
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
                        if (!isAlarmMode)
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
                        if (!isAlarmMode)
                        {
                            current_intensity = light_intensity;
                        }
                    }
                    else if (strcmp(btn.text, "Dimmer -") == 0)
                    {
                        light_intensity -= intensity_step;
                        if (light_intensity < 0.2f)
                            light_intensity = 0.2f;
                        if (!isAlarmMode)
                        {
                            current_intensity = light_intensity;
                        }
                    }
                    else if (strcmp(btn.text, "Mode Switch") == 0)
                    {
                        isAlarmMode = !isAlarmMode;
                        if (isAlarmMode)
                        {
                            resetAlarmMode();
                        }
                        else
                        {
                            resetNormalMode();
                        }
                    }
                    glutPostRedisplay();
                    return;
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
    if (isAlarmMode)
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

    glutMainLoop();
    return 0;
}