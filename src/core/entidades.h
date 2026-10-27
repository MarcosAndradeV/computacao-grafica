// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/core/entidades.h
//
// Descrição:
//   Define todas as estruturas de dados (entidades geométricas 3D), enumerações,
//   constantes de configuração espacial e a estrutura central `JogoEstado` que
//   encapsula o estado completo da simulação sem variáveis globais soltas.
//
// Conceitos de CG Relacionados:
//   - Representação de posição, velocidade e orientação no espaço 3D (R³)
//   - Ângulos de Euler (Pitch / Arfagem, Yaw / Guinada, Roll / Rolagem)
//   - Câmera Sintética 3D (Matriz de Visualização e Projeção Perspectiva)
// ============================================================================

#ifndef ENTIDADES_H
#define ENTIDADES_H

#include "raylib.h"
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// CONSTANTES DO ESPAÇO DE COMBATE E CONFIGURAÇÕES GERAIS
// ----------------------------------------------------------------------------

namespace Config {
    constexpr int LARGURA_INICIAL = 1024;
    constexpr int ALTURA_INICIAL  = 720;
    constexpr int FPS_ALVO        = 60;

    constexpr float FOV_PADRAO            = 60.0f;  // Campo de visão vertical (em graus) da Projeção Perspectiva
    constexpr float RAIO_MUNDO            = 65.0f;  // Limite esférico do espaço de combate
    constexpr float SENSIBILIDADE_VOO     = 95.0f;  // Velocidade angular de rotação (graus/s)
    constexpr float LIMITE_PITCH          = 85.0f;  // Limite de arfagem para evitar Gimbal Lock
    constexpr float VELOCIDADE_CRUZEIRO   = 12.0f;  // Velocidade padrão da nave
    constexpr float VELOCIDADE_TURBO      = 28.0f;  // Velocidade com propulsão máxima (Shift)
    constexpr float VELOCIDADE_FREIO      = 4.0f;   // Velocidade reduzida com freio espacial (Ctrl)
    constexpr float VELOCIDADE_TIRO       = 75.0f;  // Velocidade de propagação dos feixes laser
    constexpr float TEMPO_VIDA_TIRO       = 2.5f;   // Duração máxima de um projétil no espaço (s)
    constexpr int   TOTAL_ESTRELAS        = 500;    // Quantidade de estrelas na esfera celeste
    constexpr int   TOTAL_ANEIS           = 5;      // Quantidade de anéis de checkpoint
}

// ----------------------------------------------------------------------------
// ENUMERAÇÕES DE CÂMERA E TIPOS DE GEOMETRIA
// ----------------------------------------------------------------------------

// Modos da Matriz de Visualização (View Matrix - LookAt)
enum class ModoCamera {
    Chase = 0,   // 3ª Pessoa: Câmera de perseguição com interpolação inercial (Lerp)
    Cockpit = 1, // 1ª Pessoa: Câmera posicionada no cockpit acompanhando o vetor Up da nave
    Orbital = 2  // Tática: Câmera superior afastada para visualização global do espaço 3D
};

// Tipos de geometria procedural dos alvos inimigos
enum class TipoAlvo {
    DroneSentinela      = 0, // Cubo com núcleo esférico e anéis orbitais com rotações independentes
    AsteroideCristalino = 1, // Cubos entrelaçados rotacionados a 45° nos eixos X e Z
    PrismaEnergia       = 2  // Octaedro / Pirâmides duplas giratórias
};

// ----------------------------------------------------------------------------
// ESTRUTURAS DAS ENTIDADES 3D
// ----------------------------------------------------------------------------

// Projétil disparado pelos canhões das pontas das asas da nave
struct Projetil3D {
    Vector3 pos;  // Posição atual no espaço do mundo (World Space)
    Vector3 dir;  // Vetor direção unitário de propagação
    float vida;   // Tempo restante de existência (em segundos)
    bool ativo;   // Indica se o projétil ainda existe na cena
};

// Fragmento tridimensional gerado em explosões volumétricas
struct Particula3D {
    Vector3 pos;     // Posição 3D no mundo
    Vector3 vel;     // Vetor velocidade linear 3D
    Vector3 rot;     // Ângulos de Euler atuais (X, Y, Z) da partícula
    Vector3 rotVel;  // Velocidade angular em cada eixo (graus/s)
    float tamanho;   // Escala geométrica do fragmento
    Color cor;       // Cor base do fragmento (com fade-out proporcional à vida)
    float vida;      // Tempo restante de vida
    float vidaMax;   // Tempo total inicial (usado para normalizar o canal Alpha)
    bool ativo;      // Indica se a partícula está ativa
};

// Alvo espacial que sofre transformações compostas (T * R * S) a cada quadro
struct Alvo3D {
    Vector3 pos;        // Posição no espaço 3D (Translação T)
    Vector3 rotAngulo;  // Ângulos de rotação acumulados nos eixos X, Y, Z (Rotação R)
    Vector3 rotVel;     // Velocidade angular em cada eixo (graus/s)
    float escalaBase;   // Fator de escala base antes da pulsação harmônica (Escala S)
    Color cor;          // Cor primária da malha wireframe
    TipoAlvo tipo;      // Tipo de modelo geométrico 3D
    bool ativo;         // Indica se o alvo ainda não foi destruído
};

// Anel de navegação espacial (Checkpoint)
struct AnelCheckpoint {
    Vector3 pos;    // Posição central do anel no mundo
    float rotY;     // Orientação angular ao redor do eixo Y (graus)
    float raio;     // Raio de abertura do anel para detecção de passagem
    bool coletado;  // Indica se a nave já atravessou este anel
};

// Ponto luminoso distante no fundo estelar (gerado em coordenadas esféricas)
struct Estrela3D {
    Vector3 pos;       // Posição cartesiana convertida a partir de (r, theta, phi)
    Color cor;         // Tonalidade estelar (azul, branca, amarela, avermelhada)
    float tamanho;     // Dimensão base do cubo estelar
    float faseBrilho;  // Fase inicial (radianos) para cintilação senoidal
};

// Jogador: Nave espacial controlada com 6 graus de liberdade parciais (Pitch, Yaw, Roll + Translação)
struct Nave3D {
    Vector3 pos;            // Posição atual no espaço do mundo (Translação)
    Vector3 vel;            // Vetor velocidade resultante (Forward * velocidadeAtual)
    float pitch;            // Arfagem: rotação em torno do eixo local X (nariz sobe/desce)
    float yaw;              // Guinada: rotação em torno do eixo Y (vira para esquerda/direita)
    float roll;             // Rolagem: rotação em torno do eixo longitudinal Z (inclinação das asas)
    float rollDesejado;     // Ângulo alvo de rolagem para efeito de banking suave (Lerp)
    float escala;           // Fator de escala uniforme da nave
    float velocidadeAtual;  // Escalares de velocidade linear
    float velocidadeMax;
    float aceleracao;
    bool acelerando;        // Estado visual das turbinas de plasma
    bool turbo;             // Estado do pós-combustor (Shift pressionado)
    bool freando;           // Estado do freio espacial (Ctrl pressionado)
};

// ----------------------------------------------------------------------------
// ESTADO GLOBAL ENCAPSULADO DO JOGO
// ----------------------------------------------------------------------------

struct JogoEstado {
    Nave3D nave;
    Camera3D camera;
    ModoCamera modoCamera = ModoCamera::Chase;

    bool inverterPitch = true;
    bool mostrarHUD_CG = true;

    int pontuacao = 0;
    int ondaAtual = 1;

    float tempoAnimacao      = 0.0f;
    float alertaTemporizador = 0.0f;
    float cameraTrauma       = 0.0f; // Intensidade do Screen Shake na Matriz de Visualização
    std::string alertaTexto  = "";

    std::vector<Projetil3D> tiros;
    std::vector<Particula3D> particulas;
    std::vector<Alvo3D> alvos;
    std::vector<AnelCheckpoint> aneis;
    std::vector<Estrela3D> estrelas;
};

// ----------------------------------------------------------------------------
// PROTÓTIPOS DE INICIALIZAÇÃO E MATEMÁTICA AUXILIAR (src/core/entidades.cpp)
// ----------------------------------------------------------------------------

// Gera um número em ponto flutuante pseudoaleatório no intervalo [minVal, maxVal]
float AleatorioFloat(float minVal, float maxVal);

// Gera um vetor 3D em direção aleatória utilizando coordenadas esféricas (theta, phi)
Vector3 VetorAleatorio3D(float intensidade);

// Calcula a base ortonormal da nave (vetores Forward, Up e Right) a partir dos ângulos de Euler
void CalcularOrientacaoNave(const Nave3D &nave, Vector3 &outFwd, Vector3 &outUp, Vector3 &outRight);

// Inicializa o campo estelar distribuído em uma casca esférica
void InicializarEstrelas(JogoEstado &jogo);

// Posiciona os alvos 3D da onda atual distribuídos cilindricamente/esfericamente na arena
void InicializarAlvos(JogoEstado &jogo);

// Posiciona os anéis de checkpoint ao redor da arena
void InicializarAneis(JogoEstado &jogo);

// Restaura a nave, câmera, placar e entidades do mundo para o estado inicial
void ReiniciarJogoCompleto(JogoEstado &jogo);

#endif // ENTIDADES_H
