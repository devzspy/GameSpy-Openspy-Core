/*

    Copyright 2005,2006 Luigi Auriemma

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl.txt

*/

#include <common/gs_login_proof.h>

void gs_login_proof_md5(unsigned char *data, int len, unsigned char *out) {
    md5_context         md5t;
    const static char   hex[] = "0123456789abcdef";
    unsigned char       md5h[16];
    int                 i;

    md5_starts(&md5t);
    md5_update(&md5t, data, len);
    md5_finish(&md5t, md5h);

    for(i = 0; i < 16; i++) {
        *out++ = hex[md5h[i] >> 4];
        *out++ = hex[md5h[i] & 15];
    }
    *out = 0;
}



unsigned char *gs_login_proof(unsigned char *pass, unsigned char *user, unsigned char *client_chall, unsigned char *server_chall) {
    int             len;
    unsigned char   buff[512],
                    pwdmd5[33];
    static unsigned char    proof[33];

    gs_login_proof_md5(pass, strlen((const char *)pass), pwdmd5);
    len = sprintf_s(
        (char *)&buff,
        sizeof(buff) - 1,
        "%s"    /* MD5 password */
        "%s"    /* 48 spaces */
        "%s"    /* user@mail */
        "%s"    /* server challenge */
        "%s"    /* client challenge */
        "%s",   /* MD5 password */
        pwdmd5,
        "                                                ",
        user,
        server_chall,
        client_chall,
        (char *)&pwdmd5);

    if((len < 0) || (len >= sizeof(buff))) return((unsigned char *)&"");
    gs_login_proof_md5(buff, len, proof);

    return(proof);
}


