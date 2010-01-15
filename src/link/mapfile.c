#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmotor.h"

#include "link/main.h"
#include "link/mylink.h"
#include "link/assign.h"

FILE *mf = NULL;
FILE *sf = NULL;
SLONG currentbank = 0;
SLONG sfbank;

void 
SetMapfileName(char *name)
{
	mf = fopen(name, "wt");

	if (!mf)
		errx(5, "Unable to open mapfile for writing");
}

void 
SetSymfileName(char *name)
{
	sf = fopen(name, "wt");

	if (!sf)
		errx(5, "Unable to open symfile for writing");

	fprintf(sf, ";File generated by xLink v" LINK_VERSION "\n\n");
}

void 
CloseMapfile(void)
{
	if (mf) {
		fclose(mf);
		mf = NULL;
	}
	if (sf) {
		fclose(sf);
		sf = NULL;
	}
}

void 
MapfileInitBank(SLONG bank)
{
	if (mf) {
		currentbank = bank;
		if (bank == 0)
			fprintf(mf, "Bank #0 (HOME):\n");
		else if (bank <= 255)
			fprintf(mf, "Bank #%ld:\n", bank);
		else if (bank == BANK_BSS)
			fprintf(mf, "BSS:\n");
		else if (bank == BANK_HRAM)
			fprintf(mf, "HRAM:\n");
		else if (bank == BANK_VRAM)
			fprintf(mf, "VRAM:\n");
	}
	if (sf) {
		sfbank = (bank >= 1 && bank <= 255) ? bank : 0;
	}
}

void 
MapfileWriteSection(struct sSection * pSect)
{
	if (!mf && !sf)
		return;

	SLONG i;

	fprintf(mf, "  SECTION: $%04lX-$%04lX ($%04lX bytes)\n",
	    pSect->nOrg, pSect->nOrg + pSect->nByteSize - 1,
	    pSect->nByteSize);

	for (i = 0; i < pSect->nNumberOfSymbols; i += 1) {
		struct sSymbol *pSym;
		pSym = pSect->tSymbols[i];
		if ((pSym->pSection == pSect)
		    && (pSym->Type != SYM_IMPORT)) {
			if (mf) {
				fprintf(mf, "           $%04lX = %s\n",
				    pSym->nOffset + pSect->nOrg,
				    pSym->pzName);
			}
			if (sf) {
				fprintf(sf, "%02lX:%04lX %s\n", sfbank,
				    pSym->nOffset + pSect->nOrg,
				    pSym->pzName);
			}
		}
	}
}

void 
MapfileCloseBank(SLONG slack)
{
	if (!mf)
		return;

	if (slack == MaxAvail[currentbank])
		fprintf(mf, "  EMPTY\n\n");
	else
		fprintf(mf, "    SLACK: $%04lX bytes\n\n", slack);
}
