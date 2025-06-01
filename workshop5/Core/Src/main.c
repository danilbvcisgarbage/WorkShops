/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Enhanced Music + LED Effects Controller
  * @description    : Plays musical melody with synchronized LED patterns
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    NOTE_C4, NOTE_C5, NOTE_D5, NOTE_E5,
    NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5,
    NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6,
    NOTE_G6, NOTE_A6, NOTE_B6, NOTES_MAX
} MusicNoteType;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CS43L22_I2C_ADDRESS     0x94
#define I2C_COMMUNICATION_TIMEOUT    10

// LED Pin Definitions
#define LED_GREEN_PIN           GPIO_PIN_12
#define LED_ORANGE_PIN          GPIO_PIN_13
#define LED_RED_PIN             GPIO_PIN_14
#define LED_BLUE_PIN            GPIO_PIN_15
#define ALL_LEDS_MASK          (LED_GREEN_PIN | LED_ORANGE_PIN | LED_RED_PIN | LED_BLUE_PIN)
#define AUDIO_ENABLE_PIN        GPIO_PIN_4
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;

/* USER CODE BEGIN PV */
int16_t audioBuffer[100] = {0};

// Musical timing constants
const uint32_t FULL_NOTE_DURATION = 214;
const uint32_t HALF_NOTE_DURATION = FULL_NOTE_DURATION >> 1;
const uint32_t QUARTER_NOTE_DURATION = FULL_NOTE_DURATION >> 2;

// Playback control variables
uint8_t melodyPlayCount = 0;
const uint8_t MAX_MELODY_PLAYS = 3;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);

/* USER CODE BEGIN PFP */
void AudioCodec_Initialize(void);
void AudioCodec_PlayTone(MusicNoteType note, uint32_t durationMs);
void LED_TurnOn(uint16_t ledPin);
void LED_TurnOff(uint16_t ledPin);
void LED_TurnOnAll(void);
void LED_TurnOffAll(void);
void PlayMelodyWithLEDs(void);
void RunLEDOnlyMode(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/**
 * @brief Initialize CS43L22 Audio Codec
 */
void AudioCodec_Initialize(void) {
    // Enable audio codec chip
    HAL_GPIO_WritePin(GPIOD, AUDIO_ENABLE_PIN, GPIO_PIN_SET);

    uint8_t configBuffer[2];

    // Audio codec initialization sequence
    configBuffer[0] = 0x0D; configBuffer[1] = 0x01;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x00; configBuffer[1] = 0x99;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x47; configBuffer[1] = 0x80;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x32; configBuffer[1] = 0xFF;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x32; configBuffer[1] = 0x7F;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x00; configBuffer[1] = 0x00;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x04; configBuffer[1] = 0xAF;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x0D; configBuffer[1] = 0x70;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x05; configBuffer[1] = 0x81;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x06; configBuffer[1] = 0x07;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x0A; configBuffer[1] = 0x00;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x27; configBuffer[1] = 0x00;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x1A; configBuffer[1] = 0x0A;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x1B; configBuffer[1] = 0x0A;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x1F; configBuffer[1] = 0x0F;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    configBuffer[0] = 0x02; configBuffer[1] = 0x9E;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, configBuffer, 2, I2C_COMMUNICATION_TIMEOUT);
}

/**
 * @brief Play a musical tone through the audio codec
 * @param note: Musical note to play
 * @param durationMs: Duration in milliseconds
 */
void AudioCodec_PlayTone(MusicNoteType note, uint32_t durationMs) {
    uint8_t commandBuffer[2];

    // Configure volume and off time
    commandBuffer[0] = 0x1D;
    commandBuffer[1] = 0x00;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, commandBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    // Set sound frequency based on note
    commandBuffer[0] = 0x1C;
    commandBuffer[1] = (note << 4);
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, commandBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    // Enable continuous tone mode
    commandBuffer[0] = 0x1E;
    commandBuffer[1] = 0xC0;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, commandBuffer, 2, I2C_COMMUNICATION_TIMEOUT);

    // Play tone for specified duration
    HAL_Delay(durationMs);

    // Disable continuous tone mode
    commandBuffer[0] = 0x1E;
    commandBuffer[1] = 0x00;
    HAL_I2C_Master_Transmit(&hi2c1, CS43L22_I2C_ADDRESS, commandBuffer, 2, I2C_COMMUNICATION_TIMEOUT);
}

/**
 * @brief Turn on specific LED
 */
void LED_TurnOn(uint16_t ledPin) {
    HAL_GPIO_WritePin(GPIOD, ledPin, GPIO_PIN_SET);
}

/**
 * @brief Turn off specific LED
 */
void LED_TurnOff(uint16_t ledPin) {
    HAL_GPIO_WritePin(GPIOD, ledPin, GPIO_PIN_RESET);
}

/**
 * @brief Turn on all LEDs
 */
void LED_TurnOnAll(void) {
    HAL_GPIO_WritePin(GPIOD, ALL_LEDS_MASK, GPIO_PIN_SET);
}

/**
 * @brief Turn off all LEDs
 */
void LED_TurnOffAll(void) {
    HAL_GPIO_WritePin(GPIOD, ALL_LEDS_MASK, GPIO_PIN_RESET);
}

/**
 * @brief Run LED-only animation mode (after melody completion)
 */
void RunLEDOnlyMode(void) {
    // Sequential LED turn-on pattern
    LED_TurnOn(LED_GREEN_PIN);
    HAL_Delay(250);
    LED_TurnOn(LED_ORANGE_PIN);
    HAL_Delay(250);
    LED_TurnOn(LED_RED_PIN);
    HAL_Delay(250);
    LED_TurnOn(LED_BLUE_PIN);
    HAL_Delay(250);

    // Sequential LED turn-off pattern
    LED_TurnOff(LED_BLUE_PIN);
    HAL_Delay(250);
    LED_TurnOff(LED_RED_PIN);
    HAL_Delay(250);
    LED_TurnOff(LED_ORANGE_PIN);
    HAL_Delay(250);
    LED_TurnOff(LED_GREEN_PIN);
    HAL_Delay(250);
}

/**
 * @brief Play complete melody with synchronized LED effects
 */
void PlayMelodyWithLEDs(void) {
    // First phrase: C5-D5-E5-C5 (twice)
    for (int phrase = 0; phrase < 2; phrase++) {
        LED_TurnOn(LED_GREEN_PIN);
        AudioCodec_PlayTone(NOTE_C5, QUARTER_NOTE_DURATION);
        LED_TurnOff(LED_GREEN_PIN);
        HAL_Delay(QUARTER_NOTE_DURATION);

        LED_TurnOn(LED_ORANGE_PIN);
        AudioCodec_PlayTone(NOTE_D5, QUARTER_NOTE_DURATION);
        LED_TurnOff(LED_ORANGE_PIN);
        HAL_Delay(QUARTER_NOTE_DURATION);

        LED_TurnOn(LED_RED_PIN);
        AudioCodec_PlayTone(NOTE_E5, QUARTER_NOTE_DURATION);
        LED_TurnOff(LED_RED_PIN);
        HAL_Delay(QUARTER_NOTE_DURATION);

        LED_TurnOn(LED_BLUE_PIN);
        AudioCodec_PlayTone(NOTE_C5, QUARTER_NOTE_DURATION);
        LED_TurnOff(LED_BLUE_PIN);
        HAL_Delay(QUARTER_NOTE_DURATION);
    }

    // Second phrase: E5-F5-G5
    LED_TurnOn(LED_RED_PIN);
    AudioCodec_PlayTone(NOTE_E5, QUARTER_NOTE_DURATION);
    LED_TurnOff(LED_RED_PIN);
    HAL_Delay(QUARTER_NOTE_DURATION);

    LED_TurnOn(LED_BLUE_PIN);
    AudioCodec_PlayTone(NOTE_F5, QUARTER_NOTE_DURATION);
    LED_TurnOff(LED_BLUE_PIN);
    HAL_Delay(QUARTER_NOTE_DURATION);

    // G5 with all LEDs (longer note)
    LED_TurnOnAll();
    AudioCodec_PlayTone(NOTE_G5, HALF_NOTE_DURATION);
    LED_TurnOffAll();
    HAL_Delay(HALF_NOTE_DURATION);

    LED_TurnOn(LED_RED_PIN);
    AudioCodec_PlayTone(NOTE_E5, QUARTER_NOTE_DURATION);
    LED_TurnOff(LED_RED_PIN);
    HAL_Delay(QUARTER_NOTE_DURATION);

    // Third phrase: F5-G5-A5-G5
    LED_TurnOn(LED_BLUE_PIN);
    AudioCodec_PlayTone(NOTE_F5, QUARTER_NOTE_DURATION);
    LED_TurnOff(LED_BLUE_PIN);
    HAL_Delay(QUARTER_NOTE_DURATION);

    LED_TurnOn(LED_GREEN_PIN);
    AudioCodec_PlayTone(NOTE_G5, QUARTER_NOTE_DURATION);
    LED_TurnOff(LED_GREEN_PIN);
    HAL_Delay(QUARTER_NOTE_DURATION);

    // A5 with all LEDs (longer note)
    LED_TurnOnAll();
    AudioCodec_PlayTone(NOTE_A5, HALF_NOTE_DURATION);
    LED_TurnOffAll();
    HAL_Delay(HALF_NOTE_DURATION);

    LED_TurnOn(LED_GREEN_PIN);
    AudioCodec_PlayTone(NOTE_G5, QUARTER_NOTE_DURATION);
    LED_TurnOff(LED_GREEN_PIN);
    HAL_Delay(QUARTER_NOTE_DURATION);

    // Final phrase: F5-A5-F5-D5 (repeated twice)
    for (int repeat = 0; repeat < 2; repeat++) {
        LED_TurnOn(LED_BLUE_PIN);
        AudioCodec_PlayTone(NOTE_F5, QUARTER_NOTE_DURATION);
        LED_TurnOff(LED_BLUE_PIN);
        HAL_Delay(QUARTER_NOTE_DURATION);

        HAL_GPIO_WritePin(GPIOD, LED_GREEN_PIN | LED_ORANGE_PIN, GPIO_PIN_SET);
        AudioCodec_PlayTone(NOTE_A5, QUARTER_NOTE_DURATION);
        HAL_GPIO_WritePin(GPIOD, LED_GREEN_PIN | LED_ORANGE_PIN, GPIO_PIN_RESET);
        HAL_Delay(QUARTER_NOTE_DURATION);

        LED_TurnOn(LED_BLUE_PIN);
        AudioCodec_PlayTone(NOTE_F5, QUARTER_NOTE_DURATION);
        LED_TurnOff(LED_BLUE_PIN);
        HAL_Delay(QUARTER_NOTE_DURATION);

        LED_TurnOn(LED_ORANGE_PIN);
        AudioCodec_PlayTone(NOTE_D5, QUARTER_NOTE_DURATION);
        LED_TurnOff(LED_ORANGE_PIN);
        HAL_Delay(QUARTER_NOTE_DURATION);
    }
}

/* USER CODE END 0 */

/**
 * @brief Main program entry point
 */
int main(void) {
    // System initialization
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2C1_Init();
    MX_I2S3_Init();

    // Initialize audio codec and start DMA transmission
    AudioCodec_Initialize();
    HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)audioBuffer, 100);

    // Main program loop
    while (1) {
        // Check if maximum melody plays reached
        if (melodyPlayCount >= MAX_MELODY_PLAYS) {
            // Switch to LED-only mode after completing melody cycles
            RunLEDOnlyMode();
            continue;
        }

        // Play melody with synchronized LEDs
        PlayMelodyWithLEDs();

        // Increment play counter
        melodyPlayCount++;

        // Pause between melody repetitions
        HAL_Delay(1000);
    }
}

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOD, AUDIO_ENABLE_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, ALL_LEDS_MASK, GPIO_PIN_RESET);

    /* Configure GPIO pins for audio enable and LEDs */
    GPIO_InitStruct.Pin = AUDIO_ENABLE_PIN | ALL_LEDS_MASK;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
 * @brief System Clock Configuration
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PLL_PLLM_CONFIG(16);
    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief I2C1 Initialization Function
 */
static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief I2S3 Initialization Function
 */
static void MX_I2S3_Init(void) {
    hi2s3.Instance = SPI3;
    hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
    hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_48K;
    hi2s3.Init.CPOL = I2S_CPOL_LOW;
    hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
    hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
    if (HAL_I2S_Init(&hi2s3) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief DMA Initialization Function
 */
static void MX_DMA_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

/**
 * @brief Error Handler Function
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // Stay in error state
    }
}
