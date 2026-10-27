// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/fisica/logica.h
//
// Descrição:
//   Declara as rotinas de atualização de física cinemática 3D, controles de
//   voo espacial (Pitch, Yaw, Roll), disparo de projéteis laser, detecção de
//   colisão esférica (Bounding Sphere), sistema de partículas volumétricas e
//   atualização da Matriz de Visualização (Câmera 3D).
//
// Conceitos de CG Relacionados:
//   - Cinemática Vetorial no R³: P(t + dt) = P(t) + V * dt
//   - Interpolação Linear (Lerp) para amortecimento inercial de câmera e roll
//   - Colisão Esférica 3D por distância euclidiana: ||P1 - P2|| < R
//   - Atualização dinâmica dos parâmetros LookAt (eye, target, up)
// ============================================================================

#ifndef LOGICA_H
#define LOGICA_H

#include "core/entidades.h"

// Gera uma explosão volumétrica de fragmentos 3D centrada em `centro`
void GerarExplosao3D(JogoEstado &jogo, Vector3 centro, Color corBase, int quantidade);

// Atualiza todos os subsistemas da simulação a cada quadro (passo de tempo `dt`)
void AtualizarJogo(JogoEstado &jogo, float dt);

#endif // LOGICA_H
