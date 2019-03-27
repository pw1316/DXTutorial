#include "SoundManager.hpp"

#include <fstream>
#include <vector>

namespace Naiive::Manager {
void SoundManagerClass::Initialize(HWND hWnd, UINT width, UINT height) {
  HRESULT hr = S_OK;
  hr = DirectSoundCreate8(nullptr, &m_dsound, nullptr);
  FAILTHROW;
  hr = m_dsound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
  FAILTHROW;

  DSBUFFERDESC bufferDesc;
  bufferDesc.dwSize = sizeof(DSBUFFERDESC);
  bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
  bufferDesc.dwBufferBytes = 0;
  bufferDesc.dwReserved = 0;
  bufferDesc.lpwfxFormat = NULL;
  bufferDesc.guid3DAlgorithm = GUID_NULL;
  hr = m_dsound->CreateSoundBuffer(&bufferDesc, &m_primaryBuffer, nullptr);
  FAILTHROW;

  WAVEFORMATEX waveFormat;
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = 2;
  waveFormat.nSamplesPerSec = 44100;
  waveFormat.wBitsPerSample = 16;
  waveFormat.nBlockAlign =
      (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
  waveFormat.nAvgBytesPerSec =
      waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;
  hr = m_primaryBuffer->SetFormat(&waveFormat);

  // LoadWave();
  OnUpdate();
}

void SoundManagerClass::Shutdown() {
  // UnloadWave();

  SafeRelease(&m_primaryBuffer);
  SafeRelease(&m_dsound);
  return;
}

BOOL SoundManagerClass::OnUpdate() {
  // HRESULT hr = S_OK;

  // hr = m_secondaryBuffer->SetCurrentPosition(0);
  // FAILTHROW;

  // hr = m_secondaryBuffer->SetVolume(DSBVOLUME_MAX);
  // FAILTHROW;

  // hr = m_secondaryBuffer->Play(0, 0, 0);
  // FAILTHROW;
  return TRUE;
}

void SoundManagerClass::LoadWave() {
  HRESULT hr = S_OK;

  std::ifstream ifs("foo.wav", std::ios::binary);
  assert(ifs.is_open());  // TODO

  WaveHeaderType waveHeader;
  ifs.read((char*)&waveHeader, sizeof(WaveHeaderType));
  assert(ifs);

  /* RIFF */
  assert((waveHeader.chunkId[0] == 'R') && (waveHeader.chunkId[1] == 'I') &&
         (waveHeader.chunkId[2] == 'F') && (waveHeader.chunkId[3] == 'F'));

  /* WAVE */
  assert((waveHeader.format[0] == 'W') && (waveHeader.format[1] == 'A') &&
         (waveHeader.format[2] == 'V') && (waveHeader.format[3] == 'E'));

  /* fmt */
  assert(
      (waveHeader.subChunkId[0] == 'f') && (waveHeader.subChunkId[1] == 'm') &&
      (waveHeader.subChunkId[2] == 't') && (waveHeader.subChunkId[3] == ' '));

  /* WaveFormat */
  assert(waveHeader.audioFormat == WAVE_FORMAT_PCM);
  assert(waveHeader.numChannels == 2);
  assert(waveHeader.sampleRate == 44100);
  assert(waveHeader.bitsPerSample == 16);

  /* data */
  assert((waveHeader.dataChunkId[0] == 'd') &&
         (waveHeader.dataChunkId[1] == 'a') &&
         (waveHeader.dataChunkId[2] == 't') &&
         (waveHeader.dataChunkId[3] == 'a'));

  WAVEFORMATEX waveFormat;
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = 2;
  waveFormat.nSamplesPerSec = 44100;
  waveFormat.wBitsPerSample = 16;
  waveFormat.nBlockAlign =
      (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
  waveFormat.nAvgBytesPerSec =
      waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;

  DSBUFFERDESC bufferDesc;
  bufferDesc.dwSize = sizeof(DSBUFFERDESC);
  bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
  bufferDesc.dwBufferBytes = waveHeader.dataSize;
  bufferDesc.dwReserved = 0;
  bufferDesc.lpwfxFormat = &waveFormat;
  bufferDesc.guid3DAlgorithm = GUID_NULL;

  IDirectSoundBuffer* tempBuffer;
  hr = m_dsound->CreateSoundBuffer(&bufferDesc, &tempBuffer, nullptr);
  FAILTHROW;
  hr = tempBuffer->QueryInterface(IID_IDirectSound3DBuffer8,
                                  (void**)&m_secondaryBuffer);
  SafeRelease(&tempBuffer);
  FAILTHROW;

  ifs.seekg(sizeof(WaveHeaderType), std::ios::beg);
  std::vector<UCHAR> waveData(waveHeader.dataSize);
  ifs.read((char*)&waveData[0], waveHeader.dataSize);
  assert(ifs);
  ifs.close();

  UCHAR* bufferPtr;
  ULONG bufferSize;
  hr = m_secondaryBuffer->Lock(0, waveHeader.dataSize, (void**)&bufferPtr,
                               &bufferSize, nullptr, nullptr, 0);
  FAILTHROW;
  memcpy(bufferPtr, &waveData[0], waveHeader.dataSize);
  hr = m_secondaryBuffer->Unlock(bufferPtr, bufferSize, nullptr, 0);
  FAILTHROW;
}

void SoundManagerClass::UnloadWave() { SafeRelease(&m_secondaryBuffer); }

SoundManagerClass& Naiive::Manager::SoundManager() {
  static SoundManagerClass obj;
  return obj;
}
}  // namespace Naiive::Manager
