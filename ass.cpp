#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>

// 动态光强参数
float lightIntensity = 0.5f;  // 光强初始值
float time = 0.0f;            // 时间变量
float speed = 0.005f;         // 光强变化速度，减小步长以更平滑

// 相机位置参数
float x_pos = 0.0f;
float y_pos = 0.0f;
float z_pos = 5.0f;

// 初始化光照
void initLighting() {
    glEnable(GL_LIGHTING); // 启用光照
    glEnable(GL_LIGHT0);   // 启用主光源

    // 主光源初始参数
    GLfloat lightPosition[] = { 0.0f, 0.0f, 1.5f, 1.0f }; // 点光源位置
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };  // 环境光
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // 散射光
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 镜面光

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glEnable(GL_DEPTH_TEST); // 启用深度测试
}

// 初始化灯泡材质
void initLampMaterial() {
    GLfloat matAmbient[] = { 0.1f, 0.1f, 0.1f, 0.4f };  // 透明的环境光
    GLfloat matDiffuse[] = { 0.6f, 0.6f, 0.6f, 0.4f };  // 散射光
    GLfloat matSpecular[] = { 0.9f, 0.9f, 0.9f, 0.4f }; // 镜面光
    GLfloat matShininess[] = { 100.0f };                // 高光泽度

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
}

// 初始化灯泡内部光源材质
void initInnerLightMaterial() {
    GLfloat matAmbient[] = { 1.0f, 1.0f, 0.6f, 1.0f };  // 黄色的环境光
    GLfloat matDiffuse[] = { 1.0f, 1.0f, 0.6f, 1.0f };  // 散射光
    GLfloat matSpecular[] = { 1.0f, 1.0f, 0.8f, 1.0f }; // 镜面光
    GLfloat matShininess[] = { 50.0f };                 // 光泽度

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
}

// 初始化投影
void initProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 10.0); // 设置透视投影
    glMatrixMode(GL_MODELVIEW);
}

// 动态更新光照强度
void updateLighting() {
    time += speed; // 时间步长更小，变化更平滑

    // 使用正弦函数模拟光强周期变化，增加平滑度
    lightIntensity = (sin(time * 0.5) + 1.0f) / 2.0f; // 延长周期

    GLfloat lightDiffuse[] = { lightIntensity, lightIntensity, lightIntensity, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glutPostRedisplay(); // 标记窗口需要重新绘制
}

// 绘制球体（灯泡壳）
void drawLampShell() {
    initLampMaterial(); // 设置灯泡外壳材质
    glutSolidSphere(1.0, 50, 50);
}

// 绘制灯泡内部光源
void drawInnerLight() {
    initInnerLightMaterial(); // 设置内部光源材质
    glutSolidSphere(0.5, 50, 50);
}

// 显示函数
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(x_pos, y_pos, z_pos,  // 相机位置
              0.0, 0.0, 0.0,       // 观察目标
              0.0, 1.0, 0.0);      // 上方向

    // 绘制灯泡外壳
    drawLampShell();

    // 绘制灯泡内部光源
    drawInnerLight();

    glutSwapBuffers();
}

// 键盘事件处理
void handleKeyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '+': // 增加光强变化速度
            speed += 0.001f;
            break;
        case '-': // 减少光强变化速度
            if (speed > 0.001f) speed -= 0.001f;
            break;
        case 'w': // 相机向上移动
            y_pos += 0.1f;
            break;
        case 's': // 相机向下移动
            y_pos -= 0.1f;
            break;
        case 'a': // 相机向左移动
            x_pos -= 0.1f;
            break;
        case 'd': // 相机向右移动
            x_pos += 0.1f;
            break;
        case 'z': // 相机靠近场景
            z_pos -= 0.1f;
            break;
        case 'x': // 相机远离场景
            z_pos += 0.1f;
            break;
        case 27: // 按下 ESC 键退出
            exit(0);
    }
    glutPostRedisplay();
}

// 主循环中的空闲函数，用于更新光照
void idle() {
    updateLighting();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Smooth Dynamic Light Bulb");

    initLighting();
    initProjection();

    glutDisplayFunc(display);
    glutIdleFunc(idle);              // 注册空闲回调函数
    glutKeyboardFunc(handleKeyboard); // 注册键盘事件处理
    glutMainLoop();
    return 0;
}