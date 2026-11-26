# MDMA para M-VAVE FM-1

**0.5.6-controls** é a primeira versão oficial e a base de desenvolvimento da branch `main`. As versões alpha anteriores ficam no histórico de pré-lançamentos.

[Baixar o ZIP oficial](https://github.com/mdmarinato/MDMA/raw/refs/tags/v0.5.6-controls/downloads/v0.5.6-controls/MDMA-0.5.6-controls.zip) · [Releases](https://github.com/mdmarinato/MDMA/releases)

O firmware instalável dentro do ZIP é `mdma-0.5.6-controls.fwsc`. Para conferir os arquivos, extraia o ZIP e execute `sha256sum -c SHA256SUMS` dentro da pasta extraída.

## Controles no palco

- Em LIST, MAP e MIXER, FX alterna os quatro knobs entre MIX, FX e EQ. MIX e FX respondem à velocidade do giro. MAP e MIXER mostram a camada selecionada no alto à esquerda.
- Com DRUMS tocando, segure SEQ para armar o fill. Segure novamente antes da execução para cancelar. SOLO, MUTE e FILL aparecem na mesma posição.
- A última tecla preta silencia as camadas ativas e recupera a seleção anterior ao tocar novamente.
- Em EDIT, OCT− e OCT+ percorrem as camadas. As teclas de camada ligam e desligam o áudio. LEAD usa GRID.

## Gravação e recuperação

SAVE TO DEVICE grava as cidades no aparelho. POINT guarda ou recupera a cidade selecionada em um único ponto de retorno, compatível com o antigo ponto 2. DELETE POINT apaga esse ponto após confirmação. O reset de fábrica fica em GLO.

## Desenvolvimento

Para começar uma alteração, crie uma branch a partir de `main`. A tag `v0.5.6-controls` identifica o código e o pacote exatos da primeira versão oficial.

## Reproduzir

Use o Dinghy `ea1b6bf5973b11f9b4b806d08155602aaa0b505e`, o compilador JieLi Linux 20260730.1 e os arquivos do SDK AC79NN `d179b4484759423312073f5fbb232501aa491047`. O espelho do compilador é a release `toolchain-20260730.1` de `DimmKirr/setup-fm1-toolchain` no GitHub. O SHA-256 do tar.xz reempacotado é `d3fc3be92cfc07ccb0ac682b3b2e41e4d14593426067e6c3c6e908db2f90c0e0`.

Execute na árvore do Dinghy, com PKG apontando para a raiz deste repositório na versão oficial.

```sh
git apply "$PKG/firmware/MDMA-0.5.5-alpha-live.patch"
git apply "$PKG/firmware/0.5.6-alpha.patch"
git apply "$PKG/firmware/controls.patch"
node tools/mdma_patterns.cjs "$PKG/reference/stable-v0.5.html"
python3 tools/mdma_lead_tables.py
python3 tools/mdma_mix_tables.py
JIELI_TOOLCHAIN=/caminho/toolchain AC79_SDK=/caminho/sdk ./build.sh --release 0.5.6-controls
cmp build/mdma-0.5.6-controls.fwsc "$PKG/build/mdma-0.5.6-controls.fwsc"
cc -O1 -std=gnu11 -Ifirmware/src "$PKG/tests/test-controls.c" -lm -o /tmp/mdma-test-controls
/tmp/mdma-test-controls
```
