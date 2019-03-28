#ifndef __MANAGER_SOUND_MANAGER__
#define __MANAGER_SOUND_MANAGER__
#include <dsound.h>

#include <core/iview.h>

namespace naiive::Manager {
class SoundManagerClass : public core::IView {
  friend SoundManagerClass& SoundManager();

 private:
  struct WaveHeaderType {
    char chunkId[4];
    unsigned long chunkSize;
    char format[4];
    char subChunkId[4];
    unsigned long subChunkSize;
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned long sampleRate;
    unsigned long bytesPerSecond;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char dataChunkId[4];
    unsigned long dataSize;
  };

 public:
  /* Override */
  virtual void Initialize(HWND hWnd, UINT width, UINT height) override;
  virtual void Shutdown() override;
  virtual BOOL OnUpdate() override;

 private:
  void LoadWave();
  void UnloadWave();
  IDirectSound8* m_dsound = nullptr;
  IDirectSoundBuffer* m_primaryBuffer = nullptr;
  IDirectSoundBuffer8* m_secondaryBuffer = nullptr;
};

SoundManagerClass& SoundManager();
}  // namespace naiive::Manager
#endif
