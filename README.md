# MDMA 0.5.5-alpha · Ghost e cidades

Atualização da alpha com LCD para FM-1. A referência é
[`stable-v0.5.html`](reference/stable-v0.5.html) da 0.5.5-beta.

- [FWSC](build/MDMA-0.5.5-alpha-ghost.fwsc), [mapa](build/MDMA-0.5.5-alpha-ghost.map) e [checksums](SHA256SUMS).
- [Patch cumulativo](firmware/MDMA-0.5.5-alpha-ghost.patch), [testes](tests/) e [conferência de paridade](PARIDADE.md).

## Controles

No GLO, SELECT escolhe a linha. ALGORITHM ou OCT+/− ajustam BPM RELEASE,
SWING, BASS MODE, LEAD MODE e CLOCK. BASS MODE alterna HOLD e ALIEN.
Em GENERATE RANDOM CITY, DELETE CURRENT CITY ou FACTORY RESET,
OCT+ abre e confirma. OCT− cancela. PRESETS percorre as cidades.
É possível ter cinco cidades criadas junto das três de fábrica.

EDIT abre os controles disponíveis na camada selecionada. No LEAD,
KNOB1–4 ajustam CUTOFF, RES, GLIDE e GHOST RATE. SELECT escolhe a linha
e ALGORITHM alterna o tipo. Para trocar a origem de uma camada, segure
EDIT na HOME e gire PRESETS.

## SAVE

O SAVE grava MGR5 no par A/B em 0xB1000. O porte lê os registros anteriores
MGR1–MGR4 e preserva os dados detectados no aparelho. Firmwares anteriores
podem precisar da cópia legada para ler o SAVE depois de atualizar.
O teste no emulador gravou e recuperou oito cidades após reiniciar.

## Reproduzir

Use [Dinghy](https://github.com/zvenson/dinghy) em
`ea1b6bf5973b11f9b4b806d08155602aaa0b505e`, JieLi Linux 20260730.1
e o SDK AC79NN em `d179b4484759423312073f5fbb232501aa491047`.
O build verifica os arquivos `uboot.boot`, `cfg_tool.bin` e
`cfg/eq_cfg_hw.bin` em `cpu/wl82/tools` do SDK.

Com `ALPHA` apontando para este pacote, a partir do repositório Dinghy:

```sh
git apply "$ALPHA/firmware/MDMA-0.5.5-alpha-ghost.patch"
node tools/mdma_patterns.cjs "$ALPHA/reference/stable-v0.5.html"
python3 tools/mdma_lead_tables.py
JIELI_TOOLCHAIN=/caminho/toolchain AC79_SDK=/caminho/sdk ./build.sh --release 0.5.5-ghost
cmp build/mdma-0.5.5-ghost.fwsc "$ALPHA/build/MDMA-0.5.5-alpha-ghost.fwsc"
node "$ALPHA/tests/pattern-reference.cjs" "$ALPHA/reference/stable-v0.5.html" firmware/src
cc -O2 -std=c11 -Ifirmware/src "$ALPHA/tests/lead-reference.c" -o /tmp/mdma-lead
for city in 0 1 2; do
  node "$ALPHA/tests/lead-reference.cjs" "$ALPHA/reference/stable-v0.5.html" /tmp/mdma-lead "$city"
done
for mode in alien offset sync band linear; do
  node "$ALPHA/tests/lead-reference.cjs" "$ALPHA/reference/stable-v0.5.html" /tmp/mdma-lead 0 "$mode"
done
cc -O2 -std=c11 -Ifirmware/src "$ALPHA/tests/test-integration.c" -o /tmp/mdma-integration
/tmp/mdma-integration
cc -O2 -Ifirmware/hal -Ifirmware/src -Ifirmware/gen "$ALPHA/tests/test-midi.c" -o /tmp/mdma-midi
/tmp/mdma-midi
node "$ALPHA/tests/test-lcd.mjs" /caminho/fm1-emu/web/dist build/mdma-0.5.5-ghost.fwsc
```
