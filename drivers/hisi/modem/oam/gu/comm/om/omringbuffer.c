/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include "omringbuffer.h"
#include "mdrv.h"
#include "vos.h"
#include "v_private.h"


#pragma pack (4)


#define    THIS_FILE_ID         PS_FILE_ID_OMRINGBUFFER_C

#define OM_ARM_ALIGNMENT                0x03

#define OM_MIN(x, y)                    (((x) < (y)) ? (x) : (y))

#define OM_RING_BUFFER_ALLOC_MAX_SIZE   (200 * 1024)

#define OM_MAX_RING_BUFFER_NUM          16

VOS_UINT8 g_ucOMBufferOccupiedFlag[OM_MAX_RING_BUFFER_NUM] = {0};

OM_RING   g_stOMControlBlock[OM_MAX_RING_BUFFER_NUM];


void OM_RingBufferCopy( const char *source, char *destination, int nbytes )
{
    char    *dstend = VOS_NULL_PTR;
    int     *src    = VOS_NULL_PTR;
    int     *dst    = VOS_NULL_PTR;
    int      tmp;

    if (nbytes == 0)
    {
        return;
    }

    tmp = destination - source;

    if ( (tmp <= 0) || (tmp >= nbytes) )
    {
        /* forward copy */
        dstend = destination + nbytes;

        /* do byte copy if less than ten or alignment mismatch */
        if (nbytes < 10 || (((VOS_UINT_PTR)destination ^ (VOS_UINT_PTR)source) & OM_ARM_ALIGNMENT))
        {
            /*lint -e801 */
            goto byte_copy_fwd;
            /*lint +e801 */
        }

        /* if odd-aligned copy byte */
        while ((VOS_UINT_PTR)destination & OM_ARM_ALIGNMENT)
        {
            *destination++ = *source++;
        }

        src = (int *) source;
        dst = (int *) destination;

        do
        {
            *dst++ = *src++;
        }while (((char *)dst + sizeof (int)) <= dstend);

        destination = (char *)dst;
        source      = (char *)src;

byte_copy_fwd:
        while (destination < dstend)
        {
            *destination++ = *source++;
        }
    }
    else
    {
        /* backward copy */
        dstend       = destination;
        destination += nbytes;
        source      += nbytes;

        /* do byte copy if less than ten or alignment mismatch */
        if (nbytes < 10 || (((VOS_UINT_PTR)destination ^ (VOS_UINT_PTR)source) & OM_ARM_ALIGNMENT))
        {
            /*lint -e801 */
            goto byte_copy_bwd;
            /*lint +e801 */
        }

        /* if odd-aligned copy byte */
        while ((VOS_UINT_PTR)destination & OM_ARM_ALIGNMENT)
        {
            *--destination = *--source;
        }

        src = (int *) source;
        dst = (int *) destination;

        do
        {
            *--dst = *--src;
        }while (((char *)dst - sizeof(int)) >= dstend);

        destination = (char *)dst;
        source      = (char *)src;

byte_copy_bwd:
        while (destination > dstend)
        {
            *--destination = *--source;
        }
    }
}



OM_RING_ID OM_RingBufferCreate( int nbytes )
{
    char                               *buffer         = VOS_NULL_PTR;
    OM_RING_ID                          ringId         = VOS_NULL_PTR;
    VOS_SPINLOCK                       *pSpinLock      = VOS_NULL_PTR;
    VOS_ULONG                           ulLockLevel;
    VOS_INT                             lTempSufffix;
    VOS_INT                             i;

    if ((nbytes > OM_RING_BUFFER_ALLOC_MAX_SIZE) || (nbytes <= 0))
    {
        return VOS_NULL_PTR;
    }

    pSpinLock = VOS_GetStaticMemSpinLock();

    if (pSpinLock == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    VOS_SpinLockIntLock(pSpinLock, ulLockLevel);

    lTempSufffix   = VOS_NULL_WORD;

    for ( i = (OM_MAX_RING_BUFFER_NUM - 1); i >= 0; i-- )
    {
        if ( g_ucOMBufferOccupiedFlag[i] == VOS_FALSE )
        {
            lTempSufffix = i;
            g_ucOMBufferOccupiedFlag[i] = VOS_TRUE;
            break;
        }
    }

    VOS_SpinUnlockIntUnlock(pSpinLock, ulLockLevel);

    if ( lTempSufffix == VOS_NULL_WORD )
    {
        return VOS_NULL_PTR;
    }

    /*
     * bump number of bytes requested because ring buffer algorithm
     * always leaves at least one empty byte in buffer
     */

    buffer = (char *) VOS_CacheMemAllocDebug((unsigned) ++nbytes, (VOS_UINT32)OM_RNG_BUFFER_ALLOC);

    if ( buffer == VOS_NULL_PTR )
    {
        VOS_SpinLockIntLock(pSpinLock, ulLockLevel);

        g_ucOMBufferOccupiedFlag[lTempSufffix] = VOS_FALSE;

        VOS_SpinUnlockIntUnlock(pSpinLock, ulLockLevel);

        return VOS_NULL_PTR;
    }

    ringId = &(g_stOMControlBlock[lTempSufffix]);

    ringId->bufSize = nbytes;
    ringId->buf     = buffer;

    OM_RingBufferFlush (ringId);

    return ringId;
}


void OM_RingBufferFlush( OM_RING_ID ringId )
{
    if (ringId ==  VOS_NULL_PTR)
    {
        return;
    }

    ringId->pToBuf   = 0;
    ringId->pFromBuf = 0;
}


/*lint -efunc(613,OM_RingBufferGet) */
int OM_RingBufferGet( OM_RING_ID rngId, char *buffer, int maxbytes )
{
    int bytesgot;
    int pToBuf;
    int bytes2;
    int pRngTmp;

    if ((rngId ==  VOS_NULL_PTR) || (buffer == VOS_NULL_PTR))
    {
        return 0;
    }

    pToBuf = rngId->pToBuf;

    if (pToBuf >= rngId->pFromBuf)
    {
        /* pToBuf has not wrapped around */
        bytesgot = OM_MIN(maxbytes, pToBuf - rngId->pFromBuf);
        OM_RingBufferCopy (&rngId->buf [rngId->pFromBuf], buffer, bytesgot);
        rngId->pFromBuf += bytesgot;
    }
    else
    {
        /* pToBuf has wrapped around.  Grab chars up to the end of the
         * buffer, then wrap around if we need to. */
        /*lint -e613*/
        bytesgot = OM_MIN(maxbytes, rngId->bufSize - rngId->pFromBuf);
        /*lint +e613*/
        OM_RingBufferCopy(&rngId->buf [rngId->pFromBuf], buffer, bytesgot);
        pRngTmp = rngId->pFromBuf + bytesgot;

        /* If pFromBuf is equal to bufSize, we've read the entire buffer,
         * and need to wrap now.  If bytesgot < maxbytes, copy some more chars
         * in now. */
        if (pRngTmp == rngId->bufSize)
        {
            /*lint -e613*/
            bytes2 = OM_MIN(maxbytes - bytesgot, pToBuf);
            /*lint +e613*/
            OM_RingBufferCopy(rngId->buf, buffer + bytesgot, bytes2);

            rngId->pFromBuf = bytes2;
            bytesgot += bytes2;
        }
        else
        {
            rngId->pFromBuf = pRngTmp;
        }
    }

    return bytesgot;
}
/*lint +efunc(613,OM_RingBufferGet) */


int OM_RingBufferPut(OM_RING_ID rngId, const char *buffer, int nbytes)
{
    int bytesput;
    int pFromBuf;
    int bytes2;
    int pRngTmp;

    if ((rngId == VOS_NULL_PTR) || (buffer == VOS_NULL_PTR))
    {
        return 0;
    }

    pFromBuf = rngId->pFromBuf;

    if (pFromBuf > rngId->pToBuf)
    {
        /* pFromBuf is ahead of pToBuf.  We can fill up to two bytes
         * before it */
        bytesput = OM_MIN(nbytes, pFromBuf - rngId->pToBuf - 1);
        OM_RingBufferCopy(buffer, &rngId->buf [rngId->pToBuf], bytesput);
        rngId->pToBuf += bytesput;
    }
    else if (pFromBuf == 0)
    {
        /* pFromBuf is at the beginning of the buffer.  We can fill till
         * the next-to-last element */
        bytesput = OM_MIN(nbytes, rngId->bufSize - rngId->pToBuf - 1);
        OM_RingBufferCopy(buffer, &rngId->buf [rngId->pToBuf], bytesput);
        rngId->pToBuf += bytesput;
    }
    else
    {
        /* pFromBuf has wrapped around, and its not 0, so we can fill
         * at least to the end of the ring buffer.  Do so, then see if
         * we need to wrap and put more at the beginning of the buffer. */
        bytesput = OM_MIN(nbytes, rngId->bufSize - rngId->pToBuf);
        OM_RingBufferCopy(buffer, &rngId->buf [rngId->pToBuf], bytesput);
        pRngTmp = rngId->pToBuf + bytesput;

        if (pRngTmp == rngId->bufSize)
        {
            /* We need to wrap, and perhaps put some more chars */
            bytes2 = OM_MIN(nbytes - bytesput, pFromBuf - 1);
            OM_RingBufferCopy(buffer + bytesput, rngId->buf, bytes2);
            rngId->pToBuf = bytes2;
            bytesput += bytes2;
        }
        else
        {
            rngId->pToBuf = pRngTmp;
        }
    }

    return bytesput;
}

VOS_BOOL OM_RingBufferIsEmpty( OM_RING_ID ringId )
{
    if (ringId ==  VOS_NULL_PTR)
    {
        return VOS_TRUE;
    }

    return (ringId->pToBuf == ringId->pFromBuf);
}


int OM_RingBufferFreeBytes( OM_RING_ID ringId)
{
    int             n;

    if (ringId == VOS_NULL_PTR)
    {
        return 0;
    }

    n = ringId->pFromBuf - ringId->pToBuf - 1;

    if (n < 0)
    {
        n += ringId->bufSize;
    }

    return n;
}


int OM_RingBufferNBytes( OM_RING_ID ringId )
{
    int                     n;

    if (ringId ==  VOS_NULL_PTR)
    {
        return 0;
    }

    n = ringId->pToBuf - ringId->pFromBuf;

    if (n < 0)
    {
        n += ringId->bufSize;
    }

    return n;
}

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif


