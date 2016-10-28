#include "StdAfx.h"
#include "OQ_HTTP_API.h"
#include "Microhttp\microhttpd.h"

int KeepHTTPRunning = 0;

#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   512

#define GET             0
#define POST            1

struct connection_info_struct
{
  int   connectiontype;
  char  *answerstring;
  struct MHD_PostProcessor *postprocessor;
};

const char *askpage = "<html><body>\
                      List of valid \"GET\" commands : <br>\
                      ProjectList - SQT capability list<br>\
                      Restore - try to restore SQT projects from CD<br>\
                      NodeList - SQT node list<br>\
                      PerformSQT - try to perform IQ/OQ/OQMVM on specified nodes<br>\
                      <BR><BR>\
                      Example of usage :<br>\
                      http://localhost:8081/ProjectList <br>\
                      http://localhost:8081/Restore <br>\
                      http://localhost:8081/NodeList <br>\
                      http://localhost:8081/PerformSQT <br>\
                      ex : http://localhost:8081/PerformSQT?NodeName=PC371&DoIQ=1&DoOQ=1&DoOQMVM=1 <br>\
                      </body></html>";

const char *errorpage = "<html><body>This doesn't seem to be right.</body></html>";


static int send_page( struct MHD_Connection *connection, const char *page )
{
  int ret;
  struct MHD_Response *response;

  response = MHD_create_response_from_buffer (strlen (page), (void *) page, MHD_RESPMEM_PERSISTENT);
  if (!response)
    return MHD_NO;

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


static int iterate_post( void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off, size_t size )
{
  struct connection_info_struct *con_info = (connection_info_struct *)coninfo_cls;

/*
  if (0 == strcmp (key, "name"))
    {
      if ((size > 0) && (size <= MAXNAMESIZE))
        {
          char *answerstring;
          answerstring = (char*)malloc (MAXANSWERSIZE);
          if (!answerstring)
            return MHD_NO;

          _snprintf (answerstring, MAXANSWERSIZE, greetingpage, data);
          con_info->answerstring = answerstring;
        }
      else
        con_info->answerstring = NULL;

      return MHD_NO;
    }
*/
  return MHD_YES;
}

static void request_completed( void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe )
{
  struct connection_info_struct *con_info = (connection_info_struct *)*con_cls;

  if (NULL == con_info)
    return;

  if (con_info->connectiontype == POST)
    {
      MHD_destroy_post_processor (con_info->postprocessor);
      if (con_info->answerstring)
        free (con_info->answerstring);
    }

  free (con_info);
  *con_cls = NULL;
}


static int answer_to_connection (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
  if (NULL == *con_cls)
    {
      struct connection_info_struct *con_info;

      con_info = (connection_info_struct *)malloc (sizeof (struct connection_info_struct));
      if (NULL == con_info)
        return MHD_NO;
      con_info->answerstring = NULL;

      if (0 == strcmp (method, "POST"))
        {
//printf("We got a POST. We are processing received data\n");
          con_info->postprocessor = MHD_create_post_processor (connection, POSTBUFFERSIZE, iterate_post, (void *) con_info);

          if (NULL == con_info->postprocessor)
            {
              free (con_info);
              return MHD_NO;
            }

          con_info->connectiontype = POST;
        }
      else
        con_info->connectiontype = GET;

      *con_cls = (void *) con_info;

      return MHD_YES;
    }

  if (0 == strcmp (method, "GET"))
    {
printf("We got a GET. We answer with default page. URL was %s\n", url);
        int ret = HandleURL( connection, url );
        if( ret == 0 )
            return send_page (connection, askpage);
        else 
            return ret;
    }

  if (0 == strcmp (method, "POST"))
    {
      struct connection_info_struct *con_info = (connection_info_struct *)*con_cls;

      if (*upload_data_size != 0)
        {
//printf("We got a POST. We are processing received data\n");
          MHD_post_process (con_info->postprocessor, upload_data, *upload_data_size);
          *upload_data_size = 0;

          return MHD_YES;
        }
      else if (NULL != con_info->answerstring)
      {
//printf("We got a POST. We reply with a page after processing data\n");
        return send_page (connection, con_info->answerstring);
      }
    }

  return send_page (connection, errorpage);
}

struct MHD_Daemon *daemon = NULL;
int OQAPI_HTTP_Start( int port )
{
    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, port, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);
    if (NULL == daemon)
        return 1;

    //no errors
    return 0;
}

void OQAPI_HTTP_Stop()
{
    //kill HTTP thread
    if( daemon )
        MHD_stop_daemon (daemon);
}
