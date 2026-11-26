import assert from 'node:assert/strict';
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const [webDir, firmware, captureDir] = process.argv.slice(2);
assert(webDir && firmware, 'Usage: node previous-point-and-fill.mjs WEB_DIR PREVIOUS.fwsc CURRENT.fwsc');
const { jitImports } = await import(pathToFileURL(resolve(webDir, 'jit.js')));
let x;
const { instance } = await WebAssembly.instantiate(
  readFileSync(resolve(webDir, 'fm1_web.wasm')), { env: jitImports(() => x) });
x = instance.exports;
let bytes = readFileSync(firmware);
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


level(2);tap(9);turn(0,2);tap(1);tap(1);level(2);
const address=0xf6000, ss=4096;
const records=Buffer.alloc(2*(ss+4));
for(let i=0;i<2;i++){
  const off=i*(ss+4);
  records.writeUInt32LE(address+i*ss,off);
  records.set(new Uint8Array(x.memory.buffer,x.fm1_flash()+address+i*ss,ss),off+4);
}
assert(records.includes(Buffer.from('MGR6')),'old POINT 2 missing');
bytes=readFileSync(process.argv[4]);boot();
assert.equal(x.fm1_flash_restore(copy(records),records.length),0);
level(2);tap(9);turn(0,1);turn(1,1);tap(1);tap(1);
turn(0,1);tap(1);tap(1);level(1);
const raw=Buffer.from(x.memory.buffer,x.fm1_flash()+address,ss*2);
assert(raw.includes(Buffer.from('MGR6')) && [0,ss].some(o=>raw.readUInt32LE(o+12)===1 && raw.readUInt32LE(o+ss-4)===0xc0ffee01));
boot();level(2);tap(24);
for(let view=0;view<3;view++){
  const title=box(0,20),clear=area(88,22,55,14);
  x.fm1_key(11,1);level(.42);x.fm1_key(11,0);level(.05);
  assert.deepEqual(box(0,20),title);
  assert.notDeepEqual(area(88,22,55,14),clear);
  level(5);tap(8);
}
console.log('Migration POINT 2, single-point delete and held SEQ on LIST/MAP/MIXER passed.');
