#include "main.h"
#include "public.h"
#include "web_get.h"
#include "web_post.h"
#include "c_server.h"

#include <pthread.h>

/****************************************************
 * @brief  Check whether it is GET or POST
 * @note
 * @param  *method:
 * @param  Socket:
 * @retval
 ***************************************************/
const char *Judge_Method(char *method, int Socket)
{
    //判断请求方式
    if (strcmp(method, "GET") == 0)
    {
        return "GET";
    }
    else if (strcmp(method, "POST") == 0)
    {
        return "POST";
    }
    else
    {
        clienterror(Socket, method, "501", "Not Implemented", "XJH does not implement this method");
        return ERROR;
    }
}
/****************************************************
 * @brief  读取http 请求报头，
 * @note   无法使用请求报头的任何信息，读取之后忽略掉
 * @param  fd:
 * @param  *index:
 * @retval None
 ***************************************************/
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n"))
    {
        Rio_readlineb(rp, buf, MAXLINE);
    }
    return;
}
/****************************************************
 * @brief  Handle client connections
 * @note   GET or POST
 * @param  fd:
 * @retval None
 ***************************************************/
void deal_with_client_request(int fd)
{
    rio_t rio;
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];

    //初始化 rio 结构
    Rio_readinitb(&rio, fd);
    //读取http请求行
    Rio_readlineb(&rio, buf, MAXLINE);
    //格式化存入 把该行拆分
    sscanf(buf, "%s %s %s", method, url, version);
    //读取并忽略请求报头
    read_requesthdrs(&rio);

    //只能处理GET/POST请求，如果不是GET/POST请求的话返回错误
    if (Judge_Method(method, fd) == ERROR)
    {
        return;
    }
    else if(!strcasecmp("GET", method))
    {
        deal_with_get_request(fd, url);
    }
    else 
    {
        deal_with_post_request(fd, url);
    }
    return;
}
/****************************************************
 * @brief  print log
 * @note
 * @param  fd:
 * @param  *index:
 * @retval None
 ***************************************************/
void Logo(const char *ipaddr, int port)
{
    char v[16] = {0}, version[16] = {0};
    my_system("git remote -v | grep -v fetch |  tr -cd \"[0-9]\"", v, sizeof(v));
    int length = strlen(v), i = 0, j = 0;
    for (i = 0, j = 0; i < length; i++)
    {
        version[j++] = v[i];
        if (v[i+1] != '\0')
        {
            version[j++] = '.';
        }
    }
    
    printf("-----------------------------------------------------------\n");
    printf("         URL = http://%s:%d/app.html              \n", ipaddr, port);
    printf("-----------------------------------------------------------\n");
    printf("  __          ________ _______\n");
    printf("  \\ \\        / /  ____|  _____\\\n");
    printf("   \\ \\  /\\  / /| |____  |____) )\n");
    printf("    \\ \\/  \\/ / |  ____|  ____(   __  __     __ ___\n");
    printf("     \\  /\\  /  | |____  |____) )(__ |_ \\  /|_ |___)\n");
    printf("      \\/  \\/   |______|_______/  __)|__ \\/ |__|   \\\n");
    printf("\n");
    printf("            Welcome to use the Web Server!\n");
    printf("                     Version %s\n\n", version);
    printf("                         XJH\n");
    printf("-----------------------------------------------------------\n");
    return;
}
/****************************************************
 * @brief  main function
 * @note
 * @param  argc:
 * @param  **argv:
 * @retval
 ***************************************************/
void* Handle_web_client_connection_events(void *argv)
{
    pthread_detach(pthread_self());
    int *connfd = (int*)argv;
    deal_with_client_request(*connfd);
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    char *ip = NULL;
    int port = 8080;
    struct sockaddr_in clientaddr;

    char ipaddr[64] = {0};
    get_local_ip_addr(ipaddr, sizeof(ipaddr));
    switch (argc)
    {
        case 1: port = PORT;    Logo(ipaddr, port); break;
        case 2: if(strchr(argv[1], '.'))
                {
                    port = PORT;
                    ip = argv[1];
                    Logo(ip, port);
                }
                else
                {
                    port = atoi(argv[1]);
                    Logo(ipaddr, port);
                }                                                           break;
        case 3: port = atoi(argv[1]);   ip = argv[2];   Logo(ip, port);     break;
        default:fprintf(stderr, "Usage: %s <port>\n", argv[0]); exit(1);    break;
    }
    listenfd = Open_listenfd(port, ip);
    printf("The web server has been started.....\n");

    /*信号处理函数,用来处理僵尸进程*/
    signal_r(SIGCHLD, sigchild_handler);

    pthread_t thread_web;
    while (1)
    {
        clientlen = sizeof(clientaddr);
        if ((connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
                printf("Accept error...");
        }
        printf("port = %d, addr = %s\n", ntohs(clientaddr.sin_port), inet_ntoa(clientaddr.sin_addr));
        pthread_create(&thread_web, NULL, Handle_web_client_connection_events, (void*)&connfd);
        pthread_join(thread_web, NULL);
    }
}