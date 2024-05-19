// This header file is for plugin development use

#ifndef SUPERNOVA_DEV
#define SUPERNOVA_DEV

#include <string>
using std::string;

namespace supernova
{

typedef string UUID;

class SupernovaChannel
{
    public:
    // This function initializes the data if the data does not exist
    virtual void initialize_data (void) =0;
    // Load existing data
    // This function is called at the beginning
    virtual void load_data_start (int lines) =0;
    // This function is called multiple times
    virtual void load_data_line (string data) =0;
    // This function is used to finalize the load, which is not necessary but might be useful
    virtual void load_data_end (void) =0;
    // This function is used to receive updates from the remote
    virtual void receive_update (int fd) =0;
    // This function is used to return data
    virtual void query_data (int fd) =0;
    // Saving data
    // This function returns the lines of total data
    // and initializes the save
    virtual int save_data_start (void) =0;
    // This function returns a line which should contain chars between 0x21 and 0x7e
    virtual string save_data_line (int line) =0;
    // Unnecessary but might be useful
    virtual void save_data_end (void) =0;
};

typedef SupernovaChannel* (*SupernovaChannelEntrance) (UUID player);

class SupernovaPlugin
{
    public:
    virtual string name (void) =0;
    virtual string version (void) =0;
    // Get all the channels
    // This function returns the total channels
    virtual int channel_count (void) =0;
    // This function returns a channel
    virtual SupernovaChannelEntrance get_channel (int index) =0;
    // This function returns a channel name
    virtual string channel_name (int index) =0;
};

// To let Supernova load your plugin,
// You should have a SupernovaPluginEntrance named entrance in your compiled lib
// For example, you can use *SupernovaPluginEntrance entrance;* in your code
typedef SupernovaPlugin* (*SupernovaPluginEntrance) (const char* version);

}

#endif
