// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/fisica/logica.cpp
//
// Descrição:
//   Implementa a dinâmica de voo espacial 3D, entrada do usuário, disparo e
//   colisão de projéteis laser, evolução do sistema de partículas e cálculo
//   da Matriz de Visualização (View Matrix) nos três modos de câmera.
// ============================================================================

#include "fisica/logica.h"
#include "raymath.h"

#include <algorithm>
#include <cstdlib>
#include <string>

// ----------------------------------------------------------------------------
// SISTEMA DE EXPLOSÃO DE PARTÍCULAS 3D
// ----------------------------------------------------------------------------

void GerarExplosao3D(JogoEstado &jogo, Vector3 centro, Color corBase, int quantidade) {
    for (int i = 0; i < quantidade; ++i) {
        Particula3D p;
        p.pos     = centro;
        p.vel     = VetorAleatorio3D(AleatorioFloat(4.0f, 16.0f));
        p.rot     = (Vector3){AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f)};
        p.rotVel  = (Vector3){AleatorioFloat(-200.0f, 200.0f), AleatorioFloat(-200.0f, 200.0f), AleatorioFloat(-200.0f, 200.0f)};
        p.tamanho = AleatorioFloat(0.18f, 0.55f);
        p.cor     = (std::rand() % 3 == 0) ? YELLOW : (std::rand() % 2 == 0 ? ORANGE : corBase);
        p.vida    = AleatorioFloat(0.6f, 1.3f);
        p.vidaMax = p.vida;
        p.ativo   = true;
        jogo.particulas.push_back(p);
    }
}

// ----------------------------------------------------------------------------
// SUBSISTEMAS INTERNOS DE ATUALIZAÇÃO
// ----------------------------------------------------------------------------

// 1. Atualiza a orientação angular (Euler Angles) e translação 3D da nave
static void AtualizarControleVoo(JogoEstado &jogo, float dt, Vector3 &outForward, Vector3 &outUp, Vector3 &outRight) {
    Nave3D &nave = jogo.nave;
    const float sensibilidade = Config::SENSIBILIDADE_VOO;
    const float fatorInvert   = jogo.inverterPitch ? -1.0f : 1.0f;

    // Arfagem (Pitch - Rotação em torno do eixo local X)
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        nave.pitch += sensibilidade * fatorInvert * dt;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        nave.pitch -= sensibilidade * fatorInvert * dt;

    // Clamping de Pitch em [-85°, +85°] para evitar inversão abrupta (Gimbal Lock)
    nave.pitch = Clamp(nave.pitch, -Config::LIMITE_PITCH, Config::LIMITE_PITCH);

    // Guinada (Yaw - Rotação em torno do eixo Y) combinada com Banking (Auto-Roll)
    nave.rollDesejado = 0.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        nave.yaw += sensibilidade * dt;
        nave.rollDesejado = -35.0f;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        nave.yaw -= sensibilidade * dt;
        nave.rollDesejado = 35.0f;
    }

    // Rolagem Manual Adicional (Roll - Eixo Z)
    if (IsKeyDown(KEY_Q))
        nave.rollDesejado += 45.0f;
    if (IsKeyDown(KEY_E))
        nave.rollDesejado -= 45.0f;

    // Interpolação linear (Lerp) para simular inércia rotacional nas asas
    nave.roll = Lerp(nave.roll, nave.rollDesejado, 8.0f * dt);

    // Mantém o ângulo de guinada normalizado no intervalo [0°, 360°)
    if (nave.yaw > 360.0f)
        nave.yaw -= 360.0f;
    if (nave.yaw < 0.0f)
        nave.yaw += 360.0f;

    // Controle de propulsão linear (Cruzeiro, Turbo ou Freio Espacial)
    nave.turbo      = IsKeyDown(KEY_LEFT_SHIFT);
    nave.acelerando = IsKeyDown(KEY_SPACE) || nave.turbo || IsKeyDown(KEY_W);
    nave.freando    = IsKeyDown(KEY_LEFT_CONTROL);

    float velAlvo = Config::VELOCIDADE_CRUZEIRO;
    if (nave.turbo)
        velAlvo = Config::VELOCIDADE_TURBO;
    else if (nave.freando)
        velAlvo = Config::VELOCIDADE_FREIO;

    nave.velocidadeAtual = Lerp(nave.velocidadeAtual, velAlvo, 4.0f * dt);

    // Extrai os vetores diretores da nave após aplicar a matriz de rotação
    CalcularOrientacaoNave(nave, outForward, outUp, outRight);

    // Integração de Euler para translação no espaço 3D: P = P + V * dt
    nave.vel = Vector3Scale(outForward, nave.velocidadeAtual);
    nave.pos = Vector3Add(nave.pos, Vector3Scale(nave.vel, dt));

    // Contenção esférica da arena (impede que a nave saia do volume de combate)
    if (Vector3Length(nave.pos) > Config::RAIO_MUNDO) {
        nave.pos = Vector3Scale(Vector3Normalize(nave.pos), Config::RAIO_MUNDO);
    }
}

// 2. Gerencia disparos laser, propagação linear e colisão esférica com alvos
static void AtualizarCombateEColisoes(JogoEstado &jogo, float dt, const Vector3 &forward, const Vector3 &right) {
    Nave3D &nave = jogo.nave;

    // Disparo duplo a partir da posição mundial das pontas das asas
    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector3 canhaoEsq = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.4f), Vector3Scale(right, -1.85f)));
        Vector3 canhaoDir = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.4f), Vector3Scale(right, 1.85f)));

        jogo.tiros.push_back({canhaoEsq, forward, Config::TEMPO_VIDA_TIRO, true});
        jogo.tiros.push_back({canhaoDir, forward, Config::TEMPO_VIDA_TIRO, true});
        jogo.cameraTrauma += 0.06f;
    }

    // Atualização dos projéteis e teste de interseção Ponto-Esfera (Bounding Sphere)
    for (auto &t : jogo.tiros) {
        if (!t.ativo)
            continue;

        t.pos = Vector3Add(t.pos, Vector3Scale(t.dir, Config::VELOCIDADE_TIRO * dt));
        t.vida -= dt;
        if (t.vida <= 0.0f) {
            t.ativo = false;
            continue;
        }

        for (auto &a : jogo.alvos) {
            if (!a.ativo)
                continue;

            float raioColisao = a.escalaBase * 1.15f;
            if (Vector3Distance(t.pos, a.pos) < raioColisao) {
                a.ativo = false;
                t.ativo = false;
                jogo.pontuacao += 100;
                jogo.cameraTrauma += 0.35f;
                GerarExplosao3D(jogo, a.pos, a.cor, 28);
                break;
            }
        }
    }

    // Limpeza de projéteis expirados ou que já colidiram
    jogo.tiros.erase(
        std::remove_if(jogo.tiros.begin(), jogo.tiros.end(), [](const Projetil3D &t) { return !t.ativo; }),
        jogo.tiros.end()
    );

    // Rotação contínua dos alvos ativos e contagem de sobreviventes na onda
    int alvosVivos = 0;
    for (auto &a : jogo.alvos) {
        if (!a.ativo)
            continue;
        alvosVivos++;
        a.rotAngulo.x += a.rotVel.x * dt;
        a.rotAngulo.y += a.rotVel.y * dt;
        a.rotAngulo.z += a.rotVel.z * dt;
    }

    // Avança para a próxima onda caso todos os alvos tenham sido destruídos
    if (alvosVivos == 0 && !jogo.alvos.empty()) {
        jogo.ondaAtual++;
        jogo.pontuacao += 500;
        InicializarAlvos(jogo);
        jogo.alertaTexto        = "ONDA " + std::to_string(jogo.ondaAtual) + " INICIADA! +500 PTS";
        jogo.alertaTemporizador = 3.5f;
    }

    // Verificação de passagem por Anéis de Checkpoint
    for (auto &anel : jogo.aneis) {
        if (anel.coletado)
            continue;
        if (Vector3Distance(nave.pos, anel.pos) < anel.raio * 1.1f) {
            anel.coletado = true;
            jogo.pontuacao += 200;
            GerarExplosao3D(jogo, anel.pos, SKYBLUE, 20);
            jogo.alertaTexto        = "ANEL ESPACIAL CRUZADO! +200 PTS";
            jogo.alertaTemporizador = 2.0f;
        }
    }
}

// 3. Atualiza a física e o decaimento temporal do sistema de partículas 3D
static void AtualizarParticulas(JogoEstado &jogo, float dt) {
    for (auto &p : jogo.particulas) {
        if (!p.ativo)
            continue;
        p.pos = Vector3Add(p.pos, Vector3Scale(p.vel, dt));
        p.rot = Vector3Add(p.rot, Vector3Scale(p.rotVel, dt));
        p.vel = Vector3Scale(p.vel, 0.96f); // Amortecimento viscoso no espaço
        p.vida -= dt;
        if (p.vida <= 0.0f)
            p.ativo = false;
    }

    jogo.particulas.erase(
        std::remove_if(jogo.particulas.begin(), jogo.particulas.end(), [](const Particula3D &p) { return !p.ativo; }),
        jogo.particulas.end()
    );
}

// 4. Processa atalhos de sistema e atualiza a Matriz de Visualização (Camera3D)
static void AtualizarCameraEAtalhos(JogoEstado &jogo, float dt, const Vector3 &forward, const Vector3 &up) {
    // Alternância entre os 3 modos de câmera (Chase -> Cockpit -> Orbital)
    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_TAB)) {
        switch (jogo.modoCamera) {
        case ModoCamera::Chase:
            jogo.modoCamera = ModoCamera::Cockpit;
            break;
        case ModoCamera::Cockpit:
            jogo.modoCamera = ModoCamera::Orbital;
            break;
        case ModoCamera::Orbital:
            jogo.modoCamera = ModoCamera::Chase;
            break;
        }
    }

    if (IsKeyPressed(KEY_I))
        jogo.inverterPitch = !jogo.inverterPitch;
    if (IsKeyPressed(KEY_H))
        jogo.mostrarHUD_CG = !jogo.mostrarHUD_CG;
    if (IsKeyPressed(KEY_R)) {
        ReiniciarJogoCompleto(jogo);
        return;
    }

    // Cálculo dos vetores LookAt (position, target, up) da Matriz de Visualização
    const Nave3D &nave = jogo.nave;
    Camera3D &camera   = jogo.camera;

    if (jogo.modoCamera == ModoCamera::Chase) {
        // Modo 3ª Pessoa: Câmera posicionada atrás e acima da nave com interpolação suave
        Vector3 posDesejada    = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, -7.5f), Vector3Scale(up, 2.6f)));
        Vector3 targetDesejado = Vector3Add(nave.pos, Vector3Scale(forward, 15.0f));

        camera.position = Vector3Lerp(camera.position, posDesejada, 12.0f * dt);
        camera.target   = Vector3Lerp(camera.target, targetDesejado, 14.0f * dt);
        camera.up       = Vector3Lerp(camera.up, up, 10.0f * dt);
    } else if (jogo.modoCamera == ModoCamera::Cockpit) {
        // Modo 1ª Pessoa: Câmera ancorada diretamente sobre o canopy da nave
        camera.position = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.3f), Vector3Scale(up, 0.35f)));
        camera.target   = Vector3Add(camera.position, Vector3Scale(forward, 30.0f));
        camera.up       = up;
    } else {
        // Modo Visão Tática / Orbital: Câmera elevada acompanhando a nave de cima
        Vector3 posTatica = Vector3Add(nave.pos, (Vector3){0.0f, 32.0f, -24.0f});
        camera.position   = Vector3Lerp(camera.position, posTatica, 8.0f * dt);
        camera.target     = nave.pos;
        camera.up         = (Vector3){0.0f, 1.0f, 0.0f};
    }

    // Efeito de Screen Shake (perturbação estocástica na Matriz de Visualização)
    if (jogo.cameraTrauma > 1.0f)
        jogo.cameraTrauma = 1.0f;
    if (jogo.cameraTrauma > 0.001f) {
        float shakeX = AleatorioFloat(-1.0f, 1.0f) * jogo.cameraTrauma * 0.35f;
        float shakeY = AleatorioFloat(-1.0f, 1.0f) * jogo.cameraTrauma * 0.35f;
        camera.position.x += shakeX;
        camera.position.y += shakeY;
        camera.target.x += shakeX;
        camera.target.y += shakeY;
        jogo.cameraTrauma -= dt * 1.6f;
        if (jogo.cameraTrauma < 0.0f)
            jogo.cameraTrauma = 0.0f;
    }
}

// ----------------------------------------------------------------------------
// FUNÇÃO PRINCIPAL DE ATUALIZAÇÃO DO PASSO DE SIMULAÇÃO
// ----------------------------------------------------------------------------

void AtualizarJogo(JogoEstado &jogo, float dt) {
    jogo.tempoAnimacao += dt * 2.0f;

    if (jogo.alertaTemporizador > 0.0f) {
        jogo.alertaTemporizador -= dt;
    }

    Vector3 forward, up, right;
    AtualizarControleVoo(jogo, dt, forward, up, right);
    AtualizarCombateEColisoes(jogo, dt, forward, right);
    AtualizarParticulas(jogo, dt);
    AtualizarCameraEAtalhos(jogo, dt, forward, up);
}
