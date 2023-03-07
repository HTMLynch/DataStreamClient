#ifndef __LOWLATENCYCLIENT_H__
#define __LOWLATENCYCLIENT_H__

#include    <map>
#include    <vector>
#include    <mutex>
#include    <memory>
#include    <thread>
#include    <boost/asio.hpp>
#include    <nlohmann/json.hpp>

using json = nlohmann::json;
using boost::asio::ip::tcp;

#define METADATA_ID 0x80000000

//
// Definition of information for an available and/or subscribed channel
//
typedef struct {
    std::string         sName;                  // From server "available"
    std::string         sDataType;              // From server "available"
    double              dScale;                 // From server "available"
    double              dOffset;                // From server "available"
    double              dSamplePeriod;          // From server "available"
    uint32_t            nDecimationFactor;      // From user
} ChannelInfo;

//
// Definition of the header for packets sent by the low latency server
//
// NOTE:  All fields are in network byte order
//
typedef struct {
    uint32_t    id;             // if upper bit is set packet is JSON metadata
                                // otherwise it is channel id for which the
                                // accompanying data belongs

    uint32_t    length;         // Length of the data for the packet inclusive
                                // of the length of this header
} LowLatencyStreamPacketHeader;

//
// Event types for first argument to user event handler
//
typedef enum {
    EVENT_TYPE_AVAILABLE_CHANNEL,       // "available" JSON received
    EVENT_TYPE_UNAVAILABLE_CHANNEL,     // "unavailale" JSON received
    EVENT_TYPE_CHANNEL_SUBSCRIBED,      // "subscribed" JSON received
    EVENT_TYPE_CHANNEL_UNSUBSCRIBED,    // "unsubscribed" JSON received
    EVENT_TYPE_CHANNEL_FIRST_SAMPLE_TS, // "first_sample_ts" JSON received
    EVENT_TYPE_CHANNEL_DATA,            // Channel data received
    EVENT_TYPE_ACQUIRE,                 // "acquisitio_state" JSON received
} EventType;

//
// Event type specific data provided with each event type
//
typedef struct {
    std::string sName;                      // Name of the channel
    int         nId;                        // Subcribed ID of the channel
    double      dFirstSampleTimestamp;      // Timestamp of first sample
} ChannelTimestampInfo;

typedef struct {
    std::string sName;                      // Name of the channel
    int         nId;                        // Assigned Id
} ChannelSubscribedInfo;

typedef struct {
    int         nId;                        // Id of channel unsubscribed
} ChannelUnsubscribedInfo;

typedef struct {
    int         nId;                        // Id of the channel this data is 4
    float       *pData;                     // Pointer to the data
    size_t      nSamples;                   // Number of data samples
} ChannelDataInfo;

//
// Type for user event handler function
//
typedef std::function<void(EventType, const void *, size_t)> EventHandler;


//
// Definition of the low latency client class
//
class LowLatencyDataClient {
    public:
        LowLatencyDataClient() = default;

        ~LowLatencyDataClient();

        LowLatencyDataClient(boost::asio::io_context& io_context,
            std::string&, std::string&, EventHandler);

        void SubscribeChannel(std::string&, int nDecimationFactor = 1);

        void SubscribeChannels(const std::vector<ChannelInfo>&);

        int SubscribedChannelId(std::string&);

        void UnsubscribeChannels(const std::vector<int>&);
        void UnsubscribeChannel(int);

        void Acquire(void);

    private:

        void Connect(boost::asio::io_context& io_context,
            std::string&, std::string&);

        void ProcessPacket(LowLatencyStreamPacketHeader *);

        void ProcessDataPacket(LowLatencyStreamPacketHeader *);

        void ProcessUnsubscribeResponsePacket(json&);
        void ProcessSubscribeResponsePacket(json&);
        void ProcessAvailableChannelsPacket(json&);
        void ProcessUnavailableChannelsPacket(json&);
        void ProcessAcquisitionStatePacket(json&);
        void ProcessMetadataPacket(LowLatencyStreamPacketHeader *);

        void SendSubscribeChannels(const std::vector<ChannelInfo>&);
        void SendUnsubscribeChannels(const std::vector<int>&);

        void SocketReadThread(void);

        std::thread                         *m_SocketReadThread;
        volatile bool                       m_bSocketReadThreadExit;

        std::mutex                          m_AvailableChannelsListLock;
        std::mutex                          m_SubscribedChannelsListLock;

        std::map<std::string, ChannelInfo>  m_mAvailableChannelsList;
        std::map<int, ChannelInfo>          m_mSubscribedChannelsList;

        std::vector<ChannelInfo>            m_vPendingSubscribeChannelsList;

        EventHandler                        m_fEventHandler;

        enum { recvBufSize = (1024 * 1024 * 16) };

        uint8_t                             *m_Data;

        bool                                m_bAcquisitionState;

        std::unique_ptr<tcp::socket>        m_Socket;

        std::map<std::string, double>       m_mFSTS;    // first sample ts's
};

#endif
