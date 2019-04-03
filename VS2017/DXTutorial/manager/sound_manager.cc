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

#include "sound_manager.h"

#include <fstream>
#include <vector>

namespace naiive::manager {
void SoundManagerClass::Initialize(HWND hwnd, UINT width, UINT height) {
  UNREFERENCED_PARAMETER(width);
  UNREFERENCED_PARAMETER(height);
  HRESULT hr = S_OK;
  hr = DirectSoundCreate8(nullptr, &dsound_, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = dsound_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
  ASSERT(SUCCEEDED(hr));

  DSBUFFERDESC buffer_desc;
  buffer_desc.dwSize = sizeof(DSBUFFERDESC);
  buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
  buffer_desc.dwBufferBytes = 0;
  buffer_desc.dwReserved = 0;
  buffer_desc.lpwfxFormat = NULL;
  buffer_desc.guid3DAlgorithm = GUID_NULL;
  hr = dsound_->CreateSoundBuffer(&buffer_desc, &primary_buffer_, nullptr);
  ASSERT(SUCCEEDED(hr));

  WAVEFORMATEX wave_format_ex;
  wave_format_ex.wFormatTag = WAVE_FORMAT_PCM;
  wave_format_ex.nChannels = 2;
  wave_format_ex.nSamplesPerSec = 44100;
  wave_format_ex.wBitsPerSample = 16;
  wave_format_ex.nBlockAlign =
      (wave_format_ex.wBitsPerSample / 8) * wave_format_ex.nChannels;
  wave_format_ex.nAvgBytesPerSec =
      wave_format_ex.nSamplesPerSec * wave_format_ex.nBlockAlign;
  wave_format_ex.cbSize = 0;
  hr = primary_buffer_->SetFormat(&wave_format_ex);

  // LoadWave();
  OnUpdate();
}

void SoundManagerClass::Shutdown() {
  // UnloadWave();

  SafeRelease(&primary_buffer_);
  SafeRelease(&dsound_);
  return;
}

BOOL SoundManagerClass::OnUpdate() {
  // HRESULT hr = S_OK;

  // hr = secondary_buffer_->SetCurrentPosition(0);
  // ASSERT(SUCCEEDED(hr));

  // hr = secondary_buffer_->SetVolume(DSBVOLUME_MAX);
  // ASSERT(SUCCEEDED(hr));

  // hr = secondary_buffer_->Play(0, 0, 0);
  // ASSERT(SUCCEEDED(hr));
  return TRUE;
}

void SoundManagerClass::LoadWave() {
  HRESULT hr = S_OK;

  std::ifstream ifs("foo.wav", std::ios::binary);
  assert(ifs.is_open());  // TODO

  WaveHeaderType wave_header;
  ifs.read((char*)&wave_header, sizeof(WaveHeaderType));
  assert(ifs);

  /* RIFF */
  assert((wave_header.chunk_id[0] == 'R') && (wave_header.chunk_id[1] == 'I') &&
         (wave_header.chunk_id[2] == 'F') && (wave_header.chunk_id[3] == 'F'));

  /* WAVE */
  assert((wave_header.format[0] == 'W') && (wave_header.format[1] == 'A') &&
         (wave_header.format[2] == 'V') && (wave_header.format[3] == 'E'));

  /* fmt */
  assert((wave_header.sub_chunk_id[0] == 'f') &&
         (wave_header.sub_chunk_id[1] == 'm') &&
         (wave_header.sub_chunk_id[2] == 't') &&
         (wave_header.sub_chunk_id[3] == ' '));

  /* WaveFormat */
  assert(wave_header.audio_format == WAVE_FORMAT_PCM);
  assert(wave_header.num_channels == 2);
  assert(wave_header.sample_rate == 44100);
  assert(wave_header.bits_per_sample == 16);

  /* data */
  assert((wave_header.data_chunk_id[0] == 'd') &&
         (wave_header.data_chunk_id[1] == 'a') &&
         (wave_header.data_chunk_id[2] == 't') &&
         (wave_header.data_chunk_id[3] == 'a'));

  WAVEFORMATEX wave_format_ex;
  wave_format_ex.wFormatTag = WAVE_FORMAT_PCM;
  wave_format_ex.nChannels = 2;
  wave_format_ex.nSamplesPerSec = 44100;
  wave_format_ex.wBitsPerSample = 16;
  wave_format_ex.nBlockAlign =
      (wave_format_ex.wBitsPerSample / 8) * wave_format_ex.nChannels;
  wave_format_ex.nAvgBytesPerSec =
      wave_format_ex.nSamplesPerSec * wave_format_ex.nBlockAlign;
  wave_format_ex.cbSize = 0;

  DSBUFFERDESC buffer_desc;
  buffer_desc.dwSize = sizeof(DSBUFFERDESC);
  buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME;
  buffer_desc.dwBufferBytes = wave_header.data_size;
  buffer_desc.dwReserved = 0;
  buffer_desc.lpwfxFormat = &wave_format_ex;
  buffer_desc.guid3DAlgorithm = GUID_NULL;

  IDirectSoundBuffer* temp_buffer;
  hr = dsound_->CreateSoundBuffer(&buffer_desc, &temp_buffer, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = temp_buffer->QueryInterface(IID_IDirectSound3DBuffer8,
                                   (void**)&secondary_buffer_);
  SafeRelease(&temp_buffer);
  ASSERT(SUCCEEDED(hr));

  ifs.seekg(sizeof(WaveHeaderType), std::ios::beg);
  std::vector<UCHAR> wave_data(wave_header.data_size);
  ifs.read((char*)&wave_data[0], wave_header.data_size);
  assert(ifs);
  ifs.close();

  UCHAR* buffer_ptr;
  ULONG buffer_size;
  hr = secondary_buffer_->Lock(0, wave_header.data_size, (void**)&buffer_ptr,
                               &buffer_size, nullptr, nullptr, 0);
  ASSERT(SUCCEEDED(hr));
  memcpy(buffer_ptr, &wave_data[0], wave_header.data_size);
  hr = secondary_buffer_->Unlock(buffer_ptr, buffer_size, nullptr, 0);
  ASSERT(SUCCEEDED(hr));
}

void SoundManagerClass::UnloadWave() { SafeRelease(&secondary_buffer_); }

SoundManagerClass& naiive::manager::SoundManager() {
  static SoundManagerClass obj;
  return obj;
}
}  // namespace naiive::manager
