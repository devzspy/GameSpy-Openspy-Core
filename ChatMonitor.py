#!/usr/bin
import sys
import socket
import string
import re
import codecs
import time
import thread
import random
import pickle
from time import sleep
import datetime

reload(sys)
sys.setdefaultencoding("utf-8")

# -*- coding: utf8 -*-

NETWORK = 'peerchat.gamespy.com'
PORT = 6667
NICK = 'ChatMonitor-gs'
ALT_NICK = 'ChatMonitor-gs'
CHAN = ['#gsp!subhome']
IDENTD = 'chatmonitor'
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


def loadStartChannels(CHANNELS_LIST, CURRENTCHANNELS):
    for channel in CHANNELS_LIST:
        try:
            s.send("JOIN %s\r\n" % channel)
            CURRENTCHANNELS += 1
        except:
            continue

    return CURRENTCHANNELS


def joinChannel(admin, channel, CURRENTCHANNELS, MAXCHANNELS):
    if (CURRENTCHANNELS <= MAXCHANNELS):
        try:
            s.send("JOIN %s\r\n" % channel)
            s.send("PRIVMSG %s :I joined %s\r\n" % (admin, channel))
            CURRENTCHANNELS += 1
        except:
            return

    return CURRENTCHANNELS


def partChannel(admin, channel, CURRENTCHANNELS):
    if (CURRENTCHANNELS >= 0):
        try:
            s.send("PART %s\r\n" % channel)
            s.send("PRIVMSG %s :I left %s\r\n" % (admin, channel))
            CURRENTCHANNELS -= 1
        except:
            return

    return CURRENTCHANNELS

##################################################################################

'''
    Main Function
'''
def main(NETWORK, NICK, CHAN, PORT):
    flag = True
    readbuffer = ""
    MAXCHANNELS = 20
    CURRENTCHANNELS = 0
    global CONNECTED
    global DISABLE_COMMANDS
    global USER_LIST

    s.connect((NETWORK, PORT))
    s.send("NICK %s\r\n" % NICK)
    s.send("USER %s %s bla :%s\r\n" % (IDENTD, NETWORK, REALNAME))

    while (flag):
        readbuffer = readbuffer + s.recv(4096)

        # print readbuffer

        temp = string.split(readbuffer, "\n")
        readbuffer = temp.pop()

        for line in temp:
            line = string.rstrip(line)
            line = string.split(line)
            line = [re.sub("^:", "", rep) for rep in line]
            line = [x.decode('utf-8') for x in line]

            print line

            if ("PING" not in line and CONNECTED == True and "PRIVMSG" in line):
                user = line[0].split("!", 1)
                user = user[0]
                channel = line[2]
                msg = line[3:]
                print "[IN][%s][%s]%s" % (user, channel, ' '.join(msg))

            try:
                if (line[0] == "PING"):
                    s.send("PONG %s\r\n" % line[1])
                    # print "PONG :%s" % line[1]

            
                '''
                    On connect
                '''
                if ("372" in line and "Welcome" in line and "GameSpy" in line):
                    CONNECTED = True
                    operArcade(OPER_NAME, OPER_EMAIL, OPER_PASSWORD)
                    sleep(5)
                    CURRENTCHANNELS = loadStartChannels(CHAN, CURRENTCHANNELS)

            except:
                continue


main(NETWORK, NICK, CHAN, PORT)
