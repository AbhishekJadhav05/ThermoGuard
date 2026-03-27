#include <LPC17xx.h>

#define DT_CTRL  0x000000F0     // P0.4–P0.7 (LCD D4–D7)
#define RS_CTRL  (1 << 8)       // P0.8 (RS)
#define EN_CTRL  (1 << 9)       // P0.9 (EN)
#define LED_PIN  (1 << 11)      // P0.11 (LED output)
#define PIR_PIN  (1 << 15)      // P0.15 (PIR input)

void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned int);

char msg_on1[]  = "LED:ON ";
char msg_on2[]  = "OBJECT:YES";
char msg_off1[] = "LED:OFF";
char msg_off2[] = "OBJECT:NO ";

unsigned long int init_command[] = {0x33,0x32,0x28,0x0C,0x06,0x01,0x80};
unsigned long int temp1, temp2;
unsigned int i, flag1;

int main(void)
{
    unsigned long int x;

    SystemInit();
    SystemCoreClockUpdate();

    // Configure pins
    LPC_PINCON->PINSEL0 = 0;   // P0.0–P0.15 as GPIO
    LPC_PINCON->PINSEL1 = 0;

    LPC_GPIO0->FIODIR |= (DT_CTRL | RS_CTRL | EN_CTRL | LED_PIN); // outputs
    LPC_GPIO0->FIODIR &= ~PIR_PIN;  // PIR input

    LPC_GPIO0->FIOCLR = LED_PIN;    // LED off initially

    // LCD Initialization
    flag1 = 0;
    for (i = 0; i < 7; i++) {
        temp1 = init_command[i];
        lcd_write();
        delay_lcd(30000);
    }

    // Initial message: LED OFF / OBJECT NO
    flag1 = 1;
    for (i = 0; msg_off1[i] != '\0'; i++) { temp1 = msg_off1[i]; lcd_write(); }
    flag1 = 0; temp1 = 0xC0; lcd_write();
    flag1 = 1;
    for (i = 0; msg_off2[i] != '\0'; i++) { temp1 = msg_off2[i]; lcd_write(); }

    // Main loop
    while (1)
    {
        x = LPC_GPIO0->FIOPIN & PIR_PIN;
        if (x) // Motion detected
        {
            LPC_GPIO0->FIOSET = LED_PIN;

            // Update LCD: LED ON / OBJECT YES
            flag1 = 0; temp1 = 0x80; lcd_write();
            flag1 = 1;
            for (i = 0; msg_on1[i] != '\0'; i++) { temp1 = msg_on1[i]; lcd_write(); }

            flag1 = 0; temp1 = 0xC0; lcd_write();
            flag1 = 1;
            for (i = 0; msg_on2[i] != '\0'; i++) { temp1 = msg_on2[i]; lcd_write(); }
        }
        else // No motion
        {
            LPC_GPIO0->FIOCLR = LED_PIN;

            // Update LCD: LED OFF / OBJECT NO
            flag1 = 0; temp1 = 0x80; lcd_write();
            flag1 = 1;
            for (i = 0; msg_off1[i] != '\0'; i++) { temp1 = msg_off1[i]; lcd_write(); }

            flag1 = 0; temp1 = 0xC0; lcd_write();
            flag1 = 1;
            for (i = 0; msg_off2[i] != '\0'; i++) { temp1 = msg_off2[i]; lcd_write(); }
        }
        delay_lcd(50000);
    }
}

void lcd_write(void)
{
    // Send higher nibble
    temp2 = temp1 & 0xF0;   // mask high nibble
    temp2 >>= 4;            // move it to lower nibble position
    temp2 <<= 4;            // place it on P0.4–P0.7
    port_write();

    // Send lower nibble
    temp2 = temp1 & 0x0F;   // mask low nibble
    temp2 <<= 4;            // place it on P0.4–P0.7
    port_write();
}

void port_write(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL;      // clear data bits
    LPC_GPIO0->FIOSET = temp2;        // set new data bits

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL;  // command
    else
        LPC_GPIO0->FIOSET = RS_CTRL;  // data

    LPC_GPIO0->FIOSET = EN_CTRL;      // pulse enable
    delay_lcd(10000);
    LPC_GPIO0->FIOCLR = EN_CTRL;
    delay_lcd(10000);
}

void delay_lcd(unsigned int r1)
{
    unsigned int r;
    for (r = 0; r < r1; r++);
}
