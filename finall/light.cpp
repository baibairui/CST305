#include "light.h"
#include "common.h"
#include <GLUT/glut.h>

// ---------------- 初始化光照 ----------------

void initLighting() {
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
    // glTranslatef(0.0f, 0.0f, -5.0f + base_height/2); 
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
    float lamp_z = -5.0f + base_height + hemisphere_radius * scale_z;
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