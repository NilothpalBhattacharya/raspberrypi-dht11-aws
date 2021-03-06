/* config.h wrapper that forces calloc(0) to return NULL.
 * Used for testing.
 */
/*
 *  Copyright (C) 2019, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: GPL-2.0
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#ifndef MBEDTLS_CONFIG_H
/* Don't #define MBEDTLS_CONFIG_H, let config.h do it. */

#include "mbedtls/config.h"

#include <stdlib.h>
static inline void *custom_calloc( size_t nmemb, size_t size )
{
    if( nmemb == 0 || size == 0 )
        return( NULL );
    return( calloc( nmemb, size ) );
}

#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_STD_CALLOC custom_calloc

#endif /* MBEDTLS_CONFIG_H */
