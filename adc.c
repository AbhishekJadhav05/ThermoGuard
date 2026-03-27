#include <LPC17xx.h>

#define LED_PIN 11
#define ADC_CHANNEL 0
#define TEMP_THRESHOLD 372  // corresponds to ~30°C

void ADC_Init(void);
uint16_t ADC_Read(void);
void delay_ms(uint32_t ms);

int main(void) {
    uint16_t adc_value;

    SystemInit();

    // Configure P0.11 as GPIO output (LED)
    LPC_PINCON->PINSEL0 &= ~(3 << 22);  // P0.11 as GPIO
    LPC_GPIO0->FIODIR |= (1 << LED_PIN); // Output

    ADC_Init();

    while (1) {
        adc_value = ADC_Read();

        if (adc_value > TEMP_THRESHOLD)
            LPC_GPIO0->FIOSET = (1 << LED_PIN); // LED ON
        else
            LPC_GPIO0->FIOCLR = (1 << LED_PIN); // LED OFF

        delay_ms(500);
    }
}

void ADC_Init(void) {
    LPC_PINCON->PINSEL1 &= ~(3 << 14);
    LPC_PINCON->PINSEL1 |=  (1 << 14);   // P0.23 as AD0.0
    LPC_SC->PCONP |= (1 << 12);          // Power up ADC
    LPC_ADC->ADCR = (1 << ADC_CHANNEL) | // Select AD0.0
                    (4 << 8) |           // ADC clock = PCLK/5
                    (1 << 21);           // Enable ADC
}

uint16_t ADC_Read(void) {
    LPC_ADC->ADCR |= (1 << 24);          // Start conversion
    while ((LPC_ADC->ADGDR & (1U << 31)) == 0); // Wait for DONE
    uint16_t result = (LPC_ADC->ADGDR >> 4) & 0xFFF; // 12-bit result
    return result;
}

void delay_ms(uint32_t ms) {
    uint32_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 5000; j++);
}
