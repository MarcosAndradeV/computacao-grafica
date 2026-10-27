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
#include "input/input.h"
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

// 1. Atualiza a orientacao angular (Euler Angles) e translacao 3D da nave
static void AtualizarControleVoo(JogoEstado &jogo, float dt, const ComandosEntrada &cmd, Vector3 &outForward, Vector3 &outUp, Vector3 &outRight) {
    Nave3D &nave = jogo.nave;
    const float sensibilidade = Config::SENSIBILIDADE_VOO;
    const float fatorInvert   = jogo.inverterPitch ? -1.0f : 1.0f;

    // Arfagem (Pitch - Rotacao em torno do eixo local X)
    nave.pitch += cmd.arfagem * sensibilidade * fatorInvert * dt;

    // Clamping de Pitch em [-85 deg, +85 deg] para evitar inversao abrupta (Gimbal Lock)
    nave.pitch = Clamp(nave.pitch, -Config::LIMITE_PITCH, Config::LIMITE_PITCH);

    // Guinada (Yaw - Rotacao em torno do eixo Y) combinada com Banking (Auto-Roll)
    nave.yaw += cmd.guinada * sensibilidade * dt;
    nave.rollDesejado = -cmd.guinada * 35.0f;

    // Rolagem Manual Adicional (Roll - Eixo Z)
    if (std::fabs(cmd.rolagem) > 0.01f) {
        nave.rollDesejado += cmd.rolagem * 45.0f;
    }

    // Interpolacao linear (Lerp) para simular inercia rotacional nas asas
    nave.roll = Lerp(nave.roll, nave.rollDesejado, 8.0f * dt);

    // Mantem o angulo de guinada normalizado no intervalo [0 deg, 360 deg)
    if (nave.yaw > 360.0f)
        nave.yaw -= 360.0f;
    if (nave.yaw < 0.0f)
        nave.yaw += 360.0f;

    // Controle de propulsao linear (Cruzeiro, Turbo ou Freio Espacial)
    nave.turbo      = (cmd.aceleracao > 0.5f);
    nave.freando    = (cmd.aceleracao < -0.5f);
    nave.acelerando = nave.turbo || (cmd.arfagem > 0.1f) || cmd.atirarContinuo;

    float velAlvo = Config::VELOCIDADE_CRUZEIRO;
    if (nave.turbo)
        velAlvo = Config::VELOCIDADE_TURBO;
    else if (nave.freando)
        velAlvo = Config::VELOCIDADE_FREIO;

    nave.velocidadeAtual = Lerp(nave.velocidadeAtual, velAlvo, 4.0f * dt);

    // Extrai os vetores diretores da nave apos aplicar a matriz de rotacao
    CalcularOrientacaoNave(nave, outForward, outUp, outRight);

    // Integracao de Euler para translacao no espaco 3D: P = P + V * dt
    nave.vel = Vector3Scale(outForward, nave.velocidadeAtual);
    nave.pos = Vector3Add(nave.pos, Vector3Scale(nave.vel, dt));

    // Contencao esferica da arena (impede que a nave saia do volume de combate)
    if (Vector3Length(nave.pos) > Config::RAIO_MUNDO) {
        nave.pos = Vector3Scale(Vector3Normalize(nave.pos), Config::RAIO_MUNDO);
    }
}

// 2. Gerencia disparos laser, propagacao linear e colisao esferica com alvos
static void AtualizarCombateEColisoes(JogoEstado &jogo, float dt, const ComandosEntrada &cmd, const Vector3 &forward, const Vector3 &right) {
    Nave3D &nave = jogo.nave;

    // Cadencia automatica de disparo continuo
    static float s_cooldownTiro = 0.0f;
    s_cooldownTiro -= dt;

    bool querDisparar = cmd.atirar || (cmd.atirarContinuo && s_cooldownTiro <= 0.0f);

    // Disparo duplo a partir da posicao mundial das pontas das asas
    if (querDisparar) {
        Vector3 canhaoEsq = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.4f), Vector3Scale(right, -1.85f)));
        Vector3 canhaoDir = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.4f), Vector3Scale(right, 1.85f)));

        jogo.tiros.push_back({canhaoEsq, forward, Config::TEMPO_VIDA_TIRO, true});
        jogo.tiros.push_back({canhaoDir, forward, Config::TEMPO_VIDA_TIRO, true});
        jogo.cameraTrauma += 0.06f;
        s_cooldownTiro = 0.16f;

        // Feedback haptico no Gamepad
        DispararVibracaoGamepad(0.25f, 0.25f, 0.08f);
    }

    // Atualizacao dos projetis e teste de intersecao Ponto-Esfera (Bounding Sphere)
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
                DispararVibracaoGamepad(0.6f, 0.8f, 0.22f);
                break;
            }
        }
    }

    // Limpeza de projetis expirados ou que ja colidiram
    jogo.tiros.erase(
        std::remove_if(jogo.tiros.begin(), jogo.tiros.end(), [](const Projetil3D &t) { return !t.ativo; }),
        jogo.tiros.end()
    );

    // Rotacao continua dos alvos ativos e contagem de sobreviventes na onda
    int alvosVivos = 0;
    for (auto &a : jogo.alvos) {
        if (!a.ativo)
            continue;
        alvosVivos++;
        a.rotAngulo.x += a.rotVel.x * dt;
        a.rotAngulo.y += a.rotVel.y * dt;
        a.rotAngulo.z += a.rotVel.z * dt;
    }

    // Avanca para a proxima onda caso todos os alvos tenham sido destruidos
    if (alvosVivos == 0 && !jogo.alvos.empty()) {
        jogo.ondaAtual++;
        jogo.pontuacao += 500;
        InicializarAlvos(jogo);
        jogo.alertaTexto        = "ONDA " + std::to_string(jogo.ondaAtual) + " INICIADA! +500 PTS";
        jogo.alertaTemporizador = 3.5f;
        DispararVibracaoGamepad(0.5f, 0.5f, 0.35f);
    }

    // Verificacao de passagem por Aneis de Checkpoint
    for (auto &anel : jogo.aneis) {
        if (anel.coletado)
            continue;
        if (Vector3Distance(nave.pos, anel.pos) < anel.raio * 1.1f) {
            anel.coletado = true;
            jogo.pontuacao += 200;
            GerarExplosao3D(jogo, anel.pos, SKYBLUE, 20);
            jogo.alertaTexto        = "ANEL ESPACIAL CRUZADO! +200 PTS";
            jogo.alertaTemporizador = 2.0f;
            DispararVibracaoGamepad(0.4f, 0.4f, 0.15f);
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

// 4. Processa atalhos de sistema e atualiza a Matriz de Visualizacao (Camera3D)
static void AtualizarCameraEAtalhos(JogoEstado &jogo, float dt, const ComandosEntrada &cmd, const Vector3 &forward, const Vector3 &up) {
    // Alternancia entre os 3 modos de camera (Chase -> Cockpit -> Orbital)
    if (cmd.trocarCamera) {
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

    if (cmd.inverterPitch)
        jogo.inverterPitch = !jogo.inverterPitch;
    if (cmd.alternarHUD)
        jogo.mostrarHUD_CG = !jogo.mostrarHUD_CG;
    if (cmd.reiniciarJogo) {
        ReiniciarJogoCompleto(jogo);
        return;
    }

    // Calculo dos vetores LookAt (position, target, up) da Matriz de Visualizacao
    const Nave3D &nave = jogo.nave;
    Camera3D &camera   = jogo.camera;

    if (jogo.modoCamera == ModoCamera::Chase) {
        // Modo 3a Pessoa: Camera posicionada atras e acima da nave com interpolacao suave
        Vector3 posDesejada    = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, -7.5f), Vector3Scale(up, 2.6f)));
        Vector3 targetDesejado = Vector3Add(nave.pos, Vector3Scale(forward, 15.0f));

        camera.position = Vector3Lerp(camera.position, posDesejada, 12.0f * dt);
        camera.target   = Vector3Lerp(camera.target, targetDesejado, 14.0f * dt);
        camera.up       = Vector3Lerp(camera.up, up, 10.0f * dt);
    } else if (jogo.modoCamera == ModoCamera::Cockpit) {
        // Modo 1a Pessoa: Camera ancorada diretamente sobre o canopy da nave
        camera.position = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.3f), Vector3Scale(up, 0.35f)));
        camera.target   = Vector3Add(camera.position, Vector3Scale(forward, 30.0f));
        camera.up       = up;
    } else {
        // Modo Visao Tatica / Orbital: Camera elevada acompanhando a nave de cima
        Vector3 posTatica = Vector3Add(nave.pos, (Vector3){0.0f, 32.0f, -24.0f});
        camera.position   = Vector3Lerp(camera.position, posTatica, 8.0f * dt);
        camera.target     = nave.pos;
        camera.up         = (Vector3){0.0f, 1.0f, 0.0f};
    }

    // Efeito de Screen Shake (perturbacao estocastica na Matriz de Visualizacao)
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
// FUNCAO PRINCIPAL DE ATUALIZACAO DO PASSO DE SIMULACAO
// ----------------------------------------------------------------------------

void AtualizarJogo(JogoEstado &jogo, float dt) {
    jogo.tempoAnimacao += dt * 2.0f;

    if (jogo.alertaTemporizador > 0.0f) {
        jogo.alertaTemporizador -= dt;
    }

    // Leitura unificada de Teclado, Mouse e Gamepad do modulo input
    ComandosEntrada cmd = LerComandosEntrada(jogo);

    Vector3 forward, up, right;
    AtualizarControleVoo(jogo, dt, cmd, forward, up, right);
    AtualizarCombateEColisoes(jogo, dt, cmd, forward, right);
    AtualizarParticulas(jogo, dt);
    AtualizarCameraEAtalhos(jogo, dt, cmd, forward, up);
}
