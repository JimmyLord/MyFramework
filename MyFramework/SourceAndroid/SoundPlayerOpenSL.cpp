//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/CommonHeader.h"

// Interfaces
SLEngineItf g_ppEngineInterface;
SLVolumeItf g_ppOutputMixVolume;

void CheckForErrors(SLresult result, const char* string)
{
    const char* errorstrings[] =
    {
        "SL_RESULT_SUCCESS",
        "SL_RESULT_PRECONDITIONS_VIOLATED",
        "SL_RESULT_PARAMETER_INVALID",
        "SL_RESULT_MEMORY_FAILURE",
        "SL_RESULT_RESOURCE_ERROR",
        "SL_RESULT_RESOURCE_LOST",
        "SL_RESULT_IO_ERROR",
        "SL_RESULT_BUFFER_INSUFFICIENT",
        "SL_RESULT_CONTENT_CORRUPTED",
        "SL_RESULT_CONTENT_UNSUPPORTED",
        "SL_RESULT_CONTENT_NOT_FOUND",
        "SL_RESULT_PERMISSION_DENIED",
        "SL_RESULT_FEATURE_UNSUPPORTED",
        "SL_RESULT_INTERNAL_ERROR",
        "SL_RESULT_UNKNOWN_ERROR",
        "SL_RESULT_OPERATION_ABORTED",
        "SL_RESULT_CONTROL_LOST",
        "Invalid result code",
    };

    if( result != SL_RESULT_SUCCESS )
    {
        if( result > SL_RESULT_CONTROL_LOST )
            result = SL_RESULT_CONTROL_LOST + 1;

        LOGError( LOGTag, "OpenSL Error: %s: %s\n", string, errorstrings[result] );
    }
}

SoundPlayer::SoundPlayer()
{
    m_NumQueuedSounds = 0;

    g_ppOpenSLEngine = 0;
    g_ppOutputMix = 0;
    g_ppAudioPlayer = 0;

    SLresult result;

    // Create the OpenSL Engine
    {
        SLEngineOption EngineOptions[] =
        { 
            (SLuint32)SL_ENGINEOPTION_THREADSAFE, 
            (SLuint32)SL_BOOLEAN_TRUE
        };
        result = slCreateEngine( &g_ppOpenSLEngine, 1, EngineOptions, 0, 0, 0 );
        CheckForErrors( result, "slCreateEngine" );

        // Allocate the OpenSL Engine. False for synchronous allocation.
        result = (*g_ppOpenSLEngine)->Realize( g_ppOpenSLEngine, SL_BOOLEAN_FALSE );
        CheckForErrors( result, "(*g_ppOpenSLEngine)->Realize" );

        // Get the engine interface
        result = (*g_ppOpenSLEngine)->GetInterface( g_ppOpenSLEngine, SL_IID_ENGINE, (void*)&g_ppEngineInterface ); 
        CheckForErrors( result, "(*g_ppOpenSLEngine)->GetInterface SL_IID_ENGINE" );
    }

    // Create an Output Mix object
    {
        //const SLInterfaceID ids[] = { SL_IID_VOLUME };
        //const SLboolean req[] = { SL_BOOLEAN_FALSE };
        result = (*g_ppEngineInterface)->CreateOutputMix( g_ppEngineInterface, &g_ppOutputMix, 0, 0, 0 ); //1, ids, req );
        CheckForErrors( result, "(*g_ppEngineInterface)->CreateOutputMix" );

        // Allocate the Output Mix object. False for synchronous allocation.
        (*g_ppOutputMix)->Realize( g_ppOutputMix, SL_BOOLEAN_FALSE );
        CheckForErrors( result, "(*g_ppOutputMix)->Realize" );

        //result = (*g_ppOutputMix)->GetInterface( g_ppOutputMix, SL_IID_VOLUME, &g_ppOutputMixVolume );
        //CheckForErrors( result, "(*g_ppOutputMix)->GetInterface SL_IID_VOLUME" );
        //if( result != SL_RESULT_SUCCESS )
        //    g_ppOutputMixVolume = 0;
    }

    //LOGInfo( LOGTag, "SoundPlayer::SoundPlayer() Done\n" );

    //PlaySound( 0 );
    TestOpenSL_BufferQueue();
}

SoundPlayer::~SoundPlayer()
{
    Shutdown();
}

void SoundPlayer::Shutdown()
{
    (*g_ppAudioPlayer)->Destroy( g_ppAudioPlayer );
    (*g_ppOutputMix)->Destroy( g_ppOutputMix );
    (*g_ppOpenSLEngine)->Destroy( g_ppOpenSLEngine );
}

void SoundPlayer::OnFocusGained()
{
}

void SoundPlayer::OnFocusLost()
{
}

void SoundPlayer::Tick(double TimePassed)
{
}

void SoundPlayer::ReallyPlaySound(int soundid)
{
}

SoundObject* SoundPlayer::LoadSound(const char* path, const char* ext)
{
    int i=0;
    for( i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_Sound == 0 )
            break;
    }

    if( i < MAX_SOUNDS )
    {
        // store the sound id into a soundobject and return the soundobject.
        //m_Sounds[i].m_Sound = soundid;
        //return &m_Sounds[i];
    }

    return 0;
}

SoundObject* SoundPlayer::LoadSound(const char* fullpath)
{
    int i=0;
    for( i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_Sound == 0 )
            break;
    }

    if( i < MAX_SOUNDS )
    {
        // store the sound id into a soundobject and return the soundobject.
        //m_Sounds[i].m_Sound = soundid;
        //return &m_Sounds[i];
    }

    return 0;
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    //LOGInfo( LOGTag, "PlaySound - %d", soundid );

    SLresult result;

    // Setup the audio source, trying to decode a wav from assets folder.
    SLDataLocator_URI uri;
    //SLDataLocator_AndroidFD androidfd;
    SLDataFormat_MIME mime;
    SLDataSource audioSource;
    {
        uri.locatorType = SL_DATALOCATOR_URI;
        //uri.URI = (SLchar*)"file:///android_asset/Data/Audio/test.wav";
        uri.URI = (SLchar*)"/assets/Data/Audio/test.wav";
        //uri.URI = (SLchar*)"assets/Data/Audio/test.wav";

        //AAssetManager* pManager = AAssetManager_fromJava( g_pJavaEnvironment, g_pAssetManager );
        //androidfd.locatorType = SL_DATALOCATOR_ANDROIDFD;
        //androidfd.fd;
        //androidfd.offset;
        //androidfd.length;

        mime.formatType = SL_DATAFORMAT_MIME;
        mime.mimeType = 0; //(SLchar*)"audio/x-wav";
        mime.containerType = SL_CONTAINERTYPE_UNSPECIFIED; //SL_CONTAINERTYPE_WAV;

        audioSource.pLocator = (void*)&uri;
        //audioSource.pLocator = (void*)&androidfd;
        audioSource.pFormat = (void*)&mime;
    }

    // Setup the data sink structure
    SLDataLocator_OutputMix locatorOutputMix;
    SLDataSink audioSink;
    {
        locatorOutputMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
        locatorOutputMix.outputMix = g_ppOutputMix;
 
        audioSink.pLocator = &locatorOutputMix;
        audioSink.pFormat = 0;
    }

    // Create an audio player
    {
        LOGInfo( LOGTag, "About to create Audio Player\n" );

        // Set ids required for audioPlayer interface
        const SLInterfaceID ids[] = { SL_IID_PREFETCHSTATUS, SL_IID_VOLUME };
        const SLboolean req[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE };

        result = (*g_ppEngineInterface)->CreateAudioPlayer( g_ppEngineInterface, &g_ppAudioPlayer, &audioSource, &audioSink, 0, 0, 0 );//2, ids, req );
        CheckForErrors( result, "(*g_ppEngineInterface)->CreateAudioPlayer" );
 
        // Realize the player in synchronous mode.
        result = (*g_ppAudioPlayer)->Realize( g_ppAudioPlayer, SL_BOOLEAN_FALSE );
        CheckForErrors( result, "(*g_ppAudioPlayer)->Realize" );
    }
 
    // Get play interface and play the sound
    {
        SLPlayItf playInterface;
        result = (*g_ppAudioPlayer)->GetInterface( g_ppAudioPlayer, SL_IID_PLAY, &playInterface );
        CheckForErrors( result, "(*g_ppAudioPlayer)->GetInterface SL_IID_PLAY" );

        result = (*playInterface)->SetPlayState( playInterface, SL_PLAYSTATE_PLAYING );
        CheckForErrors( result, "(*playInterface)->SetPlayState -> SL_PLAYSTATE_PLAYING" );
    }

    return 0; // TODO: should return some unique id for the sound, so it can be stopped/paused/etc.
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

// code mostly adapted from these 2 sources:
// https://www.khronos.org/registry/OpenSL-ES/specs/OpenSL_ES_Specification_1.0.1.pdf
// http://vec3.ca/getting-started-with-opensl-on-android/

// Structure for passing information to bufferqueue callback function
struct CallbackContextStruct
{
    SLPlayItf playItf;
    SLint16* pDataBase; // Base address of local audio data storage
    SLint16* pData;     // Current address of local audio data storage
    SLuint32 size;
}; 

void BufferQueueCallback(SLBufferQueueItf queueInterface, void* pContext)
{
    //SLresult result;
    //CallbackContextStruct* pCallbackContext = (CallbackContextStruct*)pContext;

    //if( pCallbackContext->pData < (pCallbackContext->pDataBase + pCallbackContext->size) )
    //{
    //    int bufferSegmentSizeInBytes = 2 * AUDIO_DATA_SEGMENT_SIZE;
    //    result = (*queueInterface)->Enqueue( queueInterface, (void*)pCallbackContext->pData, bufferSegmentSizeInBytes );
    //    CheckForErrors( result, "(*queueInterface)->Enqueue" );

    //    // Increase data pointer by buffer size
    //    pCallbackContext->pData += AUDIO_DATA_SEGMENT_SIZE;

    //    LOGInfo( LOGTag, "Queued up more data\n" );
    //}
    //else
    //{
    //    LOGInfo( LOGTag, "Out of data to queue\n" );
    //}
} 

//void SLAPIENTRY play_callback(SLPlayItf player, void* context, SLuint32 event)
void PlayCallback(SLPlayItf player, void* context, SLuint32 event)
{
    //if( event & SL_PLAYEVENT_HEADATEND )
    //    is_done_buffer = true;
}

void SoundPlayer::TestOpenSL_BufferQueue()
{
    SLresult result;

    // Generate some noise
    static const int AUDIO_DATA_STORAGE_SIZE = 32768;
    SLint16 pcmData[AUDIO_DATA_STORAGE_SIZE];
    for( int i=0; i<AUDIO_DATA_STORAGE_SIZE/2; i+=2 )
    {
        pcmData[i] = -100;
        pcmData[i+1] = 100;
    }

    // Setup the data source structure for the buffer queue
    //   works as well with SLDataLocator_AndroidSimpleBufferQueue, SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE and SL_IID_ANDROIDSIMPLEBUFFERQUEUE
    SLDataLocator_BufferQueue bufferQueue;
    bufferQueue.locatorType = SL_DATALOCATOR_BUFFERQUEUE;
    bufferQueue.numBuffers = 1;
    
    // Setup the format of the content in the buffer queue
    SLDataFormat_PCM pcmDataFormat;
    pcmDataFormat.formatType = SL_DATAFORMAT_PCM;
    pcmDataFormat.numChannels = 1;
    pcmDataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1;
    pcmDataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcmDataFormat.containerSize = 16;
    pcmDataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
    pcmDataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSource;
    audioSource.pFormat = &pcmDataFormat;
    audioSource.pLocator = &bufferQueue;

    // Setup the data sink structure
    SLDataLocator_OutputMix locatorOutputMix;
    locatorOutputMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    locatorOutputMix.outputMix = g_ppOutputMix;
    
    SLDataSink audioSink;
    audioSink.pLocator = &locatorOutputMix;
    audioSink.pFormat = 0;

    // Create an audio player
    {
        // Set ids required for audioPlayer interface
        const SLInterfaceID ids[] = { SL_IID_BUFFERQUEUE };
        const SLboolean req[] = { SL_BOOLEAN_TRUE };

        result = (*g_ppEngineInterface)->CreateAudioPlayer( g_ppEngineInterface, &g_ppAudioPlayer, &audioSource, &audioSink, 1, ids, req );
        CheckForErrors( result, "(*g_ppEngineInterface)->CreateAudioPlayer" );

        // Realize the player in synchronous mode.
        result = (*g_ppAudioPlayer)->Realize( g_ppAudioPlayer, SL_BOOLEAN_FALSE );
        CheckForErrors( result, "(*g_ppAudioPlayer)->Realize" );
    }

    // Get interfaces
    SLPlayItf playInterface;
    result = (*g_ppAudioPlayer)->GetInterface( g_ppAudioPlayer, SL_IID_PLAY, &playInterface ); 
    CheckForErrors( result, "(*g_ppAudioPlayer)->GetInterface SL_IID_PLAY" );

    SLBufferQueueItf bufferQueueInterface;
    result = (*g_ppAudioPlayer)->GetInterface( g_ppAudioPlayer, SL_IID_BUFFERQUEUE, &bufferQueueInterface );
    CheckForErrors( result, "(*g_ppAudioPlayer)->GetInterface SL_IID_BUFFERQUEUE" );

    //// Register a BufferQueue callback
    //// Initialize the context for Buffer queue callback functions
    //CallbackContextStruct callbackContext;
    //callbackContext.pDataBase = (SLint16*)&pcmData;
    //callbackContext.pData = callbackContext.pDataBase;
    //callbackContext.size = sizeof( pcmDataFormat );

    //result = (*bufferQueueInterface)->RegisterCallback( bufferQueueInterface, BufferQueueCallback, NULL);
    //CheckForErrors( result, "(*bufferQueueInterface)->RegisterCallback" );

    //// Register a Play callback 
    //result = (*playInterface)->RegisterCallback( playInterface, play_callback, 0 );
    //CheckForErrors( result, "(*playInterface)->RegisterCallback" );

    //result = (*playInterface)->SetCallbackEventsMask( playInterface, SL_PLAYEVENT_HEADATEND );
    //CheckForErrors( result, "(*playInterface)->SetCallbackEventsMask" );

    //// Before we start set volume to -3dB (-300mB)
    //if( g_ppOutputMixVolume != 0 )
    //{
    //    result = (*g_ppOutputMixVolume)->SetVolumeLevel( g_ppOutputMixVolume, -300 );
    //}

    // Enqueue our entire buffer
    result = (*bufferQueueInterface)->Enqueue( bufferQueueInterface, pcmData, AUDIO_DATA_STORAGE_SIZE*2 );
    CheckForErrors( result, "(*bufferQueueInterface)->Enqueue" );

    // Start playback
    result = (*playInterface)->SetPlayState( playInterface, SL_PLAYSTATE_PLAYING );
    CheckForErrors( result, "(*playInterface)->SetPlayState -> SL_PLAYSTATE_PLAYING" );

    // Enqueue a few buffers to get the ball rolling
    //int bufferSegmentSizeInBytes = 2 * AUDIO_DATA_SEGMENT_SIZE;

    //result = (*bufferQueueInterface)->Enqueue( bufferQueueInterface, callbackContext.pData, bufferSegmentSizeInBytes );
    //CheckForErrors( result, "(*bufferQueueInterface)->Enqueue" );
    //callbackContext.pData += AUDIO_DATA_SEGMENT_SIZE;

    //result = (*bufferQueueInterface)->Enqueue( bufferQueueInterface, callbackContext.pData, bufferSegmentSizeInBytes );
    //CheckForErrors( result, "(*bufferQueueInterface)->Enqueue" );
    //callbackContext.pData += AUDIO_DATA_SEGMENT_SIZE;

    //result = (*bufferQueueInterface)->Enqueue( bufferQueueInterface, callbackContext.pData, bufferSegmentSizeInBytes );
    //CheckForErrors( result, "(*bufferQueueInterface)->Enqueue" );
    //callbackContext.pData += AUDIO_DATA_SEGMENT_SIZE;

    //// Play the PCM samples using a buffer queue
    //result = (*playInterface)->SetPlayState( playInterface, SL_PLAYSTATE_PLAYING ); 
    //CheckForErrors( result, "(*playInterface)->SetPlayState" );

    //// Wait until the PCM data is done playing, the buffer queue callback
    //// will continue to queue buffers until the entire PCM data has been
    //// played. This is indicated by waiting for the count member of the
    //// SLBufferQueueState to go to zero.
    //SLBufferQueueState bufferQueueState;
    //result = (*bufferQueueInterface)->GetState( bufferQueueInterface, &bufferQueueState );
    //CheckForErrors( result, "(*bufferQueueInterface)->GetState" );

    //LOGInfo( LOGTag, "queried buffer queue interface\n" );

    //int lastcount = -1;
    //while( bufferQueueState.count )
    //{
    //    result = (*bufferQueueInterface)->GetState( bufferQueueInterface, &bufferQueueState );
    //    CheckForErrors( result, "(*bufferQueueInterface)->GetState" );

    //    if( lastcount != bufferQueueState.count )
    //    {
    //        lastcount = bufferQueueState.count;
    //        LOGInfo( LOGTag, "queried buffer queue interface in loop - count(%d)\n", bufferQueueState.count );
    //    }
    //}

    //// Make sure player is stopped
    //result = (*playInterface)->SetPlayState( playInterface, SL_PLAYSTATE_STOPPED );
    //CheckForErrors( result, "(*playInterface)->SetPlayState -> SL_PLAYSTATE_STOPPED" );

    //LOGInfo( LOGTag, "SetPlayState to stop\n" );
}
