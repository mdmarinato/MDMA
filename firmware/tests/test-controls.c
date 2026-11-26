#define main previous_panel_suite
#include "test-panel.c"
#undef main
int main(void){
 panel_defaults();page=PAGE_HOME;panel_set_layers(GROOVE_DRUMS);
 press(BTN_SEQ);fm1_ms+=351;release(BTN_SEQ);assert(fill_queued);
 panel_key(25);assert(!qa_ready_mask);panel_key(25);assert(qa_ready_mask==GROOVE_DRUMS);
 press(BTN_PLAY);assert(qa_ready_mask==GROOVE_BASS);
 page=PAGE_EDIT;selected_layer=3;panel_key(10);
 assert(selected_layer==3 && qa_ready_mask==(GROOVE_BASS|GROOVE_DRUMS));
 press(BTN_OCT_DOWN);assert(selected_layer!=3);
 assert(SAVE_ROWS==3);
 page=PAGE_HOME;knob_bank=BANK_FX;fm1_ms+=1000;
 assert(panel_knob_motion(0,1,false)==3);
 for(unsigned i=0;i<20;i++){fm1_ms+=8;panel_knob_motion(0,1,false);}
 assert(panel_knob_motion(0,1,false)==10);
 return 0;
}
