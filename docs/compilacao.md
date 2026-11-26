# Compilar a 0.5.6-controls

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
