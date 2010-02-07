/* MegaZeux
 *
 * Copyright (C) 2004 Gilead Kutnick <exophase@adelphia.net>
 * Copyright (C) 2007 Alistair John Strachan <alistair@devzero.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// Definitions for audio.cpp

#ifndef __AUDIO_H
#define __AUDIO_H

#include "compat.h"

__M_BEGIN_DECLS

#ifdef CONFIG_AUDIO

#include "SDL.h"
#include "configure.h"

#ifdef CONFIG_MODPLUG
#include "modplug.h"
#endif

typedef struct _audio_stream audio_stream;

struct _audio_stream
{
  struct _audio_stream *next;
  struct _audio_stream *previous;
  Uint32 volume;
  Uint32 repeat;
  Uint32 (* mix_data)(audio_stream *a_src, Sint32 *buffer,
   Uint32 len);
  void (* set_volume)(audio_stream *a_src, Uint32 volume);
  void (* set_repeat)(audio_stream *a_src, Uint32 repeat);
  void (* set_order)(audio_stream *a_src, Uint32 order);
  void (* set_position)(audio_stream *a_src, Uint32 pos);
  Uint32 (* get_order)(audio_stream *a_src);
  Uint32 (* get_position)(audio_stream *a_src);
  void (* destruct)(audio_stream *a_src);
};

typedef struct _sampled_stream sampled_stream;

struct _sampled_stream
{
  audio_stream a;
  Uint32 frequency;
  Sint16 *output_data;
  Uint32 data_window_length;
  Uint32 allocated_data_length;
  Uint32 prologue_length;
  Uint32 epilogue_length;
  Uint32 stream_offset;
  Uint32 channels;
  Uint32 negative_comp;
  Uint32 use_volume;
  Sint64 frequency_delta;
  Sint64 sample_index;
  void (* set_frequency)(sampled_stream *s_src, Uint32 frequency);
  Uint32 (* get_frequency)(sampled_stream *s_src);
};

typedef struct
{
  audio_stream a;
  Uint32 volume;
  Uint32 playing;
  Uint32 frequency;
  Sint32 last_frequency;
  Uint32 note_duration;
  Uint32 last_duration;
  Uint32 last_playing;
  Uint32 sample_cutoff;
  Uint32 last_increment_buffer;
} pc_speaker_stream;

typedef struct
{
  SDL_AudioSpec audio_settings;

#ifdef CONFIG_MODPLUG
  // for config.txt settings only
  ModPlug_Settings mod_settings;
#endif

  Sint32 *mix_buffer;

  Uint32 output_frequency;
  Uint32 master_resample_mode;

  audio_stream *primary_stream;
  pc_speaker_stream *pcs_stream;
  audio_stream *stream_list_base;
  audio_stream *stream_list_end;

#ifndef PTHREAD_MUTEXES
  SDL_mutex *audio_mutex;
#else
  pthread_mutex_t audio_mutex;
#endif

  Uint32 music_on;
  Uint32 sfx_on;
  Uint32 music_volume;
  Uint32 sound_volume;
  Uint32 sfx_volume;
} audio_struct;

void init_audio(config_info *conf);
void load_module(char *filename);
void end_module();
void play_sample(int freq, char *filename);
void end_sample();
void jump_module(int order);
int get_order();
void volume_module(int vol);
void module_exit();
void module_init();
void spot_sample(int freq, int sample);
void shift_frequency(int freq);
int get_frequency();
void set_position(int pos);
int get_position();
int free_sam_cache(char clear_all);
void fix_global_volumes(void);
void sound(int frequency, int duration);
void nosound(int duration);
void set_music_on(int val);
void set_sfx_on(int val);
int get_music_on_state();
int get_sfx_on_state();
int get_music_volume();
int get_sound_volume();
int get_sfx_volume();
void set_music_volume(int volume);
void set_sound_volume(int volume);
void set_sfx_volume(int volume);

#ifdef CONFIG_MODPLUG
void convert_sam_to_wav(char *source_name, char *dest_name);
#endif

/*** these should only be exported for audio plugins */

#if defined(CONFIG_MODPLUG) || defined(CONFIG_MIKMOD)

extern audio_struct audio;

int file_length(FILE *fp);
void sampled_set_buffer(sampled_stream *s_src);
void sampled_mix_data(sampled_stream *s_src, Sint32 *dest_buffer,
 Uint32 len);
void sampled_destruct(audio_stream *a_src);
void initialize_sampled_stream(sampled_stream *s_src,
 void (* set_frequency)(sampled_stream *s_src, Uint32 frequency),
 Uint32 (* get_frequency)(sampled_stream *s_src),
 Uint32 frequency, Uint32 channels, Uint32 use_volume);
void construct_audio_stream(audio_stream *a_src,
 Uint32 (* mix_data)(audio_stream *a_src, Sint32 *buffer,
  Uint32 len),
 void (* set_volume)(audio_stream *a_src, Uint32 volume),
 void (* set_repeat)(audio_stream *a_src, Uint32 repeat),
 void (* set_order)(audio_stream *a_src, Uint32 order),
 void (* set_position)(audio_stream *a_src, Uint32 pos),
 Uint32 (* get_order)(audio_stream *a_src),
 Uint32 (* get_position)(audio_stream *a_src),
 void (* destruct)(audio_stream *a_src),
 Uint32 volume, Uint32 repeat);

#define __audio_c_maybe_static

#else // !CONFIG_MODPLUG && !CONFIG_MIKMOD

#define __audio_c_maybe_static static

#endif // CONFIG_MODPLUG || CONFIG_MIKMOD

/*** end audio plugins exports */

#else // !CONFIG_AUDIO

static inline void init_audio(config_info *conf) {}
static inline void set_music_volume(int volume) {}
static inline void set_sound_volume(int volume) {}
static inline void set_music_on(int val) {}
static inline void set_sfx_on(int val) {}
static inline void set_sfx_volume(int volume) {}
static inline void end_sample() {}
static inline void end_module() {}
static inline void load_module(char *filename) {}
static inline void volume_module(int vol) {}
static inline void set_position(int pos) {}
static inline void jump_module(int order) {}
static inline void shift_frequency(int freq) {}
static inline void play_sample(int freq, char *filename) {}
static inline void spot_sample(int freq, int sample) {}
static inline int get_music_on_state() { return 0; }
static inline int get_sfx_on_state() { return 0; }
static inline int get_music_volume() { return 0; }
static inline int get_sound_volume() { return 0; }
static inline int get_sfx_volume() { return 0; }
static inline int get_position() { return 0; }
static inline int get_order() { return 0; }
static inline int get_frequency() { return 0; }

#endif // CONFIG_AUDIO

__M_END_DECLS

#endif // __AUDIO_H
