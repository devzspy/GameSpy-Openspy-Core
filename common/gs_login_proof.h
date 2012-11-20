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

#include <stdio.h>
#include <string.h>
#include <common/md5.h>

#ifndef _WIN32
#define sprintf_s snprintf
#endif

void gs_login_proof_md5(unsigned char *data, int len, unsigned char *out);
unsigned char *gs_login_proof(unsigned char *pass, unsigned char *user, unsigned char *client_chall, unsigned char *server_chall);
