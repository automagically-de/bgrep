#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))

static int bgrep(uint8_t *pattern, uint32_t len, FILE *f, const char *fname)
{
	uint32_t r, i;
	long int pos = 0;
	uint8_t buf[1024];
	uint8_t match;

	do
	{
		r = fread(buf, 1, 1, f);
		pos ++;

		if(r && (buf[0] == pattern[0]))
		{
			match = 1;

			if(len > 1)
			{
				r = fread(buf + 1, 1, len - 1, f);
				pos += len - 1;

				if(r != (len - 1)) return 0; /* end of file */

				for(i = 1; i < len; i ++)
				{
					if(buf[i] != pattern[i])
					{
						match = 0;
						break;
					}
				}
			}

			if(match)
			{
				printf("%s: %08x\n", fname, (unsigned int)(pos - len));
			}

			fseek(f, pos, SEEK_SET);
		}
	}
	while(r > 0);

	return EXIT_SUCCESS;
}

static void show_help()
{
	fprintf(stderr,
		"usage: bgrep [options] <pattern> [<filename> ...]\n"
		"   pattern is a hexadecimal string, e.g. '0A0345F7' unless\n"
		"           the option -s is used (see below).\n"
		"   options are:\n"
		"           -h  show help\n"
		"           -s  pattern is given as ASCII string (e.g. 'ELF')\n"
		"   filename is optional, if not given, STDIN is used to read "
					"data from\n"
	);
}

int main(int argc, char *argv[])
{
	FILE *f;
	uint32_t i, fnindex = 0;
	uint8_t *pattern, s_str = 0;
	uint32_t plen;
	char hex[3] = { 0, 0, 0 };
	char *string = NULL;

	if(argc < 2)
	{
		show_help();
		return EXIT_FAILURE;
	}

	for(i = 1; i < argc; i ++)
	{
		if(argv[i][0] == '-')
		{
			/* option */
			switch(argv[i][1])
			{
				case 'h':
					show_help();
					return EXIT_FAILURE;
					break;

				case 's':
					/* search string is ASCII */
					s_str = 1;
					break;

				case '\0':
					break;

				default:
					fprintf(stderr, "%s: -%c is not a valid option\n",
						argv[0], argv[i][1]);
					break;
			}
		}
		else
		{
			if(!string) string = argv[i];
			else
			{
				fnindex = i;
				break;
			}
		}
	}

	if(!string)
	{
		show_help();
		return EXIT_FAILURE;
	}

	/* parse pattern */
	if(s_str)
	{
		plen = strlen(string);
		if(plen == 0)
		{
			fprintf(stderr, "%s: pattern has zero length\n", argv[0]);
			return EXIT_FAILURE;
		}

		pattern = (uint8_t *)calloc(plen, sizeof(uint8_t));

		fprintf(stderr, "pattern:");

		for(i = 0; i < plen; i ++)
		{
			pattern[i] = string[i];
			fprintf(stderr, " %02x", pattern[i]);
		}

		fprintf(stderr, "\n");
	}
	else
	{
		plen = strlen(string) / 2;
		if(plen == 0)
		{
			fprintf(stderr, "%s: pattern has zero length\n", argv[0]);
			return EXIT_FAILURE;
		}

		pattern = (uint8_t *)calloc(plen, sizeof(uint8_t));

		fprintf(stderr, "pattern:");

		for(i = 0; i < plen; i ++)
		{
			strncpy(hex, string + (i * 2), 2);
			pattern[i] = strtoul(hex, NULL, 16);
			fprintf(stderr, " %02x", pattern[i]);
		}

		fprintf(stderr, "\n");
	}

	if(!fnindex)
	{
		/* read from stdin */
		bgrep(pattern, plen, stdin, "<stdin>");
	}
	else
	{
		/* for all input files... */
		for(i = fnindex; i < argc; i ++)
		{
			f = fopen(argv[i], "rb");
			if(f == NULL)
			{
				fprintf(stderr, "failed to open %s\n", argv[i]);
				continue;
			}

			bgrep(pattern, plen, f, argv[i]);

			fclose(f);
		}
	}

	free(pattern);

	return EXIT_SUCCESS;
}
