# MDMA 0.5.6-alpha para FM-1

Firmware para o sintetizador M-VAVE FM-1. O pacote de distribuição inclui o firmware instalável em `build/MDMA-0.5.6-alpha.fwsc`, os patches de código, a ferramenta de backup e os testes.

[Baixar o pacote ZIP da versão 0.5.6-alpha](https://github.com/mdmarinato/MDMA/releases/download/v0.5.6-alpha/MDMA-0.5.6-alpha.zip) · [Notas da versão e firmware avulso](https://github.com/mdmarinato/MDMA/releases/tag/v0.5.6-alpha)

## Controles ao tocar

Na lista, no mapa e no mixer, FX percorre MIX, FX e EQ. HOME muda a tela e conserva o modo dos knobs. SELECT escolhe a camada. O rodapé mostra SOLO ou MUTE somente enquanto LFO ou ENV estiver pressionado. O banco EQ controla LO, LO-MID, HI-MID e HI, de −9 a +6 dB em passos de 1 dB. O centro musical é 0 dB. O mixer das camadas trabalha a 11.025 Hz, com divisões aproximadas de 150, 680 e 2.430 Hz. A banda HI da bateria também controla o componente seco de alta frequência a 44,1 kHz.

Na lista, no mapa e no mixer, segure SEQ por 350 ms com DRUMS tocando para programar o fill. FILL NEXT indica que está na fila e FILL IN indica execução. Soltar mantém a tela. Segurar novamente antes da execução cancela o pedido. Um toque curto abre o sequenciador.

EDIT abre os parâmetros da camada. Um segundo toque volta à HOME.

## Save e ponto de retorno

| Linha | Ação |
| --- | --- |
| SAVE TO DEVICE | Grava a cidade atual ou o conjunto de cidades no save principal. |
| POINT | Guarda a cidade selecionada no ponto de retorno. Quando existe um ponto gravado, ALGORITHM escolhe SAVE ou LOAD. |
| DELETE POINT | Apaga o ponto de retorno após confirmação. |
| RESET CITY | ALGORITHM escolhe uma cidade ou ALL CITIES. A confirmação restaura os parâmetros e padrões da receita e grava o conjunto atual no save principal. As cidades geradas conservam suas vagas. |

POINT usa o mesmo endereço do ponto 2 das versões anteriores. Os pontos antigos 1, 3 e 4 ficam fora do menu. OCT+ abre a confirmação, outro OCT+ confirma e OCT− cancela. Antes de voltar para uma versão anterior, exporte um backup. Saves com EQ editado usam uma extensão do registro MGR6.

## Reproduzir

Use o Dinghy `ea1b6bf5973b11f9b4b806d08155602aaa0b505e`, o compilador JieLi Linux 20260730.1 e os arquivos do SDK AC79NN `d179b4484759423312073f5fbb232501aa491047`. O espelho do compilador é a release `toolchain-20260730.1` de `DimmKirr/setup-fm1-toolchain` no GitHub. O SHA-256 do tar.xz reempacotado é `d3fc3be92cfc07ccb0ac682b3b2e41e4d14593426067e6c3c6e908db2f90c0e0`.

Execute na árvore do Dinghy, com PKG apontando para a raiz deste repositório ou do pacote extraído.

```sh
git apply "$PKG/firmware/MDMA-0.5.5-alpha-live.patch"
git apply "$PKG/firmware/0.5.6-alpha.patch"
node tools/mdma_patterns.cjs "$PKG/reference/stable-v0.5.html"
python3 tools/mdma_lead_tables.py
python3 tools/mdma_mix_tables.py
JIELI_TOOLCHAIN=/caminho/toolchain AC79_SDK=/caminho/sdk ./build.sh --release 0.5.6-alpha
cmp build/mdma-0.5.6-alpha.fwsc "$PKG/build/MDMA-0.5.6-alpha.fwsc"
sh "$PKG/tests/run-all.sh" /caminho/fm1-emu/web/dist
```

O teste de migração no fm1-emu cria um ponto 2 usando o firmware 0.5.5-alpha-eq e restaura os setores na versão 0.5.6-alpha. Também verifica o gesto de SEQ nas três telas.

```sh
node "$PKG/tests/previous-point-and-fill.mjs" /caminho/fm1-emu/web/dist /caminho/MDMA-0.5.5-alpha-eq.fwsc "$PKG/build/MDMA-0.5.6-alpha.fwsc"
```

Validação em host e no fm1-emu v0.5.1 a 100 MHz, com LCD temporizado. Avaliação física desta versão pendente.
