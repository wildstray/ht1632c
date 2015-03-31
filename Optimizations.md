# Introduction #

Software optimization is the process of modifying a software system to make it works it work more efficiently or using fewer resources.

When you make an Arduino project, **Wiring** IDE calls **avr gcc** compiler and then uploads the binary program to the board using **avrdude**.

avr gcc compiles optimizing the code size and speed of execution, but you can achieve advanced optimization following this guide.

# Assembler code analysis #

When you verify or upload a program to an Arduino board, Wiring leave some temporary files around.

On GNU/Linux and Mac OSX you can find them in: **/tmp/build**

On Windows you can find them in: **C:\TEMP\build**

ELF binaries contains debug symbols, so you can look at the assembler code.

Example:

```
avr-objdump -d /tmp/build999268808019415668.tmp/demo.cpp.elf
```

Fragment of resulting output:

```
00004d72 <_Z3rndhh>:
    4d72:       48 2f           mov     r20, r24
    4d74:       80 91 af 02     lds     r24, 0x02AF
    4d78:       90 e0           ldi     r25, 0x00       ; 0
    4d7a:       01 96           adiw    r24, 0x01       ; 1
    4d7c:       25 e1           ldi     r18, 0x15       ; 21
    4d7e:       30 e0           ldi     r19, 0x00       ; 0
    4d80:       fc 01           movw    r30, r24
    4d82:       e2 9f           mul     r30, r18
    4d84:       c0 01           movw    r24, r0
    4d86:       e3 9f           mul     r30, r19
    4d88:       90 0d           add     r25, r0
    4d8a:       f2 9f           mul     r31, r18
    4d8c:       90 0d           add     r25, r0
    4d8e:       11 24           eor     r1, r1
    4d90:       98 2f           mov     r25, r24
    4d92:       80 93 af 02     sts     0x02AF, r24
    4d96:       61 50           subi    r22, 0x01       ; 1
    4d98:       01 c0           rjmp    .+2             ; 0x4d9c <_Z3rndhh+0x2a>
    4d9a:       96 1b           sub     r25, r22
    4d9c:       96 17           cp      r25, r22
    4d9e:       e8 f7           brcc    .-6             ; 0x4d9a <_Z3rndhh+0x28>
    4da0:       89 2f           mov     r24, r25
    4da2:       84 0f           add     r24, r20
    4da4:       08 95           ret
```

Originating C code:

```
byte mod(byte n, byte d)
{
  while(n >= d)
    n -= d;

  return n;
}

byte rnd(byte min, byte max)
{
  static byte seed;
  seed = (21 * seed + 21);
  return min + mod(seed, --max);
}
```

If you need to highlight a specific piece of code, you can surround it with nop markers.

Example:

```
byte mod(byte n, byte d)
{
  while(n >= d)
    n -= d;

  return n;
}

byte rnd(byte min, byte max)
{
  static byte seed;

  asm ("nop");
  seed = (21 * seed + 21);
  asm ("nop");

  return min + mod(seed, --max);
}
```



```
00004d72 <_Z3rndhh>:
    4d72:       48 2f           mov     r20, r24
    4d74:       00 00           nop                     ; <====== nop
    4d76:       80 91 af 02     lds     r24, 0x02AF
    4d7a:       90 e0           ldi     r25, 0x00       ; 0
    4d7c:       01 96           adiw    r24, 0x01       ; 1
    4d7e:       25 e1           ldi     r18, 0x15       ; 21
    4d80:       30 e0           ldi     r19, 0x00       ; 0
    4d82:       fc 01           movw    r30, r24
    4d84:       e2 9f           mul     r30, r18
    4d86:       c0 01           movw    r24, r0
    4d88:       e3 9f           mul     r30, r19
    4d8a:       90 0d           add     r25, r0
    4d8c:       f2 9f           mul     r31, r18
    4d8e:       90 0d           add     r25, r0
    4d90:       11 24           eor     r1, r1
    4d92:       98 2f           mov     r25, r24
    4d94:       80 93 af 02     sts     0x02AF, r24
    4d98:       00 00           nop                     ; <====== nop
    4d9a:       61 50           subi    r22, 0x01       ; 1
    4d9c:       01 c0           rjmp    .+2             ; 0x4da0 <_Z3rndhh+0x2e>
    4d9e:       96 1b           sub     r25, r22
    4da0:       96 17           cp      r25, r22
    4da2:       e8 f7           brcc    .-6             ; 0x4d9e <_Z3rndhh+0x2c>
    4da4:       89 2f           mov     r24, r25
    4da6:       84 0f           add     r24, r20
    4da8:       08 95           ret
```

Using the option -S of avr-objdump you can browse a mixup of C and assembler code. With this method you can avoid using nops as markers.

```
avr-objdump -S /tmp/build999268808019415668.tmp/demo.cpp.elf
```

```
byte rnd(byte min, byte max)
    4d72:       48 2f           mov     r20, r24
{
  static byte seed;
  seed = (21 * seed + 21);
    4d74:       80 91 af 02     lds     r24, 0x02AF
    4d78:       90 e0           ldi     r25, 0x00       ; 0
    4d7a:       01 96           adiw    r24, 0x01       ; 1
    4d7c:       25 e1           ldi     r18, 0x15       ; 21
    4d7e:       30 e0           ldi     r19, 0x00       ; 0
    4d80:       fc 01           movw    r30, r24
    4d82:       e2 9f           mul     r30, r18
    4d84:       c0 01           movw    r24, r0
    4d86:       e3 9f           mul     r30, r19
    4d88:       90 0d           add     r25, r0
    4d8a:       f2 9f           mul     r31, r18
    4d8c:       90 0d           add     r25, r0
    4d8e:       11 24           eor     r1, r1
    4d90:       98 2f           mov     r25, r24
    4d92:       80 93 af 02     sts     0x02AF, r24
  return min + mod(seed, --max);
    4d96:       61 50           subi    r22, 0x01       ; 1
    4d98:       01 c0           rjmp    .+2             ; 0x4d9c <_Z3rndhh+0x2a>
/* fast integer modulus */

byte mod(byte n, byte d)
{
  while(n >= d)
    n -= d;
    4d9a:       96 1b           sub     r25, r22

/* fast integer modulus */

byte mod(byte n, byte d)
{
  while(n >= d)
    4d9c:       96 17           cp      r25, r22
    4d9e:       e8 f7           brcc    .-6             ; 0x4d9a <_Z3rndhh+0x28>
byte rnd(byte min, byte max)
{
  static byte seed;
  seed = (21 * seed + 21);
  return min + mod(seed, --max);
}
    4da0:       89 2f           mov     r24, r25
    4da2:       84 0f           add     r24, r20
    4da4:       08 95           ret
```

## C code tips and tricks ##

### Data types ###

You can write more efficient code using smallest data types. In the following example you can see the difference between using a `byte` (`unsigned char` or `uint8_t`) and a `word` (`unsigned int` or `uint16_t`). In the first case the compiled assigns `i` to `r17`, in the second case it assigns `i` to `r16:r17`.

C code:

```
  byte i;
  for (i=0; i<5; i++)
     tone(13, 440, 500);
```

Resulting compiled code:

```
    3aa8:       10 e0           ldi     r17, 0x00       ; 0
    3aaa:       8d e0           ldi     r24, 0x0D       ; 13
    3aac:       68 eb           ldi     r22, 0xB8       ; 184
    3aae:       71 e0           ldi     r23, 0x01       ; 1
    3ab0:       24 ef           ldi     r18, 0xF4       ; 244
    3ab2:       31 e0           ldi     r19, 0x01       ; 1
    3ab4:       40 e0           ldi     r20, 0x00       ; 0
    3ab6:       50 e0           ldi     r21, 0x00       ; 0
    3ab8:       0e 94 29 27     call    0x4e52  ; 0x4e52 <_Z4tonehjm>
    3abc:       1f 5f           subi    r17, 0xFF       ; 255
    3abe:       15 30           cpi     r17, 0x05       ; 5
    3ac0:       a1 f7           brne    .-24            ; 0x3aaa <loop+0x12>
```

C code:

```
  word i;
  for (i=0; i<5; i++)
     tone(13, 440, 500);
```

Resulting compiled code:

```
    3aa8:       00 e0           ldi     r16, 0x00       ; 0
    3aaa:       10 e0           ldi     r17, 0x00       ; 0
    3aac:       8d e0           ldi     r24, 0x0D       ; 13
    3aae:       68 eb           ldi     r22, 0xB8       ; 184
    3ab0:       71 e0           ldi     r23, 0x01       ; 1
    3ab2:       24 ef           ldi     r18, 0xF4       ; 244
    3ab4:       31 e0           ldi     r19, 0x01       ; 1
    3ab6:       40 e0           ldi     r20, 0x00       ; 0
    3ab8:       50 e0           ldi     r21, 0x00       ; 0
    3aba:       0e 94 2c 27     call    0x4e58  ; 0x4e58 <_Z4tonehjm>
    3abe:       0f 5f           subi    r16, 0xFF       ; 255
    3ac0:       1f 4f           sbci    r17, 0xFF       ; 255
    3ac2:       05 30           cpi     r16, 0x05       ; 5
    3ac4:       11 05           cpc     r17, r1
    3ac6:       91 f7           brne    .-28            ; 0x3aac <loop+0x14>
```

### Bit manipulation ###

There are some special cases - processor dependent - where boolean logic operations / bit manipulation operations can be optimized to save instrucations / machine cycles.

A simple example. Depending on `c` you must set or reset byte(s) `b` of variable `a`. The intuitive way could be:

```
 (c) ? a |= b : a &= ~b;
```

Resulting compiled code:

```
    3a94:       66 23           and     r22, r22
    3a96:       11 f0           breq    .+4             ; 0x3a9c <loop+0x5e>
    3a98:       81 2b           or      r24, r17
    3a9a:       02 c0           rjmp    .+4             ; 0x3aa0 <loop+0x62>
    3a9c:       10 95           com     r17
    3a9e:       81 23           and     r24, r17
```

`r24` is `a`, `r17` is `b` and `r22` is `c`. The same can be obtained in a counter intuitive but fast way: setting the bit(s) and toggling the same bit(s) depending the flag `c`:

```
  a |= b;
  if (!c)
    a ^= b;
```

Resulting compiled code:

```
    3a7e:       80 81           ld      r24, Z
    3a80:       81 2b           or      r24, r17
    3a82:       80 83           st      Z, r24
    3a84:       66 23           and     r22, r22
    3a86:       19 f4           brne    .+6             ; 0x3a8e <loop+0x50>
    3a88:       81 27           eor     r24, r17
```

### Fast integer math ###

If you need to include arithmetic operations in you code but you don't need floating point operations, you could use boolean operations instead arithmetic operations, or use smaller data types and custom functions instead of stdlib functions or C operators (expecially / and %).

Look at IntegerCodeSnippets.

Here is some ready to use fast integer 1 byte wide math functions (from ht1632c library).

```
/* fast integer (1 byte) modulus */
byte _mod(byte n, byte d)
{
  while(n >= d)
    n -= d;

  return n;
}

/* fast integer (1 byte) division */
byte _div(byte n, byte d)
{
  byte q = 0;
  while(n >= d)
  {
    n -= d;
    q++;
  }
  return q;
}

/* fast integer (1 byte) PRNG */
byte _rnd(byte min, byte max)
{
  static byte seed;
  seed = (21 * seed + 21);
  return min + _mod(seed, --max);
}
```

WARNING: don't use this `_rnd()` function for cryptography!