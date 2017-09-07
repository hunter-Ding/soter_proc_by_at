/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <strings.h>
#include <hardware/keymaster1.h>

#if defined (ANDROID)
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "sprd_SOTER"
#endif

#include "eng_diag.h"
#include "eng_modules.h"

const int DEFAULT_PUB_KEY_SIZE = 512;
const int DEFAULT_DEVICE_ID_SIZE = 128;
const int DEFAULT_AT_MAX_SIZE = 64;

const char SOTER_CMD[] = "AT+SOTER"; 
const char SOTER_RSP[] = "SOTER CMD OK"; 
const char GENERATE_CMD[] = "AT+SOTER=1"; 
const char GENERATE_RSP[] = "SOTER GENERATE ATTK OK";
const char VERIFY_CMD[] = "AT+SOTER=2"; 
const char VERIFY_RSP[] = "SOTER VERIFY ATTK OK";
const char EXPORT_CMD[] = "AT+SOTER=3"; 
const char GET_ID_CMD[] = "AT+SOTER=4"; 
const char GET_ID_RSP[] = "SOTER GET DEVICE ID OK";
const char SOTER_RSP_ERROR[] = "SOTER CMD FAILED"; 
const int  SOTER_CMD_LEN = 10; // gen/verify/export cmd length

#ifdef __cplusplus
extern "C" {
#endif

static bool initialized = false;
static const hw_module_t *mod = nullptr;
static keymaster1_device_t *km1_device = nullptr;

static bool generate_attk(keymaster1_device_t *device) {
    printf("===== generate_attk =====\n");
    ALOGE("===== generate_attk =====\n");
    keymaster_error_t ret = KM_ERROR_OK;
    ret = device->generate_attk_key_pair(device, 0);
    if (KM_ERROR_OK == ret)
    {
        printf("===== generate attk OK =====\n");
    } else {
        printf("===== generate attk FAILED =====\n");
    }
    return (ret == KM_ERROR_OK) ? true : false;
}

static bool verify_attk(keymaster1_device_t *device) {
    printf("===== verify_attk =====\n");
    ALOGE("===== verify_attk =====\n");
    keymaster_error_t ret = KM_ERROR_OK;
    ret = device->verify_attk_key_pair(device);
    if (KM_ERROR_OK == ret)
    {
        printf("===== verify attk OK =====\n");
    } else {
        printf("===== verify attk FAILED =====\n");
    }
    return (ret == KM_ERROR_OK) ? true : false;
}

static int export_attk(keymaster1_device_t *device, uint8_t *pub_key) {
    printf("===== export_attk =====\n");
    ALOGE("===== export_attk =====\n");
    size_t size = 0;
    keymaster_error_t ret = KM_ERROR_OK;
    ret = device->export_attk_public_key(device, pub_key, (unsigned *)&size);
    pub_key[size] = '\0';

    if (KM_ERROR_OK == ret) {
        printf("===== verify attk OK =====\n");
        ALOGE("export_attk.size=%d \n", size);
    } else {
        printf("===== verify attk FAILED =====\n");
        ALOGE("export_attk.size=%d \n", size);
        ALOGE("export_attk=%s \n", pub_key);
        //while(size--) {
        //    ALOGE("export_attk[i]=%02x %c \n", pub_key[size], pub_key[size]);
        //}
    }

    return size;
}

static int get_device_id(keymaster1_device_t *device, uint8_t id[]) {
    ALOGE("===== get_device_id =====\n");
    size_t size = 0;
    keymaster_error_t rt = KM_ERROR_OK;

    rt = device->get_device_id(device, id, &size);
    if (KM_ERROR_OK == rt) {
        ALOGE("===== get device id OK =====\n");
        return (int) size;
    } else {
        ALOGE("===== get device id FAILED =====\n");
        ALOGE("get_device_id.size=%d \n", size);
        ALOGE("get_device_id=%s \n", id);
        return -1;
    }
}

// req contains AT cmds mixed with others, we just get AT here
int get_at_from_req(char *req, char at[]) {
    ALOGD("file:%s, func:%s\n", __FILE__, __func__);
    char *at_beg = req + 1 + sizeof(MSG_HEAD_T);
    char *at_end = strstr(at_beg, "\r\n");
    int at_len = at_end - at_beg;
    memcpy(at, at_beg, at_len);
    ALOGD("at:%s length:%d\n", at, at_len);
    return at_len;
}

int soter_for_engpc(char *req, char *rsp)
{
    char at[DEFAULT_AT_MAX_SIZE];
    int len;
    bool ret;

    ALOGD("file:%s, func:%s\n", __FILE__, __func__);
    len = get_at_from_req(req, at);
    if (len < 0) {
        return -1;
    }

    if (!initialized) {
        hw_get_module_by_class(KEYSTORE_HARDWARE_MODULE_ID, NULL, &mod);
        ALOGD("Found keymaster1 module %s, version %x\n", mod->name, mod->module_api_version);
        mod->methods->open(mod,KEYSTORE_KEYMASTER, (struct hw_device_t**)&km1_device);
        ALOGD("Trusty session initialized\n");
        initialized = true;
    }

    if (!strncasecmp(at, GENERATE_CMD, SOTER_CMD_LEN)) {
        ALOGD("Soter cmd, generate.\n");
        ret = generate_attk(km1_device);
        if (ret) {
            strcpy(rsp, GENERATE_RSP);
        } else {
            strcpy(rsp, SOTER_RSP_ERROR);
        }
    } else if (!strncasecmp(at, VERIFY_CMD, SOTER_CMD_LEN)) {
        ALOGD("Soter cmd, verify.\n");
        ret = verify_attk(km1_device);
        if (ret) {
            strcpy(rsp, VERIFY_RSP);
        } else {
            strcpy(rsp, SOTER_RSP_ERROR);
        }
    } else if (!strncasecmp(at, EXPORT_CMD, SOTER_CMD_LEN)) {
        ALOGD("Soter cmd, exprot.\n");
        uint8_t pub_key[DEFAULT_PUB_KEY_SIZE] = {0};
        int len = export_attk(km1_device, pub_key);
        ALOGD("pub_key:%s(%d)", pub_key, len);
        if (len > 0) {
            memcpy(rsp, pub_key, len);
            ALOGD("rsp:%s(%d)", rsp, strlen(rsp));
        } else {
            strcpy(rsp, SOTER_RSP_ERROR);
        }
    } else if (!strncasecmp(at, GET_ID_CMD, SOTER_CMD_LEN)) {
        ALOGD("Soter cmd, Get device id.\n");
        uint8_t id[DEFAULT_DEVICE_ID_SIZE] = {0};
        len = get_device_id(km1_device, id);
        ALOGD("get_device_id: %s(%d)", id, len);
        if (len > 0) {
            memcpy(rsp, id, len);
        } else {
            strcpy(rsp, SOTER_RSP_ERROR);
        }
    } else if (!strncasecmp(at, SOTER_CMD, SOTER_CMD_LEN - 2)) {
        ALOGD("Soter cmd, pls specify sumcmd.\n");
        strcpy(rsp, SOTER_RSP_ERROR);
    } else {
        ALOGD("Error cmd.\n");
        strcpy(rsp, SOTER_RSP_ERROR);
    }

    return 0;
}

void register_this_module(struct eng_callback * reg)
{
    ALOGD("file:%s, func:%s\n", __FILE__, __func__);
    sprintf(reg->at_cmd, "%s", "AT+SOTER");
    reg->eng_linuxcmd_func = soter_for_engpc;
    ALOGD("module cmd:%s\n", reg->at_cmd);
}

#ifdef __cplusplus
}
#endif
