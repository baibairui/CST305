#ifndef UI_H
#define UI_H

struct Button {
    float x, y, width, height;
    const char *text;
    bool isPressed;
};

void drawButton(const Button &btn);
void drawUI();

#endif // UI_H