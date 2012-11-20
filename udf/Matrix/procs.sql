DELIMITER //
CREATE PROCEDURE SetGlobalOper (IN vprofileid INT, IN vrightsmask INT, IN vemail varchar(50), IN vname varchar(30))
BEGIN
DECLARE msgid int;
DECLARE rowcount int;
SELECT COUNT(*) INTO rowcount FROM `globalopers` WHERE `profileid` = vprofileid;
if rowcount > 0 THEN UPDATE `globalopers` SET `rightsmask` = vrightsmask, `email` = vemail, `nick` = vname WHERE `profileid` = vprofileid;
else INSERT INTO `globalopers` (`profileid`,`rightsmask`,`email`,`nick`) VALUES (vprofileid,vrightsmask,vemail,vname);
end if;
SET msgid = 32;
select matrixqueue(msgid,vprofileid,vrightsmask);
END//
DELIMITER ;

DELIMITER //
CREATE PROCEDURE DelGlobalOper (IN vprofileid INT)
BEGIN
DECLARE msgid int;
DELETE FROM `globalopers` WHERE `profileid` = vprofileid;
SET msgid = 33;
select matrixqueue(msgid,vprofileid);
END//
DELIMITER ;


DELIMITER //
CREATE PROCEDURE SetUserMode(IN vchanmask TEXT, IN vcommenttxt TEXT, IN vexpires TIMESTAMP, IN vhostmask TEXT, IN vmachineid TEXT, IN vmodeflags INT, IN vprofileid INT, IN vsetbyhost TEXT, IN vsetbynick TEXT, IN vsetbypid TEXT)
BEGIN
DECLARE usermodeid int;
DECLARE msgid int;
INSERT INTO `chanusermodes` (`chanmask`,`comment`,`expires`,`hostmask`,`machineid`,`modeflags`,`profileid`,`setbyhost`,`setbynick`,`setbypid`)  VALUES (vchanmask,vcommenttxt,vexpires,vhostmask,vmachineid,vmodeflags,vprofileid,vsetbyhost,vsetbynick,vsetbypid);
SET usermodeid = LAST_INSERT_ID();
SET msgid = 34;
select matrixqueue(msgid,vchanmask,vcommenttxt,Unix_Timestamp(vexpires),vhostmask,vmachineid,vmodeflags,vprofileid,vsetbyhost,vsetbynick,vsetbypid,Unix_Timestamp(CURRENT_TIMESTAMP),usermodeid);
END//
DELIMITER ;


DELIMITER //
CREATE PROCEDURE DelUserMode(IN vusermodeid INT)
BEGIN
DECLARE msgid int;
DELETE FROM `chanusermodes` WHERE `usermodeid` = vusermodeid;
SET msgid = 35;
select matrixqueue(msgid,vusermodeid);
END//
DELIMITER ;


DELIMITER //
CREATE PROCEDURE SetChanProps(IN vchanmask TEXT, in vchankey TEXT, IN vcommentxt TEXT, IN ventrymsg TEXT, IN vexpires TIMESTAMP, IN vgroupname TEXT, IN vchanlimit INT, IN vmodes TEXT, IN vonlyowner INT, IN vsetbynick TEXT, IN vsetbyhost TEXT, IN vsetbypid INT, IN vtopic TEXT, IN vkickexisting INT)
BEGIN
DECLARE msgid int;
DECLARE rowcount int;
DECLARE setondate TIMESTAMP;
SET setondate = CURRENT_TIMESTAMP;
SELECT COUNT(*) INTO rowcount FROM `chanprops` WHERE `chanmask` = vchanmask;
if rowcount > 0 THEN UPDATE `chanprops` SET `chankey` = vchankey, `comment` = vcommentxt, `entrymsg` = ventrymsg, `expires` = vexpires, `groupname` = vgroupname, `limit` = vchanlimit, `mode` = vmodes, `onlyowner` = vonlyowner, `setbynick` = vsetbynick, `setbyhost` = vsetbyhost, `setbypid` = vsetbypid, `topic` = vtopic, `setondate` = setondate WHERE `chanmask` = vchanmask;
ELSE INSERT INTO `chanprops` (`chanmask`,`chankey`,`comment`,`entrymsg`,`expires`,`groupname`,`limit`,`mode`,`onlyowner`,`setbynick`,`setbyhost`,`setbypid`,`topic`) VALUES (vchanmask,vchankey,vcommentxt,ventrymsg,vexpires,vgroupname,vchanlimit,vmodes,vonlyowner,vsetbynick,vsetbyhost,vsetbypid,vtopic);
end if;
SET msgid = 36;
select matrixqueue(msgid,vchanmask,vchankey,vcommentxt,ventrymsg,Unix_Timestamp(vexpires),vgroupname,vchanlimit,vmodes,vonlyowner,vsetbynick,vsetbyhost,vsetbypid,vtopic,Unix_Timestamp(setondate),vkickexisting);
END//
DELIMITER ;

DELIMITER //
CREATE PROCEDURE DelChanProps(IN vchanmask TEXT,  IN vkickexisting INT)
BEGIN
DECLARE msgid int;
DELETE FROM `chanprops` WHERE `chanmask` = vchanmask;
SET msgid = 37;
select matrixqueue(msgid,vchanmask,vkickexisting);
END//
DELIMITER ;

DELIMITER //
CREATE PROCEDURE SetChanClient(IN vchanmask TEXT,  IN vgameid INT)
BEGIN
DECLARE msgid int;
INSERT INTO `chanclients` (`chanmask`,`gameid`) VALUES (vchanmask,vgameid);
SET msgid = 38;
select matrixqueue(msgid,vchanmask,vgameid);
END//
DELIMITER ;

DELIMITER //
CREATE PROCEDURE DelChanClient(IN vchanmask TEXT,  IN vgameid INT)
BEGIN
DECLARE msgid int;
DELETE FROM `chanclients` WHERE `chanmask` = vchanmask AND `gameid` = gameid;
SET msgid = 39;
select matrixqueue(msgid,vchanmask,vgameid);
END//
DELIMITER ;

DELIMITER //
CREATE PROCEDURE AuthClient(IN vnick TEXT, IN vemail TEXT, IN pass TEXT, IN connid INT, IN sendnotice INT)
-- pass is md5 hashed
-- if email is empty nick = uniquenick
-- connid = matrix socket descriptor
-- sendnotice = send Rights Granted, etc on Matrix
BEGIN
DECLARE msgid int;
DECLARE vuserid int;
DECLARE vprofileid int;
DECLARE vuniquenick TEXT;
SET vuserid = 0; -- if the server gets a userid of 0 it will know that the pass or something was wrong
SET vprofileid = 0; -- maybe use profileid as the reason?
-- if email len = 0 check by unique nick and pass
if length(vemail) = 0 then SELECT `GameTracker`.`profiles`.`userid`,`GameTracker`.`profiles`.`profileid`,`uniquenick` INTO vuserid, vprofileid, vuniquenick FROM `GameTracker`.`profiles` INNER JOIN `GameTracker`.`users` on `GameTracker`.`users`.`userid` = `GameTracker`.`profiles`.`userid` WHERE md5(`GameTracker`.`users`.`password`) = pass AND `GameTracker`.`profiles`.`uniquenick` = vnick LIMIT 0,1;
ELSE
SELECT `GameTracker`.`profiles`.`userid`,`GameTracker`.`profiles`.`profileid`,`uniquenick` INTO vuserid, vprofileid, vuniquenick FROM `GameTracker`.`profiles` INNER JOIN `GameTracker`.`users` on `GameTracker`.`users`.`userid` = `GameTracker`.`profiles`.`userid` WHERE md5(`GameTracker`.`users`.`password`) = pass AND `GameTracker`.`profiles`.`nick` = vnick AND `GameTracker`.`users`.`email` = vemail LIMIT 0,1;
end if;
SET msgid = 40;
select matrixqueue(msgid,vuserid,vprofileid,vuniquenick,connid,sendnotice);
END//
DELIMITER ;




