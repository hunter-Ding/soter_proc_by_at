# soter_proc_by_at
该目录代码用于处理soter产线部署初级解决方案：
	通过Pandora.exe工具发送TA到ENGPC，由其解析后调用我们CA中的接口，从而将命令传递到TA中，完成相关功能。
	目前CA中定义的命令如下：
 31 const char SOTER_CMD[] = "AT+SOTER"; 
 32 const char SOTER_RSP[] = "SOTER CMD OK";  
 33 const char GENERATE_CMD[] = "AT+SOTER=1"; 
 34 const char GENERATE_RSP[] = "SOTER GENERATE ATTK OK";
 35 const char VERIFY_CMD[] = "AT+SOTER=2"; 
 36 const char VERIFY_RSP[] = "SOTER VERIFY ATTK OK";
 37 const char EXPORT_CMD[] = "AT+SOTER=3"; 
 38 const char GET_ID_CMD[] = "AT+SOTER=4"; 
 39 const char GET_ID_RSP[] = "SOTER GET DEVICE ID OK";
 40 const char SOTER_RSP_ERROR[] = "SOTER CMD FAILED"; 
 
 该目录下的源文件在android侧编译成libsotereng.so库文件，并由engpc程序动态加载到进程空间。