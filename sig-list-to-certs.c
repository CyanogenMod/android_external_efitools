#include <stdint.h>
#define __STDC_VERSION__ 199901L
#include <efi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/pem.h>
#include <openssl/err.h>

#include <variables.h>

int
main(int argc, char *argv[])
{
  char *certfile, *efifile, *name;
	const char *progname = argv[0];

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <efi sig list file> <cert file base name>\n", progname);
		exit(1);
	}

	efifile = argv[1];
	certfile = argv[2];
	name = malloc(strlen(certfile)+10);

	FILE *f = fopen(efifile, "r");
	if (!f) {
		fprintf(stderr, "Failed to open file %s: ", efifile);
		perror("");
		exit(1);
	}


	int count = 0;

	while (!feof(f)) {
		EFI_SIGNATURE_LIST l;
		EFI_SIGNATURE_DATA *d;
		char *buf;

		int s = fread(&l, 1, sizeof(l), f);
		

		if (s != sizeof(l)) {
			fprintf(stderr, "Only read %d bytes\n", s);
			continue;
		}
		if (memcmp(&l.SignatureType, &EFI_CERT_X509_GUID, sizeof(EFI_GUID))==0) {
			printf("X509 ");
		} else if (memcmp(&l.SignatureType, &EFI_CERT_TYPE_PKCS7_GUID, sizeof(EFI_GUID))==0) {
			printf("PKCS7 ");
		} else if (memcmp(&l.SignatureType, &EFI_CERT_RSA2048_GUID, sizeof(EFI_GUID))==0) {
			printf("RSA2048 ");
		} else {
			printf("UNKNOWN ");
		}
		printf("Header sls=%d, header=%d, sig=%d\n",
		       l.SignatureListSize, l.SignatureHeaderSize, l.SignatureSize);
		buf = malloc(l.SignatureListSize - sizeof(l));
		if (!buf) {
			fprintf(stderr, "Out of Memory\n");
			exit(1);
		}
		s = fread(buf, 1,  l.SignatureListSize - sizeof(l), f);
		if (s != l.SignatureListSize - sizeof(l)) {
			fprintf(stderr, "only read %d bytes\n", s);
			exit(1);
		}
		d = (void *)buf;
		buf += sizeof(*d) - 1;

		sprintf(name, "%s.%d",certfile,count++);
		  

		FILE *g = fopen(name, "w");
		fwrite(buf, 1, l.SignatureSize - sizeof(EFI_SIGNATURE_DATA) + 1,
		       g);
		fclose(g);
	}


	return 0;
}