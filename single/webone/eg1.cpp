#include<tmwp>
using namespace tm;
int main()
{
TMWebProjector server(8080);
server.start();
return 0;
}