#define p // For fun

#include "supernova.hpp"

#include <cstdio>
using std::sprintf ;p

#include <cstring>
using std::memcpy ;p

#include <unordered_map>
using std::unordered_map ;p

#include <iostream>
#include <fstream>
using std::fstream ;p
using std::ios ;p
using std::cout ;p
using std::endl ;p

#include <unordered_set>
using std::unordered_set ;p

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

#define read(a,b,c) recv(a,b,c,0)
#define write(a,b,c) send(a,b,c,0)

namespace supernova
{

/*
 * SupernovaDatabase is the main class for the database
 * SupernovaPlugin is a plugin class, which adds actual storage features
 * Starfruit is the vanilla Supernova plugin
 */

/* Supernova database stores data for each player.
 * The data is stored in a human-readable format.
 * Each channel stores some data.
 * For example, AdvancementChannel stores the players' advancements
 * and GameTimeChannel stores the players' in game time
 */

/* Player Data Format (aka PDF):
 * Loaded channels: <number>
 * <channel> data: <number> line(s)
 *  <line>
 *  <line>
 */
class SupernovaPlayer
{
    public:
    bool dirty;
    UUID uuid;
    unordered_map <string, SupernovaChannel*> channels;
    SupernovaPlayer (UUID uuid, const unordered_map <string, SupernovaChannelEntrance>& map): uuid (uuid), dirty (false)
    {
        for (auto [k, v] : map) channels [k] = v (uuid);
        fstream fin ("data/" + uuid + ".pdf");
        string s;
        int i, j, l;
        fin >> s >> s >> i;
        unordered_set <string> chan;
        for (; i>0; i--)
        {
            fin >> s;
            chan.insert (s);
            auto channel = channels [s];
            fin >> s >> l >> s;
            channel -> load_data_start (l);
            for (j=0; j<l; j++)
            {
                fin >> s;
                channel -> load_data_line (s);
            }
            channel -> load_data_end ();
        }
        for (auto [k, v] : map)
        if (chan.count (k) == 0) channels [k] -> initialize_data ();
        fin.close ();
    }
    SupernovaPlayer (const unordered_map <string, SupernovaChannelEntrance>& map, UUID uuid): uuid (uuid), dirty (false)
    { for (auto [k, v] : map) (channels [k] = v (uuid)) -> initialize_data (); }
    void save (void)
    {
        if (!dirty) return;
        fstream fout ("data/" + uuid + ".pdf", ios::out);
        fout << "Loaded channels: " << channels.size () << '\n';
        for (auto [name, channel] : channels)
        {
            int lines;
            fout << name << " data: " << (lines = channel -> save_data_start ()) << " line(s)\n";
            int i;
            for (i=0; i<lines; i++) fout << ' ' << channel -> save_data_line (i) << '\n';
        }
        fout.close ();
        dirty = false;
    }
};

class Supernova
{
    const unordered_map <string, SupernovaChannelEntrance>& map;
    unordered_map <UUID, SupernovaPlayer*> players;
    public:
    Supernova (const unordered_map <string, SupernovaChannelEntrance>& map): map (map)
    {
        fstream fpl ("players.list");
        string pl;
        while (fpl >> pl, !pl.empty()) players [pl] = new SupernovaPlayer (pl, map), pl = string ();
    }
    void accept_connection (int fd)
    {
        cout << "Connection type: ";
        // There are four types of request
        char ch;
        if (read (fd, &ch, 1) == 0) return;
        if (ch == 'P')
        {
            // Ping
            // In order to make sure the Supernova server is up
            cout << "Ping" << endl;
            write (fd, "I'm OK!", 7);
            close (fd);
            return;
        }
        if (ch == 'S')
        {
            // This is the daemon thread telling the main thread to save data
            // Usually the data is saved every minute
            // First, the player list
            cout << "Save" << endl;
            cout << "Saving data!" << endl;
            fstream fpl ("players.list", ios::out);
            for (auto [uuid, player] : players)
            {
                fpl << uuid << endl;
                player -> save ();
            }
            write (fd, "saved", 5);
            close (fd);
            return;
        }
        // Get the player's UUID
        char id [37]; id [36] = 0;
        if (read (fd, id, 36) != 36) return;
        string uuid (id);
        if (ch == 'N')
        {
            // A new player joined the Minecraft server
            cout << "New" << endl;
            if (players.count (uuid) == 0)
            players [uuid] = new SupernovaPlayer (map, uuid);
            close (fd);
            return;
        }
        if (players.count (uuid) == 0)
        {
            send (fd, "N", 1, 0);
            return;
        }
        else send (fd, "Y", 1, 0);
        if (ch == 'Y') return;
        auto player = players [uuid];
        short s;
        if (read (fd, &s, 2) != 2) return;
        char* channel = new char [s + 5];
        if (read (fd, channel, s) != s) return;
        channel [s] = 0;
        string strc (channel);
        if (player -> channels.count (string (channel)) == 0) return;
        if (ch == 'U')
        {
            // Receiving updates from the Minecraft server
            cout << "Update" << endl;
            player -> channels [string (channel)] -> receive_update (fd);
            player -> dirty = true;
        }
        else
        {
            // Data query request
            cout << "Query" << endl;
            player -> channels [string (channel)] -> query_data (fd);
        }
        delete [] channel;
        close (fd);
    }
} ;p

}

#include <iostream>
using std::cerr;
using std::strcpy;

using supernova::Supernova;
using supernova::SupernovaPluginEntrance;
using supernova::SupernovaChannelEntrance;

#include <dlfcn.h>
#include <netinet/in.h>

// Ctrl+C shutting down
#include <csignal>
#include <pthread.h>
using std::signal;

void* shut (void* ex)
{
    sockaddr_in lo;
    lo.sin_addr.s_addr = INADDR_ANY;
    lo.sin_family = AF_INET;
    lo.sin_port = 16587;
    const sockaddr* addr = (sockaddr*) &lo;
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    connect (sock, addr, sizeof (lo)) == -1;
    send (sock, "S", 1, 0);
    char tmp [5]; read (sock, tmp, 5);
    if (ex) exit (0);
    return nullptr;
}

void handle (int sig)
{
    cout << endl << "Shutting down using SIGINT" << endl;
    pthread_t tid;
    pthread_create (&tid, nullptr, shut, (void*) 0xff);
}

// The first connection might be ignored sometimes
void* rip_first_connection (void*)
{
    sleep (1);
    return shut (nullptr);
}

// The daemon is now merged into the main thread
void* save_daemon (void*)
{
    while (true)
    {
        sleep (15);
        shut (nullptr);
        cout << "Autosaved" << endl;
    }
    return nullptr;
}

const char version [] = "1.0.0";

#define entrance_name "_Z8entrancePKc" // For g++ 13.2.0 on GNU/Linux
#define debugerr perror (strerror (errno))
#define port 16587
int main (void)
{
    signal (2, handle);
    cout << "Starting Supernova Database " << version << endl;
    unordered_map <string, SupernovaChannelEntrance> map;
    // Load plugins
    fstream fplugins ("plugins.list");
    string path;
    int cnt = 0;
    while (fplugins >> path, !path.empty ())
    {
        auto lib = dlopen (path.data (), RTLD_NOW);
        auto entrance = (SupernovaPluginEntrance) dlsym (lib, entrance_name);
        if (entrance == nullptr) cerr << "Error: failed to load plugin @ " << path << endl;
        else
        {
            auto pluginL = entrance (version);
            int i, j = pluginL -> channel_count ();
            for (i=0; i<j; i++)
            {
                string name = pluginL -> name () + ":" + pluginL -> channel_name (i);
                auto en = pluginL -> get_channel (i);
                if (en == nullptr) cerr << "Error: failed to load plugin channel " << name << endl;
                else cout << "Successfully loaded channel " << name << endl; map [name] = en;
            }
            cout << "Successfully loaded plugin " << pluginL -> name () << " (" << pluginL -> version () << ") @ " << path << endl;
            cnt ++;
        }
        path = string ();
    }
    if (cnt == 0)
    {
        cerr << "Fatal error: no plugin loaded. " << endl;
        return 3;
    }
    else cout << "Successfully loaded " << cnt << " plugins, " << map.size () << " channels." << endl;
    fplugins.close ();
    Supernova* supernova = new Supernova (map);
    cout << "Created Supernova instance." << endl;
    // Socket part
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        cerr << "Error creating socket: ";
        debugerr;
        return 4;
    }
    sockaddr_in servaddr;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = port;
    servaddr.sin_family = AF_INET;
    if (bind (sock, (sockaddr*) &servaddr, sizeof (servaddr)) == -1)
    {
        cerr << "Error binding: ";
        debugerr;
        return 5;
    }
    if (listen (sock, 15) == -1)
    {
        cerr << "Error listening on socket: ";
        debugerr;
        return 6;
    }
    cout << "Starting listening @ port " << port << endl;
    int cli;
    sockaddr cliaddr;
    socklen_t addrlen;
    pthread_t tid;
    pthread_create (&tid, nullptr, rip_first_connection, nullptr);
    pthread_create (&tid, nullptr, save_daemon, nullptr);
    // Here's a smile face for you
    C:
    cli = accept (sock, &cliaddr, &addrlen);
    if (cli == -1) goto C;
    supernova -> accept_connection (cli);
    close (cli);
    cout << "Handled one connection." << endl;
    goto C;
}
