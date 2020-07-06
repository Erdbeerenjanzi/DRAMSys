/*
 * Copyright (c) 2015, Technische Universität Kaiserslautern
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *    Robert Gernhardt
 *    Matthias Jung
 *    Eder F. Zulian
 *    Luiza Correa
 */

#ifndef UTILS_H
#define UTILS_H

#include <systemc.h>
#include <map>
#include <string>
#include <ostream>
#include <tlm.h>
#include <iomanip>
#include "dramExtensions.h"
#include <fstream>
#include <sstream>
#include "../common/third_party/nlohmann/single_include/nlohmann/json.hpp"

class TimeInterval
{
public:
    sc_time start, end;
    TimeInterval() : start(SC_ZERO_TIME), end(SC_ZERO_TIME) {}
    TimeInterval(sc_time start, sc_time end) : start(start), end(end) {}

    sc_time getLength();
    bool timeIsInInterval(sc_time time);
    bool intersects(TimeInterval other);
};

constexpr const char headline[] =
    "===========================================================================";

static inline void loadbar(unsigned int x,
                           unsigned int n,
                           unsigned int w = 50,
                           unsigned int granularity = 1)
{
    if ((n < 100) || ((x != n) && (x % (n / 100 * granularity) != 0)))
        return;

    float ratio = x / (float) n;
    unsigned int c = (ratio * w);
    float rest = (ratio * w) - c;
    std::cout << std::setw(3) << round(ratio * 100) << "% |";
    for (unsigned int x = 0; x < c; x++)
        std::cout << "█";

    if (rest >= 0 && rest < 0.125 && c != w)
        std::cout << " ";
    if (rest >= 0.125 && rest < 2 * 0.125)
        std::cout << "▏";
    if (rest >= 2 * 0.125 && rest < 3 * 0.125)
        std::cout << "▎";
    if (rest >= 3 * 0.125 && rest < 4 * 0.125)
        std::cout << "▍";
    if (rest >= 4 * 0.125 && rest < 5 * 0.125)
        std::cout << "▌";
    if (rest >= 5 * 0.125 && rest < 6 * 0.125)
        std::cout << "▋";
    if (rest >= 6 * 0.125 && rest < 7 * 0.125)
        std::cout << "▊";
    if (rest >= 7 * 0.125 && rest < 8 * 0.125)
        std::cout << "▉";

    for (unsigned int x = c; x < (w - 1); x++)
        std::cout << " ";
    std::cout << "|\r" << std::flush;
}

std::string getPhaseName(tlm::tlm_phase phase);

nlohmann::json parseJSON(std::string path);
bool parseBool(nlohmann::json &obj, std::string name);
unsigned int parseUint(nlohmann::json &obj, std::string name);
double parseUdouble(nlohmann::json &obj, std::string name);
std::string parseString(nlohmann::json &obj, std::string name);

void setUpDummy(tlm::tlm_generic_payload &payload, Rank rank = Rank(0), Bank bank = Bank(0));

#endif // UTILS_H

