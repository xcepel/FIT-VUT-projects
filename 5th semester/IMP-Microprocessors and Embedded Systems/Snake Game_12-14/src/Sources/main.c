/* Name: Kateřina Čepelková  */
/* Login: xcepel03           */
/* Date: 2.12.2023           */
/* ------------------------- */

/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include <stdbool.h>


/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK	0x1Fu
#define GPIO_PIN(x)		(((1)<<(x & GPIO_PIN_MASK)))

/* Macros for buttons */
#define BTN_SW2 0x400     // Port E, bit 10
#define BTN_SW3 0x1000    // Port E, bit 12
#define BTN_SW4 0x8000000 // Port E, bit 27
#define BTN_SW5 0x4000000 // Port E, bit 26

/* Enum for snake directions */
enum Direction {
	UP,
    DOWN,
    LEFT,
    RIGHT,
	NONE
};

struct coords {
    int row; // 0-7
    int col; // 0-15
};


/* Configuration of the necessary MCU peripherals */
void SystemConfig() {
	/* Turn on all port clocks */
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;

	/* Set corresponding PTA pins (column activators of 74HC154) for GPIO functionality */
	PORTA->PCR[8] = ( 0|PORT_PCR_MUX(0x01) );  // A0
	PORTA->PCR[10] = ( 0|PORT_PCR_MUX(0x01) ); // A1
	PORTA->PCR[6] = ( 0|PORT_PCR_MUX(0x01) );  // A2
	PORTA->PCR[11] = ( 0|PORT_PCR_MUX(0x01) ); // A3

	/* Set corresponding PTA pins (rows selectors of 74HC154) for GPIO functionality */
	PORTA->PCR[26] = ( 0|PORT_PCR_MUX(0x01) );  // R0
	PORTA->PCR[24] = ( 0|PORT_PCR_MUX(0x01) );  // R1
	PORTA->PCR[9] = ( 0|PORT_PCR_MUX(0x01) );   // R2
	PORTA->PCR[25] = ( 0|PORT_PCR_MUX(0x01) );  // R3
	PORTA->PCR[28] = ( 0|PORT_PCR_MUX(0x01) );  // R4
	PORTA->PCR[7] = ( 0|PORT_PCR_MUX(0x01) );   // R5
	PORTA->PCR[27] = ( 0|PORT_PCR_MUX(0x01) );  // R6
	PORTA->PCR[29] = ( 0|PORT_PCR_MUX(0x01) );  // R7

	/* Set corresponding button pins */
    PORTE->PCR[10] = PORT_PCR_MUX(0x01); // SW2
    PORTE->PCR[12] = PORT_PCR_MUX(0x01); // SW3
    PORTE->PCR[27] = PORT_PCR_MUX(0x01); // SW4
    PORTE->PCR[26] = PORT_PCR_MUX(0x01); // SW5

	/* Set corresponding PTE pins (output enable of 74HC154) for GPIO functionality */
	PORTE->PCR[28] = ( 0|PORT_PCR_MUX(0x01) ); // #EN

	/* Change corresponding PTA port pins as outputs */
	PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);

	/* Change corresponding PTE port pins as outputs */
	PTE->PDDR = GPIO_PDDR_PDD( GPIO_PIN(28) );
}

/* Variable delay loop */
void delay(int t1, int t2)
{
	int i, j;

	for(i=0; i<t1; i++) {
		for(j=0; j<t2; j++);
	}
}

/* Conversion of requested column number into the 4-to-16 decoder control.  */
void column_select(unsigned int col_num)
{
	unsigned i, result, col_sel[4];

	for (i =0; i<4; i++) {
		result = col_num / 2;	  // Whole-number division of the input number
		col_sel[i] = col_num % 2;
		col_num = result;

		switch(i) {

			// Selection signal A0
		    case 0:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(8))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(8)));
				break;

			// Selection signal A1
			case 1:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(10))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(10)));
				break;

			// Selection signal A2
			case 2:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(6))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(6)));
				break;

			// Selection signal A3
			case 3:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(11))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(11)));
				break;

			// Otherwise nothing to do...
			default:
				break;
		}
	}
}

/* Conversion of requested row number into the corresponding number*/
void row_select(int row_pos) {
	int row[8] = {0x20000000, 0x08000000, 0x00000080, 0x10000000, 0x02000000, 0x00000200, 0x01000000, 0x04000000};
	PTA->PDOR |= GPIO_PDOR_PDO(row[row_pos]);
}

/* Reseting all display settings */
void display_clear() {
	PTA->PDOR = 0;
}

/* Returns bool if snake can move into the new position, otherwise false (out of bounds/itself) */
bool can_move(struct coords snake[5], int col, int row) {
	// out of bounds of display
	if (col < 0 || col > 15 || row < 0 || row > 7)
		return false;

	 // moving into/eating itself
	for (int i = 0; i < 5; i++) {
		if (snake[i].col == col && snake[i].row == row) {
			return false;
		}
	}

	return true;
}

/* Lights up pixels with snake + handles input*/
void display_snake(struct coords snake[5], enum Direction lastDirection, enum Direction *currentDirection) {
	for (int i = 0; i < 1000; i++) {
		// iterates through all columns and turns them on
		for (int j = 0; j < 16; j++) {
			display_clear();
			column_select(j);
			// chooses rows in column with snake and displays it
			for (int k = 0; k < 5; k++) {
				if (snake[k].col == j) {
					row_select(snake[k].row);
				}
			}
			delay(10, 1);
		}

		// Input handling - buttons
		if (!(GPIOE_PDIR & BTN_SW5) && lastDirection != DOWN)
		{
			*currentDirection = UP;
		}

		if (!(GPIOE_PDIR & BTN_SW3)  && lastDirection != UP)
		{
			*currentDirection = DOWN;
		}

		if (!(GPIOE_PDIR & BTN_SW4)  && lastDirection != RIGHT)
		{
			*currentDirection = LEFT;
		}

		if (!(GPIOE_PDIR & BTN_SW2)  && lastDirection != LEFT)
		{
			*currentDirection = RIGHT;
		}
	}
}

/* Handles movement of snake + its deaths*/
void game() {
	struct coords snake[5] = {{3, 10}, {3, 9}, {3, 8}, {3, 7}, {3, 6}}; // starting  position for snake
	int tail = 0; // for circular buffer
	int head = 4; // for circular buffer
	enum Direction lastDirection = UP; // at start - snake facing UP - forbids movement back
	enum Direction currentDirection = NONE;

	while (1) { // ENDLESS LOOP
		display_snake(snake, lastDirection, &currentDirection);

		// Changing directions according to last pressed button
		switch (currentDirection) {
			case UP:
				if (can_move(snake, snake[head].col - 1, snake[head].row)) {
					snake[tail] = snake[head];
					snake[tail].col--;
				} else {
					return;
				}
				break;
			case DOWN:
				if (can_move(snake, snake[head].col + 1, snake[head].row)) {
					snake[tail] = snake[head];
					snake[tail].col++;
				} else {
					return;
				}
				break;
			case LEFT:
				if (can_move(snake, snake[head].col, snake[head].row - 1)) {
					snake[tail] = snake[head];
					snake[tail].row--;
				} else {
					return;
				}
				break;
			case RIGHT:
				if (can_move(snake, snake[head].col, snake[head].row + 1)) {
					snake[tail] = snake[head];
					snake[tail].row++;
				} else {
					return;
				}
				break;
			case NONE: // start - no movement
				continue;
		}
		lastDirection = currentDirection;
		// Moving the circular buffer
		tail = (tail + 1) % 5;
		head = (head + 1) % 5;
	}
}

int main(void)
{
	SystemConfig();

	while(1) {
		game();
		// Game lost -> reseting snake position + game
	}

    return 0;
}
