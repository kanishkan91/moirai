/**********
 read_rent_orig.c
 
 read the original GTAP land rents into rent_orig_aez[NUM_GTAP_CTRY87 * NUM_GTAP_USE * NUM_ORIG_AEZ]
 zero values represent either zero or no data
 
 convert input units of million yr2001USD to yr2001USD
 
 arguments:
 args_struct in_args: the input file arguments

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 6 June 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "moirai.h"

int read_rent_orig(args_struct in_args) {

	// the original GTAP land rent data formatted for GCAM
	// csv file with 6 header lines
	// each line ends with a carraige return and then a newline ('\r\n')
	// there can't be any blank lines for this to work properly
	// first column: ctry87
	// second column: GTAP_use (13 aggregate use categories)
	// next 18 columns are the 18 AEZs in order
	// all ctry87 and use rows are present in the file
	// the order of the rows are in order of GTAP_GCAM_ctry87.csv first and then GTAP_use.csv in order per country
	// units are million yr2001USD
	
	int j;
	int ncols = 20;					// ctry87, GTAP_use, 18 AEZs
	int nrecords = 1131;			// number of records in file
	int nhead = 6;					// number of header lines to skip
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	char temp_str[MAXRECSIZE];		// string to test for blank line
	const char *delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = 0;				// the index of the rent_orig_aez[] array to fill
	
	long flen = 0;					// length of file in bytes
	long count_lines = 0;			// count the number of lines to skip the header
	long count_recs = 0;			// count the number of records read
	long rec_index = 0;				// the index of the record string to fill
	int is_newrec = 0;				// flag to denote end of record
	char *sptr;						// dynamically allocated buffer for whole file as a string
	char *cptr;						// pointer to traverse the file string
	
	char out_name[] = "rent_orig_aez.csv";	// diagnositic output csv file name
	
	float *lrout;					// output the original land rent diagnostics in USD for formatting purposes
	
	// allocate memory for the diagnostic output
	// this needs to be initialized with zeroes, so use pointers and calloc
	lrout = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE * NUM_ORIG_AEZ, sizeof(float));
	if(lrout == NULL) {
		fprintf(fplog,"Failed to allocate memory for lrout:  read_rent_orig()\n");
		return ERROR_MEM;
	}
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.rent_orig_fname);
	
	// read the whole file at once in binary mode using fread
	if((fpin = fopen(fname, "rb")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_rent_orig()\n", fname);
		return ERROR_FILE;
	}
	fseek(fpin, 0L, SEEK_END);
	flen = ftell(fpin);
	rewind(fpin);
	// calloc initializes the values to zero, which is the termination character
	sptr = calloc(flen + 1, sizeof(char));
	if(sptr == NULL) {
		fprintf(fplog,"Failed to allocate memory for file %s:  read_rent_orig()\n", fname);
		return ERROR_MEM;
	}
	
	fread(sptr, flen, 1, fpin);
	fclose(fpin);
	cptr = sptr;	// set the moving pointer to the beginning of the file
	
	// while loop is probably faster than for loop
	// the terminating character is zero, which will stop the while loops
	// this is the loop over the whole file
	// this reads in and skips whitespace lines, rather than throwing an error, but doesn't count them as records 
	while (*cptr) {
		rec_index = 0;
		is_newrec = 0;
		// this is the loop over each record
		while (*cptr) {
			if (!is_newrec) {
				if (*cptr == '\n') {
					is_newrec = 1;
				}
			} else {
				break;
			}
			// store the current character in the record string
			rec_str[rec_index++] = *cptr++;
		} // end while loop over record
		rec_str[rec_index] = '\0';	// terminate the record string
		
		// process this record if it is not a header and it is not a blank record
		rm_whitesp(temp_str, rec_str);
		if (!(count_lines++ < nhead) && strlen(temp_str)) {
			count_recs++;
			// loop over the values and fill the array (skip the first two columns)
			// note that field index starts at 1 for get_float_field
			for (j = 3; j <= ncols; j++) {
				if((err = get_float_field(rec_str, delim, j, &rent_orig_aez[out_index++])) != OK) {
					fprintf(fplog, "Error processing file %s: read_rent_orig(); record=%li, column=%i\n",
							fname, count_recs, j);
					return err;
				}
				// convert to USD for diagnostic output
				lrout[out_index - 1] = MIL2ONE * rent_orig_aez[out_index - 1];
			}
		
		if((j-1) != ncols)
		{
			fprintf(fplog, "Error processing file %s: read_rent_orig(); cols processed=%i != ncols=%i\n",
					fname, j, ncols);
			return ERROR_FILE;
		}
			
		}	// end if(!header) process record
		
	} // end while loop over file
	
	free(sptr);
	
	if(count_recs != nrecords)
	{
		fprintf(fplog, "Error reading file %s: read_rent_orig(); records read=%li != nrecords=%i\n",
				fname, count_recs, nrecords);
		return ERROR_FILE;
	}
	
	if (in_args.diagnostics) {
		if ((err = write_csv_float3d(lrout, country87codes_gtap, usecodes_gtap,
									NUM_GTAP_CTRY87, NUM_GTAP_USE, NUM_ORIG_AEZ, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: read_rent_orig()\n", out_name);
			return err;
		}
	}

	free(lrout);
	
	return OK;
}