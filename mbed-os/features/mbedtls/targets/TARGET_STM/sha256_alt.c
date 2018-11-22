/*
 *  sha256_alt.c for SHA256 HASH
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
#include "mbedtls/sha256.h"

#if defined(MBEDTLS_SHA256_ALT)
#include "mbedtls/platform.h"

#include "hash_mutex.h"

uint32_t inst_buffer[10] = {0};
uint16_t inst_count=0;

/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize(void *v, size_t n)
{
    volatile unsigned char *p = v;
    while (n--) {
        *p++ = 0;
    }
}

static int st_sha256_restore_hw_context(mbedtls_sha256_context *ctx)
{
    uint32_t i;
    uint32_t tickstart;
    /* allow multi-instance of HASH use: save context for HASH HW module CR */
    /* Check that there is no HASH activity on going */
    tickstart = HAL_GetTick();
    while ((HASH->SR & (HASH_FLAG_BUSY | HASH_FLAG_DMAS)) != 0) {
        if ((HAL_GetTick() - tickstart) > ST_SHA256_TIMEOUT) {
            return 0; // timeout: HASH processor is busy
        }
    }
    HASH->STR = ctx->ctx_save_str;
    HASH->CR = (ctx->ctx_save_cr | HASH_CR_INIT);
    for (i = 0; i < 38; i++) {
        HASH->CSR[i] = ctx->ctx_save_csr[i];
    }
    return 1;
}

static int st_sha256_save_hw_context(mbedtls_sha256_context *ctx)
{
    uint32_t i;
    uint32_t tickstart;
    /* Check that there is no HASH activity on going */
    tickstart = HAL_GetTick();
    while ((HASH->SR & (HASH_FLAG_BUSY | HASH_FLAG_DMAS)) != 0) {
        if ((HAL_GetTick() - tickstart) > ST_SHA256_TIMEOUT) {
            return 0; // timeout: HASH processor is busy
        }
    }
    /* allow multi-instance of HASH use: restore context for HASH HW module CR */
    ctx->ctx_save_cr = HASH->CR;
    ctx->ctx_save_str = HASH->STR;
    for (i = 0; i < 38; i++) {
        ctx->ctx_save_csr[i] = HASH->CSR[i];
    }
    return 1;
}

void mbedtls_sha256_init(mbedtls_sha256_context *ctx)
{
    mbedtls_zeroize( ctx, sizeof( mbedtls_sha256_context ) );

    /* Create mutex if not already done */
    st_start_hash_mutex();
    uint16_t i;
    for (i=0; i<10; i++) {
        if (inst_buffer[i] == (uint32_t)ctx) {
            printf("SHA256_INIT_%d (init again)\r\n", i);
	    break;
	}
    }
    if (i == 10 ) {
        inst_buffer[inst_count] = (uint32_t) ctx;
        printf("SHA256_INIT_%d addrs = 0x%08x size %d\r\n", inst_count, (uint32_t)ctx, sizeof(mbedtls_sha256_context));
        inst_count++;
    }
    /* Enable HASH clock */
    __HAL_RCC_HASH_CLK_ENABLE();
}

void mbedtls_sha256_free(mbedtls_sha256_context *ctx)
{
    if (ctx == NULL) {
        return;
    }
    uint16_t i;
    for (i=0; i<10; i++) {
        if (inst_buffer[i] == (uint32_t)ctx) {
            inst_buffer[i] = 0;
            break;
        }
    }
    printf("SHA256_FREE_%d \r\n", i);
    inst_count -=1;
    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context *dst,
                          const mbedtls_sha256_context *src)
{
    uint16_t i;
    for (i=0; i<10; i++) {
        if (inst_buffer[i] == (uint32_t)src) {
            inst_buffer[inst_count++] = (uint32_t)src;
            break;
        }
    }
    printf("SHA256_CLONE_%d \r\n", i);
    *dst = *src;
}

int mbedtls_sha256_starts_ret(mbedtls_sha256_context *ctx, int is224)
{
    if (st_acquire_hash_mutex() != osOK) {
        printf("SHA_256_ERROR1\r\n");
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }

    uint16_t i;
    for (i=0; i<10; i++) {
        if (inst_buffer[i] == (uint32_t)ctx) {
            break;
        }
    }
    printf("SHA256_START_%d \r\n", i);
    /* HASH IP initialization */
    if (HAL_HASH_DeInit(&ctx->hhash_sha256) == HAL_ERROR) {
        printf("SHA_256_ERROR2\r\n");
        st_release_hash_mutex();
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }

    ctx->is224 = is224;
    /* HASH Configuration */
    ctx->hhash_sha256.Init.DataType = HASH_DATATYPE_8B;
    /* clear CR ALGO value */
    HASH->CR &= ~HASH_CR_ALGO_Msk;
    if (HAL_HASH_Init(&ctx->hhash_sha256) == HAL_ERROR) {
        printf("SHA_256_ERROR3\r\n");
        st_release_hash_mutex();
        // error found to be returned
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    if (st_sha256_save_hw_context(ctx) != 1) {
        printf("SHA_256_ERROR4\r\n");
        st_release_hash_mutex();
        // return HASH_BUSY timeout Error here
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    st_release_hash_mutex();
    return 0;
}

int mbedtls_internal_sha256_process(mbedtls_sha256_context *ctx, const unsigned char data[ST_SHA256_BLOCK_SIZE])
{
    if (st_acquire_hash_mutex() != osOK) {
        printf("SHA_256_ERROR5\r\n");
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    uint16_t i;
    for (i=0; i<10; i++) {
        if (inst_buffer[i] == (uint32_t)ctx) {
            break;
        }
    }
    printf("SHA256_PROCESS_%d size=%d \r\n", i, (uint16_t)ST_SHA256_BLOCK_SIZE);

    if (st_sha256_restore_hw_context(ctx) != 1) {
        printf("SHA_256_ERROR6\r\n");
        st_release_hash_mutex();
        // Return HASH_BUSY timeout error here
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    if (ctx->is224 == 0) {
        if (HAL_HASHEx_SHA256_Accumulate(&ctx->hhash_sha256, (uint8_t *) data, ST_SHA256_BLOCK_SIZE) != 0) {
            printf("SHA_256_ERROR7\r\n");
            st_release_hash_mutex();
           return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
        }
    } else {
        if (HAL_HASHEx_SHA224_Accumulate(&ctx->hhash_sha256, (uint8_t *) data, ST_SHA256_BLOCK_SIZE) != 0) {
            printf("SHA_256_ERROR8\r\n");
            st_release_hash_mutex();
            return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
        }
    }

    if (st_sha256_save_hw_context(ctx) != 1) {
        // Return HASH_BUSY timeout error here
        printf("SHA_256_ERROR9\r\n");
        st_release_hash_mutex();
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    st_release_hash_mutex();
    return 0;
}

int mbedtls_sha256_update_ret(mbedtls_sha256_context *ctx, const unsigned char *input, size_t ilen)
{
    int err;
    size_t currentlen = ilen;
    if (st_acquire_hash_mutex() != osOK) {
        printf("SHA_256_ERROR10\r\n");
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    uint16_t i;
    for (i=0; i<10; i++) {
        if (inst_buffer[i] == (uint32_t)ctx) {
            break;
        }
    }
    printf("SHA256_UPDATE_%d size=%d\r\n", i, (uint16_t)ilen);
    if (st_sha256_restore_hw_context(ctx) != 1) {
        printf("SHA_256_ERROR11\r\n");
        // Return HASH_BUSY timeout error here
        st_release_hash_mutex();
       return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }

    // store mechanism to accumulate ST_SHA256_BLOCK_SIZE bytes (512 bits) in the HW
    if (currentlen == 0) { // only change HW status is size if 0
        if (ctx->hhash_sha256.Phase == HAL_HASH_PHASE_READY) {
            /* Select the SHA256 or SHA224 mode and reset the HASH processor core, so that the HASH will be ready to compute
             the message digest of a new message */
            if (ctx->is224 == 0) {
                HASH->CR |= HASH_ALGOSELECTION_SHA256 | HASH_CR_INIT;
            } else {
                HASH->CR |= HASH_ALGOSELECTION_SHA224 | HASH_CR_INIT;
            }
        }
        ctx->hhash_sha256.Phase = HAL_HASH_PHASE_PROCESS;
    } else if (currentlen < (ST_SHA256_BLOCK_SIZE - ctx->sbuf_len)) {
        // only buffurize
        memcpy(ctx->sbuf + ctx->sbuf_len, input, currentlen);
        ctx->sbuf_len += currentlen;
    } else {
        // fill buffer and process it
        memcpy(ctx->sbuf + ctx->sbuf_len, input, (ST_SHA256_BLOCK_SIZE - ctx->sbuf_len));
        currentlen -= (ST_SHA256_BLOCK_SIZE - ctx->sbuf_len);
        /* copy the content of process function to avoid releasing the mutex */
        if (ctx->is224 == 0) {
            if (HAL_HASHEx_SHA256_Accumulate(&ctx->hhash_sha256, (uint8_t *)ctx->sbuf, ST_SHA256_BLOCK_SIZE) != 0) {
                printf("SHA_256_ERROR12\r\n");
                st_release_hash_mutex();
                return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
            }
        } else {
            if (HAL_HASHEx_SHA224_Accumulate(&ctx->hhash_sha256, (uint8_t *)ctx->sbuf, ST_SHA256_BLOCK_SIZE) != 0) {
                printf("SHA_256_ERROR13\r\n");
                st_release_hash_mutex();
                return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
            }
        }
        // Process every input as long as it is %64 bytes, ie 512 bits
        size_t iter = currentlen / ST_SHA256_BLOCK_SIZE;
        if (iter != 0) {
            if (ctx->is224 == 0) {
                if (HAL_HASHEx_SHA256_Accumulate(&ctx->hhash_sha256, (uint8_t *)(input + ST_SHA256_BLOCK_SIZE - ctx->sbuf_len), (iter * ST_SHA256_BLOCK_SIZE)) != 0) {
                    printf("SHA_256_ERROR14\r\n");
                    st_release_hash_mutex();
                    return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
                }
            } else {
                if (HAL_HASHEx_SHA224_Accumulate(&ctx->hhash_sha256, (uint8_t *)(input + ST_SHA256_BLOCK_SIZE - ctx->sbuf_len), (iter * ST_SHA256_BLOCK_SIZE)) != 0) {
                    printf("SHA_256_ERROR15\r\n");
                    st_release_hash_mutex();
                    return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
                }
            }
        }
        // sbuf is completely accumulated, now copy up to 63 remaining bytes
        ctx->sbuf_len = currentlen % ST_SHA256_BLOCK_SIZE;
        if (ctx->sbuf_len != 0) {
            memcpy(ctx->sbuf, input + ilen - ctx->sbuf_len, ctx->sbuf_len);
        }
    }
    printf("SHA256_UPDATE_remain_size=%d\r\n", ctx->sbuf_len);
    if (st_sha256_save_hw_context(ctx) != 1) {
        // Return HASH_BUSY timeout error here
        printf("SHA_256_ERROR16\r\n");
        st_release_hash_mutex();
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    st_release_hash_mutex();
    return 0;
}

int mbedtls_sha256_finish_ret(mbedtls_sha256_context *ctx, unsigned char output[32])
{
    if (st_acquire_hash_mutex() != osOK) {
        printf("SHA_256_ERROR17\r\n");
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    uint16_t i;
    for (i=0; i<10; i++) {
        if (inst_buffer[i] == (uint32_t)ctx) {
            break;
        }
    }
    printf("SHA256_FINISH_%d \r\n", i);
    if (st_sha256_restore_hw_context(ctx) != 1) {
        // Return HASH_BUSY timeout error here
        printf("SHA_256_ERROR18\r\n");
        st_release_hash_mutex();
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    /* Last accumulation for extra bytes in sbuf_len */
    /* This allows the HW flags to be in place in case mbedtls_sha256_update has not been called yet */
    if (ctx->sbuf_len > 0) {
        printf("SHA256_FINISH_%d lastaccumulate%d\r\n", i, (uint16_t)ctx->sbuf_len);
        /* clear the rest of the sbuf to make sure it does not contain remaining values from previous calls */
        mbedtls_zeroize( (ctx->sbuf + ctx->sbuf_len), (ST_SHA256_BLOCK_SIZE - ctx->sbuf_len));
        if (ctx->is224 == 0) {
            if (HAL_HASHEx_SHA256_Accumulate(&ctx->hhash_sha256, ctx->sbuf, ctx->sbuf_len) != 0) {
                printf("SHA_256_ERROR19\r\n");
                st_release_hash_mutex();
                return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
            }
        } else {
            if (HAL_HASHEx_SHA224_Accumulate(&ctx->hhash_sha256, ctx->sbuf, ctx->sbuf_len) != 0) {
                printf("SHA_256_ERROR20\r\n");
                st_release_hash_mutex();
                return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
            }
        }
    }
    
    /* The following test can happen when the input is empty, and mbedtls_sha256_update has never been called */
    if(ctx->hhash_sha256.Phase == HAL_HASH_PHASE_READY) {
        if (ctx->is224 == 0) {
            /* Select the SHA256 mode and reset the HASH processor core, so that the HASH will be ready to compute
           the message digest of a new message */
            HASH->CR |= HASH_ALGOSELECTION_SHA256 | HASH_CR_INIT;
        } else {
            /* Select the SHA224 mode and reset the HASH processor core, so that the HASH will be ready to compute
           the message digest of a new message */
            HASH->CR |= HASH_ALGOSELECTION_SHA224 | HASH_CR_INIT;
        }
    }
    mbedtls_zeroize(ctx->sbuf, ST_SHA256_BLOCK_SIZE);
    ctx->sbuf_len = 0;
    __HAL_HASH_START_DIGEST();

    if (ctx->is224 == 0) {
        if (HAL_HASHEx_SHA256_Finish(&ctx->hhash_sha256, output, 10) != 0) {
            printf("SHA_256_ERROR21\r\n");
            st_release_hash_mutex();
            return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
        }
    } else {
        if (HAL_HASHEx_SHA224_Finish(&ctx->hhash_sha256, output, 10) != 0) {
            printf("SHA_256_ERROR22\r\n");
            st_release_hash_mutex();
            return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
        }
    }
    if (st_sha256_save_hw_context(ctx) != 1) {
        printf("SHA_256_ERROR23\r\n");
        // Return HASH_BUSY timeout error here
        st_release_hash_mutex();
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    st_release_hash_mutex();
    return 0;
}

#endif /*MBEDTLS_SHA256_ALT*/
