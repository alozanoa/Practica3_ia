#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo) 
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false) {
    nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const {
    return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero& tablero) {
    std::pair<int, int> mejor;
    nodosVisitados = 0;
    abortarBanda = false;
    inicioBusqueda = std::chrono::steady_clock::now();

    switch (modo)
    {
    case ModoJuego::ALEATORIO:
        return JuegaAleatorio(tablero);
        break;
    
    case ModoJuego::STATUS:
        Status(tablero, mejor);
        return mejor;
        break;    

    case ModoJuego::MINIMAX:
        minimax(tablero, 0, profundidadMax, mejor);
        return mejor;
        break; 

    case ModoJuego::INTELIGENTE:
        return JuegaInteligente(tablero);   
        break;
    }
        
    return {-1, -1};
}


/**
 * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
 * @param padre Estado inicial del tablero.
 * @param hijo Estado resultante tras un movimiento.
 * @return Un par (fila, columna) con la posición de la nueva pieza.
 */
std::pair<int, int> SacarMovimiento(const Tablero& padre, const Tablero &hijo){
    for(int f=0; f<padre.getFilas(); ++f)
        for(int c=0; c<padre.getColumnas(); ++c)
            if (padre.getCelda(f,c) == 0 && hijo.getCelda(f,c) != 0) 
                return {f, c};
    return {-1, -1};
}

/**
 * @brief Implementa un agente que juega de forma totalmente aleatoria.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida al azar.
 */
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero& tablero) {

    // Calculo los tableros descendientes de tablero
    auto sucesores = tablero.getSucesores();

    // Si no tiene descendientes, paso el turno
    if (sucesores.empty()) return {-1, -1};

    // Elijo aleatoriamente uno de los descendientes
    int elegido = rand() % sucesores.size();

    // Saco el movimiento realizado comparando el tablero original con el elegido.
    std::pair<int,int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

    return Mov;
}


/**
 * @brief Algoritmo de resolución completa para estados de final de juego.
 * Determina si una posición está matemáticamente ganada, perdida o empatada.
 * @param tablero Estado a evaluar.
 * @param Mov [Salida] La jugada óptima encontrada.
 * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
 */
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    auto sucesores = tablero.getSucesores();

    if(sucesores.empty()) {
        if(tablero.comprobarGanador() == id) {
            return Resultado::VICTORIA;
        }
        else if(tablero.comprobarGanador() == 0) {
            return Resultado::EMPATE;
        }
        else {
            return Resultado::DERROTA;
        }
    }

    Resultado mejor ;
    if(tablero.getJugadorTurno() == id) {
        mejor = Resultado::DERROTA; // El peor resultado posible para MAX
    }
    else {
        mejor = Resultado::VICTORIA; // El peor resultado posible para MIN
    }
    std::pair<int,int> mejorMov = {-1,-1};
    std::pair<int,int> vacio;

    while (!sucesores.empty()) {

        auto hijo = sucesores.back();
        sucesores.pop_back();

        Resultado resactual = Status(hijo, vacio);
        auto movimiento = SacarMovimiento(tablero, hijo);

        if (tablero.getJugadorTurno() == id) {

            if (resactual == Resultado::VICTORIA) {
                Mov = movimiento;
                return Resultado::VICTORIA;
            }

            if (resactual == Resultado::EMPATE && mejor != Resultado::VICTORIA) {
                mejor = Resultado::EMPATE;
                mejorMov = movimiento;
            }

            if (resactual == Resultado::DERROTA && mejor == Resultado::DERROTA) {
                mejorMov = movimiento;
            }

        } else {

            if (resactual == Resultado::DERROTA) {
                Mov = movimiento;
                return Resultado::DERROTA;
            }

            if (resactual == Resultado::EMPATE && mejor != Resultado::DERROTA) {
                mejor = Resultado::EMPATE;
                mejorMov = movimiento;
            }

            if (resactual == Resultado::VICTORIA && mejor == Resultado::VICTORIA) {
                mejorMov = movimiento;
            }
        }
    }

    Mov = mejorMov;
    return mejor;
}



/**
 * @brief Implementación del algoritmo Minimax clásico.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado.
 */
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

        auto sucesores = tablero.getSucesores();

    // corte por profundidad
    if (profundidad == prof_Max || sucesores.empty()) {
        return heuristica(tablero);
    }

    bool esMax = (tablero.getJugadorTurno() == id);

    double mejorValor = esMax ? -100000.0 : 100000.0;
    std::pair<int,int> mejorMov = {-1,-1};
    std::pair<int,int> vacio;

    while (!sucesores.empty()) {

        auto hijo = sucesores.back();
        sucesores.pop_back();

        auto movimiento = SacarMovimiento(tablero, hijo);

        double valor = minimax(hijo, profundidad + 1, prof_Max, vacio);

        if (esMax) {
            if (valor > mejorValor) {
                mejorValor = valor;
                mejorMov = movimiento;
            }

        } else {
            if (valor < mejorValor) {
                mejorValor = valor;
                mejorMov = movimiento;
            }
        }
    }

    Mov = mejorMov;
    return mejorValor;
}


/**
 * @brief Punto de entrada para el juego inteligente.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida por el algoritmo de búsqueda.
 */
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero& tablero) {
    std::pair<int,int> Mov;

    double valor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
    std::cout << "Valor Minimax: " << valor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
    return Mov;
}


/**
 * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param alfa Valor mínimo garantizado para el jugador MAX.
 * @param beta Valor máximo garantizado para el jugador MIN.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado tras la poda.
 */
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    auto sucesores = tablero.getSucesores();

    // Caso base: profundidad límite o sin movimientos
    if (profundidad == prof_Max || sucesores.empty()) {
        return heuristica(tablero);
    }

    bool esMax = (tablero.getJugadorTurno() == id);

    double mejorValor = esMax ? -100000.0 : 100000.0;
    std::pair<int,int> mejorMov = {-1, -1};
    std::pair<int,int> vacio;

    while (!sucesores.empty()) {

        auto hijo = sucesores.back();
        sucesores.pop_back();

        auto movimiento = SacarMovimiento(tablero, hijo);

        double valor = alfaBeta(hijo, profundidad + 1, prof_Max, alfa, beta, vacio);

        if (esMax) {
            if (valor > mejorValor) {
                mejorValor = valor;
                mejorMov = movimiento;
            }
            alfa = std::max(alfa, mejorValor); // Actualizamos alfa
            if (alfa >= beta) break;           // Poda Beta: MIN nunca elegiría esta rama

        } else {
            if (valor < mejorValor) {
                mejorValor = valor;
                mejorMov = movimiento;
            }
            beta = std::min(beta, mejorValor); // Actualizamos beta
            if (alfa >= beta) break;           // Poda Alfa: MAX nunca elegiría esta rama
        }
    }

    Mov = mejorMov;
    return mejorValor;
}

/**
 * @brief Función heurística para evaluar la calidad de un tablero.
 * @param tablero Estado a evaluar.
 * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
 */
double AgenteEstudiante::heuristica(const Tablero& tablero) {
    switch(numHeuristica) {
        case 0: return heuristicaPrueba(tablero);
                break;
        case 1: return heuristica1(tablero);
                break;
        case 2: return heuristica2(tablero);
                break;
        default: return heuristica1(tablero);
    }
}

double AgenteEstudiante::heuristicaPrueba(const Tablero& tablero) {
    // n es el número de fichas en línea para ganar.
    int n = tablero.getNParaGanar();
    int oponente = (id == 1) ? 2 : 1;
    double score_positivo = 0;

    double score_negativo = 0;

    for (int f=0; f< tablero.getFilas(); f++ ){
        for (int c = 0; c< tablero.getColumnas(); c++){
            if (tablero.getCelda(f,c) != 0 ){
                int valor = tablero.getFilas()-abs(f-(tablero.getFilas()/2)) + tablero.getColumnas()-abs(c-(tablero.getColumnas()/2)); 
                if (tablero.getCelda(f,c) == id){
                  score_positivo += valor;
                 }
                else {
                  score_negativo += valor;
                }
            }
        }
    }

   
    return score_positivo - score_negativo;
}


double AgenteEstudiante::heuristica1(const Tablero& tablero) {
    //A implementar por el estudiante
return 0;
}

double AgenteEstudiante::heuristica2(const Tablero& tablero) {
    //A implementar por el estudiante
return 0;
}

