#include "common.h"

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
// 添加Button数组中的特殊模式按钮
// 灯光状态
bool lightOn = false;

// 光照强度相关变量
float light_intensity = 0.5f;       // 初始光照强度设置为0.0，表示完全关闭
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
bool isSpecialMode = false;
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
    {0.5f, 0.9f, 0.35f, 0.18f, "Special Mode", false}
};