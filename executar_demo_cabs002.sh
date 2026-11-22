#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"

exe="LEGATECH_Core/bin/Release/LEGATECH_Core"
tuss22="data/reference/tuss22_202501.csv"
tuss63="data/reference/tuss63_202501.csv"
db="data/sample/entrada_sqlite_demo.db"

echo "LEGATECH Core - CABS002 - demonstracao Ubuntu"
echo "1 - CSV de amostra (10 registros)"
echo "2 - CSV de validacao (8 registros)"
echo "3 - SQLite de amostra (10 registros)"
read -r -p "Selecione 1, 2 ou 3: " opcao

case "$opcao" in
  1)
    modo="csv_amostra"
    entrada="data/sample/PR_202501_AMB_DET_sample.csv"
    args=("$entrada" "$tuss22" "$tuss63")
    ;;
  2)
    modo="csv_validacao"
    entrada="data/sample/PR_202501_AMB_DET_validacao.csv"
    args=("$entrada" "$tuss22" "$tuss63")
    ;;
  3)
    modo="sqlite_amostra"
    python3 preparar_sqlite_demo.py || exit $?
    args=(--sqlite "$db" 202501 "$tuss22" "$tuss63")
    ;;
  *)
    echo "Opcao invalida."
    exit 2
    ;;
esac

args+=(--catalogo-tuss 202501)

if [[ ! -x "$exe" ]]; then
  echo "Executavel Release nao encontrado: $exe"
  exit 1
fi

mkdir -p logs
timestamp=$(date '+%Y%m%d_%H%M%S')
saida="data/sample/saida_demo_${modo}_${timestamp}"
log="logs/execucao_demo_${modo}_${timestamp}.log"
commit=$(git rev-parse --short HEAD 2>/dev/null || echo "indisponivel")

{
  echo "LEGATECH Core - CABS002"
  echo "Data/hora: $(date '+%d/%m/%Y %H:%M:%S')"
  echo "Modo: $modo"
  echo "Saida: $saida"
  echo "Commit: $commit"
  echo
} | tee "$log"

"$exe" "${args[@]}" --export "$saida" 2>&1 | tee -a "$log"
rc=${PIPESTATUS[0]}

{
  echo
  echo "Exit code: $rc"
  echo "Log: $log"
  echo "Saida: $saida"
} | tee -a "$log"

exit "$rc"
