//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/CommonHeader.h"
#include "ppapi/cpp/audio.h"
#include "MainInstance.h"
#include "../SourceCommon/Sound/WaveLoader.h"

SoundPlayer::SoundPlayer()
{
    m_NumAudioBuffersLoaded = 0;

    const uint32_t kSampleFrameCount = 1000u; //32768u; //4096u;

// TODONACLUPDATE: test -> added g_pInstance to call below.
    // Ask the browser/device for an appropriate sample frame count size.
    m_SampleFrameCount = pp::AudioConfig::RecommendSampleFrameCount( g_pInstance, PP_AUDIOSAMPLERATE_44100, kSampleFrameCount );

    // Create an audio configuration resource.
    pp::AudioConfig audio_config = pp::AudioConfig( g_pInstance, PP_AUDIOSAMPLERATE_44100, m_SampleFrameCount );

    // Create an audio resource.
    m_AudioObj = pp::Audio( g_pInstance, audio_config, FillBufferCallback, this );

    // Start playback when the module instance is initialized.
    m_AudioObj.StartPlayback();

    for( int i=0; i<MAX_AUDIO_FILES; i++ )
    {
        m_WaveDescriptors[i].valid = false;
    }

    m_SoundCueQueue.AllocateObjects( MAX_AUDIO_FILES_QUEUED );
    for( unsigned int i=0; i<m_SoundCueQueue.Length(); i++ )
    {
        m_SoundCueQueue.AddInactiveObject( MyNew SoundCueWrapper );
    }

    LOGInfo( LOGTag, "NaCL SoundPlayer created: m_SampleFrameCount:%d\n", m_SampleFrameCount );
}

SoundPlayer::~SoundPlayer()
{
    m_AudioObj.StopPlayback();
    m_SoundCueQueue.DeleteAllObjectsInPool();
}

void SoundPlayer::OnFocusGained()
{
}

void SoundPlayer::OnFocusLost()
{
}

int SoundPlayer::LoadSound(const char* buffer, unsigned int buffersize)
{
    LOGInfo( LOGTag, "NaCL SoundPlayer::LoadSound, buffersize:%d\n", buffersize );

    MyAssert( m_NumAudioBuffersLoaded < MAX_AUDIO_FILES );

    m_WaveDescriptors[m_NumAudioBuffersLoaded] = WaveLoader::ParseWaveBuffer( buffer, buffersize );
    MyAssert( m_WaveDescriptors[m_NumAudioBuffersLoaded].valid );

    LOGInfo( LOGTag, "NaCL SoundPlayer parsed, bytes:%d, channels:%d, samples:%d, size:%d\n", 
        m_WaveDescriptors[m_NumAudioBuffersLoaded].bytespersample,
        m_WaveDescriptors[m_NumAudioBuffersLoaded].numchannels,
        m_WaveDescriptors[m_NumAudioBuffersLoaded].samplerate,
        m_WaveDescriptors[m_NumAudioBuffersLoaded].datasize );

    m_NumAudioBuffersLoaded++;

    return m_NumAudioBuffersLoaded-1;
}

void SoundPlayer::Shutdown()
{
}

void SoundPlayer::PlaySound(int soundid, bool looping)
{
    LOGInfo( LOGTag, "NaCL SoundPlayer PlaySound soundid %d looping %d\n", soundid, looping );

    if( soundid == -1 )
        return;
    
    SoundCueWrapper* pCueInfo = m_SoundCueQueue.MakeObjectActive();

    if( pCueInfo )
    {
        pCueInfo->descindex = soundid;
        pCueInfo->offset = 0;
        pCueInfo->looping = looping;
    }
}

void SoundPlayer::StopSound(int soundid)
{
}

void SoundPlayer::PauseSound(int soundid)
{
}

void SoundPlayer::ResumeSound(int soundid)
{
}

void SoundPlayer::PauseAll()
{
}

void SoundPlayer::ResumeAll()
{
}

inline short floattoshortclamped(float value)
{
    if( value <= -32768.0f )
    {
        return -32768.0f;
    }
    else if( value >= 32767.0f )
    {
        return 32767.0f;
    }
    
    return value;

    // TODO: try this at some point: from Glenn Barnett's comment on http://stackoverflow.com/questions/376036/algorithm-to-mix-sound
    //float waveshape_distort( float in ) {
    //    if(in <= -1.25f) {
    //        return -0.984375;
    //    } else if(in >= 1.25f) {
    //        return 0.984375;
    //    } else {    
    //        return 1.1f * in - 0.2f * in * in * in;
    //    }
    //}
}

// Function called by the browser when it needs more audio samples.
void FillBufferCallback(void* samples, uint32_t buffer_size, void* data)
{
    // https://developers.google.com/native-client/devguide/coding/audio
    // this func runs in a thread...
    // avoid c runtime and pepper calls and be thread safe without locking.
    // In short, the audio (callback) thread should use "lock-free" techniques and avoid making CRT library calls.

    // buffer_size = 16bit * 2channel * frame count(4096)

    SoundPlayer* pSoundPlayer = ((SoundPlayer*)data);

    //LOGInfo( LOGTag, "NaCL SoundPlayer FillBufferCallback start, buffer size:%d\n", buffer_size );

    short* pBuffer = (short*)samples;
    memset( samples, 0, buffer_size );

    for( unsigned int i=0; i<pSoundPlayer->m_SoundCueQueue.m_ActiveObjects.Count(); i++ )
    {
        SoundCueWrapper* pCueInfo = pSoundPlayer->m_SoundCueQueue.m_ActiveObjects[i];
        MyAssert( pCueInfo );

        MyWaveDescriptor* pWaveDesc = &pSoundPlayer->m_WaveDescriptors[pCueInfo->descindex];
        MyAssert( pWaveDesc );
            
        int sizetocopy = 0;

        if( pCueInfo->looping == false )
        {
            LOGInfo( LOGTag, "NaCL SoundPlayer FillBufferCallback playing sound:%d size:%d offset:%d\n", pCueInfo->descindex, pWaveDesc->datasize, pCueInfo->offset );

            sizetocopy = pWaveDesc->datasize - pCueInfo->offset;
            if( sizetocopy > buffer_size )
            {
                sizetocopy = buffer_size;
            }
            else if( sizetocopy <= buffer_size )
            {
                LOGInfo( LOGTag, "NaCL SoundPlayer FillBufferCallback removing sound %d\n", pCueInfo->descindex );
                // remove the sound from the list.
                pSoundPlayer->m_SoundCueQueue.MakeObjectInactive( pCueInfo );
                i--;
            }
        }
        else
        {
            sizetocopy = buffer_size;
        }

        //LOGInfo( LOGTag, "NaCL SoundPlayer FillBufferCallback copy, sizetocopy:%d offset:%d\n", sizetocopy, pCueInfo->offset );

        if( pWaveDesc->bytespersample == 2 )
        {
            short* pWaveData = ((short*)pWaveDesc->data);

            if( pWaveDesc->numchannels == 1 )
            {
                if( pWaveDesc->samplerate == 44100 )
                {
                    // convert 44khz 16-bit mono sample into 44khz 16-bit stereo.
                    for( int i=0; i<sizetocopy/2; i+=2 )
                    {
                        float newvalue;

                        newvalue = pBuffer[i+0] + pWaveData[pCueInfo->offset/2]; pBuffer[i+0] = floattoshortclamped(newvalue);
                        newvalue = pBuffer[i+1] + pWaveData[pCueInfo->offset/2]; pBuffer[i+1] = floattoshortclamped(newvalue);

                        pCueInfo->offset += 2;
                        if( pCueInfo->offset >= pWaveDesc->datasize )
                            pCueInfo->offset = 0;
                    }
                }
                else
                {
                    // convert 22khz 16-bit mono sample into 44khz 16-bit stereo.
                    for( int i=0; i<sizetocopy/2; i+=4 )
                    {
                        float newvalue;

                        newvalue = pBuffer[i+0] + pWaveData[pCueInfo->offset/2]; pBuffer[i+0] = floattoshortclamped(newvalue);
                        newvalue = pBuffer[i+1] + pWaveData[pCueInfo->offset/2]; pBuffer[i+1] = floattoshortclamped(newvalue);
                        newvalue = pBuffer[i+2] + pWaveData[pCueInfo->offset/2]; pBuffer[i+2] = floattoshortclamped(newvalue);
                        newvalue = pBuffer[i+3] + pWaveData[pCueInfo->offset/2]; pBuffer[i+3] = floattoshortclamped(newvalue);

                        pCueInfo->offset += 2;
                        if( pCueInfo->offset >= pWaveDesc->datasize )
                            pCueInfo->offset = 0;
                    }
                }
            }
            else
            {
                // 44khz, 16bit, stereo... same format as our output, so just copy it in.
                if( pWaveDesc->samplerate == 44100 )
                {
                    for( int i=0; i<sizetocopy/2; i++ )
                    {
                        float newvalue;

                        //((char*)samples)[i] = pWaveDesc->data[pCueInfo->offset];
                        newvalue = pBuffer[i] + pWaveData[pCueInfo->offset/2]; pBuffer[i] = floattoshortclamped(newvalue);
                        
                        pCueInfo->offset += 2;
                        if( pCueInfo->offset >= pWaveDesc->datasize )
                            pCueInfo->offset = 0;
                    }

                    // useless? memcpy attempt, since we'll need to mix with other sounds.
                        // probably useful if we can guarantee it comes first.
                    //// don't copy more than what's left in the source wave buffer.
                    //if( sizetocopy > pWaveDesc->datasize - pCueInfo->offset )
                    //    sizetocopy = pWaveDesc->datasize - pCueInfo->offset;

                    //memcpy( samples, &pWaveDesc->data[pCueInfo->offset], sizetocopy );
                }
                else
                {
                    // not handling 22050khz, 16bit stereo.
                    // not handling 11025khz, 16bit stereo.
                }
            }
        }
        else
        {
            // not handling anything but 16bit samples.
        }
    }

    //LOGInfo( LOGTag, "NaCL SoundPlayer FillBufferCallback end, buffer size:%d\n", buffer_size );
}
