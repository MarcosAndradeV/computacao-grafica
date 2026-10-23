// dependencias:
//      freeglut-devel

#include <GL/glut.h>
#include <cmath>
#include <vector>

struct Projétil {
    float x, y, z;
    float dirX, dirY;
    bool ativo;
};

struct Alvo {
    float x, y, z;
    float escalaBase;
    float rotAngulo;
    float rotVel;
    bool ativo;
};

float posX = 0.0f;
float posY = -3.5f;
float anguloNave = 0.0f;
float escalaNave = 0.8f;

std::vector<Projétil> tiros;
std::vector<Alvo> alvos;

float tempoAnimacao = 0.0f;
int pontos = 0;

void inicializarAlvos() {
    alvos.clear();

    alvos.push_back({-3.0f, 2.5f, 0.0f, 0.6f, 0.0f, 1.5f, true});
    alvos.push_back({-1.0f, 3.0f, 0.0f, 0.7f, 0.0f, -2.0f, true});
    alvos.push_back({1.0f, 3.0f, 0.0f, 0.7f, 0.0f, 2.5f, true});
    alvos.push_back({3.0f, 2.5f, 0.0f, 0.6f, 0.0f, -1.8f, true});
}

void desenharGeometriaNave() {
    glBegin(GL_TRIANGLES);

    glColor3f(0.2f, 0.8f, 1.0f);
    glVertex3f(0.0f, 0.8f, 0.0f);
    glColor3f(0.1f, 0.4f, 0.8f);
    glVertex3f(-0.5f, -0.6f, 0.0f);
    glVertex3f(0.5f, -0.6f, 0.0f);

    glColor3f(0.9f, 0.2f, 0.2f);
    glVertex3f(-0.5f, -0.6f, 0.0f);
    glVertex3f(-0.8f, -0.8f, 0.0f);
    glVertex3f(-0.3f, -0.2f, 0.0f);

    glVertex3f(0.5f, -0.6f, 0.0f);
    glVertex3f(0.8f, -0.8f, 0.0f);
    glVertex3f(0.3f, -0.2f, 0.0f);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    glTranslatef(posX, posY, 0.0f);
    glRotatef(anguloNave, 0.0f, 0.0f, 1.0f);
    glScalef(escalaNave, escalaNave, 1.0f);
    desenharGeometriaNave();
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    for (const auto &tiro : tiros) {
        if (!tiro.ativo)
            continue;

        glPushMatrix();
        glTranslatef(tiro.x, tiro.y, tiro.z);
        glScalef(0.15f, 0.3f, 1.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    for (size_t i = 0; i < alvos.size(); ++i) {
        if (!alvos[i].ativo)
            continue;

        glPushMatrix();
        glTranslatef(alvos[i].x, alvos[i].y, alvos[i].z);

        glRotatef(alvos[i].rotAngulo, 0.0f, 1.0f, 0.0f);
        glRotatef(alvos[i].rotAngulo * 0.5f, 0.0f, 0.0f, 1.0f);

        float fatorPulso = 1.0f + 0.2f * std::sin(tempoAnimacao * 3.0f + (float)i);
        float escFinal = alvos[i].escalaBase * fatorPulso;
        glScalef(escFinal, escFinal, escFinal);

        glColor3f(0.2f, 1.0f, 0.3f);
        glutWireCube(1.0f);
        glPopMatrix();
    }

    glutSwapBuffers();
}

void timer(int val) {
    tempoAnimacao += 0.03f;

    for (auto &tiro : tiros) {
        if (!tiro.ativo)
            continue;
        tiro.x += tiro.dirX * 0.25f;
        tiro.y += tiro.dirY * 0.25f;

        if (tiro.y > 5.5f || tiro.y < -5.5f || tiro.x > 5.5f || tiro.x < -5.5f) {
            tiro.ativo = false;
        }

        for (auto &alvo : alvos) {
            if (!alvo.ativo)
                continue;
            float dx = tiro.x - alvo.x;
            float dy = tiro.y - alvo.y;
            if (std::sqrt(dx * dx + dy * dy) < (alvo.escalaBase + 0.2f)) {
                alvo.ativo = false;
                tiro.ativo = false;
                pontos++;
                break;
            }
        }
    }

    bool todosDestruidos = true;
    for (auto &alvo : alvos) {
        if (alvo.ativo) {
            todosDestruidos = false;
            alvo.rotAngulo += alvo.rotVel;
            if (alvo.rotAngulo > 360.0f)
                alvo.rotAngulo -= 360.0f;
        }
    }

    if (todosDestruidos) {
        inicializarAlvos();
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void tecladoEspecial(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        anguloNave += 5.0f;
        break;
    case GLUT_KEY_RIGHT:
        anguloNave -= 5.0f;
        break;
    case GLUT_KEY_UP:

        posX += 0.2f * -std::sin(anguloNave * 3.14159f / 180.0f);
        posY += 0.2f * std::cos(anguloNave * 3.14159f / 180.0f);
        break;
    case GLUT_KEY_PAGE_UP:
        escalaNave *= 1.1f;
        break;
    case GLUT_KEY_PAGE_DOWN:
        escalaNave *= 0.9f;
        break;
    }
    glutPostRedisplay();
}

void tecladoNormal(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    }
    if (key == ' ') {
        float rad = (anguloNave + 90.0f) * 3.14159f / 180.0f;
        Projétil novoTiro;
        novoTiro.x = posX;
        novoTiro.y = posY;
        novoTiro.z = 0.0f;
        novoTiro.dirX = std::cos(rad);
        novoTiro.dirY = std::sin(rad);
        novoTiro.ativo = true;
        tiros.push_back(novoTiro);
    }
    if (key == 'r' || key == 'R') {
        posX = 0.0f;
        posY = -3.5f;
        anguloNave = 0.0f;
        escalaNave = 0.8f;
        inicializarAlvos();
    }
}

void init() {
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-5.0, 5.0, -5.0, 5.0, -5.0, 5.0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(150, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Jogo");

    init();
    inicializarAlvos();

    glutDisplayFunc(display);
    glutSpecialFunc(tecladoEspecial);
    glutKeyboardFunc(tecladoNormal);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
