// Lab 8 Prelab

// Daniel Valvano
//Code edited by grant postma But most was just taken by Daniel Valvano
/* DriverLib Includes */
#include "ti/devices/msp432p4xx/driverlib/driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
//#include "msp.h"
#include "ST7735.h"
#include <string.h>
/* Standard Includes */
#include <stdio.h>

//This is for storing the read bits
int RTC_registers[19];
int RTC_ADDRESS = 0x68; //Address of our slave 0110100

/* I2C Master Configuration Parameter (in the example code portion of the classic resource explorer */
const eUSCI_I2C_MasterConfig i2cConfig = {
EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        3000000,                                // SMCLK = 3MHz
        EUSCI_B_I2C_SET_DATA_RATE_400KBPS,      // Desired I2C Clock of 400khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_NO_AUTO_STOP                // No Autostop
        };

//These bits are set for setting the BCD values, The Second-Year were used int part one now its modified for part 3
int bcd_registers[12];
int bcdSecond = 0x1F;
int bcdMinute = 0x26;
int bcdHour = 0x16;
int bcdDay = 0x05;
int bcdDate = 0x03;
int bcdMonth = 0x09;
int bcdYear = 0x11;
//Flag tells when the * has been pressed
int flag = 0;

/* Port 9 Output, bit 4 is TFT CS */
#define TFT_CS (*((volatile uint8_t *)0x40004C82))
#define TFT_CS_BIT 0x10 // CS normally controlled by hardware
/* Port 9 Output, bit 3 is RESET*/
#define RESET (*((volatile uint8_t *)0x40004C82))
#define RESET_BIT 0x08

#define CALIBRATION_START 0x000200000
uint8_t simulatedCalibrationData[]; // array to hold data
//uint8_t simulatedCalibrationData[30]; // array to hold values read back from flash
uint8_t* addr_pointer; // pointer to address in flash for reading back values

int c = 0;

int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();
    SysTick_Init();

    P5->DIR |= (BIT0 | BIT1 | BIT2 | BIT3 );

    P5->OUT |= BIT0;
    P5->OUT &= ~(BIT1 | BIT2 | BIT3 );

    clockInit48MHzXTL();                   // set system clock to 48 MHz
    ST7735_InitR(INITR_REDTAB);     //Sets screen to black and initialzes colors

    P4->REN |= (BIT0 | BIT1 | BIT2 | BIT3 );
    P4->OUT |= (BIT0 | BIT1 | BIT2 | BIT3 );

    char pressed = 0xFF;
    ST7735_FillScreen(0x0000);            // set screen to black(0000)

    i2cinit(); //Copy pasted from lecture
    SysTick_Init(); //Code from lab 1
    i2cwrite(); //Copy pasted from lab
    HomeScreen();
    i2cread(); //Reading the value that you want, Flag is 1 ,2 or 3, and will if/print the wanted value


    while (1)
    {
        //most of this is the same code from part one
        pressed = keypad_getkey();
        if (pressed != 0)
        {
            DelayWait10ms(7);
            if (keypad_getkey() != pressed)
            {
                pressed = Char_Convert(pressed);

                if (pressed == '1') //if it is a * it prints it and doesnt count it as a character
                {
                    i2cread();
                    saveTimeDate();
                    loadTimeDate();
                    ST7735_SetCursor(7,10);
                    printf("Saved!");
                    DelayWait10ms(100);
                }
                if (pressed == '2') //if it is a * it prints it and doesnt count it as a character
                {

                    loadTimeDate();
                    printDate();
                    DelayWait10ms(300);

                }
                HomeScreen();

            }

        }
    }
}


void HomeScreen(){

    ST7735_FillScreen(0x0000);            // set screen to black(0000)
    ST7735_SetCursor(0,0);
    printf("1 to save Time");
    ST7735_SetCursor(0,1);
    printf("2 to print Time");
}


//Prompts the user
void printDate()
{
    ST7735_FillScreen(0x0000);            // set screen to black(0000)
    int y = 0;
    for(y = 0; y < 5;y++){
    ST7735_SetCursor(4, y*2);
    ST7735_OutUDec(simulatedCalibrationData[3 + (y*6)]);
    ST7735_SetCursor(6, y*2);
    printf("/");

    ST7735_SetCursor(7, y*2);
    ST7735_OutUDec(simulatedCalibrationData[4 + (y*6)]);
    ST7735_SetCursor(9, y*2);
    printf("/");

    ST7735_SetCursor(10, y*2);
    ST7735_OutUDec(simulatedCalibrationData[5 + (y*6)]);
    ST7735_SetCursor(12, y*2);

    ST7735_SetCursor(4, y*2 + 1);
    ST7735_OutUDec(simulatedCalibrationData[2 + (y*6)]);
    ST7735_SetCursor(6, y*2 + 1);
    printf(":");

    ST7735_SetCursor(7, y*2 + 1);
    ST7735_OutUDec(simulatedCalibrationData[1 + (y*6)]);
    ST7735_SetCursor(9, y*2 + 1);
    printf(":");

    ST7735_SetCursor(10, y*2 + 1);
    ST7735_OutUDec(simulatedCalibrationData[0 + (y*6)]);
    ST7735_SetCursor(12, y*2 + 1);
    }


}

void saveTimeDate()
{
    int i = 0;
    for(i = 0;i<4;i++){
    simulatedCalibrationData[ 29-(i*(6)) ] = simulatedCalibrationData[ 23-(i*(6)) ];
    simulatedCalibrationData[( 28-i*(6) )] = simulatedCalibrationData[( 22-i*(6) )];
    simulatedCalibrationData[( 27-i*(6) )] = simulatedCalibrationData[( 21-i*(6) )];
    simulatedCalibrationData[( 26-i*(6) )] = simulatedCalibrationData[( 20-i*(6) )];
    simulatedCalibrationData[( 25-i*(6) )] = simulatedCalibrationData[( 19-i*(6) )];
    simulatedCalibrationData[( 24-i*(6) )] = simulatedCalibrationData[( 18-i*(6) )];

    }


    simulatedCalibrationData[0] = RTC_registers[0];
    simulatedCalibrationData[1] = RTC_registers[1];
    simulatedCalibrationData[2] = RTC_registers[2];
    simulatedCalibrationData[3] = RTC_registers[4];
    simulatedCalibrationData[4] = RTC_registers[5];
    simulatedCalibrationData[5] = RTC_registers[6];


    /* Unprotecting Info Bank 0, Sector 0 */
    MAP_FlashCtl_unprotectSector(FLASH_INFO_MEMORY_SPACE_BANK0, FLASH_SECTOR0);
    /* Erase the flash sector starting CALIBRATION_START. */
    while (!MAP_FlashCtl_eraseSector(CALIBRATION_START))
        ;
    /* Program the flash with the new data. */
    while (!MAP_FlashCtl_programMemory(simulatedCalibrationData,
                                       (void*) CALIBRATION_START + 4, 30)); // leave first 4 bytes unprogrammed
    /* Setting the sector back to protected */
    MAP_FlashCtl_protectSector(FLASH_INFO_MEMORY_SPACE_BANK0, FLASH_SECTOR0);

}

void loadTimeDate()
{
    uint8_t i; // index

    addr_pointer = CALIBRATION_START + 4; // point to address in flash for saved data
    for (i = 0; i < 30; i++)
    { // read values in flash after programming
        simulatedCalibrationData[i] = *addr_pointer++; //Readback Data
    }


}

// Daniel Valvano
// private function draws a color band on the screen
void static drawthecolors(uint8_t red, uint8_t green, uint8_t blue)
{
    static uint16_t y = 0;
    ST7735_DrawFastHLine(0, y, ST7735_TFTWIDTH,
                         ST7735_Color565(red, green, blue));
    y = y + 1;
    if (y >= ST7735_TFTHEIGHT)
    {
        y = 0;
    }
    DelayWait10ms(1);
}

// Daniel Valvano
void clockInit48MHzXTL(void)
{ // sets the clock module to use the external 48 MHz crystal
/* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2,
            GPIO_PRIMARY_MODULE_FUNCTION);
    CS_setExternalClockSourceFrequency(32000, 48000000); // enables getMCLK, getSMCLK to know externally set frequencies
    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false); // false means that there are no timeouts set, will return when stable
    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

//This program takes in the pressed char and converts it to the actual character
//for 1-9 this is just adding 48, for the reset it converts to the keypad character 0,* and #
char Char_Convert(char c)
{
    c = c + 48;
    if (c == ':')
    {
        c = '*';
    }
    if (c == ';')
    {
        c = '0';
    }
    if (c == '<')
    {
        c = '#';
    }
}

//Code from Prof Bossemeyer, He took from someone else and modified
char keypad_getkey(void)
{ // assumes port 4 bits 0-3 are connected to rows
    int row, col; // bits 4,5,6 are connected to columns
    const char column_select[] = { 0x10, 0x20, 0x40 }; // one column is active
// Activates one column at a time, read the input to see which column
    for (col = 0; col < 3; col++)
    {
        P4->DIR &= ~0xF0; // disable all columns
        P4->DIR |= column_select[col]; // enable one column at a time
        P4->OUT &= ~column_select[col]; // drive the active column low
        __delay_cycles(10); // wait for signal to settle
        row = P4->IN & 0x0F; // read all rows
        P4->OUT |= column_select[col]; // drive the active column high
        if (row != 0x0F)
            break; // if one of the input is low,
// some key is pressed.
    }
    P4->OUT |= 0xF0; // drive all columns high before disable
    P4->DIR &= ~0xF0; // disable all columns
    if (col == 3)
        return 0; // if we get here, no key is pressed
// gets here when one of the columns has key pressed,
// check which row it is
    if (row == 0x0E)
        return col + 1; // key in row 0
    if (row == 0x0D)
        return 3 + col + 1; // key in row 1
    if (row == 0x0B)
        return 6 + col + 1; // key in row 2
    if (row == 0x07)
        return 9 + col + 1; // key in row 3
    return 0; // just to be safe
}

void SysTick_Init(void) //Not my code, given by prof
{
    SysTick->CTRL = 0; // disable SysTick during setup
    SysTick->LOAD = 0x00FFFFFF; // maximum reload value
    SysTick->VAL = 0; // any write to current value clears it
    SysTick->CTRL = 0x00000005; // enable SysTick, CPU clk, no interrupts
}

void SysTick_delay(uint16_t delay)  //Not my code, given by prof
{
    SysTick->LOAD = ((delay * 3000) - 1); // 1ms count down to zero
    SysTick->VAL = 0; // any write to CVR clears it
// and COUNTFLAG in CSR
// Wait for flag to be SET (Timeout happened)
    while ((SysTick->CTRL & 0x00010000) == 0)
        ;
}

void i2cinit()
{

// For example, select Port 6 for I2C
// Set Pin 4, 5 to input Primary Module Function,
// P6.4 is UCB1SDA, P6.5 is UCB1SCL
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P6,
            GPIO_PIN4 + GPIO_PIN5,
            GPIO_PRIMARY_MODULE_FUNCTION);
// Initializing I2C Master (see description in Driver Lib for
// proper configuration options)
    MAP_I2C_initMaster(EUSCI_B1_BASE, &i2cConfig);
// Specify slave address
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, RTC_ADDRESS);
// Set Master in transmit mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
// Enable I2C Module to start operations
    MAP_I2C_enableModule(EUSCI_B1_BASE);
}

//--------------------------------------------------------------------WRITE WRITE---------------------------------------------------------------------------------
void i2cwrite()
{

    /* Set Master in transmit mode */
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
// Wait for bus release, ready to write
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
// set pointer to beginning of RTC registers
    MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE, 0);
// and write to seconds register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, bcdSecond);
// write to minutes register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, bcdMinute);
// write to hours register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, bcdHour);
// write to day register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, bcdDay);
// write to date register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, bcdDate);
// write to months register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, bcdMonth);
// write to year register and send stop
    MAP_I2C_masterSendMultiByteFinish(EUSCI_B1_BASE, bcdYear);

// Set Master in transmit mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
// Wait for bus release, ready to write
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
// set pointer to beginning of RTC registers
    MAP_I2C_masterSendSingleByte(EUSCI_B1_BASE, 0);
// Wait for bus release
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
// Set Master in receive mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
// read from RTC registers (pointer auto increments after each read)
    RTC_registers[0] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
}

//--------------------------------------------------------------------READ READ---------------------------------------------------------------------------------
//This fuction was given to us in lecture then i tweaked it so that it took in all the bits so that i could grab the final values for temperature
void i2cread()
{
// Set Master in transmit mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
// Wait for bus release, ready to write
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
// set pointer to beginning of RTC registers
    MAP_I2C_masterSendSingleByte(EUSCI_B1_BASE, 0);
// Wait for bus release
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
// Set Master in receive mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
// read from RTC registers (pointer auto increments after each read)
    RTC_registers[0] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[1] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[2] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[3] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[4] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[5] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[6] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[7] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[8] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[9] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[10] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[11] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[12] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[13] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[14] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[15] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[16] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[17] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;
    RTC_registers[18] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
// Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE))
        ;

//    // whatever button was pressed  is the if it goes in
//        ST7735_SetCursor(0,5);
//        printf("Date: %x/%x/%x\n", RTC_registers[5], RTC_registers[4], //Prints these RTC values
//               RTC_registers[6]);
//        ST7735_SetCursor(0,6);
//        DelayWait10ms(100);
//        printf("Time: %x:%x:%x\n", RTC_registers[2], RTC_registers[1], //Prints these RTC values
//               RTC_registers[0]);
//        ST7735_SetCursor(0,7);
//        DelayWait10ms(100);
//        float temp = RTC_registers[17]
//                + (((RTC_registers[18] >> 6) & 0x03) * .25); //Shifts left 6 bits and masks with 0x0, This makes it n/4 for the decimal
//        printf("Temp: %f Celcius\n", temp); //Prints these RTC values

}

void Delay1ms(uint32_t n);

void DelayWait10ms(uint32_t n)
{
    Delay1ms(n * 10);
}
