// ============================================================================
// Computacao Grafica - Space Combat 3D
// Modulo: src/input/input.cpp
//
// Descricao:
//   Implementacao do subsistema unificado de entrada (Teclado, Mouse e Gamepad).
//   Processa movimentos continuos de mouse com DisableCursor/EnableCursor,
//   mapeia eixos analogicos de gamepad com calibracao de zona morta (deadzone),
//   gatilhos RT/LT, botoes de acao e feedback haptico de vibracao.
// ============================================================================

#include "input/input.h"
#include "raymath.h"
#include <cmath>

namespace {
    bool s_mouseCapturado = true;
    constexpr float SENSIBILIDADE_MOUSE = 0.08f;
    constexpr float ZONA_MORTA_GAMEPAD  = 0.14f;

    // Encontra o indice do primeiro gamepad conectado (0 a 3) ou retorna -1
    int ObterIdPrimeiroGamepad() {
        for (int i = 0; i < 4; ++i) {
            if (IsGamepadAvailable(i)) {
                return i;
            }
        }
        return -1;
    }

    // Aplica zona morta e normaliza o eixo do gamepad para o intervalo [-1.0f .. +1.0f]
    float FiltrarEixoGamepad(float valorBruto) {
        if (std::fabs(valorBruto) < ZONA_MORTA_GAMEPAD) {
            return 0.0f;
        }
        if (valorBruto > 0.0f) {
            return (valorBruto - ZONA_MORTA_GAMEPAD) / (1.0f - ZONA_MORTA_GAMEPAD);
        }
        return (valorBruto + ZONA_MORTA_GAMEPAD) / (1.0f - ZONA_MORTA_GAMEPAD);
    }
}

void InicializarEntradas() {
    s_mouseCapturado = true;
    DisableCursor(); // Trava o cursor no centro e oculta para voo relativo 3D
}

bool MouseEstaCapturado() {
    return s_mouseCapturado;
}

void AlternarCapturaMouse() {
    s_mouseCapturado = !s_mouseCapturado;
    if (s_mouseCapturado) {
        DisableCursor();
    } else {
        EnableCursor();
    }
}

bool GamepadConectado() {
    return (ObterIdPrimeiroGamepad() != -1);
}

const char *ObterNomeGamepad() {
    int id = ObterIdPrimeiroGamepad();
    if (id != -1) {
        const char *nome = GetGamepadName(id);
        if (nome != nullptr && nome[0] != '\0') {
            return nome;
        }
        return "GAMEPAD DETECTADO";
    }
    return "NENHUM";
}

void DispararVibracaoGamepad(float motorEsq, float motorDir, float duracaoSegundos) {
    int id = ObterIdPrimeiroGamepad();
    if (id != -1) {
        SetGamepadVibration(id, motorEsq, motorDir, duracaoSegundos);
    }
}

ComandosEntrada LerComandosEntrada(const JogoEstado &jogo) {
    (void)jogo; // Usado caso queira ler configuracoes
    ComandosEntrada cmd = {0.0f, 0.0f, 0.0f, 0.0f, false, false, false, false, false, false, false};

    // ------------------------------------------------------------------------
    // 1. TECLADO (KEYBOARD)
    // ------------------------------------------------------------------------
    // Arfagem (Pitch)
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        cmd.arfagem += 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        cmd.arfagem -= 1.0f;

    // Guinada (Yaw)
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        cmd.guinada += 1.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        cmd.guinada -= 1.0f;

    // Rolagem Manual (Roll)
    if (IsKeyDown(KEY_Q))
        cmd.rolagem += 1.0f;
    if (IsKeyDown(KEY_E))
        cmd.rolagem -= 1.0f;

    // Propulsao
    if (IsKeyDown(KEY_LEFT_SHIFT))
        cmd.aceleracao = 1.0f; // Turbo
    else if (IsKeyDown(KEY_LEFT_CONTROL))
        cmd.aceleracao = -1.0f; // Freio

    // Acoes do Teclado
    if (IsKeyPressed(KEY_SPACE))
        cmd.atirar = true;
    if (IsKeyDown(KEY_SPACE))
        cmd.atirarContinuo = true;

    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_TAB))
        cmd.trocarCamera = true;
    if (IsKeyPressed(KEY_I))
        cmd.inverterPitch = true;
    if (IsKeyPressed(KEY_H))
        cmd.alternarHUD = true;
    if (IsKeyPressed(KEY_R))
        cmd.reiniciarJogo = true;
    if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE))
        cmd.alternarMouse = true;

    // ------------------------------------------------------------------------
    // 2. MOUSE
    // ------------------------------------------------------------------------
    // Se o mouse estiver capturado, converte delta de movimento em voo espacial
    if (s_mouseCapturado) {
        Vector2 deltaMouse = GetMouseDelta();

        // Movimento horizontal do mouse comanda Guinada (Yaw)
        cmd.guinada -= deltaMouse.x * SENSIBILIDADE_MOUSE;

        // Movimento vertical do mouse comanda Arfagem (Pitch)
        // Empurrar para frente (delta negativo) inclina o nariz para cima ou para baixo
        cmd.arfagem -= deltaMouse.y * SENSIBILIDADE_MOUSE;

        // Botoes do mouse
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            cmd.atirar = true;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            cmd.atirarContinuo = true;

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            cmd.aceleracao = 1.0f; // Botao direito ativa turbo

        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
            cmd.trocarCamera = true; // Botao do meio troca camera

        // Roda do mouse ajusta propulsao
        float wheel = GetMouseWheelMove();
        if (wheel > 0.05f)
            cmd.aceleracao = 1.0f;
        else if (wheel < -0.05f)
            cmd.aceleracao = -1.0f;
    } else {
        // Se o mouse nao estava capturado e o jogador clicou na janela, reativa a captura
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            AlternarCapturaMouse();
        }
    }

    // ------------------------------------------------------------------------
    // 3. GAMEPAD (JOYSTICK / CONTROLE)
    // ------------------------------------------------------------------------
    int gpId = ObterIdPrimeiroGamepad();
    if (gpId != -1) {
        // Analógico Esquerdo: Voo Principal (Pitch e Yaw)
        float stickX = FiltrarEixoGamepad(GetGamepadAxisMovement(gpId, GAMEPAD_AXIS_LEFT_X));
        float stickY = FiltrarEixoGamepad(GetGamepadAxisMovement(gpId, GAMEPAD_AXIS_LEFT_Y));

        cmd.guinada -= stickX; // Esquerda = positivo, Direita = negativo
        cmd.arfagem -= stickY; // Para cima = arfar nariz, Para baixo = mergulhar

        // Analógico Direito: Rolagem manual das asas (Roll)
        float rStickX = FiltrarEixoGamepad(GetGamepadAxisMovement(gpId, GAMEPAD_AXIS_RIGHT_X));
        cmd.rolagem -= rStickX;

        // D-Pad (Controle Direcional) como alternativa digital de voo
        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_LEFT_FACE_UP))
            cmd.arfagem += 1.0f;
        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
            cmd.arfagem -= 1.0f;
        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            cmd.guinada += 1.0f;
        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            cmd.guinada -= 1.0f;

        // Bumpers (LB / RB) para rolagem manual
        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
            cmd.rolagem += 1.0f; // LB: Rolar para a esquerda
        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
            cmd.rolagem -= 1.0f; // RB: Rolar para a direita

        // Gatilhos Traseiros (LT = Freio Espacial, RT = Turbo)
        float gatilhoRT = GetGamepadAxisMovement(gpId, GAMEPAD_AXIS_RIGHT_TRIGGER);
        float gatilhoLT = GetGamepadAxisMovement(gpId, GAMEPAD_AXIS_LEFT_TRIGGER);

        if (gatilhoRT > 0.1f || IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)) {
            cmd.aceleracao = 1.0f; // Turbo via RT
        } else if (gatilhoLT > 0.1f || IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_LEFT_TRIGGER_2)) {
            cmd.aceleracao = -1.0f; // Freio via LT
        }

        // Botoes de Acao (Layout padrao A/B/X/Y ou Cross/Circle/Square/Triangle)
        if (IsGamepadButtonPressed(gpId, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || // A / Cross
            IsGamepadButtonPressed(gpId, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))   // X / Square
        {
            cmd.atirar = true;
        }

        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
            IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))
        {
            cmd.atirarContinuo = true;
        }

        if (IsGamepadButtonDown(gpId, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) { // B / Circle
            cmd.aceleracao = 1.0f; // Turbo alternativo no botao B
        }

        if (IsGamepadButtonPressed(gpId, GAMEPAD_BUTTON_RIGHT_FACE_UP)) { // Y / Triangle
            cmd.trocarCamera = true;
        }

        // Botoes Centrais (Start, Select / Back)
        if (IsGamepadButtonPressed(gpId, GAMEPAD_BUTTON_MIDDLE_RIGHT)) { // Start / Menu
            cmd.reiniciarJogo = true;
        }
        if (IsGamepadButtonPressed(gpId, GAMEPAD_BUTTON_MIDDLE_LEFT)) { // Select / Back / View
            cmd.alternarHUD = true;
        }

        // Clique no Analógico Esquerdo (L3) inverte o eixo vertical
        if (IsGamepadButtonPressed(gpId, GAMEPAD_BUTTON_LEFT_THUMB)) {
            cmd.inverterPitch = true;
        }
    }

    // Alternar captura do mouse caso o comando tenha sido disparado
    if (cmd.alternarMouse) {
        AlternarCapturaMouse();
    }

    // Normaliza amplitudes para os limites [-1.0f, 1.0f]
    cmd.arfagem = Clamp(cmd.arfagem, -1.0f, 1.0f);
    cmd.guinada = Clamp(cmd.guinada, -1.0f, 1.0f);
    cmd.rolagem = Clamp(cmd.rolagem, -1.0f, 1.0f);

    return cmd;
}
