# MDMA 0.5.5-alpha · LCD

Atualização do painel da primeira alpha nativa da série 0.5.5. A referência de comportamento e áudio é
[stable-v0.5.html](reference/stable-v0.5.html). O BSP vem do Dinghy, com a
integração feita a partir da MDMA 0.4.8-lab.

- [Firmware](build/MDMA-0.5.5-alpha-lcd.fwsc) e [mapa de memória](build/MDMA-0.5.5-alpha-lcd.map).
- [Patch cumulativo](firmware/MDMA-0.5.5-alpha-lcd.patch), [testes](tests/) e [checksums](SHA256SUMS).

## Painel

O LCD nativo usa a tipografia, a paleta e a estrutura de 240 × 240 do
HTML de referência. A HOME mostra cena, cidade, BPM, tom, master, V1–V8 e
as seis camadas com medidores. FX troca os quatro cartões inferiores
e preserva a lista. HOME abre o Stage Map e volta para o mixer.

- [HOME, FX, SEQ e Stage Map lado a lado](preview/home-fx-seq-stage.png)
- [EDIT, SCL, GLO e SAVE lado a lado](preview/edit-scl-glo-save.png)

Em SEQ, SELECT escolhe a peça da bateria, as teclas brancas editam seus
passos e o quarto knob muda a camada exibida. Em GLO, SELECT navega pela
lista e ALGORITHM ou OCT+/− ajusta a opção. Para medir o pico de áudio,
abra a linha AUDIO MAX. Em EDIT, as teclas de camada selecionam o alvo.
A origem emprestada aparece no rodapé. SAVE abre uma confirmação central
para a gravação A/B de todas as cidades.

EDIT, GLO e SAVE mostram os parâmetros e ações disponíveis no firmware.
As capturas do emulador mostram os valores efetivos do firmware, inclusive
a posição física simulada do MASTER. O Stage Map reflete o mixer atual.
A modulação espacial da stable ainda não está conectada ao áudio nativo.

A HOME, o FX e o SEQ atualizam medidores e playhead por regiões do LCD.
Os testes com o DMA temporizado do fm1-emu verificaram navegação, controles,
LCD parado sem redesenho, e gravação e restauração de FX e passos de HAT.

## Áudio e validação

HAND e Ghost usam três serras polyBLEP, filtro RBJ, glide e envelopes da
referência, em ponto fixo a 44,1 kHz. O Ghost acompanha tom, escala e a
cidade de origem da camada. Em GLO, LEAD MODE alterna HOLD e ALIEN.
Ao tocar uma nota manual, o Ghost reduz a intensidade e volta gradualmente
ao soltar.

O teste de cinco segundos chamou o DSP de LEAD do HTML original diretamente.
Em HOLD, nas três cidades de fábrica, todos os graus do Ghost coincidiram,
o erro máximo de cutoff ficou abaixo de 0,40 Hz e o erro RMS normalizado
do PCM de HAND + Ghost ficou entre 0,276% e 0,280%. Em ALIEN em Búzios,
o erro RMS foi 0,133%. Esses valores correspondem ao LEAD isolado antes
do mixer, dos efeitos e do processamento de saída.

Passaram os testes em host de seis camadas isoladas, quatro FX por camada
simultâneos, MIDI, clock e SAVE A/B. No fm1-emu web, passaram boot, PLAY,
ALL LAYERS, troca de cidade no compasso, origem de camada, FX independente,
SEQ, SCL, Stage Map, SAVE e recuperação após reiniciar. O FWSC foi
reproduzido byte a byte em dois checkouts.

## Paridade restante

Continuam pendentes a síntese e os algoritmos de DRUMS, BASS, AMB, PLUCK
e SPARKS equivalentes ao HTML, o mixer, os efeitos e os processadores de
saída equivalentes, os modos e macros completos do LEAD, parâmetros por passo,
menus completos, cidades geradas, backups e quatro pontos de retorno,
roteamento de notas MIDI por camada e testes físicos de USB/TRS e desempenho.
O tempo de áudio do emulador não substitui a medição AUDIO MAX no FM-1.

## Uso no FM-1

Use o arquivo `MDMA-0.5.5-alpha-lcd.fwsc`. Antes de SAVE, confira o áudio e os
dados recuperados, e teste o resgate OCT− no boot. O SAVE usa MGR3 no par A/B
em 0xB1000, lê MGR1 e MGR2 e bloqueia gravações sobre dados desconhecidos.
Um firmware anterior só poderá ler uma cópia legada que ainda exista.
Depois de alterar kick e FX, pressione SAVE e confirme duas vezes com
OCT+. Espere GROOVE SAVED e reinicie para conferir a recuperação.

## Build e testes

Use [Dinghy](https://github.com/zvenson/dinghy) no commit
`ea1b6bf5973b11f9b4b806d08155602aaa0b505e`, o toolchain
[JieLi Linux](https://pkgman.jieliapp.com/s/linux-toolchain) 20260730.1
e o [SDK AC79NN](https://github.com/amitv87/fw-AC79_AIoT_SDK) no commit
`d179b4484759423312073f5fbb232501aa491047`. São necessários apenas
`uboot.boot`, `cfg_tool.bin` e `cfg/eq_cfg_hw.bin` em
`cpu/wl82/tools` do SDK. O script de build verifica seus SHA-256.

A partir do checkout Dinghy, com `ALPHA` apontando para este pacote:

```sh
git apply "$ALPHA/firmware/MDMA-0.5.5-alpha-lcd.patch"
JIELI_TOOLCHAIN=/caminho/toolchain AC79_SDK=/caminho/sdk ./build.sh --release 0.5.5-lcd
cmp build/mdma-0.5.5-lcd.fwsc "$ALPHA/build/MDMA-0.5.5-alpha-lcd.fwsc"
cc -O2 -std=c11 -Ifirmware/src "$ALPHA/tests/lead-reference.c" -o /tmp/mdma-lead
for city in 0 1 2; do
  node "$ALPHA/tests/lead-reference.cjs" "$ALPHA/reference/stable-v0.5.html" /tmp/mdma-lead "$city"
done
node "$ALPHA/tests/lead-reference.cjs" "$ALPHA/reference/stable-v0.5.html" /tmp/mdma-lead 0 alien
cc -O2 -std=c11 -Ifirmware/src "$ALPHA/tests/test-integration.c" -o /tmp/mdma-integration
/tmp/mdma-integration
cc -O2 -Ifirmware/hal -Ifirmware/src -Ifirmware/gen "$ALPHA/tests/test-midi.c" -o /tmp/mdma-midi
/tmp/mdma-midi
node "$ALPHA/tests/test-lcd.mjs" /caminho/fm1-emu/web/dist build/mdma-0.5.5-lcd.fwsc
```

Para regenerar as tabelas no checkout Dinghy, execute
`python3 tools/mdma_lead_tables.py` e
`node tools/mdma_patterns.cjs "$ALPHA/reference/stable-v0.5.html"`.
