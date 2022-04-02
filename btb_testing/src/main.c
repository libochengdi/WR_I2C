/**
 * BTB Testing code for Wisconsin Racing, validating PCA9536's battery muxing
 * Created by Eric Li
 *  on March 31, 2023
 */

#include "PCA9536.h"

/**
 * @brief enable clocking for all GPIO ports
 * Slightly modified version of TI's example code
 */
void init_gpio_periph(void)
{
    // enable clocking
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // wait for clocking to be ready
    while ((!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC)) && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)) && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)))
    {
    };
}

/**
 * @brief TI's Example Initialization code 
 */
void InitI2C0(void)
{
    // enable I2C module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    // reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    // enable GPIO peripheral that contains I2C 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
    
    // I2CLoopbackEnable(I2C0_BASE);
    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    // clear I2C FIFOs
    // HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}

/**
 * @brief Read from slave's register
 * 
 * @param slave_addr Address of slave component
 * @param reg Register bit to be read
 * @return uint32_t data read from reg bit on slave
 */
uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);

    //specify register to be read
    I2CMasterDataPut(I2C0_BASE, reg);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);


    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, true);

    //send control byte and read from the register we specified
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));

    //return data pulled from the specified register
    return I2CMasterDataGet(I2C0_BASE);
}

/**
 * @brief sends an array of data via I2C to the specified slave
 * 
 * @param slave_addr address of slave component
 * @param array array of message to send
 */
void I2CSendString(uint32_t slave_addr, char array[])
{
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CSlaveACKOverride(I2C0_BASE, true);
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);

    // put data to be sent into FIFO
    I2CMasterDataPut(I2C0_BASE, array[0]);

    // if there is only one argument, we only need to use the
    // single send I2C function
    if (array[1] == '\0')
    {

        // Initiate send of data from the MCU
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

        // Wait until MCU is done transferring.
        while (I2CMasterBusy(I2C0_BASE))
            ;
    }

    // otherwise, we start transmission of multiple bytes on the
    // I2C bus
    else
    {
        // Initiate send of data from the MCU
        // 
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        // Wait until MCU is done transferring.
        while (I2CMasterBusy(I2C0_BASE))
            ;

        // initialize index into array
        uint8_t i = 1;

        // send num_of_args-2 pieces of data, using the
        // BURST_SEND_CONT command of the I2C module
        while (array[i + 1] != '\0')
        {
            // put next piece of data into I2C FIFO
            I2CMasterDataPut(I2C0_BASE, array[i++]);

            // send next data that was just placed into FIFO
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

            // Wait until MCU is done transferring.
            while (I2CMasterBusy(I2C0_BASE))
                ;
        }

        // put last piece of data into I2C FIFO
        I2CMasterDataPut(I2C0_BASE, array[i]);

        // send next data that was just placed into FIFO
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

        // Wait until MCU is done transferring.
        while (I2CMasterBusy(I2C0_BASE))
            ;
    }
}

/**
 * @brief TI's Example Initialization code 
 */
void InitConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // Can change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Enable UART0 so that we can configure the clock.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    // UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Select the alternate (UART) function for these pins.
    // Can change this to select the port/pin you are using.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    // UARTStdioConfig(0, 115200, 16000000);
}

/**
 * @brief Call peripheral initializations
 */
void init(void)
{
    // Disable All interrupts
    IntMasterDisable();

    // Enable peripheral
    init_gpio_periph();

    // Enable interrupts
    IntMasterEnable();
}

/**
 * @brief Flash LED to indicate # port is not functioning properly
 * 
 * LED Colors: 
 *      GPIO_PIN_1: Red 
 *      GPIO_PIN_2: Blue 
 *      GPIO_PIN_3: Green
 *      Purple: Red + Blue
 *      White: Red + Blue + Green
 *
 * @param portnum Port to read from
 */
void FlashLED(int portnum)
{

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Temporarily turn green off because sth is wrong

    if (portnum == 1)
    { 
        // Port 1: Flash Red
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(23000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
    }
    else if (portnum == 2)
    {
        // Port 2: Flash Blue
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        SysCtlDelay(23000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }
    else if (portnum == 3)
    {
        // Port 3: FlashPurple (Red&Blue)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        SysCtlDelay(23000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }
    else if (portnum == 4)
    {
        // Port 4: Flash White (all 3 ports)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
        SysCtlDelay(23000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    }
    SysCtlDelay(13000000);
}

int main(void)
{
    while (1)
    {
        SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

        // volatile uint32_t ui32Loop;

        init();
        InitI2C0();

        GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3); // Initialize Launchpad LED PIN
        GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_5); // Set B5 pin to take input

        // Wait until master module is done transferring
        while (I2CMasterBusy(I2C0_BASE))
        {
        }

        int counter = 0; // Counter to keep track of port num
        char output_command[2]; // [0] = Command byte, [1] = Data to register byte
        output_command[0] = PCA9536_PIN_OUTPUT;

        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Flash LED to green 

        I2CSendString(PCA9536_ADDRESS, PCA9536_PIN_CONFIG_ALL); // Turn Port 0123 into output port

        while (1)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

            // Muxing through the ports
            switch (counter)
            {
                case 0:
                    output_command[1] = PCA9536_PORT1;
                    break;

                case 1:
                    output_command[1] = PCA9536_PORT2;
                    break;
                
                case 2:
                    output_command[1] = PCA9536_PORT3;
                    break;
                
                case 3:
                    output_command[1] = PCA9536_PORT4;
                    break;
            }

            output_command[1] = 0b11110000; // This works!
            I2CSendString(PCA9536_ADDRESS, output_command);

            SysCtlDelay(13000000);

            output_command[1] = 0b11111111;
            I2CSendString(PCA9536_ADDRESS, output_command);
            
            // Reading from Launchpad's B5 (simulate battery reading)
            int status = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);

            if (status > 0) {
                FlashLED(counter+1);
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Turn on green again
            }

            if (counter < 4) {
                counter++;
            } else {
                counter = 0;
            }

        }
        return 0; // Should never reach, unless...?
    }
}
