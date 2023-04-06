#include <stdio.h>

int main()
{
	FILE* f1, * f2;
	errno_t open_err;
	open_err = fopen_s(&f1, "../Ghidra/SAF files/File 1/Very Small Line.saf", "rb");
	open_err = fopen_s(&f2, "../Ghidra/SAF files/File 2/Very Small Line.saf", "rb");
	if (f1 == NULL || f2 == NULL)
	{
		return 1;
	}

	unsigned char bf1[1000], bf2[1000];
	fread(bf1, 1, sizeof(bf1), f1);
	fread(bf2, 1, sizeof(bf2), f2);

	fclose(f1);
	fclose(f2);

//	printf("%X %X\n", bf1[8], bf2[8]);

#if 0
	for (size_t i = 0x08; i < 0xE8; i++)
	{
		printf("%X %X\n", bf1[i], bf2[i]);
	}

	printf("\n");
	for (size_t i = 0x158; i < 0x258; i++)
	{
		printf("%X %X\n", bf1[i], bf2[i]);
	}
#endif

#define ENC_OP(a,b) (a)^(b)
//#define ENC_OP(a,b) (a)+(b)
//#define ENC_OP(a,b) (b)-(a)

	for (size_t key1i = 0x08; key1i < 0xE8; key1i++)
	{
		unsigned char val1 = bf1[key1i] ^ bf1[0x158];
		for (size_t key2i = 0x08; key2i < 0xE8; key2i++)
		{
			unsigned char val2 = ENC_OP(bf2[key2i], bf2[0x158]);
			if (val1 == val2)
			{
				unsigned char val1_2 = ENC_OP(bf1[key1i + 1], bf1[0x158 + 1]);
				unsigned char val2_2 = ENC_OP(bf2[key2i + 1], bf2[0x158 + 1]);
				if (val1_2 == val2_2)
				{
					printf("maybe i1=%zu and i2=%zu : v %d %d %d\n", key1i, key2i, val2, val1_2, val2_2);
					unsigned char val1_3 = ENC_OP(bf1[key1i + 2], bf1[0x158 + 2]);
					unsigned char val2_3 = ENC_OP(bf2[key2i + 2], bf2[0x158 + 2]);
					if (val1_3 == val2_3)
					{
						printf("wow - ");
					}
				}
			}
		}
	}
	return 0;
}