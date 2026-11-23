#!/bin/bash
# Testes automatizados: compila o programa e confere saidas esperadas.
# Uso: ./testes.sh
cd "$(dirname "$0")" || exit 1
gcc -std=c99 -Wall -Wextra -pedantic -o clinica clinica.c || exit 1

ok=0; falhas=0
# verificar "descricao" "entrada (uma linha por token)" "texto esperado na saida"
verificar() {
    saida=$(printf '%s\n' $2 | ./clinica 2>&1)
    if grep -qF -- "$3" <<< "$saida"; then
        echo "OK     $1"; ok=$((ok+1))
    else
        echo "FALHOU $1 (esperado: $3)"; falhas=$((falhas+1))
    fi
}
# Cada cadastro usa: 1 <genero> <idade> [gestante lactante se F] <especial> <obeso>
verificar "Homem 30 anos -> Normal"            "1 M 30 n n 0"          "N001 - categoria Normal"
verificar "Homem 64 anos -> Normal"            "1 M 64 n n 0"          "N001 - categoria Normal"
verificar "Homem 65 anos -> Preferencial"      "1 M 65 n n 0"          "P001 - categoria Preferencial"
verificar "Mulher 59 anos -> Normal"           "1 F 59 n n n n 0"      "N001 - categoria Normal"
verificar "Mulher 60 anos -> Preferencial"     "1 F 60 n n n n 0"      "P001 - categoria Preferencial"
verificar "Mulher 59 lactante -> Preferencial" "1 F 59 n s n n 0"      "P001 - categoria Preferencial"
verificar "Gestante -> Preferencial"           "1 F 25 s n n n 0"      "P001 - categoria Preferencial"
verificar "Necessidade especial -> Pref."      "1 M 20 s n 0"          "P001 - categoria Preferencial"
verificar "Obeso -> Preferencial"              "1 M 20 n s 0"          "P001 - categoria Preferencial"
verificar "80 anos -> Preferencial (nao U)"    "1 M 80 n n 0"          "P001 - categoria Preferencial"
verificar "81 anos -> Urgente"                 "1 M 81 n n 0"          "U001 - categoria Urgente"
verificar "Gestante com 85 anos -> Urgente"    "1 F 85 s n n n 0"      "U001 - categoria Urgente"
verificar "Numeracao independente"             "1 M 20 n n 1 M 21 n n 0" "N002 - categoria Normal"
verificar "Urgente chamado antes de Normal"    "1 M 20 n n 1 M 85 n n 3 0" "Chamando senha U001"
verificar "Preferencial antes de Normal"       "1 M 20 n n 1 M 65 n n 3 0" "Chamando senha P001"
verificar "FIFO na mesma categoria"            "1 M 20 n n 1 M 21 n n 3 4 3 0" "Chamando senha N002"
verificar "Nao chama com atendimento aberto"   "1 M 20 n n 1 M 21 n n 3 3 0" "Ja existe atendimento"
verificar "Finalizar sem atendimento"          "4 0"                   "Nao ha atendimento em andamento"
verificar "Chamar com filas vazias"            "3 0"                   "Nao ha senhas aguardando"
verificar "Consultar proxima senha"            "1 M 85 n n 2 0"        "Proxima senha a ser chamada: U001"
verificar "Painel mostra atendidos"            "1 M 20 n n 3 4 5 0"    "N=1"
verificar "Idade com texto e recusada"         "1 M abc 30 n n 0"      "digite um numero inteiro"
verificar "Idade negativa e recusada"          "1 M -5 30 n n 0"       "fora do intervalo"
verificar "Genero invalido e recusado"         "1 X M 30 n n 0"        "Resposta invalida"
verificar "Resposta S/N invalida"              "1 M 30 talvez n n 0"   "Resposta invalida"
verificar "Opcao de menu invalida"             "9 0"                   "fora do intervalo"

echo; echo "Resultado: $ok passaram, $falhas falharam."
rm -f clinica
[ "$falhas" -eq 0 ]
