import assert from 'node:assert/strict';
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const [webDir, firmware, captureDir] = process.argv.slice(2);
assert(webDir && firmware, 'Uso: node test-lcd.mjs DIRETORIO_WEB ARQUIVO.fwsc [CAPTURAS]');
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
  x.fm1_lcd_timed?.(1);
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

function lcd() {
  return new Uint16Array(x.memory.buffer, x.fm1_lcd(), 240 * 240);
}

function area(left, top, width, height) {
  const pixels = lcd();
  return Uint16Array.from({length: width * height}, (_, i) =>
    pixels[(top + Math.floor(i / width)) * 240 + left + (i % width)]);
}

function pixel(x, y) {
  return lcd()[y * 240 + x];
}

function capture(name) {
  if (!captureDir) return;
  mkdirSync(captureDir, {recursive:true});
  const rgb = Buffer.alloc(240 * 240 * 3);
  lcd().forEach((p, i) => {
    rgb[i * 3] = (p >> 11) * 255 / 31;
    rgb[i * 3 + 1] = ((p >> 5) & 63) * 255 / 63;
    rgb[i * 3 + 2] = (p & 31) * 255 / 31;
  });
  writeFileSync(resolve(captureDir, `native-${name}.ppm`),
    Buffer.concat([Buffer.from('P6\n240 240\n255\n'), rgb]));
}

function turn(encoder, delta, seconds = 0.4) {
  x.fm1_enc(encoder, delta);
  level(seconds);
}

function shortEdit() {
  x.fm1_key(6, 1);
  level(0.05);
  x.fm1_key(6, 0);
  level(0.2);
}

const box = (top, height) => area(8, top, 224, height);
const sceneRows = () => box(61, 130);
const mixer = () => box(211, 24);
const headerCity = () => area(8, 22, 60, 13);

level(2);
assert(level(0.3) < 0.0001, 'camadas ativas no boot');
assert.equal(pixel(8, 1), pixel(0, 0), 'paleta do cabeçalho');
for (let row = 0; row < 6; row++)
  assert.notEqual(pixel(230, 62 + row * 22), pixel(230, 82 + row * 22), 'linha de camada ausente');
const idleRevision = x.fm1_lcd_revision();
level(0.4);
assert.equal(x.fm1_lcd_revision(), idleRevision, 'HOME em repouso redesenha sem mudança');
capture('home');
const homeRows = sceneRows(), dry = mixer(), city0 = headerCity();
tap(2);
assert.deepEqual(sceneRows(), homeRows, 'FX mudou a lista da HOME');
assert.notDeepEqual(mixer(), dry, 'FX não substituiu os cartões da HOME');
capture('fx');
turn(2, 40, 0.9);
const fx = mixer();
tap(2);
assert.deepEqual(mixer(), dry, 'FX não voltou ao mixer');
tap(8);
assert.notDeepEqual(sceneRows(), homeRows, 'Stage Map não abriu');
assert.deepEqual(mixer(), dry, 'Stage Map não manteve os cartões');
capture('stage');
tap(8);
assert.notDeepEqual(sceneRows(),homeRows,'MIXER não abriu');
capture('mixer');
tap(8);
shortEdit();
const edit = box(60, 80);
capture('edit');
turn(2, -4);
assert.notDeepEqual(box(60, 80), edit, 'EDIT não aplicou o controle da camada');
shortEdit();

tap(7);
const global = box(46, 185);
capture('glo');
turn(0, 1);
assert.notDeepEqual(box(46,185), global, 'SELECT não mudou a linha de GLO');
const highlighted = box(69,20);
turn(1, 1);
assert.notDeepEqual(box(69,20), highlighted, 'ALGORITHM não mudou a opção de GLO');
tap(7);

tap(3);
const root = box(110, 75);
capture('scl');
turn(0, 1);
assert.notDeepEqual(box(110,75), root, 'SCL não mudou a escala desenhada');
tap(3);

tap(11);
capture('seq');
const kick = pixel(50, 84), hat = pixel(50, 107);
turn(0, 1);
tap(14);
assert.equal(pixel(50, 84), kick, 'editou o kick com HAT selecionado');
assert.notEqual(pixel(50, 107), hat, 'não editou HAT');
const editedHat = pixel(50, 107);
tap(11);

tap(9);
const save = box(52, 135);
capture('save');
tap(1);
assert.notDeepEqual(box(52,135), save, 'não abriu a confirmação');
capture('confirm');
tap(0);
assert.deepEqual(box(52,135), save, 'não cancelou a confirmação');
const revision = x.fm1_flash_revision();
tap(1);
tap(1);
level(2);
assert.equal(x.fm1_flash_busy(), 0);
assert(x.fm1_flash_revision() > revision, 'SAVE não gravou');
assert.deepEqual(box(52,135), save, 'SAVE ficou preso na tela de gravação');
const sectors = new Uint8Array(x.memory.buffer, x.fm1_flash() + 0xB1000, 8192);
assert(Buffer.from(sectors).includes(Buffer.from('MGR6')));
const restore = Buffer.alloc(2 * (4096 + 4));
for (let i = 0; i < 2; i++) {
  const index = i * (4096 + 4);
  restore.writeUInt32LE(0xB1000 + i * 4096, index);
  restore.set(sectors.subarray(i * 4096, (i + 1) * 4096), index + 4);
}
boot();
assert.equal(x.fm1_flash_restore(copy(restore), restore.length), 0);
level(2);
tap(11);
assert.equal(pixel(50, 107), editedHat, 'edição de HAT não voltou');
tap(2);
assert.deepEqual(mixer(), fx, 'FX não voltou');
tap(8);

turn(6, 1);
assert.notDeepEqual(headerCity(), city0, 'cidade não mudou com PRESETS');
shortEdit();
const original = box(205, 23);
shortEdit();
x.fm1_key(6, 1);
level(0.24);
turn(6, 1);
x.fm1_key(6, 0);
level(0.2);
shortEdit();
assert.notDeepEqual(box(205, 23), original, 'rodapé de EDIT não indicou a origem');
shortEdit();

tap(12);
assert(level(0.5) > 0.001, 'PLAY sem áudio');
let meterLit=false;
for(let frame=0;frame<32&&!meterLit;frame++){
  level(.05);
  meterLit=area(171,68,55,6).some(pixel=>pixel===((31<<11)|(34<<5)));
}
assert(meterLit,'medidor de DRUMS sem atividade em 1,6 segundos');
tap(39);
level(1.5);
assert(level(0.3) < 0.0001, 'ALL LAYERS não silenciou');
tap(39);
assert(level(0.5) > 0.001, 'ALL LAYERS não restaurou o áudio');
console.log('LCD FM-1 com DMA temporizado: HOME, FX, Stage Map, EDIT, GLO, SCL, SEQ, SAVE A/B e PLAY passaram.');


shortEdit();
tap(34); // LEAD selected in EDIT without toggling the layer
const free = box(171, 32);
turn(5, 4);
turn(1, 1);
const sync = box(171, 32);
assert.notDeepEqual(sync, free, 'GHOST RATE/SYNC não atualizou o LCD');
tap(9);
tap(1);
tap(1);
level(2);
const leadSectors = new Uint8Array(x.memory.buffer, x.fm1_flash() + 0xB1000, 8192).slice();
for (let i = 0; i < 2; i++)
  restore.set(leadSectors.subarray(i * 4096, (i + 1) * 4096), i * (4096 + 4) + 4);
boot();
assert.equal(x.fm1_flash_restore(copy(restore), restore.length), 0);
level(2);
shortEdit();
tap(34);
turn(0, 3);
assert(box(171, 32).every((value, index) => value === sync[index]),
  'GHOST RATE/SYNC não voltou após reiniciar');
console.log('GHOST RATE/SYNC atualizado e recuperado no FM-1 emulado.');

tap(7);
turn(0, 2);
const hold = box(92, 20);
turn(1, 1);
assert.notDeepEqual(box(92, 20), hold, 'BASS MODE não mudou para ALIEN');
turn(0, 2);
const beforeCreate = headerCity();
tap(1);
tap(0);
assert.deepEqual(headerCity(), beforeCreate, 'Cancelar geração mudou a cidade');
const names = new Set();
for (let i = 0; i < 5; ++i) {
  tap(1);
  tap(1);
  const identity = Array.from(headerCity()).join(',');
  assert(!names.has(identity), 'receita de cidade repetida');
  names.add(identity);
  tap(7);
}
tap(7);
const generatedCity = headerCity();
tap(9);
tap(1);
tap(1);
level(2);
const citySectors = new Uint8Array(x.memory.buffer, x.fm1_flash() + 0xB1000, 8192).slice();
const savedEight = Buffer.from(citySectors).indexOf(Buffer.from([0x4d,0x47,0x52,0x36,7,8]));
assert(savedEight >= 0, 'registro com oito cidades não foi gravado');
for (let i = 0; i < 2; i++)
  restore.set(citySectors.subarray(i * 4096, (i + 1) * 4096), i * (4096 + 4) + 4);
boot();
assert.equal(x.fm1_flash_restore(copy(restore), restore.length), 0);
level(2);
assert.deepEqual(headerCity(), generatedCity, 'cidade gerada não voltou do SAVE');
tap(7);
turn(0, 4);
tap(1);
const limit = box(82, 104);
tap(1);
assert.notDeepEqual(box(82, 104), limit, 'Aviso de limite não fechou');
tap(0);
turn(0, 1);
tap(1);
tap(1);
assert.deepEqual(headerCity(), city0, 'Excluir cidade não voltou para Búzios');
tap(7);
turn(0, 1);
turn(1, 1);
tap(1);
tap(1);
assert.deepEqual(headerCity(), city0, 'Reset geral não selecionou Búzios');
console.log('GLO: ALIEN, criação/cancelamento, cinco cidades, limite, exclusão e reset passaram.');

for (const key of [24, 27, 29, 31, 34, 36]) {
  tap(key);
  const layerHeader = box(4, 18);
  x.fm1_key(6, 1);
  level(0.22);
  x.fm1_key(6, 0);
  level(0.2);
  assert.notDeepEqual(box(4, 18), layerHeader, 'EDIT não abriu com toque físico mais longo');
  shortEdit();
}
console.log('EDIT abre na camada selecionada nas seis camadas.');

boot();level(2);
tap(8);tap(8);
const strips=box(58,143);
tap(13);assert.notDeepEqual(box(58,143),strips,'REC não abriu calibração');
const zeroCounts=box(102,98);turn(2,4);
assert.notDeepEqual(box(102,98),zeroCounts,'calibração não contou detentes');
tap(13);tap(8);tap(11);
const initialStep=pixel(50,84);
x.fm1_key(14,1);level(.4);const neutralShape=box(218,18);turn(2,2);
assert.notDeepEqual(box(218,18),neutralShape,'VEL não mudou no passo segurado');
x.fm1_key(14,0);level(.25);assert.equal(pixel(50,84),initialStep,'segurar trocou o hit');
turn(2,1);const draftReady=box(219,18);turn(3,1);assert.notDeepEqual(box(219,18),draftReady,'prévia não entrou');
turn(4,1);assert.notDeepEqual(box(219,18),draftReady,'rascunho ficou ativo depois de aplicar');
console.log('Calibração, edição de passo segurado, geração, prévia e aplicação no painel passaram.');
