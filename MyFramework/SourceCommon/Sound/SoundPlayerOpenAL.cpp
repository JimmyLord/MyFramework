//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "WaveLoader.h"

//====================================================================================================
// SoundObject
//====================================================================================================
SoundObject::SoundObject()
{
    m_FullPath[0] = '\0';
    m_Sound = 0;

    m_pSourcePool = nullptr;

#if _DEBUG
    m_BaseCount = 1; // RefCount hack: since soundobjects are in an array in soundplayer, final ref won't be released.
#endif
}

void SoundObject::Release() // override from RefCount
{
    RefCount::Release();

    if( m_RefCount == 1 )
    {
        m_pSourcePool->ReturnObjectToPool( this );

        //this->Remove(); // remove from cpplist
    }
}

cJSON* SoundObject::ExportAsJSONObject()
{
    cJSON* jSound = cJSON_CreateObject();

    cJSON_AddStringToObject( jSound, "Path", m_FullPath );

    return jSound;
}

bool CheckForOpenALErrors(const char* description)
{
    ALenum error;
    if( (error = alGetError()) != AL_NO_ERROR )
    {
        LOGError( LOGTag, "OpenALError : %s - code(%d)\n", description, error);
        return true;
    }

    return false;
}

bool CheckForOpenALCErrors(ALCdevice* pDevice, const char* description)
{
    ALenum error;
    if( (error = alcGetError(pDevice)) != AL_NO_ERROR )
    {
        LOGError( LOGTag, "OpenALCError : %s - code(%d)\n", description, error);
        return true;
    }

    return false;
}

#if USE_ALUT
bool CheckForALUTErrors(const char* description)
{
    ALenum error;
    if( (error = alutGetError()) != ALUT_ERROR_NO_ERROR )
    {
        LOGError( LOGTag, "ALUTError : %s - code(%d)\n", description, error);
        return true;
    }

    return false;
}
#endif

#if false //MYFW_IOS
ALvoid  alBufferDataStaticProc(const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq)
{
    static alBufferDataStaticProcPtr proc = 0;

    if( proc == 0 )
    {
        proc = (alBufferDataStaticProcPtr)alGetProcAddress( (const ALCchar*)"alBufferDataStatic" );
    }

    if( proc )
        proc( bid, format, data, size, freq );

    return;
}
#endif

SoundPlayer::SoundPlayer()
{
    m_NextID = 0;

#if MYFW_EMSCRIPTEN
    return;
#endif

    // Init openAL.
#if USE_ALUT
    alutInit( 0, nullptr );
#else
    m_pDevice = alcOpenDevice( 0 );
    if( m_pDevice == nullptr )
    {
        LOGError( LOGTag, "alcOpenDevice() failed\n" );
        return;
    }
    m_pContext = alcCreateContext( m_pDevice, 0 );
    CheckForOpenALCErrors( m_pDevice, "alcCreateContext" );
    alcMakeContextCurrent( m_pContext );
    CheckForOpenALCErrors( m_pDevice, "alcMakeContextCurrent" );
#endif

    // Initialize the listener to 0,0,0 with no velocity, good for 2D sounds.
    alListener3f( AL_POSITION, 0, 0, 0 );
    CheckForOpenALErrors( "alListener3f( AL_POSITION" );
    alListener3f( AL_VELOCITY, 0, 0, 0 );
    CheckForOpenALErrors( "alListener3f( AL_VELOCITY" );
    //alListener3f( AL_ORIENTATION, 0, 0, -1 );
    //CheckForOpenALErrors( "alListener3f( AL_ORIENTATION" );
    alDistanceModel( AL_NONE );

    for( int i=0; i<MAX_BUFFERS; i++ )
    {
        m_Buffers[i] = 0;
    }

    // Generate the sources.
    for( int i=0; i<NUM_SOURCES; i++ )
        alGenSources( 1, &m_Sources[i].m_Sound );
    CheckForOpenALErrors( "alGenSources" );
}

SoundPlayer::~SoundPlayer()
{
    if( m_pDevice == nullptr )
        return;

    for( int i=0; i<NUM_SOURCES; i++ )
        alDeleteSources( 1, &m_Sources[i].m_Sound );
    alDeleteBuffers( MAX_BUFFERS, m_Buffers );

#if USE_ALUT
    alutExit();
#else
    alcMakeContextCurrent( 0 );
    alcDestroyContext( m_pContext );

    alcCloseDevice( m_pDevice );
#endif
}

void SoundPlayer::ActivateSoundContext()
{
    if( m_pDevice == nullptr )
        return;

#if !USE_ALUT
    alcMakeContextCurrent( m_pContext );
#endif
}

void SoundPlayer::DeactivateSoundContext()
{
    if( m_pDevice == nullptr )
        return;

#if !USE_ALUT
    alcMakeContextCurrent( 0 );
#endif
}

void SoundPlayer::OnFocusGained()
{
    if( m_pDevice == nullptr )
        return;

    for( int i=0; i<NUM_SOURCES; i++ )
    {
        int value;
        alGetSourcei( m_Sources[i].m_Sound, AL_SOURCE_STATE, &value );
        if( value == AL_PAUSED )
            PlaySound( &m_Sources[i] );
    }

    //alcMakeContextCurrent( m_pContext );
    //CheckForOpenALCErrors( m_pDevice, "alcMakeContextCurrent( m_pContext )" );

    //alcProcessContext( m_pContext );
    //CheckForOpenALCErrors( m_pDevice, "alcProcessContext( m_pContext )" );
}

void SoundPlayer::OnFocusLost()
{
    if( m_pDevice == nullptr )
        return;

    for( int i=0; i<NUM_SOURCES; i++ )
    {
        int value;
        alGetSourcei( m_Sources[i].m_Sound, AL_SOURCE_STATE, &value );
        if( value == AL_PLAYING )
            PauseSound( i );
    }

    //alcMakeContextCurrent( 0 );
    //CheckForOpenALCErrors( m_pDevice, "alcMakeContextCurrent( 0 )" );

    //alcProcessContext( m_pContext );
    //CheckForOpenALCErrors( m_pDevice, "alcProcessContext( m_pContext )" );

    //alcSuspendContext( m_pContext );
    //CheckForOpenALCErrors( m_pDevice, "alcSuspendContext( m_pContext )" );
}

SoundObject* SoundPlayer::LoadSound(const char* buffer, unsigned int buffersize)
{
    if( m_pDevice == nullptr )
        return 0;

#if USE_ALUT
    LOGInfo( LOGTag, "LoadSound %s\n", path );

    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "app/native/%s", path);

    ALuint index = alutCreateBufferFromFile( fullpath );
    if( CheckForALUTErrors( "alGenSources" ) )
        return -1;
#else
    ALuint index = 0;

    index = WaveLoader::LoadFromMemoryIntoOpenALBuffer( buffer, buffersize );
#endif

    m_Buffers[m_NextID] = index;

    // Make a single source for each sound buffer... not very general usage, but works for what I need.
    alSourcei( m_Sources[m_NextID].m_Sound, AL_BUFFER, m_Buffers[m_NextID] );
    if( CheckForOpenALErrors( "alSourcei" ) )
    {
        alDeleteBuffers( 1, &m_Buffers[m_NextID] );
        m_Buffers[m_NextID] = 0;
        return nullptr;
    }

    // Set the source to 0,0,0 for 2d again, same as listener.
    alSourcef( m_Sources[m_NextID].m_Sound, AL_PITCH, 1 );
    CheckForOpenALErrors( "alSourcef( SoundObject[m_NextID].m_Sound, AL_PITCH" );
    alSourcef( m_Sources[m_NextID].m_Sound, AL_GAIN, 1 );
    CheckForOpenALErrors( "alSourcef( SoundObject[m_NextID].m_Sound, AL_GAIN" );
    alSource3f( m_Sources[m_NextID].m_Sound, AL_POSITION, 0, 0, 0 );
    CheckForOpenALErrors( "alSourcef( SoundObject[m_NextID].m_Sound, AL_POSITION" );
    alSource3f( m_Sources[m_NextID].m_Sound, AL_VELOCITY, 0, 0, 0 );
    CheckForOpenALErrors( "alSourcef( SoundObject[m_NextID].m_Sound, AL_VELOCITY" );
    alSourcei( m_Sources[m_NextID].m_Sound, AL_LOOPING, AL_FALSE );
    CheckForOpenALErrors( "alSourcef( SoundObject[m_NextID].m_Sound, AL_LOOPING" );

    int soundid = m_NextID;

    m_NextID++;

    return &m_Sources[soundid];
}

SoundObject* SoundPlayer::LoadSound(const char* fullpath)
{
    if( m_pDevice == 0 )
        return nullptr;

    int length = 0;
    // TODO: Sounds should load through fileobjects like any other thing.
    char* PlatformSpecific_LoadFile(const char* filename, int* length, const char* file, unsigned long line);
    char* buffer = PlatformSpecific_LoadFile( fullpath, &length, fullpath, __LINE__ );

    return LoadSound( buffer, length );
}

void SoundPlayer::Shutdown()
{
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    if( m_pDevice == nullptr )
        return -1; // return value not used ATM

    alSourcePlay( pSoundObject->m_Sound ); //m_Sources[soundid] );
    CheckForOpenALErrors( "alSourcePlay" );

    return -1; // return value not used ATM
}

void SoundPlayer::StopSound(int soundid)
{
    if( m_pDevice == nullptr )
        return;

    alSourceStop( m_Sources[soundid].m_Sound );
    CheckForOpenALErrors( "alSourceStop" );
}

void SoundPlayer::PauseSound(int soundid)
{
    if( m_pDevice == nullptr )
        return;

    alSourcePause( m_Sources[soundid].m_Sound );
    CheckForOpenALErrors( "alSourcePause" );
}

void SoundPlayer::ResumeSound(int soundid)
{
    if( m_pDevice == nullptr )
        return;

    alSourcePlay( m_Sources[soundid].m_Sound );
    CheckForOpenALErrors( "alSourcePlay" );
}

void SoundPlayer::PauseAll()
{
}

void SoundPlayer::ResumeAll()
{
}
