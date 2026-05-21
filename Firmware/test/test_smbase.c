/*
 * sim65 unit test for ProDOS dispatcher's SMBASE-decode logic.
 *
 * Verifies that decode_smbase() (a verbatim copy of the @PRODOS prefix
 * in Firmware/src/ProDOS.s) maps DSNUMBER + SLOT16 to the correct
 * partition (0-7) for both ProDOS 2.5 (D/X/Y bit layout) and ProDOS 2.4
 * (phantom-slot drives 3-4) call patterns.
 *
 * Build / run:
 *   CC65_HOME=/tmp/cc65/usr/share/cc65 \
 *     /tmp/cc65/usr/bin/cl65 -t sim6502 -o test_smbase \
 *       test_smbase.c smbase_decode.s
 *   /tmp/cc65/usr/bin/sim65 test_smbase
 *
 * Exit code = number of failing cases.
 */

#include <stdio.h>

extern unsigned char zp_slot16;     /* $3E - SLOT16 */
extern unsigned char zp_dsnumber;   /* $43 - DSNUMBER */
extern unsigned char zp_smbase;     /* $4D - SMBASE */
#pragma zpsym("zp_slot16")
#pragma zpsym("zp_dsnumber")
#pragma zpsym("zp_smbase")

extern void __fastcall__ decode_smbase(void);

struct testcase {
    unsigned char slot16;
    unsigned char dsnumber;
    unsigned char expected;
    const char *desc;
};

static const struct testcase cases[] = {
    /* ProDOS 2.5 in slot 7 (SLOT16 = $70) - all 8 drives */
    { 0x70, 0x70, 0, "S7 D1 (ProDOS 2.5)" },
    { 0x70, 0xF0, 1, "S7 D2 (ProDOS 2.5)" },
    { 0x70, 0x71, 2, "S7 D3 (ProDOS 2.5, Y=1)" },
    { 0x70, 0xF1, 3, "S7 D4 (ProDOS 2.5, Y=1,D=1)" },
    { 0x70, 0x72, 4, "S7 D5 (ProDOS 2.5, X=1)" },
    { 0x70, 0xF2, 5, "S7 D6 (ProDOS 2.5, X=1,D=1)" },
    { 0x70, 0x73, 6, "S7 D7 (ProDOS 2.5, X=Y=1)" },
    { 0x70, 0xF3, 7, "S7 D8 (ProDOS 2.5, X=Y=D=1)" },

    /* ProDOS 2.4 legacy phantom-slot drives 3-4 for slot 7 */
    { 0x70, 0x60, 2, "S7 D3 (ProDOS 2.4 phantom)" },
    { 0x70, 0xE0, 3, "S7 D4 (ProDOS 2.4 phantom)" },

    /* Slot 5 (SLOT16 = $50) sample */
    { 0x50, 0x50, 0, "S5 D1 (ProDOS 2.5)" },
    { 0x50, 0xD0, 1, "S5 D2 (ProDOS 2.5)" },
    { 0x50, 0x53, 6, "S5 D7 (ProDOS 2.5)" },
    { 0x50, 0xD3, 7, "S5 D8 (ProDOS 2.5)" },
    { 0x50, 0x40, 2, "S5 D3 (ProDOS 2.4 phantom)" },
    { 0x50, 0xC0, 3, "S5 D4 (ProDOS 2.4 phantom)" },

    /* Slot 4 (SLOT16 = $40) sample - phantom would be slot 3 */
    { 0x40, 0x40, 0, "S4 D1 (ProDOS 2.5)" },
    { 0x40, 0xC0, 1, "S4 D2 (ProDOS 2.5)" },
    { 0x40, 0x42, 4, "S4 D5 (ProDOS 2.5)" },
    { 0x40, 0xC3, 7, "S4 D8 (ProDOS 2.5)" },
    { 0x40, 0x30, 2, "S4 D3 (ProDOS 2.4 phantom S3)" },
    { 0x40, 0xB0, 3, "S4 D4 (ProDOS 2.4 phantom S3)" },

    /* Slot 1 (SLOT16 = $10) sample */
    { 0x10, 0x10, 0, "S1 D1 (ProDOS 2.5)" },
    { 0x10, 0x93, 7, "S1 D8 (ProDOS 2.5)" },
};

#define NCASES (sizeof(cases) / sizeof(cases[0]))

int main(void)
{
    unsigned int i;
    int fail = 0;

    for (i = 0; i < NCASES; i++) {
        zp_slot16   = cases[i].slot16;
        zp_dsnumber = cases[i].dsnumber;
        zp_smbase   = 0xAA;  /* poison */

        decode_smbase();

        if (zp_smbase != cases[i].expected) {
            printf("FAIL: %-32s slot16=$%02X dsnum=$%02X exp=%u got=%u\n",
                   cases[i].desc, cases[i].slot16, cases[i].dsnumber,
                   cases[i].expected, zp_smbase);
            fail++;
        } else {
            printf("PASS: %-32s -> partition %u\n",
                   cases[i].desc, zp_smbase);
        }
    }

    printf("\n");
    if (fail) {
        printf("%d / %u tests FAILED\n", fail, (unsigned)NCASES);
    } else {
        printf("All %u tests passed\n", (unsigned)NCASES);
    }
    return fail;
}
