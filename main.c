#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "LiquidCrystal.h"
 
 
# define _XTAL_FREQ 1000000
#define LED PORTCbits.RC1

//void __interrupt(high_priority) toggleLed(void);
void __interrupt(low_priority) adc_sample(void);
 
volatile signed int num;
volatile signed int num2;
volatile signed int glob;
int speed;
 int flag = 0;
 int counter = 1;
 
 
 
 int main() 
{
    // Configure LCD Pins
    // Data pins connected to PORTB
      PR2 =  249; 
     T2CONbits.T2CKPS = 0b00; // Prescaler 1:1
    T2CONbits.TMR2ON = 1;
   

    // setup CCP2 in PWM mode
    TRISCbits.RC1 = 0;
    CCP2CONbits.CCP2M = 0b1100;
 
     TRISD = 0x00; 
    TRISA = 0xff;
    TRISB = 0xfd;
    TRISE = 0x00;
 
    
//    INTCON = 0x00;
    // connect the LCD pins to the appropriate PORT pins
    pin_setup(&PORTD, &PORTE);
 
    // initialize the LCD to be 16x2 (this is what I have, yours might be different)
    begin(16, 2, LCD_5x8DOTS);
 
 
 INTCONbits.GIE = 1; // Global interrupt enable
    INTCONbits.INT0E = 1; // Enable INT0
    INTCONbits.INT0IF = 0; // reset INT0 flag
    INTCON2bits.INTEDG0 = 0; // falling edge
    // INTCON2bits.INTEDG0 = 1; // rising edge
   // INTCON3bits.INT1IE = 1; // Enable INT1
   // INTCON3bits.INT1IF = 0; // reset INT1 flag
    
    // Setup interrupt priorities
    RCONbits.IPEN = 1; // enable priority
    INTCONbits.GIEH = 1; // enable all high priority
    INTCONbits.GIEL = 1; // enable all low priority
   // INTCON3bits.INT1IP = 0; // Set INT1 to low priority 
    //--------------------------------------------------------------------------
     INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    INTCON2bits.TMR0IP = 1; // high priority 
//setup timer
     T0CONbits.PSA = 0; // Prescaler is assigned
    T0CONbits.T0PS = 0x03; // 1:16 prescale value
    T0CONbits.T0CS = 0; // clock source is internal instruction cycle
    T0CONbits.T08BIT = 1; // operate in 16 bit mode now
    T0CONbits.TMR0ON = 1; // Turn on timer
     TMR0 = 101;

    // * Configure analog pins, voltage reference and digital I/O 
    // Reference voltages are VSS and VDD
    ADCON1 = 0x0D;
    

    // * Select A/D acquisition time
    // * Select A/D conversion clock
    // Right justified, ACQT = 2 TAD, ADCS = FOSC/2
    ADCON2bits.ADCS = 0; // FOSC/2
    ADCON2bits.ACQT = 1; // ACQT = 2 TAD
    ADCON2bits.ADFM = 0; // Right justified

    // * Select A/D input channel
    ADCON0bits.CHS = 0; // Channel 0 (AN0), starting with the potentiometer
  //  ADCON0bits.CHS = 1;

    // * Turn on A/D module
    ADCON0bits.ADON = 1;   
    
    // 2 - Configure A/D interrupt (if desired)
    // * Clear ADIF bit
    // * Set ADIE bit
    // * Select interrupt priority ADIP bit
    // * Set GIE bit
    
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
    IPR1bits.ADIP = 1;
  
    INTCONbits.PEIE = 1; // enable peripheral interrupts
    // 3 - Wait the required acquisition time (if required)
    // ---> using ACQT = 2 TAD, no need to manually specify a wait    

    while(1)
    {   
       ADCON0bits.GO = 1; 
         
    
       if(flag == 0)
      {
        RB1 = 0;  //led off  
        home();
        print("Max Speed: ");
       
        //  num = (ADRESH << 8) | ADRESL;
        print_int(num);
        print("       "); //simple
       setCursor(0,1);
       print("Delay: ");
       glob = num2 * 10;
       print_int(glob);
       print("ms       "); //simple
    }
       
       else {
           home();
            RB1 = 1;
            home();
            print("STOPPED!        ");
            setCursor(0,1);
            print("                ");
            counter = 9999;
           CCPR2L = 0;
            //__delay_ms(400);
     }   
       }
           
    }
 
void __interrupt(low_priority) adc_sample(void)
{
    // test which interrupt called this interrupt service routine
 
    // ADC Interrupt
    if (PIR1bits.ADIF && PIE1bits.ADIE)
    {
        // 5 Wait for A/D conversion to complete by either
        // * Polling for the GO/Done bit to be cleared
        // * Waiting for the A/D interrupt
 
        // 6 - Read A/D result registers (ADRESH:ADRESL); clear bit ADIF, if required
 
        // reset the flag to avoid recursive interrupt calls
        PIR1bits.ADIF = 0;
       
        
        if (ADCON0bits.CHS == 0) // channel AN0 (potentiometer)
        {
            //speed = (ADRESH << 8) | ADRESL;
            
            num = ADRESH << 2 | ADRESL >> 6;
            
            // PWM Duty Cycle
     //       CCPR2L = ADRESH;
     //       CCP2CONbits.DC2B = ((ADRESL >> 6) & 0x03);
            ADCON0bits.CHS = 1;
            
           
        }
        else if (ADCON0bits.CHS == 1) // channel AN1 (photo sensor))
        {
            //delay = (ADRESH << 8) | ADRESL;
            
             //TIMER0 SET-UP
           num2 = (ADRESH << 2 | ADRESL >> 6);
            
            ADCON0bits.CHS = 0;
        } 
 
    }
    if (INTCONbits.INT0IE && INTCONbits.INT0IF)
    {
        // Turn off the interrupt flag to avoid recursive interrupt calls
        INTCONbits.INT0IF = 0;
       if(flag == 1)
        {
            flag = 0;
            
        }
        else if (flag == 0)
        {
            flag = 1;
        }
     }
  
     if (INTCONbits.TMR0IE && INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IF = 0;
        
        TMR0 = 101; // because after the interrupt this will be 0
      // needs to update fan speed in acc to timer2  CCPR2L = 0;
        if(counter <= num2)
        {
            //speed = (num / num2) * counter;
            CCPR2L = ((ADRESH *100) / num2 * counter)/100;
        //    CCP2CONbits.DC2B = ((ADRESL >> 6) & 0x03);
            counter++;
        }
        if (counter > num2)
        {
            counter = 0;
        }
    }
}

