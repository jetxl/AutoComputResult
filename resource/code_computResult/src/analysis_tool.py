# -*- coding: cp936-*-
import os,re,shutil,glob
from xlrd import open_workbook,cellname
from xlutils.copy import copy
from xlwt import *
"""
说明：
1、运行脚本前，请先导入三个第三方python库：xlwt、xlrd、xlutils。可以在http://www.python-excel.org/上下载到相应的库。
解压后有两种安装方法：一种是进入解压文件中setup.py所在目录，执行python setup.py install ；另一种是将解压包放在python安装目录的lib/site-packages目录下即可。
2、将此脚本和每个厂商的性能数据文件夹放在同一层中，执行Run->Run Module即可得到相应的.xls文件；或者在命令行执行脚本也可以。
"""
#获取某指定目录下的所有子目录的列表
def getDirList(p):
    p = str(p)
    if p=="":
        return []
    p = p.replace("/","\\")
    if p[ -1] != "\\":
        p = p+"\\"
    a = os.listdir(p)
    b = [x for x in a if os.path.isdir(p+x)]
    return b        

#获取某指定目录下的所有文件的列表
def getFileList(p):
    p = str(p)
    if p=="":
        return []
    p = p.replace("/","\\")
    if p[-1] != "\\":
        p = p+"\\"
    a = os.listdir(p)
    b = [x for x in a if os.path.isfile(p+x)]
    return b

#写数据到文件中
def FileWrite(onestr,filename,mode):
    "args:string,filename,mode"
    ff=open(filename,mode)
    ff.write(onestr)
    ff.close()

#处理系统性能统计时每行数据，根据空格、逗号、tab分割
def LineSplit(inputString):
    first_elems=[x for x in str(inputString).split(',') if len(str(x).strip())>0]
    second_elems=[]
    for x1 in first_elems:
        temp_elem=[y for y in str(x1).split('\t') if len(str(y).strip())>0]
        for one in temp_elem:
            second_elems.append(one)
    #third_elems=[]
    #for x2 in second_elems:
        #temp_elem=[z for z in str(x2).split(' ') if len(str(z).strip())>0]
        #for one in temp_elem:
            #third_elems.append(one)
    return second_elems
    

#提取字符串中数字
def Str2Float(inputStr):
    if str(inputStr).isdigit():
        return float(inputStr)
    else:
        ret_string=""
        for c in inputStr:
            if str(c).isdigit() or c=='.':
                print c
                ret_string+=c
        if ret_string:
            return float(ret_string)
        else:
            return 0.0 
    
#参数为：foldername：数据源文件的文件夹名,filename：数据文件名,resultfilename：存放结果数据的文件名
#提取吞入量数据
def MainExtract_THRUPUT(foldername,filename,resultfilename,func):
    try:
        log_file=os.path.join(foldername,filename)
        fdest=open(resultfilename,'a')
        if os.path.exists(log_file)==False:
            fdest.write('None\t')
            return
        fsrc=open(log_file,'r')        
        strtxt=fsrc.read()
        if strtxt!='':
            avgtime=1.0
            str2pro=''
            strList=re.split('=*',strtxt)
            for elem in strList:
                if elem.find('Using thread pool threads')!=-1:
                    str2pro=elem
                if elem.find('Function response time statistics')!=-1:
                    tmpList=elem.split('\n')
                    for tmpelem in tmpList:
                        if tmpelem.find(func)!=-1:
                            avgtime=float(re.split(' *',tmpelem)[-1].replace(',','')) 
            if str2pro!='':
                strList2=str2pro.split('\n')
                for elem2 in strList2:
                    if elem2.find('Using thread pool threads')!=-1:
                        searObj=re.search('[0-9]+',elem2)
                        if searObj:
                            fdest.write(str('%.4f'%(float(searObj.group())/avgtime*1000))+'\t')
                        else:
                            fdest.write('None\t')
                    else:
                        pass
            else:
                fdest.write('None\t')
        else:
            fdest.write('None\t')            
    finally:
        fsrc.close()
        fdest.close()

#提取响应时间数据（Max、Min、Avg）
def MainExtract_COST(foldername,filename,resultfilename,func):    
    try:
        fsrc=open(os.path.join(foldername,filename),'r')
        
        fdest=open(resultfilename,'a')
        strTXT=fsrc.read()
        if strTXT!='':
            strList=re.split('=*',strTXT)
            for elem in strList:
                if elem.find('Function response time statistics')!=-1:
                    strList2=elem.split('\n')
                    for elem2 in strList2:
                        if elem2.find(func)!=-1:
                            tmplist=re.split(' *',elem2)
                            if len(tmplist)>=3:
                                fdest.write(str('%.2f'%float(tmplist[-2].replace(',','')))+'\t')
                                fdest.write(str('%.2f'%float(tmplist[-3].replace(',','')))+'\t')
                                fdest.write(str('%.2f'%float(tmplist[-1].replace(',','')))+'\t')
                            else:
                                fdest.write('None\tNone\tNone\t') 
                        else:
                            pass
                else:
                    pass
        else:
            fdest.write('None\tNone\tNone\t') 

        
    finally:
        fsrc.close()
        fdest.close()
        
#提取响应时间区间数据
def MainExtract_COSTSPAN(sections,functions,foldername,filename,resultfilename):
    try:
        fsrc=open(os.path.join(foldername,filename),'r')
        fdest=open(resultfilename,'a')
        strTXT=fsrc.read()
        if strTXT!='':
            strList=re.split('=*',strTXT)
            for elem in strList:
                if elem.find('Function response time distributions')!=-1:
                    strList2=elem.split('\n')
                    time_list=[]
                    all_func_list=[]
                    for elem2 in strList2:
                        if elem2.find('Proc')!=-1:
                            time_list=[x for x in re.split(' *',elem2) if len(x)>0]
                            del time_list[0]
                        
                        for func in functions:                        
                            if elem2.find(func.strip())!=-1:
                                func_list=[y for y in re.split(' *',elem2) if len(y)>0]
                                del func_list[0]
                                all_func_list.append(func_list)
                    map_list=[]
                    for one_func in all_func_list:
                        one_dict={}
                        for i in range(0,len(time_list)):
                            one_dict[time_list[i]]=one_func[i]
                        map_list.append(one_dict)
                    #自此获取到所有接口数据
                    
                    for one_section in sections:
                        if isinstance(one_section,RegionValue):                          
                           
                            for one_map in map_list:
                                print one_section
                                func_call=0
                                all_call=0
                                for key,value in one_map.items():
                                    print 'key=',key,'value=',value,'\n'
                                    all_call+=int(value)
                                    if one_section.between(float(key)):
                                        func_call+=int(value)
                                #print 'one ok!'
                                if all_call!=0:
                                    fdest.write(str('%.2f'%float(func_call*1.0/all_call*100))+'%\t')
                                else:
                                    fdest.write('None\t')                        
            
    finally:
        fsrc.close()
        fdest.close()
        
    
#提取响应时间区间数据
def MainExtract_COSTSPAN_bak(foldername,filename,resultfilename):
    Num_lt200=0
    Num_200to500=0
    Num_500to1000=0
    Num_gt1000=0
    Num_all=0
    try:
        fsrc=open(os.path.join(foldername,filename),'r')
        fdest=open(resultfilename,'a')
        strTXT=fsrc.read()
        if strTXT!='':
            strList=re.split('=*',strTXT)
            for elem in strList:
                if elem.find('Function response time distributions')!=-1:
                    strList2=elem.split('\n')
                    for elem2 in strList2:
                        if elem2.find('NLICInterpret')!=-1:
                            tmplist=re.split(' *',elem2)
                            Num_lt200=int(tmplist[-31])+int(tmplist[-30])
                            Num_200to500=int(tmplist[-29])+int(tmplist[-28])+int(tmplist[-27])
                            Num_500to1000=int(tmplist[-26])+int(tmplist[-25])+int(tmplist[-24])+int(tmplist[-23])+int(tmplist[-22])
                            for i in range(-21,0):
                                Num_gt1000+=int(tmplist[i])
                            for i in range(-31,0):
                                Num_all+=int(tmplist[i])
                            if Num_all!=0:
                                fdest.write(str('%.2f'%float(Num_lt200/Num_all*100))+'%\t')
                                fdest.write(str('%.2f'%float(Num_200to500/Num_all*100))+'%\t')
                                fdest.write(str('%.2f'%float(Num_500to1000/Num_all*100))+'%\t')
                                fdest.write(str('%.2f'%float(Num_gt1000/Num_all*100))+'%\t')
                            else:
                                fdest.write('0.000%\t'*4)
                        else:
                            pass
                else:
                    pass
        else:
            fdest.write('None\tNone\tNone\tNone\t') 
    finally:
        fsrc.close()
        fdest.close()
        
#处理所有.csv文件，统计出来cpu占用和内存占用情况。注意计算公式为：AVG/1600和AVG/1024/1024，公式改动时，脚本也要相应改动
def MainExtract_CPU_Memory(cpu_index,physicalMem_index,virtualMem_index,cpu_core,start,end,foldername,filename,resultfilename):

    foldername=foldername.replace('/','\\')
    fdest=open(resultfilename,'a')
    log_file=os.path.join(foldername,filename)
    fi_list=glob.glob(log_file)
    if len(fi_list)!=1:
        fdest.write('None\tNone\tNone\t')
        return
    log_file=fi_list[0]
    if os.path.exists(log_file)==False:            
        fdest.write('None\tNone\tNone\t')
        return
         
    fsrc=open(log_file,'r')
    
    strTXT=fsrc.read()
    if strTXT!='':
        
        strList=strTXT.split('\n')
        print 'len',len(strList),'\n'
        start=int(start)
        end=int(end)
        if len(strList)>=start:
            del strList[0:start]
            print 'lenA',len(strList),'\n'
        if len(strList)>=end:
            del strList[len(strList)-end:len(strList)]
            print 'lenB',len(strList),'\n'
        cpuSum=0.0
        cpuCount=0
        physicalMemSum=0.0#统计物理内存
        physicalMemCount=0
        virtualMemSum=0.0#统计虚拟内存
        virtualMemCount=0        
    
        for elem in strList:
            #tmplist=elem.split(',')
            tmplist=LineSplit(elem)
            if cpu_index>=0:
                if len(tmplist)>=cpu_index:
                    tmpStr=tmplist[cpu_index-1].replace('"','')#！！！注意读入的数字字符串不仅仅只有数字，还有双引号，要先替换掉。
                    tmpStr=tmpStr.strip()
                    #if re.match(r'^\d\d*.\d*\d$',tmpStr): 
                    if tmpStr[-1]=='g':
                        cpuSum+=Str2Float(tmpStr)*1000
                    else:
                        cpuSum+=Str2Float(tmpStr)                        
                    cpuCount+=1
                else:
                    pass
            if physicalMem_index>=0:
                if len(tmplist)>=physicalMem_index:
                    tmpStr=tmplist[physicalMem_index-1].replace('"','')
                    tmpStr=tmpStr.strip()
                    #if re.match(r'^\d\d*\d$',tmpStr):#以数字开头，以数字结尾 
                    if tmpStr[-1]=='g':
                        physicalMemSum+=Str2Float(tmpStr)*1000
                    else:
                        physicalMemSum+=Str2Float(tmpStr)                    
                   
                    physicalMemCount+=1
                else:                    
                    pass 
                
            if virtualMem_index>=0:
                if len(tmplist)>=virtualMem_index:
                    tmpStr=tmplist[virtualMem_index-1].replace('"','')
                    tmpstr=tmpStr.strip()
                    if tmpStr[-1]=='g':
                        virtualMemSum+=Str2Float(tmpStr)*1000
                    else:
                        virtualMemSum+=Str2Float(tmpStr)
                    #if re.match(r'^\d\d*\d$',tmpStr):#以数字开头，以数字结尾                      
                   
                    virtualMemCount+=1
                else:                    
                    pass 
          
        if cpu_index>=0:
            if cpuCount!=0:
                fdest.write(str('%.4f'%float(cpuSum/cpuCount/float(cpu_core)))+'\t')
            else:
                fdest.write('None\t')
        else:
            fdest.write('None\t')
        
        if physicalMem_index>=0:
            if physicalMemCount!=0:
                fdest.write(str('%.4f'%float(physicalMemSum/physicalMemCount))+'\t')
            else:
                fdest.write('None\t')
        else:
            fdest.write('None\t')
            
        if virtualMem_index>=0:
            if virtualMemCount!=0:
                fdest.write(str('%.4f'%float(virtualMemSum/virtualMemCount))+'\t')
            else:
                fdest.write('None\t')
        else:
            fdest.write('None\t')
    else:
        fdest.write('None\tNone\tNone\t')
    fsrc.close()
    fdest.close()  
                      
#自己写个列表排序函数，按列表中元素字符串的数字从小到大排序
def SortList(onelist):
    tmplist=[(int(elem.split('_')[-1].split('.')[0].split('mtresp')[0]),elem)for elem in onelist]#搞了很久才搞明白，要保证后面排序函数中的key项为整数才能得到正确结果。
    tmplist=sorted(tmplist,key=lambda x:x[0])
    tmplist=[elem[1] for elem in tmplist]
    return tmplist

#两种排版模式都可用但只能同时使用一种，使用时要注释掉另外一种。生成结果为，每个目录对应一个txt文件
#m_section是统计的分布区间,m_function是要统计的接口函数
def getResultTXT(m_ini):
    if isinstance(m_ini,ini_data):        
        curpathname=os.getcwd()#得到python文件当前目录
        #print (curpathname)
        folder_path=m_ini.common_log_folder
        if folder_path[-1]=='\\':
            del folder_path[-1]
            
        folderList=getDirList(folder_path)#得到当前目录下的所有文件夹名
        for folderElem in folderList:#对当前目录的每一个文件夹 
            temp_file=os.path.join(folder_path,folderElem)        
            fileList=getFileList(temp_file)#得到其文件列表,有可能没有排序
            fileList=[elem for elem in fileList if elem.split('.')[-1]=='log']#只要log文件作为列表即可
            fileList=SortList(fileList)
            #print ('fileList=',fileList)
            #fileList=['1.perf.txt','4.perf.txt','8.perf.txt','12.perf.txt','16.perf.txt','20.perf.txt','24.perf.txt','28.perf.txt']#手动指定
            destfilename=folderElem+'_result.txt'
            
            for fileElem in fileList:
                BingFaShu=fileElem.split('.')[0].split('mtresp')[0]#从文件名提取并发路数,这里改动时注意修改上面的SortList函数中的排序规则。
                temp_name=str.format('{0}\t',BingFaShu)
                FileWrite(temp_name,destfilename,'a')#换行生成另外一个数据
                temp_you_folder=os.path.join(folder_path,folderElem)
                MainExtract_THRUPUT(temp_you_folder,fileElem,destfilename,m_ini.log_function[0])#吞吐量            
                iss_csv=BingFaShu.replace('_',str.format('*_'))+'line.csv' 
                
                MainExtract_CPU_Memory(m_ini.csv_cpuIndex,m_ini.csv_physicalMemIndex,m_ini.csv_virtualMemIndex,m_ini.csv_cpuCore,m_ini.csv_startsubLines,m_ini.csv_endsubLines,temp_you_folder,iss_csv,destfilename)#CPU占用和内存占用
                MainExtract_COST(temp_you_folder,fileElem,destfilename,m_ini.log_function[0])#统计指定接口响应时间MAX、MIN、AVG
                
                MainExtract_COSTSPAN(m_ini.log_section,m_ini.log_function,temp_you_folder,fileElem,destfilename)#响应时间区间分布
                FileWrite('\n',destfilename,'a')#换行生成另外一个数据      
        print ('All data file has been done,Result in TXT files!')


  
######################### 此块代码功能是生成各个厂商的数据表（表头和txt文件中的数据），并整合到一个excel文件中。 ############################
    
#往excel中写入表格头，并把txtfilename中的数据写入sheet中。xlsName:excel文件名，sheetname：xlsName文件中的表单名txtfilename
def geneExcelTableHead(xlsName,sheetname,txtfilename,m_ini):
    ###生成表头###
    cellstyle1 = easyxf('font: name Times New Roman, color_index black;'\
                         'pattern:pattern solid,fore_colour sky_blue;'\
                         'borders: left thin, right thin, top thin, bottom thin;'\
                         'alignment:horz center,vert center')
    cellstyle2 = easyxf('font: name Times New Roman, color-index black;'\
                         #'pattern:pattern solid,fore_colour sky_blue;'\
                         'borders: left thin, right thin, top thin, bottom thin;'\
                         'alignment:horz center,vert center')
    rb=open_workbook(xlsName,formatting_info=True)#一定要设置formatting_info=True，否则修改后再保存将没有style，注意！！！
    rs=rb.sheet_by_name(sheetname)
    rows=rs.nrows
    wb=copy(rb)
    try:
        for index in range(rb.nsheets):
            if rb.sheet_by_index(index).name==sheetname:
                ws=wb.get_sheet(index)
    except:
        print('not found sheet named:'+sheetname)
        ws=wb.get_sheet(0)
    ws.write(rows,0,txtfilename.split('-')[0])#在第一行写入厂商名，如果不需要可以注释掉。
    rows=rows+1#从厂商名下一行开始写。
    ws.write_merge(rows,rows+1,0,0,'并发路数'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows+1,1,1,'吞吐量[(Sent)/sec]'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows+1,2,2,'CPU占用'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows+1,3,3,'物理内存占用'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows+1,4,4,'虚拟内存占用'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows,5,7,'响应时间[ms]'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows,8,11,'响应时间分布'.decode('gbk'),cellstyle1) 
    ws.write(rows+1,5,'最大响应时间'.decode('gbk'),cellstyle1)
    ws.write(rows+1,6,'最小响应时间'.decode('gbk'),cellstyle1)
    ws.write(rows+1,7,'平均响应时间'.decode('gbk'),cellstyle1)
    if isinstance(m_ini,ini_data):
        index=7
        for one_sec in m_ini.log_section:
            index+=1
            ws.write(rows+1,index,one_sec.__str__().decode('gbk'),cellstyle1)
            
    #ws.write(rows+1,7,'( 0, 0.2]'.decode('gbk'),cellstyle1)
    #ws.write(rows+1,8,'( 0.2, 0.5]'.decode('gbk'),cellstyle1)
    #ws.write(rows+1,9,'( 0.5, 1.0]'.decode('gbk'),cellstyle1)
    #ws.write(rows+1,10,'( 1.0, ...]'.decode('gbk'),cellstyle1)   
    ws.flush_row_data() 
    ###表头生成结束###   
    ###从txt文件导入数据###
    fop=open(txtfilename)
    strlist=fop.read().rstrip().split('\n')
    for i in range(len(strlist)):
        tmplist=strlist[i].rstrip().split('\t')
        for j in range(len(tmplist)):
            ws.write(rows+2+i,j,tmplist[j].decode('gbk'),cellstyle2)
    fop.close()
    ###导入数据结束###
    ws.write(rows+len(tmplist),0) 
    wb.save(xlsName)
    
import types
class Sections(list):
    pass


    
    
    
class  RegionValue():    
        
    def __init__(self,m_str):
        if m_str==None or len(m_str)==0:
            self.LeftRegionType=RegionEnum.LeftClose
            self.RightRegionType=RegionEnum.RightClose
            self.LeftRegionValue=0
            self.RightRegionValue=0
        else:
            m_str=str(m_str).strip()
            self.LeftRegionType=self.str2regionEnum(m_str[0])
            self.RightRegionType=self.str2regionEnum(m_str[-1])
            pos=m_str.find(',')
            self.LeftRegionValue=float((m_str[1:pos]).strip())        
            self.RightRegionValue=float((m_str[pos+1:-1]).strip()) 
        
    def between(self,value):
        leftIn=False
        rightIn=False
        
        if self.LeftRegionType==RegionEnum.LeftClose:
            if value >=self.LeftRegionValue:
                leftIn=True
        elif self.LeftRegionType==RegionEnum.LeftOpen:
            if value>self.LeftRegionValue:
                leftIn=True
        elif self.LeftRegionType==RegionEnum.LeftInfinite:
            leftIn=True
            
        if self.RightRegionType==RegionEnum.RightClose:
            if value<=self.RightRegionValue:
                rightIn=True
            
        elif self.RightRegionType==RegionEnum.RightOpen:
            if value<self.RightRegionValue:
                rightIn=True
        elif self.RightRegionType==RegionEnum.RightInfinite:
            rightIn=True
            
        if leftIn==True and rightIn==True:
            return True
        return False
    def __str__(self):
        str_content=str()
        if self.LeftRegionType==RegionEnum.LeftClose:
            str_content+=str.format('[{0},',self.LeftRegionValue)
        elif self.LeftRegionType==RegionEnum.LeftOpen:
            str_content+=str.format('({0},',self.LeftRegionValue)
        elif self.LeftRegionType==RegionEnum.LeftInfinite:
            str_content+=str.format('(-∞,')
            
        if self.RightRegionType==RegionEnum.RightClose:
            str_content+=str.format('{0}]',self.RightRegionValue)
        elif self.RightRegionType==RegionEnum.RightOpen:
            str_content+=str.format('{0})',self.RightRegionValue)
        elif self.RightRegionType==RegionEnum.RightInfinite:
            str_content+=str.format('+∞)') 
            
        return str_content        
        
    def equal(self,obj):
        return self==obj
        
        
         
    def str2regionEnum(self,m_str):
        if m_str=='(':
            return RegionEnum.LeftOpen
        elif m_str=='[':
            return RegionEnum.LeftClose
        elif m_str==')':
            return RegionEnum.RightOpen
        elif m_str==']':
            return RegionEnum.RightClose
        else:
            return RegionEnum.Other
   
    def __eq__(self,other):
        if isinstance(other,RegionValue):            
            return self.LeftRegionValue==other.LeftRegionValue and self.RightRegionValue==other.RightRegionValue and \
                   self.LeftRegionType==other.LeftRegionType and self.RightRegionType==other.RightRegionType                 
                                  
        return False
    def hello(self):
        print self.LeftRegionType,self.LeftRegionValue,self.LeftRegionValue,self.RightRegionType
        print'\n'

class RegionEnum:
    def __init__(self):
        pass
     #左括号闭区间
    LeftClose=0
    #右括号闭区间
    RightClose=1
    #左括号开区间
    LeftOpen=2
    #右括号开区间
    RightOpen=3
    #左区间无穷大
    LeftInfinite=4
    #右区间无穷大
    RightInfinite=5
    Other=6
    
    
    
        
#读取ini配置文件
import ConfigParser
class ini_data:   
    
    def open(self,ini_file):
        ini_parse=ConfigParser.ConfigParser()
        ini_parse.read(ini_file)
        self.common_log_folder=ini_parse.get('common_set','Log_Folder')
        self.log_section=[RegionValue(x) for x in  ini_parse.get('log_set','section').split('|') if len(x)>0]
        self.log_function=[func for func in ini_parse.get('log_set','function').split('|') if len(func)>0]
        self.csv_startsubLines=ini_parse.get('csv_set','StartSubLines')
        self.csv_endsubLines=ini_parse.get('csv_set','EndSubLines')
        self.csv_cpuIndex=int(ini_parse.get('csv_set','CPU_Column'))        
        self.csv_physicalMemIndex=int(ini_parse.get('csv_set','PhysicalMemory_Column'))
        self.csv_virtualMemIndex=int(ini_parse.get('csv_set','VirtualMemory_Column'))
        self.csv_cpuCore=int(ini_parse.get('csv_set','Cpu_Core'))
    def test_read(self):
        print 'common_log_folder=%s' %(self.common_log_folder)
        print 'csv_startsubLines=%s' %(self.csv_startsubLines)
        print 'csv_endsubLines=%s' %(self.csv_endsubLines)
        print 'csv_cpuIndex=%s' %(self.csv_cpuIndex)       
        print 'csv_physicalMemIndex=%s' %(self.csv_physicalMemIndex)
        print 'csv_virtualMemIndex=%s' %(self.csv_virtualMemIndex)


if __name__=="__main__":
    #先创建一个xls文件，再修改。
    try:
        ini=ini_data()
        ini.open('config.ini')
        ini.test_read()     
       
        wb = Workbook()
        curpathname=os.getcwd()#得到python文件当前目录
        folderList=getDirList(ini.common_log_folder)#得到当前目录下的所有文件夹名
        for folderElem in folderList:
            sheetname=folderElem#以folder name作为sheet name
            ws = wb.add_sheet(sheetname,cell_overwrite_ok=False)
            
        ws = wb.add_sheet('All Result')#保存所有厂商的数据表
        wb.save('Perf_Result.xls')
        
        getResultTXT(ini)#产生txt形式的结果文件
        for folderElem in folderList:#对当前目录的每一个文件夹      
            geneExcelTableHead ('Perf_Result.xls',folderElem,folderElem+r'_result.txt',ini)#生成到多个sheet中
            #下面是生成所有厂商数据表到单个sheet（即上面260加入的sheet：'All Result'）中，如果需要可以和上一行代码同时存在。
            geneExcelTableHead ('Perf_Result.xls','All Result',folderElem+r'_result.txt',ini)
            #处理完成，删除txt文件（可选）
            os.remove(r'./'+folderElem+r'_result.txt')
        print ('All task finish!')
        print ('Press any key to exit!')
    except Exception,ex:
        print 'Error:\n',ex
    finally:    
        raw_input()

"""
######################################## 这里注释的代码功能是对每个厂商生成一个excel ########################################

#往excel中写入表格头，并把txtfilename中的数据写入各个xls工作薄中。xlsName:excel文件名，sheetname：xlsName文件中的表单名txtfilename
def geneExcelTableHead2(xlsName,sheetname,txtfilename):
    cellstyle1 = easyxf('font: name Times New Roman, color_index black;'\
                         'pattern:pattern solid,fore_colour sky_blue;'\
                         'borders: left thin, right thin, top thin, bottom thin;'\
                         'alignment:horz center,vert center')
    cellstyle2 = easyxf('font: name Times New Roman, color-index black;'\
                         #'pattern:pattern solid,fore_colour sky_blue;'\
                         'borders: left thin, right thin, top thin, bottom thin;'\
                         'alignment:horz center,vert center')
    #rb=open_workbook(xlsName)#可以不设置formatting_info=True，因为表里还没有数据。
    #rs=rb.sheet_by_name(sheetname)
    #rows=rs.nrows
    #wb=copy(rb)
    rows=0
    wb=Workbook()
    ws=wb.add_sheet(sheetname)
    ws.write_merge(rows,rows+1,0,0,'并发路数'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows+1,1,1,'吞吐量[(Sent)/sec]'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows+1,2,2,'CPU占用'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows+1,3,3,'内存占用'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows,4,6,'响应时间[ms]'.decode('gbk'),cellstyle1)
    ws.write_merge(rows,rows,7,10,'响应时间分布'.decode('gbk'),cellstyle1) 
    ws.write(rows+1,4,'最大响应时间'.decode('gbk'),cellstyle1)
    ws.write(rows+1,5,'最小响应时间'.decode('gbk'),cellstyle1)
    ws.write(rows+1,6,'平均响应时间'.decode('gbk'),cellstyle1)
    ws.write(rows+1,7,'( 000, 200]'.decode('gbk'),cellstyle1)
    ws.write(rows+1,8,'( 200, 500]'.decode('gbk'),cellstyle1)
    ws.write(rows+1,9,'( 500, 1000]'.decode('gbk'),cellstyle1)
    ws.write(rows+1,10,'( 1000, ...]'.decode('gbk'),cellstyle1)   
    ws.flush_row_data()   
    ###从txt文件导入数据###
    fop=open(txtfilename)
    strlist=fop.read().rstrip().split('\n')
    for i in range(len(strlist)):
        tmplist=strlist[i].rstrip().split('\t')
        for j in range(len(tmplist)):
            ws.write(rows+2+i,j,tmplist[j].decode('gbk'),cellstyle2)
    fop.close()  
    ###导入数据结束###
    wb.save(xlsName)

    
if __name__=="__main__":
    curpathname=os.getcwd()#得到python文件当前目录
    folderList=getDirList(curpathname)#得到当前目录下的所有文件夹名
    getResultTXT()#产生txt形式的结果文件
    for folderElem in folderList:#对当前目录的每一个文件夹      
        geneExcelTableHead2(folderElem+r'.xls',folderElem,folderElem+r'-result.txt')#分别生成每个excel再合并到一个中
        #处理完成，删除txt文件（可选）
        os.remove(r'./'+folderElem+r'-result.txt')
    print ('All has been done!')
"""
