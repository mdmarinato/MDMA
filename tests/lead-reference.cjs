const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const {execFileSync} = require('node:child_process');

const [htmlPath, executable, profile = "0", mode] = process.argv.slice(2);
const html = fs.readFileSync(htmlPath, 'utf8');
const engine = html.match(/<script[^>]+id="eng"[^>]*>([\s\S]*?)<\/script>/)[1];
const hook = `E.leadReference = (i) => {
  if (i === 0) { applyKit(${[0, 2, 1][Number(profile)]}); P.leadAlien = ${mode === 'alien'}; }

  ghostOn = i < 176400;
  if (i === 44100) handOn(0);
  if (i === 88200) handOff();
  if (ghostOn && i % 5011 === 0) {
    gs = Math.floor(i / 5011);
    ghostUpdate(5011 / sr);
    gs++;
  }
  if (i % 480 === 0) ghostUpdate(480 / sr);
  return [ldRun(GHOST) + ldRun(HAND), ghostI, GHOST.cT, GHOST.g, HAND.g];
};
return E;`;
assert(engine.includes('return E;'));
const box = {exports: {}};
vm.runInThisContext('(function(module){' + engine.replace('return E;', hook) + '\n})')(box);
const stable = box.exports(44100);
const native = execFileSync(executable, mode ? [profile, mode] : [profile], {encoding: 'utf8', maxBuffer: 20 * 1024 * 1024})
  .trim().split('\n');
assert.equal(native.length, 220500);

let degrees = 0, cutoffError = 0, gainError = 0, signal = 0, error = 0;
for (let i = 0; i < native.length; ++i) {
  const [sample, degree, cutoff, ghost, hand] = native[i].split(',').map(Number);
  const [ref, targetDegree, targetCutoff, ghostGain, handGain] = stable.leadReference(i);
  degrees += degree === targetDegree;
  cutoffError = Math.max(cutoffError, Math.abs(cutoff / 256 - targetCutoff));
  gainError = Math.max(gainError, Math.abs(ghost / 2**24 - ghostGain),
    Math.abs(hand / 2**24 - handGain));
  signal += ref ** 2;
  error += (sample / 32768 - ref) ** 2;
}
const match = degrees / native.length;
const normalizedError = Math.sqrt(error / signal);
console.log({pitchDegreeMatch: match, peakCutoffErrorHz: cutoffError,
  peakEnvelopeError: gainError, normalizedPcmError: normalizedError});
assert(match > 0.99, 'Ghost degree drift diverged');
assert(cutoffError < 8, 'Ghost cutoff drift diverged');
assert(gainError < 0.01, 'HAND/GHOST envelope diverged');
assert(normalizedError < 0.01, "isolated lead PCM diverged");
