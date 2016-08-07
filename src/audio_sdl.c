/* MegaZeux
 *
 * Copyright (C) 2004 Gilead Kutnick <exophase@adelphia.net>
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

#include "audio.h"
#include "SDL.h"
#include "SDL_mixer.h"

#include <assert.h>
#include <stdlib.h>

static SDL_AudioSpec audio_settings;

static void sdl_audio_callback(void *userdata, Uint8 *stream, int len)
{
  audio_callback((Sint16 *)stream, len);
}

void init_audio_platform(struct config_info *conf)
{
  int ret = Mix_OpenAudio(audio.output_frequency, AUDIO_S16SYS, 2, conf->buffer_size);
  assert(ret == 0);
  /*int channels;
  ret = Mix_QuerySpec(&audio_settings.freq, &audio_settings.format, &channels);
  audio_settings.channels = channels;
  assert(ret != 0);*/
  audio_settings.freq = 22050;
  audio_settings.format = AUDIO_S16SYS;
  audio_settings.channels = 2;
  audio_settings.samples = 1024/*conf->buffer_size*/;
  audio_settings.callback = sdl_audio_callback;
  audio_settings.userdata = NULL;
  audio_settings.silence = 0x00;
  audio_settings.size = (audio_settings.format & 0xFF)/8;
  audio_settings.size *= audio_settings.channels;
  audio_settings.size *= audio_settings.samples;

  Mix_HookMusic(sdl_audio_callback, NULL);
  audio.mix_buffer = cmalloc(audio_settings.size * 2);
  audio.buffer_samples = audio_settings.samples;
}

void quit_audio_platform(void)
{
  SDL_PauseAudio(1);
  free(audio.mix_buffer);
}
