#include "camera.h"
#include "hud.h"
#include "lighting.h"
#include "scene.h"
#include <GL/freeglut.h>

int windowWidth = 1280;
int windowHeight = 720;

Camera camera;
LightingManager lighting;
Scene scene;
HUD hud;

const float TRANSLATE_STEP = 0.25f;
const float ROTATE_STEP = 5.0f;
const float SCALE_STEP = 0.08f;
const float CAMERA_ROT_STEP = 3.0f;
const float CAMERA_ZOOM_STEP = 0.5f;
const float CAMERA_PAN_STEP = 0.3f;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.applyProjection(windowWidth, windowHeight);

    camera.applyView();

    lighting.applyLighting();

    scene.render(lighting);

    hud.render(windowWidth, windowHeight, camera, lighting, scene);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = (h > 0) ? h : 1;
    glViewport(0, 0, windowWidth, windowHeight);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {

    case '1':
        scene.selectEntity(1);
        break;
    case '2':
        scene.selectEntity(2);
        break;
    case '3':
        scene.selectEntity(3);
        break;
    case '4':
        scene.selectEntity(4);
        break;
    case '0':
        scene.selectEntity(0);
        break;

    case 'w':
    case 'W':
        scene.translateSelected(0.0f, 0.0f, -TRANSLATE_STEP, lighting);
        break;
    case 's':
    case 'S':
        scene.translateSelected(0.0f, 0.0f, TRANSLATE_STEP, lighting);
        break;

    case 'a':
    case 'A':
        scene.translateSelected(-TRANSLATE_STEP, 0.0f, 0.0f, lighting);
        break;
    case 'd':
    case 'D':
        scene.translateSelected(TRANSLATE_STEP, 0.0f, 0.0f, lighting);
        break;

    case 'q':
    case 'Q':
        scene.translateSelected(0.0f, -TRANSLATE_STEP, 0.0f, lighting);
        break;
    case 'e':
    case 'E':
        scene.translateSelected(0.0f, TRANSLATE_STEP, 0.0f, lighting);
        break;

    case 'i':
    case 'I':
        scene.rotateSelected(-ROTATE_STEP, 0.0f, 0.0f);
        break;
    case 'k':
    case 'K':
        scene.rotateSelected(ROTATE_STEP, 0.0f, 0.0f);
        break;

    case 'j':
    case 'J':
        scene.rotateSelected(0.0f, -ROTATE_STEP, 0.0f);
        break;
    case 'l':
    case 'L':
        scene.rotateSelected(0.0f, ROTATE_STEP, 0.0f);
        break;

    case 'u':
    case 'U':
        scene.rotateSelected(0.0f, 0.0f, -ROTATE_STEP);
        break;
    case 'o':
    case 'O':
        scene.rotateSelected(0.0f, 0.0f, ROTATE_STEP);
        break;

    case '+':
    case '=':
        scene.scaleSelected(SCALE_STEP);
        break;
    case '-':
    case '_':
        scene.scaleSelected(-SCALE_STEP);
        break;

    case 'p':
    case 'P':
        camera.toggleProjection();
        break;

    case 'r':
    case 'R':
        camera.reset();
        lighting.reset();
        scene.reset();
        break;

    case 'h':
    case 'H':
        hud.toggleHelp();
        break;

    case 27:
        glutLeaveMainLoop();
        return;

    default:
        break;
    }

    glutPostRedisplay();
}

void specialKeys(int key, int /*x*/, int /*y*/) {
    int modifiers = glutGetModifiers();
    bool shiftPressed = (modifiers & GLUT_ACTIVE_SHIFT) != 0;

    switch (key) {

    case GLUT_KEY_LEFT:
        if (shiftPressed)
            camera.pan(-CAMERA_PAN_STEP, 0.0f);
        else
            camera.rotate(-CAMERA_ROT_STEP, 0.0f);
        break;

    case GLUT_KEY_RIGHT:
        if (shiftPressed)
            camera.pan(CAMERA_PAN_STEP, 0.0f);
        else
            camera.rotate(CAMERA_ROT_STEP, 0.0f);
        break;

    case GLUT_KEY_UP:
        if (shiftPressed)
            camera.pan(0.0f, CAMERA_PAN_STEP);
        else
            camera.rotate(0.0f, CAMERA_ROT_STEP);
        break;

    case GLUT_KEY_DOWN:
        if (shiftPressed)
            camera.pan(0.0f, -CAMERA_PAN_STEP);
        else
            camera.rotate(0.0f, -CAMERA_ROT_STEP);
        break;

    case GLUT_KEY_PAGE_UP:
        camera.zoom(-CAMERA_ZOOM_STEP);
        break;

    case GLUT_KEY_PAGE_DOWN:
        camera.zoom(CAMERA_ZOOM_STEP);
        break;

    case GLUT_KEY_F1:
        lighting.toggleDirectional();
        break;
    case GLUT_KEY_F2:
        lighting.togglePositional();
        break;
    case GLUT_KEY_F3:
        lighting.toggleAmbient();
        break;
    case GLUT_KEY_F4:
        lighting.toggleDiffuse();
        break;
    case GLUT_KEY_F5:
        lighting.toggleSpecular();
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

void initGL() {
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    lighting.init();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 80);

    glutCreateWindow("Galeria de Arte 3D Interativa - Computacao Grafica (FreeGLUT / OpenGL)");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}
