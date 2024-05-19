#include "supernova.hpp"
using namespace supernova;

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>

#include <iostream>
using std::cout;
using std::endl;

class GameTimeChannel: public SupernovaChannel
{
    long joined, stayed, last;
    public:
    GameTimeChannel (void): joined (time (nullptr)), stayed (0), last (time (nullptr)) {}
    virtual void initialize_data (void) override {}
    virtual void load_data_start (int) override {}
    virtual void load_data_line (string data) override
    {
        if (data.empty ()) return;
        long long d = 0ll;
        for (int i=2; i<data.size(); i++) d = d * 10 + data [i] - '0';
        if (data [0] == 'J') joined = d;
        else if (data [0] == 'S') stayed = d;
        else last = d;
    }
    virtual void load_data_end (void) override {}
    virtual int save_data_start (void) override { return 3; }
    virtual string save_data_line (int line) override
    {
        char ret [50];
        switch (line)
        {
            case 0:
            sprintf (ret, "%c_%ld", 'J', joined);
            break;
            case 1:
            sprintf (ret, "%c_%ld", 'S', stayed);
            break;
            default:
            sprintf (ret, "%c_%ld", 'L', last);
        }
        return string (ret);
    }
    virtual void save_data_end (void) override {}
    virtual void receive_update (int fd) override
    {
        char ch;
        if (recv (fd, &ch, 1, 0) == 0) return;
        if (ch == 'J') last = time (nullptr);
        else stayed += time (nullptr) - last;
    }
    virtual void query_data (int fd) override
    {
        send (fd, &joined, 8, 0);
        send (fd, &stayed, 8, 0);
        send (fd, &last, 8, 0);
    }
};

#include <set>
#include <string>
#include <vector>
using std::set;
using std::string;
using std::vector;

class AdvancementChannel: public SupernovaChannel
{
    set <string> advs;
    vector <string>* temp;
    public:
    AdvancementChannel (void): temp (nullptr) {}
    virtual void initialize_data (void) override {}
    virtual void load_data_start (int) override {}
    virtual void load_data_line (string line) override { advs.insert (line); }
    virtual void load_data_end (void) override {}
    virtual int save_data_start (void) override
    {
        temp = new vector <string>;
        for (auto str : advs) temp -> push_back (str);
        return temp -> size ();
    }
    virtual string save_data_line (int line) override { return temp -> operator <::> (line); }
    virtual void save_data_end (void) override { delete temp; }
    virtual void receive_update (int fd) override
    {
        char ch; short len;
        if (recv (fd, &ch, 1, 0) == 0) return;
        if (recv (fd, &len, 2, 0) < 2) return;
        char* adv = new char [len + 1]; adv [len] = 0;
        if (recv (fd, adv, len, 0) < len) goto ret;
        if (ch == 'G') advs.insert (string (adv));
        else advs.erase (string (adv));
        ret:
        delete [] adv;
        return;
    }
    virtual void query_data (int fd) override
    {
        int cnt, len;
        cnt = advs.size ();
        send (fd, &cnt, 4, 0);
        for (auto adv : advs)
        {
            len = adv.size ();
            send (fd, &len, 4, 0);
            send (fd, adv.data (), len, 0);
        }
    }
};

SupernovaChannel* gt (UUID player) { return new GameTimeChannel (); }
SupernovaChannel* am (UUID player) { return new AdvancementChannel (); }

class Starfruit: public SupernovaPlugin
{
    public:
    virtual string name (void) override { return string ("Starfruit"); }
    virtual string version (void) override { return string ("1.0.0"); }
    virtual int channel_count (void) override { return 2; }
    virtual SupernovaChannelEntrance get_channel (int index) override
    {
        switch (index)
        {
            case 0: return gt;
            case 1: return am;
        }
    }
    virtual string channel_name (int index) override
    {
        switch (index)
        {
            case 0: return "GameTime";
            case 1: return "Advancement";
        }
    }
};

SupernovaPlugin* entrance (const char* version)
{
    return new Starfruit;
}
