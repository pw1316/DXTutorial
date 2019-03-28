/* MIT License

Copyright (c) 2018 Joker Yough

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

#ifndef __MANAGER_SOUND_MANAGER__
#define __MANAGER_SOUND_MANAGER__
#include <dsound.h>

#include <core/iview.h>

namespace naiive::manager {
class SoundManagerClass : public core::IView {
  friend SoundManagerClass& SoundManager();

 private:
  struct WaveHeaderType {
    char chunk_id[4];
    unsigned long chunk_size;
    char format[4];
    char sub_chunk_id[4];
    unsigned long sub_chunk_size;
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned long sample_rate;
    unsigned long bytes_per_second;
    unsigned short block_align;
    unsigned short bits_per_sample;
    char data_chunk_id[4];
    unsigned long data_size;
  };

 public:
  /* Override */
  virtual void Initialize(HWND hwnd, UINT width, UINT height) override;
  virtual void Shutdown() override;
  virtual BOOL OnUpdate() override;

 private:
  void LoadWave();
  void UnloadWave();
  IDirectSound8* dsound_ = nullptr;
  IDirectSoundBuffer* primary_buffer_ = nullptr;
  IDirectSoundBuffer8* secondary_buffer_ = nullptr;
};

SoundManagerClass& SoundManager();
}  // namespace naiive::manager
#endif
