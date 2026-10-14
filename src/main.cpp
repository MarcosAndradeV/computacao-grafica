#include <GL/gl.h>
#include <GL/glut.h>

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutCreateWindow("Test");
    glutDisplayFunc([]() {
        glClearColor(1.0f, 0.5f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
    });
    glutMainLoop();
}
