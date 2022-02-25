#include <stdint.h>
#ifdef TEST
#include <stdio.h>
#endif

#include "encoded.h"

static uint32_t currentWord;
static const uint8_t* blobPtr;

// high byte of currentWord is always zero
void updateWord(void) {
#ifndef TEST
__asm
    ld a,(#_blobPtr)
    ld l,a
    ld a,(#_blobPtr+1)
    ld h,a
    ld a,(hl+)
    bit 7,a
    jr z,0001$
    and #127
    ld b,a
    xor a
    ld d,a
0005$:    
    ld e,a
0003$:  
;; inc edb
    ld a,b
    add #1
    ld b,a
    ld a,d
    adc #0
    ld d,a
    ld a,e
    adc #0
    ld e,a    
;; add edb to currentWord
    ld a,(#_currentWord)
    add b
    ld (#_currentWord),a
    ld a,(#_currentWord+1)
    adc d
    ld (#_currentWord+1),a
    ld a,(#_currentWord+2)
    adc e
    ld (#_currentWord+2),a
;; update blobPtr    
    ld a,l
    ld (#_blobPtr),a
    ld a,h
    ld (#_blobPtr+1),a
    ret    
0001$: 
    ld b,a
    ld a,(hl+)
    ld c,a
    and #127
    ld d,a
    srl d
    sla a
    sla a
    sla a
    sla a
    sla a
    sla a
    sla a
    or b
    ld b,a
    bit 7,c
    jr z,0002$
    xor a
    jr 0005$
0002$: 
    ld a,(hl+)
    ld e,a
    srl e
    srl e
    sla a
    sla a
    sla a
    sla a
    sla a
    sla a
    or d
    ld d,a
    jr 0003$
__endasm ;
#else
    uint8_t b = *blobPtr++;
    uint32_t v;
    v = b & 0x7F;
    if (0 == (b & 0x80)) {
        b = *blobPtr++;
        v |= (uint32_t)(b & 0x7F) << 7;
        if (0 == (b&0x80)) {
            v |= (uint32_t)*blobPtr << 14;
            blobPtr++;
        }
    }
    currentWord += v+1;
#endif    
}

void decodeWord(uint8_t start, uint32_t nextFour, char* buffer) {
    *buffer = start + 'A';
    buffer += 5;
    *buffer-- = 0;
    for(uint8_t i=1;i<5;i++) {
        *buffer-- = (nextFour & 0x1F) + 'A';
        nextFour >>= 5;
    }
}

void getWord(uint16_t n, char* buffer) {
    uint16_t count = 0;
    uint8_t i;
    const LetterBucket_t* w;
    w = buckets;
    for (i = 0 ; i < 26 && n >= w[1].wordNumber ; i++, w++) ;
    if (i == 26) {
        *buffer = 0;
        return;
    }
    currentWord = 0;
    blobPtr = wordBlob + w->blobOffset;
    for (uint16_t j = n - w->wordNumber + 1; j; j--) {
        updateWord();
    }
    decodeWord(i, currentWord, buffer);
}

uint8_t filterWord(char* s) {
    uint8_t i;
    /* because the input system uses an on-screen keyboard with A-Z only, no need to sanitize */
    /* for (i=0; i<5; i++)
        if (s[i] < 'A' || s[i] > 'Z')
            return 0; */
    uint32_t w = 0;
    for (i=1;i<5;i++)
        w = (w << 5) | (s[i]-'A');
    
    i = s[0]-'A';
    currentWord = 0;
    blobPtr = wordBlob + buckets[i].blobOffset;
    for (uint16_t j=buckets[i+1].wordNumber - buckets[i].wordNumber; j; j--) {
        updateWord();
        if (currentWord >= w) {
            return currentWord == w;
        }
    }
    return 0;
}

void getSpecialWord(uint16_t _n, char* buffer) {
    static uint16_t w;
    static uint16_t n;
    n = _n;

    static const AnswerBucket_t* bucket;
    bucket = answerBuckets;
    
    w = 0;

    while (bucket->numWords <= n) {
        n -= bucket->numWords;
        w += bucket->offsetDelta;
        bucket++;
    }
    
    static const uint8_t* b;
    b = answers + w;
    w *= 8;

    for(;;) { 
        static uint8_t c;
        c = *b++;
        if (c == 0) {
            w += 8;
        }
        else {
            static uint8_t mask;
            for (mask = 1 ; mask ; mask <<= 1) {
                if (c & mask) {
                    if (n == 0) {
                        getWord(w, buffer);
                        return;
                    }
                    n--;
                }
                w++;
            }
        }
    }
}

#ifdef TEST
main() {
    char w[6] = {0};
    for (int i=0; i<500; i++) {
        getSpecialWord(i, w);
        puts(w);
    }
    printf("%d\n", filterWord("SEREZ"));
    printf("%d\n", filterWord("SERER"));
    printf("%d\n", filterWord("ZYMIC"));
    printf("%d\n", filterWord("BAAED"));
}
#endif
