#include <GLUT/glut.h>
#include <cmath>
#include <cstdio>

// ---------------- 全局变量 ----------------
float lamp_x = 0.0f; // 灯罩在底座上的X坐标
float lamp_y = 0.0f; // 灯罩在底座上的Y坐标

float base_radius = 2.5f;       // 增大底座半径
float base_height = 0.3f;       // 增加底座厚度
float hemisphere_radius = 1.0f; // 增大灯罩半径

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

// 光照强度相关变量
float light_intensity = 1.0f;     // 光照强度，范围0.0-3.0
const float intensity_step = 0.2f; // 每次调整的步长增加，使变化更明显

float base_light_intensity = 1.5f;     // 基础光照强度
float min_light_intensity = 0.3f;      // 最小光照强度
float center_x = 0.0f;                 // 底座中心X坐标
float center_y = 0.0f;                 // 底座中心Y坐标

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
    
    // 设置 GL_LIGHT0 的属性（主光源）
    GLfloat ambient0[] = {0.5f, 0.45f, 0.4f, 1.0f};    // 偏暖的环境光
    GLfloat diffuse0[] = {2.0f, 1.8f, 1.6f, 1.0f};    // 偏黄的漫反射光
    GLfloat specular0[] = {1.0f, 0.95f, 0.9f, 1.0f};  // 略微发黄的高光
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
    if (!lightOn) {
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
    glClearColor(0.15f, 0.12f, 0.1f, 1.0f);  // 深褐色背景
}

// ---------------- 更新光照 ----------------
void updateLighting() {
    // 计算当前位置到中心的距离
    float distance = sqrt(pow(lamp_x - center_x, 2) + pow(lamp_y - center_y, 2));
    float max_distance = base_radius - hemisphere_radius;  // 最大可移动距离
    
    // 根据距离和开关状态计算光照强度
    float intensity_factor = 1.0f - (distance / max_distance) * 0.7f;  // 保留30%的最小强度
    float current_intensity = base_light_intensity * (min_light_intensity + (1.0f - min_light_intensity) * intensity_factor);
    
    // 应用用户调整的光照强度
    current_intensity *= light_intensity;
    
    // 如果灯是关闭的，将强度设为很小的值以模拟环境光
    if (!lightOn) {
        current_intensity = 0.1f;  // 保留微弱的环境光
    }
    
    // 应用光照强度
    GLfloat ambient0[] = {0.4f * current_intensity, 0.4f * current_intensity, 0.4f * current_intensity, 1.0f};
    GLfloat diffuse0[] = {2.0f * current_intensity, 1.9f * current_intensity, 1.7f * current_intensity, 1.0f};
    
    // 更新光源位置
    float scale_z = 0.6f;
    float lamp_z = hemisphere_radius * scale_z;
    GLfloat position0[] = {lamp_x, lamp_y, lamp_z, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, position0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    
    // 根据灯的状态设置衰减
    if (lightOn && use_custom_atten) {
        GLfloat constant = 1.0f;
        GLfloat linear = 0.02f;
        GLfloat quadratic = 0.005f;
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic);
    } else if (!lightOn) {
        // 灯关闭时，使用较强的衰减来限制光照范围
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 2.0f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1.0f);
    }
}

// ---------------- 绘制底座 ----------------
void drawBase() {
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

// ---------------- 绘制灯罩（改进版） ----------------
float calculateCurrentBrightness() {
    float distance = sqrt(pow(lamp_x - center_x, 2) + pow(lamp_y - center_y, 2));
    float max_distance = base_radius - hemisphere_radius;
    float intensity_factor = 1.0f - (distance / max_distance) * 0.7f;  // 保留30%的最小强度
    return min_light_intensity + (1.0f - min_light_intensity) * intensity_factor;
}

void drawLampCover() {
    glPushMatrix();
    
    // 计算灯罩中心的z坐标，使其底部与底座平齐
    float scale_z = 0.6f; // Z轴缩放因子，使灯罩更扁
    float lamp_z = hemisphere_radius * scale_z;
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
    glScalef(1.0f, 1.0f, scale_z);
    
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
    if (lightOn) {
        float brightness = calculateCurrentBrightness();
        GLfloat bottom_emission[] = {
            0.3f * brightness,  // 稍微减弱底部的发光强度
            0.3f * brightness,
            0.25f * brightness,
            0.2f
        };
        glMaterialfv(GL_FRONT, GL_EMISSION, bottom_emission);
    } else {
        // 使用已定义的 no_emission 而不是重新定义
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
void drawLightSource() {
    if (!lightOn) return;
    
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
        1.0f
    };
    GLfloat medium_emissive[] = {
        0.8f * brightness,
        0.75f * brightness,
        0.6f * brightness,
        0.7f
    };
    GLfloat soft_emissive[] = {
        0.6f * brightness,
        0.55f * brightness,
        0.4f * brightness,
        0.5f
    };
    
    // 增强发光效果
    glMaterialfv(GL_FRONT, GL_EMISSION, bright_emissive);
    GLUquadric *quad = gluNewQuadric();
    gluSphere(quad, 0.2f, 30, 30);  // 增大中心灯泡
    
    // 启用混合以创建光晕效果
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    // 绘制多层光晕
    glMaterialfv(GL_FRONT, GL_EMISSION, medium_emissive);
    glutSolidSphere(0.3f, 20, 20);  // 增大中等光晕
    
    glMaterialfv(GL_FRONT, GL_EMISSION, soft_emissive);
    glutSolidSphere(0.4f, 20, 20);  // 增大外层光晕
    
    glDisable(GL_BLEND);
    gluDeleteQuadric(quad);
    
    GLfloat no_emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    
    glPopMatrix();
}

// ---------------- 绘制地面 ----------------
void drawGround() {
    glPushMatrix();
    
    // 设置材质属性
    GLfloat mat_ambient[] = {0.25f, 0.22f, 0.2f, 1.0f};   // 暖灰色环境光反射
    GLfloat mat_diffuse[] = {0.35f, 0.32f, 0.3f, 1.0f};   // 暖灰色漫反射
    GLfloat mat_specular[] = {0.5f, 0.47f, 0.45f, 1.0f};  // 暖色调高光
    GLfloat mat_shininess[] = {25.0f};  // 降低光泽度使其看起来更柔和
    
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
        case 'o': // 切换灯光状态
        case 'O':
            lightOn = !lightOn;
            printf("Light is %s\n", lightOn ? "ON" : "OFF");
            break;
        case '+': // 增加光照强度
        case '=':
            light_intensity += intensity_step;
            if (light_intensity > 3.0f) light_intensity = 3.0f;
            printf("Light intensity: %.1f\n", light_intensity);
            break;
        case '-': // 减少光照强度
        case '_':
            light_intensity -= intensity_step;
            if (light_intensity < 0.2f) light_intensity = 0.2f;
            printf("Light intensity: %.1f\n", light_intensity);
            break;
        case 27: exit(0); // ESC退出
    }
    glutPostRedisplay();
}

// ---------------- 特殊键事件 ----------------
void handleSpecialKey(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    camera_angle_x += 5.0f; break;  // 向上看
        case GLUT_KEY_DOWN:  camera_angle_x -= 5.0f; break;  // 向下看
        case GLUT_KEY_LEFT:  camera_angle_y -= 5.0f; break;  // 向左看
        case GLUT_KEY_RIGHT: camera_angle_y += 5.0f; break;  // 向右看
    }
    
    // 限制俯仰角在 -89 到 89 度之间
    if (camera_angle_x > 89.0f) camera_angle_x = 89.0f;
    if (camera_angle_x < -89.0f) camera_angle_x = -89.0f;
    
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
            isDragging = 0;
        }
    }
}

// 修改鼠标移动事件以移动灯泡
void handleMotion(int x, int y) {
    if (isDragging) {
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