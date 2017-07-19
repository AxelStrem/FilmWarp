
#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <array>
#include <numeric>
#include <functional>

#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp> 
#include <opencv2\videoio.hpp>
#include <opencv2\highgui.hpp>

template<class T, class Compare>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp)
{
    return assert(!comp(hi, lo)),
        comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    return clamp(v, lo, hi, std::less<>());
}

struct IOError
{
    std::string message;
};