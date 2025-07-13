#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main();

struct VdkHeader
{
	char d;
	char k;
	int16_t hLen;
	char cVDK[8];
};
	

int main(int argc, char **argv)
{
	FILE * file_handle, * dsk_out_handle;
	struct VdkHeader VHIn;
	char * sNamePart;
	char * sDskName;
	char dc;
	long int fileSz,byteCount = 0;
	int bForce = 0;
	int bHeaderless = 0;
	int bMinimal = 0;
	
	if (argc > 1)
	{
		if (!strncmp("-f",argv[1],2))
		{
			bForce = 1;
		}
		if (!strncmp("-h",argv[1],2))
		{
			bHeaderless = 1;
		}
		if (!strncmp("-m",argv[1],2))
		{
			bMinimal = 1;
		}
	}
	// Check for mutual exclusivity of -m and -h
	if (bMinimal && bHeaderless) {
		fprintf(stderr, "Invalid header output option\n");
		fprintf(stderr, "Usage: %s [-f] [-[h|m]] filename.vdk\n  -f force output with incorrect image size\n  -h write headerless .dsk output\n  -m write minimal JVC/DSK header (only as many bytes as needed)\n", argv[0]);
		exit(-1);
	}
	if (argc != (2+bForce+bHeaderless+bMinimal))
	{
		fprintf( stderr,"Usage: %s [-f] [-[h|m]] filename.vdk\n  -f force output with incorrect image size\n  -h write headerless .dsk output\n  -m write minimal JVC/DSK header (only as many bytes as needed)\n",argv[0]);
		exit(-1);
	}
	if ((file_handle = fopen(argv[1+bForce+bHeaderless+bMinimal],"rb"))==NULL)
	{
		fprintf( stderr, "Unable to open file %s\n\n", argv[1]);
		exit(-1);
	}
	
	fseek(file_handle, 0L, SEEK_END);
	fileSz = ftell(file_handle);
	
	if (fileSz <= sizeof(struct VdkHeader)+1)
	{
		fprintf( stderr, "Bad or empty vdk file specified\n\n");
		exit(-1);
	}

	fseek(file_handle, 0L, SEEK_SET);
	
	fgets((char *) &VHIn,sizeof(struct VdkHeader)+1,file_handle);
	
	if (strncmp("dk",(char *) &VHIn,2))
	{
		fprintf( stderr, "Missing vdk header\n\n");
		exit(-1);		
	}
	printf("File size %i bytes\n",(int) fileSz);
	printf("Header ID: [%c%c]\n",VHIn.d,VHIn.k);
	printf("            %hu Header length\n",(unsigned short int) VHIn.hLen); 
	printf("            %hu VDK format version\n", (unsigned short int) VHIn.cVDK[0]); 
	printf("            %hu Backwards compat version\n", (unsigned short int) VHIn.cVDK[1]); 
	printf("            %hu Source ID\n", (unsigned short int) VHIn.cVDK[2]); 
	printf("            %hu Source version\n", (unsigned short int) VHIn.cVDK[3]); 
	printf("            %hu Disk image track count\n", (unsigned short int) VHIn.cVDK[4]); 
	printf("            %hu Disk image head count\n", (unsigned short int) VHIn.cVDK[5]); 
	printf("            %02X %u Image flags\n", (unsigned int) VHIn.cVDK[6], (unsigned int) VHIn.cVDK[6]); 
	printf("            %02X %u Compression flag and name length\n", (unsigned char) VHIn.cVDK[7], (unsigned char) VHIn.cVDK[7]); 
		
	sNamePart = malloc((unsigned short int) VHIn.hLen - 10);

	fgets(sNamePart,(unsigned short int) VHIn.hLen - 11, file_handle);
	sNamePart[(unsigned short int) VHIn.hLen - 10]='\0';

	printf("Name   	 : [%s]\n",sNamePart);
	free(sNamePart);
	
	printf("Calculated image area size (%i * %i * %i * %i) %i bytes\n",(int) VHIn.cVDK[4],
		   																(int) VHIn.cVDK[5],
		   																18,
		   																256,
		   	(int) VHIn.cVDK[4] * (int) VHIn.cVDK[5] * 18 * 256);
	printf("Actual image area size %i bytes\n",(int) fileSz - (int) VHIn.hLen);
	
	if((VHIn.cVDK[4] * (int) VHIn.cVDK[5] * 18 * 256) != (fileSz - (int) VHIn.hLen))
	{
		if (!bForce)
		{
			fprintf( stderr, "Image area size problem, aborting\n\n");
			exit(-1);
		}
		else
		{	
			printf("Forcing conversion - note no padding or truncation of image area will occur\n");
		}
	}			
	
	sDskName = malloc(strlen(argv[1+bForce+bHeaderless+bMinimal]));
	sprintf(sDskName,"%.*s.dsk",(int) strcspn(argv[1+bForce+bHeaderless+bMinimal], "."),argv[1+bForce+bHeaderless+bMinimal]);
	printf("Creating new %s%s image:\n%s\n",bHeaderless ? "headerless" : (bMinimal ? "minimal header" : ""), bHeaderless || bMinimal ? "" : "full header",sDskName);
	
	dsk_out_handle = fopen(sDskName, "wb");
	if(!bHeaderless)
	{
		if (bMinimal) {
			// JVC defaults
			unsigned char jvc_defaults[5] = {18, 1, 1, 1, 0};
			unsigned char jvc_header[5];
			jvc_header[0] = VHIn.cVDK[4]; // sectors per track
			jvc_header[1] = VHIn.cVDK[5]; // side count
			jvc_header[2] = 1; // sector size code (always 1 for 256 bytes)
			jvc_header[3] = 1; // first sector ID (always 1)
			jvc_header[4] = 0; // sector attribute flag (always 0)
			int minimal_len = 0;
			for (int i = 4; i >= 0; --i) {
				if (jvc_header[i] != jvc_defaults[i]) {
					minimal_len = i+1;
					break;
				}
			}
			if (minimal_len > 0) {
				fwrite(jvc_header, 1, minimal_len, dsk_out_handle);
			}
		} else {
			fputc('\22',dsk_out_handle);
			fputc(VHIn.cVDK[5],dsk_out_handle);
			fputc('\1',dsk_out_handle);
			fputc('\1',dsk_out_handle);
			fputc('\0',dsk_out_handle);
		}
	}
	
	while(1) {
    	dc = fgetc(file_handle);
   		if(feof(file_handle) ) { break ; }
   		fputc(dc,dsk_out_handle);
		byteCount++;	
    }
   
    fclose(dsk_out_handle);
    fclose(file_handle);
		
	free(sDskName);
	printf("%li bytes written\n%li sectors\n",byteCount,byteCount/256);
	exit(0);

}
	