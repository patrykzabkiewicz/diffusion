
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <QString>


using namespace std;


struct SourceVolume {
    float ws;
    float ds;
};

struct DiffusionVolume {
    float di;
    int vi;
};



float gaussianBlur(DiffusionVolume *sourceMatrix, float *gaussianMatrix, int gaussianMatrixSize) {

    float tmp=0.0;

    for(int i=0; i< gaussianMatrixSize; i++) {
        if(sourceMatrix[i].vi==1) tmp += sourceMatrix[i].di*gaussianMatrix[i];
    }

    return tmp;
}




int main(int argc, char** argv) {

    SourceVolume sourceVolume[100*100];
    DiffusionVolume diffusionVolume[100*100];
    float gaussianMatrix[3*3];

    for(int i=0; i<3*3; i++) gaussianMatrix[i]=0.111;

    //inicjalizacja przestrzeni
    for(int i=0; i<100*100; i++) {
        sourceVolume[i].ds = (-2);
        sourceVolume[i].ws = 0;

        diffusionVolume[i].di = 0;
        diffusionVolume[i].vi = 0;
    }


    // rysowanie obrazu poddawanego dyfuzji (dwie ukosne kreski)
    // drawing image to be defused (two diagonal lines)
    for(int i=0, j=0; j<40; j++, i++) {
        sourceVolume[100*i+j].ws = 1;
        sourceVolume[100*i+j].ds = 0;
    }

    for(int i=0, j=0; j<40; j++, i++) {
        sourceVolume[100*i+(100-j-1)].ws = 1;
        sourceVolume[100*i+(100-j-1)].ds =0;
    }


    // alternatywny przypadek (dwie proste rownolegle w roznych plaszczyznach)
    // alternative source image sample (two straigh lines on different levels)

//    for(int i=30, j=0; j<40; j++) {
//        sourceVolume[100*i+j].ws = 1;
//        sourceVolume[100*i+j].ds = 0;
//    }

//    for(int i=60, j=0; j<40; j++) {
//        sourceVolume[100*i+(100-j-1)].ws = 1;
//        sourceVolume[100*i+(100-j-1)].ds =0;
//    }


    // sampling diffusion volume
    for(int i=101; i < 100*99; i++) {
        if(sourceVolume[i].ds == 0) {

            diffusionVolume[i].di = 0;
            diffusionVolume[i+100].di=(-1.0);
            diffusionVolume[i-100].di=1;

            diffusionVolume[i].vi = 1;
            diffusionVolume[i+100].vi = 1;
            diffusionVolume[i-100].vi = 1;
        }
    }



    DiffusionVolume *tmp1 = new DiffusionVolume[100*100];
    DiffusionVolume *matrix = new DiffusionVolume[9];
    for(int i=0; i<9; i++) matrix[i].di = matrix[i].vi = 0;


    // iteracje algorytmu
    // finite algorithm iterations
    for(int iter=0; iter<500; iter++) {

        for(int i=0; i< 100*100; i++) tmp1[i].di = tmp1[i].vi = 0;

	// gaussian blur
        // rozmywanie (guassian blur)
        for(int i=1; i < 99; i++) {
            for(int j=1; j<99; j++) {

                // budownie matrycy dla alg. gaussa
		// coping matrix for gaussian convolution
                matrix[0] = diffusionVolume[100*(i-1)+j-1];
                matrix[1] = diffusionVolume[100*(i-1)+j];
                matrix[2] = diffusionVolume[100*(i-1)+j+1];

                matrix[3] = diffusionVolume[100*i+j-1];
                matrix[4] = diffusionVolume[100*i+j];
                matrix[5] = diffusionVolume[100*i+j+1];

                matrix[6] = diffusionVolume[100*(i+1)+j-1];
                matrix[7] = diffusionVolume[100*(i+1)+j];
                matrix[8] = diffusionVolume[100*(i+1)+j+1];

                // rozmycie realizowane w funkcji
		// blurring image in subfunction
                tmp1[100*i+j].di = gaussianBlur( matrix, gaussianMatrix, 9);

                tmp1[100*i+j].vi = 1;

            }
        }


        // kompozycja (lowpass filter)
	// composition of two images (lowpass filter)
        for(int i=0; i < 100*100; i++) {
            diffusionVolume[i].di = sourceVolume[i].ds*sourceVolume[i].ws + (1 - sourceVolume[i].ws) * tmp1[i].di;
            diffusionVolume[i].vi = tmp1[i].vi;
        }


        // budowanie animacji jako obrazkow w katalogu
	// storring produced temporary images in a folder to see animation
        IplImage *img1 = cvCreateImage(cvSize(100,100),IPL_DEPTH_8U,3);

        for(int i=0; i < 100; i++) {
            for(int j=0; j < 100; j++) {
                uchar* dstb = &CV_IMAGE_ELEM( img1, uchar, i, j*3 );
                uchar* dstg = &CV_IMAGE_ELEM( img1, uchar, i, j*3+1 );
                uchar* dstr = &CV_IMAGE_ELEM( img1, uchar, i, j*3+2 );

                // konstrukcja warstwy z pewna tolerancja odchylenia od zera
		// convolution of two diffused images with little tollerance from zero value
                *dstb = *dstg = *dstr = 0;

                if(diffusionVolume[100*i+j].di < 0 && diffusionVolume[100*i+j].di > (-2)) *dstb = 123-123*diffusionVolume[100*i+j].di;
                if(diffusionVolume[100*i+j].di > 0) *dstr = 255-255*diffusionVolume[100*i+j].di;

                if(diffusionVolume[100*i+j].di >= (-0.0001) && diffusionVolume[100*i+j].di <= 0.0001 && diffusionVolume[100*i+j].vi == 1) *dstr = *dstb = *dstg = 255;

            }
        }

        QString filename = "seria/";
        filename.append("wizualizacja_");
        filename.append(QString::number(iter));
        filename.append(".bmp");

        cvSaveImage(filename.toLocal8Bit().data(), img1);

    }

    delete tmp1;
    delete matrix;


    IplImage *img0 = cvCreateImage(cvSize(100,100),IPL_DEPTH_8U,1);
    IplImage *img2 = cvCreateImage(cvSize(100,100),IPL_DEPTH_8U,1);

    for(int i=0; i < 100; i++) {
        for(int j=0; j < 100; j++) {
            uchar* dst = &CV_IMAGE_ELEM( img0, uchar, i, j );

            if(sourceVolume[100*i+j].ds==0) *dst = 240; else *dst = 0;
        }
    }


    for(int i=0; i < 100; i++) {
        for(int j=0; j < 100; j++) {
            	uchar* dst = &CV_IMAGE_ELEM( img2, uchar, i, j );
            	*dst = 0;

           	// konstrukcja warstwy z pewna tolerancja odchylenia od zera
		// same as above, tollerance from zero point
            	if(diffusionVolume[100*i+j].di >= (-0.0001) && diffusionVolume[100*i+j].di <= 0.0001 && diffusionVolume[100*i+j].vi == 1 ) *dst = 240;
        }
    }

    cvSaveImage("zrodlo.bmp", img0);
    cvSaveImage("wynik.bmp", img2);


    return 0;


}

