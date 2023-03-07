//
// display.cpp - Display functions used in low latency streaming client example
//
//
// Written by:  Michael J. Lynch  (mlynch@hi-techiques.com)
//
// Copyright (c) 2023 by Hi-Techniques Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
#include    "ll-client.h"
#if ! defined(__linux__)
#include    "Windows.h"
#endif

extern int                  currentChannelRow;
extern std::recursive_mutex g_lChannelInformationLock;

extern std::map<std::string, ChannelInformationEntry>   g_mChannelInformation;

static std::recursive_mutex l_PrintLock;

//
// Clear the screen
//
void clear_screen(void) {
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    std::cout << "[2J" << std::flush;
}

//
// Clear from cursor location to end of screen
//
void clear_eos(void) {
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    std::cout << "[J" << std::flush;
}

//
// Clear from cursor locattion to end of line
//
void clear_eol(void) {
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    std::cout << "[0K" << std::flush;
}

//
// Set cursor position
//
void screen_position(int row, int col) {
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    std::cout << "[" << row << ";" << col << "H" << std::flush;
}

//
// Center the string on the indicated line
//
void center_string(int row, std::string& s) {
    int c = (80 - s.size()) / 2;

    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, c);
    std::cout << s << std::flush;
}

//
// Center the C string on the indicated line
//
void center_string(int row, char *cs) {
    std::string s(cs);
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    center_string(row, s);
}

//
// Function used to print the channel name
//
static void PrintChannelName(int row, std::string sName,
    bool bHighlight = false)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, NAME_COLUMN);

    if(bHighlight && row - CHANNEL_START_ROW == currentChannelRow)
        std::cout << "[5m" << std::flush;
    else
        std::cout << "[m" << std::flush;

    std::cout << sName << std::flush;

    if(g_mChannelInformation.find(sName) !=
        g_mChannelInformation.end()) {
        if(g_mChannelInformation[sName].nChannelId >= 0)
            std::cout << " (id=" <<
                g_mChannelInformation[sName].nChannelId << ")" << std::flush;
    }

    if(bHighlight && row - CHANNEL_START_ROW == currentChannelRow)
        std::cout << "[m" << std::flush;

    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

#if defined(WITH_EXTRA_CHANNEL_INFO)
//
// Function used to print the channel data type
//
static void PrintChannelDataType(int row, std::string sDataType)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, DATATYPE_COLUMN);
    std::cout << ci.sDataType << std::flush;
    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to print the scale value for the channel
//
static void PrintChannelScale(int row, double dScale)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, SCALE_COLUMN);
    std::cout << std::setw(8) << std::setprecision(2) << std::fixed <<
        std::right << dScale << std::flush;
    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to print the offset value for the channel
//
static void PrintChannelOffset(int row, double dOffset)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, OFFSET_COLUMN);
    std::cout << std::setw(8) << std::setprecision(2) << std::fixed <<
        std::right << dOffset << std::flush;
    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to print the decimation factor for the channel
//
static void PrintChannelDecimationFactor(int row, int nDFactor)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, DFACTOR_COLUMN);
    std::cout << std::setw(6) << std::right << nDFactor << std::flush;
    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}
#endif

//
// Function used to print the sample rate for the channel
//
static void PrintChannelSampleRate(int row, double dSampleRate)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, SAMPLE_RATE_COLUMN);
    std::cout << std::setw(10) << std::setprecision(2) << std::fixed <<
        std::right << dSampleRate << std::flush;
    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to print the first sample timestamp for the channel
//
static void PrintChannelTotalSamples(int row, uint64_t ullTotalSamples)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, TOTAL_SAMPLES_COLUMN);
    std::cout << std::setw(12) << std::right << ullTotalSamples  << std::flush;
    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to print the first sample timestamp
//
void PrintChannelFSTS(int row, double dFSTS)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    screen_position(row, FSTS_COLUMN);
    if(!std::isnan(dFSTS))
        std::cout << std::setw(12) << std::setprecision(6) << std::fixed <<
            std::right << dFSTS << std::flush;
    else
        std::cout << std::setw(12) << std::setprecision(6) << std::fixed <<
            std::right << "N/A" << std::flush;

    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Print an entire channel row of data items
//
void PrintChannelRow(int row, const ChannelInfo& ci)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    // Adjust row to start of channel area
    row += CHANNEL_START_ROW;

    // Channel name is first
    screen_position(row, NAME_COLUMN);
    clear_eol();
    PrintChannelName(row, ci.sName, true);

#if defined(WITH_EXTRA_CHANNEL_INFO)
    PrintChannelDataType(row, ci.sDataType);
    PrintChannelScale(row, ci.dScale);
    PrintChannelOffset(row, ci.dOffset);
    PrintChannelSampleRate(row, 1.0 / ci.dSamplePeriod);
    PrintChannelDecimationFactor(row, ci.nDecimationFactor);
#else
    PrintChannelSampleRate(row, 1.0 / ci.dSamplePeriod);
    PrintChannelTotalSamples(row,
        g_mChannelInformation[ci.sName].ullTotalSamples);
#endif

    if(g_mChannelInformation.find(ci.sName) != g_mChannelInformation.end())
        PrintChannelFSTS(row,
            g_mChannelInformation[ci.sName].dFirstSampleTimestamp);
    else
        PrintChannelFSTS(row, NAN);

    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to print a data value for a channel.  Note:  only the last
// sample from the buffer is printed.
//
void PrintChannelData(const ChannelDataInfo *cdi)
{
    std::unique_lock<std::recursive_mutex>  lk(l_PrintLock);
    // Find the channel information for the channel id
    for(auto it = g_mChannelInformation.begin();
        it != g_mChannelInformation.end(); it++) {

        if((*it).second.nChannelId == cdi->nId) {
            // Channel information found, set screen position and print the
            // value
            int row = std::distance(g_mChannelInformation.begin(), it);
            screen_position(CHANNEL_START_ROW + row, SAMPLE_COLUMN);

            double dScale = (*it).second.sChannelInfo.dScale;
            double dOffset = (*it).second.sChannelInfo.dOffset;
            (*it).second.ullTotalSamples += cdi->nSamples;

            std::cout << std::setw(10) << std::setprecision(6) << std::fixed <<
                std::right <<
                ((cdi->pData[cdi->nSamples - 1] * dScale) + dOffset) <<
                std::flush;

            PrintChannelTotalSamples(row + CHANNEL_START_ROW,
                (*it).second.ullTotalSamples);

            screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
            return;
        }
    }
}

//
// Function used to draw all of the fixed text on the screen
//
void DrawLabels(char *appName)
{
#if ! defined(__linux__)
    // This makes ANSI escape sequences work on Windows
    ::system(" ");
#endif

    // Clear the screen
    clear_screen();

    // Show version, build date, and build number
    std::string str(appName);
    center_string(APP_ROW, str);

    std::string heading("Available Channels");
    center_string(TITLE_ROW, heading);

    screen_position(COLUMN_HEADINGS_ROW, NAME_HEADING_COLUMN);
    std::cout << "NAME" << std::flush;
#if defined(WITH_EXTRA_CHANNEL_INFO)
    screen_position(COLUMN_HEADINGS_ROW, DT_HEADING_COLUMN);
    std::cout << "DATATYPE" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, SCALE_HEADING_COLUMN);
    std::cout << "SCALE" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, OFFSET_HEADING_COLUMN);
    std::cout << "OFFSET" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, SR_HEADING_COLUMN);
    std::cout << "RATE" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, DF_HEADING_COLUMN);
    std::cout << "DFACTOR" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, FSTS_HEADING_COLUMN);
    std::cout << "FSTS (s)" << std::flush;
#else
    screen_position(COLUMN_HEADINGS_ROW, SR_HEADING_COLUMN);
    std::cout << "RATE" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, FSTS_HEADING_COLUMN);
    std::cout << "FSTS (s)" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, TOTAL_SAMPLES_HEADING_COLUMN);
    std::cout << "NUM SAMPLES" << std::flush;
    screen_position(COLUMN_HEADINGS_ROW, SAMPLE_HEADING_COLUMN);
    std::cout << "VALUE" << std::flush;
#endif

    screen_position(LAST_ROW, 1);
    std::cout <<
        "'q'=quit, 'u'=up, 'd'=down, 'a'=acq on/off, <space>=sub/unsub" <<
        std::flush;

    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to redraw all of the channel rows.
//
void UpdateChannels(void)
{
    std::unique_lock<std::recursive_mutex>  lk(g_lChannelInformationLock);

    std::unique_lock<std::recursive_mutex>  plk(l_PrintLock);
    currentChannelRow = 0;
    screen_position(CHANNEL_START_ROW, 1);
    clear_eos();

    for(auto it = g_mChannelInformation.begin();
        it != g_mChannelInformation.end(); it++) {

        ChannelInformationEntry cie = (*it).second;
        int row = std::distance(g_mChannelInformation.begin(), it);

        PrintChannelRow(row, cie.sChannelInfo);
    }

    screen_position(LAST_ROW, 1);
    std::cout <<
        "'q'=quit, 'u'=up, 'd'=down, 'a'=acq on/off, <space>=sub/unsub" <<
        std::flush;

    screen_position(currentChannelRow + CHANNEL_START_ROW, NAME_COLUMN);
}

//
// Function used to update the channel name when moving the cursor
//
void UpdateChannelName(bool bHighlight)
{
    for(auto it = g_mChannelInformation.begin();
        it != g_mChannelInformation.end(); it++) {

        if(std::distance(g_mChannelInformation.begin(), it) ==
            currentChannelRow) {

            PrintChannelName(currentChannelRow + CHANNEL_START_ROW,
                (*it).second.sChannelInfo.sName, bHighlight);

            return;
        }
    }
}
