
#include "adc_reader.h"

#include <stdint.h>

/* RCC peripheral registers. */
#define RCC_AHB1ENR (*(volatile uint32_t *)0x40023830UL)
#define RCC_APB2ENR (*(volatile uint32_t *)0x40023844UL)

/* GPIOA peripheral registers. */
#define GPIOA_MODER (*(volatile uint32_t *)0x40020000UL)
#define GPIOA_PUPDR (*(volatile uint32_t *)0x4002000CUL)

/* ADC1 peripheral registers. */
#define ADC1_SR    (*(volatile uint32_t *)0x40012000UL)
#define ADC1_CR1   (*(volatile uint32_t *)0x40012004UL)
#define ADC1_CR2   (*(volatile uint32_t *)0x40012008UL)
#define ADC1_SMPR2 (*(volatile uint32_t *)0x40012010UL)
#define ADC1_SQR1  (*(volatile uint32_t *)0x4001202CUL)
#define ADC1_SQR3  (*(volatile uint32_t *)0x40012034UL)
#define ADC1_DR    (*(volatile uint32_t *)0x4001204CUL)

/* Common ADC control register. */
#define ADC_CCR (*(volatile uint32_t *)0x40012304UL)

/* Control bits. */
#define ADC_ADON     (1U << 0)
#define ADC_SWSTART  (1U << 30)
#define ADC_EOC      (1U << 1)

/* Software timeout for basic polling. */
#define ADC_TIMEOUT_ITERATIONS 100000U

void ADCReader_Init(void)
{
    /* Enable GPIOA and ADC1 clocks. */
    RCC_AHB1ENR |= (1U << 0);
    RCC_APB2ENR |= (1U << 8);

    (void)RCC_AHB1ENR;
    (void)RCC_APB2ENR;

    /* Configure PA0 in analog mode. */
    GPIOA_MODER |= (3U << 0);

    /* Disable pull-up and pull-down on PA0. */
    GPIOA_PUPDR &= ~(3U << 0);

    /* Disable ADC before configuration. */
    ADC1_CR2 = 0U;

    /* Default 12-bit resolution, single conversion. */
    ADC1_CR1 = 0U;

    /* ADC clock prescaler: PCLK2 / 2. */
    ADC_CCR &= ~(3U << 16);

    /*
     * Channel 0 sample time:
     * 480 ADC clock cycles.
     */
    ADC1_SMPR2 &= ~(7U << 0);
    ADC1_SMPR2 |= (7U << 0);

    /* One regular conversion. */
    ADC1_SQR1 &= ~(15U << 20);

    /* Select channel 0. */
    ADC1_SQR3 &= ~(31U << 0);

    /* Enable ADC1. */
    ADC1_CR2 |= ADC_ADON;

    /* Allow ADC startup time. */
    for (volatile uint32_t i = 0U; i < 1000U; i++)
    {
        /* Startup settling delay. */
    }
}

bool ADCReader_Read(uint16_t *adc_value)
{
    if (adc_value == 0)
        return false;

    *adc_value = 0U;

    /* Discard any previous conversion result. */
    (void)ADC1_DR;

    /* Start a software-triggered conversion. */
    ADC1_CR2 |= ADC_SWSTART;

    uint32_t timeout = ADC_TIMEOUT_ITERATIONS;

    /* Wait for end of conversion. */
    while ((ADC1_SR & ADC_EOC) == 0U)
    {
        if (timeout-- == 0U)
            return false;
    }

    /* Read 12-bit conversion result. */
    *adc_value = (uint16_t)(ADC1_DR & 0x0FFFU);

    return true;
}
