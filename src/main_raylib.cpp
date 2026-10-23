// dependencias:
//      raylib-devel (Fedora) / libraylib-dev (Debian/Ubuntu)
//      flags/bibliotecas: -lraylib

#include "raylib.h"
#include "rlgl.h"
#include <cmath>
#include <vector>

struct Projetil {
    float x, y;
    float dirX, dirY;
    bool ativo;
};

struct Alvo {
    float x, y;
    float escalaBase;
    float rotAngulo;
    float rotVel;
    bool ativo;
};

void desenharGeometriaNave() {
    rlBegin(RL_TRIANGLES);
        // Corpo principal
        rlColor3f(0.2f, 0.8f, 1.0f);
        rlVertex3f(0.0f,  0.8f, 0.0f);
        rlColor3f(0.1f, 0.4f, 0.8f);
        rlVertex3f(-0.5f, -0.6f, 0.0f);
        rlVertex3f(0.5f, -0.6f, 0.0f);

        // Asa esquerda
        rlColor3f(0.9f, 0.2f, 0.2f);
        rlVertex3f(-0.5f, -0.6f, 0.0f);
        rlVertex3f(-0.8f, -0.8f, 0.0f);
        rlVertex3f(-0.3f, -0.2f, 0.0f);

        // Asa direita
        rlVertex3f(0.5f, -0.6f, 0.0f);
        rlVertex3f(0.8f, -0.8f, 0.0f);
        rlVertex3f(0.3f, -0.2f, 0.0f);
    rlEnd();
}

int main() {
    const int largura = 600;
    const int altura = 600;
    InitWindow(largura, altura, "Transformacoes Geometricas 3D - Raylib");
    SetTargetFPS(60);

    float posX = 0.0f;
    float posY = -3.5f;
    float anguloNave = 0.0f;
    float escalaNave = 0.8f;
    float tempoAnimacao = 0.0f;

    std::vector<Projetil> tiros;
    std::vector<Alvo> alvos = {
        { -3.0f,  2.5f, 0.6f, 0.0f,  1.5f, true },
        { -1.0f,  3.0f, 0.7f, 0.0f, -2.0f, true },
        {  1.0f,  3.0f, 0.7f, 0.0f,  2.5f, true },
        {  3.0f,  2.5f, 0.6f, 0.0f, -1.8f, true }
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        tempoAnimacao += dt * 2.0f;

        // Entradas do Jogador
        if (IsKeyDown(KEY_LEFT))  anguloNave += 180.0f * dt;
        if (IsKeyDown(KEY_RIGHT)) anguloNave -= 180.0f * dt;
        if (IsKeyDown(KEY_UP)) {
            float rad = anguloNave * (3.14159f / 180.0f);
            posX += -std::sin(rad) * 4.0f * dt;
            posY +=  std::cos(rad) * 4.0f * dt;
        }

        // Disparo
        if (IsKeyPressed(KEY_SPACE)) {
            float rad = (anguloNave + 90.0f) * (3.14159f / 180.0f);
            tiros.push_back({ posX, posY, std::cos(rad), std::sin(rad), true });
        }

        // Atualização dos Tiros e Colisões
        for (auto& tiro : tiros) {
            if (!tiro.ativo) continue;
            tiro.x += tiro.dirX * 8.0f * dt;
            tiro.y += tiro.dirY * 8.0f * dt;

            if (tiro.x > 5.5f || tiro.x < -5.5f || tiro.y > 5.5f || tiro.y < -5.5f) {
                tiro.ativo = false;
            }

            for (auto& alvo : alvos) {
                if (!alvo.ativo) continue;
                float dx = tiro.x - alvo.x;
                float dy = tiro.y - alvo.y;
                if (std::sqrt(dx * dx + dy * dy) < (alvo.escalaBase + 0.2f)) {
                    alvo.ativo = false;
                    tiro.ativo = false;
                    break;
                }
            }
        }

        // Rotação dos Alvos
        for (auto& alvo : alvos) {
            if (alvo.ativo) {
                alvo.rotAngulo += alvo.rotVel * 60.0f * dt;
            }
        }

        // Renderização
        BeginDrawing();
        ClearBackground({ 12, 12, 25, 255 });

        // Configuração da Projeção Ortográfica (-5 a 5)
        rlMatrixMode(RL_PROJECTION);
        rlLoadIdentity();
        rlOrtho(-5.0, 5.0, -5.0, 5.0, -5.0, 5.0);

        rlMatrixMode(RL_MODELVIEW);
        rlLoadIdentity();

        // 1. Nave (T * R * S)
        rlPushMatrix();
            rlTranslatef(posX, posY, 0.0f);
            rlRotatef(anguloNave, 0.0f, 0.0f, 1.0f);
            rlScalef(escalaNave, escalaNave, 1.0f);
            desenharGeometriaNave();
        rlPopMatrix();

        // 2. Tiros
        rlColor3f(1.0f, 1.0f, 0.0f);
        for (const auto& tiro : tiros) {
            if (!tiro.ativo) continue;
            rlPushMatrix();
                rlTranslatef(tiro.x, tiro.y, 0.0f);
                rlScalef(0.15f, 0.3f, 1.0f);
                DrawCubeWires({0, 0, 0}, 1.0f, 1.0f, 1.0f, YELLOW);
            rlPopMatrix();
        }

        // 3. Alvos (T * R * S com pulsação)
        for (size_t i = 0; i < alvos.size(); ++i) {
            if (!alvos[i].ativo) continue;
            rlPushMatrix();
                rlTranslatef(alvos[i].x, alvos[i].y, 0.0f);
                rlRotatef(alvos[i].rotAngulo, 0.0f, 1.0f, 0.0f);
                rlRotatef(alvos[i].rotAngulo * 0.5f, 0.0f, 0.0f, 1.0f);

                float fatorPulso = 1.0f + 0.2f * std::sin(tempoAnimacao + (float)i);
                float esc = alvos[i].escalaBase * fatorPulso;
                rlScalef(esc, esc, esc);

                DrawCubeWires({0, 0, 0}, 1.0f, 1.0f, 1.0f, GREEN);
            rlPopMatrix();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
