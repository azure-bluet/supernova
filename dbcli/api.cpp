// You may include this
#include <algorithm>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <set>
#include <string>
#include <sys/socket.h>
using namespace std;
int _sock (void)
{
    sockaddr_in lo;
    lo.sin_addr.s_addr = INADDR_ANY;
    lo.sin_family = AF_INET;
    lo.sin_port = 16587;
    const sockaddr* addr = (sockaddr*) &lo;
    auto s = socket (PF_INET, SOCK_STREAM, 0);
    if (connect (s, addr, sizeof (lo)) == -1) return -1;
    else return s;
}
bool ping (void)
{
    auto s = _sock ();
    if (s == -1) return false;
    if (send (s, "P", 1, 0) < 1) return false;
    char tmp [8]; tmp [7] = 0;
    if (recv (s, tmp, 7, 0) < 7) return false;
    if (strcmp (tmp, "I'm OK!")) return false;
    return true;
}
void save (void)
{ send (_sock (), "S", 1, 0); }
int put_uuid (char ch, string uuid)
{
    auto s = _sock ();
    send (s, &ch, 1, 0);
    send (s, uuid.data (), 36, 0);
    return s;
}
void newplayer (string uuid)
{ put_uuid ('N', uuid); }
int send_channel_dat (char ch, string uuid, string channel, const char* upd, int updsize)
{
    auto s = put_uuid (ch, uuid);
    short l = channel.size ();
    send (s, &l, 2, 0);
    send (s, channel.data (), l, 0);
    if (updsize) send (s, upd, updsize, 0);
    char ret; recv (s, &ret, 1, 0);
    if (ret == 'Y') return s;
    else return -1;
}
void player_join (string uuid)
{ newplayer (uuid); send_channel_dat ('U', uuid, "Starfruit:GameTime", "J", 1); }
void player_leave (string uuid)
{ send_channel_dat ('U', uuid, "Starfruit:GameTime", "L", 1); }
void grant_advancement (string uuid, string adv)
{
    char* dat = new char [adv.size () + 4];
    dat [adv.size () + 3] = 0;
    dat [0] = 'G';
    short* sh = (short*) (dat+1);
    *sh = adv.size ();
    memcpy (dat+3, adv.data (), adv.size ());
    send_channel_dat ('U', uuid, "Starfruit:Advancement", dat, adv.size () + 3);
    delete [] dat;
}
void revoke_advancement (string uuid, string adv)
{
    char* dat = new char [adv.size () + 4];
    dat [adv.size () + 3] = 0;
    dat [0] = 'R';
    short* sh = (short*) (dat+1);
    *sh = adv.size ();
    memcpy (dat+3, adv.data (), adv.size ());
    send_channel_dat ('U', uuid, "Starfruit:Advancement", dat, adv.size () + 3);
    delete [] dat;
}
void playertime (string uuid, long* joined, long* stayed, long* last)
{
    auto s = send_channel_dat ('Q', uuid, "Starfruit:GameTime", nullptr, 0);
    recv (s, joined, 8, 0);
    recv (s, stayed, 8, 0);
    recv (s, last, 8, 0);
}
void get_advancements (string uuid, set <string>* advs)
{
    int cnt=0;
    auto s = send_channel_dat ('Q', uuid, "Starfruit:Advancement", nullptr, 0);
    int i, l;
    recv (s, &cnt, 4, 0);
    for (i=0; i<cnt; i++)
    {
        recv (s, &l, 4, 0);
        char* tmp = new char [l + 1];
        tmp [l] = 0;
        recv (s, tmp, l, 0);
        advs -> insert (string (tmp));
        delete tmp;
    }
}
bool existing (string uuid)
{
    char ch;
    return recv (put_uuid ('Y', uuid), &ch, 1, 0) == 1 && ch == 'Y';
}
#ifdef mian

#include <unistd.h>
int main (void)
{
    string test_uuid ("12345678-8765-4321-1234-567887654321");
    if (ping ())
    {
        player_join (test_uuid);
        sleep (1);
        player_leave (test_uuid);
        grant_advancement (test_uuid, "A");
        grant_advancement (test_uuid, "C");
        grant_advancement (test_uuid, "E");
        revoke_advancement (test_uuid, "C");
        long j, s, l;
        playertime (test_uuid, &j, &s, &l);
        set <string> advs;
        get_advancements (test_uuid, &advs);
        cout << j << ' ' << s << ' ' << l << endl;
        for (auto i : advs) cout << i << endl;
    }
}

#endif
