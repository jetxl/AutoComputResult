# -*- coding: utf-8 -*-
import os

#---------------------------------
#	Name 	= 	AutoComputResult
#	Author	= 	xieliang
#	Time	= 	2014.9
#	Version = 	1.0
#---------------------------------


print "Now is computing result....."
class AutoComputResult:
	#结果数据的格式(后缀)
	perfmanNFormat="##other##.perf.detail.txt"
	perfmanPFormat=".perf.detail.txt"
	mttesterFormat=""
	mtrecFormat=""

	#结果数据的路径
	resourcePath="C:\\Users\\Administrator\\Desktop\\性能测试工具\\AutoComputResult\\resource\\result_source\\Perfman\\"
	resourcePath=unicode(resourcePath,"utf-8")
	
	#用于统计暂存数据
	BytesReceived	=	0
	BytesSent		=	0      
	BytesTotal		=	0
	Handle count        Thread count        Working Set         % Processor Time    private Bytes
	perfmanProcess=[];


	def __int__(self):
		print "this is init\n"
		

	def getPerfmanN(self):
		print self.resourcePath+self.perfmanNFormat+"\n"
		perfmanNfile=open(self.resourcePath+self.perfmanNFormat,"r")
		resource=perfmanNfile.read()
		print resource[0]