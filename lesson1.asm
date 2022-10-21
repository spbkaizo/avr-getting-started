GAS LISTING /tmp/cctRGOHu.s 			page 1


   1               		.file	"lesson1.c"
   2               	__SP_H__ = 0x3e
   3               	__SP_L__ = 0x3d
   4               	__SREG__ = 0x3f
   5               	__tmp_reg__ = 0
   6               	__zero_reg__ = 1
   7               	.global	__mulsf3
   8               	.global	__ltsf2
   9               	.global	__gtsf2
  10               	.global	__fixunssfsi
  11               		.text
  12               	.global	main
  13               		.type	main, @function
  14               	main:
  15 0000 CF93      		push r28
  16 0002 DF93      		push r29
  17 0004 CDB7      		in r28,__SP_L__
  18 0006 DEB7      		in r29,__SP_H__
  19 0008 6C97      		sbiw r28,28
  20 000a 0FB6      		in __tmp_reg__,__SREG__
  21 000c F894      		cli
  22 000e DEBF      		out __SP_H__,r29
  23 0010 0FBE      		out __SREG__,__tmp_reg__
  24 0012 CDBF      		out __SP_L__,r28
  25               	/* prologue: function */
  26               	/* frame size = 28 */
  27               	/* stack size = 30 */
  28               	.L__stack_usage = 30
  29 0014 84E2      		ldi r24,lo8(36)
  30 0016 90E0      		ldi r25,0
  31 0018 2FEF      		ldi r18,lo8(-1)
  32 001a FC01      		movw r30,r24
  33 001c 2083      		st Z,r18
  34               	.L18:
  35 001e 85E2      		ldi r24,lo8(37)
  36 0020 90E0      		ldi r25,0
  37 0022 2FEF      		ldi r18,lo8(-1)
  38 0024 FC01      		movw r30,r24
  39 0026 2083      		st Z,r18
  40 0028 80E0      		ldi r24,0
  41 002a 90E0      		ldi r25,0
  42 002c AAEF      		ldi r26,lo8(-6)
  43 002e B3E4      		ldi r27,lo8(67)
  44 0030 8983      		std Y+1,r24
  45 0032 9A83      		std Y+2,r25
  46 0034 AB83      		std Y+3,r26
  47 0036 BC83      		std Y+4,r27
  48 0038 20E0      		ldi r18,0
  49 003a 30E0      		ldi r19,0
  50 003c 4AE7      		ldi r20,lo8(122)
  51 003e 55E4      		ldi r21,lo8(69)
  52 0040 6981      		ldd r22,Y+1
  53 0042 7A81      		ldd r23,Y+2
  54 0044 8B81      		ldd r24,Y+3
  55 0046 9C81      		ldd r25,Y+4
  56 0048 0E94 0000 		call __mulsf3
  57 004c DC01      		movw r26,r24
GAS LISTING /tmp/cctRGOHu.s 			page 2


  58 004e CB01      		movw r24,r22
  59 0050 8D83      		std Y+5,r24
  60 0052 9E83      		std Y+6,r25
  61 0054 AF83      		std Y+7,r26
  62 0056 B887      		std Y+8,r27
  63 0058 20E0      		ldi r18,0
  64 005a 30E0      		ldi r19,0
  65 005c 40E8      		ldi r20,lo8(-128)
  66 005e 5FE3      		ldi r21,lo8(63)
  67 0060 6D81      		ldd r22,Y+5
  68 0062 7E81      		ldd r23,Y+6
  69 0064 8F81      		ldd r24,Y+7
  70 0066 9885      		ldd r25,Y+8
  71 0068 0E94 0000 		call __ltsf2
  72 006c 8823      		tst r24
  73 006e 04F4      		brge .L23
  74 0070 81E0      		ldi r24,lo8(1)
  75 0072 90E0      		ldi r25,0
  76 0074 9A87      		std Y+10,r25
  77 0076 8987      		std Y+9,r24
  78 0078 00C0      		rjmp .L4
  79               	.L23:
  80 007a 20E0      		ldi r18,0
  81 007c 3FEF      		ldi r19,lo8(-1)
  82 007e 4FE7      		ldi r20,lo8(127)
  83 0080 57E4      		ldi r21,lo8(71)
  84 0082 6D81      		ldd r22,Y+5
  85 0084 7E81      		ldd r23,Y+6
  86 0086 8F81      		ldd r24,Y+7
  87 0088 9885      		ldd r25,Y+8
  88 008a 0E94 0000 		call __gtsf2
  89 008e 1816      		cp __zero_reg__,r24
  90 0090 04F4      		brge .L24
  91 0092 20E0      		ldi r18,0
  92 0094 30E0      		ldi r19,0
  93 0096 40E2      		ldi r20,lo8(32)
  94 0098 51E4      		ldi r21,lo8(65)
  95 009a 6981      		ldd r22,Y+1
  96 009c 7A81      		ldd r23,Y+2
  97 009e 8B81      		ldd r24,Y+3
  98 00a0 9C81      		ldd r25,Y+4
  99 00a2 0E94 0000 		call __mulsf3
 100 00a6 DC01      		movw r26,r24
 101 00a8 CB01      		movw r24,r22
 102 00aa BC01      		movw r22,r24
 103 00ac CD01      		movw r24,r26
 104 00ae 0E94 0000 		call __fixunssfsi
 105 00b2 DC01      		movw r26,r24
 106 00b4 CB01      		movw r24,r22
 107 00b6 9A87      		std Y+10,r25
 108 00b8 8987      		std Y+9,r24
 109 00ba 00C0      		rjmp .L7
 110               	.L8:
 111 00bc 80E9      		ldi r24,lo8(-112)
 112 00be 91E0      		ldi r25,lo8(1)
 113 00c0 9C87      		std Y+12,r25
 114 00c2 8B87      		std Y+11,r24
GAS LISTING /tmp/cctRGOHu.s 			page 3


 115 00c4 8B85      		ldd r24,Y+11
 116 00c6 9C85      		ldd r25,Y+12
 117               	/* #APP */
 118               	 ;  105 "/usr/local/avr/avr/include/util/delay_basic.h" 1
 119 00c8 0197      		1: sbiw r24,1
 120 00ca 01F4      		brne 1b
 121               	 ;  0 "" 2
 122               	/* #NOAPP */
 123 00cc 9C87      		std Y+12,r25
 124 00ce 8B87      		std Y+11,r24
 125 00d0 8985      		ldd r24,Y+9
 126 00d2 9A85      		ldd r25,Y+10
 127 00d4 0197      		sbiw r24,1
 128 00d6 9A87      		std Y+10,r25
 129 00d8 8987      		std Y+9,r24
 130               	.L7:
 131 00da 8985      		ldd r24,Y+9
 132 00dc 9A85      		ldd r25,Y+10
 133 00de 0097      		sbiw r24,0
 134 00e0 01F4      		brne .L8
 135 00e2 00C0      		rjmp .L9
 136               	.L24:
 137 00e4 6D81      		ldd r22,Y+5
 138 00e6 7E81      		ldd r23,Y+6
 139 00e8 8F81      		ldd r24,Y+7
 140 00ea 9885      		ldd r25,Y+8
 141 00ec 0E94 0000 		call __fixunssfsi
 142 00f0 DC01      		movw r26,r24
 143 00f2 CB01      		movw r24,r22
 144 00f4 9A87      		std Y+10,r25
 145 00f6 8987      		std Y+9,r24
 146               	.L4:
 147 00f8 8985      		ldd r24,Y+9
 148 00fa 9A85      		ldd r25,Y+10
 149 00fc 9E87      		std Y+14,r25
 150 00fe 8D87      		std Y+13,r24
 151 0100 8D85      		ldd r24,Y+13
 152 0102 9E85      		ldd r25,Y+14
 153               	/* #APP */
 154               	 ;  105 "/usr/local/avr/avr/include/util/delay_basic.h" 1
 155 0104 0197      		1: sbiw r24,1
 156 0106 01F4      		brne 1b
 157               	 ;  0 "" 2
 158               	/* #NOAPP */
 159 0108 9E87      		std Y+14,r25
 160 010a 8D87      		std Y+13,r24
 161               	.L9:
 162 010c 85E2      		ldi r24,lo8(37)
 163 010e 90E0      		ldi r25,0
 164 0110 FC01      		movw r30,r24
 165 0112 1082      		st Z,__zero_reg__
 166 0114 80E0      		ldi r24,0
 167 0116 90E0      		ldi r25,0
 168 0118 AAEF      		ldi r26,lo8(-6)
 169 011a B3E4      		ldi r27,lo8(67)
 170 011c 8F87      		std Y+15,r24
 171 011e 988B      		std Y+16,r25
GAS LISTING /tmp/cctRGOHu.s 			page 4


 172 0120 A98B      		std Y+17,r26
 173 0122 BA8B      		std Y+18,r27
 174 0124 20E0      		ldi r18,0
 175 0126 30E0      		ldi r19,0
 176 0128 4AE7      		ldi r20,lo8(122)
 177 012a 55E4      		ldi r21,lo8(69)
 178 012c 6F85      		ldd r22,Y+15
 179 012e 7889      		ldd r23,Y+16
 180 0130 8989      		ldd r24,Y+17
 181 0132 9A89      		ldd r25,Y+18
 182 0134 0E94 0000 		call __mulsf3
 183 0138 DC01      		movw r26,r24
 184 013a CB01      		movw r24,r22
 185 013c 8B8B      		std Y+19,r24
 186 013e 9C8B      		std Y+20,r25
 187 0140 AD8B      		std Y+21,r26
 188 0142 BE8B      		std Y+22,r27
 189 0144 20E0      		ldi r18,0
 190 0146 30E0      		ldi r19,0
 191 0148 40E8      		ldi r20,lo8(-128)
 192 014a 5FE3      		ldi r21,lo8(63)
 193 014c 6B89      		ldd r22,Y+19
 194 014e 7C89      		ldd r23,Y+20
 195 0150 8D89      		ldd r24,Y+21
 196 0152 9E89      		ldd r25,Y+22
 197 0154 0E94 0000 		call __ltsf2
 198 0158 8823      		tst r24
 199 015a 04F4      		brge .L25
 200 015c 81E0      		ldi r24,lo8(1)
 201 015e 90E0      		ldi r25,0
 202 0160 988F      		std Y+24,r25
 203 0162 8F8B      		std Y+23,r24
 204 0164 00C0      		rjmp .L12
 205               	.L25:
 206 0166 20E0      		ldi r18,0
 207 0168 3FEF      		ldi r19,lo8(-1)
 208 016a 4FE7      		ldi r20,lo8(127)
 209 016c 57E4      		ldi r21,lo8(71)
 210 016e 6B89      		ldd r22,Y+19
 211 0170 7C89      		ldd r23,Y+20
 212 0172 8D89      		ldd r24,Y+21
 213 0174 9E89      		ldd r25,Y+22
 214 0176 0E94 0000 		call __gtsf2
 215 017a 1816      		cp __zero_reg__,r24
 216 017c 04F4      		brge .L26
 217 017e 20E0      		ldi r18,0
 218 0180 30E0      		ldi r19,0
 219 0182 40E2      		ldi r20,lo8(32)
 220 0184 51E4      		ldi r21,lo8(65)
 221 0186 6F85      		ldd r22,Y+15
 222 0188 7889      		ldd r23,Y+16
 223 018a 8989      		ldd r24,Y+17
 224 018c 9A89      		ldd r25,Y+18
 225 018e 0E94 0000 		call __mulsf3
 226 0192 DC01      		movw r26,r24
 227 0194 CB01      		movw r24,r22
 228 0196 BC01      		movw r22,r24
GAS LISTING /tmp/cctRGOHu.s 			page 5


 229 0198 CD01      		movw r24,r26
 230 019a 0E94 0000 		call __fixunssfsi
 231 019e DC01      		movw r26,r24
 232 01a0 CB01      		movw r24,r22
 233 01a2 988F      		std Y+24,r25
 234 01a4 8F8B      		std Y+23,r24
 235 01a6 00C0      		rjmp .L15
 236               	.L16:
 237 01a8 80E9      		ldi r24,lo8(-112)
 238 01aa 91E0      		ldi r25,lo8(1)
 239 01ac 9A8F      		std Y+26,r25
 240 01ae 898F      		std Y+25,r24
 241 01b0 898D      		ldd r24,Y+25
 242 01b2 9A8D      		ldd r25,Y+26
 243               	/* #APP */
 244               	 ;  105 "/usr/local/avr/avr/include/util/delay_basic.h" 1
 245 01b4 0197      		1: sbiw r24,1
 246 01b6 01F4      		brne 1b
 247               	 ;  0 "" 2
 248               	/* #NOAPP */
 249 01b8 9A8F      		std Y+26,r25
 250 01ba 898F      		std Y+25,r24
 251 01bc 8F89      		ldd r24,Y+23
 252 01be 988D      		ldd r25,Y+24
 253 01c0 0197      		sbiw r24,1
 254 01c2 988F      		std Y+24,r25
 255 01c4 8F8B      		std Y+23,r24
 256               	.L15:
 257 01c6 8F89      		ldd r24,Y+23
 258 01c8 988D      		ldd r25,Y+24
 259 01ca 0097      		sbiw r24,0
 260 01cc 01F4      		brne .L16
 261 01ce 00C0      		rjmp .L18
 262               	.L26:
 263 01d0 6B89      		ldd r22,Y+19
 264 01d2 7C89      		ldd r23,Y+20
 265 01d4 8D89      		ldd r24,Y+21
 266 01d6 9E89      		ldd r25,Y+22
 267 01d8 0E94 0000 		call __fixunssfsi
 268 01dc DC01      		movw r26,r24
 269 01de CB01      		movw r24,r22
 270 01e0 988F      		std Y+24,r25
 271 01e2 8F8B      		std Y+23,r24
 272               	.L12:
 273 01e4 8F89      		ldd r24,Y+23
 274 01e6 988D      		ldd r25,Y+24
 275 01e8 9C8F      		std Y+28,r25
 276 01ea 8B8F      		std Y+27,r24
 277 01ec 8B8D      		ldd r24,Y+27
 278 01ee 9C8D      		ldd r25,Y+28
 279               	/* #APP */
 280               	 ;  105 "/usr/local/avr/avr/include/util/delay_basic.h" 1
 281 01f0 0197      		1: sbiw r24,1
 282 01f2 01F4      		brne 1b
 283               	 ;  0 "" 2
 284               	/* #NOAPP */
 285 01f4 9C8F      		std Y+28,r25
GAS LISTING /tmp/cctRGOHu.s 			page 6


 286 01f6 8B8F      		std Y+27,r24
 287 01f8 00C0      		rjmp .L18
 288               		.size	main, .-main
 289               		.ident	"GCC: (GNU) 4.8.1"
