#include "light.h"
#include "ui.h"
#include "particle_system.h"
#include "alarm.h"
#include "events.h"
#include "common.h"

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Interactive Smart Lamp");

    initLighting();
    initParticleSystem();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutMouseFunc(handleMouse);
    glutTimerFunc(1000, timerCheck, 0);

    glutMainLoop();
    return 0;
}