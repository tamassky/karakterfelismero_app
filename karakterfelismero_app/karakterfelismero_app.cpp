#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

struct flips
{
    int flipCount;
    int rowCount;
};

int compareFlips(flips a, flips b)
{
    return (a.flipCount < b.flipCount) ? 1 : 0;
}

//https://en.wikipedia.org/wiki/Walsh_matrix
vector<int[64][64]> GenerateWalshMatrices()
{
    vector<int[64][64]> matrices;
    int H[64][64];
    H[0][0] = 1;
    for (int x = 1; x < 64; x += x)
    {
        for (int i = 0; i < x; i++)
        {
            for (int j = 0; j < x; j++)
            {
                H[i + x][j] = H[i][j];
                H[i][j + x] = H[i][j];
                H[i + x][j + x] = -H[i][j];
            }
        }
    }
    flips f[64];
    for (int i = 0; i < 64; i++) 
    {
        f[i].rowCount = i;
        int count = 0;
        for (int j = 1; j < 64; j++) 
        {
            if (H[i][j] != H[i][j - 1]) 
            {
                count++;
            }
        }
        f[i].flipCount = count;
    }
    sort(f, f+64, compareFlips);
    int W[64][64];
    for (int i = 0; i < 64; i++) 
    {
        for (int j = 0; j < 64; j++) 
        {
            W[i][j] = H[f[i].rowCount][j];
        }
    }

    return matrices;
}

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
    Size standardCharSize(64, 64);
    vector<int[64][64]> walshMatrices = GenerateWalshMatrices();

    // Read the image file 
    Mat trainer = imread("C:\\Asztali backup\\Egyetem\\Msc\\1f_Képfeldolgozás_és_orvosi_képalkotás\\tanito.jpg");
    Mat sample = imread("C:\\Asztali backup\\Egyetem\\Msc\\1f_Képfeldolgozás_és_orvosi_képalkotás\\feldolgozando.jpg");

    // Check for failure 
    if (trainer.empty())
    {
        cout << "Tanító képfájl nem található!!!\u201E\u201D" << endl; 
        cin.get(); //wait for any key press 
        return -1;
    }
    if (sample.empty())
    {
        cout << "Feldolgozandó képfájl nem található!!!" << endl;
        cin.get(); //wait for any key press 
        return -1;
    }
    
    /*
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
    */

    Mat trainerGray = BgrToGrayscale(trainer);
    Mat trainerBinary = GrayscaleToBinary(trainerGray, threshold);
    Mat trainerFiltered = FilterIsolatedPixels(trainerBinary);

    imshow("Tanító kép szûrve", trainerFiltered);
    waitKey(0);

    vector<Mat> trainerLines = SliceHorizontally(trainerFiltered);
    vector<Mat> trainerCharsAll;
    for (int i = 0; i < trainerLines.size(); i++)
    {
        imshow("Lépegetõ", trainerLines[i]);
        waitKey(0);
        vector<Mat> chars = SliceVertically(trainerLines[i]);
        trainerCharsAll.insert(trainerCharsAll.end(), chars.begin(), chars.end());
    }

    for (int i = 0; i < trainerCharsAll.size(); i++)
    {
        resize(trainerCharsAll[i], trainerCharsAll[i], standardCharSize);
    }

    for (int i = 0; i < trainerCharsAll.size(); i++)
    {
        imshow("Lépegetõ betû", trainerCharsAll[i]);
        waitKey(0);
        PrintBinaryPixels(trainerCharsAll[i]);
    }

    return 0;
}