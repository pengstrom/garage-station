
#if !defined(_LINK_H)
#define _LINK_H

#include <sx1278.h>

class Transport
{
public:
  enum class command_t : uint8_t
  {
    ACK,
    NACK,
    PING,
    PONG,
  };
  static const size_t PACKET_SIZE = 64;
  static const size_t PACKET_DATA_LENGTH = PACKET_SIZE - sizeof(command_t) - 2 * sizeof(uint8_t);
  static const size_t PACKET_QUEUE_LENGTH = 32;

  struct packet_t
  {
    uint8_t data[PACKET_DATA_LENGTH];
    command_t cmd;
    uint8_t len;
    uint8_t seq;

    packet_t() {}
    packet_t(uint8_t bytes[PACKET_SIZE])
    {
      for (size_t i = 0; i < PACKET_DATA_LENGTH; i++)
      {
        data[i] = bytes[i];
      }
      cmd = static_cast<command_t>(bytes[PACKET_SIZE - 3]);
      len = bytes[PACKET_SIZE - 2];
      seq = bytes[PACKET_SIZE - 1];
    }

    void toBytes(uint8_t bytes[PACKET_SIZE])
    {
      for (size_t i = 0; i < PACKET_DATA_LENGTH; i++)
      {
        bytes[i] = data[i];
      }
      bytes[PACKET_SIZE - 3] = static_cast<uint8_t>(cmd);
      bytes[PACKET_SIZE - 2] = len;
      bytes[PACKET_SIZE - 1] = seq;
    }
  };

  struct msg_t
  {
    uint8_t seq;
    command_t cmd;

    msg_t(uint8_t _seq, command_t _cmd)
    {
      seq = _seq;
      cmd = _cmd;
    }
    msg_t(packet_t pkt)
    {
      seq = pkt.seq;
      cmd = pkt.cmd;
      fromData(pkt.data, pkt.len);
    }
    virtual void fromData(uint8_t data[PACKET_DATA_LENGTH], uint8_t len) = 0;
    virtual uint8_t toData(uint8_t data[PACKET_DATA_LENGTH]) = 0;

    packet_t toPacket()
    {
      packet_t pkt;
      pkt.seq = seq;
      pkt.cmd = cmd;
      uint8_t len = toData(pkt.data);
      pkt.len = len;
      return pkt;
    }

    void fromPacket(packet_t pkt)
    {
      seq = pkt.seq;
      cmd = pkt.cmd;
      fromData(pkt.data, pkt.len);
    }
  };

  struct msg_ack_t : msg_t
  {
    msg_ack_t(uint8_t seq) : msg_t(seq, command_t::ACK) {}
    void fromData(uint8_t *data) {}
    uint8_t toData(uint8_t *data) { return 0; }
  };
  struct msg_nack_t : msg_t
  {
    msg_nack_t(uint8_t seq) : msg_t(seq, command_t::NACK) {}
    void fromData(uint8_t *data) {}
    uint8_t toData(uint8_t *data) { return 0; }
  };
  struct msg_ping_t : msg_t
  {
  };

  Transport(Sx1278::config_t cfg);
  ~Transport();

  void send(packet_t pkt);

  Sx1278 *_lora;

private:
  Sx1278::config_t _cfg;
  QueueHandle_t _packet_queue;

  static void rx_task(void *arg, uint8_t *buffer, size_t size);
  void handleRx(uint8_t *buffer, size_t size);

  static void packet_task(void *self);
  void packetHandler();

  // Enforce sanity
  static_assert(sizeof(packet_t) == PACKET_SIZE);
};

#endif // _LINK_H