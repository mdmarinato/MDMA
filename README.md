# MDMA 0.5.5-alpha-stage para o FM-1

[Baixar o ZIP desta versão](https://github.com/mdmarinato/MDMA/raw/refs/tags/v0.5.5-alpha-stage/MDMA-0.5.5-alpha-stage.zip). Extraia e use o arquivo em `build/MDMA-0.5.5-alpha-stage.fwsc` com o instalador que você usou na alpha anterior.

A referência é a stable HTML da 0.5.5-beta. [Conferência por função](PARIDADE.md) · [Checksums](SHA256SUMS) · [Referência HTML](reference/stable-v0.5.html).

## Tocar e editar

- HOME percorre a lista de camadas, o Stage Map e o mixer de seis tiras. No Stage, o movimento afeta pan, filtro e envios. No mixer, segure LFO para solo ou ENV para mudo da camada selecionada. Solte para voltar à mixagem.
- Em HOME, FX, Stage e mixer, giros rápidos aceleram os controles. Um giro forte acrescenta quatro pequenos passos de inércia em 200 ms. O áudio suaviza as mudanças em cerca de 12 ms. No mixer, REC abre o contador de encoders. Dê uma volta completa em cada knob para ler os cliques e o pico por leitura. REC volta ao mixer.
- EDIT abre quatro parâmetros da camada selecionada. KNOB1–4 ajustam valores, SELECT escolhe a linha e ALGORITHM altera o tipo. No EDIT do LEAD, REC alterna STABLE e GRID por cidade. GRID aproxima o glide de 50 ms no centro, restringe a faixa e ancora o Ghost HOLD à grade. A frase de ALIEN continua própria. GLO mantém BASS MODE e LEAD MODE separados.
- Na HOME, as teclas brancas tocam a camada selecionada. DRUMS distribui seis peças, PLUCK e SPARKS disparam notas e BASS, AMB e LEAD sustentam a tecla. No MIDI, os canais 1–6 seguem DRUMS, BASS, PLUCK, LEAD, AMB e SPARKS.
- No SEQ de DRUMS, SELECT escolhe a peça. Um toque curto alterna o passo. Segure por 275 ms e use KNOB1–4 para VEL, TIME, ATK e REL. Há espaço para **16 passos com parâmetros modificados** no conjunto de cidades. Levar os quatro valores ao centro libera uma posição. Sem tecla segurada, KNOB1 cria um rascunho, KNOB2 positivo ouve a prévia e KNOB3 positivo aplica. Giro negativo de KNOB2 ou KNOB3 descarta.
- PRESETS percorre cidades. Segure EDIT na HOME e gire PRESETS para trocar a origem da camada. GLO permite criar até cinco cidades além das três de fábrica.

## SAVE e backup

O autosave exige **33 segundos consecutivos de silêncio** e dados alterados. Transporte, notas e atividade de áudio reiniciam a contagem. A rotina usa um limiar de três unidades de PCM de 16 bits para o ruído residual. SAVE manual continua disponível.

Em SAVE, use SELECT para escolher o aparelho inteiro ou um dos quatro pontos de retorno. Nos pontos, ALGORITHM escolhe salvar, carregar ou apagar. OCT+ abre a confirmação e confirma, OCT− cancela. Cada ponto carrega a cidade guardada e conserva as demais. Dados estranhos no setor físico bloqueiam sua reutilização.

A ferramenta em `backup/index.html` transfere cidades por USB MIDI SysEx no Chrome ou Edge com permissão MIDI. Se o navegador exigir uma origem local segura, abra o diretório do pacote com `python3 -m http.server 8000` e visite `http://localhost:8000/backup/`. O JSON contém o registro nativo MGR6 e seu CRC. O backup cobre as cidades atuais. Os quatro pontos possuem armazenamento separado. MGR1–MGR5 são lidos pelo firmware. A ferramenta de backup ainda requer conferência com o cabo e navegador reais.

## Reproduzir

Aplique o patch no Dinghy `ea1b6bf5973b11f9b4b806d08155602aaa0b505e`. Use o toolchain JieLi Linux 20260730.1 e os binários do SDK AC79NN `d179b4484759423312073f5fbb232501aa491047`. No diretório Dinghy, com ALPHA apontando para este pacote, execute

```sh
git apply "$ALPHA/firmware/MDMA-0.5.5-alpha-stage.patch"
node tools/mdma_patterns.cjs "$ALPHA/reference/stable-v0.5.html"
python3 tools/mdma_lead_tables.py
python3 tools/mdma_mix_tables.py
JIELI_TOOLCHAIN=/caminho/toolchain AC79_SDK=/caminho/sdk ./build.sh --release 0.5.5-stage
cmp build/mdma-0.5.5-stage.fwsc "$ALPHA/build/MDMA-0.5.5-alpha-stage.fwsc"
sh "$ALPHA/tests/run-all.sh" /caminho/fm1-emu/web/dist
```
