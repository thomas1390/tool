
һ����Ҫ
����һ��linux�����µ�С����վ���ߣ�ԭ���ڵڶ�����˵�����Ƚϼ�ª��
�ó����Ŀ��������һ����վ���������ӣ��������ӵ�ַ�ͷ��ʽ�����浽mysql���ݿ��У�������������е��úͷ��������Կ�����һ��ɨ�蹤�ߵ����沿�ְɣ���Ŀǰֻ֧�ֶ�html4���г����ӷ�������֧��javascript��flash��
����ʹ��linux c��mysql connector-c��libcurl-c��streamhtmlparser��
����mysql connector-c�������ݿ������libcurl���http����ķ�װ��streamhtmlparser��html4�ĳ����ӽ��г�ȡ��
mysql connector-c:	http://dev.mysql.com/downloads/connector/c/
libcurl-c:		http://curl.haxx.se/libcurl/c/
streamhtmlparser:	http://code.google.com/p/streamhtmlparser/

����Ȥ�ģ�������������Դ�����������ϣ����һ�����֣�����~~~��

����ԭ��˵����

1���ó�����������һ��ҳ�棬������ҳ������г����ӣ�Ȼ�󽫻�ȡ�ĳ����Ӳ��뵽mysql���ݿ�ı��У�
2����һ��ܹؼ���ͨ���������Ӧ�ֶ�����Ϊ��������֤�˲�������Ӳ����ظ���
2������һ�����ӷ�����ɺ󣬳����mysql�ж�ȡ�ڶ����������������ĸ����ӣ������η�����ȥ���������õ��ĳ�������Ȼ���뵽mysql�ı��У�
3�������һֱ��mysql�ж�ȡ��һ�����ݲ����ʡ�������ֱ���������ӷ�����ɣ������Ϊ��������վ�������ˡ�

����mysql���sql���£�����Ϊcrawl_site��������mysql.h�н����Զ��壺
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

���������ܵ�һЩ˵����
1����������Ҫ��mysqlһ��ʹ�ã�ɨ�赽��url��url���ʽ����url����ȶ���������mysql��;
2��������ʹ�ù����У����Զ��ж��Ƿ���Ŀ������������Ŀ�������Ĳ���ɨ�裻���������жϣ���ͨ��������ʽ�����е�;
3������֧�����ģ�����������������
a.���նˣ���putty������ΪGB2312�ַ�����
b.���ݿ�����ΪGB2312�ַ�����
c.linux shell���ַ�����������ã�һ������Ϊc��
4������֧�ּ򵥵��û���/�������͵ı���֤������Ŀǰֻ֧��ʹ����һ��С�����ֶ��õ���cookie�ļ�����Ϊһ��������Ա�ֹ�ʹ�õļ�ª���ߣ������ǲ�Ʒ����û����Ϊ��������֤���ܣ�����crawler������ȥ������cookie�ļ�����Ϊcookie.txt
5������֧����ͣ�������Ĺ��ܣ�ͨ������Ϊresume.txt���ļ�����ʵ�ֶϵ�������
6��linux��ʹ��ʱ�����鿪��nscd��������linux���crawler��dns������л��棬������Ƶ���Ľ���dns��ѯ��

Ŀǰ�Ĳ���֮��:
1��Ŀǰ��֧��ssl��
2��Ŀǰ��δʵ�ֶ��̣߳�
3��Ŀǰ���� �û���/���� �ı���֤��ֻ֧��ʹ������һ��С�����ֶ����ɵ�cookie�ļ���
4�������ʹ�ý��棬��δ��ȫ���а�װ���Ժ�Ҳ��׼���˳�GUI�档
5��Ŀǰ�����ӵĳ�ȡ��ʹ�õ���streamhtmlparser����֧�ֶ�javascript��flash��������ȡ��ҳ���ӣ�
��Ϊһ�������ɨ������Ϊ������վ̫�����վ����̫�࣬��������ֹ������ȥ������IBM Appsacn 8��Acunetix Web Vulnerability Scanner 7�������������������¼�����������������һЩ���ƣ���Ŀǰ��δ�ӽ�ȥ����Ϊ����ʹ�õĻ����٣�
1��û��ָ��ɨ���������ӵ���Ŀ������Ƚ����׼��ϣ�
2��û�ж����ӵ���Ƚ������ƣ�����ֻ�Ǵ�ӡ���˵�ǰ����ȣ����Ҳ�Ƚ�����ʵ�֣�
3��û�ж���վĿ¼����Ƚ������ƣ����Ҳ�Ƚ�����ʵ�֡�
4��û�ж���ͬ·���ļ�����ͬ������url����ʶ��������url���ص����ݶ�����ͬ�ģ����Է��صĳ�����Ӧ����һ���ģ�û��Ҫȫ������һ�ߣ�������bbs��ÿ�����Ӷ�Ӧ�����ӣ������Ǵ�Acunetix Web Vulnerability Scanner 7���ֲ���ժ���Ķ���仰�Ľ��ܣ�
(Maximum number of variations �C In this option you can specify the
maximum number of variations for a file. E.g. index.asp has a GET
parameter ID of which the crawler discovered 10 possible values of it
from links requesting index.asp with the ID set to a different. Each of
these links is a variation. Each variation will appear under the file in the
Scan Tree during crawling.)

����˵����
������Ǹ����û��ȥдmakefile�ļ���ʹ��gcc���е������н��б༭��
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

�ġ�����ʹ��˵����
ʵ��ʹ��ʱ����ѭ����������
1������mysql���ݿ⣬ʹ�������ṩ��sql��䣬����һ�������ݿ��IP��ַ���������û��������������mysql.h�н����޸ģ��Դ�����spider��192.168.85.135��sp��sp��
2���������
3������resume.txt�ļ�
[test@bogon 0128]$touch resume.txt
����������ʱ��������cookie�ļ���С���ߣ���ʹ�ó���ʱ���һ����������Ϊ0��
4����ʼʹ�ó��򼴿ɡ�

[test@bogon 0128]$ ./crawler
        [Usage]:./crawler url domain-port regex[scan_goal]

        [Example]:./crawler 192.168.85.5/manual/admin.php 8080 192.168.85.5/manual 0(need-cookie:0,1)

        It will try to crawl the url[192.168.85.5/manual/admin.php],this url must match the regex[192.168.85.5/manual],and this programe will try to resume last scan from resume.txt.

        This version now don't support post user/pass form,need copy the cookie.txt && creat resume.txt manually.
