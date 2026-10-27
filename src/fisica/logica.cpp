// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/fisica/logica.cpp
//
// Descrição:
//   Dinâmica de voo 3D, disparo e colisão dos lasers, progressão de ondas,
//   anéis de checkpoint, partículas e cálculo da Matriz de Visualização nos
//   três modos de câmera. A leitura de teclado/mouse/gamepad fica em
//   input/ e chega aqui já normalizada em `ComandosEntrada`.
// ============================================================================

#include "fisica/logica.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace {
    // Base ortonormal da nave no quadro atual
    struct Orientacao {
        Vector3 frente;
        Vector3 cima;
        Vector3 direita;
    };
}

// ----------------------------------------------------------------------------
// PARTÍCULAS DE EXPLOSÃO
// ----------------------------------------------------------------------------

void GerarExplosao3D(JogoEstado &jogo, Vector3 centro, Color corBase, int quantidade) {
    for (int i = 0; i < quantidade; ++i) {
        Particula3D p;
        p.pos     = centro;
        p.vel     = VetorAleatorio3D(AleatorioFloat(4.0f, 16.0f));
        p.rot     = Vector3{AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f)};
        p.rotVel  = Vector3{AleatorioFloat(-200.0f, 200.0f), AleatorioFloat(-200.0f, 200.0f), AleatorioFloat(-200.0f, 200.0f)};
        p.tamanho = AleatorioFloat(0.18f, 0.55f);

        // Um terço amarelo, um terço laranja, um terço na cor do objeto destruído
        switch (std::rand() % 3) {
        case 0:  p.cor = YELLOW;  break;
        case 1:  p.cor = ORANGE;  break;
        default: p.cor = corBase; break;
        }

        p.vida    = AleatorioFloat(0.6f, 1.3f);
        p.vidaMax = p.vida;
        p.ativo   = true;
        jogo.particulas.push_back(p);
    }
}

// ----------------------------------------------------------------------------
// SUBSISTEMAS DE ATUALIZAÇÃO (chamados em ordem por AtualizarJogo)
// ----------------------------------------------------------------------------

// 1. Voo: ângulos de Euler, banking, propulsão e translação da nave
static Orientacao AtualizarControleVoo(JogoEstado &jogo, const ComandosEntrada &cmd, float dt) {
    Nave3D &nave = jogo.nave;
    const float fatorInvert = jogo.inverterPitch ? -1.0f : 1.0f;

    // Arfagem (pitch, eixo X local), limitada para evitar gimbal lock
    nave.pitch += cmd.arfagem * Config::SENSIBILIDADE_VOO * fatorInvert * dt;
    nave.pitch  = Clamp(nave.pitch, -Config::LIMITE_PITCH, Config::LIMITE_PITCH);

    // Guinada (yaw, eixo Y) com inclinação automática das asas (banking)
    nave.yaw += cmd.guinada * Config::SENSIBILIDADE_VOO * dt;
    nave.rollDesejado = -cmd.guinada * Config::BANKING_GUINADA + cmd.rolagem * Config::ROLL_MANUAL;

    // O roll persegue o alvo com suavização exponencial: simula a inércia das asas
    nave.roll = Lerp(nave.roll, nave.rollDesejado, FatorSuavizacao(Config::SUAVIZ_ROLL, dt));

    // Mantém o yaw em [0, 360)
    nave.yaw = std::fmod(nave.yaw, 360.0f);
    if (nave.yaw < 0.0f)
        nave.yaw += 360.0f;

    // Propulsão: cruzeiro, turbo ou freio
    nave.turbo      = cmd.aceleracao > 0.5f;
    nave.freando    = cmd.aceleracao < -0.5f;
    nave.acelerando = nave.turbo;

    float velAlvo = Config::VELOCIDADE_CRUZEIRO;
    if (nave.turbo)
        velAlvo = Config::VELOCIDADE_TURBO;
    else if (nave.freando)
        velAlvo = Config::VELOCIDADE_FREIO;

    nave.velocidadeAtual = Lerp(nave.velocidadeAtual, velAlvo, FatorSuavizacao(Config::SUAVIZ_VELOCIDADE, dt));

    // Base ortonormal e integração de Euler: P = P + V·dt
    Orientacao o;
    CalcularOrientacaoNave(nave, o.frente, o.cima, o.direita);
    nave.vel = Vector3Scale(o.frente, nave.velocidadeAtual);
    nave.pos = Vector3Add(nave.pos, Vector3Scale(nave.vel, dt));

    // Contenção esférica: a nave não sai da arena
    if (Vector3Length(nave.pos) > Config::RAIO_MUNDO) {
        nave.pos = Vector3Scale(Vector3Normalize(nave.pos), Config::RAIO_MUNDO);
    }

    return o;
}

// 2. Disparo: dois lasers saem das pontas das asas e convergem no ponto de mira
static void AtualizarDisparo(JogoEstado &jogo, const ComandosEntrada &cmd, float dt, const Orientacao &o) {
    jogo.cooldownTiro -= dt;

    bool querDisparar = cmd.atirar || (cmd.atirarContinuo && jogo.cooldownTiro <= 0.0f);
    if (!querDisparar)
        return;

    const Nave3D &nave = jogo.nave;
    Vector3 base = Vector3Add(nave.pos, Vector3Scale(o.frente, Config::CANHAO_AVANCO));

    Vector3 canhaoEsq = Vector3Add(base, Vector3Scale(o.direita, -Config::CANHAO_AFASTAMENTO));
    Vector3 canhaoDir = Vector3Add(base, Vector3Scale(o.direita,  Config::CANHAO_AFASTAMENTO));

    // Cada laser aponta para o ponto focal comum (o mesmo onde o HUD desenha a mira)
    Vector3 pontoFocal = Vector3Add(nave.pos, Vector3Scale(o.frente, Config::DISTANCIA_CONVERGENCIA));
    Vector3 dirEsq = Vector3Normalize(Vector3Subtract(pontoFocal, canhaoEsq));
    Vector3 dirDir = Vector3Normalize(Vector3Subtract(pontoFocal, canhaoDir));

    jogo.tiros.push_back({canhaoEsq, dirEsq, Config::TEMPO_VIDA_TIRO, true});
    jogo.tiros.push_back({canhaoDir, dirDir, Config::TEMPO_VIDA_TIRO, true});

    jogo.cameraTrauma += Config::TREMOR_POR_TIRO;
    jogo.cooldownTiro  = Config::CADENCIA_TIRO;
    DispararVibracaoGamepad(0.25f, 0.25f, 0.08f);
}

// 3. Projéteis: avanço e colisão segmento × esfera contra os alvos
static void AtualizarProjeteis(JogoEstado &jogo, float dt) {
    for (auto &t : jogo.tiros) {
        if (!t.ativo)
            continue;

        Vector3 posAnterior = t.pos;
        t.pos = Vector3Add(t.pos, Vector3Scale(t.dir, Config::VELOCIDADE_TIRO * dt));
        t.vida -= dt;
        if (t.vida <= 0.0f) {
            t.ativo = false;
            continue;
        }

        // Testa o trajeto percorrido no quadro (não só o ponto final), para que
        // um tiro rápido não atravesse o alvo entre dois quadros
        for (auto &a : jogo.alvos) {
            if (!a.ativo)
                continue;

            if (SegmentoInterceptaEsfera(posAnterior, t.pos, a.pos, a.escalaBase * Config::FATOR_COLISAO_ALVO)) {
                a.ativo = false;
                t.ativo = false;
                jogo.pontuacao    += Config::PONTOS_ALVO;
                jogo.cameraTrauma += Config::TREMOR_POR_ALVO;
                GerarExplosao3D(jogo, a.pos, a.cor, 28);
                DispararVibracaoGamepad(0.6f, 0.8f, 0.22f);
                break;
            }
        }
    }

    jogo.tiros.erase(
        std::remove_if(jogo.tiros.begin(), jogo.tiros.end(), [](const Projetil3D &t) { return !t.ativo; }),
        jogo.tiros.end()
    );
}

// 4. Alvos: rotação contínua e avanço de onda quando todos foram destruídos
static void AtualizarAlvos(JogoEstado &jogo, float dt) {
    int vivos = 0;
    for (auto &a : jogo.alvos) {
        if (!a.ativo)
            continue;
        vivos++;
        a.rotAngulo = Vector3Add(a.rotAngulo, Vector3Scale(a.rotVel, dt));
    }

    if (vivos == 0 && !jogo.alvos.empty()) {
        jogo.ondaAtual++;
        jogo.pontuacao += Config::PONTOS_ONDA;
        InicializarAlvos(jogo);
        jogo.alertaTexto        = "ONDA " + std::to_string(jogo.ondaAtual) + " INICIADA! +" + std::to_string(Config::PONTOS_ONDA) + " PTS";
        jogo.alertaTemporizador = 3.5f;
        DispararVibracaoGamepad(0.5f, 0.5f, 0.35f);
    }
}

// 5. Anéis: contam quando a nave ATRAVESSA o plano do anel dentro do raio de abertura
static void VerificarAneis(JogoEstado &jogo, Vector3 posAnterior) {
    const Vector3 posAtual = jogo.nave.pos;

    for (auto &anel : jogo.aneis) {
        if (anel.coletado)
            continue;

        // Distâncias (com sinal) da nave ao plano do anel, antes e depois do movimento
        float d0 = Vector3DotProduct(Vector3Subtract(posAnterior, anel.pos), anel.normal);
        float d1 = Vector3DotProduct(Vector3Subtract(posAtual, anel.pos), anel.normal);

        // Sinais opostos = o segmento do movimento cruzou o plano
        if (d0 * d1 > 0.0f || d0 == d1)
            continue;

        // Ponto exato da travessia; só vale se estiver dentro da abertura
        Vector3 ponto = Vector3Lerp(posAnterior, posAtual, d0 / (d0 - d1));
        if (Vector3Distance(ponto, anel.pos) >= anel.raio)
            continue;

        anel.coletado = true;
        jogo.pontuacao += Config::PONTOS_ANEL;
        GerarExplosao3D(jogo, anel.pos, SKYBLUE, 20);
        jogo.alertaTexto        = "ANEL ESPACIAL CRUZADO! +" + std::to_string(Config::PONTOS_ANEL) + " PTS";
        jogo.alertaTemporizador = 2.0f;
        DispararVibracaoGamepad(0.4f, 0.4f, 0.15f);
    }
}

// 6. Partículas: movimento, amortecimento e remoção das expiradas
static void AtualizarParticulas(JogoEstado &jogo, float dt) {
    for (auto &p : jogo.particulas) {
        if (!p.ativo)
            continue;
        p.pos = Vector3Add(p.pos, Vector3Scale(p.vel, dt));
        p.rot = Vector3Add(p.rot, Vector3Scale(p.rotVel, dt));
        p.vel = Vector3Scale(p.vel, 0.96f);  // Atrito viscoso
        p.vida -= dt;
        if (p.vida <= 0.0f)
            p.ativo = false;
    }

    jogo.particulas.erase(
        std::remove_if(jogo.particulas.begin(), jogo.particulas.end(), [](const Particula3D &p) { return !p.ativo; }),
        jogo.particulas.end()
    );
}

// 7. Atalhos que alteram preferências da partida
static void AtualizarAtalhos(JogoEstado &jogo, const ComandosEntrada &cmd) {
    if (cmd.trocarCamera) {
        // Chase -> Cockpit -> Orbital -> Chase
        jogo.modoCamera = static_cast<ModoCamera>((static_cast<int>(jogo.modoCamera) + 1) % 3);
    }
    if (cmd.inverterPitch)
        jogo.inverterPitch = !jogo.inverterPitch;
    if (cmd.alternarHUD)
        jogo.mostrarHUD_CG = !jogo.mostrarHUD_CG;
}

// 8. Câmera: vetores LookAt (position, target, up) e tremor
static void AtualizarCamera(JogoEstado &jogo, float dt, const Orientacao &o) {
    const Nave3D &nave = jogo.nave;
    Camera3D &camera   = jogo.camera;

    switch (jogo.modoCamera) {
    case ModoCamera::Chase: {
        // 3ª pessoa: atrás e acima da nave, seguindo-a com inércia
        Vector3 posDesejada = Vector3Add(nave.pos,
            Vector3Add(Vector3Scale(o.frente, -Config::CAM_CHASE_DISTANCIA), Vector3Scale(o.cima, Config::CAM_CHASE_ALTURA)));
        Vector3 alvoDesejado = Vector3Add(nave.pos, Vector3Scale(o.frente, Config::CAM_CHASE_FOCO));

        camera.position = Vector3Lerp(camera.position, posDesejada,  FatorSuavizacao(Config::SUAVIZ_CAM_POS, dt));
        camera.target   = Vector3Lerp(camera.target,   alvoDesejado, FatorSuavizacao(Config::SUAVIZ_CAM_ALVO, dt));
        camera.up       = Vector3Lerp(camera.up,       o.cima,       FatorSuavizacao(Config::SUAVIZ_CAM_UP, dt));
        break;
    }
    case ModoCamera::Cockpit:
        // 1ª pessoa: ancorada na canópia, sem suavização
        camera.position = Vector3Add(nave.pos,
            Vector3Add(Vector3Scale(o.frente, Config::CAM_COCKPIT_FRENTE), Vector3Scale(o.cima, Config::CAM_COCKPIT_ALTURA)));
        camera.target   = Vector3Add(camera.position, Vector3Scale(o.frente, Config::CAM_COCKPIT_FOCO));
        camera.up       = o.cima;
        break;
    case ModoCamera::Orbital: {
        // Tática: elevada, olhando para a nave
        Vector3 posTatica = Vector3Add(nave.pos, Vector3{0.0f, Config::CAM_ORBITAL_ALTURA, -Config::CAM_ORBITAL_ATRAS});
        camera.position = Vector3Lerp(camera.position, posTatica, FatorSuavizacao(Config::SUAVIZ_CAM_ORBITAL, dt));
        camera.target   = nave.pos;
        camera.up       = Vector3{0.0f, 1.0f, 0.0f};
        break;
    }
    }

    // Tremor: o deslocamento fica à parte e só é somado à câmera na hora de
    // desenhar (ObterCameraRender), sem contaminar o estado da câmera
    jogo.cameraTrauma = Clamp(jogo.cameraTrauma, 0.0f, 1.0f);
    if (jogo.cameraTrauma > 0.001f) {
        float amplitude = jogo.cameraTrauma * Config::TREMOR_AMPLITUDE;
        jogo.tremorCamera = Vector3{AleatorioFloat(-1.0f, 1.0f) * amplitude, AleatorioFloat(-1.0f, 1.0f) * amplitude, 0.0f};
        jogo.cameraTrauma = std::max(0.0f, jogo.cameraTrauma - dt * Config::TREMOR_DECAIMENTO);
    } else {
        jogo.tremorCamera = Vector3{0.0f, 0.0f, 0.0f};
    }
}

// ----------------------------------------------------------------------------
// PASSO DE SIMULAÇÃO
// ----------------------------------------------------------------------------

void AtualizarJogo(JogoEstado &jogo, const ComandosEntrada &cmd, float dt) {
    jogo.faseAnimacao += dt * Config::VEL_FASE_JOGO;
    if (jogo.alertaTemporizador > 0.0f)
        jogo.alertaTemporizador -= dt;

    if (cmd.reiniciarJogo) {
        ReiniciarJogoCompleto(jogo);
        return;
    }
    AtualizarAtalhos(jogo, cmd);

    const Vector3 posAnterior = jogo.nave.pos;  // Necessária para detectar a travessia dos anéis

    Orientacao o = AtualizarControleVoo(jogo, cmd, dt);
    AtualizarDisparo(jogo, cmd, dt, o);
    AtualizarProjeteis(jogo, dt);
    AtualizarAlvos(jogo, dt);
    VerificarAneis(jogo, posAnterior);
    AtualizarParticulas(jogo, dt);
    AtualizarCamera(jogo, dt, o);
}
