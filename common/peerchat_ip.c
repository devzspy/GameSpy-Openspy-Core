/*

Peerchat IP encoding/decoding algorithm 0.2
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org


INTRODUCTION
============
Peerchat is the name of the encrypted IRC server of Gamespy
peerchat.gamespy.com:6667.
The game clients that use this server (like Race Driver, Haegemonia,
Ground Control II, Gamespy Arcade and many others) send their IP
(returned by the same Peerchat server after the command USRIP) in the
command USER so when someone does a /whois on an user he gets some
informations similar to the following:

   X19s4Fp1DX|123456

The text string before the '|' is just the IP address of the client and
is encoded using the simple algorithm available in this file.
FYI the number after the '|' is the unique profile ID used by Gamespy
Arcade and identify your registered account on Gamespy.
The games that don't need it set this number to 0.


LICENSE
=======
    Copyright 2004,2005,2006,2007,2008,2009 Luigi Auriemma

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

    http://www.gnu.org/licenses/gpl-2.0.txt
*/



#include "peerchat_ip.h"


char *peerchat_ip_encoder(unsigned int ip, int encdata_num) {
    static char ret[16];
    int         i;
    char        *encdata;

    if(!encdata_num) {
        encdata = peerchat_ip_encdata0;
    } else {
        encdata = peerchat_ip_encdata1;
    }

    ip ^= 0xc3801dc7;
    for(i = 7; i >= 0; i--) {
        ret[i] = encdata[ip & 0xf];
        ip >>= 4;
    }
    ret[8] = 0;
    return(ret);
}



unsigned int peerchat_ip_decoder(char *data, int encdata_num) {
    unsigned int    ip,
                    c;
    int             i;
    char            *encdata;

    if(!encdata_num) {
        encdata = peerchat_ip_encdata0;
    } else {
        encdata = peerchat_ip_encdata1;
    }

    ip = 0;
    for(i = 0; i < 8; i++) {
        c = strchr(encdata, data[i]) - encdata;
        if(c > 0xf) return(0);
        ip = (ip << 4) | c;
    }
    ip ^= 0xc3801dc7;
    return(ip);
}



char *peerchat_room_encoder(unsigned int ip, unsigned int b, unsigned int port) {
    ip = ntohl(ip);
    b = ntohl(b);           // useless on little endian but needed on big endian
    b = (((b & 0xff0000) | (b >> 16)) >> 8) | (((b << 16) | (b & 0xff00)) << 8);
    port |= (port << 16);   // this instruction doesn't exist in Rogue Trooper and possibly other games
    return(peerchat_ip_encoder(b ^ port ^ ip, 1));
}



unsigned int peerchat_room_decoder(char *data, unsigned int b, unsigned int port) {
    unsigned int    ip;

    ip = peerchat_ip_decoder(data, 1);
    port |= (port << 16);
    b = (((b & 0xff0000) | (b >> 16)) >> 8) | (((b << 16) | (b & 0xff00)) << 8);
    b = htonl(b);
    ip ^= b ^ port;
    return(htonl(ip));
}


