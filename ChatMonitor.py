#!/usr/bin/python
import sys
import socket
import string
import re
import codecs
import time
import thread
from time import sleep
import datetime

reload(sys)
sys.setdefaultencoding("utf-8")

# -*- coding: utf8 -*-

NETWORK = 'peerchat.gamespy.com'
PORT = 6667
NICK = 'ChatMonitor-gs'
ALT_NICK = 'ChatMonitor-gs'
CHAN = ['#gsp!subhome', '#gsp!gsarcadetour', '#gsp!chatmain', '#gsp!servers', '#gsp!arena', '#gsp!livewire', '#GSP!webgames',
        '#gsp!fileplanet']
IDENTD = 'XaaaaaaaaX|10008'
REALNAME = 'ChatMonitor'
CONNECTED = False
OPER_NAME = "ChatMonitor"
OPER_EMAIL = 'chatmonitor@gamespy.com'
OPER_PASSWORD = 'Password'

s = socket.socket()

'''
    Sub Functions
'''
def operArcade(name, email, password):
    try:
        s.send("OPER %s %s %s\r\n" % (name, email, password))
        s.send("MODE %s +x\r\n" % NICK)
    except:
        return


def loadStartChannels(CHANNELS_LIST):
    for channel in CHANNELS_LIST:
        try:
            s.send("JOIN %s\r\n" % channel)
            #Uncomment to set profile picture and subscriber status (can't remember if ChatMonitor had "subscriber" oh well)
            #s.send("setckey %s %s :\\b_look\\portrait,icon\r\n" % (channel, NICK))
            #s.send("setckey %s %s :\\b_reg60\\1\r\n" % (channel, NICK))
        except:
            continue

##################################################################################

'''
    Main Function
'''
def main(NETWORK, NICK, CHAN, PORT):
    flag = True
    readbuffer = ""
    global CONNECTED

    banned_words = ['nigger', 'spic', 'chink', 'faggot', 'crack', 'gameranger', 'voobly', 'xfire', 'gook', 'warez']

    s.connect((NETWORK, PORT))
    s.send("NICK %s\r\n" % NICK)
    s.send("USER %s %s bla :%s\r\n" % (IDENTD, NETWORK, REALNAME))

    while (flag):
        try:
        readbuffer = readbuffer + s.recv(4096)

        #Uncomment for debugging
        # print readbuffer

        temp = string.split(readbuffer, "\n")
        readbuffer = temp.pop()

        for line in temp:
            line = string.rstrip(line)
            line = string.split(line)
            line = [re.sub("^:", "", rep) for rep in line]
            line = [x.decode('utf-8') for x in line]

            #Uncomment for debugging
            #print line

            if ("PING" not in line and CONNECTED == True and "PRIVMSG" in line):
                user = line[0].split("!", 1)
                user = user[0]
                channel = line[2]
                msg = line[3:]
                print "[IN][%s][%s]%s" % (user, channel, ' '.join(msg))

            try:
                if (line[0] == "PING"):
                    s.send("PONG %s\r\n" % line[1])
                    print "[OUT]PONG %s" % line[1]
            
                '''
                    On connect
                '''
                if ("372" in line and "Welcome" in line and "GameSpy" in line):
                    CONNECTED = True
                    operArcade(OPER_NAME, OPER_EMAIL, OPER_PASSWORD)
                    sleep(5)
                    loadStartChannels(CHAN)

                if("PRIVMSG" in line and line[2] == NICK):
                    user = line[0].split("!", 1)
                    user = user[0]
                    s.send("PRIVMSG %s :I'm a bot. I'm here to enforce chat standards. You can read them here: http://www.gamespyarcade.com/support/chatrules.shtml" % user)
                    print "[OUT][%s]I'm a bot. I'm here to enforce chat standards. You can read them here: http://www.gamespyarcade.com/support/chatrules.shtml" % user

                if("ATM" in line and line[2] == NICK and "?IMP" in line and "INFO" in line):
                    user = line[0].split("!", 1)
                    user = user[0]
                    s.send("NOTICE %s :Client Info: build=[5228] ip=[54.191.86.65] email=[chatmonitor@gamespy.com] pid=[10008] reg=[1]\r\n" % user)
                    print "[OUT][%s]Client Info: build=[5228] ip=[54.191.86.65] email=[chatmonitor@gamespy.com] pid=[10008] reg=[1]" % user

                if any(word in [x.lower() for x in line] for word in banned_words):
                    channel = line[2]
                    for bword in banned_words:
                        if bword in [x.lower() for x in line]:
                            new_line = [x.lower() for x in line]
                            word = new_line.index(bword)
                            s.send("ATM %s ATM :CHDEL %s\r\n" % (channel, line[word]))
                            #Uncomment if you wish to "Gag" a user from talking for 24 hours. Requires extra formatting that I don't care about for now. Below is a mIRC script codeblock
                            #s.send("setusermode X \hostmask\ $+  $mid($wildsite,5) $+ \modeflags\g\expiressec\86400\comment\User used offensive word - Gag period 1 day")
            except s.timeout:
                global CONNECTED
                CONNECTED = False
                main(NETWORK, NICK, CHAN, PORT)
                continue


main(NETWORK, NICK, CHAN, PORT)
