# Sistema de Gerenciamento de Senhas de Atendimento Clínico

Aplicação de console em **linguagem C** que informatiza a triagem e a organização das filas de uma clínica de especialidades médicas, identificando automaticamente os pacientes com prioridade de chamada.

> **Aviso:** as categorias e faixas etárias são **regras didáticas** definidas para esta atividade. Não reproduzem a legislação de atendimento prioritário nem substituem a classificação de risco ou a avaliação clínica.

## Objetivo

Permitir que o atendente cadastre pacientes, emita senhas sequenciais classificadas por categoria, controle a ordem de chamada e acompanhe o atendimento por meio de um painel textual.

## Regras de classificação

| Código | Categoria    | Condição |
|--------|--------------|----------|
| `U001` | Urgente      | Idade **superior a 80 anos** |
| `P001` | Preferencial | Gestante, lactante, necessidade especial, obesidade, mulher com **60 anos ou mais**, homem com **65 anos ou mais** |
| `N001` | Normal       | Sem nenhuma condição de prioridade |

- A numeração é **independente por categoria** (N001, N002…; P001, P002…; U001, U002…).
- Se o paciente se enquadra em mais de uma regra, vale a de maior precedência: **Urgente > Preferencial > Normal**.
- A ordem de chamada segue essa precedência; dentro de cada categoria vale a ordem de emissão (**FIFO**).
- Uma senha atendida sai da fila definitivamente e não pode retornar.

## Funcionalidades

1. **Emitir nova senha:** coleta gênero, idade, gestante/lactante (apenas para o gênero feminino), necessidade especial e obesidade; classifica e emite a senha.
2. **Consultar próxima senha:** mostra quem será chamado, sem retirá-lo da fila.
3. **Chamar próximo paciente:** retira da fila a senha de maior precedência e a coloca em atendimento.
4. **Finalizar atendimento:** conclui o atendimento atual e libera o guichê para a próxima chamada.
5. **Painel de chamada:** exibe a senha em atendimento e sua categoria, as próximas 5 senhas na ordem de chamada, a quantidade de pacientes em espera por categoria e os atendimentos concluídos.

### Validações e tratamento de erros

- Menu e idade aceitam apenas inteiros no intervalo permitido (menu: 0–5; idade: 0–130). Texto, números negativos ou fora da faixa são recusados e o dado é pedido novamente.
- Gênero aceita apenas `M` ou `F`; perguntas de sim/não aceitam apenas `S` ou `N` (maiúsculas ou minúsculas).
- Não é possível chamar um paciente enquanto outro estiver em atendimento.
- Chamar ou consultar com as filas vazias, ou finalizar sem atendimento em andamento, exibe uma mensagem em vez de falhar.
- Cada fila comporta até 500 senhas em espera, e a numeração vai até 999 por categoria. Ao atingir o limite, a emissão é recusada com aviso.
- Se a entrada de dados for encerrada (EOF/Ctrl+D), o programa finaliza sem travar.

## Estruturas de dados

- `Paciente`: gênero, idade e as condições de prioridade.
- `Senha`: categoria, número sequencial e dados do paciente.
- `Fila`: fila circular (FIFO) em vetor de tamanho fixo.
- `Sistema`: três filas (uma por categoria), contadores de numeração, contadores de atendidos e a senha em atendimento.

## Compilação e execução

Requer um compilador C com suporte a C99 (por exemplo, GCC ou Clang).

```bash
# Compilar
gcc -std=c99 -Wall -Wextra -pedantic -o clinica clinica.c

# Executar (Linux/macOS)
./clinica

# Executar (Windows)
clinica.exe
```

## Funções implementadas

| Módulo | Função | Responsabilidade |
|--------|--------|------------------|
| Entrada | `lerLinha` | Lê uma linha com `fgets`, descarta excedente e trata EOF |
| Entrada | `lerInteiro` | Lê e valida um inteiro dentro de um intervalo |
| Entrada | `lerSimNao` | Lê e valida resposta S/N |
| Entrada | `lerGenero` | Lê e valida M/F |
| 1 – Registro | `cadastrarPaciente` | Coleta os dados do paciente |
| 1 – Registro | `emitirSenha` | Gera o número sequencial da categoria e enfileira a senha |
| 2 – Classificação | `temPrioridade` | Verifica as condições de prioridade (gestante, idade, etc.) |
| 2 – Classificação | `classificarPaciente` | Define a categoria: Urgente, Preferencial ou Normal |
| 2 – Classificação | `prefixoCategoria`, `nomeCategoria` | Letra (U/P/N) e nome da categoria |
| 2 – Classificação | `formatarSenha` | Monta o código da senha (ex.: `P002`) |
| Filas | `inicializarSistema` | Zera filas e contadores |
| Filas | `filaVazia`, `filaCheia` | Consultam o estado da fila |
| Filas | `enfileirar`, `desenfileirar` | Inserem no fim e removem do início (FIFO) |
| Filas | `espiarFila` | Consulta uma posição sem remover |
| Filas | `totalEmEspera` | Total de pacientes aguardando |
| Filas | `proximaSenha` | Localiza a próxima senha respeitando a precedência |
| 3 – Atendimento | `consultarProximaSenha` | Mostra a próxima senha |
| 3 – Atendimento | `chamarProximoPaciente` | Chama o próximo paciente |
| 3 – Atendimento | `finalizarAtendimento` | Conclui o atendimento atual |
| 4 – Painel | `exibirPainel` | Mostra o painel de chamada |
| Interface | `exibirMenu`, `registrarNovaSenha`, `executarMenu` | Menu interativo e despacho das opções |

A função `main()` apenas cria o sistema e inicia o menu.

## Exemplos de utilização

### Emissão de senhas

```
Escolha uma opcao: 1
Genero do paciente (M/F): F
Idade (anos): 25
Esta gestante? (S/N): S
E lactante? (S/N): N
Possui necessidade especial? (S/N): N
Possui condicao de obesidade? (S/N): N

Senha emitida: P001 - categoria Preferencial
```

```
Escolha uma opcao: 1
Genero do paciente (M/F): M
Idade (anos): 85
Possui necessidade especial? (S/N): N
Possui condicao de obesidade? (S/N): N

Senha emitida: U001 - categoria Urgente
```

### Painel de chamada

Com as senhas N001, P001, U001, P002 (homem de 65 anos) e N002 emitidas, nesta ordem:

```
==============================================
            PAINEL DE CHAMADA
==============================================
 Senha em atendimento: nenhuma
----------------------------------------------
 Proximas senhas (ordem de chamada):
   1. U001 (Urgente)
   2. P001 (Preferencial)
   3. P002 (Preferencial)
   4. N001 (Normal)
   5. N002 (Normal)
----------------------------------------------
 Pacientes em espera por categoria:
   Urgente      : 1
   Preferencial : 2
   Normal       : 2
   Total        : 5
----------------------------------------------
 Atendimentos concluidos: U=0  P=0  N=0
==============================================
```

### Chamada e finalização

```
Escolha uma opcao: 3
>>> Chamando senha U001 (Urgente) <<<

Escolha uma opcao: 3
Ja existe atendimento em andamento (senha U001). Finalize-o antes de chamar outro.

Escolha uma opcao: 4
Atendimento da senha U001 finalizado.

Escolha uma opcao: 3
>>> Chamando senha P001 (Preferencial) <<<
```

### Tratamento de entradas inválidas

```
Idade (anos): abc
  Entrada invalida: digite um numero inteiro.
Idade (anos): 200
  Valor fora do intervalo (0 a 130).
```

## Casos de teste de classificação

| Paciente | Senha |
|----------|-------|
| Homem, 64 anos, sem condições | Normal |
| Homem, 65 anos | Preferencial |
| Mulher, 59 anos, sem condições | Normal |
| Mulher, 59 anos, lactante | Preferencial |
| Mulher, 60 anos | Preferencial |
| Homem, 80 anos | Preferencial |
| Homem ou mulher, 81 anos | Urgente |
| Mulher, 30 anos, gestante | Preferencial |
| Mulher, 85 anos, gestante | Urgente |
