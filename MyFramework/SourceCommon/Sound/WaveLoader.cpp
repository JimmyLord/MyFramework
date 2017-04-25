//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "WaveLoader.h"
#if USE_OPENAL
#include "SoundPlayerOpenAL.h"
#endif

WaveLoader g_WaveLoader;

WaveLoader::WaveLoader()
{
}

WaveLoader::~WaveLoader()
{
}

#if USE_OPENAL
ALuint WaveLoader::LoadFromMemoryIntoOpenALBuffer(const char* buffer, unsigned int buffersize)
{
    const char* pBuffer = buffer;

    // expecting a riff chunk at the start.
    CHUNK_HEADER* pChunk = (CHUNK_HEADER*)pBuffer;
    if( strncmp( pChunk->chunkid, "RIFF", 4 ) == 0 )
    {
        CHUNK_RIFF* pRiffChunk = (CHUNK_RIFF*)pBuffer;
    
        if( strncmp( pRiffChunk->format, "WAVE", 4 ) )
            return 0;
    
        pBuffer += sizeof( CHUNK_RIFF );

        // next we should probably find a format chunk
        CHUNK_HEADER* pChunk = (CHUNK_HEADER*)pBuffer;
        if( strncmp( pChunk->chunkid, "fmt ", 4 ) == 0 )
        {
            CHUNK_FORMAT* pFormatChunk = (CHUNK_FORMAT*)pBuffer;

            int headersize = sizeof( CHUNK_HEADER );
            int formatsize = sizeof( CHUNK_FORMAT );
            int chunksize = pChunk->chunksize;

            // check if the format chunk is the right size.
            if( chunksize != formatsize - headersize )
                return 0;

            pBuffer += pChunk->chunksize + headersize;

            // next we should find data chunks, we'll ignore anything that isn't a data chunk.
            // TODO: a wave file without a "data" chunk will never exit... we should test against buffersize
            CHUNK_HEADER* pChunk = (CHUNK_HEADER*)pBuffer;
            while( strncmp( pChunk->chunkid, "data", 4 ) != 0 )
            {
                // skip this block by advancing pointer.
                pBuffer += pChunk->chunksize + headersize;
                pChunk = (CHUNK_HEADER*)pBuffer;
            }

            if( strncmp( pChunk->chunkid, "data", 4 ) == 0 )
            {
                CHUNK_DATA* pDataChunk = (CHUNK_DATA*)pBuffer;

                // copy the data into an OpenAL buffer.
                ALenum alformat = AL_FORMAT_MONO8;

                if( pFormatChunk->bitspersample == 8 )
                {
                    if( pFormatChunk->numchannels == 1 )
                        alformat = AL_FORMAT_MONO8;
                    else if( pFormatChunk->numchannels == 2 )
                        alformat = AL_FORMAT_STEREO8;
                    else
                        return 0;
                }
                else if( pFormatChunk->bitspersample == 16 )
                {
                    if( pFormatChunk->numchannels == 1 )
                        alformat = AL_FORMAT_MONO16;
                    else if( pFormatChunk->numchannels == 2 )
                        alformat = AL_FORMAT_STEREO16;
                    else
                        return 0;
                }
                else
                    return 0;

                ALuint albuffer;
                alGenBuffers( 1, &albuffer );
                CheckForOpenALErrors( "alGenBuffers" );

                int datasize = pDataChunk->header.chunksize;
                char* data = &pDataChunk->data;

                alBufferData( albuffer, alformat, data, datasize, pFormatChunk->samplerate );
                CheckForOpenALErrors( "alBufferData" );

                //pBuffer += pChunk->chunksize + headersize;

                return albuffer;
            }
        }
    }

    return 0;
}
#endif // USE_OPENAL

MyWaveDescriptor WaveLoader::ParseWaveBuffer(const char* buffer, unsigned int buffersize)
{
    // This method returns a copy of the wave descriptor.
    MyWaveDescriptor waveDesc;

    waveDesc.valid = false;

    const char* pBuffer = buffer;
    //const char* pDestBuffer = buffer;

    // expecting a riff chunk at the start.
    CHUNK_HEADER* pChunk = (CHUNK_HEADER*)pBuffer;
    if( strncmp( pChunk->chunkid, "RIFF", 4 ) == 0 )
    {
        CHUNK_RIFF* pRiffChunk = (CHUNK_RIFF*)pBuffer;
    
        if( strncmp( pRiffChunk->format, "WAVE", 4 ) )
            return waveDesc;
    
        pBuffer += sizeof( CHUNK_RIFF );

        // next we should probably find a format chunk
        CHUNK_HEADER* pChunk = (CHUNK_HEADER*)pBuffer;
        if( strncmp( pChunk->chunkid, "fmt ", 4 ) == 0 )
        {
            CHUNK_FORMAT* pFormatChunk = (CHUNK_FORMAT*)pBuffer;

            waveDesc.audioformat = pFormatChunk->audioformat; // using in WP8 code that want full "fmt" block
            waveDesc.numchannels = pFormatChunk->numchannels;
            waveDesc.samplerate = pFormatChunk->samplerate;
            waveDesc.byterate = pFormatChunk->byterate; // using in WP8 code that want full "fmt" block
            waveDesc.blockalign = pFormatChunk->blockalign; // using in WP8 code that want full "fmt" block
            waveDesc.bytespersample = pFormatChunk->bitspersample / 8;

            int headersize = sizeof( CHUNK_HEADER );
            int formatsize = sizeof( CHUNK_FORMAT );
            int chunksize = pChunk->chunksize;

            // check if the format chunk is the right size.
            if( chunksize <= formatsize - headersize )
                return waveDesc;

            pBuffer += pChunk->chunksize + headersize;

            // next we should find data chunks, we'll ignore anything that isn't a data chunk.
            CHUNK_HEADER* pChunk = (CHUNK_HEADER*)pBuffer;
            while( strncmp( pChunk->chunkid, "data", 4 ) != 0 )
            {
                pBuffer += pChunk->chunksize + headersize;
                pChunk = (CHUNK_HEADER*)pBuffer;
            }

            if( strncmp( pChunk->chunkid, "data", 4 ) == 0 )
            {
                CHUNK_DATA* pDataChunk = (CHUNK_DATA*)pBuffer;

                int datasize = pDataChunk->header.chunksize;
                char* data = &pDataChunk->data;

                waveDesc.data = data;
                waveDesc.datasize = datasize;

                //// to be marginally safer, don't use memcpy, since we're copying from same memory space
                //for( int i=0; i<datasize; i++ )
                //    pDestBuffer[i] = data[i];

                //pBuffer += datasize + headersize;

                //pDestBuffer += datasize;

                waveDesc.valid = true;

                return waveDesc;
            }
            else
            {
                // skip this block by advancing pointer.
                //pBuffer += pChunk->chunksize + headersize;
            }
        }
    }

    return waveDesc;
}