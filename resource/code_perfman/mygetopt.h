/** 
 * @file	mygetopt.h
 * @brief	对GNU的getopt做了再包装
 * 
 *  请将getopt.h优化处理，放入这个文件中.
 * 
 * @author	boxu
 * @version	1.0
 * @date	2010年11月18日
 * 
 * @see		
 * 
 * @par 版本记录：
 * <table border=1>
 *  <tr> <th>版本	<th>日期			<th>作者	<th>备注 </tr>
 *  <tr> <td>1.0	<td>2010年11月18日	<td>boxu	<td>创建 </tr>
 * </table>
 */
#ifndef __MY_GET_OPT_H__
#define __MY_GET_OPT_H__

#include "getopt.h"

#define MY_DISABLE_DEFAULT_UNKNOWN_OPTS_ERR()  {opterr = 0;}

#ifdef	__cplusplus
extern "C" {
#endif


#ifdef	__cplusplus
}
#endif

#endif /* __MY_GET_OPT_H__ */