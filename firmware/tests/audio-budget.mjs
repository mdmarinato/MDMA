import fs from 'node:fs';
import {resolve} from 'node:path';
import {pathToFileURL} from 'node:url';
const [dist,fw,mhz='100']=process.argv.slice(2);
const {jitImports}=await import(pathToFileURL(resolve(dist,'jit.js')));
let x;
x=(await WebAssembly.instantiate(fs.readFileSync(resolve(dist,'fm1_web.wasm')),{env:jitImports(()=>x)})).instance.exports;
const bytes=fs.readFileSync(fw),ptr=x.fm1_alloc(bytes.length);new Uint8Array(x.memory.buffer,ptr,bytes.length).set(bytes);
if(x.fm1_boot_fwsc(ptr,bytes.length,0,0,+mhz))throw Error('boot');
x.fm1_jit(1,16);x.fm1_ahead(1024);x.fm1_lcd_timed(1);
function render(sec){for(let i=0;i<sec*44100;i+=128){x.fm1_render(128,50000000);if(x.fm1_halted())throw Error('halted')}}
render(1.5);
let mem=Buffer.from(x.memory.buffer),marker=Buffer.from('0b111da942a711f7','hex');
let idx=-1,found=-1;
while((idx=mem.indexOf(marker,idx+1))>=0)if(mem.readUInt32LE(idx+12)>0)found=idx;
if(found<0)throw Error('no live timer');
for(const [label,key] of [['drums',24],['drums+bass',27],['+amb',29],['+pluck',31],['+lead',34],['seis',36],['GROOVE',17],['TENSION',19],['PEAK',22]]){
 x.fm1_key(key,1);render(.15);x.fm1_key(key,0);render([17,19,22].includes(key)?2.1:.2);
 mem=Buffer.from(x.memory.buffer);mem.writeUInt32LE(0,found+8);
 const overruns=mem.readUInt32LE(found+16),blocks=mem.readUInt32LE(found+12);
 render(4);mem=Buffer.from(x.memory.buffer);
 const late=mem.readUInt32LE(found+16)-overruns;
 console.log(label,{maxUs:mem.readUInt32LE(found+8)/24,blocks:mem.readUInt32LE(found+12)-blocks,overruns:mem.readUInt32LE(found+16)-overruns,budgetUs:256e6/44100});
 if(late || mem.readUInt32LE(found+12)-blocks<680)process.exitCode=1;
}
