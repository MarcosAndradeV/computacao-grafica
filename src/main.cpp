#include <GL/freeglut.h>
#include "camera.h"
#include "lighting.h"
#include "scene.h"
#include "hud.h"

namespace {
    int windowWidth = 1280;
    int windowHeight = 720;

    Camera camera;
    LightingManager lighting;
    Scene scene;
    HUD hud;

    const float TRANSLATE_STEP = 0.25f;
    const float ROTATE_STEP    = 5.0f;
    const float SCALE_STEP     = 0.08f;
    const float CAMERA_ROT_STEP= 3.0f;
    const float CAMERA_ZOOM_STEP= 0.5f;
    const float CAMERA_PAN_STEP = 0.3f;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Configurar Projeção da Câmera (Perspectiva ou Ortográfica)
    camera.applyProjection(windowWidth, windowHeight);

    // 2. Configurar Matriz de Visualização (gluLookAt)
    camera.applyView();

    // 3. Aplicar Iluminação no Espaço de Mundo (após View, antes da geometria)
    lighting.applyLighting();

    // 4. Renderizar a Galeria de Arte 3D (Sala, Pedestais, Obras e Lâmpada Móvel)
    scene.render(lighting);

    // 5. Renderizar o HUD 2D com informações e guia de teclas
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
        // Seleção de Objetos e Luz Posicional
        case '1': scene.selectEntity(1); break;
        case '2': scene.selectEntity(2); break;
        case '3': scene.selectEntity(3); break;
        case '4': scene.selectEntity(4); break;
        case '0': scene.selectEntity(0); break; // Seleciona Luz Posicional

        // Translação no Eixo Z (W / S)
        case 'w': case 'W': scene.translateSelected(0.0f, 0.0f, -TRANSLATE_STEP, lighting); break;
        case 's': case 'S': scene.translateSelected(0.0f, 0.0f,  TRANSLATE_STEP, lighting); break;

        // Translação no Eixo X (A / D)
        case 'a': case 'A': scene.translateSelected(-TRANSLATE_STEP, 0.0f, 0.0f, lighting); break;
        case 'd': case 'D': scene.translateSelected( TRANSLATE_STEP, 0.0f, 0.0f, lighting); break;

        // Translação no Eixo Y (Q / E)
        case 'q': case 'Q': scene.translateSelected(0.0f, -TRANSLATE_STEP, 0.0f, lighting); break;
        case 'e': case 'E': scene.translateSelected(0.0f,  TRANSLATE_STEP, 0.0f, lighting); break;

        // Rotação no Eixo X (I / K)
        case 'i': case 'I': scene.rotateSelected(-ROTATE_STEP, 0.0f, 0.0f); break;
        case 'k': case 'K': scene.rotateSelected( ROTATE_STEP, 0.0f, 0.0f); break;

        // Rotação no Eixo Y (J / L)
        case 'j': case 'J': scene.rotateSelected(0.0f, -ROTATE_STEP, 0.0f); break;
        case 'l': case 'L': scene.rotateSelected(0.0f,  ROTATE_STEP, 0.0f); break;

        // Rotação no Eixo Z (U / O)
        case 'u': case 'U': scene.rotateSelected(0.0f, 0.0f, -ROTATE_STEP); break;
        case 'o': case 'O': scene.rotateSelected(0.0f, 0.0f,  ROTATE_STEP); break;

        // Escala (+ / -)
        case '+': case '=': scene.scaleSelected( SCALE_STEP); break;
        case '-': case '_': scene.scaleSelected(-SCALE_STEP); break;

        // Alternar Projeção Perspectiva / Ortográfica (P)
        case 'p': case 'P': camera.toggleProjection(); break;

        // Restaurar Configurações Iniciais (R)
        case 'r': case 'R':
            camera.reset();
            lighting.reset();
            scene.reset();
            break;

        // Alternar exibição do guia de teclas do HUD (H)
        case 'h': case 'H': hud.toggleHelp(); break;

        // Fechar Aplicação (Esc)
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
        // Câmera: Rotacionar ou Deslocar Ponto de Observação (Shift + Setas)
        case GLUT_KEY_LEFT:
            if (shiftPressed) camera.pan(-CAMERA_PAN_STEP, 0.0f);
            else camera.rotate(-CAMERA_ROT_STEP, 0.0f);
            break;

        case GLUT_KEY_RIGHT:
            if (shiftPressed) camera.pan(CAMERA_PAN_STEP, 0.0f);
            else camera.rotate(CAMERA_ROT_STEP, 0.0f);
            break;

        case GLUT_KEY_UP:
            if (shiftPressed) camera.pan(0.0f, CAMERA_PAN_STEP);
            else camera.rotate(0.0f, CAMERA_ROT_STEP);
            break;

        case GLUT_KEY_DOWN:
            if (shiftPressed) camera.pan(0.0f, -CAMERA_PAN_STEP);
            else camera.rotate(0.0f, -CAMERA_ROT_STEP);
            break;

        // Aproximar e Afastar Câmera - Zoom (Page Up / Page Down)
        case GLUT_KEY_PAGE_UP:
            camera.zoom(-CAMERA_ZOOM_STEP);
            break;

        case GLUT_KEY_PAGE_DOWN:
            camera.zoom(CAMERA_ZOOM_STEP);
            break;

        // Controle das Fontes de Iluminação e Componentes
        case GLUT_KEY_F1: lighting.toggleDirectional(); break;
        case GLUT_KEY_F2: lighting.togglePositional();  break;
        case GLUT_KEY_F3: lighting.toggleAmbient();     break;
        case GLUT_KEY_F4: lighting.toggleDiffuse();     break;
        case GLUT_KEY_F5: lighting.toggleSpecular();    break;

        default:
            break;
    }

    glutPostRedisplay();
}

void initGL() {
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Inicializar subsistema de iluminação OpenGL
    lighting.init();
}

int main(int argc, char** argv) {
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
