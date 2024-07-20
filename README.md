# SimpleLogger

```cpp
#include "simplelogger.h"

SimpleLogger::Logger::Init(SimpleLogger::LOGMODE_CONSOLE | SimpleLogger::LOGMODE_FILE, "[MyPrefix]");

Logi << "Info Log";
// >> [MyPrefix][INF] Info Log
Logd << "Debug log";
// >> [MyPrefix][DBG] Debug log
Logw << "Warn log";
// >> [MyPrefix][WRN] Warn log
Loge << "Error Log" << Error;
// >> [MyPrefix][ERR] Error Log Error message
```
