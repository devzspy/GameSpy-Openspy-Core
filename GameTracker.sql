-- MySQL dump 10.13  Distrib 5.5.35, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: GameTracker
-- ------------------------------------------------------
-- Server version 5.5.35-0ubuntu0.12.04.2

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
-- Table structure for table `games`
--

DROP TABLE IF EXISTS `games`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `games` (
  `id` int(3) NOT NULL AUTO_INCREMENT,
  `gamename` varchar(30) NOT NULL,
  `queryport` int(5) NOT NULL,
  `disabledservices` text NOT NULL,
  `backendflags` int(11) NOT NULL,
  `secretkey` varchar(64) NOT NULL,
  `keylist` text NOT NULL,
  `keytypelist` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `profiles`
--

DROP TABLE IF EXISTS `profiles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `profiles` (
  `profileid` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL,
  `nick` text NOT NULL,
  `uniquenick` text NOT NULL,
  `firstname` text NOT NULL,
  `lastname` text NOT NULL,
  `sex` smallint(6) NOT NULL DEFAULT '0' COMMENT '0 = male, 1 = female, else unknown',
  `icquin` text NOT NULL,
  `deleted` tinyint(1) NOT NULL DEFAULT '0',
  `aimname` text NOT NULL,
  `pic` int(11) NOT NULL DEFAULT '0',
  `homepage` text NOT NULL,
  `place` text NOT NULL,
  `zipcode` text NOT NULL,
  `countrycode` varchar(3) NOT NULL DEFAULT 'US',
  `lon` float NOT NULL,
  `lat` float NOT NULL,
  `birthday` date NOT NULL,
  `ooc` int(11) NOT NULL,
  `ind` int(11) NOT NULL,
  `inc` int(11) NOT NULL,
  `mar` int(11) NOT NULL,
  `chc` int(11) NOT NULL,
  `i1` int(11) NOT NULL,
  `o1` int(11) NOT NULL,
  `conn` int(11) NOT NULL,
  `createddate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `useddate` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`profileid`)
) ENGINE=MyISAM AUTO_INCREMENT=10001 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users` (
  `userid` int(11) NOT NULL AUTO_INCREMENT,
  `email` text NOT NULL,
  `lastip` int(11) NOT NULL DEFAULT '0',
  `password` text NOT NULL,
  `createddate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `useddate` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `streetaddr` text NOT NULL,
  `streetaddr2` text NOT NULL,
  `city` text NOT NULL,
  `cpubrandid` int(11) NOT NULL DEFAULT '0',
  `cpuspeed` smallint(6) NOT NULL DEFAULT '0',
  `memory` tinyint(4) NOT NULL DEFAULT '0',
  `videocard1string` text NOT NULL,
  `videocard1ram` tinyint(4) NOT NULL DEFAULT '0',
  `videocard2string` text NOT NULL,
  `videocard2ram` tinyint(4) NOT NULL DEFAULT '0',
  `subscription` int(11) NOT NULL DEFAULT '0',
  `emailverified` tinyint(1) NOT NULL DEFAULT '0',
  `publicmask` int(1) NOT NULL DEFAULT '-1',
  `adminrights` int(11) NOT NULL DEFAULT '0',
  `deleted` tinyint(1) NOT NULL,
  PRIMARY KEY (`userid`)
) ENGINE=MyISAM AUTO_INCREMENT=10001 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-11-22  0:38:31
