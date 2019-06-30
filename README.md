# ProxyGate

## Descrição

Programa que implementa um portão proxy para requests entre um browser e o computador. Permite o controle da passagem de requests, a inspeção e edição de campos da request, a criação da árvore de referências do website autor da request e o download da estrutura html do site autor da request. Trabalho final da disciplina Teleinformática e redes II 2019/1 da Universidade de Brasília.

## Integrantes

Nome | Matrícula
---  | ---
André Filipe Caldas Laranjeira | 16/0023777
Luiz Antônio Borges Martins | 16/0013615

## Instruções de compilação

1) Vá para o diretório raiz do projeto.
2) Execute o comando `qmake ProxyGate.pro`. Isso deve gerar um arquivo
_Makefile_.
3) Execute o comando `make` para executar o arquivo _Makefile_. Isso deve gerar
um arquivo executável _ProxyGate_.

## Modo de uso
1) Execute o comando `./ProxyGate [Número de porta]` para que o proxy seja
inicializado.

## Documentação

O projeto foi documentado utilizando-se o programa _doxygen_. Para gerar a documentação do projeto, basta executar o comando `doxygen Doxyfile`. O _doxygen_ deve gerar a documentação do projeto na pasta _doc_, tanto no formato HTML quanto no formato Latex.