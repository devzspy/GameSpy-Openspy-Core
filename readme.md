![GameSpy Arcade](https://i.imgur.com/t1IET6z.png)

# Hosts File Required DNS Records

* motd.gamespy.com
* peerchat.gamespy.com
* gamestats.gamespy.com
* gpcm.gamespy.com
* gpsp.gamespy.com
* key.gamespy.com
* master.gamespy.com
* master0.gamespy.com
* natneg1.gamespy.com
* natneg2.gamespy.com
* natneg3.gamespy.com
* chat.gamespynetwork.com
* available.gamespy.com
* gamespy.com
* gamespyarcade.com
* www.gamespy.com
* www.gamespyarcade.com
* chat.master.gamespy.com

## OS Build Recommendation
* Debian 7
* Ubuntu 12
* Ubuntu 14

## Peerchat Oper commands

* /oper user email password (Required to perform oper commands below. Gives you admin rights)
* /raw join #channel x i  (Invisibly join channel as an oper'd user)
* /raw join #channel i i (unhide yourself, may require leaving and rejoining)
* /raw ATM username :?IMP INFO (GameSpy Arcade client /whois check)
* /raw ATM #channel ATM :CHDEL word (Delete messages containing the specified word. Causes chat to scroll when line gets deleted)
* /wallops (sends a global message in notice form to all users from SERVER)
* /stats (shows a lot of stats such as messages per second etc)
* /kill username (kills a user from the server)
* /kline ip (klines {bans} an ip, also accepts ranges)

## Client Spoofing

* /setckey #channel current_connected_username :\b_reg60\1  (Subscriber)
* /setckey #channel current_connected_username :\b_reg60\2  (Founders Club)
* /setckey #channel current_connected_username :\b_reg60\3  (Subscriber with Blue star)
* /setckey #channel current_connected_username :\b_look\393217001,399770601 (Spoof portrait and icon)
* /setckey #channel current_connected_username :\b_flags\ (Default status)
* /setckey #channel current_connected_username :\b_flags\a (Away status)
* /setckey #channel current_connected_username :\b_flags\s (Staging lobby status)
* /setckey #channel current_connected_username :\b_flags\g (Launched game status)
* /getckey #channel User_logged_into_arcade-gs 000 0 :\b_look\ (Pulls that users portrait and icon)

## Fun Stuff for Arcade. (Some may require admin in channels)

* @@1 insert channel topic (makes a popup appear for that lobby. place in channel topic)
* /notice #channel message (send blue text to channel via IRC client. color changes based on arcade skin)

## GameSpy Arcade images

* Custom portraits go under /software/imglib/portraits/00000/00001.jpg. Database must reflect the number at end. Must be 96 pixels by 96 pixels
* Custom icons go under /software/imglib/icons/00000/00001.jpg. Database must reflect the number at end. Must be 20 pixels by 16 pixels
* Category sections (Admins, Helpers, Wizards, Staff, Speaking, Away) all go under /software/imglib/portraits/07100/(insert_category_name).jpg