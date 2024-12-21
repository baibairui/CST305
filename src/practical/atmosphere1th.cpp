#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>

// 粒子结构体
struct Particle {
    float x, y, z;    // 位置
    float vx, vy, vz; // 速度
    float life;       // 生命周期
    float r, g, b;    // 颜色
    float maxLife;    // 添加最大生命周期属性
};

std::vector<Particle> particles;
const int MAX_PARTICLES = 400; // 增加粒子数量
const float PARTICLE_SPEED = 0.00025f; // 粒子移动速度

// 添加全局变量
const float MIN_LIFE_TIME = 6.0f;  // 最小生命周期（秒）
const float MAX_LIFE_TIME = 7.0f;  // 最大生命周期（秒）
const float LIFE_SPEED = 0.0002f;   // 降低生命周期变化速度
float globalAngle = 0.0f; // 控制整体旋转
const float ROTATION_SPEED = 0.002f; // 降低10倍
bool increasing = true; // 控制生命周期是增加还是减少

// 修改碰撞检测函数，增加检测精度
bool isParticleIntersectingWalls(const Particle& p) {
    const float WALL_THRESHOLD = 0.02f; // 降低阈值提高精度
    
    // 检测与左墙面的相交 (x = -1.0)
    if (abs(p.x + 1.0f) < WALL_THRESHOLD &&
        p.y >= -1.0f && p.y <= 1.0f &&
        p.z >= -1.0f && p.z <= 1.0f) {
        return true;
    }
    
    // 检测与地面的相交 (y = -1.0)
    if (abs(p.y + 1.0f) < WALL_THRESHOLD &&
        p.x >= -1.0f && p.x <= 1.0f &&
        p.z >= -1.0f && p.z <= 1.0f) {
        return true;
    }
    
    // 检测与后墙面的相交 (z = -1.0)
    if (abs(p.z + 1.0f) < WALL_THRESHOLD &&
        p.x >= -1.0f && p.x <= 1.0f &&
        p.y >= -1.0f && p.y <= 1.0f) {
        return true;
    }
    
    return false;
}

// 修改初始化粒子函数
void initParticle(Particle& p) {
    // 从光源位置发出
    p.x = p.y = p.z = -0.8f;
    
    // 随机生成方向向量（球面均匀分布）
    float theta = (rand() % 360) * 3.14159f / 180.0f;
    float phi = (rand() % 360) * 3.14159f / 180.0f;
    
    // 计算速度方向
    p.vx = PARTICLE_SPEED * sin(phi) * cos(theta);
    p.vy = PARTICLE_SPEED * sin(phi) * sin(theta);
    p.vz = PARTICLE_SPEED * cos(phi);
    
    // 为每个粒子随机分配一个介于3-4秒之间的最大生命周期
    p.maxLife = MIN_LIFE_TIME + (rand() % 1000) * (MAX_LIFE_TIME - MIN_LIFE_TIME) / 1000.0f;
    p.life = p.maxLife;  // 初始生命值设为最大值
    
    p.r = 1.0f;
    p.g = 0.8f + (rand() % 20) / 100.0f;
    p.b = 0.6f + (rand() % 40) / 100.0f;
}

// 修改更新粒子系统函数
void updateParticles() {
    // 确保粒子数量
    while (particles.size() < MAX_PARTICLES) {
        Particle p;
        initParticle(p);
        particles.push_back(p);
    }

    // 更新所有粒子
    for (auto& p : particles) {
        // 更新位置
        p.x += p.vx;
        p.y += p.vy;
        p.z += p.vz;
        
        // 更新生命周期
        p.life -= LIFE_SPEED;
        
        // 如果生命周期结束，重新初始化粒子
        if (p.life <= 0.0f || 
            p.x < -2.0f || p.x > 2.0f ||
            p.y < -2.0f || p.y > 2.0f ||
            p.z < -2.0f || p.z > 2.0f) {
            initParticle(p);
        }
    }
}

// 修改绘制粒子函数
void drawParticles() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glPointSize(5.0f); // 增大点的大小
    
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        // 只绘制与墙面相交的粒子
        if (isParticleIntersectingWalls(p)) {
            // 增加亮度，使相交点更明显
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

// 绘制地面和墙面
void drawWalls() {
    // 地面
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.2f, 0.2f); // 地面颜色（暗色）
    glVertex3f(-1.0f, -1.0f, -1.0f); // 左下角
    glVertex3f(1.0f, -1.0f, -1.0f);  // 右下角
    glVertex3f(1.0f, -1.0f, 1.0f);   // 右上角
    glVertex3f(-1.0f, -1.0f, 1.0f);  // 左上角
    glEnd();

    // 垂直墙面（Z方向），从 (-1, -1, -1) 到 (1, 1, -1)
    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f); // 墙面颜色（稍微亮一点）
    glVertex3f(-1.0f, -1.0f, -1.0f); // 左下角
    glVertex3f(1.0f, -1.0f, -1.0f);  // 右下角
    glVertex3f(1.0f, 1.0f, -1.0f);   // 右上角
    glVertex3f(-1.0f, 1.0f, -1.0f);  // 左上角
    glEnd();

    // 水平墙面（X方向），从 (-1, -1, -1) 到 (-1, 1, 1)
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.4f, 0.4f); // 墙面颜色（更亮一些）
    glVertex3f(-1.0f, -1.0f, -1.0f); // 左下角
    glVertex3f(-1.0f, 1.0f, -1.0f);  // 左上角
    glVertex3f(-1.0f, 1.0f, 1.0f);   // 右上角
    glVertex3f(-1.0f, -1.0f, 1.0f);  // 右下角
    glEnd();
}
void drawLight() {
    glPushMatrix();
    glTranslatef(-0.8f, -0.8f, -0.8f);  // 移动到墙角
    glColor3f(1.0f, 1.0f, 1.0f);      // 白色
    glutSolidSphere(0.1, 20, 20);      // 绘制球体作为灯
    glPopMatrix();
}

// 显示场景
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawWalls(); // 绘制地面和墙面
    drawLight(); 
    updateParticles();
    drawParticles();
    glutSwapBuffers();
}

// 初始化
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
    glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // 在这里设置固定的视角
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // 绕X轴旋转30度
    glRotatef(-45.0f, 0.0f, 1.0f, 0.0f); // 绕Y轴旋转45度

    // 设置点的大小
    glPointSize(3.0f);
    
    // 设置光照参数
    GLfloat light_position[] = {-0.8f, -0.8f, -0.8f, 1.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 0.8f, 0.6f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
}

void idle() {
    glutPostRedisplay();
}

// 主函数
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Walls Only");
    init();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
