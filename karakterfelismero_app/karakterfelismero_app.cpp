#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

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

vector<Mat> SliceHorizontally(Mat img)
{
    vector<Mat> lines;
    bool contentPrev = false;
    int start = -1, stop = -1;
    for (int i = 0; i < img.rows; i++)
    {
        bool contentCurr = false;
        for (int j = 0; j < img.cols; j++)
        {
            uchar pixel = (int)img.at<uchar>(i, j);
            if (pixel == 0)
            {
                contentCurr = true;
                break;
            }
        }
        if (contentPrev == false && contentCurr == true)
        {
            start = i;
        }
        if ((contentPrev == true && contentCurr == false) || (i == img.rows - 1 && contentCurr == true))
        {
            stop = i;
            uint8_t* sliceArr = (uint8_t*)malloc(sizeof(uint8_t) * (stop - start + 1) * img.cols);
            for (int ii = start; ii <= stop; ii++)
            {
                for (int jj = 0; jj < img.cols; jj++)
                {
                    sliceArr[(ii - start) * img.cols + jj] = (int)img.at<uchar>(ii, jj);
                }
            }
            Mat line(stop - start + 1, img.cols, CV_8U, sliceArr);
            lines.push_back(line);
        }
        contentPrev = contentCurr;
    }
    return lines;
}

vector<Mat> SliceVertically(Mat img)
{
    vector<Mat> chars;
    bool contentPrev = false;
    int start = -1, stop = -1;
    for (int i = 0; i < img.cols; i++)
    {
        bool contentCurr = false;
        for (int j = 0; j < img.rows; j++)
        {
            uchar pixel = (int)img.at<uchar>(j, i);
            if (pixel == 0)
            {
                contentCurr = true;
                break;
            }
        }
        if (contentPrev == false && contentCurr == true)
        {
            start = i;
        }
        if ((contentPrev == true && contentCurr == false) || (i == img.cols - 1 && contentCurr == true))
        {
            stop = i;
            uint8_t* sliceArr = (uint8_t*)malloc(sizeof(uint8_t) * img.rows * (stop - start + 1));
            for (int ii = 0; ii < img.rows; ii++)
            {
                for (int jj = start; jj <= stop; jj++)
                {
                    sliceArr[ii * (stop - start + 1) + (jj - start)] = (int)img.at<uchar>(ii, jj);
                }
            }
            Mat ch(img.rows, stop - start + 1, CV_8U, sliceArr);
            chars.push_back(ch);
        }
        contentPrev = contentCurr;
    }
    return chars;
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
    /*imshow("Feldolgozandó kép", image);
    waitKey(0);
    imshow("Feldolgozandó kép szürkében", gray);
    waitKey(0);
    imshow("Feldolgozandó kép binárisan", binary);
    waitKey(0);
    imshow("Feldolgozandó kép szûrve", filtered);
    waitKey(0);*/

    //To grayscale
    Mat grayt = BgrToGrayscale(trainer);

    //To binary
    Mat binaryt = GrayscaleToBinary(grayt, threshold);

    //Filter isolated
    Mat filteredt = FilterIsolatedPixels(binaryt);

    imshow("Tanító kép szürkében", grayt);
    waitKey(0);
    imshow("Tanító kép binárisan", binaryt);
    waitKey(0);
    imshow("Tanító kép szûrve", filteredt);
    waitKey(0);
    //PrintBinaryPixels(trainer);
    vector<Mat> lines = SliceHorizontally(filteredt);
    vector<Mat> charsAll;
    for (int i = 0; i < lines.size(); i++)
    {
        imshow("Lépegetõ", lines[i]);
        waitKey(0);
        vector<Mat> chars = SliceVertically(lines[i]);
        charsAll.insert(charsAll.end(), chars.begin(), chars.end());
    }
    for (int i = 0; i < charsAll.size(); i++)
    {
        imshow("Lépegetõ betû", charsAll[i]);
        waitKey(0);
        PrintBinaryPixels(charsAll[i]);
    }

    return 0;
}