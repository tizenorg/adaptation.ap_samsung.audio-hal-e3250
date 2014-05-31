#ifndef footizenaudiointernalfoo
#define footizenaudiointernalfoo

/*
 * audio-hal
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyunseok Lee <hs7388.lee@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dlog.h>
#include <alsa/asoundlib.h>

#include "tizen-audio.h"

/* Debug */

#define AUDIO_DEBUG
#define AUDIO_DUMP_STR_LEN              256
#ifdef USE_DLOG
#ifdef DLOG_TAG
#undef DLOG_TAG
#endif
#define DLOG_TAG "AUDIO_HAL"
#define AUDIO_LOG_ERROR(...)            SLOG(LOG_ERROR, DLOG_TAG, __VA_ARGS__)
#define AUDIO_LOG_WARN(...)             SLOG(LOG_WARN, DLOG_TAG, __VA_ARGS__)
#define AUDIO_LOG_INFO(...)             SLOG(LOG_INFO, DLOG_TAG, __VA_ARGS__)
#define AUDIO_LOG_DEBUG(...)            SLOG(LOG_DEBUG, DLOG_TAG, __VA_ARGS__)
#define AUDIO_LOG_VERBOSE(...)          SLOG(LOG_DEBUG, DLOG_TAG, __VA_ARGS__)
#else
#define AUDIO_LOG_ERROR(...)            fprintf(stderr, __VA_ARGS__)
#define AUDIO_LOG_WARN(...)             fprintf(stderr, __VA_ARGS__)
#define AUDIO_LOG_INFO(...)             fprintf(stdout, __VA_ARGS__)
#define AUDIO_LOG_DEBUG(...)            fprintf(stdout, __VA_ARGS__)
#define AUDIO_LOG_VERBOSE(...)          fprintf(stdout, __VA_ARGS__)
#endif

#define AUDIO_RETURN_IF_FAIL(expr, val) do { \
    if (!expr) { \
        AUDIO_LOG_ERROR("%s failed", #expr); \
        return; \
    } \
} while (0)
#define AUDIO_RETURN_VAL_IF_FAIL(expr, val) do { \
    if (!expr) { \
        AUDIO_LOG_ERROR("%s failed", #expr); \
        return val; \
    } \
} while (0)

/* Session */
typedef struct audio_session_mgr {
    audio_session_t session;
    audio_subsession_t subsession;
} audio_session_mgr_t;


/* Device */

typedef struct audio_device_mgr {
    audio_device_in_t active_in;
    audio_device_out_t active_out;
} audio_device_mgr_t;


/* Stream */

#define AUDIO_VOLUME_LEVEL_MAX 16

typedef struct audio_volume_gain_table {
    double volume[AUDIO_VOLUME_TYPE_MAX][AUDIO_VOLUME_LEVEL_MAX];
    uint32_t volume_level_max[AUDIO_VOLUME_LEVEL_MAX];
    double gain[AUDIO_GAIN_TYPE_MAX];
} audio_volume_gain_table_t;

enum {
    AUDIO_VOLUME_DEVICE_SPEAKER,
    AUDIO_VOLUME_DEVICE_RECEIVER,
    AUDIO_VOLUME_DEVICE_EARJACK,
    AUDIO_VOLUME_DEVICE_BT_SCO,
    AUDIO_VOLUME_DEVICE_BT_A2DP,
    AUDIO_VOLUME_DEVICE_DOCK,
    AUDIO_VOLUME_DEVICE_HDMI,
    AUDIO_VOLUME_DEVICE_MIRRORING,
    AUDIO_VOLUME_DEVICE_USB,
    AUDIO_VOLUME_DEVICE_MULTIMEDIA_DOCK,
    AUDIO_VOLUME_DEVICE_MAX,
};

typedef struct audio_stream_mgr {
    uint32_t volume_level[AUDIO_VOLUME_TYPE_MAX];
    audio_volume_gain_table_t *volume_gain_table;
} audio_stream_mgr_t;

typedef struct audio_mixer_mgr {
    snd_ctl_t* mixer;
    pthread_mutex_t mutex;
} audio_mixer_mgr_t;

/* Overall */

typedef struct audio_mgr {
    audio_device_mgr_t device;
    audio_stream_mgr_t stream;
    audio_session_mgr_t session;
    audio_mixer_mgr_t mixer;
} audio_mgr_t;

audio_return_t _audio_stream_init (audio_mgr_t *am);
audio_return_t _audio_stream_deinit (audio_mgr_t *am);
audio_return_t _audio_update_volume_level (audio_mgr_t *am);

audio_return_t _audio_device_init (audio_mgr_t *am);
audio_return_t _audio_device_deinit (audio_mgr_t * am);

audio_return_t _audio_session_init (audio_mgr_t *am);
audio_return_t _audio_session_deinit (audio_mgr_t *am);

#define MIXER_SPK_VOLUME            "Speaker Volume"
#define CDSP_VOLUME                 "CDSP Volume"
#define MIXER_MIC1_SWITCH           "Mic1 Switch"
#define MIXER_MIC2_SWITCH           "Mic2 Switch"

audio_return_t _audio_util_init (audio_mgr_t *am);
audio_return_t _audio_util_deinit (audio_mgr_t *am);
audio_return_t _audio_mixer_control_set_value(audio_mgr_t *am, const char *ctl_name, int val);
audio_return_t _audio_mixer_control_get_value(audio_mgr_t *am, const char *ctl_name, int *val);

#endif
