-- 移除資料庫權限
--REVOKE ALL PRIVILEGES ON *.* FROM 'gameuser'@'localhost';
--REVOKE GRANT OPTION ON *.* FROM 'gameuser'@'localhost';
--FLUSH PRIVILEGES;

-- 刪除用戶
--DROP USER 'gameuser'@'localhost';

-- 刪除資料庫
DROP DATABASE IF EXISTS `chatbar`;
DROP TABLE IF EXISTS `user`;
DROP TABLE IF EXISTS `timeRecord`;
