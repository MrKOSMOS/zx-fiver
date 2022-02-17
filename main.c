/* Fiver for Game Boy (akin to Wordle).
 * A weekend project by stacksmashing.
 * Contact: twitter.com/ghidraninja
 *          code@stacksmashing.net
 */
#include <gb/gb.h>
#include <gb/drawing.h>
#include <sms/hardware.h>
#include <gbdk/console.h>
#include <gbdk/font.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <rand.h>
#include <time.h>

#include "decode.h"

void set_box_color_for_letter(int position);
void set_letter_color_for_letter(int position);

const char *kb[3] = {
"Q W E R T Y U I O P",
" A S D F G H J K L",
"  Z X C V B N M"};

int kb_coords[3] = {
    10,
    9,
    7
};

int kb_offsets[3] = {
    0,
    1,
    2
};

int kb_x = 0;
int kb_y = 0;
int guess_nr;
char guess[6];
char guesses[6][5];
uint8_t guessed[26];
char word[6];
#define RIGHT_LETTER_RIGHT_PLACE  4
#define RIGHT_LETTER_WRONG_PLACE  2
#define WRONG_LETTER              1
uint8_t eval[5]; 

static void waitpaduprepeat() {
    static uint8_t firstPress = 1;
    static uint16_t delta = (uint32_t)350 * CLOCKS_PER_SEC / 1000;
    uint8_t j;
    uint16_t start = sys_time;
    while ((j = joypad()) && (uint16_t)(sys_time-start) < delta) ;
    if (j)
        delta = (uint32_t)100 * CLOCKS_PER_SEC / 1000;
    else
        delta = (uint32_t)350 * CLOCKS_PER_SEC / 1000;
}

uint8_t contains(char *str, char c) {
    uint8_t count = 0;
    while(*str) {
        if(*str == c) {
            count++;
        } 
        str++;
    }
    return count;
}

void evaluate_letters(char* guess) {
    for (uint8_t i=0;i<5;i++) {
        char c = guess[i];
        if (word[i] == c) { 
            eval[i] = RIGHT_LETTER_RIGHT_PLACE;
            guessed[c-'A'] = RIGHT_LETTER_RIGHT_PLACE;
        }
        else {
            eval[i] = WRONG_LETTER;
            guessed[c-'A'] |= WRONG_LETTER;
        }
    }
    for (uint8_t i=0;i<5;i++) {
        if (eval[i] == WRONG_LETTER) {
            char c = guess[i];
            uint8_t count = contains(word, c);
            if (count) {
                uint8_t already = 0;
                for (uint8_t j=0;j<5;j++) {
                    if (eval[j] && guess[j] == c)
                        already++;
                }
                if (already < count) {
                    eval[i] = RIGHT_LETTER_WRONG_PLACE;
                    guessed[c-'A'] |= RIGHT_LETTER_WRONG_PLACE;
                }
            }            
        }
    }
}

void draw_word_rect(int x, int y, char *guess) {
    int gx = x/8;
    int gy = y/8;
    x -= 3;
    y -= 4;
    if (guess) {
        evaluate_letters(guess);
    }
    for(uint8_t i=0; i < 5; i++) {
        if(guess) {
            char letter = guess[i];
            set_box_color_for_letter(i);
            box(x, y, x+14, y+14, M_FILL);
            set_letter_color_for_letter(i);
            gotogxy(gx, gy);
            wrtchr(letter);
            gx += 2;
        } else {
            color(BLACK, WHITE, M_NOFILL);
            box(x, y, x+14, y+14, M_NOFILL);
        }
        
        x += 16;
    }
}




void set_box_color_for_letter(int position) {
    if(eval[position] == RIGHT_LETTER_RIGHT_PLACE) {
        color(BLACK, BLACK, M_FILL);
    } else if(eval[position] == RIGHT_LETTER_WRONG_PLACE) {
        color(BLACK, DKGREY, M_FILL);
    } else {
        color(BLACK, WHITE, M_NOFILL);
    }
}

void set_letter_color_for_letter(int position) {
    if(eval[position] == RIGHT_LETTER_RIGHT_PLACE) {
        color(WHITE, BLACK, M_NOFILL);
    } else if(eval[position] == RIGHT_LETTER_WRONG_PLACE) {
        color(WHITE, DKGREY, M_NOFILL);
    } else {
        color(BLACK, WHITE, M_NOFILL);
    }
}


void set_color_for_letter(char letter) {
    if(letter == ' ') {
        color(BLACK, WHITE, M_NOFILL);
    }
    else {
        uint8_t g = guessed[letter-'A'];
        if (g & RIGHT_LETTER_RIGHT_PLACE) {
            color(DKGREY, WHITE, M_NOFILL);
        } else if(g & RIGHT_LETTER_WRONG_PLACE) {
            color(DKGREY, WHITE, M_NOFILL);
        } else if(g & WRONG_LETTER) {
            color(LTGREY, WHITE, M_NOFILL);
        } else {
            color(BLACK, WHITE, M_NOFILL);
        }
    }
}

int kb_vert_offset = 15;

void erase_keyboard() {
    color(WHITE, WHITE, SOLID);
    box(0, kb_vert_offset*8-1, 160, 144, M_FILL);
}

void draw_keyboard(int x, int y) {
    for(int i=0; i < 3; i++) {
        gotogxy(x, y + i);
        int kbl = strlen(kb[i]);
        for(int j=0; j < kbl; j++) {
            char letter = kb[i][j];
            set_color_for_letter(letter);
            wrtchr(letter);
        }
    }
}

void highlight_key() {
    int x = (kb_x * 16) + (kb_offsets[kb_y] * 8);
    int y = (kb_vert_offset + kb_y) * 8;
    color(BLACK, WHITE, M_NOFILL);
    box(x, y-1, x+8, y+7, M_NOFILL);
}

void dehighlight_key() {
    int x = (kb_x * 16) + (kb_offsets[kb_y] * 8);
    int y = (kb_vert_offset + kb_y) * 8;
    color(WHITE, WHITE, M_NOFILL);
    box(x, y-1, x+8, y+7, M_NOFILL);


    int gx = (kb_x * 2) + (kb_offsets[kb_y]);
    int gy = (kb_vert_offset + kb_y);
    gotogxy(gx, gy);
    char letter = kb[kb_y][kb_offsets[kb_y]+ (kb_x*2)];
    set_color_for_letter(letter);
    wrtchr(letter);
}

char getletter() {
    return kb[kb_y][kb_offsets[kb_y] + (kb_x*2)];
}

void show_answer() {
    erase_keyboard();
    int line = 2 + (6 * 2);
    int x = 5;
    for(int i=0; i < 5; i++) {
        color(BLACK, WHITE, M_NOFILL);
        gotogxy(x, line);
        wrtchr(word[i]);

        x += 2;
    }
    waitpad(J_START | J_A);
    reset();
}

void render_guess() {
    // first box is at 5, 2
    int line = 2 + (guess_nr * 2);
    int x = 5;
    for(int i=0; i < 5; i++) {
        color(BLACK, WHITE, M_NOFILL);
        gotogxy(x, line);
        if(guess[i] != 0) {
            wrtchr(guess[i]);
        } else {
            wrtchr(' ');
        }

        x += 2;
    }
}

void draw_board() {
    for(int i=0; i < 6; i++) {
        char *g = NULL;
        if(i < guess_nr) {
            g = guesses[i];
        }
        draw_word_rect(40, 16+(i*16), g);
    }

}


void show_win() {
    erase_keyboard();
    gotogxy(5, 14);
    color(BLACK, WHITE, M_NOFILL);
    gprint("You won!!!");
    gotogxy(3, 16);
    wrtchr('0'+guess_nr);
    gprint("/6 - Congrats!");
    waitpad(J_START | J_A);
    reset();
}


void run_fiver(void)
{
    strcpy(word, "EMPTY");
    int has_random = 0;
    
    guess_nr = 0;
    memset(guess, 0, 5);
    memset(guessed, 0, 26);

    for(int i=0; i < 6; i++) {
        strcpy(guesses[i], "");
    }
    for(int i=0; i < 6; i++) {
        draw_word_rect(40, 16+(i*16), NULL);
    }

    gotogxy(2, 0);
    gprint("Game Boy  FIVER");
    draw_keyboard(0, kb_vert_offset);

    color(LTGREY, WHITE, M_NOFILL);
    highlight_key();
    while(1) {
        int j = joypad();
        if((has_random == 0) && (j != 0)) {
            uint16_t n = NUM_ANSWERS;
            uint16_t seed = LY_REG;
            seed |= (uint16_t)DIV_REG << 8;
            initrand(seed);
            int r = rand();
            while(r >= NUM_ANSWERS) {
                r = rand();
            }
            getSpecialWord(r, word);
            has_random = 1;
        }

        switch(j) {
            case J_RIGHT:
                dehighlight_key();
                kb_x += 1;
                if(kb_x >= kb_coords[kb_y]) {
                    kb_x = 0;
                }
                highlight_key();
                waitpaduprepeat();
                break;
            case J_LEFT:
                dehighlight_key();
                kb_x -= 1;
                if(kb_x < 0) {
                    kb_x = kb_coords[kb_y] - 1;
                }
                highlight_key();
                waitpaduprepeat();
                break;
            case J_UP:
                dehighlight_key();
                kb_y -= 1;
                if(kb_y < 0) {
                    kb_y = 2;
                }
                if(kb_x >= kb_coords[kb_y]) {
                    kb_x = kb_coords[kb_y] - 1;
                }
                highlight_key();
                waitpaduprepeat();
                break;
            case J_DOWN:
                dehighlight_key();
                kb_y += 1;
                if(kb_y > 2) {
                    kb_y = 0;
                }
                if(kb_x >= kb_coords[kb_y]) {
                    kb_x = kb_coords[kb_y] - 1;
                }
                highlight_key();
                waitpaduprepeat();
                break;
            case J_SELECT:
            case J_START:
                if(strlen(guess) != 5) break;
                if(!filterWord(guess)) {
                    guess[4] = 0;
                    render_guess();
                    break;
                }
                strcpy(guesses[guess_nr], guess);
                guess_nr += 1;
                draw_board();
                draw_keyboard(0, kb_vert_offset);
                highlight_key();
                if(strcmp(word, guess) == 0) {
                    show_win();
                    return;
                    break;
                }
                if(guess_nr == 6) {
                    show_answer();
                    return;
                    break;
                }
                // empty guess
                memset(guess, 0, 5);
                // TODO
                break;
            case J_A:
                if(strlen(guess) == 5) break;
                guess[strlen(guess)] = getletter();
                render_guess();
                waitpaduprepeat();
                break;
            case J_B:
                if(strlen(guess) == 0) break;
                guess[strlen(guess)-1] = 0;
                render_guess();
                waitpaduprepeat();
                break;
            default:
            break;
        }

        wait_vbl_done();
    }
}

void main() {
    while(1) {
        run_fiver();    
    }
}
