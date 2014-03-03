/*
 *  Audio driver based on driver from peridiummmm (http://pouet.net/prod.php?which=59095)
 *  via Benjamin's robotics (http://vedder.se/2012/07/play-mp3-on-the-stm32f4-discovery/)
 */

#include "audio_driver.h"
#include "stm32f4xx_hal.h"
#include "applog.h"

#include <stdlib.h>

static void WriteRegister(uint8_t address, uint8_t value);
static void StartAudioDMA( int16_t* data, int nsamples );
static void StopAudioDMA();

static AudioCallbackFunction *ISRCallbackFunction;
static void *CallbackContext;
static int16_t * volatile NextBufferSamples;
static volatile int NextBufferLength;
static volatile int BufferNumber;
static volatile bool DMARunning;
static uint8_t bufferUnderrun = 0;
static int16_t dummy[50] = {0};


void InitializeAudio()
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Intitialize state.
    NextBufferSamples = NULL;
	NextBufferLength = 0;
	BufferNumber = 0;
	DMARunning = false;

	// Turn on peripherals.
    /* Enable GPIOs clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();

    __DMA1_CLK_ENABLE();
    __I2C1_CLK_ENABLE();
    __SPI3_CLK_ENABLE();

    // Configure reset pin.
    GPIO_InitStructure.Pin = GPIO_PIN_4;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Alternate = 0;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	// Configure I2C SCL and SDA pins.
    GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_9;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStructure.Alternate = GPIO_AF4_I2C1;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Configure I2S MCK, SCK, SD pins.
    GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_12;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = GPIO_AF6_SPI3;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configure I2S WS pin.
    GPIO_InitStructure.Pin = GPIO_PIN_4;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = GPIO_AF6_SPI3;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);


	// Reset the codec.
	GPIOD ->BSRRH = 1 << 4;
	for (volatile int i = 0; i < 0x4fff; i++) {
		__asm__ volatile("nop");
	}
	GPIOD ->BSRRL = 1 << 4;


	// Reset I2C.
	__I2C1_FORCE_RESET();
    __I2C1_RELEASE_RESET();

	// Configure I2C.
	uint32_t pclk1 = 42000000;

	I2C1 ->CR2 = pclk1 / 1000000; // Configure frequency and disable interrupts and DMA.
	I2C1 ->OAR1 = I2C_OAR1_ADDMODE | 0x33;

	// Configure I2C speed in standard mode.
	const uint32_t i2c_speed = 100000;
	int ccrspeed = pclk1 / (i2c_speed * 2);
	if (ccrspeed < 4) {
		ccrspeed = 4;
	}
	I2C1 ->CCR = ccrspeed;
	I2C1 ->TRISE = pclk1 / 1000000 + 1;

	I2C1 ->CR1 = I2C_CR1_ACK | I2C_CR1_PE; // Enable and configure the I2C peripheral.

	// Configure codec.
	WriteRegister(0x02, 0x01); // Keep codec powered off.
	WriteRegister(0x04, 0xaf); // SPK always off and HP always on.

	WriteRegister(0x05, 0x81); // Clock configuration: Auto detection.
	WriteRegister(0x06, 0x04); // Set slave mode and Philips audio standard.

	//SetAudioVolume(0xff);
}

void AudioOn() {
	WriteRegister(0x02, 0x9e);
	SPI3 ->I2SCFGR = SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG_1
			| SPI_I2SCFGR_I2SE; // Master transmitter, Phillips mode, 16 bit values, clock polarity low, enable.
}

void AudioOff() {
	WriteRegister(0x02, 0x9f);
	SPI3 ->I2SCFGR = 0;
}

void SetAudioVolume(int volume)
{
    /* For registers 0x20 and 0x21:
     *   0x18 to 0x00 = +12 dB to 0 dB
     *   0xFF to 0x35 = -0.5 dB to -102 dB
     *   0x34 to 0x19 = -102 dB */
	WriteRegister(0x20, (volume + 0x19) & 0xff);
	WriteRegister(0x21, (volume + 0x19) & 0xff);
}

void OutputAudioSample(int16_t sample) {
	while (!(SPI3 ->SR & SPI_SR_TXE ))
		;
	SPI3 ->DR = sample;
}

void OutputAudioSampleWithoutBlocking(int16_t sample) {
	SPI3 ->DR = sample;
}

void EnableAudio(int plln, int pllr, int i2sdiv, int i2sodd, AudioCallbackFunction *isrcallback, void *context)
{
    ISRCallbackFunction = isrcallback;
    CallbackContext = context;

    // Power on the codec.
    WriteRegister(0x02, 0x9e);

    // Configure codec for fast shutdown.
    WriteRegister(0x0a, 0x00); // Disable the analog soft ramp.
    WriteRegister(0x0e, 0x04); // Disable the digital soft ramp.

    WriteRegister(0x27, 0x00); // Disable the limiter attack level.
    WriteRegister(0x1f, 0x0f); // Adjust bass and treble levels.

    WriteRegister(0x1a, 0x0a); // Adjust PCM volume level.
    WriteRegister(0x1b, 0x0a);

    // Disable I2S.
    SPI3 ->I2SCFGR = 0;

    // I2S clock configuration
    RCC ->CFGR &= ~RCC_CFGR_I2SSRC; // PLLI2S clock used as I2S clock source.
    RCC ->PLLI2SCFGR = (pllr << 28) | (plln << 6);

    // Enable PLLI2S and wait until it is ready.
    RCC ->CR |= RCC_CR_PLLI2SON;
    while (!(RCC ->CR & RCC_CR_PLLI2SRDY ))
        ;

    // Configure I2S.
    SPI3 ->I2SPR = i2sdiv | (i2sodd << 8) | SPI_I2SPR_MCKOE;
    SPI3 ->I2SCFGR = SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG_1
            | SPI_I2SCFGR_I2SE; // Master transmitter, Phillips mode, 16 bit values, clock polarity low, enable.

	HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
	HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 6, 0);

	SPI3 ->CR2 |= SPI_CR2_TXDMAEN; // Enable I2S TX DMA request.
}

void DisableAudio() {
	StopAudioDMA();
	SPI3 ->CR2 &= ~SPI_CR2_TXDMAEN; // Disable I2S TX DMA request.
	NVIC_DisableIRQ(DMA1_Stream7_IRQn);
}

void ProvideAudioBuffer(void *samples, int numsamples) {
	while (!ProvideAudioBufferWithoutBlocking(samples, numsamples))
		__asm__ volatile ("wfi");
}

bool ProvideAudioBufferWithoutBlocking(void *samples, int numsamples) {
    static unsigned int padCount = 0;

	if (NextBufferSamples)
		return false;

	NVIC_DisableIRQ(DMA1_Stream7_IRQn);

    /* due to clock frequencies and division numbers etc we're actually playing at 44108 Hz...
     * Here's a dumbass hack to compensate...
     */
    padCount += (numsamples/2);
    if ( padCount > 5513 )
    {
        uint16_t* buf = samples;
        buf[numsamples] = buf[numsamples-2];
        buf[numsamples+1] = buf[numsamples-1];
        numsamples+=2;
        padCount -= 5513;
    }

    BufferNumber ^= 1;

	if ( DMARunning )
	{
	    NextBufferSamples = samples;
	    NextBufferLength = numsamples;
	}
	else
	{
		StartAudioDMA( samples, numsamples );
	}

	NVIC_EnableIRQ(DMA1_Stream7_IRQn);

	return true;
}

static void WriteRegister(uint8_t address, uint8_t value) {
	while (I2C1 ->SR2 & I2C_SR2_BUSY )
		;

	I2C1 ->CR1 |= I2C_CR1_START; // Start the transfer sequence.
	while (!(I2C1 ->SR1 & I2C_SR1_SB ))
		; // Wait for start bit.

	I2C1 ->DR = 0x94;
	while (!(I2C1 ->SR1 & I2C_SR1_ADDR ))
		; // Wait for master transmitter mode.
	I2C1 ->SR2;

	I2C1 ->DR = address; // Transmit the address to write to.
	while (!(I2C1 ->SR1 & I2C_SR1_TXE ))
		; // Wait for byte to move to shift register.

	I2C1 ->DR = value; // Transmit the value.

	while (!(I2C1 ->SR1 & I2C_SR1_BTF ))
		; // Wait for all bytes to finish.
	I2C1 ->CR1 |= I2C_CR1_STOP; // End the transfer sequence.
}

static void StartAudioDMA( int16_t* data, int nsamples )
{
    DMA1_Stream7 ->CR = (0 * DMA_SxCR_CHSEL_0 ) | // Channel 0
            (1 * DMA_SxCR_PL_0 ) | // Priority 1
            (1 * DMA_SxCR_PSIZE_0 ) | // PSIZE = 16 bit
            (1 * DMA_SxCR_MSIZE_0 ) | // MSIZE = 16 bit
            DMA_SxCR_MINC | // Increase memory address
            (1 * DMA_SxCR_DIR_0 ) | // Memory to peripheral
            DMA_SxCR_TCIE; // Transfer complete interrupt
    DMA1_Stream7 ->NDTR = nsamples;
    DMA1_Stream7 ->PAR = (uint32_t) &SPI3 ->DR;
    DMA1_Stream7 ->M0AR = (uint32_t) data;
    DMA1_Stream7 ->FCR = DMA_SxFCR_DMDIS;
    DMA1_Stream7 ->CR |= DMA_SxCR_EN;

    DMARunning = true;
}


static void StopAudioDMA() {
	DMA1_Stream7 ->CR &= ~DMA_SxCR_EN; // Disable DMA stream.
	while (DMA1_Stream7 ->CR & DMA_SxCR_EN )
		; // Wait for DMA stream to stop.

	DMARunning = false;
}
void DMA1_Stream7_IRQHandler()
{
    DMA1 ->HIFCR |= DMA_HIFCR_CTCIF7; // Clear interrupt flag.

    if ( NextBufferSamples )
    {

        StartAudioDMA( NextBufferSamples, NextBufferLength );
        NextBufferSamples = NULL;

        /* Notify that it's now ok to load a new buffer */
        if (ISRCallbackFunction)
            ISRCallbackFunction(CallbackContext, BufferNumber);

        if ( !bufferUnderrun )
        {
            bufferUnderrun = 0;
            //WriteRegister(0x04, 0xaf);
            //WriteRegister(0x02, 0x9e);
            //AudioOn();
        }
    }
    else
    {
        // buffer underrun, play silence (just muting doesn't help, seems like we have to
        // continuously play something until audio has shut down properly)
        StartAudioDMA( dummy, 50 );

        if ( !bufferUnderrun )
        {
            //WriteRegister(0x04, 0xff);
            //WriteRegister(0x02, 0x9f);
            //AudioOff();
            //DMARunning = false;

            bufferUnderrun = 1;
        }
	}
}
