# -*- coding: utf-8 -*-  
from AutoComputResult import *


COMput=AutoComputResult()
if COMput.startCompute():
	print u"计算成功"
else:
	print u"计算错误,请检查文件是否正常！"