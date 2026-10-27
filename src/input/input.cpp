// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/input/input.cpp
//
// Descrição:
//   Implementação da leitura de teclado, mouse e gamepad (com zona morta nos
//   analógicos, gatilhos e vibração). Quando várias fontes pedem propulsão ao
//   mesmo tempo, o turbo tem prioridade sobre o freio.
// ============================================================================

#include "input/input.h"
#include "raylib.h"
#include "raymath.h"
#include <cmath>

namespace {
    constexpr float ZONA_MORTA_GAMEPAD = 0.14f;
    constexpr int   MAX_GAMEPADS       = 4;

    // Índice do primeiro gamepad conectado, ou -1
    int ObterIdPrimeiroGamepad() {
        for (int i = 0; i < MAX_GAMEPADS; ++i) {
            if (IsGamepadAvailable(i)) {
                return i;
            }
        }
        return -1;
    }

    // Remove a zona morta e reescala o eixo para cobrir todo o intervalo [-1, +1]
    float FiltrarEixoGamepad(float valorBruto) {
        if (std::fabs(valorBruto) < ZONA_MORTA_GAMEPAD) {
            return 0.0f;
        }
        float sinal = (valorBruto > 0.0f) ? 1.0f : -1.0f;
        return sinal * (std::fabs(valorBruto) - ZONA_MORTA_GAMEPAD) / (1.0f - ZONA_MORTA_GAMEPAD);
    }
}

bool GamepadConectado() {
    return ObterIdPrimeiroGamepad() != -1;
}

const char *ObterNomeGamepad() {
    int id = ObterIdPrimeiroGamepad();
    if (id == -1) {
        return "NENHUM";
    }
    const char *nome = GetGamepadName(id);
    return (nome != nullptr && nome[0] != '\0') ? nome : "GAMEPAD DETECTADO";
}

void DispararVibracaoGamepad(float motorEsq, float motorDir, float duracaoSegundos) {
    int id = ObterIdPrimeiroGamepad();
    if (id != -1) {
        SetGamepadVibration(id, motorEsq, motorDir, duracaoSegundos);
    }
}

ComandosEntrada LerComandosEntrada() {
    ComandosEntrada cmd;
    bool pedeTurbo = false;
    bool pedeFreio = false;

    // ------------------------------------------------------------------------
    // 1. TECLADO (voo principal)
    // ------------------------------------------------------------------------
    // Arfagem (pitch): W / ↑ sobe o nariz, S / ↓ desce
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        cmd.arfagem += 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        cmd.arfagem -= 1.0f;

    // Guinada (yaw): A / ← vira à esquerda, D / → à direita
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        cmd.guinada += 1.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        cmd.guinada -= 1.0f;

    // Rolagem manual (roll): Q inclina à esquerda, E à direita
    if (IsKeyDown(KEY_Q))
        cmd.rolagem += 1.0f;
    if (IsKeyDown(KEY_E))
        cmd.rolagem -= 1.0f;

    pedeTurbo |= IsKeyDown(KEY_LEFT_SHIFT);
    pedeFreio |= IsKeyDown(KEY_LEFT_CONTROL);

    cmd.atirar         |= IsKeyPressed(KEY_SPACE);
    cmd.atirarContinuo |= IsKeyDown(KEY_SPACE);

    cmd.trocarCamera  |= IsKeyPressed(KEY_C) || IsKeyPressed(KEY_TAB);
    cmd.inverterPitch |= IsKeyPressed(KEY_I);
    cmd.alternarHUD   |= IsKeyPressed(KEY_H);
    cmd.reiniciarJogo |= IsKeyPressed(KEY_R);
    cmd.pausar        |= IsKeyPressed(KEY_ESCAPE);

    // ------------------------------------------------------------------------
    // 2. MOUSE (só dispara e nitro; mover o mouse não gira a nave)
    // ------------------------------------------------------------------------
    cmd.atirar         |= IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    cmd.atirarContinuo |= IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    pedeTurbo          |= IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

    // ------------------------------------------------------------------------
    // 3. GAMEPAD (opcional)
    // ------------------------------------------------------------------------
    int gp = ObterIdPrimeiroGamepad();
    if (gp != -1) {
        // Analógico esquerdo: pitch e yaw
        cmd.guinada -= FiltrarEixoGamepad(GetGamepadAxisMovement(gp, GAMEPAD_AXIS_LEFT_X));
        cmd.arfagem -= FiltrarEixoGamepad(GetGamepadAxisMovement(gp, GAMEPAD_AXIS_LEFT_Y));

        // Analógico direito: roll manual
        cmd.rolagem -= FiltrarEixoGamepad(GetGamepadAxisMovement(gp, GAMEPAD_AXIS_RIGHT_X));

        // D-Pad: alternativa digital de voo
        if (IsGamepadButtonDown(gp, GAMEPAD_BUTTON_LEFT_FACE_UP))    cmd.arfagem += 1.0f;
        if (IsGamepadButtonDown(gp, GAMEPAD_BUTTON_LEFT_FACE_DOWN))  cmd.arfagem -= 1.0f;
        if (IsGamepadButtonDown(gp, GAMEPAD_BUTTON_LEFT_FACE_LEFT))  cmd.guinada += 1.0f;
        if (IsGamepadButtonDown(gp, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) cmd.guinada -= 1.0f;

        // Bumpers: roll manual
        if (IsGamepadButtonDown(gp, GAMEPAD_BUTTON_LEFT_TRIGGER_1))  cmd.rolagem += 1.0f;
        if (IsGamepadButtonDown(gp, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) cmd.rolagem -= 1.0f;

        // Gatilhos: RT / botão B = turbo, LT = freio
        pedeTurbo |= GetGamepadAxisMovement(gp, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.1f
                  || IsGamepadButtonDown(gp, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)
                  || IsGamepadButtonDown(gp, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
        pedeFreio |= GetGamepadAxisMovement(gp, GAMEPAD_AXIS_LEFT_TRIGGER) > 0.1f
                  || IsGamepadButtonDown(gp, GAMEPAD_BUTTON_LEFT_TRIGGER_2);

        // Botões de ação: A / X disparam
        cmd.atirar         |= IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)
                           || IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
        cmd.atirarContinuo |= IsGamepadButtonDown(gp, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)
                           || IsGamepadButtonDown(gp, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);

        cmd.trocarCamera  |= IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_RIGHT_FACE_UP);   // Y / Triângulo
        cmd.pausar        |= IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_MIDDLE_RIGHT);    // Start
        cmd.alternarHUD   |= IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_MIDDLE_LEFT);     // Select
        cmd.inverterPitch |= IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_LEFT_THUMB);      // L3
    }

    // Turbo vence freio quando as duas fontes pedem ao mesmo tempo
    cmd.aceleracao = pedeTurbo ? 1.0f : (pedeFreio ? -1.0f : 0.0f);

    cmd.arfagem = Clamp(cmd.arfagem, -1.0f, 1.0f);
    cmd.guinada = Clamp(cmd.guinada, -1.0f, 1.0f);
    cmd.rolagem = Clamp(cmd.rolagem, -1.0f, 1.0f);

    return cmd;
}
