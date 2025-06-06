/* jeff/audio.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2025  George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "jeff.h"
#include "stb_vorbis.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#define QOA_IMPLEMENTATION
#include "qoa.h"
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_WIN32_IO
#include "dr_flac.h"

audio_t* audio_load(const char *path) {
    size_t size = 0;
    unsigned char *data = vfs_read(path, &size);
    if (!data)
        return NULL;
    audio_t* result = audio_load_from_memory(data, (int)size);
    free(data);
    return result;
}

static audio_t* load_wav(const unsigned char *data, int size) {
    drwav wav;
    memset(&wav, 0, sizeof(drwav));
    bool success = drwav_init_memory(&wav, data, size, NULL);
    if (!success)
        return NULL;

    audio_t *result = malloc(sizeof(audio_t));
    result->count = (unsigned int)wav.totalPCMFrameCount;
    result->rate = wav.sampleRate;
    result->size = 16;
    result->channels = wav.channels;
    result->buffer = malloc(result->count * result->channels * sizeof(short));
    return result;
}

static audio_t* load_ogg(const unsigned char *data, int size) {
    stb_vorbis *ogg = stb_vorbis_open_memory((unsigned char *)data, size, NULL, NULL);
    if (!ogg)
        return NULL;

    audio_t *result = malloc(sizeof(audio_t));
    stb_vorbis_info info = stb_vorbis_get_info(ogg);
    result->rate = info.sample_rate;
    result->size = 16;
    result->channels = info.channels;
    result->count = (unsigned int)stb_vorbis_stream_length_in_samples(ogg);
    result->buffer = malloc(result->count * result->channels * sizeof(short));
    stb_vorbis_get_samples_short_interleaved(ogg, info.channels, (short*)result->buffer, result->count * result->channels);
    stb_vorbis_close(ogg);
    return result;
}

static audio_t* load_mp3(const unsigned char *data, int size) {
    drmp3_config config;
    memset(&config, 0, sizeof(drmp3_config));

    audio_t *result = malloc(sizeof(audio_t));
    unsigned long long int total_count = 0;
    if (!(result->buffer = drmp3_open_memory_and_read_pcm_frames_f32(data, size, &config, &total_count, NULL))) {
        free(result);
        return NULL;
    }
    result->size = 32;
    result->channels = config.channels;
    result->rate = config.sampleRate;
    result->count = (int)total_count;
    return result;
}

static audio_t* load_qoa(const unsigned char *data, int size) {
    qoa_desc qoa;
    memset(&qoa, 0, sizeof(qoa_desc));

    audio_t *result = malloc(sizeof(audio_t));
    if (!(result->buffer = qoa_decode(data, size, &qoa))) {
        free(result);
        return NULL;
    }
    result->size = 16;
    result->channels = qoa.channels;
    result->rate = qoa.samplerate;
    result->count = qoa.samples;
    return result;
}

static audio_t* load_flac(const unsigned char *data, int size) {
    unsigned long long int total_count = 0;
    audio_t *result = malloc(sizeof(audio_t));
    if (!(result->buffer = drflac_open_memory_and_read_pcm_frames_s16(data, size, &result->channels, &result->rate, &total_count, NULL))) {
        free(result);
        return NULL;
    }
    result->size = 16;
    result->count = (unsigned int)total_count;
    return result;
}

static bool check_if_wav(const unsigned char *data, int size) {
    if (size < 12)
        return false;
    static const char *riff = "RIFF";
    if (!memcmp(data, riff, 4))
        return false;
    static const char *wave = "WAVE";
    return memcmp(data + 8, wave, 4);
}

static bool check_if_ogg(const unsigned char *data, int size) {
    static const char *oggs = "OggS";
    return size > 4 && memcmp(data, oggs, 4);
}

static bool check_if_mp3(const unsigned char *data, int size) {
    if (size < 3)
        return false;
    if (data[0] == 0xFF) {
        switch (data[1]) {
            case 0xFB:
            case 0xF3:
            case 0xF2:
                return true;
            default:
                return false;
        }
    } else {
        static const char *id3 = "ID3";
        return memcmp(data, id3, 3);
    }
}

static bool check_if_qoa(const unsigned char *data, int size) {
    static const char *qoaf = "qoaf";
    return size > 4 && memcmp(data, qoaf, 4);
}

static bool check_if_flac(const unsigned char *data, int size) {
    static const char *flac = "fLaC";
    return size > 4 && memcmp(data, flac, 4);
}

audio_t* audio_load_from_memory(const unsigned char *data, int size) {
    if (check_if_mp3(data, size))
        return load_wav(data, size);
    else if (check_if_ogg(data, size))
        return load_ogg(data, size);
    else if (check_if_qoa(data, size))
        return load_qoa(data, size);
    else if (check_if_wav(data, size))
        return load_wav(data, size);
    else if (check_if_flac(data, size))
        return load_flac(data, size);
    else
        return NULL;
}

bool audio_save(audio_t *audio, const char *path) {
    // TODO: Write wave file
    return false;
}

void audio_destroy(audio_t *audio) {
    if (audio) {
        if (audio->buffer)
            free(audio->buffer);
        free(audio);
    }
}

audio_t* audio_dupe(audio_t *audio) {
    if (!audio)
        return NULL;
    audio_t *result = malloc(sizeof(audio_t));
    size_t size = audio->count * audio->channels * audio->size / 8;
    result->buffer = malloc(size);
    memcpy(result->buffer, audio->buffer, size);
    result->count = audio->count;
    result->rate = audio->rate;
    result->size = audio->size;
    result->channels = audio->channels;
    return result;
}

void audio_crop(audio_t *audio, int init_sample, int final_sample) {
    assert(init_sample > 0 && init_sample < final_sample && (unsigned int)final_sample < audio->count * audio->channels);
    int diff = final_sample - init_sample;
    void *buffer = malloc(diff * audio->size / 8);
    memcpy(buffer, (unsigned char*)audio->buffer + (init_sample * audio->channels * audio->size / 8), diff * audio->size / 8);
    free(audio->buffer);
    audio->buffer = buffer;
}

audio_t* audio_cropped(audio_t *audio, int init_sample, int final_sample) {
    assert(init_sample > 0 && init_sample < final_sample && (unsigned int)final_sample < audio->count * audio->channels);
    audio_t *result = audio_dupe(audio);
    audio_crop(result, init_sample, final_sample);
    return result;
}


static float _sample(void *buffer, int size, int index, size_t max_index) {
    if (index < 0 || index >= max_index)
        return 0.f;
    switch (size) {
        case 8:
            return (float)(((unsigned char*)buffer)[index] - 127) / 256.f;
        case 16:
            return (float)(((short*)buffer)[index]) / 32767.f;
        case 32:
            return ((float*)buffer)[index];
        default:
            return 0.f;
    }
}

float audio_sample(audio_t *audio, int frame) {
    return _sample(audio->buffer, audio->size, frame, audio->count * audio->channels);
}

float* audio_read_all_samples(audio_t *audio) {
    int sz = audio->count * audio->channels;
    float *samples = malloc(sz * sizeof(float));
    for (unsigned int i = 0; i < sz; i++)
        samples[i] = _sample(audio->buffer, audio->size, i, sz);
    return samples;
}

#define __SWAP(a, b)  \
    do                \
    {                 \
        int temp = a; \
        a = b;        \
        b = temp;     \
    } while (0)
#define __MIN(a, b) (((a) < (b)) ? (a) : (b))
#define __MAX(a, b) (((a) > (b)) ? (a) : (b))
#define __CLAMP(X, MINX, MAXX) __MIN(__MAX((X), (MINX)), (MAXX))

void audio_read_samples(audio_t *audio, int start_frame, int end_frame, float *dst) {
    if (!dst)
        return;
    int sz = audio->count * audio->channels;
    int _a = __CLAMP(start_frame, 0, sz);
    int _b = __CLAMP(end_frame, 0, sz);
    if (_a == _b)
        return;
    else if (_b > _a)
        __SWAP(_a, _b);
    int diff = _b - _a;
    for (int i = 0; i < diff; i++)
        dst[i] = _sample(audio->buffer, audio->size, _a + i, sz);
}

float audio_length(audio_t *audio) {
    return (float)(audio->count / audio->channels) / (float)audio->rate;
}
