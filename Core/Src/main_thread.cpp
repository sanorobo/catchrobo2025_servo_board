#include <cstdio>

#include <halx/core.hpp>
#include <halx/driver/c6x0.hpp>
#include <halx/peripheral.hpp>

extern UART_HandleTypeDef huart1; // serial servo
extern UART_HandleTypeDef huart2; // rs485
extern UART_HandleTypeDef huart3; // stlink
extern UART_HandleTypeDef huart5;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
extern FDCAN_HandleTypeDef hfdcan3;

static uint8_t dma_buf[8192];

extern "C" void main_thread(void *) {
  using namespace halx::peripheral;
  using namespace halx::driver;

  huart1.Init.BaudRate = 1000000;
  huart2.Init.BaudRate = 115200;
  huart3.Init.BaudRate = 115200;
  huart5.Init.BaudRate = 115200;

  std::span dma_span{dma_buf};
  Uart<&huart1, UartTxDma, UartRxDma> uart1{dma_span.subspan<512 * 0, 512>(), dma_span.subspan<512 * 1, 512>()};
  Uart<&huart2, UartTxDma, UartRxDma> uart2{dma_span.subspan<512 * 2, 512>(), dma_span.subspan<512 * 3, 512>()};
  Uart<&huart3, UartTxDma, UartRxDma> uart3{dma_span.subspan<512 * 4, 512>(), dma_span.subspan<512 * 5, 512>()};
  Uart<&huart5, UartTxDma, UartRxDma> uart5{dma_span.subspan<512 * 6, 512>(), dma_span.subspan<512 * 7, 512>()};

  Can<&hfdcan1> can1;
  Can<&hfdcan2> can2;
  Can<&hfdcan3> can3;

  enable_stdout(uart3);

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
