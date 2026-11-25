#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define MDMA_PANEL_LCD
#define FM1_TICKS_PER_US 24
static uint32_t simulated_ticks_per_read=100;
static uint32_t fm1_ticks(void){static uint32_t t;return t+=simulated_ticks_per_read;}
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
enum { BTN_HOME, BTN_PLAY, BTN_FX, BTN_SEQ, BTN_EDIT, BTN_LFO, BTN_ENV, BTN_ARP, BTN_SAVE, BTN_REC, BTN_OCT_UP, BTN_OCT_DOWN, BTN_SEL, BTN_GLO };
enum { ENC_SELECT, ENC_ALGORITHM, ENC_PRESETS, ENC_KNOB1, ENC_KNOB2, ENC_KNOB3, ENC_KNOB4 };
static int32_t enc[8];
static int32_t fm1_enc_take(unsigned e){int32_t v=enc[e];enc[e]=0;return v;}
static uint32_t fm1_ms;
static volatile uint32_t mdma_audio_max_ticks;
static uint16_t cv_px[240*240];
#define RGB(r,g,b) 0
#define C_WHITE 1
#define C_BLACK 0
static void cv_rect(int x,int y,int w,int h,uint16_t c){(void)x;(void)y;(void)w;(void)h;(void)c;}
static void cv_pset(int x,int y,uint16_t c){(void)x;(void)y;(void)c;}
static void cv_line(int a,int b,int c,int d,uint16_t e){(void)a;(void)b;(void)c;(void)d;(void)e;}
static void cv_begin(int w,int h,uint16_t c){(void)w;(void)h;(void)c;}
static void cv_blit(int x,int y){(void)x;(void)y;}
static void lcd_blit(int x,int y,int w,int h,uint16_t*p){(void)x;(void)y;(void)w;(void)h;(void)p;}
static volatile int32_t master_q12=2048;static unsigned qa_clock_count;

#include "mdma_groove.c"
static bool quiet(void){return !groove_enabled && !groove_sounding;}
#define draw_text_box(x,y,w,f,t,c,a) snprintf(shown,sizeof shown,"%s",t)
#include "mdma_save_probe.c"
#include "mdma_panel.c"

static void press(unsigned button){panel_buttons(1u<<button,0);}
static void release(unsigned button){panel_buttons(0,1u<<button);}

int main(void){
    panel_defaults();groove_publish();
    page=PAGE_HOME;selected_layer=1;qa_ready_mask=63;

    /* Switch and revisit scenes via the keyboard and the same encoder as HOME. */
    assert(city_ghost_grid[0]);
    panel_key(3);assert(layer_variation[0][1]==1);
    enc[ENC_ALGORITHM]=3;panel_scan_knobs();
    selected_layer=3;enc[ENC_ALGORITHM]=2;panel_scan_knobs();
    panel_key(5);assert(layer_variation[0][1]==3 && layer_variation[0][3]==2);
    panel_key(8);assert(layer_variation[0][1]==2);
    uint8_t saved[GROOVE_RECORD];groove_encode(saved);
    memset(scene_variation,0,sizeof scene_variation);
    assert(groove_decode(saved,sizeof saved,true));
    panel_key(3);assert(layer_variation[0][1]==4 && layer_variation[0][3]==2);
    page=PAGE_EDIT;press(BTN_REC);release(BTN_REC);assert(!city_ghost_grid[0]);
    panel_key(1);assert(!city_ghost_grid[0]);
    page=PAGE_HOME;selected_layer=1;
    for(unsigned scene=0;scene<4;++scene){
        int previous=-1;bool up=false,down=false;
        for(unsigned step=0;step<64;++step){
            int note=scene_lead_note(0,scene,scene_defaults[scene][1],step,true);
            if(previous>=0){up|=note>previous;down|=note<previous;}
            previous=note;
        }
        assert(up && down);
    }

    /* ARP has no function. */
    press(BTN_ARP);assert(qa_ready_mask==63 && selected_layer==1);

    /* EDIT: OCT+/- walk the layers in HOME list order and never touch the BPM. */
    page=PAGE_EDIT;selected_layer=1;
    const unsigned bpm=groove_edits[active_city].bpm;
    press(BTN_OCT_UP);assert(selected_layer==4);
    press(BTN_OCT_UP);assert(selected_layer==2);
    press(BTN_OCT_DOWN);assert(selected_layer==4);
    press(BTN_OCT_DOWN);assert(selected_layer==1);
    press(BTN_OCT_DOWN);assert(selected_layer==0);
    press(BTN_OCT_DOWN);assert(selected_layer==5);
    release(BTN_OCT_DOWN);fm1_ms+=4000;press(BTN_OCT_UP);release(BTN_OCT_UP);
    assert(groove_edits[active_city].bpm==bpm && tempo_direction==0 && page==PAGE_EDIT);
    /* Outside EDIT the OCT buttons still change the tempo. */
    page=PAGE_HOME;fm1_ms+=1000;press(BTN_OCT_UP);fm1_ms+=100;release(BTN_OCT_UP);
    assert(groove_edits[active_city].bpm==bpm+1);
    groove_edits[active_city].bpm=(uint8_t)bpm;
    /* Solo and mute are momentary on HOME, STAGE and MIX only. */
    selected_layer=1;
    for(unsigned p=0;p<3;++p){
        page=p==0?PAGE_HOME:p==1?PAGE_STAGE:PAGE_MIX;
        press(BTN_LFO);assert(listen_mask==2u);release(BTN_LFO);assert(listen_mask==63u);
        press(BTN_ENV);assert(listen_mask==(63u^2u));release(BTN_ENV);assert(listen_mask==63u);
    }
    page=PAGE_EDIT;press(BTN_LFO);assert(listen_mask==63u);release(BTN_LFO);
    page=PAGE_HOME;press(BTN_LFO);assert(listen_mask==2u);page=PAGE_FX;panel_buttons(0,0);assert(listen_mask==63u);
    release(BTN_LFO);

    /* Knob feel: 1 step per click when slow, gain only after a sustained fast spin, no coasting. */
    page=PAGE_HOME;selected_layer=0;panel_mix[active_city][0][MIX_PAN]=20;panel_publish();
    fm1_ms=10000;panel_scan_knobs();
    for(unsigned i=0;i<10;++i){fm1_ms+=60;enc[ENC_KNOB2]=1;panel_scan_knobs();}
    assert(panel_mix[active_city][0][MIX_PAN]==30);
    fm1_ms+=2000;panel_scan_knobs();assert(panel_mix[active_city][0][MIX_PAN]==30);
    fm1_ms+=60;enc[ENC_KNOB2]=-1;panel_scan_knobs();assert(panel_mix[active_city][0][MIX_PAN]==29);
    fm1_ms+=60;enc[ENC_KNOB2]=1;panel_scan_knobs();assert(panel_mix[active_city][0][MIX_PAN]==30);
    /* a 30 clicks/s turn is a normal turn: still 1 step per click */
    fm1_ms+=1000;panel_mix[active_city][0][MIX_PAN]=0;
    for(unsigned i=0;i<20;++i){fm1_ms+=33;enc[ENC_KNOB2]=1;panel_scan_knobs();}
    assert(panel_mix[active_city][0][MIX_PAN]==20);
    /* a 200 clicks/s spin ramps up smoothly to no more than 3 steps per click */
    fm1_ms+=1000;panel_mix[active_city][0][MIX_PAN]=0;
    unsigned previous=0,largest=0,first=0;
    for(unsigned i=0;i<30;++i){
        fm1_ms+=5;enc[ENC_KNOB2]=1;panel_scan_knobs();
        const unsigned now=panel_mix[active_city][0][MIX_PAN],step=now-previous;previous=now;
        if(!i)first=step;
        if(step>largest)largest=step;
        assert(step<=3);
    }
    assert(first==1 && largest==3);
    fm1_ms+=3000;panel_scan_knobs();assert(panel_mix[active_city][0][MIX_PAN]==previous);

    /* SAVE: ALGORITHM switches ALL CITIES / CURRENT CITY on the main row, the action on the points. */
    page=PAGE_SAVE;save_row=save_kind=save_action=0;save_current_city=false;
    enc[ENC_ALGORITHM]=1;panel_scan_knobs();assert(save_current_city && !save_action);
    enc[ENC_ALGORITHM]=1;panel_scan_knobs();assert(!save_current_city);
    save_row=save_kind=1;enc[ENC_ALGORITHM]=1;panel_scan_knobs();assert(!save_current_city && save_action==1);
    save_row=save_kind=save_action=0;page=PAGE_HOME;

    /* SEQ: editing a held step that is off switches it on. */
    page=PAGE_SEQ;seq_layer=0;
    {
        const unsigned slot=seq_current_slot(),lane=1;
        const GroovePattern *g=&groove_patterns[city_pattern_bank(layer_origin[active_city][0])][slot&3][slot>>2];
        unsigned step=0;
        while(step<16 && ((g->hit[lane]^drum_edit_mask(active_city,lane,slot))&(1u<<step)))++step;
        assert(step<16);
        held_seq_step=step;held_seq_lane=lane;held_seq_slot=slot;held_seq_city=active_city;
        held_seq_key=1;held_seq_at=fm1_ms-300u;
        enc[ENC_KNOB1]=2;panel_scan_knobs();
        assert((g->hit[lane]^drum_edit_mask(active_city,lane,slot))&(1u<<step));
        assert(shape_index(step_key(active_city,slot,lane,step))>=0);
        /* a second edit does not toggle it back off */
        enc[ENC_KNOB1]=1;panel_scan_knobs();
        assert((g->hit[lane]^drum_edit_mask(active_city,lane,slot))&(1u<<step));
        panel_seq();
        held_seq_step=16;
    }

    /* Calibration page shows the audio load readout. */
    page=PAGE_MIX;mix_calibration=true;mdma_audio_max_ticks=240u*900u;panel_dirty=true;panel_render();
    press(BTN_REC);assert(!mix_calibration);mdma_audio_max_ticks=5;press(BTN_REC);assert(mix_calibration && mdma_audio_max_ticks==0);mix_calibration=false;

    /* 256 stereo frames give about 5805 us at 44.1 kHz. */
    int32_t block[512]={0};
    simulated_ticks_per_read=4500u*FM1_TICKS_PER_US;
    groove_audio(block,256,false);
    assert(audio_lite);
    simulated_ticks_per_read=100u*FM1_TICKS_PER_US;
    for(unsigned i=0;i<1378u;++i)groove_audio(block,256,false);
    assert(audio_lite);
    groove_audio(block,256,false);
    assert(!audio_lite);

    /* Exercise both synth targets on the audio path at real sequencer steps. */
    active_city=0;panel_reset_city(0);qa_source=1;qa_running=1;
    groove_enabled=GROOVE_BASS|GROOVE_LEAD;
    for(unsigned scene=0;scene<4;++scene){
        select_scene(0,scene);groove_publish();qa_ticks=0;++qa_start_serial;
        uint32_t first_bass=0;int last_lead=0;
        bool bass_changed=false,up=false,down=false;
        for(unsigned tick=1;tick<=128;++tick){
            qa_ticks=tick;memset(block,0,sizeof block);groove_audio(block,256,false);
            if(tick==1)first_bass=acid.goal_inc;
            bass_changed|=acid.goal_inc!=first_bass;
            if(last_lead){up|=lead_ghost.target_pitch>last_lead;down|=lead_ghost.target_pitch<last_lead;}
            last_lead=lead_ghost.target_pitch;
        }
        assert(bass_changed && up && down);
    }
    puts("BASS e LEAD HOLD: movimento de pitch confirmado nas quatro cenas");
    /* Every page renders with the stubbed canvas. */
    for(page=PAGE_HOME;page<=PAGE_MIX;++page){panel_notice=0;panel_dirty=true;panel_render();}
    puts("panel: ARP, EDIT layer walk, solo/mute hold, knob feel, SAVE scope, SEQ step edit OK");
    return 0;
}
