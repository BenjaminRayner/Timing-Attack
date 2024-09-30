#include <iostream>
#include <math.h>
using namespace std;

const int ITERATIONS = 10000000;
const int CHAR_OFFSET = 97;
const int ALPHABET_LEN = 26;
const double Z_VALUE = 1.96;    //95%

// Check input against c
bool check_password (const char* pwd)
{
    const char* c = "password";
    while (*pwd == *c && *pwd != '\0' && *c != '\0')
    {
        ++pwd; ++c;
    }
    return *pwd == *c && *pwd == '\0';
}

// Clock cycle counter 
static __inline__ uint64_t rdtsc()
{
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

// Check for what character's interval does not have intersections
char checkIntersect(double arr[][2])
{
    for (int i = 0; i < ALPHABET_LEN; ++i)
    {
        int j;
        // Check if interval does not intersect with all other intervals
        for (j = 0; j < ALPHABET_LEN; ++j)
        {
            if (((arr[i][0] > arr[j][0]) && (arr[i][0] < arr[j][1]))            // start overlap
             || ((arr[i][1] > arr[j][0]) && (arr[i][1] < arr[j][1]))            // end overlap
             || ((arr[i][0] < arr[j][0]) && (arr[i][1] > arr[j][1]))) break;    // full overlap
        }
        // Character's confidence interval does not intersect
        if (j == ALPHABET_LEN) return i + CHAR_OFFSET;
    }
 
    return 0;
}

int main(int argc, char* argv[])
{
    srand(time(0));
    string pwd = " ";
    int index = 0;
    while (true)
    {
        double N[ALPHABET_LEN] = {0};
        uint64_t sum_t[ALPHABET_LEN] = {0};
        uint64_t sum_tt[ALPHABET_LEN] = {0};
        double confInterval[ALPHABET_LEN][2];

        // Sample time and time^2 of check_password with random last character
        for (int i = 0; i < ITERATIONS; ++i)
        {
            char letter = rand() % ('z' - 'a' + 1) + 'a';
            pwd[index] = letter;
            const char* pwdArray = pwd.c_str();

            uint64_t startTime = rdtsc();
            check_password(pwdArray);
            uint64_t endTime = rdtsc();

            uint64_t time = endTime - startTime;
            int charMap = letter - CHAR_OFFSET;
            sum_t[charMap] += time;
            sum_tt[charMap] += time * time;
            ++N[charMap];
        }
        // Get confidence interval for the time taken for each different password
        for (int i = 0; i < ALPHABET_LEN; ++i)
        {
            double mu = sum_t[i]/N[i];
            double sigmaSquared = 1/(N[i]-1) * (sum_tt[i]-N[i]*(mu*mu));

            double lowerInterval = mu - Z_VALUE * sqrt(sigmaSquared / N[i]);
            double upperInterval = mu + Z_VALUE * sqrt(sigmaSquared / N[i]);
            confInterval[i][0] = lowerInterval;
            confInterval[i][1] = upperInterval;
        }

        // Debug
        cout << "Index" << " : " << index << endl;
        for (int i = 0; i < ALPHABET_LEN; ++i)
        {
            cout << char(i+CHAR_OFFSET) << " : " << confInterval[i][0] << " - " << confInterval[i][1] << endl;
        }

        // If a password's confidence does not intersect, we assume that password is closer to true password
        char letter = checkIntersect(confInterval);
        if (letter != 0) pwd[index] = letter;
        else { cout << "Not enough confidence!" << endl; continue; }
        
        // If correct password, stop
        if (check_password(pwd.c_str())) break;

        // Try next index
        pwd.resize(++index + 1);
    }

    cout << "Password is: " << pwd << endl;
}