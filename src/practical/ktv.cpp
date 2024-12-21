#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>

// 粒子结构体
struct Particle {
    float x, y, z;    // 位置
    float vx, vy, vz; // 速度
    float life;       // 当前生命周期
    float r, g, b;    // 颜色
    float maxLife;    // 最大生命周期
    
    bool onWall;      // 是否已经贴到墙上
    int  wallID;      // 记录是哪一面墙：0=左墙(x=-1)，1=地面(y=-1)，2=后墙(z=-1)
    
    // 为实现在墙面上的“绕圈”运动，增加以下参数
    float angle;      // 在墙面上的运动角度
    float angleSpeed; // 在墙面上的旋转速度
    float radius;     // 在墙面上绕圈的半径
    float centerY;    // 对于贴到左墙的粒子，用于存放绕圈中心
    float centerZ;
    float centerX;    // 对于贴到地面或后墙的粒子也可能用得上
};

// 一些全局变量
std::vector<Particle> particles;
const int   MAX_PARTICLES   = 400;      // 粒子数量
const float PARTICLE_SPEED  = 0.00025f; // 粒子飞出的移动速度
const float MIN_LIFE_TIME   = 6.0f;     // 最小生命周期（秒）
const float MAX_LIFE_TIME   = 7.0f;     // 最大生命周期（秒）
const float LIFE_SPEED      = 0.0002f;  // 生命周期衰减速度

// 修改检测阈值（决定是否碰到墙面）
bool isParticleIntersectingWalls(const Particle& p) {
    const float WALL_THRESHOLD = 0.02f;

    // 检测与左墙面的相交 (x = -1.0)
    if (fabs(p.x + 1.0f) < WALL_THRESHOLD &&
        p.y >= -1.0f && p.y <= 1.0f &&
        p.z >= -1.0f && p.z <= 1.0f) {
        return true;
    }

    // 检测与地面的相交 (y = -1.0)
    if (fabs(p.y + 1.0f) < WALL_THRESHOLD &&
        p.x >= -1.0f && p.x <= 1.0f &&
        p.z >= -1.0f && p.z <= 1.0f) {
        return true;
    }

    // 检测与后墙面的相交 (z = -1.0)
    if (fabs(p.z + 1.0f) < WALL_THRESHOLD &&
        p.x >= -1.0f && p.x <= 1.0f &&
        p.y >= -1.0f && p.y <= 1.0f) {
        return true;
    }

    return false;
}

// 根据碰撞点来判断它到底撞了哪一面墙，返回枚举值
int whichWall(const Particle& p) {
    const float WALL_THRESHOLD = 0.02f;
    // 左墙
    if (fabs(p.x + 1.0f) < WALL_THRESHOLD) return 0;
    // 地面
    if (fabs(p.y + 1.0f) < WALL_THRESHOLD) return 1;
    // 后墙
    if (fabs(p.z + 1.0f) < WALL_THRESHOLD) return 2;
    // 不符合则返回 -1
    return -1;
}

// 初始化粒子
void initParticle(Particle& p) {
    // 起始位置统一设置在灯光处
    p.x = p.y = p.z = -0.8f;

    // 随机生成方向向量（球面均匀分布简化实现）
    float theta = (rand() % 360) * 3.14159f / 180.0f;
    float phi   = (rand() % 360) * 3.14159f / 180.0f;

    // 计算飞出的速度方向
    p.vx = PARTICLE_SPEED * sin(phi) * cos(theta);
    p.vy = PARTICLE_SPEED * sin(phi) * sin(theta);
    p.vz = PARTICLE_SPEED * cos(phi);

    // 生命值设定
    p.maxLife = MIN_LIFE_TIME + (rand() % 1000) * (MAX_LIFE_TIME - MIN_LIFE_TIME) / 1000.0f;
    p.life    = p.maxLife;  

    // 随机颜色
    p.r = 1.0f;
    p.g = 0.8f + (rand() % 20) / 100.0f;
    p.b = 0.6f + (rand() % 40) / 100.0f;

    // 在刚生成时还没贴墙
    p.onWall = false;
    p.wallID = -1;
}

// 在墙面上初始化一些绕圈运动参数
void initWallMovement(Particle &p) {
    p.onWall     = true;
    p.angle      = (float)(rand() % 360) * 3.14159f / 180.0f; // 随机初始相位
    p.angleSpeed = 0.005f + (rand() % 5) * 0.001f;            // 随机角速度
    p.radius     = 0.2f + (rand() % 30) * 0.01f;              // 随机半径
    
    switch (p.wallID) {
    case 0: // 左墙(x=-1)
        // 围绕 y-z 平面做圆周运动，所以 x = -1 固定
        // 先记录当前 y, z 作为绕圈中心也可以，或者直接绕 (0,0)
        p.centerY = 0.0f;   
        p.centerZ = 0.0f;
        // 也可以让中心点是碰撞时的 y,z
        // p.centerY = p.y;
        // p.centerZ = p.z;
        break;
    case 1: // 地面(y=-1)
        // 围绕 x-z 平面做圆周运动，所以 y = -1 固定
        p.centerX = 0.0f; 
        p.centerZ = 0.0f;
        break;
    case 2: // 后墙(z=-1)
        // 围绕 x-y 平面做圆周运动，所以 z = -1 固定
        p.centerX = 0.0f;
        p.centerY = 0.0f;
        break;
    default:
        break;
    }
}

// 更新粒子的运动
void updateParticles() {
    // 保证粒子数量够用
    while (particles.size() < MAX_PARTICLES) {
        Particle p;
        initParticle(p);
        particles.push_back(p);
    }

    // 更新每个粒子
    for (auto& p : particles) {
        // 如果还没贴墙，就正常飞
        if (!p.onWall) {
            // 更新位置
            p.x += p.vx;
            p.y += p.vy;
            p.z += p.vz;

            // 更新生命周期
            p.life -= LIFE_SPEED;

            // 如果飞出边界或生命周期结束，则重置该粒子
            if (p.life <= 0.0f ||
                p.x < -2.0f || p.x > 2.0f ||
                p.y < -2.0f || p.y > 2.0f ||
                p.z < -2.0f || p.z > 2.0f) {
                initParticle(p);
                continue;
            }

            // 检查是否撞墙
            if (isParticleIntersectingWalls(p)) {
                // 固定在墙面（精确到墙的坐标）
                int wID = whichWall(p);
                if (wID >= 0) {
                    p.wallID = wID;
                    switch (wID) {
                    case 0: p.x = -1.0f; break; // 左墙
                    case 1: p.y = -1.0f; break; // 地板
                    case 2: p.z = -1.0f; break; // 后墙
                    }
                    // 速度清零
                    p.vx = p.vy = p.vz = 0.0f;
                    // 初始化在墙面上的运动（绕圈、随机游走等）
                    initWallMovement(p);
                }
            }
        }
        else {
            // 已经贴到墙上了，就让它在对应墙面上做圆周运动
            p.life -= LIFE_SPEED;   // 生命衰减

            switch (p.wallID) {
            case 0: {
                // 左墙(x=-1), 围绕 centerY, centerZ 做圆周
                p.angle += p.angleSpeed;
                // x固定 -1
                p.x = -1.0f;
                // y,z 按极坐标更新
                p.y = p.centerY + p.radius * cos(p.angle);
                p.z = p.centerZ + p.radius * sin(p.angle);
            } break;
            case 1: {
                // 地面(y=-1), 围绕 centerX, centerZ 做圆周
                p.angle += p.angleSpeed;
                p.y = -1.0f;
                p.x = p.centerX + p.radius * cos(p.angle);
                p.z = p.centerZ + p.radius * sin(p.angle);
            } break;
            case 2: {
                // 后墙(z=-1), 围绕 centerX, centerY 做圆周
                p.angle += p.angleSpeed;
                p.z = -1.0f;
                p.x = p.centerX + p.radius * cos(p.angle);
                p.y = p.centerY + p.radius * sin(p.angle);
            } break;
            default:
                break;
            }

            // 如果生命周期结束，就重置
            if (p.life <= 0.0f) {
                initParticle(p);
            }
        }
    }
}

// 仅绘制已经撞上墙的粒子（“灯光斑”效果）
void drawParticles() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        if (p.onWall) {
            // 使用归一化后的剩余生命值作为 alpha
            float alpha = p.life / p.maxLife;
            float brightness = 1.5f; // 增加亮度
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
    // 地面 y=-1
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.2f, 0.2f); 
    glVertex3f(-1.0f, -1.0f, -1.0f); 
    glVertex3f( 1.0f, -1.0f, -1.0f); 
    glVertex3f( 1.0f, -1.0f,  1.0f); 
    glVertex3f(-1.0f, -1.0f,  1.0f); 
    glEnd();

    // 后墙 z=-1
    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();

    // 左墙 x=-1
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.4f, 0.4f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glEnd();
}

// 绘制灯
void drawLight() {
    glPushMatrix();
    glTranslatef(-0.8f, -0.8f, -0.8f); // 移动到墙角
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidSphere(0.1, 20, 20);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawWalls(); 
    drawLight();
    updateParticles();
    drawParticles();
    glutSwapBuffers();
}

void initGL() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    
    // 启用光照
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);

    // 设置可视区域
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // 设置固定视角
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f);  // 绕X轴30度
    glRotatef(-45.0f, 0.0f, 1.0f, 0.0f); // 绕Y轴-45度

    // 设置光照参数
    GLfloat light_position[] = {-0.8f, -0.8f, -0.8f, 1.0f};
    GLfloat light_ambient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[]  = {1.0f, 0.8f, 0.6f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
}

void idle() {
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Disco Ball Wall Effect");
    initGL();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
