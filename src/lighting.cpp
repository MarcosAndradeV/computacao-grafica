#include "lighting.h"
#include <GL/freeglut.h>
#include <cmath>

LightingManager::LightingManager() { reset(); }

void LightingManager::reset() {
    directionalEnabled = true;
    positionalEnabled = true;
    ambientEnabled = true;
    diffuseEnabled = true;
    specularEnabled = true;

    for (int i = 0; i < 4; ++i) {
        positionalPos[i] = defaultPositionalPos[i];
        directionalDir[i] = defaultDirectionalDir[i];
    }
}

void LightingManager::init() {
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01f);
}

void LightingManager::toggleDirectional() { directionalEnabled = !directionalEnabled; }
void LightingManager::togglePositional() { positionalEnabled = !positionalEnabled; }
void LightingManager::toggleAmbient() { ambientEnabled = !ambientEnabled; }
void LightingManager::toggleDiffuse() { diffuseEnabled = !diffuseEnabled; }
void LightingManager::toggleSpecular() { specularEnabled = !specularEnabled; }

void LightingManager::translatePositionalLight(float dx, float dy, float dz) {
    positionalPos[0] += dx;
    positionalPos[1] += dy;
    positionalPos[2] += dz;

    if (positionalPos[1] < 0.3f)
        positionalPos[1] = 0.3f;
    if (positionalPos[1] > 6.0f)
        positionalPos[1] = 6.0f;
}

void LightingManager::applyLighting() {

    GLfloat globalAmbient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat noAmbient[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientEnabled ? globalAmbient : noAmbient);

    if (directionalEnabled) {
        glEnable(GL_LIGHT0);

        GLfloat dirAmbient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        GLfloat dirDiffuse[4] = {0.7f, 0.7f, 0.65f, 1.0f};
        GLfloat dirSpecular[4] = {0.6f, 0.6f, 0.6f, 1.0f};
        GLfloat black[4] = {0.0f, 0.0f, 0.0f, 1.0f};

        glLightfv(GL_LIGHT0, GL_POSITION, directionalDir);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientEnabled ? dirAmbient : black);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseEnabled ? dirDiffuse : black);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specularEnabled ? dirSpecular : black);
    } else {
        glDisable(GL_LIGHT0);
    }

    if (positionalEnabled) {
        glEnable(GL_LIGHT1);

        GLfloat posAmbient[4] = {0.12f, 0.12f, 0.08f, 1.0f};
        GLfloat posDiffuse[4] = {1.0f, 0.95f, 0.8f, 1.0f};
        GLfloat posSpecular[4] = {1.0f, 1.0f, 0.9f, 1.0f};
        GLfloat black[4] = {0.0f, 0.0f, 0.0f, 1.0f};

        glLightfv(GL_LIGHT1, GL_POSITION, positionalPos);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambientEnabled ? posAmbient : black);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseEnabled ? posDiffuse : black);
        glLightfv(GL_LIGHT1, GL_SPECULAR, specularEnabled ? posSpecular : black);
    } else {
        glDisable(GL_LIGHT1);
    }
}

void LightingManager::renderLightMarker(bool isSelected) {
    if (!positionalEnabled)
        return;

    glPushMatrix();
    glTranslatef(positionalPos[0], positionalPos[1], positionalPos[2]);

    GLfloat emissionColor[4] = {1.0f, 0.95f, 0.4f, 1.0f};
    GLfloat noEmission[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissionColor);
    glutSolidSphere(0.22, 24, 24);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);

    if (isSelected) {
        glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.85f, 0.0f);
        glLineWidth(2.0f);

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 36; ++i) {
            float rad = 2.0f * 3.1415926f * float(i) / 36.0f;
            glVertex3f(0.38f * std::cos(rad), 0.0f, 0.38f * std::sin(rad));
        }
        glEnd();

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 36; ++i) {
            float rad = 2.0f * 3.1415926f * float(i) / 36.0f;
            glVertex3f(0.38f * std::cos(rad), 0.38f * std::sin(rad), 0.0f);
        }
        glEnd();

        glPopAttrib();
    }

    glPopMatrix();
}
