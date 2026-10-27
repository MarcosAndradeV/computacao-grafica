// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/fisica/logica.h
//
// Descrição:
//   Atualização da simulação a cada quadro: voo da nave, disparos, colisões,
//   ondas, anéis, partículas e câmera.
//
// Conceitos de CG relacionados:
//   - Cinemática no R³: P(t + dt) = P(t) + V·dt
//   - Suavização exponencial (inércia da câmera e do roll)
//   - Colisão contínua segmento × esfera e travessia de plano (anéis)
//   - Parâmetros LookAt (eye, target, up) da Matriz de Visualização
// ============================================================================

#ifndef LOGICA_H
#define LOGICA_H

#include "core/entidades.h"
#include "input/input.h"

// Gera uma explosão de fragmentos 3D centrada em `centro`
void GerarExplosao3D(JogoEstado &jogo, Vector3 centro, Color corBase, int quantidade);

// Avança a simulação em `dt` segundos usando os comandos já lidos do quadro
void AtualizarJogo(JogoEstado &jogo, const ComandosEntrada &cmd, float dt);

#endif // LOGICA_H
