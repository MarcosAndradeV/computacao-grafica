// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/input/input.h
//
// Descrição:
//   Leitura unificada de periféricos (teclado, mouse e gamepad). O voo é
//   controlado por teclado/gamepad; o mouse só atira (botão esquerdo) e
//   aciona o nitro (botão direito). O resultado é um `ComandosEntrada`
//   normalizado, que desacopla a física do hardware.
// ============================================================================

#ifndef INPUT_H
#define INPUT_H

// Comandos normalizados resultantes de todas as fontes de entrada
struct ComandosEntrada {
    float arfagem = 0.0f;     // Pitch: [-1, +1] (nariz sobe/desce)
    float guinada = 0.0f;     // Yaw:   [-1, +1] (esquerda = +1)
    float rolagem = 0.0f;     // Roll:  [-1, +1] (inclinação manual das asas)
    float aceleracao = 0.0f;  // -1 = freio, 0 = cruzeiro, +1 = turbo (turbo vence freio)

    bool atirar = false;          // Clique/tecla recém-pressionado
    bool atirarContinuo = false;  // Botão/tecla mantido
    bool trocarCamera = false;
    bool inverterPitch = false;
    bool alternarHUD = false;
    bool reiniciarJogo = false;
    bool pausar = false;
};

// Lê todas as fontes de entrada ativas e devolve os comandos do quadro
ComandosEntrada LerComandosEntrada();

// Vibração háptica no gamepad conectado (sem efeito se não houver)
void DispararVibracaoGamepad(float motorEsq, float motorDir, float duracaoSegundos);

bool GamepadConectado();

// Nome do gamepad conectado ou "NENHUM"
const char *ObterNomeGamepad();

#endif // INPUT_H
