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

#include "CommonHeader.h"
#include "MediaPlayer.h"

#include <sys/stat.h>

MediaPlayer::MediaPlayer()
{
    m_Initialized = false;
    m_Playing = false;

    m_pMediaPlayerConnectionHandle = 0;
    m_pMediaContext = 0;
    m_AudioOutputID = -1;
}

MediaPlayer::~MediaPlayer()
{
    ShutdownMediaPlayer();
}

void MediaPlayer::StartUpMediaPlayer()
{
    if( m_Initialized )
        return;

    // todo: pass this in to separate play func and cut this function in half.
    const char* audiofiletoplay = "app/native/Data/Audio/WordsInBedRemixMusic.mp3";

    char workingdir[PATH_MAX];
    char audiofileurl[PATH_MAX];
    int ret;

    getcwd( workingdir, PATH_MAX );
    sprintf( audiofileurl, "file://%s/%s", workingdir, audiofiletoplay );

    assert( m_pMediaPlayerConnectionHandle == 0 );
    m_pMediaPlayerConnectionHandle = mmr_connect( 0 );
    if( m_pMediaPlayerConnectionHandle == 0 )
    {
        LOGError( LOGTag, "mmr_connect" );
        return;
    }

    mode_t mode = S_IRUSR | S_IXUSR;
    m_pMediaContext = mmr_context_create( m_pMediaPlayerConnectionHandle, "mygamemusicplayer", 0, mode );
    if( m_pMediaContext == 0 )
    {
        LOGError( LOGTag, "mmr_context_create" );
        return;
    }

    const char* audiourl = "audio:default";
    m_AudioOutputID = mmr_output_attach( m_pMediaContext, audiourl, "audio" );
    if( m_AudioOutputID < 0 )
    {
        LOGError( LOGTag, "mmr_output_attach" );
        return;
    }

    // put it in a list, so we can specify repeat below
    ret = mmr_input_attach( m_pMediaContext, audiofileurl, "autolist" );
    //ret = mmr_input_attach( m_pMediaContext, audiofileurl, "track" );
    if( ret < 0 )
    {
        const mmr_error_info_t* errorinfo = mmr_error_info( m_pMediaContext );

        LOGError( LOGTag, "mmr_input_attach" );
        return;
    }

    strm_dict_t* inputparams = strm_dict_new(); // repeat:all only works with playlists.
    if( inputparams )
    {
        inputparams = strm_dict_set( inputparams, "repeat", "all" );
        ret = mmr_input_parameters( m_pMediaContext, inputparams );
        if( ret < 0 )
        {
            LOGError( LOGTag, "mmr_input_parameters" );
            // don't care too much if this fails... should just mean audio won't loop.
            //return;
        }
    }

    ret = mmr_play( m_pMediaContext );
    if( ret < 0 )
    {
        const mmr_error_info_t* errorinfo = mmr_error_info( m_pMediaContext );

        LOGError( LOGTag, "mmr_play" );
        return;
    }

    m_Initialized = true;
    m_Playing = true;

    SetVolume( 0 );
}

void MediaPlayer::ShutdownMediaPlayer()
{
    if( m_Initialized == false )
        return;

    Stop();

    if( m_pMediaContext )
    {
        mmr_input_detach( m_pMediaContext );
        mmr_output_detach( m_pMediaContext, m_AudioOutputID );
        m_AudioOutputID = -1;
        mmr_context_destroy( m_pMediaContext );
        m_pMediaContext = 0;
    }

    if( m_pMediaPlayerConnectionHandle )
    {
        mmr_disconnect( m_pMediaPlayerConnectionHandle );
        m_pMediaPlayerConnectionHandle = 0;
    }
}

void MediaPlayer::Play()
{
    if( m_Initialized == false || m_Playing == true )
        return;

    int ret = mmr_seek( m_pMediaContext, "1:0" ); // todo: seek to correct spot.
    if( ret < 0 )
    {
        const mmr_error_info_t* errorinfo = mmr_error_info( m_pMediaContext );

        LOGError( LOGTag, "mmr_seek" );
    }

    ret = mmr_play( m_pMediaContext );
    if( ret < 0 )
    {
        const mmr_error_info_t* errorinfo = mmr_error_info( m_pMediaContext );

        LOGError( LOGTag, "mmr_play" );
    }
    else
    {
        m_Playing = true;
    }
}

void MediaPlayer::Stop()
{
    if( m_Initialized == false || m_Playing == false )
        return;

    int ret = mmr_stop( m_pMediaContext );
    if( ret < 0 )
    {
        const mmr_error_info_t* errorinfo = mmr_error_info( m_pMediaContext );

        LOGError( LOGTag, "mmr_seek" );
    }

    m_Playing = false;
}

void MediaPlayer::Pause()
{
    if( m_Initialized == false || m_Playing == false )
        return;

    // TODO: store the current position and feed it back into "Play" function above.
    int ret = mmr_stop( m_pMediaContext );
    if( ret < 0 )
    {
        const mmr_error_info_t* errorinfo = mmr_error_info( m_pMediaContext );

        LOGError( LOGTag, "mmr_seek" );
    }

    m_Playing = false;
}

void MediaPlayer::SetVolume(float volume)
{
    if( m_Initialized == false )
        return;

    char volumestr[10];
    sprintf_s( volumestr, 10, "%.0f", volume*100 );
    
    strm_dict_t* param = strm_dict_new();
    param = strm_dict_set( param, "volume", volumestr );

    if( param )
        mmr_output_parameters( m_pMediaContext, m_AudioOutputID, param );
}
