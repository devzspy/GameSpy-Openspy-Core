/*

gs_sesskey \authp\ resp 0.1
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org


INTRO
=====
The following code is referred to data exchanged with the Gamespy master
server on port 29920 (remember that the data from/to this port is EVER
XORed with the word "GameSpy3D").
My function is needed to generate the text string that must be placed
after the password and on which is then calculated the MD5 hash, used
in the \authp\ data block.
(thanx to Alex for having noticed that the md5 must be calculated on the
password and not on the nickname)


EXAMPLE
=======
SERVER:  \lc\2\sesskey\123456789\proof\0\id\1\final\
CLIENT:  \authp\\pid\87654321\resp\7fcb80a6255c183dc149fb80abcd4675\lid\0\final\

  resp is the MD5 hash of "passwordDxtLwy}K"
  password is your Gamespy password
  DxtLwy}K is the result of gs_sesskey(123456789);


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
#include <stdio.h>

char *gs_sesskey(int sesskey);
