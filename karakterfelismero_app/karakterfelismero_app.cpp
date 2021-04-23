#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

Size standardCharSize(64, 64);

struct charSeq
{
    char character;
    int propVec[64];
};

struct flips
{
    int flipCount;
    int rowCount;
};

int compareFlips(flips a, flips b)
{
    return (a.flipCount < b.flipCount) ? 1 : 0;
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

//https://en.wikipedia.org/wiki/Walsh_matrix
vector<Mat> GenerateWalshMatrices()
{
    vector<Mat> matrices;
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
            if (H[f[i].rowCount][j] == -1)
            {
                W[i][j] = 255;
            }
            else 
            {
                W[i][j] = 0;
            }
        }
    }
    uint8_t* WArr = (uint8_t*)malloc(sizeof(uint8_t) * 64 * 64);
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            WArr[i * 64 + j] = W[i][j];
        }
    }
    Mat WMat(64, 64, CV_8U, WArr);
    
    for (int i = 0; i < 64; i += 8)
    {
        for (int j = 0; j < 64; j += 8)
        {
            Mat subMat = WMat(Rect(i, j, 8, 8));
            resize(subMat, subMat, standardCharSize);
            matrices.push_back(subMat);
        }
    }
    return matrices;
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

charSeq* GetTrainerValues(char* chars, vector<Mat> trainerChars, vector<Mat> walshMatrices)
{
    charSeq* trainers = (charSeq*)malloc(sizeof(charSeq) * trainerChars.size());
    for (int j = 0; j < 80; j++) 
    {
        trainers[j].character = chars[j];
        for (int i = 0; i < 64; i++)
        {
            int commonPoints = 0;
            for (int x = 0; x < 64; x++)
            {
                for (int y = 0; y < 64; y++)
                {
                    int pixelWalsh = (int)walshMatrices[i].at<uchar>(x, y);
                    int pixelChar = (int)trainerChars[j].at<uchar>(x, y);
                    if (pixelWalsh == 0 && pixelChar == 0)
                    {
                        commonPoints++;
                    }
                }
            }
            trainers[j].propVec[i] = commonPoints;
        }
    }
    return trainers;
}

charSeq* GetSampleValues(vector<Mat> sampleChars, vector<Mat> walshMatrices)
{
    charSeq* samples = (charSeq*)malloc(sizeof(charSeq) * sampleChars.size());
    for (int j = 0; j < sampleChars.size(); j++)
    {
        for (int i = 0; i < 64; i++)
        {
            int commonPoints = 0;
            for (int x = 0; x < 64; x++)
            {
                for (int y = 0; y < 64; y++)
                {
                    int pixelWalsh = (int)walshMatrices[i].at<uchar>(x, y);
                    int pixelChar = (int)sampleChars[j].at<uchar>(x, y);
                    if (pixelWalsh == 0 && pixelChar == 0)
                    {
                        commonPoints++;
                    }
                }
            }
            samples[j].propVec[i] = commonPoints;
        }
    }
    return samples;
}

char* compareSamplesWithTrainers(charSeq* samples, charSeq* trainers, int samplesSize, int trainersSize)
{
    char* solution = (char*)malloc(sizeof(char) * samplesSize);
    for (int i = 0; i < samplesSize; i++) 
    {
        int diffSum = INT_MAX;
        for (int j = 0; j < trainersSize; j++)
        {
            int diff = 0;
            for (int k = 0; k < 64; k++)
            {
                diff += abs(samples[i].propVec[k] - trainers[j].propVec[k]);
            }
            if (diff < diffSum) 
            {
                diffSum = diff;
                solution[i] = trainers[j].character;
                cout << solution[i];
            }
        }
        cout << endl;
    }
    return solution;
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    int threshold = 150;
    vector<Mat> walshMatrices = GenerateWalshMatrices();

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

    Mat trainerGray = BgrToGrayscale(trainer);
    Mat trainerBinary = GrayscaleToBinary(trainerGray, threshold);
    Mat trainerFiltered = FilterIsolatedPixels(trainerBinary);

    vector<Mat> trainerLines = SliceHorizontally(trainerFiltered);
    vector<Mat> trainerCharsAll;
    for (int i = 0; i < trainerLines.size(); i++)
    {
        vector<Mat> chars = SliceVertically(trainerLines[i]);
        trainerCharsAll.insert(trainerCharsAll.end(), chars.begin(), chars.end());
    }

    for (int i = 0; i < trainerCharsAll.size(); i++)
    {
        resize(trainerCharsAll[i], trainerCharsAll[i], standardCharSize);
    }

    char chars[] = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789,.!?-()’„”_+%=/@<>";
    charSeq* trainers = GetTrainerValues(chars, trainerCharsAll, walshMatrices);

    imshow("Feldolgozandó kép", sample);
    waitKey(0);
    Mat sampleGray = BgrToGrayscale(sample);
    imshow("Feldolgozandó kép szürkében", sampleGray);
    waitKey(0);
    Mat sampleBinary = GrayscaleToBinary(sampleGray, threshold);
    imshow("Feldolgozandó kép binárisan", sampleBinary);
    waitKey(0);
    Mat sampleFiltered = FilterIsolatedPixels(sampleBinary);
    imshow("Feldolgozandó kép szûrve", sampleFiltered);
    waitKey(0);

    vector<Mat> sampleLines = SliceHorizontally(sampleFiltered);
    vector<Mat> sampleCharsAll;
    for (int i = 0; i < sampleLines.size(); i++)
    {
        vector<Mat> chars = SliceVertically(sampleLines[i]);
        sampleCharsAll.insert(sampleCharsAll.end(), chars.begin(), chars.end());
    }

    for (int i = 0; i < sampleCharsAll.size(); i++)
    {
        resize(sampleCharsAll[i], sampleCharsAll[i], standardCharSize);
    }

    charSeq* samples = GetSampleValues(sampleCharsAll, walshMatrices);

    char* solution;
    solution = compareSamplesWithTrainers(samples, trainers, sampleCharsAll.size(), trainerCharsAll.size());

    for (int i = 0; i <= sampleCharsAll.size(); i++)
    {
        cout << solution[i];
    }

    return 0;
}