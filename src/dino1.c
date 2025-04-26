

#include <lpc214x.h>
#include <stdlib.h>

/* Define standard integer types that aren't available */
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int     int32_t;

/* Macros for better readability */
#define BIT(x) (1 << (x))
#define DELAY_SHORT for(i = 0; i < 5000; i++)    
#define DELAY_MEDIUM for(i = 0; i < 10000; i++)  
#define DELAY_LONG for(i = 0; i < 20000; i++)  

/* LCD Pin Definitions */
#define LCD_DATA_PORT 0       // P0.0-P0.7 for data
#define LCD_RS BIT(8)         // Register Select pin
#define LCD_RW BIT(9)         // Read/Write pin
#define LCD_EN BIT(10)        // Enable pin
#define JUMP_BUTTON BIT(11)   // Jump button input pin

/* LCD Commands */
#define LCD_CMD_CLEAR       0x01
#define LCD_CMD_HOME        0x02
#define LCD_CMD_MODE        0x06  // Auto-increment cursor
#define LCD_CMD_DISPLAY_ON  0x0C  // Display on, cursor off
#define LCD_CMD_8BIT_2LINE  0x38  // 8-bit mode, 2 lines, 5x7 font
#define LCD_CMD_CGRAM_ADDR  0x40  // Start address for custom characters
#define LCD_CMD_LINE1       0x80  // First line address
#define LCD_CMD_LINE2       0xC0  // Second line address

/* Game Constants */
#define MAX_CACTUS_POS      15    // Furthest cactus position
#define DINO_POS            0     // Dinosaur position (column)

/* Custom Character Addresses */
#define CHAR_DINO          0
#define CHAR_CACTUS_SMALL  1
#define CHAR_CACTUS_BIG    2

/* Game State */
typedef enum {
    STATE_RUNNING,
    STATE_GAME_OVER
} GameState;

typedef enum {
    JUMP_NONE,
    JUMP_RISING,
    JUMP_FALLING
} JumpState;

/* Global Variables */
volatile uint32_t i;
volatile uint16_t score = 0;
volatile JumpState jump_state = JUMP_NONE;
volatile GameState game_state = STATE_RUNNING;
volatile uint8_t cactus_pos = MAX_CACTUS_POS;
volatile uint8_t cactus_type = CHAR_CACTUS_SMALL;
volatile uint16_t jump_counter = 0;
volatile uint8_t jump_scored = 0;  // Flag to track if current jump has been scored

/* Custom Character Data */
const uint8_t dino_char[8] = {
    0x07, 0x05, 0x07, 0x16, 
    0x1F, 0x1E, 0x0E, 0x04
};

const uint8_t cactus_small[8] = {
    0x04, 0x05, 0x15, 0x15, 
    0x17, 0x1C, 0x04, 0x00
};

const uint8_t cactus_big[8] = {
    0x00, 0x04, 0x05, 0x15, 
    0x16, 0x0C, 0x04, 0x04
};

/* Function Prototypes */
void system_init(void);
void lcd_init(void);
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_send_string(const char *str);
void lcd_create_custom_char(uint8_t location, const uint8_t *pattern);
void lcd_clear_line(uint8_t line);
void game_init(void);
void update_score(void);
void display_score(void);
void move_cactus(void);
void handle_jump(void);
void check_collision(void);
void check_successful_jump(void);
void game_over(void);
void display_game_screen(void);

/**
 * Main Function 
 */
int main(void) {
    system_init();
    lcd_init();
    game_init();
    
    while (1) {
        switch (game_state) {
            case STATE_RUNNING:
                display_game_screen();
                move_cactus();
                handle_jump();
                check_successful_jump();
                
                // Check for button press to initiate jump
                if ((IO0PIN & JUMP_BUTTON) && jump_state == JUMP_NONE) {
                    jump_state = JUMP_RISING;
                    jump_counter = 0;
                    jump_scored = 0;  // Reset jump score flag
                }
                
                check_collision();
                DELAY_MEDIUM;
                break;
                
            case STATE_GAME_OVER:
                game_over();
                // Wait for reset or implement restart function
                break;
        }
    }
}

/**
 * Initialize system hardware
 */
void system_init(void) {
    // Configure I/O pins for LCD
    IO0DIR |= 0xFF | LCD_RS | LCD_RW | LCD_EN;  // Set data pins and control pins as output
    
    // Configure jump button pin as input with pull-up
    IO0DIR &= ~JUMP_BUTTON;
}

/**
 * Initialize LCD display
 */
void lcd_init(void) {
    // Wait for LCD to power up
    DELAY_LONG;
    
    // Initialize LCD in 8-bit mode, 2 lines
    lcd_send_cmd(LCD_CMD_8BIT_2LINE);
    lcd_send_cmd(LCD_CMD_DISPLAY_ON);
    lcd_send_cmd(LCD_CMD_MODE);
    lcd_send_cmd(LCD_CMD_CLEAR);
    
    // Create custom characters
    lcd_create_custom_char(CHAR_DINO, dino_char);
    lcd_create_custom_char(CHAR_CACTUS_SMALL, cactus_small);
    lcd_create_custom_char(CHAR_CACTUS_BIG, cactus_big);
    
    DELAY_SHORT;
}

/**
 * Send command to LCD
 */
void lcd_send_cmd(uint8_t cmd) {
    // Clear data port and set new command
    IO0PIN &= ~0xFF;
    IO0PIN |= cmd;
    
    // Set control signals for command mode
    IO0CLR = LCD_RS;  // RS = 0 (Command mode)
    IO0CLR = LCD_RW;  // RW = 0 (Write mode)
    
    // Generate pulse on EN pin
    IO0SET = LCD_EN;
    DELAY_SHORT;
    IO0CLR = LCD_EN;
    DELAY_SHORT;
}

/**
 * Send data to LCD
 */
void lcd_send_data(uint8_t data) {
    // Clear data port and set new data
    IO0PIN &= ~0xFF;
    IO0PIN |= data;
    
    // Set control signals for data mode
    IO0SET = LCD_RS;  // RS = 1 (Data mode)
    IO0CLR = LCD_RW;  // RW = 0 (Write mode)
    
    // Generate pulse on EN pin
    IO0SET = LCD_EN;
    DELAY_SHORT;
    IO0CLR = LCD_EN;
    DELAY_SHORT;
}

/**
 * Send string to LCD
 */
void lcd_send_string(const char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

/**
 * Create custom character in CGRAM
 */
void lcd_create_custom_char(uint8_t location, const uint8_t *pattern) {
    uint8_t i;
    
    // Set CGRAM address (8 bytes per character)
    lcd_send_cmd(LCD_CMD_CGRAM_ADDR + (location * 8));
    
    // Send pattern data
    for (i = 0; i < 8; i++) {
        lcd_send_data(pattern[i]);
    }
    
    // Return to DDRAM address
    lcd_send_cmd(LCD_CMD_LINE1);
}

/**
 * Clear specific line on LCD
 */
void lcd_clear_line(uint8_t line) {
    uint8_t i;
    
    // Set cursor to beginning of specified line
    lcd_send_cmd(line == 1 ? LCD_CMD_LINE1 : LCD_CMD_LINE2);
    
    // Write spaces to clear the line
    for (i = 0; i < 16; i++) {
        lcd_send_data(' ');
    }
}

/**
 * Initialize game state
 */
void game_init(void) {
    score = 0;
    jump_state = JUMP_NONE;
    game_state = STATE_RUNNING;
    cactus_pos = MAX_CACTUS_POS;
    cactus_type = CHAR_CACTUS_SMALL;
    jump_scored = 0;
    
    // Display initial game screen
    lcd_send_cmd(LCD_CMD_CLEAR);
    lcd_send_cmd(LCD_CMD_LINE1);
    lcd_send_string("Score: 0       ");
    
    // Display dino at starting position
    lcd_send_cmd(LCD_CMD_LINE2 + DINO_POS);
    lcd_send_data(CHAR_DINO);
}

/**
 * Update the score when a successful jump occurs
 */
void update_score(void) {
    // Increment score for successful jump
    score++;
    display_score();
}

/**
 * Display current score on LCD
 */
void display_score(void) {
    char score_str[6];
    
    // Convert score to string
    score_str[0] = ' ';
    score_str[1] = (score / 1000) % 10 + '0';
    score_str[2] = (score / 100) % 10 + '0';
    score_str[3] = (score / 10) % 10 + '0';
    score_str[4] = (score % 10) + '0';
    score_str[5] = '\0';
    
    // Display updated score
    lcd_send_cmd(LCD_CMD_LINE1 + 6);
    lcd_send_string(score_str);
}

/**
 * Move cactus obstacle
 */
void move_cactus(void) {
    // Clear current cactus position
    if (cactus_pos <= MAX_CACTUS_POS) {
        lcd_send_cmd(LCD_CMD_LINE2 + cactus_pos);
        lcd_send_data(' ');
    }
    
    // Move cactus left
    cactus_pos--;
    
    // Check if cactus has moved off screen
    if (cactus_pos < 0) {
        cactus_pos = MAX_CACTUS_POS;
        
        // Choose cactus type randomly
        cactus_type = (rand() % 2 == 0) ? CHAR_CACTUS_SMALL : CHAR_CACTUS_BIG;
    }
    
    // Draw cactus at new position
    if (cactus_pos <= MAX_CACTUS_POS) {
        lcd_send_cmd(LCD_CMD_LINE2 + cactus_pos);
        lcd_send_data(cactus_type);
    }
}

/**
 * Handle dinosaur jumping logic
 */
void handle_jump(void) {
    // Handle different jump states
    switch (jump_state) {
        case JUMP_NONE:
            // Dino on ground
            lcd_send_cmd(LCD_CMD_LINE2 + DINO_POS);
            lcd_send_data(CHAR_DINO);
            break;
            
        case JUMP_RISING:
            // Clear dino from ground
            lcd_send_cmd(LCD_CMD_LINE2 + DINO_POS);
            lcd_send_data(' ');
            
            // Draw dino in air
            lcd_send_cmd(LCD_CMD_LINE1 + DINO_POS);
            lcd_send_data(CHAR_DINO);
            
            // Increment jump counter
            jump_counter++;
            
            // Start falling after certain time
            if (jump_counter >= 3) {
                jump_state = JUMP_FALLING;
            }
            break;
            
        case JUMP_FALLING:
            // Clear dino from air
            lcd_send_cmd(LCD_CMD_LINE1 + DINO_POS);
            lcd_send_data(' ');
            
            // Draw dino on ground
            lcd_send_cmd(LCD_CMD_LINE2 + DINO_POS);
            lcd_send_data(CHAR_DINO);
            
            // Reset jump state
            jump_state = JUMP_NONE;
            break;
    }
}

/**
 * Check if jump was successful (jumped over a cactus)
 */
void check_successful_jump(void) {
    // Check if dino is jumping and cactus is at the dino position
    if (jump_state != JUMP_NONE && cactus_pos == DINO_POS && !jump_scored) {
        // Mark this jump as scored
        jump_scored = 1;
        // Increase score for successful jump
        update_score();
    }
}

/**
 * Check for collision between dinosaur and cactus
 */
void check_collision(void) {
    // Collision occurs when cactus is at dino position and dino is not jumping
    if (cactus_pos == DINO_POS && jump_state == JUMP_NONE) {
        game_state = STATE_GAME_OVER;
    }
}

/**
 * Display game over screen
 */
void game_over(void) {
	char score_str[5];
    // Display game over message
    lcd_clear_line(1);
    lcd_send_cmd(LCD_CMD_LINE1);
    lcd_send_string("  GAME  OVER   ");
    
    // Display final score
    lcd_clear_line(2);
    lcd_send_cmd(LCD_CMD_LINE2);
    lcd_send_string("Score: ");
    
    // Convert score to string
    
    score_str[0] = (score / 1000) % 10 + '0';
    score_str[1] = (score / 100) % 10 + '0';
    score_str[2] = (score / 10) % 10 + '0';
    score_str[3] = (score % 10) + '0';
    score_str[4] = '\0';
    
    lcd_send_string(score_str);
    
    // Wait a moment
    DELAY_LONG;
    DELAY_LONG;
}

/**
 * Display main game screen
 */
void display_game_screen(void) {
    // Update score display if needed
    static uint16_t last_displayed_score = 0;
    if (score != last_displayed_score) {
        display_score();
        last_displayed_score = score;
    }
}