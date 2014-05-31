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
#include <pthread.h>

#include "tizen-audio-internal.h"

audio_return_t _audio_util_init (audio_mgr_t *am)
{
    char *card_name = 0;
    int ret = 0;

    pthread_mutex_init(&am->mixer.mutex, NULL);
    snd_card_get_name(0, &card_name);
    if (!card_name)
        card_name = "default";
    ret = snd_ctl_open(&am->mixer.mixer, card_name, 0);
    if (ret < 0 || !am->mixer.mixer) {
        AUDIO_LOG_ERROR("mixer_open failed");
        return AUDIO_ERR_RESOURCE;
    }

    return AUDIO_RET_OK;
}

audio_return_t _audio_util_deinit (audio_mgr_t *am)
{
    pthread_mutex_destroy(&am->mixer.mutex);
    snd_ctl_close(am->mixer.mixer);
    am->mixer.mixer = NULL;

    return AUDIO_RET_OK;
}

audio_return_t _audio_mixer_control_get_value(audio_mgr_t *am, const char *ctl_name, int *val)
{
    audio_return_t ret = AUDIO_RET_USE_HW_CONTROL;
    snd_ctl_elem_value_t *control;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_type_t type;
    int count = 0, i = 0;

    pthread_mutex_lock(&am->mixer.mutex);

    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_value_alloca(&control);

    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, ctl_name);

    snd_ctl_elem_info_set_id(info, id);
    if (snd_ctl_elem_info(am->mixer.mixer, info) < 0) {
        AUDIO_LOG_ERROR("snd_ctl_elem_info %s failed", ctl_name);
        ret = AUDIO_ERR_IOCTL;
        goto exit;
    }
    snd_ctl_elem_info_get_id(info, id);

    type = snd_ctl_elem_info_get_type(info);
    count = snd_ctl_elem_info_get_count(info);

    snd_ctl_elem_value_set_id(control, id);

    if(snd_ctl_elem_read(am->mixer.mixer, control) < 0) {
        AUDIO_LOG_ERROR("snd_ctl_elem_read failed \n");
        ret = AUDIO_ERR_IOCTL;
        goto exit;
    }

    switch (type) {
    case SND_CTL_ELEM_TYPE_BOOLEAN:
        *val = snd_ctl_elem_value_get_boolean(control, i);
        break;
    case SND_CTL_ELEM_TYPE_INTEGER:
        for (i = 0; i < count; i++)
        *val = snd_ctl_elem_value_get_integer(control, i);
        break;
    default:
        AUDIO_LOG_WARN("unsupported control element type\n");
        ret = AUDIO_ERR_IOCTL;
        goto exit;
    }

    AUDIO_LOG_DEBUG("mixer_ctl_get %s=%d success", ctl_name, *val);

exit:
    pthread_mutex_unlock(&am->mixer.mutex);

    return ret;
}

audio_return_t _audio_mixer_control_set_value(audio_mgr_t *am, const char *ctl_name, int val)
{
    audio_return_t ret = AUDIO_RET_USE_HW_CONTROL;
    int mixer_ret = -1;
    snd_ctl_elem_value_t *control;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_type_t type;
    int count = 0, i = 0;

    pthread_mutex_lock(&am->mixer.mutex);

    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_value_alloca(&control);

    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, ctl_name);

    snd_ctl_elem_info_set_id(info, id);
    if (snd_ctl_elem_info(am->mixer.mixer, info) < 0) {
        AUDIO_LOG_ERROR("snd_ctl_elem_info %s failed", ctl_name);
        ret = AUDIO_ERR_IOCTL;
        goto exit;
    }
    snd_ctl_elem_info_get_id(info, id);

    type = snd_ctl_elem_info_get_type(info);
    count = snd_ctl_elem_info_get_count(info);

    snd_ctl_elem_value_set_id(control, id);

    snd_ctl_elem_read(am->mixer.mixer, control);

    switch (type) {
    case SND_CTL_ELEM_TYPE_BOOLEAN:
        for (i = 0; i < count; i++)
            snd_ctl_elem_value_set_boolean(control, i, val);
        break;
    case SND_CTL_ELEM_TYPE_INTEGER:
        for (i = 0; i < count; i++)
            snd_ctl_elem_value_set_integer(control, i,val);
        break;
    default:
        AUDIO_LOG_WARN("unsupported control element type\n");
        ret = AUDIO_ERR_IOCTL;
        goto exit;
    }

    if (snd_ctl_elem_write(am->mixer.mixer, control) < 0) {
        AUDIO_LOG_ERROR("snd_ctl_elem_write failed \n");
        ret = AUDIO_ERR_IOCTL;
        goto exit;
    }

    AUDIO_LOG_DEBUG("set_mixer %s=%d success", ctl_name, val);

exit:
    pthread_mutex_unlock(&am->mixer.mutex);

    return ret;
}
