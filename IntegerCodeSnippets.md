# Fast integer math using bitwise operations #

## Multiply by power of two using left shift ##

```
x *= 2;
x *= 4;
x *= 64;

//equals:
x <<= 1;
x <<= 2;
x <<= 6;
```

NB: On AVR bitwise shift ops (lsl, lsr) can do a single bit shift. The more is the shift is deep the less is fast.

## Divide by power of two using right shifting ##

```
x /= 2;
x /= 4;
x /= 64;

//equals:
x >>= 1;
x >>= 2;
x >>= 6;
```

NB: On AVR bitwise shift ops (lsl, lsr) can do a single bit shift. The more is the shift is deep the less is fast.

## Swap integers without a temporary variable using XOR ##

```
int a,b,tmp;
tmp = a;
a = b;
b = tmp;

//equals:
a ^= b;
b ^= a;
a ^= b;
```


## Sign flipping using NOT ##

```
i = -i;

//equals:
i = ~i + 1;
```

## Sign flipping using XOR ##

```
i = -i;

//equals:
i = (i ^ -1) + 1;
```

## checking if even using AND ##

```
even = ((i % 2) == 0);

//equals:
even = ((i & 1) == 0);
```

## integer abs() ##

```
x = abs(x);

//equals:
x = (x < 0) ? -x : x;
```

## comparing integers sign ##

```
sign = a * b > 0;

//equals:
sign = a ^ b >= 0;
```

## fast integer log2() ##

```
y = log2(x);

//equals:
y = 0;
while(x >= 2) {
  y++;
  x >>= 1;
}
```

## fast integer pow2() ##

```
result = pow(2, exp);

//equals:
result = 1;
while(exp--) {
    result *= 2;
}
```

## rotate right ##

```
  y = (x >> shift) | (x << (bits - shift));
```

shift is the rotate dept, bits is the size in bits of the variables used (8,16,32 or 64 bits).

## rotate left ##

```
  y = (x << shift) | (x >> (bits - shift));
```

shift is the rotate dept, bits is the size in bits of the variables used (8,16,32 or 64 bits).

## integer sqrt() ##

```
y = sqrt(x);

//equals:
y = 1;
while (y*y <= x) {
  ++y;
}

//equals:
square = 1;
delta = 3;
while (square <= x) {
  square += delta;
  delta += 2;
}
y = (delta/2) - 1;
```

## fast integer % (modulus) ##

```

m = n % d;

//equals:
while(n >= d) {
    n -= d;
}
m = n;
```

## Helpful links ##

  * [Arduino playground - BitMath](http://www.arduino.cc/playground/Code/BitMath)
  * [Bit Twiddling Hacks](http://graphics.stanford.edu/~seander/bithacks.html)