
#ifndef ADC_READER_H
#define ADC_READER_H

#include <stdbool.h>
#include <stdint.h>

/* Initialize ADC1 channel 0 on PA0. */
void ADCReader_Init(void);

/*
 * Perform one ADC conversion.
 *
 * Returns true if conversion succeeds.
 * Returns false if the ADC times out.
 */
bool ADCReader_Read(uint16_t *adc_value);

#endif
