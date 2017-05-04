#include "stdafx.h"

static uint32_t
lcg(uint32_t in)
{
	return (1103515245 * in + 12345) & (0x7fffffff);
}

#if GENERATING

static uint32_t
rotl(uint32_t in, unsigned int bits)
{
	return ((in << bits) | (in >> (32 - bits)));
}

static void
generateSerial(uint32_t serial[])
{
	serial[0] = rotl((uint32_t)time(NULL), 9);
	serial[1] = lcg(serial[0]);
}

static void
encodeKey(char *key, const uint32_t serial[])
{
	static const char *alphabet = "KLMNOPQRSTUVWXYZ";
	const uint8_t *serialb = (const uint8_t *)serial;
	size_t i;

	for (i = 0; i < 2 * sizeof(*serial); ++i) {
		*key++ = alphabet[serialb[i] >> 4];
		*key++ = alphabet[serialb[i] & 0xF];

		if (i != 0 && i != 2 * sizeof(*serial) - 1 && i % 2 == 1)
			*key++ = '-';
	}

	key[19] = '\0';
}

#else

static int
decodeKey(uint32_t serial[], char *key)
{
	uint8_t *serialb = (uint8_t *)serial;

	serial[0] = 0;
	serial[1] = 0;

	while (*key != '\0') {
		if (*key == '-') {
			++key;
			continue;
		}
		if (*key < 'K' || *key > 'Z')
			return -1;

		*serialb++ = ((key[0] - 'K') << 4) | (key[1] - 'K');
		key += 2;
	}

	return 0;
}

bool
isValidKey(char *key)
{
	uint32_t serial[2];
	uint32_t *seed = serial, *checksum = serial + 1;

	if (strlen(key) != 19 || key[4] != '-' || key[9] != '-' || key[14] != '-')
		return false;

	if (decodeKey(serial, key) != 0)
		return false;

	return lcg(serial[0]) == serial[1];
}

#endif

int
main(int argc, char *argv[])
{
#if !(GENERATING)
	if (argc < 2) {
		fprintf(stderr, "Usage: %s key\n"
		"This program will print \"PASS\" if the key is valid.\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (isValidKey(argv[1])) {
		puts("PASS");
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
#else
	uint32_t serial[2];
	uint32_t *seed = serial, *checksum = serial + 1;
	/* VMNW-WOKK-KNOO-VSOL */
	char key[20];

	generateSerial(serial);
	encodeKey(key, serial);

	printf("%s (== 0x%08x / 0x%08x)\n", key, serial[0], serial[1]);

	return EXIT_SUCCESS;
#endif
}
