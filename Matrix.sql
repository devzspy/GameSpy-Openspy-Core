-- MySQL dump 10.13  Distrib 5.5.54, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: Matrix
-- ------------------------------------------------------
-- Server version	5.5.54-0ubuntu0.12.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `chanclients`
--

DROP TABLE IF EXISTS `chanclients`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `chanclients` (
  `chanmask` text NOT NULL,
  `gameid` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `chanclients`
--

LOCK TABLES `chanclients` WRITE;
/*!40000 ALTER TABLE `chanclients` DISABLE KEYS */;
INSERT INTO `chanclients` VALUES ('#GPG!27',0),('#GPG!25',0),('#GPG!26',0),('#staff',0),('#dtr',0),('#thps',0),('#GSP!cossacks',0),('*',-1),('*_updates',0),('#GPG!5',0),('#GPG!6',0),('#GSP!sacrifice',0);
/*!40000 ALTER TABLE `chanclients` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `chanprops`
--

DROP TABLE IF EXISTS `chanprops`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `chanprops` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `chankey` text NOT NULL,
  `chanmask` text NOT NULL,
  `comment` text NOT NULL,
  `entrymsg` text NOT NULL,
  `expires` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `groupname` text NOT NULL,
  `limit` int(11) NOT NULL DEFAULT '0',
  `mode` text NOT NULL,
  `onlyowner` tinyint(1) NOT NULL DEFAULT '0',
  `setbynick` text NOT NULL,
  `setbypid` int(11) NOT NULL,
  `setondate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `topic` text NOT NULL,
  `setbyhost` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `chanprops`
--

LOCK TABLES `chanprops` WRITE;
/*!40000 ALTER TABLE `chanprops` DISABLE KEYS */;
INSERT INTO `chanprops` VALUES (1,'','#gsp!subhome','','Welcome to GameSpy','0000-00-00 00:00:00','',0,'tn',0,'Falcon',10000,'2018-08-25 10:33:00','Welcome to GameSpy! Hosted by Falcon','192.168.1.32');
/*!40000 ALTER TABLE `chanprops` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `chanusermodes`
--

DROP TABLE IF EXISTS `chanusermodes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `chanusermodes` (
  `chanmask` text NOT NULL,
  `comment` text NOT NULL,
  `expires` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `hostmask` text NOT NULL,
  `machineid` text NOT NULL,
  `modeflags` int(11) NOT NULL,
  `profileid` int(11) NOT NULL,
  `setbyhost` text NOT NULL,
  `setbynick` text NOT NULL,
  `setbypid` text NOT NULL,
  `setondate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `usermodeid` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`usermodeid`)
) ENGINE=MyISAM AUTO_INCREMENT=252 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `chanusermodes`
--

LOCK TABLES `chanusermodes` WRITE;
/*!40000 ALTER TABLE `chanusermodes` DISABLE KEYS */;
/*!40000 ALTER TABLE `chanusermodes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `globalopers`
--

DROP TABLE IF EXISTS `globalopers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `globalopers` (
  `email` text NOT NULL,
  `nick` text NOT NULL,
  `profileid` int(11) NOT NULL,
  `rightsmask` int(11) NOT NULL,
  `setondate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`profileid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `globalopers`
--

LOCK TABLES `globalopers` WRITE;
/*!40000 ALTER TABLE `globalopers` DISABLE KEYS */;
INSERT INTO `globalopers` VALUES ('falcon@gamespy.com','Falcon',10000,65535,'2018-08-26 05:36:29');
/*!40000 ALTER TABLE `globalopers` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-08-27  1:00:32
