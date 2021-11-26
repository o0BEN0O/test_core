#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/stat.h>
#include "string.h"
#include "queue.h"
#include "uemf.h"
#include "webs.h"
#include "sock_clt.h"
#include "webs_define.h"

static jrd_sock_info_t jrd_sock_info;
static pthread_mutex_t jrd_sock_mute;
static pthread_cond_t jrd_webs_tx_cond;
static pthread_mutex_t jrd_webs_tx_mute;
static pthread_cond_t jrd_sock_tx_cond;
static pthread_mutex_t jrd_sock_tx_mute;
static q_type jrd_webs_tx_q;
static q_type jrd_sock_tx_q;
int remote_fd;
int local_fd;

extern char * jrd_webs_token_str_extern;
extern jrd_webs_pridata_t g_webs_data;

static void jrd_sock_msg_q_free(jrd_sock_msg_q_t* q_ptr)
{
  jrd_sock_msg_t *p_sock_msg = NULL;

  if(!q_ptr)
  {
    //error handler here
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_sock_msg_q_free q_ptr is NULL\n");
    return;
  }
  p_sock_msg = q_ptr->sock_msg;
  if(p_sock_msg)
  {
    if(p_sock_msg->data_buf)
    {
      free((p_sock_msg->data_buf));
      p_sock_msg->data_buf = NULL;
    }
    free(p_sock_msg);   
    p_sock_msg = NULL;
  }
  free(q_ptr);  
  q_ptr = NULL;
  
  return;
}

static int
client_connect(char *path, int* p_fd)
{
  struct sockaddr_un client_addr;
  int  sock_fd = JRD_SOCK_INVALID_FD;
  int len, rc;

  if(!p_fd)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_connect: invalid fd pointer\n");
    return -1;
  }
  /* Get the connection listener socket */
  pthread_mutex_lock(&jrd_sock_mute);
  if(*p_fd != JRD_SOCK_INVALID_FD)
  {
    pthread_mutex_unlock(&jrd_sock_mute);
    return 0;
  }
  if ((sock_fd = socket (AF_UNIX,SOCK_STREAM,0)) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_connect: unable to open client socket, fd = %d\n", sock_fd);
    goto fail;
  }

  /* setup for bind */
  memset (&client_addr,0, sizeof (struct sockaddr_un));
  len = strlen (path);
  len = MIN(len, (int)(sizeof(client_addr.sun_path)-1));
  client_addr.sun_family = AF_UNIX;
  client_addr.sun_path[0]='\0';
  memcpy (&client_addr.sun_path[1], path, len);

  len = 1+offsetof (struct sockaddr_un, sun_path) + len;
  /* Connect to the server's connection socket */
  if ((rc = connect (sock_fd, (struct sockaddr *) &client_addr, len)) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_connect[%d]: unable to connect to server, rc=%d\n",
                   sock_fd,
                   rc);
    goto fail;    
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_connect [%d]: successfully connected to server\n",
                     sock_fd);
  }

  if(p_fd)
  {
    *p_fd = sock_fd;
  }

  pthread_mutex_unlock(&jrd_sock_mute);
  return 0;

fail:
  if (sock_fd != JRD_SOCK_INVALID_FD)
  {
    close(sock_fd);
  }
  pthread_mutex_unlock(&jrd_sock_mute);
  return -1;
}

static int jrd_put_data_to_web_tx_q
(
  char  *rx_buff,
  int sock_len
)
{
  char * ptr_tmp;
  jrd_sock_msg_t *p_sock_msg;
  p_sock_msg = (jrd_sock_msg_t*)malloc(sizeof(jrd_sock_msg_t));
  if(!p_sock_msg)
  {
    //Error handler here, should hang up
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc sock_msg_p failed, should hang up program\n");
    return -1;
  }
  memset(p_sock_msg, 0, sizeof(jrd_sock_msg_t));
  ptr_tmp = rx_buff;
  if(!p_sock_msg->data_buf)
  {
    //add by ou, should be confirmed, why add 1
    p_sock_msg->data_buf = (uint8*)malloc(sock_len);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "client_recv_msg, rcv buf size: %d\n", sock_len);
  }
  if(!p_sock_msg->data_buf)
  {
    //error log here
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_recv_msg, malloc data_buf failed, buf size: %d\n", sock_len);
    return -1;
  }

  memset(p_sock_msg->data_buf, 0, sock_len);        
  memcpy(&(p_sock_msg->msg_hdr), ptr_tmp, sizeof(jrd_sock_hdr_t));
  ptr_tmp += sizeof(jrd_sock_hdr_t);
  p_sock_msg->data_len = sock_len - sizeof(jrd_sock_hdr_t);
  memcpy(p_sock_msg->data_buf, ptr_tmp, p_sock_msg->data_len);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "client_recv_msg, len: %d, data_buf: %s\n", gstrlen(p_sock_msg->data_buf), p_sock_msg->data_buf);
  do
  {
    jrd_sock_msg_q_t* sock_msg_q_data = NULL;
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "receiv data from core-app!\n");
    sock_msg_q_data = (jrd_sock_msg_q_t*)malloc(sizeof(jrd_sock_msg_q_t));
    if(!sock_msg_q_data)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc sock_msg_q_data failed, should hang up program\n");
      //Error handler here, should hang up
      continue;
    }
    memset(sock_msg_q_data, 0, sizeof(jrd_sock_msg_q_t));
    sock_msg_q_data->sock_msg = p_sock_msg;
    pthread_mutex_lock(&jrd_webs_tx_mute);
    q_link(sock_msg_q_data, &sock_msg_q_data->link);
    q_put(&jrd_webs_tx_q, &sock_msg_q_data->link);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "notify recv msg to jrd_webs_tx_thread, msg: 0x%08X\n", sock_msg_q_data);
    pthread_cond_signal(&jrd_webs_tx_cond);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "put to webs tx q end!\n");
    pthread_mutex_unlock(&jrd_webs_tx_mute);
  }while(0);
  return 0;
}

static int
client_recv_msg
(
  jrd_sock_info_t  *p_sock_info
)
{
  static boolean             b_initialized = FALSE;
  fd_set                select_fd_set;
  int   sock_fd = JRD_SOCK_INVALID_FD;
  uint8*                 ptr_tmp = NULL;
  int                      max_fd=0;  
  int connect_retry_cnt = 0;
  int rc = 0;
  int sock_len = 0;
  static int remind_buf_size = 0;
  int offset = 0;
  int sock_head = 0;

  if(!p_sock_info)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_recv_msg, invalid thread parameter pointer\n");
    return -1;
  }
  if(JRD_SOCK_INVALID_FD == p_sock_info->sock_fd)
  {
     while(client_connect(JRD_SERVER_PATH, &p_sock_info->sock_fd))
     {
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_connect failed retry again, retry count: %d\n", ++connect_retry_cnt);
       sleep(1);
     }
  }
  //else
  if(FALSE == b_initialized)
  {
     FD_ZERO (&select_fd_set);    
     FD_SET (p_sock_info->sock_fd, &select_fd_set);               
     b_initialized = TRUE;
  }    
  max_fd = (p_sock_info->sock_fd + 1);
  if(select (max_fd, &select_fd_set, NULL, NULL, NULL) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_rx_msg[%d] : select errno[%d:%s]\n",
                   p_sock_info->sock_fd,
                   errno,
                   strerror(errno));
    return -1;
  }
  /* Read in new message header */
  
  rc = recv (p_sock_info->sock_fd,
             (void *)(p_sock_info->rx_buf + remind_buf_size),
             JRD_SOCK_WEB_RX_BUF_SIZE - remind_buf_size,
             0);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "recv buff size:%d\n", rc);
  if(rc < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_rx_msg[%d] : recv returns errno[%d:%s]\n",
                    p_sock_info->sock_fd,
                    errno,
                    strerror(errno));
    //continue;
    return -1;
  }
  else if(0 == rc)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_rx_msg[%d] : server go away[%d:%s]\n",
                    p_sock_info->sock_fd,
                    errno,
                    strerror(errno));
      
    close(p_sock_info->sock_fd);
    p_sock_info->sock_fd = JRD_SOCK_INVALID_FD;
    b_initialized = FALSE;
    return -1;
  }
  else
  {
    remind_buf_size += rc;
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "recv total buff len:%d\n",remind_buf_size);
    if(sizeof(jrd_sock_hdr_t)+2*sizeof(int) >= remind_buf_size)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "msg not end 1\n");
      return 0;
    }
    else
    {
      memcpy(&sock_head, p_sock_info->rx_buf, sizeof(int));
      if(sock_head != PACK_HEAD)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "sock head error:%08x\n",sock_head);
        return 0;
      }
      memcpy(&sock_len, p_sock_info->rx_buf+sizeof(int), sizeof(int));
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sock len:%d\n",sock_len);
      
      if(remind_buf_size < sock_len)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "msg not end 2\n");
        return 0;
      }
    }
  }
  
  offset = 0;
  while(remind_buf_size >= sock_len)
  {
    if(jrd_put_data_to_web_tx_q(p_sock_info->rx_buf + offset + 2*sizeof(int), sock_len - 2*sizeof(int)))
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_put_data_to_web_tx_q fail!\n");
      return -1;
    }
    remind_buf_size -= sock_len;

    offset += sock_len;
    if (remind_buf_size == 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sock finished\n");
      return 0;
    }
    else if(remind_buf_size < 2*sizeof(int)+sizeof(jrd_sock_hdr_t))
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sock not finish,buf_size:%d\n",remind_buf_size);
      memcpy(p_sock_info->rx_buf,p_sock_info->rx_buf + offset, remind_buf_size);
      return 0;
    }
    else
    {
      memcpy(&sock_head, p_sock_info->rx_buf + offset, sizeof(int));
      if(sock_head != PACK_HEAD)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "sock head error:%08x\n",sock_head);
        return 0;
      }
      memcpy(&sock_len, p_sock_info->rx_buf + offset + sizeof(int), sizeof(int));
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sock len:%d\n",sock_len);
      if(remind_buf_size < sock_len)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sock not finish,buf_size:%d,sock_len:%d\n",remind_buf_size,sock_len);
        memcpy(p_sock_info->rx_buf,p_sock_info->rx_buf + offset, remind_buf_size);
        return 0;
      }
    }
  }
  return 0;
}

static int client_send_msg
(
  jrd_sock_info_t  *p_sock_info,   
  jrd_sock_msg_t *p_sock_msg
)
{
  int                rc = 0;
  int                total_len = 0;
  uint8*           ptr_tmp = NULL;
  int pack_head = PACK_HEAD;

  if(!p_sock_msg || !p_sock_info)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_send_msg, invalid send parameters\n");
    return -1;
  }
  ptr_tmp = p_sock_info->tx_buf;
  memcpy(ptr_tmp, &pack_head, sizeof(int));//add pack head
  ptr_tmp += sizeof(int);
  ptr_tmp += sizeof(int);//reserv for pack len
  total_len += 2*sizeof(int);
  memcpy(ptr_tmp, &(p_sock_msg->msg_hdr), sizeof(jrd_sock_hdr_t));
  ptr_tmp += sizeof(jrd_sock_hdr_t);
  total_len += sizeof(jrd_sock_hdr_t);
  memcpy(ptr_tmp, p_sock_msg->data_buf, p_sock_msg->data_len);
  total_len += p_sock_msg->data_len;
  memcpy(p_sock_info->tx_buf + sizeof(int), &total_len, sizeof(int));//add pack len to buff
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "total_len: %d\n", total_len);

  //JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "client_send_msg, len: %d, content: %s\n", p_sock_msg->data_len, p_sock_msg->data_buf);
  if ((rc = send (p_sock_info->sock_fd,
                  (void *) p_sock_info->tx_buf,
                  (size_t)total_len,
                  0)) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_send_msg, send failed\n");
	 return -1;
  }
 
   return 0;
}

static void *
jrd_sock_rx_thread
(
void *info
)
{
  jrd_sock_info_t  *p_sock_info = (jrd_sock_info_t *)info;
  //jrd_sock_msg_t* sock_msg_p = NULL;
  //jrd_sock_msg_q_t* sock_msg_q_data = NULL;

  if(!p_sock_info)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_recv_msg, invalid thread parameter pointer, should hang up program\n");
    //Error handler here, should hang up
    return NULL;
  }

  while(1)
  {
    /*sock_msg_p = (jrd_sock_msg_t*)malloc(sizeof(jrd_sock_msg_t));
    if(!sock_msg_p)
    {
      //Error handler here, should hang up
	  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc sock_msg_p failed, should hang up program\n");
      continue;
    }
    memset(sock_msg_p, 0, sizeof(jrd_sock_msg_t));*/
    while(client_recv_msg(p_sock_info))
    {
      //error log here
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_recv_msg, return error, retry again\n");
    }
    //else
    /*{
      sock_msg_q_data = (jrd_sock_msg_q_t*)malloc(sizeof(jrd_sock_msg_q_t));
      if(!sock_msg_q_data)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc sock_msg_q_data failed, should hang up program\n");
        //Error handler here, should hang up
        continue;
      }
      memset(sock_msg_q_data, 0, sizeof(jrd_sock_msg_q_t));
      sock_msg_q_data->sock_msg = sock_msg_p;
      pthread_mutex_lock(&jrd_webs_tx_mute);
      q_link(sock_msg_q_data, &sock_msg_q_data->link);
      q_put(&jrd_webs_tx_q, &sock_msg_q_data->link);
      pthread_cond_signal(&jrd_webs_tx_cond); 
      pthread_mutex_unlock(&jrd_webs_tx_mute);
    }*/
  } 
}

void app_client_msg_async_cb
(
  uint32  txn_id,
  msg_enum_type msg_type,
  uint8 *rx_buf,
  uint32 rx_buf_len,
  void   *rx_cb_data
)
{
   JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "app_client_msg_async_cb\n");
   if(!rx_buf)
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "rx_buf is NULL\n");
   jrd_put_data_to_web_tx_q(rx_buf,rx_buf_len);
}

static void *
jrd_sock_tx_thread
(
void *info
)
{
  jrd_sock_info_t  *p_sock_info = (jrd_sock_info_t *)info;
  jrd_sock_msg_t* sock_msg_p = NULL;
  jrd_sock_msg_q_t* sock_msg_q_data = NULL;
  uint8 *temp_buf = NULL;
  uint32 temp_buf_len = 0;
  int rc = 0;
  int sock_fd = 0;

  if(!p_sock_info)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_recv_msg, invalid thread parameter pointer, should hang up program\n");
    //Error handler here, should hang up
    return NULL;
  }  
  while(1)
  {    
    pthread_mutex_lock(&jrd_sock_tx_mute); 
    while((sock_msg_q_data = q_get(&jrd_sock_tx_q)) == NULL)
    {
      pthread_cond_wait(&jrd_sock_tx_cond, &jrd_sock_tx_mute);
    }
    pthread_mutex_unlock(&jrd_sock_tx_mute);
    sock_msg_p = sock_msg_q_data->sock_msg;
    sock_fd = sock_msg_q_data->sock_fd;
    //rc = client_send_msg(p_sock_info, sock_msg_p);
    temp_buf_len = sock_msg_p->data_len + sizeof(jrd_sock_hdr_t);
    temp_buf = malloc(temp_buf_len);
    if(temp_buf == NULL)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "malloc temp_buf fail!\n");
      //Error handler here, should hang up
      return NULL;
    }
    memcpy(temp_buf, &(sock_msg_p->msg_hdr), sizeof(jrd_sock_hdr_t));
    memcpy(temp_buf+sizeof(jrd_sock_hdr_t), sock_msg_p->data_buf, sock_msg_p->data_len);
    rc = client_send_async_msg(sock_fd,temp_buf,temp_buf_len,app_client_msg_async_cb,NULL);
    /*rc = client_send_sync_msg(sock_msg_p->data_buf,sock_msg_p->data_len,recv_buf,&recv_buf_len);
    if(recv_buf_len)
      jrd_put_data_to_web_tx_q(recv_buf,recv_buf_len);
    else
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "recv buf len is 0!!!\n");*/
    free(temp_buf);
    temp_buf = NULL;
    jrd_sock_msg_q_free(sock_msg_q_data);
    sock_msg_q_data = NULL;
    if(rc)
    {
      //error log & handler here
      continue;
    }     
  }
}

int jrd_sock_test_response_flag = 0;
int jrd_main_sock_response_flag = 0;

static void *
jrd_webs_tx_thread
(
void *info
)
{
  jrd_sock_info_t  *p_sock_info = (jrd_sock_info_t *)info;
  jrd_sock_msg_t* sock_msg_p = NULL;
  jrd_sock_msg_q_t* sock_msg_q_data = NULL;
  webs_t wp = NULL;
  int rc = 0;
  int send_time = 0,i = 0;
  char send_data[WEBS_BUFSIZE];
  char *ptr = NULL;
  if(!p_sock_info)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid thread parameter pointer, should hang up program\n");
    //Error handler here, should hang up
    return NULL;
  }
  
  while(1)
  {   
    pthread_mutex_lock(&jrd_webs_tx_mute);   
    while((sock_msg_q_data = q_get(&jrd_webs_tx_q)) == NULL)
    {
      pthread_cond_wait(&jrd_webs_tx_cond, &jrd_webs_tx_mute);
    }
    pthread_mutex_unlock(&jrd_webs_tx_mute);
    sock_msg_p = sock_msg_q_data->sock_msg;
    wp = (webs_t)(sock_msg_p->msg_hdr.usr_p);
    if(wp == 0xFFFFFFFF)
    {
      jrd_sock_test_response_flag = 1;
      jrd_sock_msg_q_free(sock_msg_q_data); 
      continue;
    }
    if(!wp || !websValid(wp))
    {
      //error handler here
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid wp handler 0x%08X\n", wp);
      jrd_sock_msg_q_free(sock_msg_q_data);    
      continue;
    }
    if(sock_msg_p->msg_hdr.data_start)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "msg_type = %d, cust_flag = %d\n", sock_msg_p->msg_hdr.msg_type, sock_msg_p->msg_hdr.cust_flag);
      if (sock_msg_p->msg_hdr.msg_type == E_JSON_MSG_RSP)
      {
        websJsonHeader(wp, sock_msg_p->msg_hdr.content_len, sock_msg_p->msg_hdr.cust_flag);
      }
      else if(sock_msg_p->msg_hdr.msg_type == E_TN_MSG_RSP)
      {
#ifdef JRD_EE_CUST_FEATURE_DEVINFO
        if(E_CUST_EE_DEVINFO == sock_msg_p->msg_hdr.cust_flag)
        {
          websJsonHeader_ee_cust(wp);
        }
        else
#endif
        {
          webstnHeader(wp);
        }
      }
          
    }
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "wp: 0x%08X, send to web %s\n", wp, sock_msg_p->data_buf);
  	send_time = (sock_msg_p->data_len)/(WEBS_BUFSIZE-1);
  	if(((sock_msg_p->data_len)%(WEBS_BUFSIZE-1)) != 0)
  	{
  	  send_time = send_time + 1;
  	}
  	ptr = (char*)sock_msg_p->data_buf;
  	for(i=0;i<send_time;i++)
  	{
  	  memset(send_data,0,sizeof(send_data));
  	  if((i+1) == send_time)
  	  {
  	    memcpy(send_data,ptr,sock_msg_p->data_len-i*(WEBS_BUFSIZE-1));
  	  }
  	  else
  	  {
  	    memcpy(send_data,ptr,(WEBS_BUFSIZE-1));
  	    ptr = ptr + (WEBS_BUFSIZE-1);
  	  }
  	  websWrite(wp, T("%s"), send_data);
  	}
    if(sock_msg_p->msg_hdr.data_complete)
    {
#ifdef JRD_WEBS_DONE_MUTEX
       pthread_mutex_lock(&webs_done_handle_mutex);
#endif
       websDone(wp, 200);
#ifdef JRD_WEBS_DONE_MUTEX
       pthread_mutex_unlock(&webs_done_handle_mutex); 
#endif
    }
    jrd_sock_msg_q_free(sock_msg_q_data);
  }
}

int  start_sock_client(char *path)
{
   pthread_t                         thrd_sock_rx_id;
   pthread_t                         thrd_sock_tx_id;
   pthread_t                         thrd_web_tx_id;
   int rc = 0;
   int connect_retry_cnt = 0;
   pthread_attr_t sock_rx_attr, sock_tx_attr, web_tx_attr;

   pthread_mutex_init(&jrd_sock_mute, NULL); 
   memset(&jrd_sock_info, 0, sizeof(jrd_sock_info_t));
   jrd_sock_info.sock_fd = JRD_SOCK_INVALID_FD;
   pthread_cond_init(&jrd_webs_tx_cond, NULL);
   pthread_mutex_init(&jrd_webs_tx_mute, NULL); 
   q_init(&jrd_webs_tx_q);
   pthread_cond_init(&jrd_sock_tx_cond, NULL);
   pthread_mutex_init(&jrd_sock_tx_mute, NULL);
   q_init(&jrd_sock_tx_q);
   
   remote_fd = jrd_init_app_client_inet("192.168.225.1",2016);
   local_fd = jrd_init_app_client(JRD_SERVER_PATH);

   /*while((rc = client_connect(path, &(jrd_sock_info.sock_fd))) != 0)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_connect failed retry again, retry count: %d\n", ++connect_retry_cnt);
     sleep(1);
   }
   if ( pthread_attr_init(&sock_rx_attr) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "init thread attribute failed\n");
  }

  if ( pthread_attr_setstacksize (&sock_rx_attr, JRD_OEM_SOCK_CLT_STACK_SIZE) !=0)
  {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set stack size failed\n");
  }

  if ( pthread_attr_setdetachstate(&sock_rx_attr,
                      PTHREAD_CREATE_DETACHED) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set setdetachstate failed\n");
  }
   if ((pthread_create (&thrd_sock_rx_id,
                        &sock_rx_attr,
                        jrd_sock_rx_thread,
                        (void *)(&jrd_sock_info))) != 0)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "start_app_client [%d]:  can't create RX thread\n",jrd_sock_info.sock_fd);
     if(JRD_SOCK_INVALID_FD != jrd_sock_info.sock_fd)
     {
        close(jrd_sock_info.sock_fd);
        jrd_sock_info.sock_fd = JRD_SOCK_INVALID_FD;
     }

     return -1;
   }*/
   if ( pthread_attr_init(&sock_tx_attr) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "init thread attribute failed\n");
  }

  if ( pthread_attr_setstacksize (&sock_tx_attr, JRD_OEM_SOCK_CLT_STACK_SIZE) !=0)
  {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set stack size failed\n");
  }

  if ( pthread_attr_setdetachstate(&sock_tx_attr,
                      PTHREAD_CREATE_DETACHED) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set setdetachstate failed\n");
  }
   if ((pthread_create (&thrd_sock_tx_id,
                        &sock_tx_attr,
                        jrd_sock_tx_thread,
                        (void *)(&jrd_sock_info))) != 0)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "start_app_client [%d]:  can't create RX thread\n",jrd_sock_info.sock_fd);
     /*if(JRD_SOCK_INVALID_FD != jrd_sock_info.sock_fd)
     {
        close(jrd_sock_info.sock_fd);
        jrd_sock_info.sock_fd = JRD_SOCK_INVALID_FD;
     }*/

     return -1;
   }

   if ( pthread_attr_init(&web_tx_attr) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "init thread attribute failed\n");
  }

  if ( pthread_attr_setstacksize (&web_tx_attr, JRD_OEM_SOCK_CLT_STACK_SIZE) !=0)
  {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set stack size failed\n");
  }

  if ( pthread_attr_setdetachstate(&web_tx_attr,
                      PTHREAD_CREATE_DETACHED) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set setdetachstate failed\n");
  }
   if ((pthread_create (&thrd_web_tx_id,
                        &web_tx_attr,
                        jrd_webs_tx_thread,
                        (void *)(&jrd_sock_info))) != 0)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "start_app_client [%d]:  can't create RX thread\n",jrd_sock_info.sock_fd);
     /*if(JRD_SOCK_INVALID_FD != jrd_sock_info.sock_fd)
     {
        close(jrd_sock_info.sock_fd);
        jrd_sock_info.sock_fd = JRD_SOCK_INVALID_FD;
     }*/

     return -1;
   }

    return 0;
}

static int webs_creat_q_and_put_q(webs_t wp,e_jrd_sock_msg_type msg_type, uint32 usr_p, uint32 data_len, uint8* data_buf);

static void *
jrd_sock_test_thread
(
void *info
)
{
  //jrd_sock_info_t  *p_sock_info = (jrd_sock_info_t *)info;
  //jrd_sock_msg_t* sock_msg_p = NULL;
  //jrd_sock_msg_q_t* sock_msg_q_data = NULL;
  uint8 *temp_buf = "{\"jsonrpc\":\"2.0\",\"method\":\"TestRequest\",\"params\":null,\"id\":\"99.99\"}";
  uint32 temp_buf_len = gstrlen(temp_buf)+1;
  uint8 *data_buf = NULL;
  
  int rc = 0;
  sleep(10);
  while(1)
  {
    data_buf = malloc(temp_buf_len);
    memcpy(data_buf, temp_buf, temp_buf_len);
    data_buf[temp_buf_len -2] = 0;
    jrd_sock_test_response_flag = 0;
    jrd_main_sock_response_flag = 0;
    webs_creat_q_and_put_q(NULL,E_JSON_MSG_REQ, 0xFFFFFFFF, temp_buf_len, data_buf);
    sleep(6);
    if(!jrd_sock_test_response_flag)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "*****core_app no respons\n");
      system("echo restart_core_app > /dev/kmsg");
      system("start-stop-daemon -S -b -o -a /usr/oem/restart_core_app_webs both");
      sleep(10);
    }
    else if(!jrd_main_sock_response_flag)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "*****webs no respons\n");
      system("echo webs > /dev/kmsg");
      system("start-stop-daemon -S -b -o -a /usr/oem/restart_core_app_webs webs");
    }
    else
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "core_app respons received\n");
    }
  }
}
int start_sock_test_thread(void)
{
  pthread_attr_t sock_test_attr;
  pthread_t  thrd_sock_test_id;
  if ( pthread_attr_init(&sock_test_attr) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "init thread attribute failed\n");
  }

  if ( pthread_attr_setstacksize (&sock_test_attr, 32*1024) !=0)
  {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set stack size failed\n");
  }

  if ( pthread_attr_setdetachstate(&sock_test_attr,
                      PTHREAD_CREATE_DETACHED) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set setdetachstate failed\n");
  }
   if ((pthread_create (&thrd_sock_test_id,
                        &sock_test_attr,
                        jrd_sock_test_thread,
                        NULL)) != 0)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "create thread failed!!!\n");
     return -1;
   }
}

int jrd_printf(char* sprint_buf, int size, char *fmt, ...)
{
  va_list args;
  int n;
  
  va_start(args, fmt);
  n = vsnprintf(sprint_buf, size, fmt, args);
  va_end(args);
  
  return n;
}

#define JRD_WEB_RAND_KEY T("rand=0.")
#define JRD_WEB_DUMMY_POST_KEY T("d=1")
#define JRD_WEB_DUMMY_POST_KEY_2 T("some=a")
#define JRD_WEB_DUMMY_POST_KEY_3 T("page_reboot=login.html")
#define JRD_WEB_FILTER_REQ T("uploadBackupSettings")
#define JRD_WEB_PICOPOINT_REQ T("activatePicopoint")

#ifdef JRD_DEBUG_CATCH_LOG
#define JRD_DEBUG_FILTER_REQ T("importLogCfgFile")
#endif

#define JRD_WEB_SWAP_BUF_SIZE (JRD_SOCK_WEB_TX_BUF_SIZE - sizeof(jrd_sock_hdr_t))
static uint8 jrd_web_swap_buf[JRD_WEB_SWAP_BUF_SIZE] = {0};

int jrd_write_file(const char* file_path_name,const char *data, const int data_len)
{
  int loc_write, numLeft, numWrite;
  FILE *       fp;
  if ((fp = fopen(file_path_name, "w")) == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "open file error\n");
    return -1;
  } 
  else 
  {
    loc_write = 0;
    numLeft = data_len;
    while (numLeft > 0) 
    {
      numWrite = fwrite(&(data[loc_write]), sizeof(*(data)), numLeft, fp);
      loc_write += numWrite;
      numLeft -= numWrite;
      if(numWrite == 0)
      {
        fclose(fp);
        return -1;
      }
    }
    if (fclose(fp) != 0) 
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "close file error\n");
      return -1;
    } 
  }
  return 0;
}

// Connie add start, 2014/12/16
/* The param data_buf need to be malloced before invoke webs_creat_q_and_put_q()*/
static int webs_creat_q_and_put_q(webs_t wp,e_jrd_sock_msg_type msg_type, uint32 usr_p, uint32 data_len, uint8* data_buf)
{
  jrd_sock_msg_t*   p_jrd_sock_msg        = NULL;
  jrd_sock_msg_q_t* p_jrd_sock_msg_q_data = NULL;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "start!\n");
    
  p_jrd_sock_msg_q_data = (jrd_sock_msg_q_t*)malloc(sizeof(jrd_sock_msg_q_t));
  if(!p_jrd_sock_msg_q_data)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "malloc p_jrd_sock_msg_q_data failed\n");
    if(data_buf) free(data_buf);
    return -1;
  }
  memset(p_jrd_sock_msg_q_data, 0,  sizeof(jrd_sock_msg_q_t));
  p_jrd_sock_msg = (jrd_sock_msg_t*)malloc(sizeof(jrd_sock_msg_t));
  if(!p_jrd_sock_msg)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "malloc p_jrd_sock_msg failed\n");
    if(data_buf) free(data_buf);
    free(p_jrd_sock_msg_q_data);
    return -1;
  }
  memset(p_jrd_sock_msg, 0,  sizeof(jrd_sock_msg_t));

  p_jrd_sock_msg->msg_hdr.msg_type = msg_type;
  p_jrd_sock_msg->msg_hdr.usr_p    = usr_p;
  p_jrd_sock_msg->data_len         = data_len;
  p_jrd_sock_msg->data_buf         = data_buf;
  
  pthread_mutex_lock(&jrd_sock_tx_mute);
  p_jrd_sock_msg_q_data->sock_msg = p_jrd_sock_msg;
  if (wp == NULL)
  	p_jrd_sock_msg_q_data->sock_fd = local_fd;
  else	
  	p_jrd_sock_msg_q_data->sock_fd = wp->sock_fd;
  q_link(p_jrd_sock_msg_q_data, &(p_jrd_sock_msg_q_data->link));
  q_put(&jrd_sock_tx_q, &(p_jrd_sock_msg_q_data->link));
  pthread_cond_signal(&jrd_sock_tx_cond);
  pthread_mutex_unlock(&jrd_sock_tx_mute);
  return 0;
}
// Connie add end, 2014/12/16

// Connie add start, 2014/11/11
static int websFormatJsonRequest(char_t *req_name, char_t *filename, char_t *id, char_t *req_buf, int req_buf_len)
{
	struct json_object *post_obj = NULL;
	struct json_object *param_obj = NULL;
  const char *json_data;
  int copy_len;

  if (req_buf == NULL || req_buf_len == 0)
    return -1;
    
  post_obj = json_object_new_object();
  if (post_obj == NULL)
  {
    return -1;
  }
  
  param_obj = json_object_new_object();
  if (param_obj == NULL)
  {
    json_object_put(post_obj);
    return -1;
  }
  
  json_object_object_add(param_obj, "filename", json_object_new_string(filename));
  json_object_object_add(post_obj, "jsonrpc", json_object_new_string("2.0"));
  json_object_object_add(post_obj, "method", json_object_new_string(req_name));
  json_object_object_add(post_obj, "params", param_obj);
  json_object_object_add(post_obj, "id", json_object_new_string(id));
  json_data = json_object_to_json_string(post_obj);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Connie: websFormatJsonRequest, json_data: %s, length: %d\n", json_data, strlen(json_data));
  
  memset(req_buf, 0, req_buf_len);  
  copy_len = (strlen(json_data) < req_buf_len) ? strlen(json_data):(req_buf_len - 1);
  memcpy(req_buf, json_data, copy_len);  

  json_object_put(post_obj);
	return copy_len;
}
// Connie add end, 2014/11/11
//#define Y858_JSON_RESTORE
// Connie add start, 2014/12/16
int websJRDtnHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	char_t *url, char_t *path, char_t *query)
{
  uint8* data_ptr = NULL;
  char_t		*cp = NULL, *req_name = NULL;
  boolean post_data_need = TRUE;
  char post_key_file_name[128] = "filename=";
  int  JsonRequestlen;
  uint32 data_len = 0;
  uint8* data_buf = NULL;
  
  //JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "\nreq_wp: 0x%08X\nurlPrefix: %s\nwebDir:%s\narg:%d\nurl:%s\npath:%s\nquery:%s\n", wp, urlPrefix, webDir, arg, url, path, query);
  
  if(!wp || !path)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid wp or path pointer, wp: 0x%08X, path: 0x%08X\n", wp, path);
    return 0;
  }
  req_name = gstrchr(&path[1], '/');
  if(!req_name)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "req_name is NULL\n");
    return 0;
  }
  req_name++;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "req_name is %s\n",req_name);
  if(!strncmp(req_name, JRD_WEB_FILTER_REQ, sizeof(JRD_WEB_FILTER_REQ))
  || !strncmp(req_name, JRD_WEB_PICOPOINT_REQ, sizeof(JRD_WEB_PICOPOINT_REQ)))
  {
    char *fn = NULL;
    char file_name[128] = "/tmp/jrd_dir/";
    char *token = NULL;
    post_data_need = FALSE;
    
    if(g_webs_data.json_method_token_check)
    {
      token = websGetVar(wp, T("_TclRequestVerificationToken"), T(""));
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "token is :%s\n",token);
      if(gstrlen(token))
      {
        token = jrd_decrypt(token);
      }
      else if(wp->token)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "wp->token is :%s\n",wp->token);
        token = wp->token;
      }
      if(gstrcmp(jrd_webs_token_str_extern,token)!=0)
      {
        char id[MAX_ID_LENGTH] = {0};
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "token is incorrect of method:%s, wp->token:%s\n",req_name,token);
        
        json_webs_get_id(wp, id, MAX_ID_LENGTH);
        json_webs_rsp_err(wp,WEBS_TOKEN_ERROR,id);
        return 1;
      }
    }
    
    fn = websGetVar(wp, T("filename"), T(""));
    if (fn == NULL || *fn == '\0')
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "filename is NULL\n");
      return 0;
    }
    else
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "filename fn is %s\n",fn);
      strcat(file_name,fn);
    }
    if(jrd_write_file(file_name, wp->postData, wp->lenPostData))
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "write bak file failed\n");
      return 0;
    }
    strcat(post_key_file_name,file_name);
    #ifdef Y858_JSON_RESTORE
    JsonRequestlen = websFormatJsonRequest(req_name, file_name, "13.8", jrd_web_swap_buf, JRD_WEB_SWAP_BUF_SIZE);
    #endif
  }
  
  #ifdef JRD_DEBUG_CATCH_LOG
  if(!strncmp(req_name, JRD_DEBUG_FILTER_REQ, sizeof(JRD_DEBUG_FILTER_REQ)))
  {
    char *fn = NULL;
    char file_name[128] = "/usr/oem/";
    post_data_need = FALSE;
    
    fn = websGetVar(wp, T("filename"), T(""));
    if (fn == NULL || *fn == '\0')
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "filename is NULL\n");
      return 0;
    }
    else
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "filename fn is %s\n",fn);
      strcat(file_name,fn);
    }
    if(jrd_write_file(file_name, wp->postData, wp->lenPostData))
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "write bak file failed\n");
      return 0;
    }
    strcat(post_key_file_name,file_name);
    #ifdef Y858_JSON_RESTORE
    JsonRequestlen = websFormatJsonRequest(req_name, file_name, "13.8", jrd_web_swap_buf, JRD_WEB_SWAP_BUF_SIZE);
    #endif
  }
  #endif

#ifdef Y858_JSON_RESTORE
  if (post_data_need == FALSE && JsonRequestlen == -1)
  {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "websFormatJsonRequest fail!\n");
     return 0;
  }
#endif
  
  #ifndef Y858_JSON_RESTORE
  memset(jrd_web_swap_buf, 0, JRD_WEB_SWAP_BUF_SIZE);
  data_ptr = jrd_web_swap_buf;
  data_ptr += gsprintf((char*)data_ptr, T("%s"), req_name);
  
  if(query[0])
  {
    cp = gstrstr(query, JRD_WEB_RAND_KEY);
  	if(cp)
  	{
        //JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "STR JRD_WEB_RAND_KEY:\n%s\n", cp);
  	}
    if(cp != query && cp)
    {
      gstrcat((char*)data_ptr, T("&"));
      data_ptr += gstrlen(T("&"));
      memcpy(data_ptr, query, (cp - query -1));
      data_ptr += (cp - query -1);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "After pend query:\n%s\n", jrd_web_swap_buf);
    }
  }
  if(wp->postData && wp->lenPostData)
  {
    cp = gstrstr(wp->postData, JRD_WEB_DUMMY_POST_KEY);
    if(!cp)
    {
      cp = gstrstr(wp->postData, JRD_WEB_DUMMY_POST_KEY_2);
    }
    if(!cp)
    {
      cp = gstrstr(wp->postData, JRD_WEB_DUMMY_POST_KEY_3);
    }
    if(cp != wp->postData || !cp)
    {
      //data_ptr += gsprintf((char*)data_ptr, T("&%s"), wp->postData);
      gstrcat((char*)data_ptr, T("&"));
      data_ptr += gstrlen(T("&"));
      if(post_data_need)
      {
        memcpy(data_ptr, wp->postData, wp->lenPostData);
        data_ptr += wp->lenPostData;
      }
      else
      {
        memcpy(data_ptr, post_key_file_name, strlen(post_key_file_name)+1);
        data_ptr += (strlen(post_key_file_name)+1);
      }
      //JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "After pend post data:\n%s\n", jrd_web_swap_buf);
    }  
  }
  #endif

#ifdef Y858_JSON_RESTORE
    data_len = JsonRequestlen + 1;  /*Include '\0', so plus 1 to length*/
#else
    data_len = data_ptr - jrd_web_swap_buf + 1;
#endif
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "MAX size = %d, data_len = %d\n", (JRD_WEB_SWAP_BUF_SIZE -1), data_len);

  data_buf = (uint8*)malloc(data_len);
  if(!data_buf)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Maclloc data_buf failed\n");
    return 0;
  }
  memset(data_buf , 0, data_len);
    
  memcpy(data_buf , jrd_web_swap_buf, data_len);
  
  //data_ptr += sprintf((char*)p_jrd_sock_msg->data_buf, "%s&%s", query, wp->postData);
  //JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "data_buf: \n%s\nlen: %d, mem len: %d\n", 
  //                               data_buf, 
  //                               gstrlen(data_buf), 
   //                              data_len);
#ifdef Y858_JSON_RESTORE
  webs_creat_q_and_put_q(NULL,E_JSON_MSG_REQ, (uint32)wp, data_len, data_buf);
#else
  webs_creat_q_and_put_q(NULL,E_TN_MSG_REQ, (uint32)wp, data_len, data_buf);
#endif
  return 1;
}
#ifdef JRD_FEATURE_SDSHARE
int sdshareFileFormat(char_t *req_name, char_t *file_name)
{
	return websFormatJsonRequest(req_name, file_name, "13.8", jrd_web_swap_buf, JRD_WEB_SWAP_BUF_SIZE);
}
#endif

int websJRDJsonHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	                               char_t *url, char_t *path, char_t *query)
{
  uint32 data_len = 0;
  uint8* data_buf = NULL;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "\nreq_wp: 0x%08X\nurlPrefix: %s\nwebDir:%s\narg:%d\nurl:%s\npath:%s\nquery:%s\n", wp, urlPrefix, webDir, arg, url, path, query);
  
  if(!wp || !path)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid wp or path pointer, wp: 0x%08X, path: 0x%08X\n", wp, path);
    return 0;
  }

  data_len = wp->lenPostData + 1;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "MAX size = %d, data_len = %d\n", (JRD_WEB_SWAP_BUF_SIZE -1), data_len);

  data_buf = (uint8*)malloc(data_len);
  if(!data_buf)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Maclloc data_buf failed\n");
    return 0;
  }
  memset(data_buf , 0, data_len);
  memcpy(data_buf , wp->postData, wp->lenPostData);
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "data_buf: \n%s\nlen: %d, mem len: %d\n", 
                                 data_buf, 
                                 gstrlen(data_buf), 
                                 data_len);
  webs_creat_q_and_put_q(wp,E_JSON_MSG_REQ, (uint32)wp, data_len, data_buf);
  return 1;
}

int websJRDXMLHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	char_t *url, char_t *path, char_t *query)
{
  uint32 data_len = 0;
  uint8* data_buf = NULL;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "req_wp: 0x%08X, urlPrefix: %s\n, webDir:%s\n, arg:%d\n", wp, urlPrefix, webDir, arg);
  
  if(!wp || !path)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid wp or path pointer, wp: 0x%08X, path: 0x%08X\n", wp, path);
    return 0;
  }

  data_len = wp->lenPostData + 1;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "MAX size = %d, data_len = %d\n", (JRD_WEB_SWAP_BUF_SIZE -1), data_len);

  data_buf = (uint8*)malloc(data_len);
  if(!data_buf)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Maclloc data_buf failed\n");
    return 0;
  }
  memset(data_buf , 0, data_len);
  memcpy(data_buf , wp->postData, wp->lenPostData);
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "data_buf: \n%s\nlen: %d, mem len: %d\n", 
                                   data_buf, 
                                   gstrlen(data_buf), 
                                   data_len);
  webs_creat_q_and_put_q(NULL,E_VODAFONE_REQ, (uint32)wp, data_len, data_buf);
  return 1;
}
// Connie add end, 2014/12/16

//Li.kuang add start for bug #629025
void send_userpw_change_ind(void)
{
  e_jrd_sock_msg_ind_type* e_msg_ind      = NULL;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Connie send_userpw_change_ind start!\n");
  e_msg_ind = (e_jrd_sock_msg_ind_type*)malloc(sizeof(e_jrd_sock_msg_ind_type));  
  if(!e_msg_ind)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Maclloc p_jrd_sock_msg->data_buf failed\n");
    return;
  }
  *e_msg_ind = E_MSG_USERPW_CHANGE_IND;
  webs_creat_q_and_put_q(NULL,E_MSG_IND, 0, sizeof(e_jrd_sock_msg_ind_type), e_msg_ind);
}
//Li.kuang add end for bug #629025

static json_object* jrd_create_params_object(jrd_param_info_type* params)
{
	struct json_object *params_obj = NULL;
  int i = 0;
  
  params_obj = json_object_new_object();
  if (params_obj == NULL)
    return NULL;
  
  if (params == NULL)
    return params_obj;

  while(params[i].name != NULL)
  {
    switch (params[i].type)
    {
      case json_type_string:
        json_object_object_add(params_obj, params[i].name, json_object_new_string(params[i].u.str));
        break;
      case json_type_int:
        json_object_object_add(params_obj, params[i].name, json_object_new_int(params[i].u.val_int));
        break;
      default:
        break;
    }
    i++;
  }
  return params_obj;
}

void jrd_free_req_buf(uint8 **req_buf)
{
  free(*req_buf);
  *req_buf = NULL;
}

int jrd_create_json_req_buf(const char* method, jrd_param_info_type* req_params, uint8 **req_buf, int *req_buf_len)
{
	struct json_object *post_obj = NULL;
  const char *json_data;

  *req_buf = NULL;
  post_obj = json_object_new_object();
  if (post_obj == NULL)
    return -1;
  
  json_object_object_add(post_obj, "jsonrpc", json_object_new_string("2.0"));
  json_object_object_add(post_obj, "method", json_object_new_string(method));
  json_object_object_add(post_obj, "params", jrd_create_params_object(req_params));
  json_object_object_add(post_obj, "id", json_object_new_string("1.1"));
  json_data = json_object_to_json_string(post_obj);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "%s(%d): json_data: %s, length: %d\n", __func__, __LINE__, json_data, strlen(json_data));

  *req_buf_len = sizeof(jrd_sock_hdr_t) + strlen(json_data);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "%s(%d): req_buf_len: %d\n", __func__, __LINE__, *req_buf_len);
  
  *req_buf = malloc(*req_buf_len);
  if (*req_buf == NULL)
  {
    json_object_put(post_obj);
    return -1;
  }
  memset(*req_buf, 0, sizeof(jrd_sock_hdr_t));  
  memcpy(*req_buf + sizeof(jrd_sock_hdr_t), json_data, strlen(json_data));  

  json_object_put(post_obj);
	return 0;
}

int jrd_convert_utf8_to_unicode(const char* UTF8, uint16* unicode_data)
{
    char * UTF8_local = (char*)UTF8;
    char encode_UTF8[4] = {'\0'};
    int UTF_len = strlen(UTF8_local);
    int char_len = 0;
    int step = 0;

    if(UTF8 == NULL || unicode_data == NULL || UTF8_local == NULL){
      return step;
    }

    if (UTF_len == 0)
    {
      unicode_data[0] = 0;
      return step;
    }

    while (UTF_len != 0)
    {
        if (!(UTF8_local[0] & 0x80))
            char_len = 1;
        else if ((UTF8_local[0] & 0x80)
                &&(UTF8_local[0] & 0x40))
        {
            if (!(UTF8_local[0] & 0x20))
                char_len = 2;
            else if (UTF8_local[0] & 0x20)
            {
                if (!(UTF8_local[0] & 0x10))
                    char_len = 3;
                else if ((UTF8_local[0] & 0x10)
                          &&(!(UTF8_local[0] & 0x08)))
                    char_len = 4;
            }
        }
        
        //JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "char_len = %d\n", char_len);
        switch (char_len)
        {
            case 1:
                unicode_data[step++] = UTF8_local[0];
                UTF8_local += char_len;
                UTF_len -= char_len;
                break;
            case 2:
                encode_UTF8[0] = UTF8_local[0];
                encode_UTF8[1] = UTF8_local[1];
                unicode_data[step] = (encode_UTF8[0] & 0x1F) << 6;
                unicode_data[step++] |= (encode_UTF8[1] & 0x3F);
                UTF8_local += char_len;
                UTF_len -= char_len;
                break;
            case 3:
                encode_UTF8[0] = UTF8_local[0];
                encode_UTF8[1] = UTF8_local[1];
                encode_UTF8[2] = UTF8_local[2];
                unicode_data[step] = (encode_UTF8[0] & 0x0F) << 12;
                unicode_data[step] |= (encode_UTF8[1] & 0x3F) << 6;
                unicode_data[step++] |= (encode_UTF8[2] & 0x3F);  
                UTF8_local += char_len;
                UTF_len -= char_len;
                break;
            case 4:
                encode_UTF8[0] = UTF8_local[0];
                encode_UTF8[1] = UTF8_local[1];
                encode_UTF8[2] = UTF8_local[2];
                encode_UTF8[3] = UTF8_local[3];
                //unicode_data[step] = (encode_UTF8[0] & 0x07) << 2;
                //unicode_data[step++] |= (encode_UTF8[1] & 0x30) >> 4;
                //unicode_data[step] = (encode_UTF8[1] & 0x0F) << 12;
                //unicode_data[step] |= (encode_UTF8[2] & 0x3F) << 6;
                //unicode_data[step++] |= (encode_UTF8[3] & 0x3F);
                unicode_data[step++] = 0xF60A;
                UTF8_local += char_len;
                UTF_len -= char_len;
                break;

            default:
                break;
        }
        char_len = 0;
    }
    
    return step;
}

int jrd_convert_unicode_to_utf8(uint16_t *in, int insize, uint8_t **out)
{

    int i = 0;
    int outsize = 0;
    int charscount = 0;
    uint8_t *result = NULL;
    uint8_t *tmp = NULL; 

    charscount = insize / sizeof(uint16_t);
    result = (uint8_t *)malloc(charscount * 3 + 1);

    memset(result, 0, charscount * 3 + 1);
    tmp = result;
 
    for (i = 0; i < charscount; i++){
        uint16_t unicode = in[i];
        if (unicode >= 0x0000 && unicode <= 0x007f){
            *tmp = (uint8_t)unicode;
            tmp += 1;
            outsize += 1;
        } else if (unicode >= 0x0080 && unicode <= 0x07ff){
            *tmp = 0xc0 | (unicode >> 6);
            tmp += 1;
            *tmp = 0x80 | (unicode & (0xff >> 2));
            tmp += 1;
            outsize += 2;
        } else if (unicode >= 0x0800 && unicode <= 0xffff){
            *tmp = 0xe0 | (unicode >> 12);
            tmp += 1;
            *tmp = 0x80 | (unicode >> 6 & 0x00ff);
            tmp += 1;
            *tmp = 0x80 | (unicode & (0xff >> 2));
            tmp += 1;
            outsize += 3;
        }
    }
    *tmp = '/0';
    *out = result;
    return 0;
}

int jrd_create_tn_req_buf(const char* method, jrd_param_info_type* req_params, uint8 **req_buf, int *req_buf_len)
{
  int i = 0;
  char buf[1024] = {0};
  jrd_sock_hdr_t sock_hdr = {0};

  strncpy(buf, method, sizeof(buf));

  if(req_params != NULL)
  {
    while(req_params[i].param_id != JRD_PARAMS_ID_INVALID)
    {
      if (req_params[i].type == json_type_int)
        sprintf(buf, "%s&%s=%llu", buf, req_params[i].name, req_params[i].u.val_int);
      else if (req_params[i].type == json_type_string)
      {
        char * p_buf = NULL;
        jrd_convert_unicode_to_utf8(req_params[i].u.str, 128, &p_buf);
        sprintf(buf, "%s&%s=%s", buf, req_params[i].name, p_buf);
        free(p_buf);
      }
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "buf: %s\n", buf);
      i++;
    }
  }
  *req_buf_len = sizeof(jrd_sock_hdr_t) + strlen(buf);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "req_buf_len: %d\n", *req_buf_len);
  
  *req_buf = malloc(*req_buf_len);
  if (*req_buf == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "malloc req_buf fail!\n");
    return -1;
  }
  sock_hdr.msg_type = E_TN_MSG_REQ;
  memcpy(*req_buf, &sock_hdr, sizeof(jrd_sock_hdr_t));  
  memcpy(*req_buf + sizeof(jrd_sock_hdr_t), buf, strlen(buf));  
   
 	return 0;
}
int jrd_get_rsp_params(const char* rsp_buf, jrd_param_info_type *rsp_params)
{
  int  i = 0;
  json_object * object_rsp_data = NULL;
  json_object * object_result = NULL;
  json_object * object_one_param = NULL;

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "rsp_buf: %s\n", rsp_buf);
  object_rsp_data = json_tokener_parse(rsp_buf);
  if( is_error(object_rsp_data) ) 
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "parser data fail!\n");
    return -1;
  }
    
  object_result = json_object_object_get(object_rsp_data, "result");
  if (object_result == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "can't get result object\n");
    //json_object_put(object_rsp_data);
    //return -1;
    object_result = object_rsp_data;
  }

  if (rsp_params != NULL)
  {
    while (rsp_params[i].name != NULL)
    {
      object_one_param = json_object_object_get(object_result, rsp_params[i].name);
      if(object_one_param == NULL)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get %s\n", rsp_params[i].name);
	      i ++;
	      rsp_params[i].got_value = FALSE;
        continue;
        //json_object_put(object_rsp_data);
        //return -1;
      }
      rsp_params[i].got_value = TRUE;
      if (rsp_params[i].type == json_type_string && json_object_get_type(object_one_param) == json_type_string)
      {
        //strncpy(rsp_params[i].u.str,json_object_get_string(object_one_param),MAX_PARAM_LEN);
        memset(rsp_params[i].u.str, 0, sizeof(rsp_params[i].u.str));
        jrd_convert_utf8_to_unicode(json_object_get_string(object_one_param), (uint16*)rsp_params[i].u.str);
      } 
      else if(rsp_params[i].type == json_type_int && json_object_get_type(object_one_param) == json_type_int)
      {
        rsp_params[i].u.val_int = json_object_get_int(object_one_param);
      }
      else
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "%s intend type: %d, rsp type: %d\n", rsp_params[i].name, rsp_params[i].type, json_object_get_type(object_one_param));
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Not support getting other type param now, add further if needed!\n");
        json_object_put(object_rsp_data);
        rsp_params[i].got_value = FALSE;
        return -1;
      }
      //JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "param%d: %s\n",i, rsp_params[i].u.str);
      i++;
    }
  }
  json_object_put(object_rsp_data);
  return 0;
}

int jrd_SendRequestToCoreApp(e_jrd_sock_msg_type msg_type, const char* method, jrd_param_info_type* req_params, jrd_param_info_type* rsp_params)
{
  int rc;
  uint8 *req_buf = NULL;
  uint8 rsp_buf[3072] = {0};
  int   req_buf_len;
  int   rsp_buf_len;

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "method is %s, msg_type is %d(json:0;tn:3)\n", method, msg_type);
  // make buf
  if (msg_type == E_JSON_MSG_REQ)
    rc = jrd_create_json_req_buf(method, req_params, &req_buf, &req_buf_len);
  else if(msg_type == E_TN_MSG_REQ)
    rc = jrd_create_tn_req_buf(method, req_params, &req_buf, &req_buf_len);
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid msg_type!\n");
    return -1;
  }
  
  if(rc != 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_create_json_req_buf fail!\n");
    return rc;
  }
   
  // send req_buf to core-app and get rsp_buf
  rc = client_send_sync_msg(req_buf,req_buf_len,rsp_buf,&rsp_buf_len,5000);
  jrd_free_req_buf(&req_buf);
  if (rc != 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "client_send_sync_msg error, rc = %d\n",rc);
    return rc;
  }

  rc = jrd_get_rsp_params(rsp_buf + sizeof(jrd_sock_hdr_t), rsp_params);
  if(rc != 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "%s(%d): jrd_get_rsp_params error, rc = %d\n", __func__, __LINE__, rc);
  }
  else if(rsp_params != NULL)
  {
    int i = 0;
    while(rsp_params[i].name)
    {
      if(rsp_params[i].type == json_type_string)
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "%s: %s\n", rsp_params[i].name, rsp_params[i].u.str);
      else if (rsp_params[i].type == json_type_int)
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "%s: %llu\n", rsp_params[i].name, rsp_params[i].u.val_int);
      i++;
    }
  }
 	return rc;
}

jrd_param_info_type GetConn_status_rsp_params[] = {
  {JRD_PARAMS_ID_CONNECTION_STATUS, "ConnectionStatus", 0, json_type_int, ""},
  {JRD_PARAMS_ID_INVALID,           NULL, 0, 0, ""},
};

int jrd_GetConnStatus(int *status)
{
  int rc,i=0;
  rc = jrd_SendRequestToCoreApp(E_JSON_MSG_REQ, "GetConnectionState", NULL, GetConn_status_rsp_params);
  if(rc)
	  return rc;
	
  while(GetConn_status_rsp_params[i].param_id != JRD_PARAMS_ID_INVALID)
  {
    if (GetConn_status_rsp_params[i].param_id == JRD_PARAMS_ID_CONNECTION_STATUS)
    {
      if (GetConn_status_rsp_params[i].type == json_type_int&&GetConn_status_rsp_params[i].got_value)
      {
        *status = GetConn_status_rsp_params[i].u.val_int;
        return 0;
      }
      else
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Not support param type %d!\n", GetConn_status_rsp_params[i].type);
        return -1;
      }
    }
    i++;
  }
  return -1;
}

