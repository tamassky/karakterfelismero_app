#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void PrintBinaryPixels(Mat img)
{
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            uchar pixel = (int)img.at<uchar>(i, j);
            if (pixel == 255)
                cout << " ";
            else
                cout << "0";
        }
        cout << "\n";
    }
}

//Súlyozott módszer: https://www.tutorialspoint.com/dip/grayscale_to_rgb_conversion.htm
Mat BgrToGrayscale(Mat img)
{
    uint8_t* grayArr = (uint8_t*) malloc(sizeof(uint8_t) * img.rows * img.cols);
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            Vec3b bgrPixel = img.at<Vec3b>(i, j);
            grayArr[i * img.cols + j] = (uint8_t)(bgrPixel[0] * 0.11 + bgrPixel[1] * 0.59 + bgrPixel[2] * 0.3); 
        }
    }
    Mat result(img.rows, img.cols, CV_8U, grayArr);
    return result;
}

Mat GrayscaleToBinary(Mat img, int threshold)
{
    uint8_t* binaryArr = (uint8_t*)malloc(sizeof(uint8_t) * img.rows * img.cols);
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            int graypixel = (int)img.at<uchar>(i, j);
            if (graypixel > threshold)
                binaryArr[i * img.cols + j] = 255;
            else
                binaryArr[i * img.cols + j] = 0;
        }
    }
    Mat result(img.rows, img.cols, CV_8U, binaryArr);
    return result;
}

Mat FilterIsolatedPixels(Mat img) 
{
    uint8_t* filteredArr = (uint8_t*)malloc(sizeof(uint8_t) * img.rows * img.cols);
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            int pixelorigin = (int)img.at<uchar>(i, j);
            if (pixelorigin == 255) 
            {
                filteredArr[i * img.cols + j] = 255;
                continue;
            }
            else 
            {
                if (i > 0 && j > 0) //ÉNy
                {
                    int pixel = (int)img.at<uchar>(i - 1, j - 1);
                    if (pixel == 0) 
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (i > 0) //É
                {
                    int pixel = (int)img.at<uchar>(i - 1, j);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (i > 0 && j < img.cols - 1) //ÉK
                {
                    int pixel = (int)img.at<uchar>(i - 1, j + 1);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (j > 0) //Ny
                {
                    int pixel = (int)img.at<uchar>(i , j - 1);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (j < img.cols - 1) //K
                {
                    int pixel = (int)img.at<uchar>(i, j + 1);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (j < img.cols - 1) //K
                {
                    int pixel = (int)img.at<uchar>(i, j + 1);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (i < img.rows -1 && j > 0) //DNy
                {
                    int pixel = (int)img.at<uchar>(i + 1, j - 1);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (i < img.rows - 1) //D
                {
                    int pixel = (int)img.at<uchar>(i + 1, j);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                if (i < img.rows - 1 && j < img.cols - 1) //DK
                {
                    int pixel = (int)img.at<uchar>(i + 1, j + 1);
                    if (pixel == 0)
                    {
                        filteredArr[i * img.cols + j] = 0;
                        continue;
                    }
                }
                filteredArr[i * img.cols + j] = 255;
            }
        }
    }
    Mat result(img.rows, img.cols, CV_8U, filteredArr);
    return result;
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    int threshold = 100;

    // Read the image file 
    Mat trainer = imread("C:\\Asztali backup\\Egyetem\\Msc\\1f_Képfeldolgozás_és_orvosi_képalkotás\\tanito.jpg");
    Mat image = imread("C:\\Asztali backup\\Egyetem\\Msc\\1f_Képfeldolgozás_és_orvosi_képalkotás\\feldolgozando.jpg");

    // Check for failure 
    if (trainer.empty())
    {
        cout << "Tanító képfájl nem található!!!\u201E\u201D" << endl; 
        cin.get(); //wait for any key press 
        return -1;
    }
    if (image.empty())
    {
        cout << "Feldolgozandó képfájl nem található!!!" << endl;
        cin.get(); //wait for any key press 
        return -1;
    }

    //To grayscale
    Mat gray = BgrToGrayscale(image);
    
    //To binary
    Mat binary = GrayscaleToBinary(gray, threshold);

    //Filter isolated
    Mat filtered = FilterIsolatedPixels(binary);

    // Show our image inside a window. 
    imshow("Tanító kép", trainer);
    waitKey(0);
    imshow("Feldolgozandó kép", image);
    waitKey(0);
    imshow("Feldolgozandó kép szürkében", gray);
    waitKey(0);
    imshow("Feldolgozandó kép binárisan", binary);
    waitKey(0);
    imshow("Feldolgozandó kép szûrve", filtered);
    waitKey(0);
    //PrintBinaryPixels(binary);

    return 0;
}