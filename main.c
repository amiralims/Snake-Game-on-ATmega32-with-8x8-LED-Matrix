/**
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  Firmware for a classic Snake game on an ATmega32 microcontroller,
 *                  driving an 8x8 LED dot matrix display. This project was developed
 *                  for a university Microprocessors course.
 *
 *        Version:  1.0
 *        Created:  Fall 2018 (Original Project Date: Nimsal Aval Sal 1397)
 *       Compiler:  AVR-GCC
 *
 *         Author:  Amirali Mirsajadi
 *
 * =====================================================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// --- Port Definitions for LED Matrix ---
// Note: In the original report, these were lowercase. Uppercase is standard for macros.
#define DDROW   DDRD
#define DDCOL   DDRB
#define ROW     PORTD
#define COL     PORTB

// --- Port Definitions for Push Buttons ---
#define UP_BUTTON      (PINC & (1 << PINC0))
#define RIGHT_BUTTON   (PINC & (1 << PINC1))
#define LEFT_BUTTON    (PINC & (1 << PINC2))
#define DOWN_BUTTON    (PINC & (1 << PINC3))

// --- Global Variables ---
unsigned int snake[64][2] = {{0,0}, {0,1}};
unsigned int snake_len = 2;
unsigned int apple[2];
unsigned int gameON = 0;
unsigned int speed = 200;
unsigned char button = 'R'; // Snake starts moving Right
unsigned int collision = 0;
volatile unsigned int snake_counter = 0; // Volatile because it's used in an ISR

// --- Function Prototypes ---
void DotGoRight(unsigned int dot[2]);
void DotGoLeft(unsigned int dot[2]);
void DotGoUp(unsigned int dot[2]);
void DotGoDown(unsigned int dot[2]);
void move(unsigned char c);
void GetRandom(void);
void SnakeGrow(void); // Corrected typo from "SankeGrow"
void collisionCheck(void);
void gameover(void);
void play(void);
unsigned char control(void);
void clearLED(void);
void setup(void);

// --- Main Program ---
int main(void) {
    setup(); // Initialize all hardware
    
    while (1) {
        if (collision == 0) {
            play();
        } else {
            gameover();
        }
    }
    return 0; // Should never be reached
}

void setup(void) {
    // Configure Ports
    DDRC = 0x00; // PORTC is input for buttons
    PORTC = 0xFF; // Enable pull-up resistors

    // Configure Timer0 for display multiplexing (~2ms overflow)
    // Mode: Normal, Prescaler: 64 (CS01 and CS00 set)
    // In original report TCCR0=0x03. (Prescaler 64 for 8MHz clock)
    TCCR0 = (1 << CS01) | (1 << CS00);
    TCNT0 = 0x00;
    TIMSK |= (1 << TOIE0); // Enable Timer0 Overflow Interrupt

    // Configure Timer2 for random number generation
    // Mode: CTC, Prescaler: 1024 (TCCR2=0x09 is not a standard value)
    // Let's assume CTC mode with a prescaler. This sets up a free-running counter.
    TCCR2 = (1 << WGM21) | (1 << CS22) | (1 << CS21) | (1 << CS20); // CTC mode, 1024 prescaler
    TCNT2 = 0;
    OCR2 = 63; // Counter will count from 0 to 63 repeatedly

    // Configure ADC for speed control
    ADMUX = (1 << REFS0) | (1 << ADLAR); // AVCC reference, Left Adjust result (ADCH contains 8-bit result)
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1); // Enable ADC, Start, Auto Trigger, Interrupt Enable, 64 prescaler
    SFIOR &= ~( (1<<ADTS2) | (1<<ADTS1) | (1<<ADTS0) ); // Set to Free Running Mode

    // Enable global interrupts
    sei(); 

    // Generate the first apple
    GetRandom();
}

// --- Game Logic Functions ---
void play(void) {
    // Wait for a key press to start the game
    if (gameON == 0) {
        if (UP_BUTTON || RIGHT_BUTTON || LEFT_BUTTON || DOWN_BUTTON) {
            gameON = 1;
        }
        return; // Don't run game logic until started
    }

    _delay_ms(speed);
    button = control();
    move(button);
    SnakeGrow();
    collisionCheck();
}

void SnakeGrow(void) {
    // Check if the snake's head is on the apple
    if (snake[snake_len - 1][0] == apple[0] && snake[snake_len - 1][1] == apple[1]) {
        if (snake_len < 64) { // Prevent buffer overflow
            // The new segment is added at the position of the apple
            snake[snake_len][0] = apple[0];
            snake[snake_len][1] = apple[1];
            snake_len++;
            GetRandom(); // Generate a new apple
        }
    }
}

void collisionCheck(void) {
    // Check for collision with self
    if (snake_len > 4) {
        for (unsigned int j = 0; j < snake_len - 2; j++) {
            if (snake[j][0] == snake[snake_len - 1][0] && snake[j][1] == snake[snake_len - 1][1]) {
                collision = 1;
            }
        }
    }
}

void gameover(void) {
    // Turn all LEDs on to signify game over
    DDROW = 0xFF;
    DDCOL = 0xFF;
    ROW = 0xFF;
    COL = 0x00;
    // Game freezes here
}


// --- Snake Movement Functions ---
void move(unsigned char c) {
    unsigned int j;
    unsigned int copysnake[64][2];
    
    // Create a copy of the snake's current state
    for (j = 0; j < snake_len; j++) {
        copysnake[j][0] = snake[j][0];
        copysnake[j][1] = snake[j][1];
    }
    
    // Move the head in the chosen direction
    switch (c) {
        case 'R': DotGoRight(snake[snake_len - 1]); break;
        case 'L': DotGoLeft(snake[snake_len - 1]);  break;
        case 'U': DotGoUp(snake[snake_len - 1]);    break;
        case 'D': DotGoDown(snake[snake_len - 1]);  break;
        default: return;
    }

    // Shift the body to follow the head
    for (j = 0; j < snake_len - 1; j++) {
        snake[j][0] = copysnake[j + 1][0];
        snake[j][1] = copysnake[j + 1][1];
    }
}

void DotGoRight(unsigned int dot[2]) {
    if (dot[1] < 7) {
        dot[1]++;
    } else {
        dot[1] = 0; // Wrap around
    }
}

void DotGoLeft(unsigned int dot[2]) {
    if (dot[1] > 0) {
        dot[1]--;
    } else {
        dot[1] = 7; // Wrap around
    }
}

void DotGoUp(unsigned int dot[2]) {
    if (dot[0] > 0) {
        dot[0]--;
    } else {
        dot[0] = 7; // Wrap around
    }
}

void DotGoDown(unsigned int dot[2]) {
    if (dot[0] < 7) {
        dot[0]++;
    } else {
        dot[0] = 0; // Wrap around
    }
}

// --- Input & Random Generation ---
unsigned char control(void) {
    if (UP_BUTTON && button != 'D') return 'U';
    if (RIGHT_BUTTON && button != 'L') return 'R';
    if (LEFT_BUTTON && button != 'R') return 'L';
    if (DOWN_BUTTON && button != 'U') return 'D';
    return button; // Return current direction if no valid new input
}

void GetRandom(void) {
    unsigned int appleonsnake = 1;
    unsigned int rand, j;
    unsigned int rand2d[2];

    while (appleonsnake) {
        appleonsnake = 0;
        rand = TCNT2; // Use the free-running Timer2 for a pseudo-random number
        rand2d[0] = rand >> 3; // rand / 8
        rand2d[1] = rand & 0x07; // rand % 8
        
        for (j = 0; j < snake_len; j++) {
            if (snake[j][0] == rand2d[0] && snake[j][1] == rand2d[1]) {
                appleonsnake = 1;
                // Simple way to avoid getting stuck if random is not changing
                TCNT2++; 
                break;
            }
        }
    }
    apple[0] = rand2d[0];
    apple[1] = rand2d[1];
}


// --- Interrupt Service Routines ---
void clearLED(void) {
    DDROW = 0x00;
    DDCOL = 0x00;
}

// ISR for Timer0 Overflow - Used for LED Matrix Multiplexing
// The original used "interrupt [12]", the standard AVR-GCC syntax is ISR()
ISR(TIMER0_OVF_vect) {
    clearLED();
    
    if (gameON == 1) {
        if (snake_counter < snake_len) {
            // Display a segment of the snake
            DDROW = 0xFF;
            DDCOL = 0xFF;
            ROW = (1 << snake[snake_counter][0]);
            COL = ~(1 << snake[snake_counter][1]);
        } else if (snake_counter == snake_len) {
            // Display the apple
            DDROW = 0xFF;
            DDCOL = 0xFF;
            ROW = (1 << apple[0]);
            COL = ~(1 << apple[1]);
        }
        
        snake_counter++;
        
        if (snake_counter > snake_len) {
            snake_counter = 0;
        }
    }
    // TCNT0 is automatically cleared in some modes, but it's good practice to reset if needed
    // TCNT0 = 0x00; // Reset for next cycle if not using CTC
}

// ISR for ADC Conversion Complete - Used for Speed Control
// The original used "interrupt [17]", the standard AVR-GCC syntax is ISR()
ISR(ADC_vect) {
    // The result is left-adjusted, so ADCH has the 8 most significant bits
    speed = ADCH + 70; // Map ADC value (0-255) to a delay (70-325ms)
    // The ADC is in free-running mode, so it will start the next conversion automatically.
}