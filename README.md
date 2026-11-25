# MDMA 0.5.5-alpha-flow para o FM-1

[Baixar o ZIP desta versão](https://github.com/mdmarinato/MDMA/raw/refs/tags/v0.5.5-alpha-flow/MDMA-0.5.5-alpha-flow.zip). Extraia e use o arquivo em `build/MDMA-0.5.5-alpha-flow.fwsc` com o instalador que você usou na alpha anterior.

A referência é a stable HTML da 0.5.5-beta. [Conferência por função](PARIDADE.md) · [Checksums](SHA256SUMS) · [Referência HTML](reference/stable-v0.5.html).

## Tocar e editar

- HOME percorre a lista de camadas, o Stage Map e o mixer de seis tiras. No Stage, o movimento afeta pan, filtro e envios. Na lista, no mapa e no mixer, segure LFO para solo ou ENV para mudo da camada selecionada. Solte para voltar à mixagem. O mixer mostra seis VUs e os quatro controles da camada embaixo. ARP está livre.
- Os knobs de mix e FX avançam uma unidade por clique em giro normal. Acima de 60 cliques/s de velocidade suavizada, a resposta aumenta progressivamente até três unidades por clique a 160 cliques/s. Soltar o knob encerra a edição. O áudio suaviza a transição em cerca de 12 ms.
- EDIT abre quatro parâmetros da camada selecionada. OCT+ e OCT− navegam DRUMS, BASS, AMB, PLUCK, LEAD e SPARKS, mantendo o BPM. KNOB1–4 ajustam valores, SELECT escolhe a linha e ALGORITHM altera o tipo. No EDIT do LEAD, REC alterna STABLE e GRID por cidade. GRID aproxima o glide de 50 ms no centro, restringe a faixa e ancora o Ghost HOLD à grade. A frase de ALIEN continua própria. GLO mantém BASS MODE e LEAD MODE separados.
- Na HOME, as teclas brancas tocam a camada selecionada. DRUMS distribui seis peças, PLUCK e SPARKS disparam notas e BASS, AMB e LEAD sustentam a tecla. No MIDI, os canais 1–6 seguem DRUMS, BASS, PLUCK, LEAD, AMB e SPARKS.
- No SEQ de DRUMS, SELECT escolhe a peça. Um toque curto alterna o passo. Segure por 275 ms e use KNOB1–4 para VEL, TIME, ATK e REL. Cada valor aparece abaixo do rótulo. Girar um knob ativa um passo vazio. Solte a tecla para terminar. Um novo toque alterna o passo. Há espaço para **16 passos com parâmetros modificados** no conjunto de cidades. Levar os quatro valores ao centro libera uma posição. Sem tecla segurada, KNOB1 cria um rascunho, KNOB2 positivo ouve a prévia e KNOB3 positivo aplica. Giro negativo de KNOB2 ou KNOB3 descarta.
- PRESETS percorre cidades. Segure EDIT na HOME e gire PRESETS para trocar a origem da camada. GLO permite criar até cinco cidades além das três de fábrica.

## SAVE e backup

O autosave exige **33 segundos consecutivos de silêncio** e dados alterados. Transporte, notas e atividade de áudio reiniciam a contagem. A rotina usa um limiar de três unidades de PCM de 16 bits para o ruído residual. SAVE manual continua disponível.

Em SAVE, na linha SAVE TO DEVICE, ALGORITHM alterna ALL CITIES e CURRENT CITY. CURRENT CITY substitui a cidade atual e os seus passos editados no save principal. Faça um ALL CITIES primeiro ao criar ou reorganizar cidades. Se a combinação de passos exceder a capacidade, o aparelho também pedirá ALL CITIES. O autosave continua gravando todas as cidades após 33 s de silêncio e pode persistir outras edições pendentes. SELECT escolhe um dos quatro pontos de retorno. Nos pontos, ALGORITHM escolhe salvar, carregar ou apagar. OCT+ abre a confirmação e confirma, OCT− cancela. Cada ponto carrega a cidade guardada e conserva as demais. Dados estranhos no setor físico bloqueiam sua reutilização.

A ferramenta em `backup/index.html` transfere cidades por USB MIDI SysEx no Chrome ou Edge com permissão MIDI. Se o navegador exigir uma origem local segura, abra o diretório do pacote com `python3 -m http.server 8000` e visite `http://localhost:8000/backup/`. O JSON contém o registro nativo MGR6 e seu CRC. O backup cobre as cidades atuais. Os quatro pontos possuem armazenamento separado. MGR1–MGR5 são lidos pelo firmware. A ferramenta de backup ainda requer conferência com o cabo e navegador reais.

## Medir o áudio no FM-1

Abra o mixer, pressione REC e ligue só DRUMS. AUDIO MAX mostra o maior tempo de atendimento desde que a calibração foi aberta. Depois ligue as outras camadas. Para começar outra medição, saia e volte com REC. Cada bloco contém 256 frames a 44,1 kHz e vence em aproximadamente 5805 µs. Um pico de 2900 µs ocupa metade desse intervalo. A proteção reduz o reverb ao longo de cerca de 46 ms quando a síntese supera 75% do prazo e tenta restaurá-lo após oito segundos de carga menor. A tela indica quando está ativa.

No emulador a 100 MHz em Búzios, com LCD temporizado e janelas de quatro segundos, DRUMS sozinho passou de 336 atrasos em 354 blocos na alpha-stage para zero em 690 nesta versão. O pico caiu de 8375 para 3885 µs. Com as seis camadas, o pico foi 5070 µs e os 689 blocos cumpriram o prazo. Ainda falta ouvir e medir esta compilação no FM-1 físico.

## Reproduzir

Aplique o patch no Dinghy `ea1b6bf5973b11f9b4b806d08155602aaa0b505e`. Use o toolchain JieLi Linux 20260730.1 e os binários do SDK AC79NN `d179b4484759423312073f5fbb232501aa491047`. No diretório Dinghy, com ALPHA apontando para este pacote, execute

```sh
git apply "$ALPHA/firmware/MDMA-0.5.5-alpha-flow.patch"
node tools/mdma_patterns.cjs "$ALPHA/reference/stable-v0.5.html"
python3 tools/mdma_lead_tables.py
python3 tools/mdma_mix_tables.py
JIELI_TOOLCHAIN=/caminho/toolchain AC79_SDK=/caminho/sdk ./build.sh --release 0.5.5-flow
cmp build/mdma-0.5.5-flow.fwsc "$ALPHA/build/MDMA-0.5.5-alpha-flow.fwsc"
sh "$ALPHA/tests/run-all.sh" /caminho/fm1-emu/web/dist
```
