// (Implemented from the OpenCV book sample)
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/highgui/highgui.hpp"

#define CHANNELS 3

typedef struct ce {
	uchar   learnHigh[CHANNELS];    // High side threshold for learning
	uchar   learnLow[CHANNELS];     // Low side threshold for learning
	uchar   max[CHANNELS];          // High side of box boundary
	uchar   min[CHANNELS];          // Low side of box boundary
	int     t_last_update;          // This is book keeping to allow us to kill stale entries
	int     stale;                  // max negative run (biggest period of inactivity)
} code_element;                     //

typedef struct code_book {
	code_element    **cb;
	int             numEntries;
	int             t;              // count every access
} codeBook;


///////////////////////////////////////////////////////////////////////////////////
// int updateCodeBook(uchar *p, codeBook &c, unsigned cbBounds)
// Updates the codebook entry with a new data point
//
// p            Pointer to a YUV pixel
// c            Codebook for this pixel
// cbBounds     Learning bounds for codebook (Rule of thumb: 10)
// numChannels  Number of color channels we're learning
//
// NOTES:
//      cvBounds must be of size cvBounds[numChannels]
//
// RETURN
//  codebook index
int cvupdateCodeBook(uchar *p, codeBook &c, unsigned *cbBounds, int numChannels)
{
	if(c.numEntries == 0) c.t = 0;
	// 0
	c.t += 1;   // Record learning event
	// ,

	//SET HIGH AND LOW BOUNDS
	int n;
	unsigned int high[3],low[3];
	for (n=0; n<numChannels; n++)
	{
		high[n] = *(p+n) + *(cbBounds+n);
		// *(p+n)  p[n] ,*(p+n)
		if(high[n] > 255) high[n] = 255;
		low[n] = *(p+n)-*(cbBounds+n);
		if(low[n] < 0) low[n] = 0;
		// p ,cbBonds,
	}

	//SEE IF THIS FITS AN EXISTING CODEWORD
	int matchChannel;
	int i;
	for (i=0; i<c.numEntries; i++)
	{
		// ,p
		matchChannel = 0;
		for (n=0; n<numChannels; n++)
			//
		{
			if((c.cb[i]->learnLow[n] <= *(p+n)) && (*(p+n) <= c.cb[i]->learnHigh[n])) //Found an entry for this channel
				// p
			{
				matchChannel++;
			}
		}
		if (matchChannel == numChannels)        // If an entry was found over all channels
			// p
		{
			c.cb[i]->t_last_update = c.t;
			//
			// adjust this codeword for the first channel
			for (n=0; n<numChannels; n++)
				//
			{
				if (c.cb[i]->max[n] < *(p+n))
					c.cb[i]->max[n] = *(p+n);
				else if (c.cb[i]->min[n] > *(p+n))
					c.cb[i]->min[n] = *(p+n);
			}
			break;
		}
	}

	// ENTER A NEW CODE WORD IF NEEDED
	if(i == c.numEntries)  // No existing code word found, make a new one
		// p ,
	{
		code_element **foo = new code_element* [c.numEntries+1];
		// c.numEntries+1
		for(int ii=0; ii<c.numEntries; ii++)
			// c.numEntries
			foo[ii] = c.cb[ii];

		foo[c.numEntries] = new code_element;
		//
		if(c.numEntries) delete [] c.cb;
		// c.cb
		c.cb = foo;
		// foo c.cb
		for(n=0; n<numChannels; n++)
			//
		{
			c.cb[c.numEntries]->learnHigh[n] = high[n];
			c.cb[c.numEntries]->learnLow[n] = low[n];
			c.cb[c.numEntries]->max[n] = *(p+n);
			c.cb[c.numEntries]->min[n] = *(p+n);
		}
		c.cb[c.numEntries]->t_last_update = c.t;
		c.cb[c.numEntries]->stale = 0;
		c.numEntries += 1;
	}

	// OVERHEAD TO TRACK POTENTIAL STALE ENTRIES
	for(int s=0; s<c.numEntries; s++)
	{
		// This garbage is to track which codebook entries are going stale
		int negRun = c.t - c.cb[s]->t_last_update;
		//
		if(c.cb[s]->stale < negRun)
			c.cb[s]->stale = negRun;
	}

	// SLOWLY ADJUST LEARNING BOUNDS
	for(n=0; n<numChannels; n++)
		// ,,
	{
		if(c.cb[i]->learnHigh[n] < high[n])
			c.cb[i]->learnHigh[n] += 1;
		if(c.cb[i]->learnLow[n] > low[n])
			c.cb[i]->learnLow[n] -= 1;
	}

	return(i);
}

///////////////////////////////////////////////////////////////////////////////////
// uchar cvbackgroundDiff(uchar *p, codeBook &c, int minMod, int maxMod)
// Given a pixel and a code book, determine if the pixel is covered by the codebook
//
// p        pixel pointer (YUV interleaved)
// c        codebook reference
// numChannels  Number of channels we are testing
// maxMod   Add this (possibly negative) number onto max level when code_element determining if new pixel is foreground
// minMod   Subract this (possible negative) number from min level code_element when determining if pixel is foreground
//
// NOTES:
// minMod and maxMod must have length numChannels, e.g. 3 channels => minMod[3], maxMod[3].
//
// Return
// 0 => background, 255 => foreground
uchar cvbackgroundDiff(uchar *p, codeBook &c, int numChannels, int *minMod, int *maxMod)
{
	//
	int matchChannel;
	//SEE IF THIS FITS AN EXISTING CODEWORD
	int i;
	for (i=0; i<c.numEntries; i++)
	{
		matchChannel = 0;
		for (int n=0; n<numChannels; n++)
		{
			if ((c.cb[i]->min[n] - minMod[n] <= *(p+n)) && (*(p+n) <= c.cb[i]->max[n] + maxMod[n]))
				matchChannel++; //Found an entry for this channel
			else
				break;
		}
		if (matchChannel == numChannels)
			break; //Found an entry that matched all channels
	}
	if(i == c.numEntries)
		// p,
		return(255);

	return(0);
}


//UTILITES/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//int clearStaleEntries(codeBook &c)
// After learning for some period of time, periodically call this to clear out stale codebook entries
//
//     Codebook to clean up
//
// Return
// number of entries cleared
int cvclearStaleEntries(codeBook &c)
{
	int staleThresh = c.t >> 1;           //
	int *keep = new int [c.numEntries]; //
	int keepCnt = 0;                    //
	//SEE WHICH CODEBOOK ENTRIES ARE TOO STALE
	for (int i=0; i<c.numEntries; i++)
		//
	{
		if (c.cb[i]->stale > staleThresh)
			// ,
			keep[i] = 0; //Mark for destruction
		else
		{
			keep[i] = 1; //Mark to keep
			keepCnt += 1;
		}
	}

	// KEEP ONLY THE GOOD
	c.t = 0;                        //Full reset on stale tracking
	//
	code_element **foo = new code_element* [keepCnt];
	// keepCnt
	int k=0;
	for(int ii=0; ii<c.numEntries; ii++)
	{
		if(keep[ii])
		{
			foo[k] = c.cb[ii];
			foo[k]->stale = 0;       //We have to refresh these entries for next clearStale
			foo[k]->t_last_update = 0;
			k++;
		}
	}
	//CLEAN UP
	delete [] keep;
	delete [] c.cb;
	c.cb = foo;
	// foo c.cb
	int numCleared = c.numEntries - keepCnt;
	//
	c.numEntries = keepCnt;
	//
	return(numCleared);
}



int main()
{
	///////////////////////////////////////
	//
	CvCapture*  capture;
	IplImage*   rawImage;
	IplImage*   yuvImage;
	IplImage*   ImaskCodeBook;
	codeBook*   cB;
	unsigned    cbBounds[CHANNELS];
	uchar*      pColor; //YUV pointer
	int         imageLen;
	int         nChannels = CHANNELS;
	int         minMod[CHANNELS];
	int         maxMod[CHANNELS];

	//////////////////////////////////////////////////////////////////////////
	//
	cvNamedWindow("Raw");
	cvNamedWindow("CodeBook");

	capture = cvCaptureFromCAM( 0 );
	if (!capture)
	{
		printf("Couldn't open the capture!");
		return -1;
	}

	rawImage = cvQueryFrame(capture);
	yuvImage = cvCreateImage(cvGetSize(rawImage), 8, 3);
	// yuvImage rawImage ,83
	ImaskCodeBook = cvCreateImage(cvGetSize(rawImage), IPL_DEPTH_8U, 1);
	// ImaskCodeBook rawImage ,8
	cvSet(ImaskCodeBook, cvScalar(255));
	// 255,

	imageLen = rawImage->width * rawImage->height;
	cB = new codeBook[imageLen];
	// ,

	for (int i=0; i<imageLen; i++)
		// 0
		cB[i].numEntries = 0;
	for (int i=0; i<nChannels; i++)
	{
		cbBounds[i] = 10;   //

		minMod[i]   = 20;   //
		maxMod[i]   = 20;   //
	}


	//////////////////////////////////////////////////////////////////////////
	//
	for (int i=0;;i++)
	{
		cvCvtColor(rawImage, yuvImage, CV_BGR2YCrCb);
		// ,rawImage YUV,yuvImage
		//
		// yuvImage = cvCloneImage(rawImage);

		if (i <= 30)
			// 30
		{
			pColor = (uchar *)(yuvImage->imageData);
			// yuvImage
			for (int c=0; c<imageLen; c++)
			{
				cvupdateCodeBook(pColor, cB[c], cbBounds, nChannels);
				// ,,
				pColor += 3;
				// 3 ,
			}
			if (i == 30)
				// 30 ,
			{
				for (int c=0; c<imageLen; c++)
					cvclearStaleEntries(cB[c]);
			}
		}
		else
		{
			uchar maskPixelCodeBook;
			pColor = (uchar *)((yuvImage)->imageData); //3 channel yuv image
			uchar *pMask = (uchar *)((ImaskCodeBook)->imageData); //1 channel image
			// ImaskCodeBook
			for(int c=0; c<imageLen; c++)
			{
				maskPixelCodeBook = cvbackgroundDiff(pColor, cB[c], nChannels, minMod, maxMod);
				// ,codeBook
				*pMask++ = maskPixelCodeBook;
				pColor += 3;
				// pColor 3
			}
		}
		if (!(rawImage = cvQueryFrame(capture)))
			break;
		cvShowImage("Raw", rawImage);
		cvShowImage("CodeBook", ImaskCodeBook);

		if (cvWaitKey(30) == 27)
			break;
		if (i == 56 || i == 63)
			cvWaitKey();
	}

	cvReleaseCapture(&capture);
	if (yuvImage)
		cvReleaseImage(&yuvImage);
	if(ImaskCodeBook)
		cvReleaseImage(&ImaskCodeBook);
	cvDestroyAllWindows();
	delete [] cB;

	return 0;
}
