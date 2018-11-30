# Script para executar simuladores para vários traces e configurações, automaticamente.
# Também organizar, de forma automatica, os resultados das execuções
# em formato que possa ser transferido para planilha. - Os parâmetros de execução estão sempre na primeira linha e os resultados estão organizados em colunas.
clear
CONTINUE=S
i=1
> simulacoes.txt
while [ "$CONTINUE" != "N" ]
do
  # echo "Digite a tecnica de predicao" 
  read tecnica_pred
  # echo "Digite o trace a ser utilizado:"
  read trace
  if [[ "$tecnica_pred" == "nt" ]]; then
    echo -e "sim_$i\t$tecnica_pred\t$trace" >> simulacoes.txt
    ./simpred $tecnica_pred $trace >> simulacoes.txt 
  elif [[ "$tecnica_pred" == "cr" ]]; then
    # echo "Digite o numero de linhas"
    read num_linhas
    # echo "Digite m"
    read m
    # echo "Digite n"
    read n
    echo -e "sim_$i\t$tecnica_pred\t$trace\t$num_linhas\t$m\t$n" >> simulacoes.txt
    ./simpred $tecnica_pred $trace $num_linhas $m $n>> simulacoes.txt 
  else
    # echo "Digite o numero de linhas"
    read num_linhas
    echo -e "sim_$i\t$tecnica_pred\t$trace\t$num_linhas" >> simulacoes.txt
    ./simpred $tecnica_pred $trace $num_linhas >> simulacoes.txt 
  fi
  echo >> simulacoes.txt
  echo
  # echo "Deseja realizar mais simulações? [S/N]"
  read CONTINUE
  let "i++"
done
echo "Fim das simulações. A saída foi escrita no arquivo simulacoes.txt".
