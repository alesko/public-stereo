/*
 * 
 *
 * Written by Alexander Skoglund <alesko@member.fsf.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef __CONVERSION_H__
#define __CONVERSION_H__

/* image format conversion functions */
/* The function iyu12yuy2 takes upp half of the time in execution -> use GPU? */

static inline
void iyu12yuy2 (unsigned char *src, unsigned char *dest, uint32_t NumPixels) {
  int i=0,j=0;
  register int y0, y1, y2, y3, u, v;
  while (i < NumPixels*3/2) {
    u = src[i++];
    y0 = src[i++];
    y1 = src[i++];
    v = src[i++];
    y2 = src[i++];
    y3 = src[i++];

    dest[j++] = y0;
    dest[j++] = u;
    dest[j++] = y1;
    dest[j++] = v;

    dest[j++] = y2;
    dest[j++] = u;
    dest[j++] = y3;
    dest[j++] = v;
  }
}


static inline
void rgb2yuy2 (unsigned char *RGB, unsigned char *YUV, uint32_t NumPixels) {
  int i, j;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  for (i = 0, j = 0; i < 3 * NumPixels; i += 6, j += 4) {
    r = RGB[i + 0];
    g = RGB[i + 1];
    b = RGB[i + 2];
    RGB2YUV (r, g, b, y0, u0 , v0);
    r = RGB[i + 3];
    g = RGB[i + 4];
    b = RGB[i + 5];
    RGB2YUV (r, g, b, y1, u1 , v1);
    YUV[j + 0] = y0;
    YUV[j + 1] = (u0+u1)/2;
    YUV[j + 2] = y1;
    YUV[j + 3] = (v0+v1)/2;
  }
}

#endif
