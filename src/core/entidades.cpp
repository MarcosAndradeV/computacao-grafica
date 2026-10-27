// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/core/entidades.cpp
//
// Descrição:
//   Funções matemáticas de orientação 3D, geração procedural (coordenadas
//   esféricas/cilíndricas) e inicialização das entidades do mundo.
//
// Conceitos de CG relacionados:
//   - Composição de matrizes de rotação 3D
//   - Conversão de coordenadas esféricas (r, θ, φ) para cartesianas (x, y, z)
//   - Interseção segmento × esfera (ponto mais próximo sobre o segmento)
// ============================================================================

#include "core/entidades.h"
#include "raymath.h"

#include <cmath>
#include <cstdlib>

// ----------------------------------------------------------------------------
// MATEMÁTICA AUXILIAR
// ----------------------------------------------------------------------------

float AleatorioFloat(float minVal, float maxVal) {
    return minVal + ((float)std::rand() / (float)RAND_MAX) * (maxVal - minVal);
}

// Esféricas -> cartesianas, com azimute θ e elevação φ:
//   x = r·cos(φ)·cos(θ)   y = r·sin(φ)   z = r·cos(φ)·sin(θ)
Vector3 VetorAleatorio3D(float intensidade) {
    float theta = AleatorioFloat(0.0f, 2.0f * PI);
    float phi   = AleatorioFloat(-PI * 0.5f, PI * 0.5f);
    return Vector3{
        intensidade * std::cos(phi) * std::cos(theta),
        intensidade * std::sin(phi),
        intensidade * std::cos(phi) * std::sin(theta)
    };
}

// Orientação da nave: transforma os eixos locais (Z+ frente, Y+ cima, X+ direita)
// pela matriz de rotação composta dos ângulos de Euler.
//
// Nota sobre convenções (as duas notações descrevem a MESMA transformação):
//   - O raymath usa vetores-linha: MatrixMultiply(A, B) aplica A primeiro e depois B.
//     Aqui: Rz(roll), depois Rx(-pitch), depois Ry(yaw).
//   - O OpenGL/rlgl usa vetores-colunas: a matriz M = T·Ry·Rx·Rz·S aplica Rz primeiro.
//     É a ordem das chamadas rlRotatef em cena_3d.cpp e a fórmula exibida no HUD.
void CalcularOrientacaoNave(const Nave3D &nave, Vector3 &outFwd, Vector3 &outUp, Vector3 &outRight) {
    Matrix matRot = MatrixMultiply(
        MatrixRotateZ(nave.roll * DEG2RAD),
        MatrixMultiply(MatrixRotateX(-nave.pitch * DEG2RAD), MatrixRotateY(nave.yaw * DEG2RAD))
    );

    outFwd   = Vector3Normalize(Vector3Transform(Vector3{0.0f, 0.0f, 1.0f}, matRot));
    outUp    = Vector3Normalize(Vector3Transform(Vector3{0.0f, 1.0f, 0.0f}, matRot));
    outRight = Vector3Normalize(Vector3Transform(Vector3{1.0f, 0.0f, 0.0f}, matRot));
}

Vector3 ObterFrenteNave(const Nave3D &nave) {
    Vector3 frente, cima, direita;
    CalcularOrientacaoNave(nave, frente, cima, direita);
    return frente;
}

float FatorSuavizacao(float k, float dt) {
    return 1.0f - std::exp(-k * dt);
}

float FracaoRaioMundo(const Nave3D &nave) {
    return Vector3Length(nave.pos) / Config::RAIO_MUNDO;
}

// Projeta o centro da esfera sobre o segmento (parâmetro t limitado a [0, 1]) e
// compara a distância ao ponto mais próximo com o raio.
bool SegmentoInterceptaEsfera(Vector3 a, Vector3 b, Vector3 centro, float raio) {
    Vector3 ab = Vector3Subtract(b, a);
    float comprimentoQuadrado = Vector3DotProduct(ab, ab);

    float t = 0.0f;
    if (comprimentoQuadrado > 1e-8f) {
        t = Clamp(Vector3DotProduct(Vector3Subtract(centro, a), ab) / comprimentoQuadrado, 0.0f, 1.0f);
    }

    Vector3 maisProximo = Vector3Add(a, Vector3Scale(ab, t));
    return Vector3DistanceSqr(maisProximo, centro) < raio * raio;
}

Camera3D ObterCameraRender(const JogoEstado &jogo) {
    Camera3D cam = jogo.camera;
    cam.position = Vector3Add(cam.position, jogo.tremorCamera);
    cam.target   = Vector3Add(cam.target, jogo.tremorCamera);
    return cam;
}

// ----------------------------------------------------------------------------
// INICIALIZAÇÃO PROCEDURAL DO MUNDO
// ----------------------------------------------------------------------------

void InicializarEstrelas(JogoEstado &jogo) {
    jogo.estrelas.clear();
    jogo.estrelas.reserve(Config::TOTAL_ESTRELAS);

    for (int i = 0; i < Config::TOTAL_ESTRELAS; ++i) {
        // Casca esférica distante (350 a 650 unidades), além da cúpula da arena
        float r     = AleatorioFloat(350.0f, 650.0f);
        float theta = AleatorioFloat(0.0f, 2.0f * PI);
        float phi   = AleatorioFloat(-PI * 0.48f, PI * 0.48f);

        Vector3 pos = {
            r * std::cos(phi) * std::cos(theta),
            r * std::sin(phi),
            r * std::cos(phi) * std::sin(theta)
        };

        Color cor = WHITE;
        switch (std::rand() % 5) {
        case 0: cor = SKYBLUE; break;
        case 1: cor = Color{200, 220, 255, 255}; break;
        case 2: cor = Color{255, 230, 180, 255}; break;
        case 3: cor = Color{255, 180, 180, 255}; break;
        default: break;
        }

        jogo.estrelas.push_back({pos, cor, AleatorioFloat(0.6f, 1.6f), AleatorioFloat(0.0f, 2.0f * PI)});
    }
}

void InicializarAlvos(JogoEstado &jogo) {
    jogo.alvos.clear();

    // Cada onda concluída acrescenta 3 alvos
    const int qtd = 12 + (jogo.ondaAtual - 1) * 3;
    jogo.alvos.reserve(qtd);

    for (int i = 0; i < qtd; ++i) {
        Alvo3D alvo;

        // Posição em coordenadas esféricas: azimute espalhado uniformemente (com
        // pequeno ruído) e elevação limitada para evitar alvos muito baixos.
        float theta    = ((float)i / (float)qtd) * 2.0f * PI + AleatorioFloat(-0.25f, 0.25f);
        float phi      = AleatorioFloat(-PI * 0.25f, PI * 0.35f);
        float raioDist = AleatorioFloat(50.0f, 175.0f);

        alvo.pos = Vector3{
            raioDist * std::cos(phi) * std::cos(theta),
            raioDist * std::sin(phi) + AleatorioFloat(5.0f, 35.0f),  // Deslocamento para cima
            raioDist * std::cos(phi) * std::sin(theta)
        };

        alvo.rotAngulo = Vector3{AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f)};
        alvo.rotVel    = Vector3{AleatorioFloat(-45.0f, 45.0f), AleatorioFloat(40.0f, 90.0f), AleatorioFloat(-30.0f, 30.0f)};
        alvo.escalaBase = AleatorioFloat(1.6f, 2.6f);
        alvo.tipo       = static_cast<TipoAlvo>(i % 3);

        switch (alvo.tipo) {
        case TipoAlvo::DroneSentinela:      alvo.cor = Color{255, 80, 80, 255};  break;  // Vermelho neon
        case TipoAlvo::AsteroideCristalino: alvo.cor = Color{60, 255, 160, 255}; break;  // Verde esmeralda
        case TipoAlvo::PrismaEnergia:       alvo.cor = Color{255, 215, 0, 255};  break;  // Dourado
        }

        alvo.ativo = true;
        jogo.alvos.push_back(alvo);
    }
}

void InicializarAneis(JogoEstado &jogo) {
    jogo.aneis.clear();
    jogo.aneis.reserve(Config::TOTAL_ANEIS);

    for (int i = 0; i < Config::TOTAL_ANEIS; ++i) {
        float ang  = ((float)i / (float)Config::TOTAL_ANEIS) * 2.0f * PI;
        float dist = AleatorioFloat(85.0f, 150.0f);
        float alt  = AleatorioFloat(10.0f, 70.0f);

        AnelCheckpoint anel;
        anel.pos      = Vector3{std::cos(ang) * dist, alt, std::sin(ang) * dist};
        anel.normal   = Vector3{std::cos(ang), 0.0f, std::sin(ang)};  // Radial: o portão "olha" para o centro
        anel.rotY     = -ang * RAD2DEG;  // Girar o eixo X do modelo até a direção radial (ver cena_3d.cpp)
        anel.raio     = 6.5f;
        anel.coletado = false;
        jogo.aneis.push_back(anel);
    }
}

void ReiniciarJogoCompleto(JogoEstado &jogo) {
    Nave3D &nave = jogo.nave;
    nave.pos             = Vector3{0.0f, 0.0f, -15.0f};
    nave.vel             = Vector3{0.0f, 0.0f, 0.0f};
    nave.pitch           = 0.0f;
    nave.yaw             = 0.0f;
    nave.roll            = 0.0f;
    nave.rollDesejado    = 0.0f;
    nave.escala          = 1.0f;
    nave.velocidadeAtual = Config::VELOCIDADE_INICIAL;
    nave.acelerando      = false;
    nave.turbo           = false;
    nave.freando         = false;

    // Câmera sintética inicial (Matriz de Visualização + Projeção Perspectiva)
    jogo.camera.position   = Vector3{0.0f, 6.0f, -28.0f};
    jogo.camera.target     = Vector3{0.0f, 0.0f, 0.0f};
    jogo.camera.up         = Vector3{0.0f, 1.0f, 0.0f};
    jogo.camera.fovy       = Config::FOV_PADRAO;
    jogo.camera.projection = CAMERA_PERSPECTIVE;
    jogo.tremorCamera      = Vector3{0.0f, 0.0f, 0.0f};

    jogo.pontuacao    = 0;
    jogo.ondaAtual    = 1;
    jogo.cameraTrauma = 0.0f;
    jogo.cooldownTiro = 0.0f;

    jogo.tiros.clear();
    jogo.particulas.clear();

    InicializarAlvos(jogo);
    InicializarAneis(jogo);
    InicializarEstrelas(jogo);

    jogo.alertaTexto        = "COMBATE INICIADO! DESTRUA OS ALVOS 3D";
    jogo.alertaTemporizador = 3.5f;
}
