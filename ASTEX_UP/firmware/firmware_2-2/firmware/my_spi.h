/*
 * my_spi.h
 *
 * Created: 12/27/2014 10:57:58 AM
 *  Author: Aman
 */ 


#ifndef MY_SPI_H_
#define MY_SPI_H_

//SPI pin configuration for Atmega32 standard
#define DDR_SPI DDRB
#define DD_MOSI 5
#define DD_SCK 7
#define DD_MISO 6

#include <avr/io.h>

//SPI initialization
//clock rate: 125Khz

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI |= (1<<DD_MOSI)|(1<<DD_SCK);
	/* Enable SPI, Master, set clock rate fck/64=125kHz@8MHz */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<CPHA);
}

unsigned char SPI_MasterTransmit_Receive(char cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	cData=SPDR;
	return cData;
}

void SPI_SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDR_SPI |= (1<<DD_MISO);
	/* Enable SPI */
	SPCR = (1<<SPE);
}
unsigned char SPI_SlaveReceive(void)
{
	/* Wait for reception complete */
	while(!(SPSR & (1<<SPIF)));
	/* Return data register */
	return SPDR;
}


#endif /* MY_SPI_H_ */