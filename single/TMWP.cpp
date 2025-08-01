#include<stdio.h>
#include<string.h>
#include<windows.h>
typedef struct _request
{
char *method;
char *resource;
char isClientSideTechnologyResource;
char *mimeType;
}REQUEST;
int extensionEquals(const char *left,const char *right)
{
char a,b;
while(*left && *right)
{
a=*left;
b=*right;
if(a>=65 && a<=90) a+=32;
if(b>=65 && b<=90) b+=32;
if(a!=b) return 0;
left++;
right++;
}
return *left==*right;
}
char * getMIMEType(char * resource)
{
char *mimeType;
mimeType=NULL;
int len=strlen(resource);
if(len<5) return mimeType; 
int lastIndexOfDot=len-1;
while(lastIndexOfDot>0 && resource[lastIndexOfDot]!='.') lastIndexOfDot--;
if(lastIndexOfDot==0) return mimeType;
if(extensionEquals(resource+lastIndexOfDot+1,"html"))
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"text/html");
}
if(extensionEquals(resource+lastIndexOfDot+1,"css"))
{
mimeType=(char *)malloc(sizeof(char)*9);
strcpy(mimeType,"text/css");
}
if(extensionEquals(resource+lastIndexOfDot+1,"js"))
{
mimeType=(char *)malloc(sizeof(char)*16);
strcpy(mimeType,"text/javascript");
}
return mimeType;
}
char isClientSideResource(char *resource)
{
return 'Y'; //this will have to be change later on
}
REQUEST * parseRequest(char * bytes)
{
char method[11];
char resource[1001];
int i,j;
for(i=0;bytes[i]!=' ';i++) method[i]=bytes[i];
method[i]='\0';
i+=2;
for(j=0;bytes[i]!=' ';j++,i++) resource[j]=bytes[i];
resource[j]='\0';
printf("Request arrived for %s\n",resource);

REQUEST *request=(REQUEST *)malloc(sizeof(REQUEST));
request->method=(char *)malloc((sizeof(char)*strlen(method))+1);
strcpy(request->method,method);
if(resource[0]=='\0')
{
request->resource=NULL;
request->isClientSideTechnologyResource='Y';
request->mimeType=NULL;
}
else
{
request->resource=(char *)malloc((sizeof(char)*strlen(resource))+1);
strcpy(request->resource,resource);
request->isClientSideTechnologyResource=isClientSideResource(resource);
request->mimeType=getMIMEType(resource);
}
return request;
}
int main()
{
FILE *f;
int length;
int g;
int i;
char responseBuffer[1025]; // A Chunk of 1024 + 1 space for String terminator 
char requestBuffer[8193];  
int bytesExtracted;
int serverSocketDescriptor;
int clientSocketDescriptor;
int successCode;
int len;
struct sockaddr_in serverSocketInformation;
struct sockaddr_in clientSocketInformation;
WORD ver;
ver=MAKEWORD(1,1);
WSADATA wsaData;
WSAStartup(ver,&wsaData);
serverSocketDescriptor=socket(AF_INET,SOCK_STREAM,0);
if(serverSocketDescriptor<0)
{
printf("Unable to create socket\n");
return 0;
}
serverSocketInformation.sin_family=AF_INET;
serverSocketInformation.sin_port=htons(5050);
serverSocketInformation.sin_addr.s_addr=htonl(INADDR_ANY);

successCode=bind(serverSocketDescriptor,(struct sockaddr *)&serverSocketInformation,sizeof(serverSocketInformation));
if(successCode<0)
{
printf("Unable to bind socket to port 5050\n");
WSACleanup();
return 0;
}
listen(serverSocketDescriptor,10);

len=sizeof(clientSocketInformation);
while(1)
{
printf("TMServer is ready to accept requestBuffer on port 5050\n");

clientSocketDescriptor=accept(serverSocketDescriptor,(struct sockaddr *)&clientSocketInformation,&len);
if(clientSocketDescriptor<0)
{
printf("Unable to accept client connection");
closesocket(serverSocketDescriptor);
WSACleanup();
return 0;
}
bytesExtracted=recv(clientSocketDescriptor,requestBuffer,8192,0);
if(bytesExtracted<0)
{
// what to do is yet to be decided
}else if(bytesExtracted==0)
{
// what to do is yet to be decided
}else
{
requestBuffer[bytesExtracted]='\0';
REQUEST *request=parseRequest(requestBuffer);
if(request->isClientSideTechnologyResource)
{
if(request->resource==NULL)
{
f=fopen("index.html","rb");
if(f!=NULL) printf("Sending index.html\n");
if(f==NULL)
{
f=fopen("index.htm","rb");
if(f!=NULL) printf("Sending index.html\n");
}
if(f==NULL)
{
strcpy(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:163\n\n<DOCTYPE html><html lang='en'><head><meta charset='utf-8'><title>TM Web Projector</title></head><body><h2 style='color:red'>Resource / not found</h2></body></html>");
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
printf("Sending 404 page\n");
}
else
{
fseek(f,0,2); // move the internal pointer to the end of file
length=ftell(f);
fseek(f,0,0); // move the internal pointer to the start of file
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\nKeep-Alive: timeout=5, max=1000\n\n",length);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
while(1)
{
g=getc(f);
if(feof(f))
{
responseBuffer[i]='\0';
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
break;
}
responseBuffer[i++]=g;
if(i==1024)
{
responseBuffer[i]='\0';
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
}
}
fclose(f);
}
}
else
{
f=fopen(request->resource,"rb");
if(f!=NULL) printf("Sending %s\n",request->resource);
if(f==NULL)
{
printf("Sending 404 page\n");
char tmp[501];
sprintf(tmp,"<DOCTYPE html><html lang='en'><head><meta charset='utf-8'><title>TM Web Projector</title></head><body><h2 style='color:red'>Resource /%s not found</h2></body></html>",request->resource);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\n\n",strlen(tmp));
strcat(responseBuffer,tmp);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
}
else
{
fseek(f,0,2); // move the internal pointer to the end of file
length=ftell(f);
fseek(f,0,0); // move the internal pointer to the start of file
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\nKeep-Alive: timeout=5, max=1000\n\n",length);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
while(1)
{
g=getc(f);
if(feof(f))
{
responseBuffer[i]='\0';
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
break;
}
responseBuffer[i++]=g;
if(i==1024)
{
responseBuffer[i]='\0';
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
}
}
fclose(f);
}
}
}
else
{
// what to do in case of server side resources, is yet to be decided
}
}
} // the infinite loop related to accept method ends here
/*if(successCode>0)
{
printf("Response send\n");
}
else
{
printf("Unable to send response\n");
}*/
closesocket(clientSocketDescriptor);
closesocket(serverSocketDescriptor);
WSACleanup();
return 0;
}