const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');

const [htmlPath, firmware] = process.argv.slice(2);
const html = fs.readFileSync(htmlPath, 'utf8');
const engine = html.match(/<script[^>]+id="eng"[^>]*>([\s\S]*?)<\/script>/)[1];
const marker = '  return E;';
assert(engine.includes(marker));
const box = {exports: {}};
const source = engine.replace(marker, `
  E.testPatterns = {
    names: GENERATED_NAMES,
    seed: defaults.seed,
    factory: buildFull,
    generated(name, seed) {
      generatedCities = [{name, seed}];
      loadCity(3);
    },
    livePattern(scene, variation, layer) {
      P.bank = scene;
      P.vari[generatorOrder[layer]] = variation;
      return patternFor(generatorOrder[layer]);
    }
  };
  return E;
`);
vm.runInThisContext('(function(module){' + source + '\n})')(box);
const reference = box.exports(44100).testPatterns;
function parseHeader(file) {
  const header = fs.readFileSync(path.join(firmware, file), 'utf8');
  const initializer = header.match(/= (\{[\s\S]*\});/)[1]
    .replaceAll('{', '[').replaceAll('}', ']').replace(/,\s*]/g, ']');
  return JSON.parse(initializer);
}

const groove = parseHeader('mdma_groove_patterns.h');
const melody = parseHeader('mdma_melody_patterns.h');
const lanes = ['kick', 'hat', 'open', 'clap', 'perc', 'noise'];
const bits = values => values.reduce((acc, value, i) => acc + (value ? 2**i : 0), 0);
assert.equal(groove.length, 3 + reference.names.length);
assert.equal(melody.length, groove.length);
for (let city = 0; city < groove.length; city++) {
  const generated = city >= 3;
  if (generated) reference.generated(reference.names[city - 3], 34907 + 70249 * (city - 3));
  for (let scene = 0; scene < 4; ++scene) {
    for (let variation = 0; variation < 8; ++variation) {
      const [drums, bass, pluck, lead, amb, sparks] = Array.from({length: 6},
        (_, layer) => generated
          ? reference.livePattern(scene, variation, layer)
          : reference.factory(scene, variation, [0, 2, 1][city], layer, reference.seed));
      const label = `cidade ${city}, cena ${scene}, V${variation + 1}`;
      assert.deepEqual(groove[city][scene][variation], [
        lanes.map(lane => bits(drums[lane])),
        bass.bass_notes.map((note, step) => note < 0 ? -1 : note + bass.bass_octave[step] * 12),
        bits(bass.bass_accent), bits(bass.bass_slide),
      ], label);
      assert.deepEqual(melody[city][scene][variation], [
        pluck.pluck_notes, pluck.pluck_density, pluck.pluck_spacing, lead.lead_notes, lead.lead_timbre,
        lead.lead_phrase, amb.amb_mode, amb.amb_br, amb.amb_density, amb.amb_reverb, sparks.spk_pitch, sparks.spk_density, sparks.spk_pan,
      ], label);
    }
  }
}
console.log(`${groove.length * 32} combinações cidade/cena/variação verificadas contra a stable-v0.5.`);
