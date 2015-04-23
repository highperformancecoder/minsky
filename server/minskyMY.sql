-- phpMyAdmin SQL Dump
-- version 3.5.3
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Mar 20, 2013 at 03:51 AM
-- Server version: 5.5.28-log
-- PHP Version: 5.3.15

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `minsky`
--

-- --------------------------------------------------------

--
-- Table structure for table `elements`
--

DROP TABLE IF EXISTS `elements`;
CREATE TABLE IF NOT EXISTS `elements` (
  `modelid` int(11) NOT NULL,
  `id` int(11) NOT NULL,
  `type` varchar(255) NOT NULL,
  `data` text NOT NULL,
  PRIMARY KEY (`modelid`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `layouts`
--

DROP TABLE IF EXISTS `layouts`;
CREATE TABLE IF NOT EXISTS `layouts` (
  `modelid` int(11) NOT NULL,
  `id` int(11) NOT NULL,
  `type` varchar(255) NOT NULL,
  `data` text NOT NULL,
  PRIMARY KEY (`modelid`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `models`
--

DROP TABLE IF EXISTS `models`;
CREATE TABLE IF NOT EXISTS `models` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `owner` varchar(255) NOT NULL,
  `maxelem` int(11) NOT NULL,
  `data` text NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=3 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
