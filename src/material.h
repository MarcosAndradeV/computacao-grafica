#pragma once
#include <GL/gl.h>

struct Material {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat emission[4];
    GLfloat shininess;
};

namespace Materials {

// Materiais clássicos da computação gráfica baseados em propriedades de reflexão reais

// 1. Ouro (Metal nobre com alta componente especular e difusa dourada)
inline Material Gold() {
    return {
        {0.24725f, 0.1995f, 0.0745f, 1.0f},
        {0.75164f, 0.60648f, 0.22648f, 1.0f},
        {0.628281f, 0.555802f, 0.366065f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        51.2f
    };
}

// 2. Rubi (Gema preciosa avermelhada com forte brilho vítreo)
inline Material Ruby() {
    return {
        {0.1745f, 0.01175f, 0.01175f, 1.0f},
        {0.61424f, 0.04136f, 0.04136f, 1.0f},
        {0.727811f, 0.626959f, 0.626959f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        76.8f
    };
}

// 3. Esmeralda (Gema verde polida com reflexão especular destacada)
inline Material Emerald() {
    return {
        {0.0215f, 0.1745f, 0.0215f, 1.0f},
        {0.07568f, 0.61424f, 0.07568f, 1.0f},
        {0.633f, 0.727811f, 0.633f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        76.8f
    };
}

// 4. Cromo / Prata (Metal prateado polido de altíssima reflexão)
inline Material Chrome() {
    return {
        {0.25f, 0.25f, 0.25f, 1.0f},
        {0.4f, 0.4f, 0.4f, 1.0f},
        {0.774597f, 0.774597f, 0.774597f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        89.6f
    };
}

// Pedestal de exibição (Mármore escuro fosco)
inline Material Pedestal() {
    return {
        {0.12f, 0.12f, 0.14f, 1.0f},
        {0.35f, 0.35f, 0.38f, 1.0f},
        {0.2f, 0.2f, 0.22f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        20.0f
    };
}

// Piso ladrilhado claro
inline Material FloorWhite() {
    return {
        {0.2f, 0.2f, 0.2f, 1.0f},
        {0.8f, 0.8f, 0.82f, 1.0f},
        {0.15f, 0.15f, 0.15f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        10.0f
    };
}

// Piso ladrilhado escuro
inline Material FloorDark() {
    return {
        {0.05f, 0.05f, 0.06f, 1.0f},
        {0.18f, 0.18f, 0.20f, 1.0f},
        {0.1f, 0.1f, 0.1f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        10.0f
    };
}

// Paredes da galeria (tom suave e elegante de museu)
inline Material Wall() {
    return {
        {0.15f, 0.15f, 0.16f, 1.0f},
        {0.65f, 0.65f, 0.68f, 1.0f},
        {0.1f, 0.1f, 0.1f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        5.0f
    };
}

// Rodapés e molduras de madeira nobre
inline Material WoodTrim() {
    return {
        {0.12f, 0.06f, 0.02f, 1.0f},
        {0.35f, 0.18f, 0.08f, 1.0f},
        {0.15f, 0.15f, 0.15f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        15.0f
    };
}

// Material emissivo para a lâmpada indicadora da luz posicional
inline Material LightBulb() {
    return {
        {1.0f, 1.0f, 0.6f, 1.0f},
        {1.0f, 1.0f, 0.8f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.95f, 0.4f, 1.0f},
        128.0f
    };
}

// Aplica as propriedades do material na pipeline fixa do OpenGL
inline void apply(const Material& mat, bool enableAmbient = true, bool enableDiffuse = true, bool enableSpecular = true) {
    GLfloat black[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, enableAmbient ? mat.ambient : black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, enableDiffuse ? mat.diffuse : black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, enableSpecular ? mat.specular : black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat.emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess);
}

} // namespace Materials
