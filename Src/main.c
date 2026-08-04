#include "stm32f4xx_hal.h"
#include "stm32469i_discovery.h"
#include "stm32469i_discovery_lcd.h"
#include "fonts.h"
#include <stdio.h>
#include "bip39_lookup.h"
#include "stm32469i_discovery_ts.h"

static void SystemClock_Config( void );
static void ClearEntry( void );
static void AddBit( uint8_t bit );

int main( void )
{
    HAL_Init();
    SystemClock_Config();

    BSP_LED_Init( LED4 );
    BSP_LCD_Init( );
    BSP_TS_Init( 800, 480 );

    BSP_LCD_LayerDefaultInit( 0, LCD_FB_START_ADDRESS );
    BSP_LCD_SelectLayer( 0 );
    BSP_LCD_SetFont( &Font24 );


    BSP_LCD_Clear( LCD_COLOR_BLACK );

    //  Buttons to select 0 or 1.
    BSP_LCD_SetTextColor( LCD_COLOR_LIGHTGRAY );
    BSP_LCD_FillRect( 0, 240, 400, 240 );
    BSP_LCD_SetTextColor( LCD_COLOR_DARKGRAY );
    BSP_LCD_FillRect( 400, 240, 400, 240 );

    BSP_LCD_SetTextColor( LCD_COLOR_BLACK );
    BSP_LCD_SetBackColor( LCD_COLOR_LIGHTGRAY );
    BSP_LCD_DisplayStringAt( 200, 330, ( uint8_t *)"0", LEFT_MODE );

    BSP_LCD_SetTextColor( LCD_COLOR_WHITE );
    BSP_LCD_SetBackColor( LCD_COLOR_DARKGRAY );
    BSP_LCD_DisplayStringAt( 600, 330, ( uint8_t *)"1", LEFT_MODE );


    //  CLEAR button.
    BSP_LCD_SetFont( &Font24);
    BSP_LCD_SetTextColor( LCD_COLOR_BLUE );
    BSP_LCD_FillRect( 600, 0, 200, 80 );
    BSP_LCD_SetTextColor( LCD_COLOR_WHITE );
    BSP_LCD_SetBackColor( LCD_COLOR_BLUE);
    BSP_LCD_DisplayStringAt( 650, 25, ( uint8_t *)"CLEAR", LEFT_MODE );

    while( 1 )
    {
        TS_StateTypeDef ts_state;

        BSP_TS_GetState( &ts_state );

        if( ts_state.touchDetected )
        {
            uint16_t x = ts_state.touchX[0];
            uint16_t y = ts_state.touchY[0];

            if( x >= 600 && y < 100 )
            {
                ClearEntry();
            }

            else if( y >= 240 )
            {
                 if( x < 400 )
                 {
                    AddBit( 0 );
                 }
                 else
                 {
                    AddBit( 1 );
                 }

                 while( ts_state.touchDetected )
                 {
                    BSP_TS_GetState( &ts_state );
                    HAL_Delay( 20 );
                 }
            }
        }

        HAL_Delay( 20 );
    }
}


static void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    HAL_StatusTypeDef ret = HAL_OK;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /*   The voltage scaling allows optimizing the power consumption when the device is
         clocked below the maximum system frequency, to update the voltage scaling value
        regarding system frequency refer to product datasheet.  */
     __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    #if defined(USE_STM32469I_DISCO_REVA)
    RCC_OscInitStruct.PLL.PLLM = 25;
    #else
    RCC_OscInitStruct.PLL.PLLM = 8;
    #endif /* USE_STM32469I_DISCO_REVA */
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 6;

    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if(ret != HAL_OK)
    {
        while(1) { ; }
    }

    /* Activate the OverDrive to reach the 180 MHz Frequency */
    ret = HAL_PWREx_EnableOverDrive();
    if(ret != HAL_OK)
    {
         while(1) { ; }
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
    if(ret != HAL_OK)
    {
        while(1) { ; }
    }
}

static void AddBit( uint8_t bit )
{
    bip39_add_bit( bit );

    char bits[ 12 ];   // 11 bits + null
    uint8_t count = bip39_get_bit_count( );
    uint16_t value = bip39_get_value( );

    // build string from right to left
    for( int ii = 0; ii < 11; ii++ )
    {
        if( ii < count )
        {
            bits[10 - ii] = ( value & ( 1 << ii ) ) ? '1' : '0';
        }
        else
        {
            bits[10 - ii] = ' ';
        }
    }

    bits[ 11 ] = '\0';

    BSP_LCD_SetFont( &Font24 );
    BSP_LCD_SetTextColor( LCD_COLOR_WHITE );
    BSP_LCD_SetBackColor( LCD_COLOR_BLACK );

    BSP_LCD_DisplayStringAt( 100, 130, ( uint8_t *)bits, RIGHT_MODE );


    if( bip39_is_complete( ) )
    {
        char buffer[ 32 ];

        const char *seed_word = bip39_get_word( );
        sprintf( buffer, "%u: %s", bip39_get_value() + 1, seed_word );

        BSP_LCD_DisplayStringAt( 50, 130, ( uint8_t *)buffer, LEFT_MODE );

        BSP_LED_On( LED4 );
    }
}


static void ClearEntry( void )
{
    bip39_clear( );

    BSP_LED_Off( LED4 );

    BSP_LCD_SetTextColor( LCD_COLOR_BLACK );
    BSP_LCD_FillRect( 0, 100, 800, 120 );

    BSP_LCD_SetTextColor( LCD_COLOR_WHITE );
    BSP_LCD_SetBackColor( LCD_COLOR_BLACK );
}

