You need to depend on: GDAL, OpenCV

Instructions:
int nOriX=288; //The pixel length col of tif after resampling
int nOriY=266; // pixel width row of tif after resampling
resizeGByteTiff(nOriX, nOriY, "Original data.tif", "Resampled data.tif")