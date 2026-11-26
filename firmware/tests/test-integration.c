#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define FL_IN(a,n,lo,hi) ((a)>=(lo) && (a)<=(hi) && (n)<=(hi)-(a))
#define FL_GUARD(a,n) (FL_IN(a,n,0xB1000u,0xB3000u)||FL_IN(a,n,0xF4000u,0xFC000u))
#define FONT_S 0
#define C_OFF 0
#define LEAD_CUTOFF_LAST 32
#define LEAD_CUTOFF_DEFAULT 14
static bool flash_ok=true;
static struct {uint32_t notes;} fm1_in;
static uint8_t flash[40960];
static unsigned erases;
static char shown[32];
static int32_t lead_cutoff_position=14;
static void lead_cutoff_turn(int32_t delta){lead_cutoff_position+=delta;}
static int st_read(uint32_t a,void*p,uint32_t n){assert(FL_GUARD(a,n));memcpy(p,flash+(a>=0xF4000u?a-0xF4000u+8192u:a-0xB1000u),n);return 0;}
static int fl_write(uint32_t a,const void*p,uint32_t n){assert(FL_GUARD(a,n));for(uint32_t i=0;i<n;i++)flash[(a>=0xF4000u?a-0xF4000u+8192u:a-0xB1000u)+i]&=((const uint8_t*)p)[i];return 0;}
static int fl_erase4k(uint32_t a,uint32_t*t){assert(FL_GUARD(a,4096u));*t=0;memset(flash+(a>=0xF4000u?a-0xF4000u+8192u:a-0xB1000u),255,4096);++erases;return 0;}
enum { BTN_HOME, BTN_PLAY, BTN_FX, BTN_SEQ, BTN_EDIT, BTN_LFO, BTN_ENV, BTN_ARP };
#include "mdma_groove.c"
static bool quiet(void){return !groove_enabled && !groove_sounding;}
#define draw_text_box(x,y,w,f,t,c,a) snprintf(shown,sizeof shown,"%s",t)
#define TEST_AUTOSAVE
static uint32_t fm1_ms;
#include "mdma_save_probe.c"
static uint8_t inbound[160],answer[160];
static uint32_t in_len,out_len;
static int ota_frame_get(const uint8_t **ptr,uint32_t *size){*ptr=inbound;*size=in_len;return in_len>0;}
static void ota_frame_done(void){in_len=0;}
static void ota_wire_send(const uint8_t *msg,uint32_t size){assert(size<=sizeof answer);memcpy(answer,msg,size);out_len=size;}
static void panel_show(void){}
#include "mdma_backup.c"
static void backup_command(unsigned op,unsigned at,const uint8_t *data,unsigned len){
    const uint8_t prefix[]={125,77,68,77,65};
    memcpy(inbound,prefix,sizeof prefix);inbound[5]=op;in_len=6;out_len=0;
    if(op==2||op==4){inbound[in_len++]=at&127;inbound[in_len++]=at>>7;}
    if(len){memcpy(inbound+in_len,data,len);in_len+=len;}
    assert(mdma_backup_service()&&out_len>=11&&answer[6]==op);
}
static void boot(void){groove_save_ready=false;active_city=0;lead_cutoff_position=14;save_probe_boot();}
static void service(void){for(unsigned i=0;i<32;i++)save_probe_poll(false);}
int main(void){
    panel_defaults();
    uint8_t legacy[GROOVE_LEGACY]={ 'M','G','R','1',0,1,0,0,132,0,0,80,80,50,14,132,0,0,80,80,50,14 };
    assert(groove_decode(legacy,sizeof legacy,true)&&panel_fx[0][0][FX_DLY]==127);
    uint8_t v2[GROOVE_V2]={ 'M','G','R','2',1,0,0,0,132,0,0,80,80,50,14,132,0,4,80,80,50,14 };
    v2[GROOVE_LEGACY + 92 + 24 + FX_DLY] = 80;
    assert(groove_decode(v2,sizeof v2,true));
    assert(active_city==1 && layer_variation[1][5]==4 && layer_origin[1][5]==1);
    assert(panel_fx[1][0][FX_DLY]==80 && panel_fx[1][5][FX_DLY]==80);
    uint8_t old[GROOVE_V3] = {'M','G','R','3',0,0,0,0};
    for (unsigned city=0;city<FACTORY_COUNT;++city) {
        uint8_t *record = old + 8 + city * GROOVE_V3_CITY;
        record[0]=132; record[3]=80; record[4]=80; record[5]=50; record[6]=19;
        for (unsigned layer=0;layer<LAYER_COUNT;++layer) {
            record[105+layer]=(uint8_t)city; record[99+layer]=city==1 ? 4 : 0;
        }
    }
    assert(groove_decode(old,sizeof old,true));
    assert(lead_edit_detent(city_lead_home[0],LEAD_EDIT_CUTOFF)==5);
    assert(city_ghost_time_q12[0]==0);
    active_city=0;
    panel_defaults();
    int32_t l_mix=0,r_mix=0;
    panel_mix[0][0][MIX_PAN]=0;panel_publish();
    qa_layer(0,1000,1000,&l_mix,&r_mix);assert(l_mix==803&&r_mix==0);
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
    memset(delay_line,0,sizeof delay_line);
    return_lp[0].z1=return_lp[0].z2=0;
    delay_index=0;mix_delay_send=4000;mix_room_send=0;
    int32_t dry[2]={0},result[2]={0};
    master_mix(0,15033,dry,dry,dry,result);
    assert(delay_line[0]==4000);
    mix_delay_send=0;
    for(unsigned i=1;i<=15033;i++)master_mix(0,15033,dry,dry,dry,result);
    assert(delay_line[15033]>0);
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
    const uint32_t edits = (1u << (24u+LEAD_EDIT_RATE)) | (7u << (6u*LEAD_EDIT_RATE));
    city_lead_home[1]=edits;
    city_lead_borrowed[1]=1u << (24u+LEAD_EDIT_GLIDE);
    city_ghost_offset[1]=-4096;
    city_ghost_time_q12[1]=16384;
    city_voice_edit[1][2]=(5u<<24)|12u;
    lead_cutoff_position=21;save_probe_poll(true);service();assert(erases==2);
    memset(groove_edits,0,sizeof groove_edits);
    memset(layer_variation,0,sizeof layer_variation);
    memset(layer_origin,0,sizeof layer_origin);
    memset(panel_fx,0,sizeof panel_fx);
    memset(city_tonic,0,sizeof city_tonic);
    memset(city_major,0,sizeof city_major);
    memset(city_swing,0,sizeof city_swing);
    panel_drum[1][4][7]=0;
    city_lead_home[1]=city_lead_borrowed[1]=city_ghost_time_q12[1]=0;
    city_ghost_offset[1]=0;
    city_voice_edit[1][2]=0;boot();
    assert(city_voice_edit[1][2]==((5u<<24)|12u));
    assert(!strcmp(shown,"RIO    S1 V5 BPM 118"));
    assert(active_city==1 && groove_edits[1].tone==77 && lead_cutoff_position==21);
    assert(panel_fx[1][0][FX_DLY]==100 && panel_fx[1][0][FX_REV]==88);
    assert(panel_mix[1][3][MIX_FLT]==98 && panel_kick[1][5]==0x4001);
    assert(city_lead_home[1]==edits && city_lead_borrowed[1]==(1u << (24u+LEAD_EDIT_GLIDE)));
    assert(city_ghost_time_q12[1]==16384 && city_ghost_offset[1]==-4096);
    assert(layer_variation[1][3]==7 && layer_origin[1][3]==2);
    assert(panel_drum[1][4][7]==0x8004 && city_tonic[1]==11 && city_major[1]==1 && city_swing[1]==22);
    assert(panel_fx[1][3][FX_SAT]==67 && panel_fx[1][0][FX_SAT]==0);
    assert(groove_edits[0].scene==2 && layer_variation[0][0]==4 && groove_edits[0].bass==43);
    assert(groove_edits[0].bpm==140);
    for(unsigned k=1;k<=4;++k){
        save_kind=k;save_action=0;
        city_voice_edit[1][2]=k;save_probe_poll(true);service();
        assert(point_present[k]);
    }
    for(unsigned k=1;k<=4;++k){
        assert(point_restore(k) && city_voice_edit[1][2]==k);
        save_kind=k;save_action=2;save_probe_poll(true);service();
        assert(!point_present[k] && !point_restore(k));
    }
    save_kind=save_action=0;
    uint8_t v5[GROOVE_V5]={0},current[GROOVE_RECORD];
    groove_encode(current);memcpy(v5,current,8);v5[3]='5';
    for(unsigned k=0;k<city_count;++k)memcpy(v5+8+k*GROOVE_V5_CITY,current+8+k*GROOVE_CITY_BYTES,GROOVE_V5_CITY);
    for(unsigned k=0;k<city_count;++k){
        uint8_t *row=v5+8+k*GROOVE_V5_CITY;
        row[34]=0;
        for(unsigned l=0;l<LAYER_COUNT;++l)row[99+l]&=7;
    }
    assert(groove_decode(v5,sizeof v5,true) && city_voice_edit[1][2]==0);
    unsigned before=erases;
    flash[4096+20]^=1;boot();assert(active_city==0 && groove_edits[0].bpm==140);
    save_probe_poll(true);assert(!strcmp(shown,"SAVE BLOCKED")&&erases==before);
    memset(flash,255,sizeof flash);boot();
    groove_sounding=groove_enabled=qa_running=0;fm1_ms=100;groove_autosave_boot();
    city_swing[active_city]=12;
    fm1_ms=33099;groove_autosave();assert(!mdma_store_pending(&groove_store));
    ++audio_activity_serial;groove_autosave();
    fm1_ms+=32999;groove_autosave();assert(!mdma_store_pending(&groove_store));
    ++fm1_ms;groove_autosave();assert(mdma_store_pending(&groove_store));service();
    fm1_ms+=33000;groove_autosave();assert(!mdma_store_pending(&groove_store));
    puts("Autosave timing passed at 32.999s and 33.000s, including tail, timer restart and duplicate-write checks");
    backup_command(1,0,0,0);assert(answer[7]==0 && (answer[10]+128u*answer[11])==GROOVE_RECORD);
    uint8_t copy[GROOVE_RECORD],crc[5];memcpy(crc,answer+12,5);
    for(unsigned at=0;at<GROOVE_RECORD;at+=64){
        backup_command(2,at,0,0);assert(!answer[7]);
        for(unsigned j=10;j+1<out_len-1;j+=2)copy[at+(j-10)/2]=answer[j]+128*answer[j+1];
    }
    ++city_swing[active_city];
    backup_command(3,0,0,0);
    for(unsigned at=0;at<GROOVE_RECORD;at+=64){
        uint8_t chunk[128];unsigned len=GROOVE_RECORD-at;if(len>64)len=64;
        for(unsigned j=0;j<len;++j){chunk[2*j]=copy[at+j]&127;chunk[2*j+1]=copy[at+j]>>7;}
        backup_command(4,at,chunk,2*len);assert(!answer[7]);
    }
    crc[0]^=1;backup_command(5,0,crc,5);assert(answer[7]==1 && city_swing[active_city]==13);
    crc[0]^=1;backup_command(5,0,crc,5);assert(!answer[7] && city_swing[active_city]==12);
    service();backup_command(6,0,0,0);assert(!answer[7]);
    puts("USB backup export, block transfer, CRC rejection and persistent import passed");
    /* SAVE CURRENT CITY: only the active city is replaced in the stored main record. */
    {
        const unsigned c=active_city, o=c?0u:1u;
        uint8_t stored[GROOVE_RECORD];uint32_t n;
        assert(mdma_store_load(&groove_store,0,stored,sizeof stored,&n)&&n==GROOVE_RECORD);
        uint8_t other_row[GROOVE_CITY_BYTES];
        memcpy(other_row,stored+8+o*GROOVE_CITY_BYTES,sizeof other_row);
        const uint8_t old_other=stored[8+o*GROOVE_CITY_BYTES];
        groove_edits[o].bpm=old_other==170?171:170;groove_edits[c].bpm=111;
        groove_sounding=groove_enabled=qa_running=0;
        save_kind=0;save_current_city=true;
        save_probe_poll(true);service();assert(!strcmp(shown,"GROOVE SAVED"));
        assert(mdma_store_load(&groove_store,0,stored,sizeof stored,&n)&&n==GROOVE_RECORD);
        assert(stored[8+c*GROOVE_CITY_BYTES]==111 && stored[8+o*GROOVE_CITY_BYTES]==old_other);
        assert(!memcmp(other_row,stored+8+o*GROOVE_CITY_BYTES,sizeof other_row));
        save_current_city=false;
        puts("SAVE CURRENT CITY replaced only the selected city in the main save");
    }
    puts("groove: silence gate, per-city live controls, two saves, restart and fallback OK");
}
