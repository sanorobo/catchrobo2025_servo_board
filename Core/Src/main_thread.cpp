#include <cstdio>

#include <halx/core.hpp>
#include <halx/driver/c6x0.hpp>
#include <halx/peripheral.hpp>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart5;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
extern FDCAN_HandleTypeDef hfdcan3;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

template <UART_HandleTypeDef *Handle> auto uart_init(uint32_t baud_rate) {
  static uint8_t tx_buf[512];
  static uint8_t rx_buf[512];

  HAL_UART_DeInit(Handle);
  Handle->Init.BaudRate = baud_rate;
  HAL_UART_Init(Handle);

  return halx::peripheral::Uart<Handle, halx::peripheral::UartTxDma, halx::peripheral::UartRxDma>{tx_buf, rx_buf};
}

extern "C" void main_thread(void *) {
  using namespace halx::peripheral;
  using namespace halx::driver;

  auto uart1 = uart_init<&huart1>(1000000); // serial servo
  auto uart2 = uart_init<&huart2>(115200);  // rs485
  auto uart3 = uart_init<&huart3>(115200);  // stlink
  auto uart5 = uart_init<&huart5>(115200);  // uart / i2c

  Can<&hfdcan1> can1;
  Can<&hfdcan2> can2;
  Can<&hfdcan3> can3;

  Tim<&htim3> tim3;   // servo 1 / 2
  Tim<&htim16> tim16; // 1kHz
  Tim<&htim17> tim17; // 10kHz

  enable_stdout(uart3);

  // ここより上はbaud rate以外触らない

  C6x0Manager c6x0_manager{can1};
  C6x0 c6x0{c6x0_manager, C6x0Type::C610, C6x0Id::ID_1};

  can1.start();

  while (true) {
    c6x0_manager.update();

    printf("%d\r\n", c6x0.get_rpm());

    c6x0.set_current_ref(400.0f);

    c6x0_manager.transmit();

    halx::core::delay(10);
  }
}
