#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define FL_IN(a,n,lo,hi) ((a)>=(lo) && (a)<=(hi) && (n)<=(hi)-(a))
#define FL_GUARD(a,n) FL_IN(a,n,0xB1000u,0xB3000u)
#define FONT_S 0
#define C_OFF 0
#define LEAD_CUTOFF_LAST 32
static bool flash_ok=true;
static struct {uint32_t notes;} fm1_in;
static uint8_t flash[8192];
static unsigned erases;
static char shown[32];
static int32_t lead_cutoff_position=14;
static void lead_cutoff_turn(int32_t delta){lead_cutoff_position+=delta;}
static int st_read(uint32_t a,void*p,uint32_t n){assert(FL_GUARD(a,n));memcpy(p,flash+a-0xB1000u,n);return 0;}
static int fl_write(uint32_t a,const void*p,uint32_t n){assert(FL_GUARD(a,n));for(uint32_t i=0;i<n;i++)flash[a-0xB1000u+i]&=((const uint8_t*)p)[i];return 0;}
static int fl_erase4k(uint32_t a,uint32_t*t){assert(FL_GUARD(a,4096u));*t=0;memset(flash+a-0xB1000u,255,4096);++erases;return 0;}
enum { BTN_HOME, BTN_PLAY, BTN_FX, BTN_SEQ, BTN_EDIT, BTN_LFO, BTN_ENV, BTN_ARP };
#include "mdma_groove.c"
static bool quiet(void){return !groove_enabled && !groove_sounding;}
#define draw_text_box(x,y,w,f,t,c,a) snprintf(shown,sizeof shown,"%s",t)
#include "mdma_save_probe.c"
static void boot(void){groove_save_ready=false;active_city=0;lead_cutoff_position=14;save_probe_boot();}
static void service(void){for(unsigned i=0;i<12;i++)save_probe_poll(false);}
int main(void){
    panel_defaults();
    uint8_t legacy[GROOVE_LEGACY]={ 'M','G','R','1',0,1,0,0,132,0,0,80,80,50,14,132,0,0,80,80,50,14 };
    assert(groove_decode(legacy,sizeof legacy,true)&&panel_fx[0][0][FX_DLY]==127);
    uint8_t v2[GROOVE_V2]={ 'M','G','R','2',1,0,0,0,132,0,0,80,80,50,14,132,0,4,80,80,50,14 };
    v2[GROOVE_LEGACY + 92 + 24 + FX_DLY] = 80;
    assert(groove_decode(v2,sizeof v2,true));
    assert(active_city==1 && layer_variation[1][5]==4 && layer_origin[1][5]==1);
    assert(panel_fx[1][0][FX_DLY]==80 && panel_fx[1][5][FX_DLY]==80);
    active_city=0;
    panel_defaults();
    int32_t l_mix=0,r_mix=0;
    panel_mix[0][0][MIX_PAN]=0;panel_publish();
    qa_layer(0,1000,1000,&l_mix,&r_mix);assert(l_mix==1000&&r_mix==0);
    panel_defaults();

    int32_t stereo[256]={0};
    groove_publish();
    groove_enabled=GROOVE_ALL;qa_source=1;qa_running=1;qa_start_serial++;
    groove_audio(stereo,128,false);assert(groove_step==0);
    qa_ticks=1;groove_audio(stereo,128,false);assert(groove_step==1);
    groove_audio(stereo,128,false);assert(groove_step==1);
    qa_ticks=2;groove_audio(stereo,128,false);assert(groove_step==2);
    qa_source=qa_running=groove_enabled=0;
    for(unsigned i=0;i<1500;i++){memset(stereo,0,sizeof stereo);groove_audio(stereo,128,false);}
    assert(!groove_sounding);
    int32_t l=4000,r=0;qa_echo(&layer_effects[0],&l,&r);assert(l==4000&&r==0);
    for(unsigned i=1;i<4096;i++){l=r=0;qa_echo(&layer_effects[0],&l,&r);}
    l=r=0;qa_echo(&layer_effects[0],&l,&r);assert(l==500&&r==0);
    puts("external step and echo delay verified");
    groove_edits[0].scene=1;
    groove_publish();
    for(unsigned layer=1;layer<=GROOVE_SPARKS;layer<<=1) {
        groove_enabled=layer;
        int32_t frames[256];
        bool sounded=false;
        for(unsigned block=0;block<800;block++) {
            memset(frames,0,sizeof frames);
            groove_audio(frames,128,false);
            for(unsigned i=0;i<256;i++) {
                assert(frames[i]>-32768 && frames[i]<32768);
                sounded |= frames[i]!=0;
            }
        }
        assert(sounded);
        groove_enabled=0;
        for(unsigned block=0;block<1500;block++) {
            memset(frames,0,sizeof frames);
            groove_audio(frames,128,false);
        }
        assert(!groove_sounding);
    }
    puts("six isolated voices produce PCM and return to silence");
    for (unsigned layer=0;layer<LAYER_COUNT;layer++)
        for(unsigned k=0;k<4;k++) panel_fx[0][layer][k]=127;
    panel_publish();
    groove_enabled=GROOVE_ALL;
    int32_t stressed[256];int32_t peak=0;
    for(unsigned b=0;b<1200;b++){
        memset(stressed,0,sizeof stressed);
        groove_audio(stressed,128,false);
        for(unsigned i=0;i<256;i++){
            const int32_t a=stressed[i]<0?-stressed[i]:stressed[i];
            if(a>peak)peak=a;
        }
    }
    assert(peak>0 && peak<32768);
    printf("six layers + four layer FX peak: %d\n",peak);
    groove_enabled=0;
    for(unsigned b=0;b<1500;b++){memset(stressed,0,sizeof stressed);groove_audio(stressed,128,false);}
    assert(!groove_sounding);
    panel_defaults();

    groove_buttons(1u << BTN_HOME);assert(groove_enabled==GROOVE_ALL);
    groove_buttons(1u << BTN_HOME);assert(!groove_enabled);
    memset(flash,255,sizeof flash);boot();assert(!strcmp(shown,"NO GROOVE SAVE"));
    groove_edits[0].scene=2;layer_variation[0][0]=4;
    groove_edits[0].bpm=groove_adjust(132,8,60,240);groove_edits[0].bass=43;
    groove_publish();
    int32_t audio[256]={0};
    groove_enabled=3;groove_audio(audio,128,false);
    assert(groove_sounding);
    save_probe_poll(true);service();assert(!erases&&!strcmp(shown,"STOPPING FOR SAVE"));
    for(unsigned i=0;i<1500;i++) {memset(audio,0,sizeof audio);groove_audio(audio,128,false);}
    service();assert(erases==1&&!strcmp(shown,"GROOVE SAVED"));
    city_select(1, false);groove_edits[1].bpm=118;groove_edits[1].tone=77;panel_fx[1][0][FX_DLY]=100;panel_fx[1][0][FX_REV]=88;panel_mix[1][3][MIX_FLT]=98;panel_kick[1][5]=0x4001;
    layer_variation[1][3]=7;layer_origin[1][3]=2;
    panel_drum[1][4][7]=0x8004;city_tonic[1]=11;city_major[1]=1;city_swing[1]=22;
    panel_fx[1][3][FX_SAT]=67;
    lead_cutoff_position=21;save_probe_poll(true);service();assert(erases==2);
    memset(groove_edits,0,sizeof groove_edits);
    memset(layer_variation,0,sizeof layer_variation);
    memset(layer_origin,0,sizeof layer_origin);
    memset(panel_fx,0,sizeof panel_fx);
    memset(city_tonic,0,sizeof city_tonic);
    memset(city_major,0,sizeof city_major);
    memset(city_swing,0,sizeof city_swing);
    panel_drum[1][4][7]=0;
    boot();
    assert(!strcmp(shown,"RIO    S1 V5 BPM 118"));
    assert(active_city==1 && groove_edits[1].tone==77 && lead_cutoff_position==21);
    assert(panel_fx[1][0][FX_DLY]==100 && panel_fx[1][0][FX_REV]==88);
    assert(panel_mix[1][3][MIX_FLT]==98 && panel_kick[1][5]==0x4001);
    assert(layer_variation[1][3]==7 && layer_origin[1][3]==2);
    assert(panel_drum[1][4][7]==0x8004 && city_tonic[1]==11 && city_major[1]==1 && city_swing[1]==22);
    assert(panel_fx[1][3][FX_SAT]==67 && panel_fx[1][0][FX_SAT]==0);
    assert(groove_edits[0].scene==2 && layer_variation[0][0]==4 && groove_edits[0].bass==43);
    assert(groove_edits[0].bpm==140);
    flash[4096+20]^=1;boot();assert(active_city==0 && groove_edits[0].bpm==140);
    save_probe_poll(true);assert(!strcmp(shown,"SAVE BLOCKED")&&erases==2);
    puts("groove: silence gate, per-city live controls, two saves, restart and fallback OK");
}
