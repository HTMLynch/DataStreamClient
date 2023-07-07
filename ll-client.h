//
// ll-client.h - Include file for the ll-client application
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
#ifndef __LL_CLIENT_H__
#define __LL_CLIENT_H__

#if defined(__linux__)
#include    <termios.h>
#else
//#include    "Windows.h"
#endif
#include    <iostream>
#include    <iomanip>
#include    <string>
#include    <map>
#include    <vector>
#include    <mutex>
#include    <thread>
#include    <functional>
#include    <cmath>
#include    "LowLatencyDataClient.h"


// Row and column definitions for various items

//#define WITH_EXTRA_CHANNEL_INFO

#define NAME_COLUMN         1
#define NAME_HEADING_COLUMN 1

#if defined(WITH_EXTRA_CHANNEL_INFO)
#define DATATYPE_COLUMN         (NAME_COLUMN + 15)
#define SCALE_COLUMN            (DATATYPE_COLUMN + 12)
#define OFFSET_COLUMN           (SCALE_COLUMN + 10)
#define SAMPLE_RATE_COLUMN      (OFFSET_COLUMN + 10)
#define DFACTOR_COLUMN          (SAMPLE_RATE_COLUMN + 10)
#define FSTS_COLUMN             (DFACTOR_COLUMN + 8)

#define DT_HEADING_COLUMN       (NAME_HEADING_COLUMN + 14)
#define SCALE_HEADING_COLUMN    (DT_HEADING_COLUMN + 16)
#define OFFSET_HEADING_COLUMN   (SCALE_HEADING_COLUMN + 9)
#define SR_HEADING_COLUMN       (OFFSET_HEADING_COLUMN + 8)
#define DF_HEADING_COLUMN       (SR_HEADING_COLUMN + 5)
#define FSTS_HEADING_COLUMN     (DF_HEADING_COLUMN + 13)

#else

#define SAMPLE_RATE_COLUMN      (NAME_COLUMN + 17)
#define FSTS_COLUMN             (SAMPLE_RATE_COLUMN + 20)
#define TOTAL_SAMPLES_COLUMN    (FSTS_COLUMN + 17)

#define SR_HEADING_COLUMN       (NAME_HEADING_COLUMN + 23)
#define FSTS_HEADING_COLUMN     (SR_HEADING_COLUMN + 18)
#define TOTAL_SAMPLES_HEADING_COLUMN    (FSTS_HEADING_COLUMN + 14)
#endif

#define SAMPLE_COLUMN           (80 - 9)
#define SAMPLE_HEADING_COLUMN   (80 - 4)

#define APP_ROW             1
#define TITLE_ROW           (APP_ROW + 2)
#define ACQ_TIME_ROW        (TITLE_ROW + 2)
#define COLUMN_HEADINGS_ROW (ACQ_TIME_ROW + 2)
#define CHANNEL_START_ROW   (COLUMN_HEADINGS_ROW + 1)
#define LAST_ROW            25

//
// Definiton of per channel information maintained in this application
//
typedef struct {
    ChannelInfo sChannelInfo;           // Info from LowLatencyDataClient object
    int         nChannelId;             // >= 0 = subscribed
    double      dFirstSampleTimestamp;  // Timestamp of first sample
    uint64_t    ullTotalSamples;        // Total number of samples
} ChannelInformationEntry;

//
// Function prototypes
//
void clear_screen(void);
void clear_eos(void);
void clear_eol(void);
void screen_position(int, int);
void center_string(int, std::string&);
void center_string(int, char *);

void DrawLabels(char *appName);

void PrintChannelFSTS(int row, double dFSTS);
void UpdateChannels(void);
void PrintChannelRow(int row, const ChannelInfo& ci);
void UpdateChannelName(bool);
void PrintChannelData(const ChannelDataInfo *);
void PrintAcquisitionStartTime(const std::string&);

void keyPressMonitor(uint32_t, std::function<void(char)>);

#endif
