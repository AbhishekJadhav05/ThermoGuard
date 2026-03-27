#include <LPC17xx.h>
#include <stdio.h>

#define DT_CTRL  0x000000F0     // P0.4–P0.7 (LCD D4–D7)
#define RS_CTRL  (1 << 8)       // P0.8 (RS)
#define EN_CTRL  (1 << 9)       // P0.9 (EN)
#define PIR_PIN  (1 << 15)      // P0.15 (PIR input)
#define ADC_CHANNEL 0
#define TEMP_THRESHOLD 30  // corresponds to ~30°C
#define TEMP_ADJUST 50
#define BYPASS_PIR 0
#define BYPASS_PIR_VALUE 0

void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned int);

void ADC_Init(void);
uint16_t ADC_Read(void);
void delay_ms(uint32_t ms);


char msg_on1[]  = "INTRUSION: ";
char msg_values[15];
char msg_off1[] = "DETECTING: ";

unsigned long int init_command[] = {0x33,0x32,0x28,0x0C,0x06,0x01,0x80};
unsigned long int temp1, temp2;
unsigned int i, flag1;

int main(void)
{
		int sumTemp = 0;
		int tickrate=0;
    int ifDetectedPIR=0;
	  int ifDetectedTEMP=0;
		uint16_t adc_value=0;
		int temperature_c=0;
	
    SystemInit();
    SystemCoreClockUpdate();

    // Configure pins
    LPC_PINCON->PINSEL0 = 0;   // P0.0–P0.15 as GPIO
    LPC_PINCON->PINSEL1 = 0;
		LPC_GPIO0->FIODIR |= (DT_CTRL | RS_CTRL | EN_CTRL); // outputs

    LPC_GPIO0->FIODIR &= ~PIR_PIN;  // PIR input

    // LCD Initialization
    flag1 = 0;
    for (i = 0; i < 7; i++) {
        temp1 = init_command[i];
        lcd_write();
        delay_lcd(30000);
    }
		ADC_Init();

    // Initial message
    flag1 = 1;
    for (i = 0; msg_off1[i] != '\0'; i++) { temp1 = msg_off1[i]; lcd_write(); }

    // Main loop
    while (1)
    {
				sumTemp = 0;
				tickrate = 0;
			  while(tickrate<5){
				  	adc_value = ADC_Read();
					sumTemp = ((adc_value / 4095.0f) * 330.0f)- TEMP_ADJUST; // Vref is 3.3, so 3.3 * 100 = 330
					delay_ms(1000);
				  	tickrate +=1;
				}
				temperature_c = sumTemp/5;
				if(temperature_c > TEMP_THRESHOLD){
					ifDetectedTEMP = 1;
				}
				else{
					ifDetectedTEMP = 0;
				}
        ifDetectedPIR = (LPC_GPIO0->FIOPIN & PIR_PIN) ? 1 : 0; // Normalize to 0 or 1
				if(BYPASS_PIR){
						ifDetectedPIR = BYPASS_PIR_VALUE;
				}
        if (ifDetectedTEMP & ifDetectedPIR) // Motion detected
        {

            // Update LCD: INTRUSION!!!!!!!!!!
            flag1 = 0; temp1 = 0x80;  lcd_write();
            flag1 = 1;
            for (i = 0; msg_on1[i] != '\0'; i++) { temp1 = msg_on1[i]; lcd_write(); }

            flag1 = 0; temp1 = 0xC0; lcd_write();
            flag1 = 1;
						sprintf(msg_values,"TEMP %d PIR %d",temperature_c,ifDetectedPIR);
            for (i = 0; msg_values[i] != '\0'; i++) { temp1 = msg_values[i]; lcd_write(); }
						delay_lcd(50000);
						delay_ms(10000);
						
        }
        else // No motion
        {
            // Update LCD: DETECTING << values
            flag1 = 0; temp1 = 0x80; lcd_write();
            flag1 = 1;
            for (i = 0; msg_off1[i] != '\0'; i++) { temp1 = msg_off1[i]; lcd_write(); }

            flag1 = 0; temp1 = 0xC0; lcd_write();
            flag1 = 1;
						sprintf(msg_values,"TEMP %d PIR %d",temperature_c,ifDetectedPIR);
            for (i = 0; msg_values[i] != '\0'; i++) { temp1 = msg_values[i]; lcd_write(); }
        }
        delay_lcd(50000);
    }
		
}

void lcd_write(void)
{

    temp2 = temp1 & 0xF0;   
    temp2 >>= 4;            
    temp2 <<= 4;            
    port_write();

    temp2 = temp1 & 0x0F;   
    temp2 <<= 4;            
    port_write();
}

void port_write(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL;      
    LPC_GPIO0->FIOSET = temp2;        

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL;  // command
    else
        LPC_GPIO0->FIOSET = RS_CTRL;  // data

    LPC_GPIO0->FIOSET = EN_CTRL;      
    delay_lcd(10000);
    LPC_GPIO0->FIOCLR = EN_CTRL;
    delay_lcd(10000);
}

void ADC_Init(void) {
    LPC_PINCON->PINSEL1 &= ~(3 << 14);
    LPC_PINCON->PINSEL1 |=  (1 << 14);   
    LPC_SC->PCONP |= (1 << 12);          
    LPC_ADC->ADCR = (1 << ADC_CHANNEL) | 
                    (4 << 8) |           
                    (1 << 21);           
}

uint16_t ADC_Read(void) {
		uint16_t result;
    LPC_ADC->ADCR |= (1 << 24);          
    while ((LPC_ADC->ADGDR & (1U << 31)) == 0); 
    result = (LPC_ADC->ADGDR >> 4) & 0xFFF; 
    return result;
}

void delay_ms(uint32_t ms) {
    uint32_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 5000; j++);
}

void delay_lcd(unsigned int r1)
{
    unsigned int r;
    for (r = 0; r < r1; r++);
}
