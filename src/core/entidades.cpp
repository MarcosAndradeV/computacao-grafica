// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/core/entidades.cpp
//
// Descrição:
//   Implementa as funções matemáticas de orientação tridimensional, geração
//   procedural em coordenadas esféricas/cilíndricas e inicialização do estado
//   das entidades do mundo 3D.
//
// Conceitos de CG Relacionados:
//   - Composição de Matrizes de Rotação 3D (R = Rz(roll) * Rx(-pitch) * Ry(yaw))
//   - Transformação de vetores da Base Canônica Local para o Espaço do Mundo
//   - Conversão de Coordenadas Esféricas (r, theta, phi) para Cartesianas (x, y, z)
// ============================================================================

#include "core/entidades.h"
#include "raymath.h"

#include <cmath>
#include <cstdlib>

// ----------------------------------------------------------------------------
// FUNÇÕES AUXILIARES DE MATEMÁTICA VETORIAL E COORDENADAS ESFÉRICAS
// ----------------------------------------------------------------------------

float AleatorioFloat(float minVal, float maxVal) {
    return minVal + ((float)std::rand() / (float)RAND_MAX) * (maxVal - minVal);
}

// Converte coordenadas esféricas aleatórias (azimute theta e elevação phi)
// em um vetor cartesiano 3D de magnitude `intensidade`:
//   x = r * cos(phi) * cos(theta)
//   y = r * sin(phi)
//   z = r * cos(phi) * sin(theta)
Vector3 VetorAleatorio3D(float intensidade) {
    float theta = AleatorioFloat(0.0f, 2.0f * PI);
    float phi   = AleatorioFloat(-PI * 0.5f, PI * 0.5f);
    return (Vector3){
        intensidade * std::cos(phi) * std::cos(theta),
        intensidade * std::sin(phi),
        intensidade * std::cos(phi) * std::sin(theta)
    };
}

// Calcula os vetores diretores ortonormais da nave (Forward, Up e Right)
// multiplicando os eixos canônicos locais (Z+, Y+, X+) pela Matriz de Rotação
// Composta construída a partir dos ângulos de Euler (Yaw, Pitch, Roll).
void CalcularOrientacaoNave(const Nave3D &nave, Vector3 &outFwd, Vector3 &outUp, Vector3 &outRight) {
    Matrix matRot = MatrixMultiply(
        MatrixRotateZ(nave.roll * DEG2RAD),
        MatrixMultiply(MatrixRotateX(-nave.pitch * DEG2RAD), MatrixRotateY(nave.yaw * DEG2RAD))
    );

    outFwd   = Vector3Normalize(Vector3Transform((Vector3){0.0f, 0.0f, 1.0f}, matRot));
    outUp    = Vector3Normalize(Vector3Transform((Vector3){0.0f, 1.0f, 0.0f}, matRot));
    outRight = Vector3Normalize(Vector3Transform((Vector3){1.0f, 0.0f, 0.0f}, matRot));
}

// ----------------------------------------------------------------------------
// INICIALIZAÇÃO PROCEDURAL DO AMBIENTE E ENTIDADES 3D
// ----------------------------------------------------------------------------

void InicializarEstrelas(JogoEstado &jogo) {
    jogo.estrelas.clear();
    jogo.estrelas.reserve(Config::TOTAL_ESTRELAS);

    for (int i = 0; i < Config::TOTAL_ESTRELAS; ++i) {
        // Distribui as estrelas em uma casca esférica com raio entre 80 e 180 unidades
        float r     = AleatorioFloat(80.0f, 180.0f);
        float theta = AleatorioFloat(0.0f, 2.0f * PI);
        float phi   = AleatorioFloat(-PI * 0.48f, PI * 0.48f);

        Vector3 pos = {
            r * std::cos(phi) * std::cos(theta),
            r * std::sin(phi),
            r * std::cos(phi) * std::sin(theta)
        };

        Color cor = WHITE;
        int tipoCor = std::rand() % 5;
        if (tipoCor == 0)
            cor = SKYBLUE;
        else if (tipoCor == 1)
            cor = (Color){200, 220, 255, 255};
        else if (tipoCor == 2)
            cor = (Color){255, 230, 180, 255};
        else if (tipoCor == 3)
            cor = (Color){255, 180, 180, 255};

        jogo.estrelas.push_back({
            pos,
            cor,
            AleatorioFloat(0.4f, 1.2f),
            AleatorioFloat(0.0f, 6.28f)
        });
    }
}

void InicializarAlvos(JogoEstado &jogo) {
    jogo.alvos.clear();

    // A quantidade de alvos escala progressivamente a cada onda concluída
    const int qtd = 8 + (jogo.ondaAtual - 1) * 2;
    jogo.alvos.reserve(qtd);

    for (int i = 0; i < qtd; ++i) {
        Alvo3D alvo;

        // Distribuição em coordenadas cilíndricas (ângulo, raio, altura Y)
        float angulo = ((float)i / (float)qtd) * 2.0f * PI + AleatorioFloat(-0.2f, 0.2f);
        float dist   = AleatorioFloat(18.0f, 42.0f);
        float altura = AleatorioFloat(-8.0f, 14.0f);

        alvo.pos = (Vector3){
            std::cos(angulo) * dist,
            altura,
            std::sin(angulo) * dist
        };

        // Orientação inicial e velocidades angulares independentes nos 3 eixos
        alvo.rotAngulo = (Vector3){
            AleatorioFloat(0.0f, 360.0f),
            AleatorioFloat(0.0f, 360.0f),
            AleatorioFloat(0.0f, 360.0f)
        };
        alvo.rotVel = (Vector3){
            AleatorioFloat(-45.0f, 45.0f),
            AleatorioFloat(40.0f, 90.0f),
            AleatorioFloat(-30.0f, 30.0f)
        };
        alvo.escalaBase = AleatorioFloat(1.2f, 2.0f);
        alvo.tipo       = static_cast<TipoAlvo>(i % 3);

        if (alvo.tipo == TipoAlvo::DroneSentinela)
            alvo.cor = (Color){255, 80, 80, 255};   // Vermelho Neon
        else if (alvo.tipo == TipoAlvo::AsteroideCristalino)
            alvo.cor = (Color){60, 255, 160, 255};  // Verde Esmeralda
        else
            alvo.cor = (Color){255, 215, 0, 255};   // Dourado

        alvo.ativo = true;
        jogo.alvos.push_back(alvo);
    }
}

void InicializarAneis(JogoEstado &jogo) {
    jogo.aneis.clear();
    jogo.aneis.reserve(Config::TOTAL_ANEIS);

    for (int i = 0; i < Config::TOTAL_ANEIS; ++i) {
        float ang  = ((float)i / (float)Config::TOTAL_ANEIS) * 2.0f * PI;
        float dist = 28.0f;

        jogo.aneis.push_back({
            (Vector3){std::cos(ang) * dist, AleatorioFloat(2.0f, 8.0f), std::sin(ang) * dist},
            ang * RAD2DEG + 90.0f,
            4.0f,
            false
        });
    }
}

void ReiniciarJogoCompleto(JogoEstado &jogo) {
    // Estado inicial da Nave no Espaço 3D
    jogo.nave.pos             = (Vector3){0.0f, 0.0f, -15.0f};
    jogo.nave.vel             = (Vector3){0.0f, 0.0f, 0.0f};
    jogo.nave.pitch           = 0.0f;
    jogo.nave.yaw             = 0.0f;
    jogo.nave.roll            = 0.0f;
    jogo.nave.rollDesejado    = 0.0f;
    jogo.nave.escala          = 1.0f;
    jogo.nave.velocidadeAtual = 8.0f;
    jogo.nave.velocidadeMax   = 26.0f;
    jogo.nave.aceleracao      = 14.0f;
    jogo.nave.acelerando      = false;
    jogo.nave.turbo           = false;
    jogo.nave.freando         = false;

    // Configuração inicial da Câmera Sintética (Matriz de Visualização e Projeção)
    jogo.camera.position   = (Vector3){0.0f, 6.0f, -28.0f};
    jogo.camera.target     = (Vector3){0.0f, 0.0f, 0.0f};
    jogo.camera.up         = (Vector3){0.0f, 1.0f, 0.0f};
    jogo.camera.fovy       = Config::FOV_PADRAO;
    jogo.camera.projection = CAMERA_PERSPECTIVE;

    jogo.pontuacao    = 0;
    jogo.ondaAtual    = 1;
    jogo.cameraTrauma = 0.0f;

    jogo.tiros.clear();
    jogo.particulas.clear();

    InicializarAlvos(jogo);
    InicializarAneis(jogo);
    InicializarEstrelas(jogo);

    jogo.alertaTexto        = "COMBATE INICIADO! DESTRUA OS ALVOS 3D";
    jogo.alertaTemporizador = 3.5f;
}
