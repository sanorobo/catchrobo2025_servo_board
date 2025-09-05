#include <cstdio>

#include <halx/core.hpp>
#include <halx/driver/c6x0.hpp>
#include <halx/peripheral.hpp>

extern UART_HandleTypeDef huart3;

extern FDCAN_HandleTypeDef hfdcan1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

static uint8_t uart3_tx_buf[512];
static uint8_t uart3_rx_buf[512];

extern "C" void main_thread(void *) {
  using namespace halx::peripheral;

  HAL_UART_DeInit(&huart3);

  huart3.Init.BaudRate = 115200;

  HAL_UART_Init(&huart3);

  Uart<&huart3, UartTxDma, UartRxDma> uart3{uart3_tx_buf, uart3_rx_buf}; // stlink

  Can<&hfdcan1> can1;

  // Pwm pwm3_ch3{&htim4, TIM_CHANNEL_3}; // servo
  // Pwm pwm3_ch4{&htim4, TIM_CHANNEL_4}; // servo

  Gpio led_r{GPIOA, GPIO_PIN_6};
  Gpio led_g{GPIOA, GPIO_PIN_7};
  Gpio led_b{GPIOA, GPIO_PIN_5};

  Tim<&htim16> tim16; // 1kHz
  Tim<&htim17> tim17; // 10kHz

  enable_stdout(uart3);

  // ここより上はbaud rate以外触らない

  using namespace halx::driver;

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
