# Conferência com a 0.5.5-beta

A referência é o `stable-v0.5.html` do ZIP 0.5.5-beta, SHA-256
`50302022da4ab83527823f9576983529f08755178d080cee3c15a184e6f83d3c`.
Os ZIPs 0.5.3-beta revisado e 0.5.4 confirmam o histórico de BASS MODE,
geração de cidade, Ghost e navegação do EDIT.

| Área | Estado desta build |
| --- | --- |
| Padrões de fábrica | Os campos gravados para todas as cenas e V1–V8 de Búzios, Ibiza e Rio coincidem com o gerador do HTML. O teste compara máscaras de bateria, notas e oitavas do BASS, acentos, slides, notas, densidade e espaçamento de PLUCK, dados de LEAD, AMB e SPARKS. |
| Padrões especiais | O gerador extrai também a frase de BASS ALIEN, os graus e o comprimento de P57 HOLD e a frase PLUCK V1 de Búzios diretamente do HTML. Ainda há diferenças de execução, como humanização, eventos antecipados, ornamentos, distribuições de probabilidade e envelopes. |
| Cidades criadas | A build escolhe entre 16 receitas compiladas com o gerador da stable. Cada receita tem nome, semente, cor, tom, BPM, swing e seus próprios padrões com as transformações de síncope do HTML. Até cinco podem coexistir. A stable sorteia sementes em tempo de execução num espaço bem maior. Parte das macros de timbre geradas ainda usa aproximações nativas. |
| Origem de camada | A troca de patterns entra no compasso. O LEAD usa brilho e velocidade da origem e mantém o deslocamento manual sobre a deriva. Ao excluir a cidade de origem, camadas que dependiam dela passam a usar a própria cidade. Snapshots completos de presets emprestados e edições por origem seguem pendentes. |
| Deriva e Ghost | HOLD, ALIEN, FREE/SYNC, offset sobre a deriva, filtro low/band pass, ressonância CLASSIC/SOFT e glide SMOOTH/LINEAR têm testes comparativos. Os testes são do LEAD isolado, antes do mixer, FX e saída. A fase, o offset e os ajustes são salvos por cidade. Ainda falta a movimentação efetiva do áudio ligada ao Stage Map e a comparação de transições completas de todas as cidades. |
| BASS | GLO alterna HOLD/ALIEN por cidade. A sequência e os graus vêm da stable e a voz usa duas serras polyBLEP com sub senoidal. Filtro, modulação de cutoff, envelope, drive, glide e duck do motor completo ainda diferem. |
| Demais vozes e mix | DRUMS, AMB, PLUCK e SPARKS ainda têm vozes nativas parciais. A mixagem, os retornos de FX, o compressor e o limitador ainda diferem do HTML. |
| EDIT | O botão abre a camada selecionada nas seis camadas e aceita um toque físico mais longo. Os quatro parâmetros e tipos do LEAD estão disponíveis. DRUMS e BASS ainda têm o conjunto parcial desta alpha. AMB, PLUCK e SPARKS têm controles HOME/FX e ainda precisam do DSP dedicado aos seus parâmetros EDIT. |
| GLO / PRESETS | BPM RELEASE, SWING, BASS MODE, LEAD MODE, geração, exclusão e reset estão na ordem da stable. PRESETS percorre as cidades. CLOCK fica como opção nativa. |
| SEQ | As seis peças, 16 passos, V1–V8 e FILL são operacionais. VEL/TIME/ATK/REL por passo e o fluxo de geração/prévia do rascunho ainda precisam do porte completo. |
| SAVE | MGR5 preserva as cidades, BASS MODE, LEAD e deriva em A/B. Esta build lê MGR1–MGR4. Exportação/importação JSON, quatro pontos de retorno e autosave da stable seguem pendentes. |
| MIDI e desempenho | A soltura da voz diferencia USB/TRS e canal. As notas ainda não têm rota por camada. Testes em host e no fm1-emu passaram. A carga e o comportamento no FM-1 físico ainda precisam da verificação desta build. |

Foram verificadas 608 combinações de cidade, cena e variação. Nas oito
comparações isoladas de HAND/Ghost, todos os graus do Ghost coincidiram.
O maior erro de cutoff foi 0,42 Hz e o maior erro RMS normalizado de PCM
ficou abaixo de 0,62%.
