#ifndef __SOUNDTRIGGER_SOCDSP_PCM_H
#define __SOUNDTRIGGER_SOCDSP_PCM_H

enum socdsp_wakeup_mode {
	NORMAL_WAKEUP_MODE = 0,
	LP_WAKEUP_MODE,
	WAKEUP_MODE_BUTT,
};

void soundtrigger_socdsp_pcm_flag_init(unsigned int wakeup_mode);
int soundtrigger_socdsp_pcm_fastbuffer_filled(unsigned int wakeup_mode, unsigned int fast_len);
int soundtrigger_socdsp_pcm_init(void);
int soundtrigger_socdsp_pcm_deinit(void);
int soundtrigger_socdsp_pcm_elapsed(unsigned int wakeup_mode, unsigned int start, int buffer_len);
#endif
