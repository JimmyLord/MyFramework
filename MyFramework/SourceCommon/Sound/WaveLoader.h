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

#ifndef __WaveLoader_H__
#define __WaveLoader_H__

// based off https://ccrma.stanford.edu/courses/422/projects/WaveFormat/

class WaveLoader;
extern WaveLoader g_WaveLoader;

struct CHUNK_HEADER
{
    char chunkid[4];
    int chunksize;
};

struct CHUNK_RIFF
{
    CHUNK_HEADER header; // chunkid should be "RIFF"
    char format[4]; // should be "WAVE"
};

struct CHUNK_FORMAT
{
    CHUNK_HEADER header; // chunkid should be "fmt "
    short audioformat;
    short numchannels;
    int samplerate;
    int byterate;
    short blockalign;
    short bitspersample;
};

struct CHUNK_DATA
{
    CHUNK_HEADER header; // chunkid should be "data"
    char data; // actual data, this is just the first byte...
};

struct MyWaveDescriptor
{
    bool valid;

    short audioformat; // using in WP8 code that want full "fmt" block
    int numchannels;
    int samplerate;
    int byterate; // using in WP8 code that want full "fmt" block
    short blockalign; // using in WP8 code that want full "fmt" block
    int bytespersample; // NOTE: not bits per sample.

    char* data;
    int datasize;
};

class WaveLoader
{
protected:

public:
    WaveLoader(void);
    virtual ~WaveLoader(void);

#if USE_OPENAL
    static ALuint LoadFromMemoryIntoOpenALBuffer(const char* buffer, unsigned int buffersize);
#endif
    static MyWaveDescriptor ParseWaveBuffer(const char* buffer, unsigned int buffersize);
};

#endif //__WaveLoader_H__

