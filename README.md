# MDMA 0.5.5-alpha-live para o FM-1

[Baixar o ZIP desta versão](https://github.com/mdmarinato/MDMA/raw/refs/tags/v0.5.5-alpha-live/MDMA-0.5.5-alpha-live.zip). Extraia e use o arquivo em `build/MDMA-0.5.5-alpha-live.fwsc` com o instalador que você usou na alpha anterior.

A referência é a stable HTML da 0.5.5-beta. [Conferência por função](PARIDADE.md) · [Checksums](SHA256SUMS) · [Referência HTML](reference/stable-v0.5.html).

## Cenas e variações

INTRO, GROOVE, TENSION e PEAK mudam a bateria. PLUCK e SPARKS continuam consultando a cena e AMB acompanha a tonalidade. BASS e LEAD guardam uma escolha de V1–V8 por camada e cidade. O transporte continua na mesma posição quando a cena muda. Por exemplo, deixe o LEAD de Ibiza em V1 e troque a bateria de INTRO para PEAK.

V1 dá acesso às frases da INTRO. No BASS HOLD de fábrica, V2 alterna duas notas, V3 trabalha a oitava e V4 sobe pela escala. Na HOME, ALGORITHM muda a variação da camada selecionada. Ao abrir um save da alpha-scenes ou ghost-edit, vale a seleção da cena que estava gravada como ativa naquela cidade. No EDIT do LEAD, GHOST abre selecionado. ALGORITHM à esquerda escolhe FREE e à direita GRID. HOLD e ALIEN ficam no GLO.

## Sequenciador de BASS e LEAD

Dê dois toques em EDIT com BASS ou LEAD selecionado. A tela mostra os 16 passos em duas linhas de oito. SELECT ou as 16 teclas brancas escolhem o passo. KNOB1 define uma nota MIDI de 24 a 96. KNOB2 percorre AUTO (frase original), nota, pausa e sustentar. ALGORITHM troca V1–V8. EDIT volta aos parâmetros. As notas programadas também são enviadas pelo MIDI nos canais 2 (BASS) e 4 (LEAD); pausa encerra a nota e sustentar prolonga a anterior.

O save tem 16 posições compartilhadas com as edições de VEL/TIME/ATK/REL da bateria. Um grupo de quatro passos melódicos consecutivos (1–4, 5–8, 9–12 ou 13–16) usa uma posição quando tem alguma edição. Com a memória livre, cabem quatro frases de 16 passos inteiramente personalizadas. Voltar todos os passos de um grupo para AUTO libera a posição. STEP MEMORY FULL indica que a nova edição não coube. Faça backup antes de voltar a um firmware antigo, que pode recusar saves com passos melódicos.

## Tocar e editar

- HOME percorre a lista de camadas, o Stage Map e o mixer de seis tiras. No Stage, o movimento afeta pan, filtro e envios. Na lista, no mapa e no mixer, segure LFO para solo ou ENV para mudo da camada selecionada. Solte para voltar à mixagem. O mixer mostra seis VUs e os quatro controles da camada embaixo. ARP está livre.
- Os knobs de mix e FX avançam uma unidade por clique em giro normal. Acima de 36 cliques/s de velocidade suavizada, o ganho aumenta progressivamente até cinco unidades por clique a 135 cliques/s. O EDIT aplica a mesma leitura de velocidade com ganho menor para a escala de 41 posições. O MASTER físico usa entrada analógica e os outros knobs usam encoders. Soltar o knob encerra a edição. O áudio suaviza a transição em cerca de 12 ms.
- EDIT abre quatro parâmetros da camada selecionada. OCT+ e OCT− navegam DRUMS, BASS, AMB, PLUCK, LEAD e SPARKS, mantendo o BPM. KNOB1–4 ajustam valores, SELECT escolhe a linha e ALGORITHM altera o tipo. No EDIT do LEAD, SELECT navega pelas cinco linhas. Em GHOST, ALGORITHM escolhe FREE ou GRID por cidade. GRID aproxima o glide de 50 ms no centro, restringe a faixa e ancora o Ghost HOLD à grade. A frase de ALIEN continua própria. GLO mantém BASS MODE e LEAD MODE separados.
- Na HOME, as teclas brancas tocam a camada selecionada. DRUMS distribui seis peças, PLUCK e SPARKS disparam notas e BASS, AMB e LEAD sustentam a tecla. No MIDI, os canais 1–6 seguem DRUMS, BASS, PLUCK, LEAD, AMB e SPARKS.
- No SEQ de DRUMS, SELECT escolhe a peça. Um toque curto alterna o passo. Segure por 275 ms e use KNOB1–4 para VEL, TIME, ATK e REL. Cada valor aparece abaixo do rótulo. Girar um knob ativa um passo vazio. Solte a tecla para terminar. Um novo toque alterna o passo. Há espaço para **16 passos com parâmetros modificados** no conjunto de cidades. Levar os quatro valores ao centro libera uma posição. Sem tecla segurada, KNOB1 cria um rascunho, KNOB2 positivo ouve a prévia e KNOB3 positivo aplica. Giro negativo de KNOB2 ou KNOB3 descarta.
- PRESETS percorre cidades. Segure EDIT na HOME e gire PRESETS para trocar a origem da camada. GLO permite criar até cinco cidades além das três de fábrica.

## SAVE e backup

O autosave exige **33 segundos consecutivos de silêncio** e dados alterados. Transporte, notas e atividade de áudio reiniciam a contagem. A rotina usa um limiar de três unidades de PCM de 16 bits para o ruído residual. SAVE manual continua disponível.

Em SAVE, na linha SAVE TO DEVICE, ALGORITHM alterna ALL CITIES e CURRENT CITY. CURRENT CITY substitui a cidade atual e os seus passos editados no save principal. Faça um ALL CITIES primeiro ao criar ou reorganizar cidades. Se a combinação de passos exceder a capacidade, o aparelho também pedirá ALL CITIES. O autosave continua gravando todas as cidades após 33 s de silêncio e pode persistir outras edições pendentes. SELECT escolhe um dos quatro pontos de retorno. Nos pontos, ALGORITHM escolhe salvar, carregar ou apagar. OCT+ abre a confirmação e confirma, OCT− cancela. Cada ponto carrega a cidade guardada e conserva as demais. Dados estranhos no setor físico bloqueiam sua reutilização.

A ferramenta em `backup/index.html` transfere cidades por USB MIDI SysEx no Chrome ou Edge com permissão MIDI. Se o navegador exigir uma origem local segura, abra o diretório do pacote com `python3 -m http.server 8000` e visite `http://localhost:8000/backup/`. O JSON contém o registro nativo MGR6 e seu CRC. O backup cobre as cidades atuais. Os quatro pontos possuem armazenamento separado. MGR1–MGR5 são lidos pelo firmware. A ferramenta de backup ainda requer conferência com o cabo e navegador reais.

## Medir o áudio no FM-1

Abra o mixer, pressione REC e ligue só DRUMS. AUDIO MAX mostra o maior tempo de atendimento desde que a calibração foi aberta. Depois ligue as outras camadas. Para começar outra medição, saia e volte com REC. Cada bloco contém 256 frames a 44,1 kHz e vence em aproximadamente 5805 µs. Um pico de 2900 µs ocupa metade desse intervalo. A proteção reduz o reverb ao longo de cerca de 46 ms quando a síntese supera 75% do prazo e tenta restaurá-lo após oito segundos de carga menor. A tela indica quando está ativa.

A tabela de medições desta compilação está em PARIDADE.md. A confirmação com áudio e encoders físicos continua pendente.

## Reproduzir

Aplique o patch no Dinghy `ea1b6bf5973b11f9b4b806d08155602aaa0b505e`. Use o toolchain JieLi Linux 20260730.1 e os binários do SDK AC79NN `d179b4484759423312073f5fbb232501aa491047`. No diretório Dinghy, com ALPHA apontando para este pacote, execute

```sh
git apply "$ALPHA/firmware/MDMA-0.5.5-alpha-live.patch"
node tools/mdma_patterns.cjs "$ALPHA/reference/stable-v0.5.html"
python3 tools/mdma_lead_tables.py
python3 tools/mdma_mix_tables.py
JIELI_TOOLCHAIN=/caminho/toolchain AC79_SDK=/caminho/sdk ./build.sh --release 0.5.5-live
cmp build/mdma-0.5.5-live.fwsc "$ALPHA/build/MDMA-0.5.5-alpha-live.fwsc"
sh "$ALPHA/tests/run-all.sh" /caminho/fm1-emu/web/dist
```
