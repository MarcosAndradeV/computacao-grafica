// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/cena_3d.cpp
//
// Descrição:
//   Implementa o passe de renderização do Mundo 3D. Demonstra a montagem da
//   cena tridimensional combinando objetos estáticos, objetos dinâmicos e o
//   pipeline de transformação T * Ry * Rx * Rz * S da nave do jogador.
// ============================================================================

#include "render/cena_3d.h"
#include "render/modelos_3d.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>

void RenderizarMundo3D(const JogoEstado &jogo) {
    // BeginMode3D carrega a Matriz de Projeção Perspectiva e a Matriz de Visualização (Camera3D)
    BeginMode3D(jogo.camera);
    rlDisableBackfaceCulling(); // Exibe ambas as faces dos triângulos da nave durante manobras

    // 1. Campo Estelar 3D com cintilação senoidal
    for (const auto &est : jogo.estrelas) {
        float pulso = 0.8f + 0.2f * std::sin(jogo.tempoAnimacao * 2.0f + est.faseBrilho);
        DrawCube(est.pos, est.tamanho * pulso, est.tamanho * pulso, est.tamanho * pulso, est.cor);
    }

    // 2. Grade de Referência Espacial Holográfica (Transladada para o plano Y = -16.0)
    rlPushMatrix();
    rlTranslatef(0.0f, -16.0f, 0.0f);
    DrawGrid(50, 2.5f);
    rlPopMatrix();

    // Esfera Wireframe demarcando a fronteira do volume de combate
    DrawSphereWires((Vector3){0, 0, 0}, Config::RAIO_MUNDO, 16, 16, (Color){20, 60, 100, 70});

    // 3. Anéis de Checkpoint Espaciais
    for (const auto &anel : jogo.aneis) {
        rlPushMatrix();
        rlTranslatef(anel.pos.x, anel.pos.y, anel.pos.z);
        rlRotatef(anel.rotY, 0.0f, 1.0f, 0.0f);
        Color cAnel = anel.coletado ? DARKGRAY : (Color){0, 240, 255, 220};
        DrawCylinderWires((Vector3){0, 0, 0}, anel.raio, anel.raio, 0.35f, 24, cAnel);
        DrawCylinderWires((Vector3){0, 0, 0}, anel.raio * 0.94f, anel.raio * 0.94f, 0.15f, 24, ColorAlpha(cAnel, 0.4f));
        rlPopMatrix();
    }

    // 4. Alvos Tridimensionais (Transformações Compostas T * R * S em modelos_3d.cpp)
    for (size_t i = 0; i < jogo.alvos.size(); ++i) {
        if (jogo.alvos[i].ativo) {
            DesenharAlvo3D(jogo.alvos[i], jogo.tempoAnimacao, (int)i);
        }
    }

    // 5. Feixes de Laser 3D (Cilindros concêntricos para efeito de brilho/glow)
    for (const auto &t : jogo.tiros) {
        if (!t.ativo)
            continue;
        Vector3 fim = Vector3Add(t.pos, Vector3Scale(t.dir, 1.8f));
        DrawCylinderEx(t.pos, fim, 0.08f, 0.08f, 6, YELLOW);
        DrawCylinderEx(t.pos, fim, 0.20f, 0.20f, 6, ColorAlpha(ORANGE, 0.45f));
    }

    // 6. Nave do Jogador (Oculta apenas na câmera em 1ª Pessoa / Cockpit)
    if (jogo.modoCamera != ModoCamera::Cockpit) {
        rlPushMatrix();

        // Pipeline de Transformação Geométrica da Nave: M = T * Ry * Rx * Rz * S
        rlTranslatef(jogo.nave.pos.x, jogo.nave.pos.y, jogo.nave.pos.z);
        rlRotatef(jogo.nave.yaw, 0.0f, 1.0f, 0.0f);
        rlRotatef(-jogo.nave.pitch, 1.0f, 0.0f, 0.0f);
        rlRotatef(jogo.nave.roll, 0.0f, 0.0f, 1.0f);
        rlScalef(jogo.nave.escala, jogo.nave.escala, jogo.nave.escala);

        DesenharGeometriaNave3D(jogo.tempoAnimacao, jogo.nave.acelerando, jogo.nave.turbo);
        rlPopMatrix();
    }

    // 7. Sistema de Partículas Volumétricas 3D
    for (const auto &p : jogo.particulas) {
        if (!p.ativo)
            continue;
        rlPushMatrix();
        rlTranslatef(p.pos.x, p.pos.y, p.pos.z);
        rlRotatef(p.rot.x, 1.0f, 0.0f, 0.0f);
        rlRotatef(p.rot.y, 0.0f, 1.0f, 0.0f);
        rlRotatef(p.rot.z, 0.0f, 0.0f, 1.0f);

        float alpha = p.vida / p.vidaMax;
        Color cor   = ColorAlpha(p.cor, alpha);
        DrawCubeWires((Vector3){0, 0, 0}, p.tamanho, p.tamanho, p.tamanho, cor);
        rlPopMatrix();
    }

    EndMode3D();
}
