/*
 * form.c -- Form processing (in-memory CGI) for the GoAhead Web server
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * 
 */

/********************************** Description *******************************/

/*
 *	This module implements the /goform handler. It emulates CGI processing
 *	but performs this in-process and not as an external process. This enables
 *	a very high performance implementation with easy parsing and decoding 
 *	of query strings and posted data.
 */

/*********************************** Includes *********************************/

#include	"wsIntrn.h"
#include <json-c/json.h>  // jie.li add, 2014/9/5
#include <common/jrd_common_def.h>
#include "sock_client.h"
/************************************ Locals **********************************/

static sym_fd_t	formSymtab = -1;			/* Symbol table for form handlers */
static sym_fd_t localAppTab = -1;
/************************************* Code ***********************************/
/*
 *	check if it's local core_app process api
 */
int websLocalappApiHandler(webs_t wp,char_t *formName)
{
	sym_t      *sp_localapp;
	sp_localapp = symLookup(localAppTab, formName);
	if (sp_localapp == NULL){
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "call from remote remote_fd:%d,\n",remote_fd);
		wp->sock_fd = remote_fd;
		return 0;
	}else{
		wp->sock_fd = local_fd;
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "call from local local_fd:%d\n",local_fd);

		return 0;
	}
}

/*
 *	Process a form request. Returns 1 always to indicate it handled the URL
 */

int websFormHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	char_t *url, char_t *path, char_t *query)
{
	sym_t		*sp;
	char_t		formBuf[FNAMESIZE];
	char_t		*cp, *formName;
	char func_name[64]={0};
	int			(*fn)(void *sock, char_t *path, char_t *args);
	// jie.li add start
	json_object * object_web_data = NULL;
	json_object * object_method = NULL;
    // jie.li add end
  char    *json_data;
	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path == '/');

	websStats.formHits++;

/*
 *	Extract the form name
 */
  if((gstrncmp(T("/jrd/webapi"), path, 11)) == 0)
  {
    json_data = malloc(wp->lenPostData + 1);
    memcpy(json_data, wp->postData, wp->lenPostData);
    json_data[wp->lenPostData] = 0;

	  object_web_data = json_tokener_parse(json_data);
    free(json_data);
	  if( is_error(object_web_data) ) 
	  {
	    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "parser data fail!\n");
	    return 0;
	  }

	  object_method = json_object_object_get(object_web_data, "method");
    if(object_method == NULL) 
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get object_method\n");
      json_object_put(object_web_data);
      return 0;
    }

	formName = json_object_get_string(object_method);
  }
  else if ((gstrncmp(T("/goform"), path, 7)) == 0)
  {
  	gstrncpy(formBuf, path, TSZ(formBuf));
  	if ((formName = gstrchr(&formBuf[1], '/')) == NULL) {
  	   JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Missing form name\n");
  		websError(wp, 200, T("Missing form name"));
  		return 1;
  	}
  	formName++;
  	if ((cp = gstrchr(formName, '/')) != NULL) {
  		*cp = '\0';
  	}
  }
  else if ((gstrncmp(T("/vodafoneapi"), path, 12)) == 0)
  {
    // parser vodafone later!
    return 0;
  }
  else
  {
    return 0;
  }
/*
 *	Lookup the C form function first and then try tcl (no javascript support 
 *	yet).
 */
   JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "formName: %s\n", formName);
   snprintf(func_name,sizeof(func_name),"%s",formName);
	sp = symLookup(formSymtab, formName);
  if (object_web_data)
    json_object_put(object_web_data); // Connie add, 2014/9/5
	if (sp == NULL) {
    	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Not fount in form table!\n");
    	return websLocalappApiHandler(wp,func_name);
	} else {
		fn = (int (*)(void *, char_t *, char_t *)) sp->content.value.integer;
		a_assert(fn);
		if (fn) {
/*
 *			For good practice, forms must call websDone()
 */
			(*fn)((void*) wp, formName, query);

/*
 *			Remove the test to force websDone, since this prevents
 *			the server "push" from a form>
 */
		}
	}
	return 1;
}
/******************************************************************************/
/*
 *	Define a form function in the "form" map space.
 */

int websFormDefine(char_t *name, void (*fn)(webs_t wp, char_t *path, 
	char_t *query))
{
	a_assert(name && *name);
	a_assert(fn);

	if (fn == NULL) {
		return -1;
	}

	symEnter(formSymtab, name, valueInteger((int) fn), (int) NULL);
	return 0;
}

/******************************************************************************/
/*
 *	Open the symbol table for forms.
 */

void websFormOpen()
{
	formSymtab = symOpen(WEBS_SYM_INIT);
}
#define JRD_ADD_PRAM_ID_TO_MASK(val,id) (val[id>>5] |= (1<<(id&31)))
static int jrd_file_get_one_json(FILE *fp, char *buffer, int max_len)
{
  int rtnval,rtnval_pre = 0;
  int string_len = 0;
  boolean is_one_json_finish = FALSE;
  boolean is_comment_line = FALSE;
  while(!feof(fp))
  { 
    rtnval = fgetc(fp);

    if(rtnval == '/')
    {
      if(rtnval_pre == '/')
      {
        is_comment_line = TRUE;
      }
      else
      {
        rtnval_pre = '/';
      }
      continue;
    }
    else
    {
      rtnval_pre = rtnval;
    }
    
    if(rtnval == EOF)
    {
      if(!string_len)
      {
        buffer[0] = '\0';
        return -1;
      }
    }
    else if(rtnval == '\n')
    {
      if(is_one_json_finish)
      {
        break;
      }
      else if(is_comment_line)
      {
        is_comment_line = FALSE;
      }
    }
    else if(rtnval == ' '|| rtnval == '\r')
    {
      continue;
    }
    else if(!is_comment_line)
    { 
      if(';' == rtnval)
      {
        is_one_json_finish = TRUE;//maybe have conmment so continue read
      }
      if(is_one_json_finish)
        continue;
      buffer[string_len++] = rtnval;
      if(string_len + 1 == max_len)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Json is too big, max len:%d\n", max_len);
        break;
      }
    }
  }
  
  buffer[string_len] = '\0';
  return string_len;
}

int jrd_oem_parser_req_conf_file
(
  char * file_name,
  sym_fd_t	symtab
)
{
  FILE *fp; 
  char json_data[768] = {0};
  int data_len = 0;
  json_object * object_data = NULL;
  json_object * info_list_obj = NULL;
  json_object * act_info_obj = NULL;
  json_object * param_list_obj = NULL;
  json_object * param_id_obj = NULL;
  int act_index = 0,param_id_index = 0;
  char *req_str,*module_str,*act_str,*param_str;
  int module_id,act_id,param_id;
  web_act_info_type **act_info = NULL;
  web_act_info_type *one_act_info = NULL;
  if((fp = fopen(file_name, "r")) == NULL) 
  { 
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"have no such file %s !\n",file_name);
      return -1; 
  }
  while(!feof(fp)) 
  { 
    data_len = jrd_file_get_one_json(fp, json_data, sizeof(json_data));
    if(data_len > 0)
    {
      object_data = json_tokener_parse(json_data);
      if(!object_data)
      {
        //parser error
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "parser json error str:%s!!!\n", json_data);
        continue;
      }
      req_str = (char *)json_object_get_string(json_object_object_get(object_data,"req"));
      if(!req_str)
      {
        continue;
      }
      info_list_obj = json_object_object_get(object_data,"info");
      
      act_index = 0;
      do{
        act_info_obj = json_object_array_get_idx(info_list_obj, act_index);//json_object_get_object(object_data);
        act_index ++;
      }while(act_info_obj);
      act_index --;
      
      if(0 == act_index)
      {
        return;
      }
      
      JRD_MALLOC((act_index+1) * sizeof(*act_info), act_info);
      symEnter(symtab, req_str, valueInteger((long)act_info), (int) NULL);
      act_index = 0;
      do{
        act_info_obj = json_object_array_get_idx(info_list_obj, act_index);//json_object_get_object(object_data);
        act_index ++;
        if(act_info_obj)
        {
          module_str = (char *)json_object_get_string(json_object_object_get(act_info_obj,"module"));
          act_str = (char *)json_object_get_string(json_object_object_get(act_info_obj,"act"));
          if(!module_str||!act_str)
          {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "module_str:%s, act_str:%s!!!\n", module_str, act_str);
            break;
          }
          module_id = JRD_ATOI(module_str);
          act_id = JRD_ATOI(act_str);
          JRD_MALLOC(sizeof(*one_act_info), one_act_info);
          *act_info++ = one_act_info;
          one_act_info->module_id = module_id;
          one_act_info->act_id = act_id;
          param_list_obj = json_object_object_get(act_info_obj,"id");
          param_id_index = 0;
          do{
            param_id_obj = json_object_array_get_idx(param_list_obj, param_id_index);
            param_str = (char *)json_object_get_string(param_id_obj);
            if(param_str)
            {
              param_id = JRD_ATOI(param_str);
            }
            else
            {
              continue;
            }
            if(param_id > 128)
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid param_id:%d!!!\n", param_id);
              continue;
            }
            
            //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "param_str:%s,param_id:%d\n", param_str,param_id);
            JRD_ADD_PRAM_ID_TO_MASK(one_act_info->sub_mask, param_id);
            param_id_index ++;
          }while(param_id_obj);
          
          //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "module:%d,act:%d,mask:0x%04x\n", one_act_info->module_id,one_act_info->act_id,one_act_info->sub_mask[0]);
        }
      }while(act_info_obj);
      json_object_put(object_data);
      JRD_MALLOC(sizeof(*one_act_info), one_act_info);
      one_act_info->module_id = MODULE_INVALID;
      *act_info = one_act_info;
    }
    else
    {
      
    }
  }
  JRD_FCLOSE(fp); 
  return 0; 
}
#define JSON_REQ_PARSER_DATA_FILE "/jrd-resource/resource/jrdcfg/json_req_config_file"
int websLocalappApiOpen()
{
	localAppTab = symOpen(WEBS_SYM_INIT);
	if(localAppTab == JRD_INVALID_SYM_FD)
	{
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_json_symtab open failed\n");
		return -1;
	}
	return 0;
}

int websLocalappApiinit(void)
{
	if (-1 != websLocalappApiOpen())
    	return jrd_oem_parser_req_conf_file(JSON_REQ_PARSER_DATA_FILE, localAppTab);
	else{
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_json_symtab open failed\n");
		return -1;
	}
}

/******************************************************************************/
/*
 *	Close the symbol table for forms.
 */

void websFormClose()
{
	if (formSymtab != -1) {
		symClose(formSymtab);
		formSymtab = -1;
	}
}

void websLocalappApiClose()
{
	if (localAppTab != -1) {
		symClose(localAppTab);
		localAppTab = -1;
	}
}

/******************************************************************************/
/*
 *	Write a webs header. This is a convenience routine to write a common
 *	header for a form back to the browser.
 */

void websHeader(webs_t wp)
{
	a_assert(websValid(wp));

	websWrite(wp, T("HTTP/1.0 200 OK\n"));

/*
 *	The Server HTTP header below must not be modified unless
 *	explicitly allowed by licensing terms.
 */
#ifdef WEBS_SSL_SUPPORT
	websWrite(wp, T("Server: %s/%s %s/%s\r\n"), 
		WEBS_NAME, WEBS_VERSION, SSL_NAME, SSL_VERSION);
#else
	websWrite(wp, T("Server: %s/%s\r\n"), WEBS_NAME, WEBS_VERSION);
#endif

	websWrite(wp, T("X-Frame-Options: SAMEORIGIN\n"));
	websWrite(wp, T("x-xss-protection: 1; mode=block\n"));

	websWrite(wp, T("Pragma: no-cache\n"));
	websWrite(wp, T("Cache-control: no-cache\n"));
	websWrite(wp, T("Content-Type: text/html\n"));
	websWrite(wp, T("\n"));
	websWrite(wp, T("<html>\n"));
}

/******************************************************************************/
/*
 *	Write a webs footer
 */

void websFooter(webs_t wp)
{
	a_assert(websValid(wp));

	websWrite(wp, T("</html>\n"));
}

/******************************************************************************/

