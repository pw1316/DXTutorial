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
  buffer_desc.dwFlags =
      DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
  buffer_desc.dwBufferBytes = 0;
  buffer_desc.dwReserved = 0;
  buffer_desc.lpwfxFormat = nullptr;
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

  hr = primary_buffer_->QueryInterface(IID_IDirectSound3DListener8,
                                       (LPVOID*)&listener_);
  ASSERT(SUCCEEDED(hr));
  hr = listener_->SetPosition(0, 0, 0, DS3D_IMMEDIATE);
  ASSERT(SUCCEEDED(hr));

  LoadWave();
  OnUpdate();
}

void SoundManagerClass::Shutdown() {
  UnloadWave();

  SafeRelease(&listener_);
  SafeRelease(&primary_buffer_);
  SafeRelease(&dsound_);
  return;
}

BOOL SoundManagerClass::OnUpdate() {
  HRESULT hr = S_OK;

  DWORD status;
  secondary_buffer_1_->GetStatus(&status);
  if (status == 0) {
    hr = secondary_buffer_1_->SetCurrentPosition(0);
    ASSERT(SUCCEEDED(hr));

    hr = secondary_buffer_1_->SetVolume(DSBVOLUME_MAX);
    ASSERT(SUCCEEDED(hr));

    // hr = secondary_buffer_1_->Play(0, 0, 0);
    // ASSERT(SUCCEEDED(hr));
  }
  secondary_buffer_2_->GetStatus(&status);
  if (status == 0) {
    hr = secondary_buffer_2_->SetCurrentPosition(0);
    ASSERT(SUCCEEDED(hr));

    hr = secondary_buffer_2_->SetVolume(DSBVOLUME_MAX);
    ASSERT(SUCCEEDED(hr));

    LOG(LOG_INFO)(position_.x, position_.y, position_.z);
    hr = secondary_buffer_2_3d_->SetPosition(position_.x, position_.y,
                                             position_.z, DS3D_IMMEDIATE);
    ASSERT(SUCCEEDED(hr));

    hr = secondary_buffer_2_->Play(0, 0, 0);
    ASSERT(SUCCEEDED(hr));
  }
  return TRUE;
}

void SoundManagerClass::LoadWave() {
  HRESULT hr = S_OK;

  // 2D Sound
  std::ifstream ifs("res/foo.wav", std::ios::binary);
  ASSERT(ifs.is_open());

  WaveHeaderType wave_header;
  ReadAndCheckWaveHeader(ifs, wave_header);

  WAVEFORMATEX wave_format_ex;
  ASSERT(wave_header.num_channels == 2);
  wave_format_ex.wFormatTag = WAVE_FORMAT_PCM;
  wave_format_ex.nChannels = wave_header.num_channels;
  wave_format_ex.nSamplesPerSec = wave_header.sample_rate;
  wave_format_ex.wBitsPerSample = wave_header.bits_per_sample;
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
  hr = temp_buffer->QueryInterface(IID_IDirectSoundBuffer8,
                                   (void**)&secondary_buffer_1_);
  SafeRelease(&temp_buffer);
  ASSERT(SUCCEEDED(hr));

  ifs.seekg(sizeof(WaveHeaderType), std::ios::beg);
  std::vector<UCHAR> wave_data(wave_header.data_size);
  ifs.read((char*)&wave_data[0], wave_header.data_size);
  ASSERT(ifs);
  ifs.close();

  UCHAR* buffer_ptr;
  ULONG buffer_size;
  hr = secondary_buffer_1_->Lock(0, wave_header.data_size, (void**)&buffer_ptr,
                                 &buffer_size, nullptr, nullptr, 0);
  ASSERT(SUCCEEDED(hr));
  memcpy(buffer_ptr, &wave_data[0], wave_header.data_size);
  hr = secondary_buffer_1_->Unlock(buffer_ptr, buffer_size, nullptr, 0);
  ASSERT(SUCCEEDED(hr));

  // 3D Sound
  ifs.open("res/foo3d.wav", std::ios::binary);
  ASSERT(ifs.is_open());

  ZeroMemory(&wave_header, sizeof(wave_header));
  ReadAndCheckWaveHeader(ifs, wave_header);

  ASSERT(wave_header.num_channels == 1);
  ZeroMemory(&wave_format_ex, sizeof(wave_format_ex));
  wave_format_ex.wFormatTag = WAVE_FORMAT_PCM;
  wave_format_ex.nChannels = wave_header.num_channels;
  wave_format_ex.nSamplesPerSec = wave_header.sample_rate;
  wave_format_ex.wBitsPerSample = wave_header.bits_per_sample;
  wave_format_ex.nBlockAlign =
      (wave_format_ex.wBitsPerSample / 8) * wave_format_ex.nChannels;
  wave_format_ex.nAvgBytesPerSec =
      wave_format_ex.nSamplesPerSec * wave_format_ex.nBlockAlign;
  wave_format_ex.cbSize = 0;

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.dwSize = sizeof(DSBUFFERDESC);
  buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
  buffer_desc.dwBufferBytes = wave_header.data_size;
  buffer_desc.dwReserved = 0;
  buffer_desc.lpwfxFormat = &wave_format_ex;
  buffer_desc.guid3DAlgorithm = GUID_NULL;

  hr = dsound_->CreateSoundBuffer(&buffer_desc, &temp_buffer, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = temp_buffer->QueryInterface(IID_IDirectSoundBuffer8,
                                   (void**)&secondary_buffer_2_);
  SafeRelease(&temp_buffer);
  ASSERT(SUCCEEDED(hr));

  ifs.seekg(sizeof(WaveHeaderType), std::ios::beg);
  wave_data.resize(wave_header.data_size);
  ifs.read((char*)&wave_data[0], wave_header.data_size);
  ASSERT(ifs);
  ifs.close();

  hr = secondary_buffer_2_->Lock(0, wave_header.data_size, (void**)&buffer_ptr,
                                 &buffer_size, nullptr, nullptr, 0);
  ASSERT(SUCCEEDED(hr));
  memcpy(buffer_ptr, &wave_data[0], wave_header.data_size);
  hr = secondary_buffer_2_->Unlock(buffer_ptr, buffer_size, nullptr, 0);
  ASSERT(SUCCEEDED(hr));

  hr = secondary_buffer_2_->QueryInterface(IID_IDirectSound3DBuffer8,
                                           (LPVOID*)&secondary_buffer_2_3d_);
  ASSERT(SUCCEEDED(hr));
}

void SoundManagerClass::UnloadWave() {
  SafeRelease(&secondary_buffer_2_3d_);
  SafeRelease(&secondary_buffer_2_);
  SafeRelease(&secondary_buffer_1_);
}

void SoundManagerClass::ReadAndCheckWaveHeader(std::ifstream& ifs,
                                               WaveHeaderType& header) {
  ifs.read((char*)&header, sizeof(WaveHeaderType));
  ASSERT(ifs);

  /* RIFF */
  ASSERT((header.chunk_id[0] == 'R') && (header.chunk_id[1] == 'I') &&
         (header.chunk_id[2] == 'F') && (header.chunk_id[3] == 'F'));

  /* WAVE */
  ASSERT((header.format[0] == 'W') && (header.format[1] == 'A') &&
         (header.format[2] == 'V') && (header.format[3] == 'E'));

  /* fmt */
  ASSERT((header.sub_chunk_id[0] == 'f') && (header.sub_chunk_id[1] == 'm') &&
         (header.sub_chunk_id[2] == 't') && (header.sub_chunk_id[3] == ' '));

  /* WaveFormat */
  ASSERT(header.audio_format == WAVE_FORMAT_PCM);
  // ASSERT(header.num_channels == 2);
  ASSERT(header.sample_rate == 44100);
  ASSERT(header.bits_per_sample == 16);

  /* data */
  ASSERT((header.data_chunk_id[0] == 'd') && (header.data_chunk_id[1] == 'a') &&
         (header.data_chunk_id[2] == 't') && (header.data_chunk_id[3] == 'a'));
}

SoundManagerClass& naiive::manager::SoundManager() {
  static SoundManagerClass obj;
  return obj;
}
}  // namespace naiive::manager
