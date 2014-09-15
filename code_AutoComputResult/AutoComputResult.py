# -*- coding: utf-8 -*-
import os

#---------------------------------
#	Name 	= 	AutoComputResult
#	Author	= 	xieliang
#	Time	= 	2014.9
#	Version = 	1.0
#---------------------------------



class AutoComputResult:
	#结果数据的格式(后缀)
	perfmanNFormat="##other##.perf.detail.txt"
	perfmanPFormat=".perf.detail.txt"
	mttesterFormat="none"
	mtrecFormat="none"

	#数据来源的路径
	resourcePath=u"C:\\Users\\Administrator\\Desktop\\性能测试工具\\AutoComputresult\\code_AutoComputResult\\result_source\\test1"

	#perfman统计数据规则
	perfmanRList={u'cpu占用':6,u'物理内存占用':5,u'虚拟内存占用':7}
	
	#统计数据暂存,resultList[0]为计算结果名称
	resultList=['name']

	#excel路径
	excelPath='none'

	#进程列表
	processList=['lttssvc','ses']

	#核数
	cop=8

	#设置是否显示运行信息
	displayset=0

	#快捷计算(核数,进程列表,数据来源路径,excel路径)
	def QstartCompute(self,cop,processList,rePath,excelPath):
		self.cop=cop
		self.processList=processList
		self.resourcePath=rePath
		self.excelPath=excelPath
		return self.startCompute()

	#开始统计
	def startCompute(self):
		#检查初始化数据
		checkerror=self.checkData()
		if 1 in checkerror:
			if checkerror[0]:
				#计算perfman进程数据
				for process in self.processList:
					if os.path.isfile(self.resourcePath+"\\perfman\\"+process+self.perfmanPFormat):
						computePerfmanP(self.resourcePath+"\\perfman\\"+process+self.perfmanPFormat)
		else:
			return 0
		#统计perfman中的数据
		return 1


	#统计perfman中process的数据
	def computePerfmanP(self,perfmanPath):
		print "computePerfmanP"

	#检查初始数据是否正常
	def checkData(self):
		checkerror=[0,0,0,0,0]
		if os.path.isdir(self.resourcePath):

			#检查perfmanProcess数据文件
			self.logv(u"----------------------检查数据文件是否存在-----------------------")
			for process in self.processList:
				if os.path.isfile(self.resourcePath+"\\perfman\\"+process+self.perfmanPFormat):
					self.logv(process+u"数据文件存在")
					checkerror[0]=1
				else:
					self.logv(process+u"数据文件不存在")
					

			#检查perfmanNetWork数据文件
			if os.path.isfile(self.resourcePath+"\\perfman\\"+self.perfmanNFormat):
				self.logv(u"perfman网络数据文件存在")
				checkerror[1]=1
			else:
				self.logv(u"perfman网络数据文件不存在")
				

			#检查mttester数据文件
			if os.path.isfile(self.resourcePath+"\\mttester\\"+self.mttesterFormat):
				self.logv(u"mttester数据文件存在")
				checkerror[2]=1
			else:
				self.logv(u"mttester数据文件不存在")
				

			#检查mttester数据文件
			if os.path.isfile(self.resourcePath+"\\mtrec\\"+self.mttesterFormat):
				self.logv(u"mtrec数据文件存在")
				checkerror[3]=1
			else:
				self.logv(u"mtrec数据文件不存在")
				

			#检查excel文件
			if os.path.isfile(self.excelPath):
				self.logv(u"excel文件存在")
				checkerror[4]=1
			else:
				self.logv(u"excel文件不存在")
				

			#统计数据确认
			self.logv(u"---------------------perfman数据中统计的数值---------------------")
			for key in self.perfmanRList.keys():
				self.logv(key+u"所在列数=>"+str(self.perfmanRList[key]))
			self.logv(u"---------------------mttester数据中统计的数值--------------------")
			self.logv(u"----------------------mtrec数据中统计的数值----------------------")
			self.logv(u"----------------------------其他参数-----------------------------")
			self.logv(u"cpu核数："+str(self.cop))

		else:
			self.logv(u"数据源文件路劲出错")

		#初始化结果暂存数据>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

		return checkerror

	#用于控制显示信息
	def logv(self,str):
		if self.displayset:
			print str

	#设置数据来源路径
	def setResourcePath(self,path):
		self.resourcePath=path

	#设置需要统计的进程
	def setResultList(self,processList):
		self.processList=processList

	#统计perfmanProcess的数据
	#def getResultList(self):
		#print self.resourcePath
		#print self.perfmanNetD
		#print self.resourcePath+self.perfmanNFormat+"\n"
		#perfmanNfile=open(self.resourcePath+self.perfmanNFormat,"r")
		#resource=perfmanNfile.read()
		#print resource[0]