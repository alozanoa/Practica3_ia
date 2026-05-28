#!/bin/bash

# Configuración
EJECUTABLE="./n_en_rayaSH"
PROFUNDIDAD=7
NINJAS=(ninja1 ninja2 ninja3 ninja4)
LOG_DIR="logs_torneo"
mkdir -p "$LOG_DIR"

echo "=== TORNEO EN PARALELO ==="
echo "Combinaciones: ${#NINJAS[@]} ninjas × 2 posiciones = $((${#NINJAS[@]} * 2)) partidas"
echo ""

pids=()

for ninja in "${NINJAS[@]}"; do
    # Ninja como Jugador 1, inteligente como Jugador 2
    log="${LOG_DIR}/${ninja}_vs_inteligente.log"
    echo "Lanzando: $ninja (J1) vs inteligente (J2)"
    $EJECUTABLE -p1 "$ninja" -p2 inteligente -d $PROFUNDIDAD -id2 1 -nogui \
        > "$log" 2>&1 &
    pids+=($!)

    # Inteligente como Jugador 1, ninja como Jugador 2
    log="${LOG_DIR}/inteligente_vs_${ninja}.log"
    echo "Lanzando: inteligente (J1) vs $ninja (J2)"
    $EJECUTABLE -p1 inteligente -p2 "$ninja" -d $PROFUNDIDAD -id2 2 -nogui \
        > "$log" 2>&1 &
    pids+=($!)
done

echo ""
echo "--- Esperando a que terminen ${#pids[@]} partidas... ---"

# Esperar a todos y recoger resultados
errores=0
for i in "${!pids[@]}"; do
    wait "${pids[$i]}"
    status=$?
    if [ $status -ne 0 ]; then
        echo "  [WARN] Proceso $i (PID ${pids[$i]}) terminó con código $status"
        ((errores++))
    fi
done

echo ""
echo "=== RESULTADOS ==="
victorias_inteligente=0
derrotas_inteligente=0
empates=0

for log in "$LOG_DIR"/*.log; do
    nombre=$(basename "$log" .log)
    resultado=$(grep -E "PARTIDA FINALIZADA" "$log" | tail -1)

    if [ -n "$resultado" ]; then
        echo "  [$nombre] $resultado"

        # Determinar quién ganó según el nombre del log y el resultado
        if echo "$resultado" | grep -q "Empate"; then
            ((empates++))
        elif echo "$nombre" | grep -q "^inteligente_vs_"; then
            # inteligente es J1 → gana si "Jugador 1"
            if echo "$resultado" | grep -q "Jugador 1"; then
                ((victorias_inteligente++))
            else
                ((derrotas_inteligente++))
            fi
        else
            # inteligente es J2 → gana si "Jugador 2"
            if echo "$resultado" | grep -q "Jugador 2"; then
                ((victorias_inteligente++))
            else
                ((derrotas_inteligente++))
            fi
        fi
    else
        echo "  [$nombre] (sin resultado — revisa $log)"
    fi
done

echo ""
echo "=== MARCADOR FINAL (inteligente) ==="
echo "  Victorias : $victorias_inteligente"
echo "  Derrotas  : $derrotas_inteligente"
echo "  Empates   : $empates"
echo ""
if [ $errores -eq 0 ]; then
    echo "Todas las partidas terminaron sin errores."
else
    echo "$errores partida(s) terminaron con error. Revisa $LOG_DIR/"
fi
