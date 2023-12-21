CREATE DATABASE `chatbar`;
USE `chatbar`;

CREATE TABLE `user` (
    `UserID` int  NOT NULL AUTO_INCREMENT,
    `UserName` varchar(100)  NOT NULL ,
    `UserPassword` varchar(100)  NOT NULL ,
    PRIMARY KEY (
        `UserID`
    )
);