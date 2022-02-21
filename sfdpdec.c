#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

static int sfdp_check_header(char *buf)
{
	return memcmp(buf, "SFDP", 4);
}

struct bytemap {
	uint8_t byte;
	const char *str;
};

void print_cols(const char *prefix, const char *fmt, ...)
{
	va_list ap;

	printf("%s:%*s", prefix, 40 - (int)strlen(prefix), "");

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	printf("\n");
}

void print_bytemap(const char *prefix, struct bytemap *bm, uint8_t byte)
{
	while (bm->str) {
		if (bm->byte == byte) {
			break;
		}
		bm++;
	}

	if (bm->str) {
		print_cols(prefix, "[%02x] (%s)", bm->byte, bm->str);
	} else {
		print_cols(prefix, "[%02x]", byte);
	}
}

struct bytemap sfdp_access_protocol[] = {
	{ 0xf0, "xSPI NAND class-1" },
	{ 0xf1, "xSPI NAND class-2" },
	{ 0xf4, "xSPI NAND class-1" },
	{ 0xf5, "xSPI NAND class-2" },
	{ 0xfa, "xSPI NOR Profile 2, 5-byte addressing" },
	{ 0xfc, "xSPI NOR Profile 1, 3-byte addressing" },
	{ 0xfd, "xSPI NOR Profile 1, 4-byte addressing" },
	{ 0xfe, "xSPI NOR Profile 1, 4-byte addressing" },
	{ 0xff, "legacy option, JESD216B, 3-byte addressing" },
	{ }
};

static void sfdp_dump_header(char *buf)
{
	print_cols("SFDP Revision", "%d.%d", buf[5], buf[4]);
	print_cols("Number of Parameter Headers", "%d", buf[6] + 1);
	print_bytemap("Access Protocol", sfdp_access_protocol, buf[7]);
}

static void sfdp_dump_param_header(char *buf)
{
	print_cols("ID", "%x", buf[7] << 8 | buf[0]);
	print_cols("  Revision", "%d.%d", buf[2], buf[1]);
	print_cols("  Length", "%d", buf[3]);
	print_cols("  Pointer", "@%06x", buf[6] << 16 | buf[5] << 8 | buf[4]);
}

static void sfdp_dump_params(char *buf)
{
	sfdp_dump_param_header(buf);
}

static void sfdp_dump(char *buf)
{
	int i;
	int nph = buf[6];

	sfdp_dump_header(buf);

	for (i = 0; i <= nph; i++) {
		buf += 8;
		sfdp_dump_params(buf);
	}
}


int main(int argc, char **argv)
{
	FILE *fp;
	char buf[4096];
	int len;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <sfdp binary>\n", argv[0]);
		return EXIT_FAILURE;
	};

	fp = fopen(argv[1], "r");
	if (!fp) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	len = fread(buf, 1, sizeof(buf), fp);

	if (len < 16) {
		fprintf(stderr, "SFDP too short (expect at least 16 bytes)\n");
		return EXIT_FAILURE;
	}

	if (sfdp_check_header(buf)) {
		fprintf(stderr, "Bad SFDP header\n");
		return EXIT_FAILURE;
	}

	sfdp_dump(buf);

	return EXIT_SUCCESS;
}
