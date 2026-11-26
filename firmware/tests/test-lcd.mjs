import assert from 'node:assert/strict';
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const [webDir, firmware, captureDir] = process.argv.slice(2);
assert(webDir && firmware, 'Usage: node test-lcd.mjs WEB_DIR FIRMWARE.fwsc [SCREENSHOTS]');
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
  assert(samples > seconds * 44100 * 0.75, 'Insufficient audio samples');
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

function shortEdit(settle=true) {
  x.fm1_key(6, 1);
  level(0.05);
  x.fm1_key(6, 0);
  level(settle ? 0.4 : 0.10);
}

const box = (top, height) => area(8, top, 224, height);
const sceneRows = () => box(61, 130);
const mixer = () => box(211, 24);
const headerCity = () => area(8, 22, 60, 13);

level(2);
assert(level(0.3) < 0.0001, 'Layers active at boot');
assert.equal(pixel(8, 1), pixel(0, 0), 'Unexpected header palette');
for (let row = 0; row < 6; row++)
  assert.notEqual(pixel(230, 62 + row * 22), pixel(230, 82 + row * 22), 'Missing layer row');
const idleRevision = x.fm1_lcd_revision();
level(0.4);
assert.equal(x.fm1_lcd_revision(), idleRevision, 'Idle HOME redrew without a state change');
capture('home');
const homeRows = sceneRows(), dry = mixer(), city0 = headerCity();
tap(2);
assert.deepEqual(sceneRows(), homeRows, 'FX changed the HOME list');
assert.notDeepEqual(mixer(), dry, 'FX did not replace the HOME controls');
capture('fx');
turn(2, 40, 0.9);
const fx = mixer();
tap(2);
const eqCards=mixer();
assert.notDeepEqual(eqCards,fx);
turn(2,2);
assert.notDeepEqual(mixer(),eqCards,'LO did not move');
const changedEq=mixer();
tap(8);
assert.deepEqual(mixer(),changedEq,'Switching to MAP lost the EQ bank');
const mapTitle=box(0,21);
tap(2);assert.deepEqual(mixer(),dry);
tap(2);assert.deepEqual(mixer(),fx);
assert.deepEqual(box(0,21),mapTitle,'FX left MAP');
tap(2);tap(8);
assert.deepEqual(mixer(),changedEq,'Switching to MIXER lost the EQ bank');
const mixTitle=box(0,21);
tap(2);tap(2);assert.deepEqual(box(0,21),mixTitle,'FX left MIXER');
tap(2);tap(2);tap(8);
assert.deepEqual(mixer(), dry, 'MIX was not restored');
tap(8);
assert.notDeepEqual(sceneRows(), homeRows, 'Stage Map did not open');
assert.deepEqual(mixer(), dry, 'Stage Map lost the control cards');
capture('stage');
tap(8);
assert.notDeepEqual(sceneRows(),homeRows,'MIXER did not open');
capture('mixer');
tap(8);
shortEdit();
const edit = box(60, 80);
capture('edit');
turn(2, -4);
assert.notDeepEqual(box(60, 80), edit, 'EDIT did not apply the layer control');
shortEdit();

tap(7);
const global = box(46, 185);
capture('glo');
turn(0, 1);
assert.notDeepEqual(box(46,185), global, 'SELECT did not change the GLO row');
const highlighted = box(69,20);
turn(1, 1);
assert.notDeepEqual(box(69,20), highlighted, 'ALGORITHM did not change the GLO setting');
tap(7);

tap(3);
const root = box(110, 75);
capture('scl');
turn(0, 1);
assert.notDeepEqual(box(110,75), root, 'SCL did not update the displayed scale');
tap(3);

tap(11);
capture('seq');
const kick = pixel(50, 84), hat = pixel(50, 107);
turn(0, 1);
tap(14);
assert.equal(pixel(50, 84), kick, 'Editing HAT changed the kick');
assert.notEqual(pixel(50, 107), hat, 'HAT was not edited');
const editedHat = pixel(50, 107);
tap(11);

tap(9);
const save = box(52, 135);
capture('save');
tap(1);
assert.notDeepEqual(box(52,135), save, 'Confirmation did not open');
capture('confirm');
tap(0);
assert.deepEqual(box(52,135), save, 'Confirmation was not cancelled');
const revision = x.fm1_flash_revision();
tap(1);
tap(1);
level(2);
assert.equal(x.fm1_flash_busy(), 0);
assert(x.fm1_flash_revision() > revision, 'SAVE did not write to flash');
assert.deepEqual(box(52,135), save, 'SAVE stayed on the writing screen');
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
assert.equal(pixel(50, 107), editedHat, 'HAT edit was not restored');
tap(2);
assert.deepEqual(mixer(), fx, 'FX was not restored');
tap(8);

turn(6, 1);
assert.notDeepEqual(headerCity(), city0, 'PRESETS did not switch cities');
shortEdit();
const original = box(205, 23);
shortEdit();
x.fm1_key(6, 1);
level(0.24);
turn(6, 1);
x.fm1_key(6, 0);
level(0.2);
shortEdit();
assert.notDeepEqual(box(205, 23), original, 'EDIT footer did not show the source');
shortEdit();

tap(12);
assert(level(0.5) > 0.001, 'PLAY produced no audio');
let meterLit=false;
for(let frame=0;frame<32&&!meterLit;frame++){
  level(.05);
  meterLit=area(171,68,55,6).some(pixel=>pixel===((31<<11)|(34<<5)));
}
assert(meterLit,'DRUMS meter inactive for 1.6 seconds');
tap(39);
level(1.5);
assert(level(0.3) < 0.0001, 'ALL LAYERS did not silence playback');
tap(39);
assert(level(0.5) > 0.001, 'ALL LAYERS did not restore playback');
console.log('FM-1 timed DMA display checks passed for HOME, FX, Stage Map, EDIT, GLO, SCL, SEQ, SAVE A/B and PLAY.');


shortEdit();
tap(34); // LEAD selected in EDIT without toggling the layer
const gridRow=box(206,19);
turn(1,-1);assert.notDeepEqual(box(206,19),gridRow);
turn(1,1);assert.deepEqual(box(206,19),gridRow);
const free = box(171, 32);
turn(5, 4);
turn(1, 1);
const sync = box(171, 32);
assert.notDeepEqual(sync, free, 'GHOST RATE/SYNC did not update the display');
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
turn(0, -1);
assert(box(171, 32).every((value, index) => value === sync[index]),
  'GHOST RATE/SYNC was not restored after restart');
console.log('GHOST RATE/SYNC updated and restored on the emulated FM-1.');

tap(7);
turn(0, 2);
const hold = box(92, 20);
turn(1, 1);
assert.notDeepEqual(box(92, 20), hold, 'BASS MODE did not switch to ALIEN');
turn(0, 2);
const beforeCreate = headerCity();
tap(1);
tap(0);
assert.deepEqual(headerCity(), beforeCreate, 'Cancelling generation changed the city');
const names = new Set();
for (let i = 0; i < 5; ++i) {
  tap(1);
  tap(1);
  const identity = Array.from(headerCity()).join(',');
  assert(!names.has(identity), 'Duplicate city recipe');
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
assert(savedEight >= 0, 'Eight-city record was not saved');
for (let i = 0; i < 2; i++)
  restore.set(citySectors.subarray(i * 4096, (i + 1) * 4096), i * (4096 + 4) + 4);
boot();
assert.equal(x.fm1_flash_restore(copy(restore), restore.length), 0);
level(2);
assert.deepEqual(headerCity(), generatedCity, 'SAVE did not restore the generated city');
tap(7);
turn(0, 4);
tap(1);
const limit = box(82, 104);
tap(1);
assert.notDeepEqual(box(82, 104), limit, 'City limit notice did not close');
tap(0);
turn(0, 1);
tap(1);
tap(1);
assert.deepEqual(headerCity(), city0, 'Deleting the city did not return to Búzios');
tap(7);
turn(0, 1);
turn(1, 1);
tap(1);
tap(1);
assert.deepEqual(headerCity(), city0, 'Factory reset did not select Búzios');
console.log('GLO checks passed for ALIEN, city generation, cancellation, capacity, deletion and reset.');

for (const key of [24, 27, 29, 31, 34, 36]) {
  tap(key);
  const layerHeader = box(4, 18);
  x.fm1_key(6, 1);
  level(0.22);
  x.fm1_key(6, 0);
  level(0.2);
  assert.notDeepEqual(box(4, 18), layerHeader, 'EDIT did not open after a longer button press');
  shortEdit();
}
console.log('EDIT opens on the selected layer for all six layers.');

boot();level(2);
tap(8);tap(8);
const strips=box(58,143);
tap(13);assert.notDeepEqual(box(58,143),strips,'REC did not open calibration');
const zeroCounts=box(102,98);turn(2,4);
assert.notDeepEqual(box(102,98),zeroCounts,'Calibration did not count detents');
tap(13);tap(8);tap(11);
const initialStep=pixel(50,84);
x.fm1_key(14,1);level(.4);const neutralShape=box(218,18);turn(2,2);
assert.notDeepEqual(box(218,18),neutralShape,'VEL did not change on the held step');
x.fm1_key(14,0);level(.25);assert.equal(pixel(50,84),initialStep,'Holding the step toggled the hit');
turn(2,1);const draftReady=box(219,18);turn(3,1);assert.notDeepEqual(box(219,18),draftReady,'Draft preview did not start');
turn(4,1);assert.notDeepEqual(box(219,18),draftReady,'Draft remained active after applying');
console.log('Calibration, held-step editing, generation, preview and apply checks passed.');

boot();level(1.8);
tap(10);
assert(level(.35)<.0001,'ARP activated a layer');
shortEdit();
const bassEditTitle=box(0,21);
const tempo=area(175,23,57,14);
tap(1);
const ambEditTitle=box(0,21);
assert.notDeepEqual(ambEditTitle,bassEditTitle,'OCT+ did not advance from BASS to AMB');
assert.deepEqual(tempo,area(175,23,57,14),'OCT+ changed BPM in EDIT');
tap(0);assert.deepEqual(box(0,21),bassEditTitle);
tap(0);assert.notDeepEqual(box(0,21),ambEditTitle);
shortEdit();tap(9);
const allScope=box(52,122);
turn(1,1);assert.notDeepEqual(box(52,122),allScope,'SAVE did not show CURRENT CITY');
turn(1,-1);assert.deepEqual(box(52,122),allScope,'SAVE did not return to ALL CITIES');
console.log('ARP, OCT navigation in EDIT and SAVE scope checks passed.');

boot();level(2);
const homeTitle=box(0,21);
shortEdit(false);
assert.notDeepEqual(box(0,21),homeTitle,'EDIT did not open the parameters');
shortEdit();
assert.deepEqual(box(0,21),homeTitle,'Second EDIT press did not return to HOME');
