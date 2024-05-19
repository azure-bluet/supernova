// This is the daemon process for Supernova server
#include "api.cpp"
#include <bits/stdc++.h>
using namespace std;
#include <sys/socket.h>
#include <netinet/in.h>
#define debugerr(info) cerr << #info, perror (strerror (errno)), 0
int main (void)
{
    sockaddr_in lo;
    lo.sin_addr.s_addr = INADDR_ANY;
    lo.sin_family = AF_INET;
    lo.sin_port = 16587;
    const sockaddr* addr = (sockaddr*) &lo;
    while (true)
    {
        if (! ping ()) return debugerr (Cannot ping server:);
        save ();
        cout << "SAVE DAEMON: Successfully pinged the server." << endl;
        sleep (60);
    }
}
