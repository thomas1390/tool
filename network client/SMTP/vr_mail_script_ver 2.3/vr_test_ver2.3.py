#RFC of SMTP：5321	2821	821
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import smtplib
import email
import time
import random
import sys
from email.mime.text import MIMEText
from email import utils
from email.mime.multipart import MIMEMultipart
from email.charset import add_charset, Charset,QP,BASE64
#from email import encoders

def	send_mail(subject,mail_txt,mail_format):
	sender = 'chinavr23@sau.edu.cn'
	receiver1 = 'test@aliyun.com'
	smtpserver = 'smtp.sau.edu.cn'
	username = 'chinavr23@sau.edu.cn'
	password = 'chinavr2014'

	#You can create a new object structure by creating Message instances, adding attachments and all the appropriate headers manually.For MIME messages though, the email package provides some convenient subclasses to make things easier.
	msgRoot = MIMEMultipart('alternative')

	#header section
	msgRoot['Date']= email.utils.formatdate()          # curr datetime, rfc2822
	msgRoot['From']= sender
	msgRoot['Subject']= subject
	msgRoot['X-Priority']= '1'
	msgRoot['X-Has-Attach']= 'no'
	msgRoot['X-Mailer']= 'Foxmail 7, 2, 5, 140[en]'
	#MIMEMultipart方法将添加MIME-Version header
	#msgRoot['Mime-Version']='1.0'
	msgRoot['Message-ID'] = email.utils.make_msgid()

	#body section
	fp = open(mail_txt, 'rt',encoding='utf-8')
	msgText = MIMEText(fp.read(),'plain','utf-8')
	fp.close()
	msgRoot.attach(msgText)

	fp = open(mail_format,"rt",encoding='utf-8')
	msgHtml=fp.read()
	fp.close()
	#这里需要重新定义字符集，否则MIMEText会使用Base64对HTML格式进行编码，这里须要注意
	add_charset('utf-8', BASE64, QP, 'utf-8')
	msgHtml = MIMEText(msgHtml,'html','utf-8')
	msgRoot.attach(msgHtml)

	#envelope section
	smtp = smtplib.SMTP()
	smtp.connect(smtpserver)
	smtp.login(username, password)

	#log section
	f1 = open("addr_list.txt", "rt")
	f2 = open("send_log.txt", "a")
	f2.write('\n'+email.utils.formatdate()+'\n')
	i=0
	while True:
		line = f1.readline()
		if line:
			i+=1
			msgRoot['To']= line

			if i!=1:
				time.sleep(random.randint(0,40))
			smtp.sendmail(sender, line, msgRoot.as_string())
			#每次连接，尽可能多的RCPT信件，然后再quit，尽可能降低被识别为垃圾邮件的概率
			#学校邮件服务器限制每次SMTP连接最多RCPT15封邮件，所以每到15封，需要重新进行SMTP连接
			if i%15 == 0:
				smtp.quit()
				time.sleep(random.randint(0,40))
				#smtp = smtplib.SMTP()
				smtp.connect('smtp.sau.edu.cn')
				smtp.login(username, password)
			else:
				smtp.rset()
			#msgRoot为一个列表，删除之前的收件人，否则每次会向该列表中依次添加收件人，导致收件人的地址栏会同时显示之前已发送所有收件人的地址，看起来似乎群发，虽然实际上并没向已发送过的地址重复发送
			del msgRoot['To']
			print('id:',i,line)
			f2.write('sent'+str(i)+':'+line)	
		else:
			print('\nsent over')
			f2.write('\nsent over!\n')
			break
	f1.close()
	f2.close()

	smtp.quit()
	return

subject_en = 'The 4th International Conference on Virtual Reality and Visualization (ICVRV 2014) Call for Paper'
subject_ch = '=?utf-8?B?56ys5Y2B5Zub5bGK5Lit5Zu96Jma5ouf546w5a6e5aSn5Lya5b6B56i/6YCa55+l?='
# Script starts from here
if len(sys.argv) < 2:
    print ('No action specified.\n')
    sys.exit()
if sys.argv[1] == '--help':
	print ('\tusage:',sys.argv[0],'--ch')
	print('\t\tto send the chinese mail\n')
	print ('\t      ',sys.argv[0],'--en')
	print('\t\tto send the english mail')
elif sys.argv[1] == '--ch':
	send_mail(subject_ch,'ch\ch_txt.txt','ch\ch_format.txt')
elif sys.argv[1] == '--en':
	send_mail(subject_en,'en\en_txt.txt','en\en_format.txt')
else:
	print ('Unknown arguments.\n')  
	sys.exit()