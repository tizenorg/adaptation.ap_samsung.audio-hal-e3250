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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <vconf.h>
#include <iniparser.h>

#include "tizen-audio-internal.h"

#define VOLUME_INI_CSC_PATH         "/opt/system/csc-default/usr/tuning/mmfw_audio_volume.ini"
#define VOLUME_INI_DEFAULT_PATH     "/usr/etc/mmfw_audio_volume.ini"
#define VOLUME_INI_TEMP_PATH        "/opt/system/mmfw_audio_volume.ini"
#define VOLUME_VALUE_MAX            (1.0f)
#define GAIN_VALUE_MAX              (1.0f)

enum {
    STREAM_DEVICE_SPEAKER,
    STREAM_DEVICE_BLUETOOTH,
    STREAM_DEVICE_MAX,
};

static const char *g_volume_vconf[AUDIO_VOLUME_TYPE_MAX] = {
    "file/private/sound/volume/system",         /* AUDIO_VOLUME_TYPE_SYSTEM */
    "file/private/sound/volume/notification",   /* AUDIO_VOLUME_TYPE_NOTIFICATION */
    "file/private/sound/volume/alarm",          /* AUDIO_VOLUME_TYPE_ALARM */
    "file/private/sound/volume/ringtone",       /* AUDIO_VOLUME_TYPE_RINGTONE */
    "file/private/sound/volume/media",          /* AUDIO_VOLUME_TYPE_MEDIA */
    "file/private/sound/volume/call",           /* AUDIO_VOLUME_TYPE_CALL */
    "file/private/sound/volume/voip",           /* AUDIO_VOLUME_TYPE_VOIP */
    "file/private/sound/volume/svoice",         /* AUDIO_VOLUME_TYPE_SVOICE */
    "file/private/sound/volume/fixed",          /* AUDIO_VOLUME_TYPE_FIXED */
    "file/private/sound/volume/java"            /* AUDIO_VOLUME_TYPE_EXT_JAVA */
};

static inline uint8_t __get_volume_dev_index(audio_mgr_t *am, uint32_t volume_type)
{
    return 0;
}

static const uint8_t __get_stream_dev_index (uint32_t device_out)
{
    switch (device_out) {
    case AUDIO_DEVICE_OUT_SPEAKER:          return STREAM_DEVICE_SPEAKER;
    case AUDIO_DEVICE_OUT_RECEIVER:         return STREAM_DEVICE_SPEAKER;
    case AUDIO_DEVICE_OUT_WIRED_ACCESSORY:  return STREAM_DEVICE_SPEAKER;
    case AUDIO_DEVICE_OUT_BT_SCO:           return STREAM_DEVICE_BLUETOOTH;
    case AUDIO_DEVICE_OUT_BT_A2DP:          return STREAM_DEVICE_BLUETOOTH;
    case AUDIO_DEVICE_OUT_DOCK:             return STREAM_DEVICE_SPEAKER;
    case AUDIO_DEVICE_OUT_HDMI:             return STREAM_DEVICE_SPEAKER;
    case AUDIO_DEVICE_OUT_MIRRORING:        return STREAM_DEVICE_SPEAKER;
    case AUDIO_DEVICE_OUT_USB_AUDIO:        return STREAM_DEVICE_SPEAKER;
    case AUDIO_DEVICE_OUT_MULTIMEDIA_DOCK:  return STREAM_DEVICE_SPEAKER;
    default:
        AUDIO_LOG_DEBUG("invalid device_out:%d", device_out);
        break;
    }

    return STREAM_DEVICE_SPEAKER;
}

static const char *__get_device_string_by_idx (uint32_t dev_idx)
{
    switch (dev_idx) {
    case STREAM_DEVICE_SPEAKER:             return "speaker";
    case STREAM_DEVICE_BLUETOOTH:           return "btheadset";
    default:                                return "invalid";
    }
}

static const char *__get_direction_string_by_idx (uint32_t dir_idx)
{
    switch (dir_idx) {
    case AUDIO_DIRECTION_NONE:              return "none";
    case AUDIO_DIRECTION_IN:                return "in";
    case AUDIO_DIRECTION_OUT:               return "out";
    default:                                return "invalid";
    }
}

static const char *__get_volume_type_string_by_idx (uint32_t vol_type_idx)
{
    switch (vol_type_idx) {
    case AUDIO_VOLUME_TYPE_SYSTEM:          return "system";
    case AUDIO_VOLUME_TYPE_NOTIFICATION:    return "notification";
    case AUDIO_VOLUME_TYPE_ALARM:           return "alarm";
    case AUDIO_VOLUME_TYPE_RINGTONE:        return "ringtone";
    case AUDIO_VOLUME_TYPE_MEDIA:           return "media";
    case AUDIO_VOLUME_TYPE_CALL:            return "call";
    case AUDIO_VOLUME_TYPE_VOIP:            return "voip";
    case AUDIO_VOLUME_TYPE_SVOICE:          return "svoice";
    case AUDIO_VOLUME_TYPE_FIXED:           return "fixed";
    case AUDIO_VOLUME_TYPE_EXT_JAVA:        return "java";
    default:                                return "invalid";
    }
}

static const char *__get_gain_type_string_by_idx (uint32_t gain_type_idx)
{
    switch (gain_type_idx) {
    case AUDIO_GAIN_TYPE_DEFAULT:           return "default";
    case AUDIO_GAIN_TYPE_DIALER:            return "dialer";
    case AUDIO_GAIN_TYPE_TOUCH:             return "touch";
    case AUDIO_GAIN_TYPE_AF:                return "af";
    case AUDIO_GAIN_TYPE_SHUTTER1:          return "shutter1";
    case AUDIO_GAIN_TYPE_SHUTTER2:          return "shutter2";
    case AUDIO_GAIN_TYPE_CAMCODING:         return "camcording";
    case AUDIO_GAIN_TYPE_MIDI:              return "midi";
    case AUDIO_GAIN_TYPE_BOOTING:           return "booting";
    case AUDIO_GAIN_TYPE_VIDEO:             return "video";
    case AUDIO_GAIN_TYPE_TTS:               return "tts";
    default:                                return "invalid";
    }
}

static void __dump_info(char *dump, audio_info_t *info)
{
    int len;

    if (info->device.api == AUDIO_DEVICE_API_ALSA) {
        len = sprintf(dump, "device:alsa(%d.%d)", info->device.alsa.card_idx, info->device.alsa.device_idx);
    } else if (info->device.api == AUDIO_DEVICE_API_ALSA) {
        len = sprintf(dump, "device:bluez(%s,nrec:%d)", info->device.bluez.protocol, info->device.bluez.nrec);
    } else {
        len = sprintf(dump, "device:unknown");
    }

    if (len > 0)
        dump += len;

    len = sprintf(dump, "stream:%s(%dhz,%dch,vol:%s,gain:%s)",
        info->stream.name ? info->stream.name : "null", info->stream.samplerate, info->stream.channels,
        __get_volume_type_string_by_idx(info->stream.volume_type), __get_gain_type_string_by_idx(info->stream.gain_type));

    if (len > 0)
        dump += len;

    *dump = '\0';
}

#ifdef AUDIO_DEBUG
static void __dump_tb (audio_mgr_t *am)
{
    audio_volume_gain_table_t *volume_gain_table = am->stream.volume_gain_table;
    uint32_t dev_idx, vol_type_idx, vol_level_idx, gain_type_idx;
    const char *gain_type_str[] = {
        "def",          /* AUDIO_GAIN_TYPE_DEFAULT */
        "dial",         /* AUDIO_GAIN_TYPE_DIALER */
        "touch",        /* AUDIO_GAIN_TYPE_TOUCH */
        "af",           /* AUDIO_GAIN_TYPE_AF */
        "shut1",        /* AUDIO_GAIN_TYPE_SHUTTER1 */
        "shut2",        /* AUDIO_GAIN_TYPE_SHUTTER2 */
        "cam",          /* AUDIO_GAIN_TYPE_CAMCODING */
        "midi",         /* AUDIO_GAIN_TYPE_MIDI */
        "boot",         /* AUDIO_GAIN_TYPE_BOOTING */
        "video",        /* AUDIO_GAIN_TYPE_VIDEO */
        "tts",          /* AUDIO_GAIN_TYPE_TTS */
    };
    char dump_str[AUDIO_DUMP_STR_LEN], *dump_str_ptr;

    /* Dump volume table */
    AUDIO_LOG_DEBUG("<<<<< volume table >>>>>");

    for (dev_idx = 0; dev_idx < STREAM_DEVICE_MAX; dev_idx++) {
        const char *dev_str = __get_device_string_by_idx(dev_idx);

        AUDIO_LOG_DEBUG("<< %s >>", dev_str);

        for (vol_type_idx = 0; vol_type_idx < AUDIO_VOLUME_TYPE_MAX; vol_type_idx++) {
            const char *vol_type_str = __get_volume_type_string_by_idx(vol_type_idx);

            dump_str_ptr = &dump_str[0];
            memset(dump_str, 0x00, sizeof(char) * sizeof(dump_str));
            snprintf(dump_str_ptr, 8, "%6s:", vol_type_str);
            dump_str_ptr += strlen(dump_str_ptr);

            for (vol_level_idx = 0; vol_level_idx < volume_gain_table->volume_level_max[vol_type_idx]; vol_level_idx++) {
                snprintf(dump_str_ptr, 6, "%01.2f ", volume_gain_table->volume[vol_type_idx][vol_level_idx]);
                dump_str_ptr += strlen(dump_str_ptr);
            }
            AUDIO_LOG_DEBUG("%s", dump_str);
        }
        volume_gain_table++;
    }

    volume_gain_table = am->stream.volume_gain_table;

    /* Dump gain table */
    AUDIO_LOG_DEBUG("<<<<< gain table >>>>>");

    dump_str_ptr = &dump_str[0];
    memset(dump_str, 0x00, sizeof(char) * sizeof(dump_str));

    snprintf(dump_str_ptr, 11, "%10s", " ");
    dump_str_ptr += strlen(dump_str_ptr);

    for (gain_type_idx = 0; gain_type_idx < AUDIO_GAIN_TYPE_MAX; gain_type_idx++) {
        snprintf(dump_str_ptr, 7, "%5s ", gain_type_str[gain_type_idx]);
        dump_str_ptr += strlen(dump_str_ptr);
    }
    AUDIO_LOG_DEBUG("%s", dump_str);

    for (dev_idx = 0; dev_idx < STREAM_DEVICE_MAX; dev_idx++) {
        const char *dev_str = __get_device_string_by_idx(dev_idx);

        dump_str_ptr = &dump_str[0];
        memset(dump_str, 0x00, sizeof(char) * sizeof(dump_str));

        snprintf(dump_str_ptr, 11, "%9s:", dev_str);
        dump_str_ptr += strlen(dump_str_ptr);

        for (gain_type_idx = 0; gain_type_idx < AUDIO_GAIN_TYPE_MAX; gain_type_idx++) {
            snprintf(dump_str_ptr, 7, "%01.3f ", volume_gain_table->gain[gain_type_idx]);
            dump_str_ptr += strlen(dump_str_ptr);
        }
        AUDIO_LOG_DEBUG("%s", dump_str);

        volume_gain_table++;
    }
}
#endif

static audio_return_t __load_volume_gain_table_from_ini (audio_mgr_t *am)
{
    dictionary * dict = NULL;
    uint32_t dev_idx, vol_type_idx, vol_level_idx, gain_type_idx;
    audio_volume_gain_table_t *volume_gain_table = am->stream.volume_gain_table;

    dict = iniparser_load(VOLUME_INI_TEMP_PATH);
    if (!dict) {
        AUDIO_LOG_DEBUG("Use csc & default volume&gain ini file");
        dict = iniparser_load(VOLUME_INI_CSC_PATH);
        if (!dict) {
            AUDIO_LOG_DEBUG("Use default volume&gain ini file");
            dict = iniparser_load(VOLUME_INI_DEFAULT_PATH);
            if (!dict) {
                AUDIO_LOG_WARN("Loading volume&gain table from ini file failed");
                return AUDIO_ERR_UNDEFINED;
            }
       }
   }
    for (dev_idx = 0; dev_idx < STREAM_DEVICE_MAX; dev_idx++) {
        const char delimiter[] = ", ";
        char *key, *list_str, *token, *ptr = NULL;
        const char *dev_str = __get_device_string_by_idx(dev_idx);

        /* Load volume table */
        for (vol_type_idx = 0; vol_type_idx < AUDIO_VOLUME_TYPE_MAX; vol_type_idx++) {
            const char *vol_type_str = __get_volume_type_string_by_idx(vol_type_idx);

            volume_gain_table->volume_level_max[vol_type_idx] = 0;

            key = malloc(strlen(dev_str) + strlen(vol_type_str) + 2);
            if (key) {
                sprintf(key, "%s:%s", dev_str, vol_type_str);
                list_str = iniparser_getstr(dict, key);
                if (list_str) {
                    token = strtok_r(list_str, delimiter, &ptr);
                    while (token) {
                        /* convert dB volume to linear volume */
                        volume_gain_table->volume[vol_type_idx][volume_gain_table->volume_level_max[vol_type_idx]++] = pow(10.0, (atof(token) - 100) / 20.0);
                        token = strtok_r(NULL, delimiter, &ptr);
                    }
                } else {
                    audio_volume_gain_table_t *volume_gain_table_spk = &am->stream.volume_gain_table[STREAM_DEVICE_SPEAKER];

                    volume_gain_table->volume_level_max[vol_type_idx] = volume_gain_table_spk->volume_level_max[vol_type_idx];
                    memcpy((double *)&volume_gain_table->volume[vol_type_idx][0], (double *)&volume_gain_table_spk->volume[vol_type_idx][0], AUDIO_VOLUME_LEVEL_MAX * sizeof(double));
                }
                free(key);
            }
        }

        /* Load gain table */
        volume_gain_table->gain[AUDIO_GAIN_TYPE_DEFAULT] = GAIN_VALUE_MAX;
        for (gain_type_idx = AUDIO_GAIN_TYPE_DEFAULT + 1; gain_type_idx < AUDIO_GAIN_TYPE_MAX; gain_type_idx++) {
            const char *gain_type_str = __get_gain_type_string_by_idx(gain_type_idx);

            key = malloc(strlen(dev_str) + strlen("gain") + strlen(gain_type_str) + 3);
            if (key) {
                sprintf(key, "%s:gain_%s", dev_str, gain_type_str);
                token = iniparser_getstr(dict, key);
                if (token) {
                    volume_gain_table->gain[gain_type_idx] = atof(token);
                } else {
                    volume_gain_table->gain[gain_type_idx] = GAIN_VALUE_MAX;
                }
                free(key);
            } else {
                volume_gain_table->gain[gain_type_idx] = GAIN_VALUE_MAX;
            }
        }
        volume_gain_table++;
    }

    iniparser_freedict(dict);
#ifdef AUDIO_DEBUG
    __dump_tb(am);
#endif

    return AUDIO_RET_OK;
}

audio_return_t _audio_stream_init (audio_mgr_t *am)
{
    int i, value;

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    for (i = 0; i < AUDIO_VOLUME_TYPE_MAX; i++) {
        /* Get volume value string from VCONF */
        if (vconf_get_int(g_volume_vconf[i], &value) != 0) {
            AUDIO_LOG_ERROR("vconf_get_int(%s) failed", g_volume_vconf[i]);
            continue;
        }
        AUDIO_LOG_INFO("read vconf. %s = %d", g_volume_vconf[i], value);

        am->stream.volume_level[i] = value;
    }

    if (!(am->stream.volume_gain_table = malloc(STREAM_DEVICE_MAX * sizeof(audio_volume_gain_table_t)))) {
        AUDIO_LOG_ERROR("volume_gain_table malloc failed");
        return AUDIO_ERR_RESOURCE;
    }

    return __load_volume_gain_table_from_ini(am);
}

audio_return_t _audio_stream_deinit (audio_mgr_t *am)
{
    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    if (am->stream.volume_gain_table) {
        free(am->stream.volume_gain_table);
        am->stream.volume_gain_table = NULL;
    }

    return AUDIO_RET_OK;
}

audio_return_t audio_get_volume_level_max (void *userdata, uint32_t volume_type, uint32_t *level)
{
    audio_mgr_t *am = (audio_mgr_t *)userdata;
    audio_volume_gain_table_t *volume_gain_table;

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);
    AUDIO_RETURN_VAL_IF_FAIL(am->stream.volume_gain_table, AUDIO_ERR_PARAMETER);

    /* Get max volume level by device & type */
    volume_gain_table = am->stream.volume_gain_table + __get_stream_dev_index(am->device.active_out);
    *level = volume_gain_table->volume_level_max[volume_type];

    /* temporary fix for SVoice */
    if (volume_type == AUDIO_VOLUME_TYPE_VOIP) {
        *level = 7;
    }

    AUDIO_LOG_DEBUG("get_volume_level_max:%s=>%d", __get_volume_type_string_by_idx(volume_type), *level);

    return AUDIO_RET_OK;
}

audio_return_t audio_get_volume_level (void *userdata, uint32_t volume_type, uint32_t *level)
{
    audio_mgr_t *am = (audio_mgr_t *)userdata;

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    *level = am->stream.volume_level[volume_type];

    return AUDIO_RET_OK;
}

audio_return_t audio_get_volume_value (void *userdata, audio_info_t *info, uint32_t volume_type, uint32_t level, double *value)
{
    audio_mgr_t *am = (audio_mgr_t *)userdata;
    audio_volume_gain_table_t *volume_gain_table;
    char dump_str[AUDIO_DUMP_STR_LEN];

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);
    AUDIO_RETURN_VAL_IF_FAIL(am->stream.volume_gain_table, AUDIO_ERR_PARAMETER);

    /* Get basic volume by device & type & level */
    volume_gain_table = am->stream.volume_gain_table + __get_stream_dev_index(am->device.active_out);
    if (volume_gain_table->volume_level_max[volume_type] < level)
        *value = VOLUME_VALUE_MAX;
    else
        *value = volume_gain_table->volume[volume_type][level];
    if (info) {
        __dump_info(&dump_str[0], info);

        *value *= volume_gain_table->gain[info->stream.gain_type];

        AUDIO_LOG_DEBUG("get_volume_value:%d(%s)=>%f %s", level, __get_volume_type_string_by_idx(volume_type), *value, &dump_str[0]);
    } else {
        AUDIO_LOG_DEBUG("get_volume_value:%d(%s)=>%f", level, __get_volume_type_string_by_idx(volume_type), *value);
    }

    return AUDIO_RET_OK;
}

audio_return_t audio_set_volume_level (void *userdata, audio_info_t *info, uint32_t volume_type, uint32_t level)
{
    audio_return_t audio_ret = AUDIO_RET_OK;
    audio_mgr_t *am = (audio_mgr_t *)userdata;

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    if (info == NULL) {
        double volume_linear = 1.0f, volume_db = 0.0f;
        int volume_value = 0;

        /* Update volume level */
        am->stream.volume_level[volume_type] = level;

        if ((volume_type == AUDIO_VOLUME_TYPE_CALL && am->session.session == AUDIO_SESSION_VOICECALL)
            || (volume_type == AUDIO_VOLUME_TYPE_VOIP && am->session.session == AUDIO_SESSION_VOIP)
            || (volume_type == AUDIO_VOLUME_TYPE_SVOICE && am->session.session == AUDIO_SESSION_VOICE_RECOGNITION)) {
#if 0   /* NOT NEED set for alsa lib */
            audio_get_volume_value(userdata, info, volume_type, level, &volume_linear);
            volume_db = 20.0 * log10(volume_linear);

            /* 0x00~0x2E : min(-96dB) step(0dB)
               0x2F~0x43 : min(-36dB) step(1dB)
               0x44~0x57 : min(-15.5dB) step(0.5dB)
               0x58~0x6F : min(-5.75dB) step(0.25dB) */
            if (volume_db >= -5.75)
                volume_value = 0x58 + (volume_db + 5.75) / 0.25;
            else if (volume_db >= -15.5)
                volume_value = 0x44 + (volume_db + 15.5) / 0.5;
            else if (volume_db >= -36)
                volume_value = 0x2F + (volume_db + 36) / 1;

            audio_ret = _audio_mixer_control_set_value(am, MIXER_SPK_VOLUME, volume_value); */
#endif
            AUDIO_LOG_INFO("set_volume_level:%d(%s)", level, __get_volume_type_string_by_idx(volume_type));

            /* cdsp volume range 0~5 */
            audio_ret = _audio_mixer_control_set_value(am, CDSP_VOLUME, level-1);
        } else {
#ifdef AUDIO_DEBUG
            AUDIO_LOG_DEBUG("skip set_volume_level:%d(%s)", level, __get_volume_type_string_by_idx(volume_type));
#endif
        }
    }

    return audio_ret;
}

audio_return_t _audio_update_volume_level (audio_mgr_t *am)
{
    int i;

    for (i = 0; i < AUDIO_VOLUME_TYPE_MAX; i++) {
        /* Update vconf */
        if (vconf_set_int(g_volume_vconf[i], am->stream.volume_level[i]) != 0) {
            AUDIO_LOG_ERROR("vconf_set_int(%s) failed", g_volume_vconf[i]);
            continue;
        }
    }

    return AUDIO_RET_OK;
}

audio_return_t audio_get_mute (void *userdata, audio_info_t *info, uint32_t volume_type, uint32_t direction, uint32_t *mute)
{
    audio_return_t audio_ret = AUDIO_RET_OK;
    audio_mgr_t *am = (audio_mgr_t *)userdata;

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    if (info == NULL) {
        const char *ctl_name = NULL;

        if (volume_type == AUDIO_VOLUME_TYPE_CALL) {
            int mute_mic1 = 0, mute_mic2 = 0;

            audio_ret = _audio_mixer_control_get_value(am, MIXER_MIC1_SWITCH, &mute_mic1);
            if (AUDIO_IS_ERROR(audio_ret))
                return audio_ret;
            audio_ret = _audio_mixer_control_get_value(am, MIXER_MIC2_SWITCH, &mute_mic2);
            *mute = !(mute_mic1 & mute_mic2);

            AUDIO_LOG_INFO("get_mute:%s,%s=>%d(ret:0x%x)", __get_volume_type_string_by_idx(volume_type), __get_direction_string_by_idx(direction), *mute, audio_ret);
        } else {
#ifdef AUDIO_DEBUG
            AUDIO_LOG_DEBUG("skip get_mute:%s,%s=>%d(ret:0x%x)", __get_volume_type_string_by_idx(volume_type), __get_direction_string_by_idx(direction), *mute, audio_ret);
#endif
        }
    }

    return audio_ret;
}

audio_return_t audio_set_mute (void *userdata, audio_info_t *info, uint32_t volume_type, uint32_t direction, uint32_t mute)
{
    audio_return_t audio_ret = AUDIO_RET_OK;
    audio_mgr_t *am = (audio_mgr_t *)userdata;

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    if (info == NULL) {
        double volume_linear = 1.0f, volume_db = 0.0f;
        int volume_value = 0;

        if (volume_type == AUDIO_VOLUME_TYPE_CALL) {
            AUDIO_LOG_INFO("set_mute:%d(%s,%s)", mute, __get_volume_type_string_by_idx(volume_type), __get_direction_string_by_idx(direction));

            audio_ret = _audio_mixer_control_set_value(am, MIXER_MIC1_SWITCH, (int)!mute);
            if (AUDIO_IS_ERROR(audio_ret))
                return audio_ret;
            audio_ret = _audio_mixer_control_set_value(am, MIXER_MIC2_SWITCH, (int)!mute);
        }
    }

    return audio_ret;
}
