from pathlib import Path
import csv
import os
import shutil
import sqlite3
import subprocess
import sys


def exigir(condicao, mensagem):
    if not condicao:
        raise AssertionError(f"Teste falhou: {mensagem}")


ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT / "LEGATECH_Core"

EXE = CORE / "bin" / "Release" / (
    "LEGATECH_Core.exe" if os.name == "nt" else "LEGATECH_Core"
)

ENTRADA = ROOT / "data" / "sample" / "PR_202501_AMB_DET_validacao.csv"
TUSS22 = ROOT / "data" / "reference" / "tuss22_202501.csv"
TUSS63 = ROOT / "data" / "reference" / "tuss63_202501.csv"
TUSS20 = ROOT / "data" / "reference" / "tuss20_202501.csv"

SAIDA = ROOT / "data" / "output" / "saida_regressao_auto"

ACEITOS = SAIDA / "aceitos.csv"
REJEITADOS = SAIDA / "rejeitados.csv"
CONTEXTO = SAIDA / "contexto.csv"
BANCO = SAIDA / "resultados.db"


def contar_csv(caminho):
    with caminho.open("r", encoding="utf-8-sig", newline="") as arquivo:
        linhas = list(csv.reader(arquivo, delimiter=";"))
    return max(0, len(linhas) - 1)


def main():
    print("=== LEGATECH Core - teste de regressao do fluxo ===")

    exigir(EXE.exists(), f"Executavel nao encontrado: {EXE}")
    exigir(ENTRADA.exists(), f"Massa controlada nao encontrada: {ENTRADA}")
    exigir(TUSS22.exists(), f"TUSS 22 nao encontrada: {TUSS22}")
    exigir(TUSS63.exists(), f"TUSS 63 nao encontrada: {TUSS63}")
    exigir(TUSS20.exists(), f"TUSS 20 nao encontrada: {TUSS20}")

    if SAIDA.exists():
        shutil.rmtree(SAIDA)

    ambiente = os.environ.copy()

    # Runtime do MinGW usado pelo Code::Blocks no Windows.
    if os.name == "nt":
        mingw = Path(
            ambiente.get(
                "LEGATECH_MINGW_BIN",
                r"C:\Program Files\CodeBlocks\MinGW\bin"
            )
        )

        if mingw.exists():
            ambiente["PATH"] = str(mingw) + os.pathsep + ambiente.get("PATH", "")

    comando = [
        str(EXE),
        str(ENTRADA),
        str(TUSS22),
        str(TUSS63),
        "--export",
        str(SAIDA),
    ]

    print("\nExecutando Core...")
    processo = subprocess.run(
        comando,
        cwd=CORE,
        env=ambiente,
        check=False
    )

    exigir(processo.returncode == 0, f"Core retornou RC={processo.returncode}")

    # -------------------------------------------------------
    # Artefatos
    # -------------------------------------------------------

    exigir(SAIDA.exists(), "pasta de exportacao nao foi criada")
    exigir(ACEITOS.exists(), "aceitos.csv nao foi criado")
    exigir(REJEITADOS.exists(), "rejeitados.csv nao foi criado")
    exigir(CONTEXTO.exists(), "contexto.csv nao foi criado")
    exigir(BANCO.exists(), "resultados.db nao foi criado")

    # CSV:
    # 2 VALIDOS + 1 VALIDO_COM_ALERTA = 3 aceitos
    exigir(contar_csv(ACEITOS) == 3, "aceitos.csv deveria conter 3 registros")
    exigir(contar_csv(REJEITADOS) == 4, "rejeitados.csv deveria conter 4 registros")
    exigir(contar_csv(CONTEXTO) == 1, "contexto.csv deveria conter 1 execucao")

    # -------------------------------------------------------
    # SQLite
    # -------------------------------------------------------

    with sqlite3.connect(BANCO) as banco:

        execucao = banco.execute(
            """
            select
                total,
                validos,
                alertas,
                rejeitados,
                estrutura_divergente
            from auditoria_execucao
            """
        ).fetchone()

        exigir(execucao is not None, "auditoria_execucao nao encontrada")

        exigir(
            execucao == (7, 2, 1, 4, 1),
            f"totais inesperados: {execucao}"
        )

        estados = dict(
            banco.execute(
                """
                select estado, count(*)
                from auditoria_resultado
                group by estado
                """
            ).fetchall()
        )

        exigir(estados.get("VALIDO") == 2, "esperados 2 VALIDOS")
        exigir(
            estados.get("VALIDO_COM_ALERTA") == 1,
            "esperado 1 VALIDO_COM_ALERTA"
        )
        exigir(estados.get("REJEITADO") == 4, "esperados 4 REJEITADOS")

        # ---------------------------------------------------
        # Caso crítico:
        # REJEITADO preservando ALERTA + MOTIVO
        # ---------------------------------------------------

        estado_005 = banco.execute(
            """
            select estado
            from auditoria_resultado
            where id_evento = '990000000005'
            """
        ).fetchone()

        exigir(
            estado_005 == ("REJEITADO",),
            "990000000005 deveria estar REJEITADO"
        )

        ocorrencias_005 = banco.execute(
            """
            select tipo, mensagem
            from auditoria_ocorrencia o
            join auditoria_resultado r
              on r.id = o.resultado_id
            where r.id_evento = '990000000005'
            order by o.ordem
            """
        ).fetchall()

        tem_alerta = any(
            tipo == "ALERTA" and "PAR_TUSS" in mensagem
            for tipo, mensagem in ocorrencias_005
        )

        tem_motivo = any(
            tipo == "MOTIVO" and "IND_PACOTE" in mensagem
            for tipo, mensagem in ocorrencias_005
        )

        exigir(
            tem_alerta,
            "990000000005 deveria preservar ALERTA PAR_TUSS"
        )

        exigir(
            tem_motivo,
            "990000000005 deveria possuir MOTIVO IND_PACOTE"
        )

        # ---------------------------------------------------
        # Tabela própria contextual
        # ---------------------------------------------------

        estado_006 = banco.execute(
            """
            select estado
            from auditoria_resultado
            where id_evento = '990000000006'
            """
        ).fetchone()

        exigir(
            estado_006 == ("VALIDO",),
            "990000000006 deveria permanecer VALIDO"
        )

        # ---------------------------------------------------
        # Estrutura divergente
        # ---------------------------------------------------

        divergente = banco.execute(
            """
            select count(*)
            from auditoria_ocorrencia
            where mensagem = 'ESTRUTURA_DIVERGENTE_DO_CONTRATO'
            """
        ).fetchone()[0]

        exigir(
            divergente == 1,
            "esperada 1 ocorrencia de estrutura divergente"
        )

    print()
    print("==========================================")
    print("PASS - regressao do fluxo validada")
    print("==========================================")
    print("Total:                 7")
    print("VALIDO:                2")
    print("VALIDO_COM_ALERTA:     1")
    print("REJEITADO:             4")
    print("Estrutura divergente:  1")
    print("CSV:                    OK")
    print("SQLite:                 OK")
    print("Alerta + motivo:        OK")
    print("Tabela propria:         OK")


if __name__ == "__main__":
    try:
        main()
    except Exception as erro:
        print()
        print(f"FAIL - {erro}", file=sys.stderr)
        sys.exit(1)