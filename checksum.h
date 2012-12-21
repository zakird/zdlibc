#ifndef CHECKSUM_H
#define CHECKSUM_H

#define unlikely(x) __builtin_expect(!!(x), 0)

static inline unsigned short from32to16(unsigned a) 
{
        unsigned short b = a >> 16; 
        asm("addw %w2,%w0\n\t"
            "adcw $0,%w0\n" 
            : "=r" (b)
            : "" (b), "r" (a));
        return b;
}

static inline uint16_t csum_fold(uint32_t sum)
{
    asm("  addl %1,%0\n"
        "  adcl $0xffff,%0"
        : "=r" (sum)
        : "r" ((uint32_t)sum << 16),
          "0" ((uint32_t)sum & 0xffff0000));
    return (uint16_t)(~(uint32_t)sum >> 16);
}

static inline uint32_t add32_with_carry(uint32_t a, uint32_t b)
{
    asm("addl %2,%0\n\t"
        "adcl $0,%0"
        : "=r" (a)
        : "0" (a), "r" (b));
    return a;
}

static inline uint32_t csum_tcpudp_nofold(uint32_t saddr,
		uint32_t daddr, unsigned short len,
		unsigned short proto, uint32_t sum)
{
    asm("  addl %1, %0\n"
        "  adcl %2, %0\n"
        "  adcl %3, %0\n"
        "  adcl $0, %0\n"
        : "=r" (sum)
        : "g" (daddr), "g" (saddr),
          "g" ((len + proto)<<8), "0" (sum));
    return sum;
}

static uint32_t do_csum(char *buff, uint32_t len)
{
	unsigned odd, count;
	unsigned long result = 0;

	if (unlikely(len == 0)) {
		return result; 
	}
	odd = 1 & (unsigned long) buff;
	if (unlikely(odd)) {
		result = *buff << 8;
		len--;
		buff++;
	}
	count = len >> 1;		/* nr of 16-bit words.. */
	if (count) {
		if (2 & (unsigned long) buff) {
			result += *(unsigned short *)buff;
			count--;
			len -= 2;
			buff += 2;
		}
		count >>= 1;		/* nr of 32-bit words.. */
		if (count) {
			unsigned long zero;
			unsigned count64;
			if (4 & (unsigned long) buff) {
				result += *(unsigned int *) buff;
				count--;
				len -= 4;
				buff += 4;
			}
			count >>= 1;	/* nr of 64-bit words.. */

			/* main loop using 64byte blocks */
			zero = 0;
			count64 = count >> 3;
			while (count64) { 
				asm("addq 0*8(%[src]),%[res]\n\t"
				    "adcq 1*8(%[src]),%[res]\n\t"
				    "adcq 2*8(%[src]),%[res]\n\t"
				    "adcq 3*8(%[src]),%[res]\n\t"
				    "adcq 4*8(%[src]),%[res]\n\t"
				    "adcq 5*8(%[src]),%[res]\n\t"
				    "adcq 6*8(%[src]),%[res]\n\t"
				    "adcq 7*8(%[src]),%[res]\n\t"
				    "adcq %[zero],%[res]"
				    : [res] "=r" (result)
				    : [src] "r" (buff), [zero] "r" (zero),
				    "[res]" (result));
				buff += 64;
				count64--;
			}

			/* last upto 7 8byte blocks */
			count %= 8; 
			while (count) { 
				asm("addq %1,%0\n\t"
				    "adcq %2,%0\n" 
					    : "=r" (result)
				    : "m" (*(unsigned long *)buff), 
				    "r" (zero),  "0" (result));
				--count; 
					buff += 8;
			}
			result = add32_with_carry(result>>32,
						  result&0xffffffff); 

			if (len & 4) {
				result += *(unsigned int *) buff;
				buff += 4;
			}
		}
		if (len & 2) {
			result += *(unsigned short *) buff;
			buff += 2;
		}
	}
	if (len & 1)
		result += *buff;
	result = add32_with_carry(result>>32, result & 0xffffffff); 
	if (unlikely(odd)) { 
		result = from32to16(result);
		result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
	}
	return result;
}
#endif
