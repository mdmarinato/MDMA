# Paridade e diagnóstico da 0.5.5-alpha-live

Referência funcional e musical em `stable-v0.5.html` da 0.5.5-beta, SHA-256 `50302022da4ab83527823f9576983529f08755178d080cee3c15a184e6f83d3c`. O diagnóstico da alpha-ghost orientou os testes de trajetória e os controles opcionais. O relato físico de picotes veio da alpha-stage. A alpha-scenes foi avaliada em host e emulador.

| Área | Implementação e verificação | Limites conhecidos |
| --- | --- | --- |
| Padrões e cidades | Comparação com o JavaScript de 608 conjuntos de seis camadas, quatro cenas e oito variações. Búzios, Ibiza e Rio preservam os dados originais. Macros de síntese e valores de retorno vêm da stable. | A criação escolhe entre 16 receitas pré-compiladas. Sementes arbitrárias e snapshots completos de edição por origem continuam pendentes. |
| Ghost | O núcleo isolado do Ghost passa pelas oito comparações de PCM anteriores. No arranjo novo, HOLD recebe as notas da variação da camada e o caminho de ida e volta. A frase guiada é uma escolha musical desta alpha. GRID é opcional por cidade. Em 120 s de HOLD, A menor, 132 BPM, com amostragem de pitch a cada 32 frames, STABLE passou 33,35% do tempo a mais de 25 cents de uma nota, GRID 2,40%. As 24 mudanças observadas em GRID ocorreram em mudança de slot. | Esta medida cobre uma configuração. A comparação PCM isolada precede mixer e FX. A avaliação musical no palco continua aberta. |
| Vozes e EDIT | Duas serras polyBLEP e sub no BASS com filtros em cascata, drive, gate, glide e duck. PLUCK tem oito vozes, SPARKS seis, AMB usa root, fifth e mist além do drone manual. DRUMS tem 24 vozes de ruído e 12 osciladores. As seis páginas EDIT ligam quatro knobs e quatro seletores de tipo ao áudio. | Filtros, coeficientes, aleatoriedade, envelopes e agendamento em ponto fixo têm aproximações. As demais cinco vozes ainda não têm comparação PCM cobrindo todas as cidades e todas as edições. As curvas de kick e punch pedem A/B. |
| Stage e mixer | Pan, filtro e pesos de três envios comparados com o HTML em 768 posições. A maior diferença foi 1,25 passo ou menos na escala de 127. O painel expõe medidores, calibração de knobs, solo e mudo momentâneos. | A comparação é dos controles projetados. Som estéreo final, resposta dos encoders e margem de processamento sob carga precisam de ensaio no FM-1. Pan e volume conservam a curva da stable. |
| FX e dinâmica | Filtro DJ bipolar, phaser de quatro estágios, saturação, delay de três semicolcheias, reverb estéreo com oito combs e quatro all-pass por lado, compressor do synth bus, compressor e limitador na saída. Os testes exercitam seis camadas com os quatro FX em máximo. | O tempo do primeiro eco e o pico do teste estão verificados em host. A bateria e o LEAD conservam caminho a 44,1 kHz. O barramento de FX e parte das vozes usam 11,025 kHz com interpolação. A proteção de carga pode retirar o reverb. A soma de envios por origem, o ganho de saída e a cauda ainda precisam de comparação de PCM estéreo com a stable. |
| SEQ, transporte e MIDI | Edição por toque ou tecla segurada, rascunho, prévia, aplicação e descarte. Notas locais por camada e roteamento MIDI USB/TRS. Teste em host de START, STOP, CONTINUE, pulsos e isolamento entre fontes. | Até 16 passos com shape por conjunto. O rascunho movimenta hits de HAT e PERC, com divergências no sorteio e nas formas dos passos. Antecipação limitada no primeiro passo do compasso e com clock externo. |
| Persistência | MGR6 com passos melódicos em grupos de quatro e leitura de MGR1–5, dois setores do save principal, dois por ponto e escrita com CRC e commit por último. Testes de quatro pontos, restauração, exclusão, corrupção, migração, USB em host do SAVE por cidade e autosave de todas as cidades aos 33 segundos. | JSON usa o esquema do firmware. Conversão para o JSON interno do simulador, backup de pontos via navegador e ensaio de ponta a ponta com cabo físico estão pendentes. |
| Ideias adicionais do diagnóstico | GRID, calibração, ganho progressivo dos knobs, suavização, solo/mudo e mixer estão nesta alpha. | Perfis de cidades adicionais elaborados à mão, MORPH/RANDOM geral com prévia, aftertouch, duck ajustável, transições de cidade completas e medição de carga física continuam em aberto. |

## Orçamento de áudio

Emulador FM-1 a 100 MHz (revisão `736aded65c8e2e9d652cf595fca248734483a877`), Búzios, LCD temporizado. As seis primeiras linhas acrescentam camadas na INTRO. As três últimas mudam a cena com as seis camadas ligadas. Janela de quatro segundos após estabilização. Prazo por bloco de 5805 µs.

| Camadas | Pico em µs | Blocos | Atrasos |
| --- | ---: | ---: | ---: |
| DRUMS | 3910 | 690 | 0 |
| DRUMS+BASS | 4193 | 690 | 0 |
| +AMB | 4084 | 690 | 0 |
| +PLUCK | 4633 | 689 | 0 |
| +LEAD | 4786 | 689 | 0 |
| SEIS | 5083 | 689 | 0 |
| GROOVE | 4696 | 690 | 0 |
| TENSION | 4140 | 689 | 0 |
| PEAK | 5077 | 690 | 0 |
