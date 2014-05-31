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
#include <string.h>
#include <stdbool.h>

#include "tizen-audio-internal.h"

audio_return_t _audio_device_init (audio_mgr_t *am)
{
    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    am->device.active_in = AUDIO_DEVICE_IN_NONE;
    am->device.active_out = AUDIO_DEVICE_OUT_NONE;

    return AUDIO_RET_OK;
}

audio_return_t _audio_device_deinit (audio_mgr_t *am)
{
    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    return AUDIO_RET_OK;
}

audio_return_t audio_set_route (void *userdata, uint32_t session, uint32_t subsession, uint32_t device_in, uint32_t device_out, uint32_t route_flag)
{
    audio_return_t audio_ret = AUDIO_RET_OK;
    audio_mgr_t *am = (audio_mgr_t *)userdata;
    bool update_volume = false;

    AUDIO_RETURN_VAL_IF_FAIL(am, AUDIO_ERR_PARAMETER);

    AUDIO_LOG_INFO("session:%d,%d in:%d out:%d flag:0x%x", session, subsession, device_in, device_out, route_flag);

    if (am->device.active_out != device_out)
        update_volume = true;

    am->device.active_in = device_in;
    am->device.active_out = device_out;

    /* Volume level should be updated if device changed */
    if (update_volume)
        audio_ret = _audio_update_volume_level(am);

    return audio_ret;
}
