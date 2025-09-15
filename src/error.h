/*
 * BlueALSA - error.h
 * Copyright (c) 2016-2025 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#pragma once
#ifndef BLUEALSA_ERROR_H_
#define BLUEALSA_ERROR_H_

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdint.h>

/**
 * Type used for storing error codes.
 *
 * The first 4 bits defines the error domain, while the remaining 28 bits
 * defines the actual error code within the given domain. */
typedef uint32_t error_code_t;

/**
 * Error domain used by the error code. */
enum error_domain {
	ERROR_DOMAIN_APP = 0,
	ERROR_DOMAIN_SYSTEM,
};

#define ERROR(domain, code) (((error_code_t)(domain) << 28) | ((code) & 0x0FFFFFFF))
#define ERROR_SYSTEM(code)  ERROR(ERROR_DOMAIN_SYSTEM, (code))

/**
 * Error codes used by the application. */
enum error_code {
	ERROR_CODE_OK = 0,
	ERROR_CODE_CONTINUE,
	ERROR_CODE_NOT_FOUND,
	ERROR_CODE_INVALID_SIZE,
	ERROR_CODE_INVALID_STREAM,
	ERROR_CODE_MISSING_CAPABILITIES,
	ERROR_CODE_A2DP_INVALID_CHANNELS,
	ERROR_CODE_A2DP_NOT_SUPPORTED_CHANNELS,
	ERROR_CODE_A2DP_INVALID_CHANNEL_MODE,
	ERROR_CODE_A2DP_NOT_SUPPORTED_CHANNEL_MODE,
	ERROR_CODE_A2DP_INVALID_SAMPLE_RATE,
	ERROR_CODE_A2DP_NOT_SUPPORTED_SAMPLE_RATE,
	ERROR_CODE_A2DP_INVALID_SAMPLE_RATE_MUSIC,
	ERROR_CODE_A2DP_NOT_SUPPORTED_SAMPLE_RATE_MUSIC,
	ERROR_CODE_A2DP_INVALID_SAMPLE_RATE_VOICE,
	ERROR_CODE_A2DP_NOT_SUPPORTED_SAMPLE_RATE_VOICE,
	ERROR_CODE_A2DP_INVALID_BLOCK_LENGTH,
	ERROR_CODE_A2DP_NOT_SUPPORTED_BLOCK_LENGTH,
	ERROR_CODE_A2DP_INVALID_SUB_BANDS,
	ERROR_CODE_A2DP_NOT_SUPPORTED_SUB_BANDS,
	ERROR_CODE_A2DP_INVALID_ALLOCATION_METHOD,
	ERROR_CODE_A2DP_NOT_SUPPORTED_ALLOCATION_METHOD,
	ERROR_CODE_A2DP_INVALID_MIN_BIT_POOL_VALUE,
	ERROR_CODE_A2DP_NOT_SUPPORTED_MIN_BIT_POOL_VALUE,
	ERROR_CODE_A2DP_INVALID_MAX_BIT_POOL_VALUE,
	ERROR_CODE_A2DP_NOT_SUPPORTED_MAX_BIT_POOL_VALUE,
	ERROR_CODE_A2DP_INVALID_LAYER,
	ERROR_CODE_A2DP_NOT_SUPPORTED_LAYER,
	ERROR_CODE_A2DP_INVALID_OBJECT_TYPE,
	ERROR_CODE_A2DP_NOT_SUPPORTED_OBJECT_TYPE,
	ERROR_CODE_A2DP_INVALID_DIRECTIONS,
	ERROR_CODE_A2DP_NOT_SUPPORTED_DIRECTIONS,
	ERROR_CODE_A2DP_INVALID_FRAME_DURATION,
	ERROR_CODE_A2DP_NOT_SUPPORTED_FRAME_DURATION,
	ERROR_CODE_A2DP_INVALID_BIT_DEPTH,
	ERROR_CODE_A2DP_NOT_SUPPORTED_BIT_DEPTH,
};

const char * error_code_strerror(error_code_t err);

#if ENABLE_MP3LAME
const char * lame_encode_strerror(int err);
#endif

#if ENABLE_AAC
# include <fdk-aac/aacdecoder_lib.h>
# include <fdk-aac/aacenc_lib.h>
const char * aacdec_strerror(AAC_DECODER_ERROR err);
const char * aacenc_strerror(AACENC_ERROR err);
#endif

#if ENABLE_LC3PLUS
# include <lc3plus.h>
const char * lc3plus_strerror(LC3PLUS_Error err);
#endif

#if ENABLE_LDAC
const char * ldacBT_strerror(int err);
#endif

#if ENABLE_LHDC
const char * lhdcBT_dec_strerror(int err);
#endif

#endif
