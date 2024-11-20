#include "transport.h"
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

constexpr Sx1278::config_t SX1278_CONFIG = {
    .dio0_pin = GPIO_NUM_17,
    .rst_pin = GPIO_NUM_16,
    .spi = {
        .host = SPI2_HOST,
        // .clock_freq = Sx1278::CLOCK_MAX_FREQ,
        .clock_freq = 9 * 1000 * 1000,
        .mosi_pin = GPIO_NUM_13,
        .miso_pin = GPIO_NUM_12,
        .nss_pin = GPIO_NUM_15,
        .sck_pin = GPIO_NUM_14,
    },
};

const Sx1278::bandwidth_t BANDWIDTH = Sx1278::bandwidth_t::KHZ_125;
const Sx1278::coding_rate_t CODING_RATE = Sx1278::coding_rate_t::CR_4_5;
const Sx1278::spreading_factor_t SPREADING_FACTOR = Sx1278::spreading_factor_t::SF_128;

Transport::Transport(Sx1278::config_t cfg) : _cfg(cfg)
{
  _packet_queue = xQueueCreate(32, sizeof(packet_t *));
  xTaskCreate(packet_task, "Packet Task", 4096, this, 15, NULL);

  _lora = new Sx1278(cfg);

  _lora->setMode(Sx1278::mode_t::SLEEP);

  // Enable LoRa
  Sx1278::reg_op_mode_t op_mode = _lora->readOpMode();
  op_mode.long_range_mode = true;
  _lora->writeOpMode(op_mode);

  auto cfg_1 = _lora->readCfg1();
  auto cfg_2 = _lora->readCfg2();
  auto cfg_3 = _lora->readCfg3();
  cfg_1.bw = BANDWIDTH;
  cfg_1.coding_rate = CODING_RATE;
  cfg_2.spreading_factor = SPREADING_FACTOR;
  cfg_2.rx_payload_crc = true;
  cfg_3.low_data_rate_optimize = true;

  _lora->writeCfg1(cfg_1);
  _lora->writeCfg2(cfg_2);
  _lora->writeCfg3(cfg_3);

  _lora->registerDio0Callback(rx_task, this);
  _lora->setDio0Mapping(Sx1278::dio0_t::RX_DONE);

  _lora->setMode(Sx1278::mode_t::RX_CONT);
}

Transport::~Transport()
{
  delete _lora;
}

void Transport::send(packet_t pkt)
{
}

void Transport::packetHandler()
{
  packet_t *pkt;
  xQueueReceive(_packet_queue, pkt, portMAX_DELAY); // Just stall an wait for item

  uint8_t seq = pkt->seq;
  command_t cmd = pkt->cmd;

  // if ack for awaited command
  // continue logic

  // if out of sequece
  // resend original command

  // if new command
  // continue logic

  delete pkt;
}

void Transport::rx_task(void *arg)
{
  Transport *self = static_cast<Transport *>(arg);
  self->handleRx();
}

void Transport::handleRx()
{
  Sx1278::reg_irq_flags_t flags = _lora->readIrqFlags();
  if (flags.rx_done && flags.valid_header && !flags.payload_crc_error)
  {
    uint8_t payload_size = _lora->rxSize();
    if (payload_size != PACKET_SIZE)
    {
      _lora->resetRx();
      return;
    }

    uint8_t buffer[PACKET_SIZE];
    _lora->readRx(buffer);
    packet_t *msg = new packet_t(buffer); // deleted in packet handler

    BaseType_t woken;
    // Drop packet in case of queue full
    xQueueSendToBackFromISR(_packet_queue, msg, &woken);
    if (woken == pdTRUE)
    {
      taskYIELD();
    }
  }

  _lora->resetRx();
}

void Transport::packet_task(void *arg)
{
  Transport *self = static_cast<Transport *>(arg);
  while (1)
  {
    self->packetHandler();
  }
}
