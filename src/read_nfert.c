/**********
 read_nfert.c
 
 deprecated and removed from lds target
    the original processing ported here was wrong
 
 read the nitrogen application rate (kg/ha) into nfert_grid[NUM_CELLS]
 values range from 0 to ~221
 no nodata value
 
 from Potter et al 2010
    these data are based on SAGE harvested area
 
 arguments:
 char* fname:          file name to open, with path
 float* nfert_grid:    the array to load the data into
 args_struct in_args: the input file arguments
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 12 Jan 2016

 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "lds.h"

int read_nfert(char *fname, float *nfert_grid, args_struct in_args) {
    
    // use this function to input data to the working grid
    
    // nfert data
    // image file with one band (starts at upper left corner)
    // 4 byte float
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // values are nitrogen application rate (kg/ha)
    
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int insize = 4;					// 4 byte floats
    //double res = 5.0 / 60.0;		// resolution
    //double xmin = -180.0;			// longitude min grid boundary
    //double xmax = 180.0;			// longitude max grid boundary
    //double ymin = -90.0;			// latitude min grid boundary
    //double ymax = 90.0;				// latitude max grid boundary
    
    FILE *fpin;
    int num_read;					// how many values read in
    
    int err = OK;								// store error code from the dignostic write file
    char out_name[] = "nfert.bil";		// file name for output diagnostics raster file
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_nfert()\n", fname);
        return ERROR_FILE;
    }
    
    // read the data and check for consistency
    num_read = fread(nfert_grid, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_nfert(); num_read=%i != NUMCELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    fclose(fpin);
    
    if (in_args.diagnostics) {
        if ((err = write_raster_float(nfert_grid, ncells, out_name, in_args))) {
            fprintf(fplog, "Error writing file %s: read_nfert()\n", out_name);
            return err;
        }
    }
    
    return OK;
}