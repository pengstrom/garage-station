#include <transport.h>
#include <rom/ets_sys.h>
#include <config.h>

extern "C"
{
  extern void app_main()
  {
    vTaskDelay(pdMS_TO_TICKS(2000));
    Transport *link = new Transport(SX1278_CONFIG);
    auto lora = link->_lora;

    while (1)
    {
      vTaskDelay(100);
    }
  }
}
