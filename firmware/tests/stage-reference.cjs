const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const [html,exe]=process.argv.slice(2);
const engine=fs.readFileSync(html,'utf8').match(/<script[^>]+id="eng"[^>]*>([\s\S]*?)<\/script>/)[1];
const names=['drums','bass','pluck','lead','amb','sparks'];
const hook=`E.stageRef=(layer,index,step)=>{
 stageEnabled=playing=true;LY[layer]=1;mute[layer]=false;gs=step/8;stepCnt=0;stepLen=5011;
 ghostT=step/8;ghostAuto=.5;sparkStageAt=now;sparkStageSide=step&1?1:-1;sparkStageDuration=1000;
 const m=homeMixer[layer];m.target=[.8,.5,(index%2?84:44)/128,.5];updateStage(layer,m);
 return [m.stagePan*128,m.stageFilter*128,...[m.stageRev,m.stageDelay,m.stagePhaser].map(x=>Math.max(0,Math.min(127,55*(1+x))))];
};return E;`;
const ctx={exports:{}};
vm.runInThisContext('(function(module){'+engine.replace('return E;',hook)+'\n})')(ctx);
const stable=ctx.exports(44100);
const rows=execFileSync(exe,{encoding:'utf8'}).trim().split('\n');
assert.equal(rows.length,128*6);
let worst=0;
for(let i=0;i<rows.length;i++){
 const l=i%6,[mix,fx]=rows[i].split(' ').map(Number);
 const actual=[mix>>>8&127,mix>>>16&127,fx&127,fx>>>8&127,fx>>>16&127];
 stable.stageRef(names[l],l,Math.floor(i/6)).forEach((v,k)=>{worst=Math.max(worst,Math.abs(actual[k]-v))});
}
console.log('Stage Map maximum control-step difference across 768 positions:',worst);
assert(worst<2);
