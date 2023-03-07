//
// ll-client.cpp - Low Latency Streaming Data Client example
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

//
// Global debug flag
int nDebug = 0;

//
// Map of channel information
//
std::map<std::string, ChannelInformationEntry>  g_mChannelInformation;

//
// Resource lock for channel information list above
//
std::recursive_mutex    g_lChannelInformationLock;

//
// Current channel row the cursor is on
//
int currentChannelRow = 0;

//
// IO Context for Boost
//
static boost::asio::io_context io_context;

//
// Function used to show help on how to use this application
//
static void usage(void)
{
    std::vector<std::string>    vUsageStrings = {
        "",
        "USAGE: ll-client <host> [port]",
        "",
        "   host    IP address of host to connect to",
        "   port    Port number of host to connect to (def=10006)",
        ""
    };

    for(auto& e : vUsageStrings) std::cout << e << std::endl;
}

//
// Funtion used to remove an available channel
//
static void RemoveAvailableChannel(const std::string& sName)
{
    std::unique_lock<std::recursive_mutex>  lk(g_lChannelInformationLock);

    if(g_mChannelInformation.find(sName) !=
        g_mChannelInformation.end()) {
        g_mChannelInformation.erase(sName);
    }
}

//
// Event handler function to handle available channel events
//
static void HandleAvailableChannelEvent(const void *p)
{
    ChannelInformationEntry cie;
    const ChannelInfo *ci = reinterpret_cast<const ChannelInfo *>(p);
    cie.sChannelInfo = *ci;
    cie.nChannelId = -1;
    cie.dFirstSampleTimestamp = NAN;
    cie.ullTotalSamples = 0ULL;
    PrintChannelRow(g_mChannelInformation.size(), cie.sChannelInfo);

    std::unique_lock<std::recursive_mutex> lk(g_lChannelInformationLock);

    g_mChannelInformation[ci->sName] = cie;
    UpdateChannels();
}

//
// Event handler function used to handle unavaliable channel events
//
static void HandleUnavailableChannelEvent(const void *p)
{
    std::string sName(reinterpret_cast<char *>(const_cast<void *>(p)));
    RemoveAvailableChannel(sName);
    UpdateChannels();
}

//
// Event handler to handle channel subscribed events
//
static void HandleChannelSubscribedEvent(const void *p)
{
    const ChannelSubscribedInfo   *csi =
        reinterpret_cast<const ChannelSubscribedInfo *>(p);
    std::unique_lock<std::recursive_mutex>  lk(g_lChannelInformationLock);

    if(g_mChannelInformation.find(csi->sName) !=
        g_mChannelInformation.end()) {
        g_mChannelInformation[csi->sName].nChannelId = csi->nId;

        int row = std::distance(g_mChannelInformation.begin(),
            g_mChannelInformation.find(csi->sName));

        g_mChannelInformation[csi->sName].ullTotalSamples = 0ULL;

        PrintChannelRow(row,
            g_mChannelInformation[csi->sName].sChannelInfo);
    }
}

//
// Event handler function to handle channel unsubscribed events
//
static void HandleChannelUnsubscribedEvent(const void *p)
{
    const ChannelUnsubscribedInfo   *cui =
        reinterpret_cast<const ChannelUnsubscribedInfo *>(p);
    std::unique_lock<std::recursive_mutex>
        lk(g_lChannelInformationLock);

    for(auto it = g_mChannelInformation.begin();
        it != g_mChannelInformation.end(); it++) {
        if((*it).second.nChannelId == cui->nId) {
            (*it).second.nChannelId = -1;
            (*it).second.dFirstSampleTimestamp = NAN;

            int row = std::distance(g_mChannelInformation.begin(), it);
            PrintChannelRow(row, (*it).second.sChannelInfo);
        }
    }
}

//
// Function used to subscribe/unsubscribe channels
//
static void ToggleSubscribeState(int nChannelRow, LowLatencyDataClient& llc)
{
    if(static_cast<size_t>(nChannelRow) > g_mChannelInformation.size()) return;

    for(auto it = g_mChannelInformation.begin();
        it != g_mChannelInformation.end(); it++) {
        if(std::distance(g_mChannelInformation.begin(), it) == nChannelRow) {

            ChannelInformationEntry& ci = (*it).second;
            std::string sName = (*it).first;

            if(ci.nChannelId < 0) llc.SubscribeChannel(sName);
            else llc.UnsubscribeChannel(ci.nChannelId);
        }
    }
}

//
// Function used to toggle acquistion on/off
//
static void ToggleAcquisitionState(LowLatencyDataClient& llc)
{
    llc.Acquire();
}

//
// Function used to unsubscribe from ALL subscribed channels
//
static void UnsubscribeAll(LowLatencyDataClient& llc)
{
    for(auto it = g_mChannelInformation.begin();
        it != g_mChannelInformation.end(); it++) {
        if((*it).second.nChannelId != -1) {

            int row = std::distance(g_mChannelInformation.begin(), it);
            ToggleSubscribeState(row, llc);

            while((*it).second.nChannelId != -1)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));

            PrintChannelRow(row, (*it).second.sChannelInfo);
        }
    }
}

//
// Event handler to handle channel first sample timestamp events
//
static void HandleChannelFirstSampleTimestampEvent(const void *p)
{
    const ChannelTimestampInfo  *ctsi =
        reinterpret_cast<const ChannelTimestampInfo *>(p);

    std::unique_lock<std::recursive_mutex>
        lk(g_lChannelInformationLock);

    if(g_mChannelInformation.find(ctsi->sName) !=
        g_mChannelInformation.end()) {
        if(g_mChannelInformation[ctsi->sName].nChannelId ==
            ctsi->nId) {
            g_mChannelInformation[ctsi->sName].dFirstSampleTimestamp =
                ctsi->dFirstSampleTimestamp;

            int row = std::distance(g_mChannelInformation.begin(),
                g_mChannelInformation.find(ctsi->sName));

            PrintChannelFSTS(CHANNEL_START_ROW + row,
                ctsi->dFirstSampleTimestamp);
        }
    }
}

//
// Event handler to handle channel data events
//
static void HandleChannelDataEvent(const void *p)
{
    const ChannelDataInfo   *cdi =
        reinterpret_cast<const ChannelDataInfo *>(p);
    PrintChannelData(cdi);
}

//
// Event handler to handle aquire events
//
static void HandleAcquireEvent(const void *p)
{
    const bool  bState = *reinterpret_cast<const bool *>(p);

    if(!bState) {
        // If acquisition is stopping, clear the sample counts
        for(auto& e : g_mChannelInformation) e.second.ullTotalSamples = 0ULL;
    }
}

//
// Map of event types to event handlers
//
static std::map<EventType, std::function<void(const void *)>>
    g_mEventHandlers = {
        {
            EVENT_TYPE_AVAILABLE_CHANNEL,
            HandleAvailableChannelEvent
        },
        {
            EVENT_TYPE_UNAVAILABLE_CHANNEL,
            HandleUnavailableChannelEvent
        },
        {
            EVENT_TYPE_CHANNEL_SUBSCRIBED,
            HandleChannelSubscribedEvent
        },
        {
            EVENT_TYPE_CHANNEL_UNSUBSCRIBED,
            HandleChannelUnsubscribedEvent
        },
        {
            EVENT_TYPE_CHANNEL_FIRST_SAMPLE_TS,
            HandleChannelFirstSampleTimestampEvent
        },
        {
            EVENT_TYPE_ACQUIRE,
            HandleAcquireEvent
        },
        {
            EVENT_TYPE_CHANNEL_DATA,
            HandleChannelDataEvent
        },
};

//
// Event handler function
//
static void HandleEvents(EventType nType, const void *p, size_t nSize)
{
    if(g_mEventHandlers.find(nType) == g_mEventHandlers.end()) {
        std::cerr << "Unknown event type " << nType << std::endl;
        return;
    }
    g_mEventHandlers[nType](p);
}

//
// Entry point of the application
//
int main(int argc, char *argv[])
{
    // Make sure the correct number of arguments has been given
    if(argc < 2 || argc > 3) {
        usage();
        return 0;
    }

    // Get host and, optionally, port from command line
    std::string host(argv[1]);
    std::string port("10006");
    if(argc > 2) port.assign(argv[2]);

    // Draw all of the static text on the screen
    DrawLabels(argv[0]);

    // Instatiate the connection to the low latency data server
    LowLatencyDataClient    llc(io_context, host, port, HandleEvents);

    // Monitor for keypresses
    keyPressMonitor(100, [&llc](char key) mutable {
        if(key == 'u') {
            // Move channel row up 1
            UpdateChannelName(false);
            currentChannelRow--;
            if(currentChannelRow < 0)
                currentChannelRow = g_mChannelInformation.size() - 1;
            UpdateChannelName(true);

        } else if(key == 'd') {
            // Move channel row down 1
            UpdateChannelName(false);
            currentChannelRow++;
            if(static_cast<size_t>(currentChannelRow) >=
                g_mChannelInformation.size())
                currentChannelRow = 0;
            UpdateChannelName(true);

        } else if(key == 'D') {
            nDebug ^= 1;

        } else if(key == ' ') {
            // Subscribe/Unsubscribe channel on current channel row
            ToggleSubscribeState(currentChannelRow, llc);

        } else if(key == 'a') {
            // Acquisition start/stop
            ToggleAcquisitionState(llc);
        }

    });


    // Unsubscribe from all of the subscribed channels before leaving
    UnsubscribeAll(llc);

    // Leaving, posistion curson on last row and clear the line
    screen_position(LAST_ROW, 1);
    clear_eol();

    // Return success
    return 0;
}
