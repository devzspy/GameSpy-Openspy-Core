/*

gs_chresp_num \auth\ response 0.1
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org


INTRO
=====
The following code is referred to data exchanged with the Gamespy master
server on port 29920 (remember that the data from/to this port is EVER
XORed with the word "GameSpy3D").
My function is needed to generate the number that must be placed before
the gamekey of the specific game you are using and on which is then
calculated the MD5 hash, used in the \auth\ data block.


EXAMPLE
=======
SERVER:  \lc\1\challenge\ABCDEFGHIL\id\1\final\
CLIENT:  \auth\\gamename\gslive\response\e451ab04ce1c742184407cbc99ef0fee\port\0\id\1\final\

  response is the MD5 hash of "884373937Xn221z"
  884373937 is the signed number returned by gs_chresp_num("ABCDEFGHIL");
  Xn221z is the gamekey of GamespyArcade identified by gslive so if you
         use the gamename ccgenzh the gamekey to use is D6s9k3
         Gslist and gshkeys.txt are the references for the gamekeys


LICENSE
=======
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

int gs_chresp_num(char *challenge);

