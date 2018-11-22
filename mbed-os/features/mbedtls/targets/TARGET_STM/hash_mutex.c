/*
 *  hash_mutex.c handle mutex for HASH hw ip
 *******************************************************************************
 * Copyright (c) 2017, STMicroelectronics
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
#include "hash_mutex.h"

#if (defined(MBEDTLS_SHA256_ALT) || defined(MBEDTLS_SHA1_ALT) || defined(MBEDTLS_MD5_ALT))

osMutexId_t st_hash_mutex = 0;
osRtxMutex_t st_hash_mutex_cb;
osMutexAttr_t st_hash_mutex_attr = {
  "st_hash_Mutex",
  0,
  &st_hash_mutex_cb,
  sizeof(st_hash_mutex_cb)
};

osStatus_t st_start_hash_mutex()
{
    /* Create mutex if not already done */
    if (st_hash_mutex == NULL) {
        st_hash_mutex = osMutexNew(&st_hash_mutex_attr);
        if (st_hash_mutex == NULL) {
            return  osError;
        }
    }
    return osOK;
}

osStatus_t st_acquire_hash_mutex()
{
    return osMutexAcquire(st_hash_mutex, MUTEX_WAIT_MS);
}

osStatus_t st_release_hash_mutex()
{
    return osMutexRelease(st_hash_mutex);
}

#endif /*defined(MBEDTLS_SHA256_ALT) || defined(MBEDTLS_SHA1_ALT) || defined(MBEDTLS_MD5_ALT)*/
