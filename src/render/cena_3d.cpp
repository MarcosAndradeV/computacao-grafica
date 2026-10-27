// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/cena_3d.cpp
//
// Descrição:
//   Passe de renderização do mundo 3D: monta a cena combinando objetos
//   estáticos (estrelas, grade, cúpula, anéis) e dinâmicos (alvos, lasers,
//   nave, partículas), cada um isolado na pilha de matrizes.
// ============================================================================

#include "render/cena_3d.h"
#include "render/modelos_3d.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>

// Cúpula que delimita a arena. Quase invisível no centro; acende em azul ao se
// aproximar da borda e fica vermelha perto do limite.
static void DesenharCupulaFronteira(const JogoEstado &jogo) {
    float inicio = Config::BORDA_INICIO_BRILHO;
    float proximidade = Clamp((FracaoRaioMundo(jogo.nave) - inicio) / (1.0f - inicio), 0.0f, 1.0f);

    unsigned char alpha = (unsigned char)(25 + (int)(proximidade * 155));
    Color cor = (proximidade > 0.75f) ? Color{255, 60, 60, alpha} : Color{0, 160, 255, alpha};

    // 48 subdivisões: com menos, a esfera aparece como uma rede poligonal grosseira
    DrawSphereWires(Vector3{0, 0, 0}, Config::RAIO_MUNDO, 48, 48, cor);
}

void RenderizarMundo3D(const JogoEstado &jogo) {
    // BeginMode3D carrega a projeção perspectiva e a matriz de visualização (LookAt).
    // A câmera de desenho inclui o tremor do quadro.
    BeginMode3D(ObterCameraRender(jogo));
    rlDisableBackfaceCulling();  // A nave é uma malha aberta: precisa mostrar as duas faces

    // 1. Campo estelar com cintilação senoidal
    for (const auto &est : jogo.estrelas) {
        float pulso = 0.8f + 0.2f * std::sin(jogo.faseAnimacao * 2.0f + est.faseBrilho);
        float lado  = est.tamanho * pulso;
        DrawCube(est.pos, lado, lado, lado, est.cor);
    }

    // 2. Grade de referência (plano Y = -80) para dar noção de profundidade
    rlPushMatrix();
    rlTranslatef(0.0f, -80.0f, 0.0f);
    DrawGrid(140, 4.0f);
    rlPopMatrix();

    // 3. Cúpula da arena
    DesenharCupulaFronteira(jogo);

    // 4. Anéis de checkpoint
    // O cilindro do raylib tem o eixo ao longo de Y. Para ficar em pé, como um
    // portão, giramos 90° em Z (eixo Y -> -X) e depois `rotY` em Y alinha esse
    // eixo à direção radial. Matriz resultante: T · Ry(rotY) · Rz(90°).
    for (const auto &anel : jogo.aneis) {
        rlPushMatrix();
        rlTranslatef(anel.pos.x, anel.pos.y, anel.pos.z);
        rlRotatef(anel.rotY, 0.0f, 1.0f, 0.0f);
        rlRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        Color cAnel = anel.coletado ? DARKGRAY : Color{0, 240, 255, 220};
        DrawCylinderWires(Vector3{0, 0, 0}, anel.raio, anel.raio, 0.35f, 24, cAnel);
        DrawCylinderWires(Vector3{0, 0, 0}, anel.raio * 0.94f, anel.raio * 0.94f, 0.15f, 24, ColorAlpha(cAnel, 0.4f));
        rlPopMatrix();
    }

    // 5. Alvos (transformação composta T·R·S em modelos_3d.cpp)
    for (size_t i = 0; i < jogo.alvos.size(); ++i) {
        if (jogo.alvos[i].ativo) {
            DesenharAlvo3D(jogo.alvos[i], jogo.faseAnimacao, (int)i);
        }
    }

    // 6. Lasers: dois cilindros concêntricos simulam o brilho (glow)
    for (const auto &t : jogo.tiros) {
        if (!t.ativo)
            continue;
        Vector3 fim = Vector3Add(t.pos, Vector3Scale(t.dir, 1.8f));
        DrawCylinderEx(t.pos, fim, 0.08f, 0.08f, 6, YELLOW);
        DrawCylinderEx(t.pos, fim, 0.20f, 0.20f, 6, ColorAlpha(ORANGE, 0.45f));
    }

    // 7. Nave (oculta na câmera de cockpit, que fica dentro dela)
    if (jogo.modoCamera != ModoCamera::Cockpit) {
        rlPushMatrix();

        // M = T · Ry(yaw) · Rx(-pitch) · Rz(roll) · S
        rlTranslatef(jogo.nave.pos.x, jogo.nave.pos.y, jogo.nave.pos.z);
        rlRotatef(jogo.nave.yaw, 0.0f, 1.0f, 0.0f);
        rlRotatef(-jogo.nave.pitch, 1.0f, 0.0f, 0.0f);
        rlRotatef(jogo.nave.roll, 0.0f, 0.0f, 1.0f);
        rlScalef(jogo.nave.escala, jogo.nave.escala, jogo.nave.escala);

        DesenharGeometriaNave3D(jogo.faseAnimacao, jogo.nave.acelerando, jogo.nave.turbo);
        rlPopMatrix();
    }

    // 8. Partículas de explosão, com fade-out proporcional à vida restante
    for (const auto &p : jogo.particulas) {
        if (!p.ativo)
            continue;
        rlPushMatrix();
        rlTranslatef(p.pos.x, p.pos.y, p.pos.z);
        rlRotatef(p.rot.x, 1.0f, 0.0f, 0.0f);
        rlRotatef(p.rot.y, 0.0f, 1.0f, 0.0f);
        rlRotatef(p.rot.z, 0.0f, 0.0f, 1.0f);

        Color cor = ColorAlpha(p.cor, p.vida / p.vidaMax);
        DrawCubeWires(Vector3{0, 0, 0}, p.tamanho, p.tamanho, p.tamanho, cor);
        rlPopMatrix();
    }

    rlEnableBackfaceCulling();  // Restaura o estado padrão do OpenGL
    EndMode3D();
}
