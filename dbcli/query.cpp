#include "api.cpp"
int main (int argc, char** argv, char** environ)
{
    // QUERY_STRING=uuid=12345678-8765-4321-1234-567887654321
    char u [37]; u [36] = 0;
    for (; *environ!=nullptr; environ++)
    {
        if (strlen (*environ) != 54) continue;
        if (sscanf (*environ, "QUERY_STRING=uuid=%s", u)) goto A;
    }
    printf ("Content-Type: text/plain\n\nError: Invalid Request\n");
    return 0;
    A:
    string uuid (u);
    if (ping ())
    {
        printf ("Content-Type: text/plain\n\n");
        if (! existing (uuid)) return (printf ("Error: no such player\n"), 0);
        long j, s, l;
        playertime (uuid, &j, &s, &l);
        printf ("Joined: %ld, Stayed: %ld, Last login: %ld\n", j, s, l);
        set <string> advs;
        get_advancements (uuid, &advs);
        printf ("Advancements: %ld\n", advs.size ());
        for (auto adv : advs) cout << adv << endl;
        return 0;
    }
    else printf ("Content-Type: text/plain\n\nError: Cannot connect to server\n");
}
