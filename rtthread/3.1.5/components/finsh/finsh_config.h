/* FinSH config file */

#ifndef __MSH_CFG_H__
#define __MSH_CFG_H__

// <<< Use Configuration Wizard in Context Menu >>>
#define RT_USING_FINSH          //使能 FinSH
#define FINSH_USING_MSH         //使能 msh 模式
#define FINSH_USING_MSH_ONLY    //只使用 msh 模式

#define FINSH_USING_HISTORY     //打开历史回溯功能
// #define FINSH_USING_AUTH        //使能权限验证

// <h>FinSH Configuration
// <o>the priority of finsh thread <1-30>
//  <i>the priority of finsh thread
//  <i>Default: 21
#define FINSH_THREAD_PRIORITY       21
// <o>the stack of finsh thread <1-4096>
//  <i>the stack of finsh thread
//  <i>Default: 4096  (4096Byte)
#define FINSH_THREAD_STACK_SIZE     1024

#define FINSH_USING_SYMTAB
// <c1>Enable command description
//  <i>Enable command description
#define FINSH_USING_DESCRIPTION
//  </c>
// </h>

// <<< end of configuration section >>>
#endif

