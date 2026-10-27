// ============================================================================
// Computacao Grafica - Space Combat 3D
// Modulo: src/input/input.h
//
// Descricao:
//   Gerenciamento unificado de perifericos de entrada (Teclado, Mouse e Gamepad).
//   Isola a leitura de hardware, zonas mortas (deadzones), sensibilidade do
//   mouse e mapeamento de comandos para a simulacao de voo espacial e combate.
// ============================================================================

#ifndef INPUT_H
#define INPUT_H

#include "core/entidades.h"

// Estrutura com os comandos normalizados resultantes de todas as entradas
struct ComandosEntrada {
    float arfagem;          // Pitch: [-1.0f .. +1.0f] (Nariz sobe/desce no eixo local X)
    float guinada;          // Yaw:   [-1.0f .. +1.0f] (Vira esquerda/direita no eixo Y)
    float rolagem;          // Roll:  [-1.0f .. +1.0f] (Inclinacao manual das asas no eixo Z)
    float aceleracao;       // Throttle: -1.0f (Freio), 0.0f (Cruzeiro), +1.0f (Turbo)

    bool atirar;            // Disparo instantaneo dos canhoes laser (single click)
    bool atirarContinuo;    // Disparo continuo ao segurar botao
    bool trocarCamera;      // Alternancia do modo de visualizacao (Chase/Cockpit/Orbital)
    bool inverterPitch;     // Alternar inversao do eixo vertical
    bool alternarHUD;       // Exibir/ocultar painel de CG
    bool reiniciarJogo;     // Reiniciar rodada
    bool alternarMouse;     // Ativar/desativar captura do cursor do mouse
};

// Inicializa o subsistema de entrada (captura do cursor do mouse para voo 3D)
void InicializarEntradas();

// Processa todas as fontes de entrada ativas (Teclado, Mouse e Gamepad)
ComandosEntrada LerComandosEntrada(const JogoEstado &jogo);

// Aplica vibracao haptica no gamepad conectado (se disponivel)
void DispararVibracaoGamepad(float motorEsq, float motorDir, float duracaoSegundos);

// Verifica se ha pelo menos um gamepad conectado e ativo
bool GamepadConectado();

// Retorna o identificador amigavel do gamepad conectado ou "NENHUM"
const char *ObterNomeGamepad();

// Retorna se o cursor do mouse esta atualmente capturado (modo voo)
bool MouseEstaCapturado();

// Alterna o modo de captura do mouse (DisableCursor / EnableCursor)
void AlternarCapturaMouse();

#endif // INPUT_H
