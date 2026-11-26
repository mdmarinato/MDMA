#include <math.h>
#define main test_original_panel
#include "test-panel.c"
#undef main

static void finish_write(void)
{
    for(unsigned i=0;i<160 && mdma_store_pending(&groove_store);++i)save_probe_poll(false);
    assert(!mdma_store_pending(&groove_store));
}

static void confirm_action(unsigned row,unsigned target)
{
    page=PAGE_SAVE;save_row=SAVE_MAIN;save_confirm=false;
    enc[ENC_SELECT]=row;panel_scan_knobs();
    if(row==SAVE_RESET_CITY)reset_target=target;
    press(BTN_OCT_UP);
    assert(save_confirm);
    if(panel_buttons(1u<<BTN_OCT_UP,0))save_probe_poll(true);
    finish_write();
}

int main(void)
{
    memset(flash,255,sizeof flash);
    panel_defaults();save_probe_boot();
    page=PAGE_HOME;
    for(unsigned i=0;i<3;++i){
        unsigned view=page;
        press(BTN_FX);assert(page==view && knob_bank==BANK_FX);
        enc[ENC_KNOB1]=1;panel_scan_knobs();
        press(BTN_FX);assert(page==view && knob_bank==BANK_EQ);
        enc[ENC_KNOB2]=1;panel_scan_knobs();
        press(BTN_HOME);assert(page!=view && knob_bank==BANK_EQ);
        press(BTN_FX);assert(knob_bank==BANK_MIX);
    }
    const unsigned views[]={PAGE_HOME,PAGE_STAGE,PAGE_MIX};
    for(unsigned i=0;i<3;++i){
        page=views[i];groove_enabled=GROOVE_DRUMS;fill_active=fill_queued=false;
        press(BTN_SEQ);fm1_ms+=349;panel_buttons(0,0);assert(!fill_queued);
        ++fm1_ms;panel_buttons(0,0);assert(fill_queued);
        fm1_ms+=500;release(BTN_SEQ);assert(fill_queued && page==views[i]);
        press(BTN_SEQ);fm1_ms+=350;release(BTN_SEQ);assert(!fill_queued && page==views[i]);
        press(BTN_SEQ);fm1_ms+=100;release(BTN_SEQ);assert(page==PAGE_SEQ);
    }
    page=PAGE_HOME;groove_enabled=0;
    uint8_t raw[GROOVE_RECORD];
    panel_create_city();
    unsigned city=active_city,recipe=city_recipe[city];
    for(unsigned l=0;l<6;++l)
        for(unsigned k=0;k<4;++k)panel_eq[city][l][k]=(4*l+k)%16;
    panel_eq[0][0][0]=2;
    groove_encode(raw);
    memset(panel_eq,0,sizeof panel_eq);
    assert(groove_decode(raw,sizeof raw,true));
    for(unsigned l=0;l<6;++l)
        for(unsigned k=0;k<4;++k)assert(panel_eq[city][l][k]==(4*l+k)%16);
    assert(panel_eq[0][0][0]==2 && panel_eq[1][0][0]==EQ_FLAT);

    for(unsigned k=1;k<=4;++k){
        save_kind=k;save_action=0;
        save_probe_poll(true);finish_write();assert(point_present[k]);
    }
    save_probe_boot();assert(point_restore(2));
    confirm_action(SAVE_RESET_CITY,city);
    assert(city_count==city+1 && city_recipe[city]==recipe && panel_eq[city][0][0]==EQ_FLAT);
    assert(panel_eq[0][0][0]==2 && point_present[1]);
    assert(point_restore(2) && panel_eq[city][0][0]==0);
    confirm_action(SAVE_RESET_CITY,city_count);
    save_probe_boot();
    assert(city_count==city+1 && city_recipe[city]==recipe && panel_eq[0][0][0]==EQ_FLAT);
    confirm_action(SAVE_DELETE_POINT,0);
    save_probe_boot();
    assert(point_present[1] && !point_present[2] && point_present[3] && point_present[4]);

    /* Four separated frequency regions at the real layer-mixer rate. */
    const double hz[]={35,360,1200,4500};
    for(unsigned band=0;band<4;++band){
        uint32_t controls=0x09090909u+(6u<<(8*band));
        double input_energy=0,output_energy=0;
        memset(eq_state,0,sizeof eq_state);
        for(unsigned t=0;t<11025;++t){
            int sample=(int)(8000*sin(2*3.141592653589793*hz[band]*t/11025));
            int y=layer_eq(0,0,sample,controls);
            if(t>500){input_energy+=(double)sample*sample;output_energy+=(double)y*y;}
        }
        assert(output_energy>1.2*input_energy);
    }
    puts("EQ PCM, bancos, persistência, exclusão e reset por cidade/total: OK");
}
