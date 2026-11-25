import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const [webDir, firmware] = process.argv.slice(2);
assert(webDir && firmware, 'Uso: node test-port.mjs DIRETORIO_WEB ARQUIVO.fwsc');
const { jitImports } = await import(pathToFileURL(resolve(webDir, 'jit.js')));
let x;
const { instance } = await WebAssembly.instantiate(
  readFileSync(resolve(webDir, 'fm1_web.wasm')), { env: jitImports(() => x) });
x = instance.exports;
const bytes = readFileSync(firmware);
function copy(data) {
  const address = x.fm1_alloc(data.length);
  new Uint8Array(x.memory.buffer, address, data.length).set(data);
  return address;
}
function boot() {
  assert.equal(x.fm1_boot_fwsc(copy(bytes), bytes.length, 0, 0, 100), 0);
  x.fm1_jit(1, 16);
  x.fm1_ahead(1024);
}
boot();

function level(seconds) {
  let sum = 0;
  let samples = 0;
  for (let i = 0; i < seconds * 44100; i += 128) {
    const n = x.fm1_render(128, 50_000_000);
    const pcm = new Float32Array(x.memory.buffer, x.fm1_out_l(), n);
    for (const value of pcm) sum += value * value;
    samples += n;
  }
  assert.equal(x.fm1_halted(), 0);
  assert(samples > seconds * 44100 * 0.75, 'áudio insuficiente');
  return Math.sqrt(sum / samples);
}

function tap(id) {
  x.fm1_key(id, 1);
  level(0.12);
  x.fm1_key(id, 0);
  level(0.12);
}

function panel() {
  const pixels = new Uint16Array(x.memory.buffer, x.fm1_lcd(), 240 * 240)
    .slice(136 * 240, 192 * 240);
  for (let row = 8; row < 16; ++row) pixels.fill(0, row * 240 + 222, row * 240 + 230);
  return pixels;
}

level(2);
assert(level(0.4) < 0.0001, 'camadas ativas no boot');
const home = panel();
assert(home.some(Boolean), 'HOME vazio');
tap(12);
assert(level(0.5) > 0.001, 'PLAY não produziu áudio');
tap(39); // F#5: ALL LAYERS em 14 + 25
level(1.5);
assert(level(0.4) < 0.0001, 'ALL LAYERS não silenciou');
x.fm1_enc(2, -10);
level(0.2);
x.fm1_enc(2, 5);
assert(level(0.4) < 0.0001, 'VOL religou a camada durante ALL OFF');
tap(39);
assert(level(0.5) > 0.001, 'ALL LAYERS não restaurou o áudio');
tap(2);
const fx = panel();
assert.notDeepEqual(fx, home, 'FX não abriu');
x.fm1_enc(2, 40);
level(0.9);
const editedFx = panel();
assert.notDeepEqual(editedFx, fx, 'indicador de FX não acompanhou o encoder');
tap(11);
const seq = panel();
tap(14);
const editedSeq = panel();
assert.notDeepEqual(editedSeq, seq, 'primeiro passo do kick não mudou');
tap(9);
const save = panel();
tap(1);
assert.notDeepEqual(panel(), save, 'OCT+ não abriu a confirmação');
tap(0);
assert.deepEqual(panel(), save, 'OCT- não cancelou a confirmação');
console.log('Boot silencioso, PLAY, ALL LAYERS, VOL, FX, SEQ e cancelamento SAVE: OK');

const revision = x.fm1_flash_revision();
tap(1);
tap(1);
level(2);
assert.equal(x.fm1_flash_busy(), 0, 'flash ainda ocupado');
assert(x.fm1_flash_revision() > revision, 'SAVE confirmado não gravou na flash');
const slot = new Uint8Array(x.memory.buffer, x.fm1_flash() + 0xB1000, 8192);
assert(Buffer.from(slot).includes(Buffer.from('MGR3')), 'registro MGR3 ausente');
console.log('SAVE confirmado gravou MGR3 na flash do emulador: OK');

const restore = Buffer.alloc(2 * (4096 + 4));
for (let i = 0; i < 2; i++) {
  const at = i * (4096 + 4);
  restore.writeUInt32LE(0xB1000 + i * 4096, at);
  restore.set(slot.subarray(i * 4096, (i + 1) * 4096), at + 4);
}
boot();
assert.equal(x.fm1_flash_restore(copy(restore), restore.length), 0);
level(2);
tap(11);
assert.deepEqual(panel(), editedSeq, 'o kick editado não voltou após reiniciar');
tap(2);
assert.deepEqual(panel(), editedFx, 'o valor de FX não voltou após reiniciar');
console.log('Reinicialização recuperou a edição de kick e o valor de FX: OK');

function cityStatus() {
  return new Uint16Array(x.memory.buffer, x.fm1_lcd(), 240 * 240)
    .slice(196 * 240, 212 * 240);
}
tap(8);
const first = cityStatus();
x.fm1_enc(6, 1);
level(0.2);
const second = cityStatus();
assert.notDeepEqual(second, first, 'primeira troca de cidade parada');
x.fm1_enc(6, 1);
level(0.2);
const third = cityStatus();
assert.notDeepEqual(third, second, 'segunda troca de cidade parada');
assert.notDeepEqual(third, first, 'a terceira cidade está ausente');
tap(12);
x.fm1_enc(6, 1);
level(0.2);
assert.deepEqual(cityStatus(), third, 'a cidade mudou antes do compasso');
level(2.1);
assert.deepEqual(cityStatus(), first, 'a cidade não mudou na virada');
tap(8);
const map = panel();
tap(3);
const scale = panel();
assert.notDeepEqual(map, scale, 'SCL não abriu');
x.fm1_enc(0, 1);
level(0.2);
assert.notDeepEqual(panel(), scale, 'SELECT não mudou o tom em SCL');
console.log('Três cidades, troca quantizada, Stage Map e SCL: OK');
tap(8);
tap(2);
const selectedFx = panel();
x.fm1_enc(0, 1);
level(0.35);
const otherFx = panel();
assert.notDeepEqual(otherFx, selectedFx, 'FX não acompanhou a camada selecionada');
x.fm1_enc(2, 15);
level(0.6);
assert.notDeepEqual(panel(), otherFx, 'FX da segunda camada não mudou');
x.fm1_enc(0, -1);
level(0.35);
assert.deepEqual(panel(), selectedFx, 'FX da primeira camada foi alterado por engano');
tap(8);
const beforeOrigin = cityStatus();
const homeMix = panel();
x.fm1_key(6, 1);
level(0.24);
x.fm1_enc(6, 1);
level(0.3);
x.fm1_key(6, 0);
level(0.2);
assert.deepEqual(cityStatus(), beforeOrigin, 'origem por camada trocou a cidade global');
assert.deepEqual(panel(), homeMix, 'origem por camada alterou o mixer');
x.fm1_key(6, 1);
level(0.05);
x.fm1_key(6, 0);
level(0.16);
assert.notDeepEqual(panel(), homeMix, 'toque curto de EDIT não abriu a edição');
const editWithOrigin = panel();
x.fm1_key(6, 1);
level(0.05);
x.fm1_key(6, 0);
level(0.16);
assert.deepEqual(panel(), homeMix, 'segundo EDIT não voltou à HOME');
x.fm1_key(6, 1);
level(0.24);
x.fm1_enc(6, -1);
level(0.3);
x.fm1_key(6, 0);
level(0.2);
x.fm1_key(6, 1);
level(0.05);
x.fm1_key(6, 0);
level(0.16);
assert.notDeepEqual(panel(), editWithOrigin, 'origem não apareceu no LCD');
console.log('FX independente e origem por camada preservando o mixer: OK');

boot();
level(2);
tap(39);
assert(level(0.75) > 0.001, 'mix de seis camadas indisponível');
x.fm1_key(14, 1);
assert(level(0.75) > 0.001, 'mix com LEAD manual indisponível');
x.fm1_key(14, 0);
assert(level(0.75) > 0.001, 'mix parou após soltar o LEAD');
console.log('Seis camadas com LEAD manual e soltura no emulador: OK');
