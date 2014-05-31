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

#include "tizen-audio-internal.h"

int audio_get_revision (void)
{
    return AUDIO_REVISION;
}

audio_return_t audio_init (void **userdata)
{
    audio_mgr_t *am;
    audio_return_t ret = AUDIO_RET_OK;

    if (!(am = malloc(sizeof(audio_mgr_t)))) {
        AUDIO_LOG_ERROR("am malloc failed");
        return AUDIO_ERR_RESOURCE;
    }
    if (AUDIO_IS_ERROR((ret = _audio_session_init(am)))) {
        AUDIO_LOG_ERROR("session init failed");
        goto error_exit;
    }
    if (AUDIO_IS_ERROR((ret = _audio_device_init(am)))) {
        AUDIO_LOG_ERROR("device init failed");
        goto error_exit;
    }
    if (AUDIO_IS_ERROR((ret = _audio_stream_init(am)))) {
        AUDIO_LOG_ERROR("stream init failed");
        goto error_exit;
    }
    if (AUDIO_IS_ERROR((ret = _audio_util_init(am)))) {
        AUDIO_LOG_ERROR("mixer init failed");
        goto error_exit;
    }

    *userdata = (void *)am;
    return AUDIO_RET_OK;

error_exit:
    if (am)
        free(am);

    return ret;
}

audio_return_t audio_deinit (void **userdata)
{
    audio_mgr_t *am = (audio_mgr_t *)*userdata;
 
    if (am) {
        _audio_session_deinit(am);
        _audio_device_deinit(am);
        _audio_stream_deinit(am);
        _audio_util_deinit(am);
        free(am);
        *userdata = NULL;
    }

    return AUDIO_RET_OK;
}

audio_return_t audio_reset (void **userdata)
{
    audio_mgr_t *am = (audio_mgr_t *)*userdata;
    audio_return_t ret = AUDIO_RET_OK;

    if (am) {
        _audio_device_deinit(am);
        _audio_stream_deinit(am);
        _audio_util_deinit(am);

        if (AUDIO_IS_ERROR((ret = _audio_session_init(am)))) {
            AUDIO_LOG_ERROR("session init failed");
            goto error_exit;
        }
        if (AUDIO_IS_ERROR((ret = _audio_device_init(am)))) {
            AUDIO_LOG_ERROR("device init failed");
            goto error_exit;
        }
        if (AUDIO_IS_ERROR((ret = _audio_stream_init(am)))) {
            AUDIO_LOG_ERROR("stream init failed");
            goto error_exit;
        }
        if (AUDIO_IS_ERROR((ret = _audio_util_init(am)))) {
            AUDIO_LOG_ERROR("mixer init failed");
            goto error_exit;
        }
    }

    return AUDIO_RET_OK;

error_exit:
    if (am)
        free(am);
    *userdata = NULL;

    return ret;
}
