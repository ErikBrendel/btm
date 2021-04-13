#pragma once

#include<random>
#include <chrono>

#define all(x) std::begin(x), std::end(x)
#define rep(i, n) for (unsigned int i = 0; i < (n); ++i)


#define timeNow() std::chrono::steady_clock::now()
#define timeDiff(a, b) std::chrono::duration_cast<std::chrono::milliseconds>((b) - (a)).count()
#define timeSince(a) timeDiff(a, timeNow())


//#define randrange(upperbound) (rand() % (upperbound))
//#define randrange(upperbound) std::uniform_int_distribution<int>(0, upperbound)(my_randomengine)

double square(double val);
