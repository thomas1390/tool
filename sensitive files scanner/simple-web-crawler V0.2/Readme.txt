
一、概要
这是一个linux环境下的小的爬站工具，原理在第二部分说明，比较简陋。
该程序的目的是爬出一个网站的所有链接，并将链接地址和访问结果保存到mysql数据库中，供其他程序进行调用和分析。可以看成是一个扫描工具的爬虫部分吧：）目前只支持对html4进行超链接分析，不支持javascript和flash。
开发使用linux c，mysql connector-c，libcurl-c，streamhtmlparser。
其中mysql connector-c用于数据库操作，libcurl完成http请求的封装，streamhtmlparser对html4的超链接进行抽取。
mysql connector-c:	http://dev.mysql.com/downloads/connector/c/
libcurl-c:		http://curl.haxx.se/libcurl/c/
streamhtmlparser:	http://code.google.com/p/streamhtmlparser/

感兴趣的，可以向我所有源代码和相关资料，大家一起研讨，进步~~~！

二、原理说明：

1、该程序首先请求一个页面，分析该页面的所有超链接，然后将获取的超链接插入到mysql数据库的表中；
2、这一点很关键：通过将表的相应字段设置为主键，保证了插入的链接不会重复；
2、当第一个链接访问完成后，程序从mysql中读取第二个、第三个、第四个链接，并依次访问下去，将分析得到的超链接仍然插入到mysql的表中；
3、程序会一直从mysql中读取下一条数据并访问、分析，直到所有链接访问完成，这就认为是整个网站被爬完了。

创建mysql表的sql如下，表名为crawl_site，可以在mysql.h中进行自定义：
/*
Navicat MySQL Data Transfer
Source Host     : 192.168.85.135:3306
Source Database : spider
Target Host     : 192.168.85.135:3306
Target Database : spider
Date: 2011-02-21 14:24:35
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for crawl_site
-- ----------------------------
DROP TABLE IF EXISTS `crawl_site`;
CREATE TABLE `crawl_site` (
  `id` int(16) NOT NULL,
  `links` varchar(256) NOT NULL default '',
  `depth` int(8) default NULL,
  `ret_code` int(8) default NULL,
  PRIMARY KEY  (`links`)
) ENGINE=MyISAM DEFAULT CHARSET=gb2312;

-- ----------------------------
-- Records of crawl_site
-- ----------------------------

三、程序功能的一些说明：
1）本程序需要和mysql一起使用，扫描到的url、url访问结果、url的深度都被保存在mysql中;
2）程序在使用过程中，会自动判断是否是目标域名，不是目标域名的不会扫描；对域名的判断，是通过正则表达式来进行的;
3）程序支持中文，须满足以下条件：
a.将终端（如putty）设置为GB2312字符集；
b.数据库设置为GB2312字符集；
c.linux shell的字符集可随便设置，一般设置为c；
4）程序支持简单的用户名/密码类型的表单认证，但是目前只支持使用另一个小工具手动得到的cookie文件，作为一个测试人员手工使用的简陋工具，而不是产品，并没有作为单独的认证功能，集成crawler程序中去；而且cookie文件名字为cookie.txt
5）程序支持暂停，继续的功能，通过命名为resume.txt的文件，来实现断点续传；
6）linux下使用时，建议开启nscd服务，这样linux会对crawler的dns请求进行缓存，避免了频繁的进行dns查询；

目前的不足之处:
1、目前不支持ssl；
2、目前还未实现多线程；
3、目前对于 用户名/密码 的表单验证，只支持使用另外一个小工具手动生成的cookie文件；
4、程序的使用界面，还未完全进行包装，以后也不准备退出GUI版。
5、目前对链接的抽取，使用的是streamhtmlparser，不支持对javascript和flash分析来获取网页链接；
作为一个成熟的扫描器，为避免网站太大或网站内容太多，导致无休止的爬下去，参照IBM Appsacn 8和Acunetix Web Vulnerability Scanner 7的做法，可以设置以下几个参数，对爬行做一些订制，但目前均未加进去，因为工具使用的还较少；
1）没有指定扫描的最大链接的数目，这个比较容易加上；
2）没有对链接的深度进行限制，现在只是打印出了当前的深度，这个也比较容易实现；
3）没有对网站目录的深度进行限制，这个也比较容易实现。
4）没有对相同路径文件、不同参数的url进行识别，这样的url返回的内容都是相同的，所以返回的超链接应该是一样的，没必要全部都爬一边；常见于bbs中每个帖子对应的链接；下面是从Acunetix Web Vulnerability Scanner 7的手册中摘出的对这句话的介绍：
(Maximum number of variations – In this option you can specify the
maximum number of variations for a file. E.g. index.asp has a GET
parameter ID of which the crawler discovered 10 possible values of it
from links requesting index.asp with the ID set to a different. Each of
these links is a variation. Each variation will appear under the file in the
Scan Tree during crawling.)

编译说明：
：）抱歉，还没有去写makefile文件，使用gcc进行的命令行进行编辑：
[test@bogon 0128]$ ll
total 92
-rw-rw-r-- 1 test test   862 Feb 20 16:00 cookie.txt
-rw-rw-r-- 1 test test  4920 Mar  6 09:27 curl_mem.h
-rw-rw-r-- 1 test test  3087 Feb 24 16:31 mysql.h
-rw-rw-r-- 1 test test   100 Mar  6 09:35 resume.txt
-rw-rw-r-- 1 test test   690 Jan 30 23:20 rxp.h
-rwxrwxr-x 1 test test 45438 Mar 10 17:27 crawler
-rw-rw-r-- 1 test test 19442 Mar 10 17:36 crawler.c
[test@bogon 0128]$gcc -o crawler crawler.c -lstreamhtmlparser -lcurl `mysql_config --cflags --libs`
crawler.c: In function 'main':
crawler.c:679: warning: passing argument 2 of 'signal' from incompatible pointer type
crawler.c:680: warning: passing argument 2 of 'signal' from incompatible pointer type
[test@bogon 0128]$ 

四、程序使用说明：
实际使用时，遵循以下三步：
1、开启mysql数据库，使用上面提供的sql语句，创建一个表，数据库的IP地址、库名、用户名、密码可以在mysql.h中进行修改，自带的是spider、192.168.85.135、sp、sp；
2、编译程序；
3、创建resume.txt文件
[test@bogon 0128]$touch resume.txt
发布包中暂时不带生成cookie文件的小工具，故使用程序时最后一个参数必须为0；
4、开始使用程序即可。

[test@bogon 0128]$ ./crawler
        [Usage]:./crawler url domain-port regex[scan_goal]

        [Example]:./crawler 192.168.85.5/manual/admin.php 8080 192.168.85.5/manual 0(need-cookie:0,1)

        It will try to crawl the url[192.168.85.5/manual/admin.php],this url must match the regex[192.168.85.5/manual],and this programe will try to resume last scan from resume.txt.

        This version now don't support post user/pass form,need copy the cookie.txt && creat resume.txt manually.
